#include "base.h"
#include "cpu.h"
#include "mem.h"
#include "video.h"
#include <SDL.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

SDLComponents *initialize_sdl_core(void) {
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
            "Couldn't initialize SDL: %s", SDL_GetError());
        exit(1);
    }
    SDLComponents *sdl = malloc(sizeof(SDLComponents));
    if (sdl == NULL) {
        printf("Failed to allocate sdl struct\n");
        exit(1);
    }

    sdl->window = SDL_CreateWindow("DMG emulator",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        EMU_WIDTH_PX, EMU_HEIGHT_PX,
        SDL_WINDOW_RESIZABLE
    );
    sdl->surface = SDL_GetWindowSurface(sdl->window);
    sdl->renderer = SDL_CreateSoftwareRenderer(sdl->surface);
    SDL_RenderSetLogicalSize(sdl->renderer, GB_WIDTH_PX, GB_HEIGHT_PX);
    //SDL_SetSurfaceBlendMode(sdl->surface, SDL_BLENDMODE_NONE);
    //SDL_SetColorKey(sdl->surface, SDL_TRUE, 0);
    SDL_SetRenderDrawColor(sdl->renderer, 0, 0, 0, 0xFF);
    SDL_RenderClear(sdl->renderer);
    
    return sdl;
}

void teardown_sdl_core(SDLComponents *sdl) {
    //SDL_FreeSurface(state->gb_surface);
    SDL_DestroyRenderer(sdl->renderer);
    SDL_DestroyWindow(sdl->window);
    free(sdl);
    SDL_Quit();
}

GBState *initialize_gb(MemInitFlag flag) {
    GBState *state = malloc(sizeof(GBState));

    state->cpu = initialize_cpu();
    state->counter = 0;

    state->sdl = initialize_sdl_core();
    state->ppu = initialize_ppu();
    state->mem = initialize_memory(flag);
    state->timer = initialize_timer();
    state->dma = initialize_dma();
    state->should_quit = OFF;

    /*
        * Load the boot ROM and the cartridge header at least
        * Need info from the header to set up memory (0x147 - cartridge type,
          0x148 - ROM size, 0x149 - RAM size)
    */
    return state;
}

void teardown_gb(GBState *state) {
    teardown_sdl_core(state->sdl);
    teardown_cpu(state->cpu);
    teardown_memory(state->mem);
    teardown_ppu(state->ppu);
    teardown_timer(state->timer);
    teardown_dma(state->dma);
    free(state);
}

void task_event(GBState *state) {
    SDL_Event *ev = &state->sdl->event;
    SDL_PollEvent(ev);
    switch (ev->type) {
         case SDL_QUIT:
            state->should_quit = ON;
            break;
        case SDL_KEYDOWN:
            printf("Key %s pressed (%s)\n", 
                SDL_GetScancodeName(ev->key.keysym.scancode),
                SDL_GetKeyName(ev->key.keysym.sym)
            );
            break;
        case SDL_KEYUP:
            printf("Key %s released (%s)\n", 
                SDL_GetScancodeName(ev->key.keysym.scancode),
                SDL_GetKeyName(ev->key.keysym.sym)
            );
            break;
        default:
            break;
    }
}

GBTask gb_tasks[] = {
    {.period=1<<12,
    .mask=0x0FFF,
    .run_task=&task_event
    },
    {.period=256,
    .mask=0xFF,
    .run_task=&task_div_timer
    },
    {.period=16,
    .mask=0xF,
    .run_task=&task_tima_timer
    },
    /*
    {.period=4,
    .run_task=&task_interrupt_cycle
    },*/
    {.period=4,
    .run_task=&task_dma_cycle
    },
    {.period=1,
    .mask=0,
    .run_task=&task_ppu_cycle
    }, 
    {.period=4,
    .mask=0x3,
    .run_task=&task_cpu_m_cycle
    },
    {.period=0,
    .mask=-1,
    .run_task=NULL
    }
};

/* The main loop for Game Boy operation. The iterations of this loop are 
what synchronizes everything, through the gb_tasks array. Checks each entry in gb_tasks against the counter and 
runs the task if counter is divisible by the task's period. Then increments the counter. 
*/
void main_loop(GBState *state) {
    GBTask task;
    int t = 0;
    float elapsed = 0.0;
    clock_t pre, post;

    pre = clock();
    while (state->should_quit == OFF) {
        t = 0;
        task = gb_tasks[0];
        while (task.mask != -1) {
            if (!(state->counter & task.mask)) {
                (*task.run_task)(state);
            }
            t++;
            task = gb_tasks[t];
        }

        state->counter++; 
    }
    post = clock();
    elapsed = (float)(post - pre)/CLOCKS_PER_SEC;

    printf("Cycles: %d\nSeconds: %f (%f MHz)\n", 
    state->counter, elapsed, (state->counter/elapsed)/exp2(20));
}

/* Reads the cartridge header from a given open file descriptor.
Seeks the file back to the start afterward. Returns the raw data
casted into CartridgeHeader struct, which must later be freed by
caller.
 */
CartridgeHeader *read_cart_header(FILE *fp) {
    BYTE *_header = malloc(sizeof(CartridgeHeader));
    if (_header == NULL) {
        printf("Error allocating header\n");
        return NULL;
    }
    memset(_header, 0, sizeof(CartridgeHeader));

    if (fseek(fp, 0x100, SEEK_SET) != 0) {
        printf("Error seeking to 0x100 to read header\n");
        free(_header);
        return NULL;
    }
    int n_read = fread(_header, 1, sizeof(CartridgeHeader), fp);
    if (n_read != sizeof(CartridgeHeader)) {
        printf("Read wrong number %d of bytes\n", n_read);
        free(_header);
        return NULL;
    }
    if (fseek(fp, 0, SEEK_SET) != 0) {
        printf("Error seeking back to 0 after read header\n");
        free(_header);
        return NULL;
    }

    CartridgeHeader *header = (CartridgeHeader *)_header;
    return header;
    
}

#ifdef GB_MAIN

int main(int argc, char *argv[]) {

    if (argc < 2) {
        printf("Usage: gb <ROM file>\n");
        exit(2);
    }
    GBState *state;
    
    FILE *fp;
    fp = fopen(argv[1], "r");
    int n_read = 0;

    if (fp == NULL) {
        printf("Error opening %s\n", argv[1]);
        exit(1);
    }

    CartridgeHeader *header = read_cart_header(fp);

    if (header->cartridge_type == 0x00 &&
        header->rom_size == 0 &&
        header->rom_size == 0) {
            state = initialize_gb(BASIC);
            n_read = fread(
                ((BasicCartState *)state->mem->cartridge->state)->rom, 
                1, 
                0x8000, 
                fp
            );
            if (n_read != 0x8000) {
                printf("Error: ready %04x rather than 0x8000\n", n_read);
            }
    } else {
        printf("Currently unsupported cart type %02x, rom type %02x, ram type %02x\n",
            header->cartridge_type, header->rom_size, header->ram_size);
        
        fclose(fp);
        exit(1);
    }
    
    fclose(fp);

    print_state_info(state, 1);

    main_loop(state);
    
    teardown_gb(state);
    return 0;
}

#endif