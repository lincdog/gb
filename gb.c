#include "base.h"
#include "cpu.h"
#include "mem.h"
#include "video.h"
#include <SDL.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

const SDL_Color colors[] = {
    {.r = 0xFF, .g = 0xFF, .b = 0xFF, .a = 0xFF },
    {.r = 0x90, .g = 0x90, .b = 0x90, .a = 0xFF },
    {.r = 0x50, .g = 0x50, .b = 0x50, .a = 0xFF },
    {.r = 0x00, .g = 0x00, .b = 0x00, .a = 0xFF }
};


void initialize_sdl_core(GBState *state) {
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
            "Couldn't initialize SDL: %s", SDL_GetError());
        exit(1);
    }

    state->gb_window = SDL_CreateWindow("Test",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        EMU_WIDTH_PX, EMU_HEIGHT_PX,
        SDL_WINDOW_RESIZABLE
    );
    state->gb_window_surface = SDL_GetWindowSurface(state->gb_window);
    state->gb_surface = new_8bit_surface(GB_WIDTH_PX, GB_HEIGHT_PX, COLORS_BGWIN);
    
    //state->gb_renderer = SDL_CreateSoftwareRenderer(state->gb_surface);
    //SDL_RenderSetLogicalSize(ppu->gb_renderer, GB_WIDTH_PX, GB_HEIGHT_PX);
    //SDL_RenderSetScale(ppu->gb_renderer, 4, 4);
    /*ppu->gb_surface = new_8bit_surface(
        GB_WIDTH_PX, 
        GB_HEIGHT_PX, 
        COLORS_BGWIN
    );
    SDL_SetSurfaceAlphaMod(ppu->gb_surface, 0xFF);
    */
}

void teardown_sdl_core(GBState *state) {
    //SDL_FreeSurface(state->gb_surface);
    SDL_DestroyRenderer(state->gb_renderer);
    SDL_DestroyWindow(state->gb_window);
    SDL_Quit();
}

GBState *initialize_gb(MemInitFlag flag) {
    GBState *state = malloc(sizeof(GBState));
    initialize_sdl_core(state);

    state->cpu = initialize_cpu();
    state->counter = 0;

    state->ppu = initialize_ppu();
    state->mem = initialize_memory(flag);
    state->timer = initialize_timer();


    /*
        * Load the boot ROM and the cartridge header at least
        * Need info from the header to set up memory (0x147 - cartridge type,
          0x148 - ROM size, 0x149 - RAM size)
    */
    return state;
}

void teardown_gb(GBState *state) {
    teardown_sdl_core(state);
    teardown_cpu(state->cpu);
    teardown_memory(state->mem);
    teardown_ppu(state->ppu);
    teardown_timer(state->timer);
    free(state);
}

void task_event(GBState *state) {
    SDL_PollEvent(&state->event);
    switch (state->event.type) {
        
    }
}

int _dummy(void) { return 1; }

GBTask gb_tasks[] = {
    {.period=1,
    .run_task=&task_event
    },
    {.period=256,
    .run_task=&task_div_timer
    },
    {.period=16,
    .run_task=&task_tima_timer
    },
    /*
    {.period=4,
    .run_task=&task_interrupt_cycle
    },
    {.period=4,
    .run_task=&task_dma_cycle
    },
    {.period=1,
    .run_task=&task_ppu_cycle
    },
    */
    {.period=4,
    .run_task=&task_cpu_m_cycle
    },
    {.period=0,
    .run_task=NULL
    }
};

void main_loop(GBState *state, int n_cycles) {
    GBTask task;
    int t = 0;

    while ((n_cycles < 0) || (state->counter < n_cycles)) {
        t = 0;
        task = gb_tasks[0];
        while (task.period != 0) {

            if ((state->counter % task.period) == 0) {
                (*task.run_task)(state);
            }
            t++;
            task = gb_tasks[t];
        }

        state->counter++;
    }
}

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

    main_loop(state, -1);
    
    teardown_gb(state);
    return 0;
}

#endif