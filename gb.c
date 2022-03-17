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

static my_timer_t a, b, c, d, e;

void timer_init(my_timer_t *t, uint64_t bad_thresh) {
    t->max_nsec = 0;
    t->total_nsec = 0;
    t->n_calls = 0;
    t->bad_thresh = bad_thresh;
    t->n_bad = 0;
}

void timer_begin(my_timer_t *t) {
    t->pre = clock_gettime_nsec_np(CLOCK_MONOTONIC);
}
void timer_split(my_timer_t *t) {
    uint64_t diff = (clock_gettime_nsec_np(CLOCK_MONOTONIC) - t->pre)>>10;
    t->total_nsec += diff;
    if (diff > t->max_nsec)
        t->max_nsec = diff;
    
    if (diff > t->bad_thresh)
        t->n_bad++;
    
    t->n_calls++;
}
float timer_get_average(my_timer_t *t) {
    return (float)t->total_nsec / (float)t->n_calls;
}

void timer_print_result(my_timer_t *t, char *name) {
    printf("Timer %s: %llu usec, %llu calls, %f per call, %llu max, %llu above %llu\n",
    name, t->total_nsec, t->n_calls, timer_get_average(t), t->max_nsec,
    t->n_bad, t->bad_thresh);
}

SDLComponents *initialize_sdl_core(void) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
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
    SDL_SetRenderDrawColor(sdl->renderer, 0, 0, 0, 0xFF);
    SDL_RenderClear(sdl->renderer);
    
    sdl->button_select = 0xFF;
    sdl->action_buttons = 0xFF;
    sdl->direction_buttons = 0xFF;

    return sdl;
}

void teardown_sdl_core(SDLComponents *sdl) {
    //SDL_FreeSurface(state->gb_surface);
    SDL_DestroyRenderer(sdl->renderer);
    SDL_DestroyWindow(sdl->window);
    free(sdl);
    SDL_Quit();
}

GBState *initialize_gb(CartridgeHeader *header) {
    GBState *state = malloc(sizeof(GBState));
    timer_init(&a, 500); 
    timer_init(&b, 1000); 
    timer_init(&c, 100); 
    timer_init(&d, 100); 
    timer_init(&e, 100);

    state->mem = initialize_memory(header);
    if (state->mem == NULL) {
        free(state);
        return NULL;
    }
    
    state->cpu = initialize_cpu();
    state->counter = 0;

    state->sdl = initialize_sdl_core();
    state->ppu = initialize_ppu();
    ppu_early_init(state);
    
    state->timer = initialize_timer();
    state->dma = initialize_dma();
    state->should_quit = OFF;

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

    printf("GB Timing statistics\n");
    timer_print_result(&a, "A");
    timer_print_result(&b, "B");
    timer_print_result(&c, "C");
    timer_print_result(&d, "D");
    timer_print_result(&e, "E");
}


void task_event(GBState *state) {
    SDLComponents *sdl = state->sdl;
    SDL_Event *ev = &sdl->event;
    int button_pressed = 0;
    //SDL_PollEvent(ev);
    while(SDL_PollEvent(ev)) {
        switch (ev->type) {
            case SDL_QUIT:
                state->should_quit = ON;
                break;
            case SDL_KEYDOWN:
                if (! ev->key.repeat)
                    printf("Key %s pressed (%s)\n", 
                        SDL_GetScancodeName(ev->key.keysym.scancode),
                        SDL_GetKeyName(ev->key.keysym.sym)
                    );

                switch (ev->key.keysym.scancode) {
                    case KEY_MAP_DOWN:
                        button_pressed = 1;
                        joypad_down_press(sdl->direction_buttons);
                        break;
                    case KEY_MAP_UP:
                        button_pressed = 1;
                        joypad_up_press(sdl->direction_buttons);
                        break;
                    case KEY_MAP_LEFT:
                        button_pressed = 1;
                        joypad_left_press(sdl->direction_buttons);
                        break;
                    case KEY_MAP_RIGHT:
                        button_pressed = 1;
                        joypad_right_press(sdl->direction_buttons);
                        break;
                    case KEY_MAP_A:
                        button_pressed = 1;
                        joypad_a_press(sdl->action_buttons);
                        break;
                    case KEY_MAP_B:
                        button_pressed = 1;
                        joypad_b_press(sdl->action_buttons);
                        break;
                    case KEY_MAP_SELECT:
                        button_pressed = 1;
                        joypad_select_press(sdl->action_buttons);
                        break;
                    case KEY_MAP_START:
                        button_pressed = 1;
                        joypad_start_press(sdl->action_buttons);
                        break;
                    default:
                        break;
                }
                if (button_pressed) {
                    REQUEST_INTERRUPT(state, INT_JOYPAD);
                    if (state->cpu->state == STOP)
                        state->cpu->state = PREINIT;
                }
                break;
            case SDL_KEYUP:
                printf("Key %s released (%s)\n", 
                    SDL_GetScancodeName(ev->key.keysym.scancode),
                    SDL_GetKeyName(ev->key.keysym.sym)
                );
                switch (ev->key.keysym.scancode) {
                    case KEY_MAP_DOWN:
                        joypad_down_release(sdl->direction_buttons);
                        break;
                    case KEY_MAP_UP:
                        joypad_up_release(sdl->direction_buttons);
                        break;
                    case KEY_MAP_LEFT:
                        joypad_left_release(sdl->direction_buttons);
                        break;
                    case KEY_MAP_RIGHT:
                        joypad_right_release(sdl->direction_buttons);
                        break;
                    case KEY_MAP_A:
                        joypad_a_release(sdl->action_buttons);
                        break;
                    case KEY_MAP_B:
                        joypad_b_release(sdl->action_buttons);
                        break;
                    case KEY_MAP_SELECT:
                        joypad_select_release(sdl->action_buttons);
                        break;
                    case KEY_MAP_START:
                        joypad_start_release(sdl->action_buttons);
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }
}

GBTask gb_tasks[] = {
    {
    .mask=0xFFFF,
    .run_task=&task_event
    },
    {
    .mask=0xFF,
    .run_task=&task_div_timer
    },
    {
    .mask=0xF,
    .run_task=&task_tima_timer
    },
    {
    .mask=0x3,
    .run_task=&task_dma_cycle
    },
    {
    .mask=0,
    .run_task=&task_ppu_cycle
    }, 
    {
    .mask=0x3,
    .run_task=&task_cpu_m_cycle
    },
    {
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
    register unsigned long t = 0;
    double elapsed = 0.0;
    time_t pre, post;

    pre = time(NULL);
    while (state->should_quit == OFF) {
        t = state->counter;
        timer_begin(&a);
        task_ppu_cycle(state);
        timer_split(&a);

        if (!(t & 0x3)) {
            if (!(t & 0xF)) {
                if (!(t & 0xFF)) {
                    
                    if (!(t & 0xFFFF)) {
                        timer_begin(&b);
                        task_event(state);
                        timer_split(&b);
                    }
                    task_div_timer(state);
                }
                task_tima_timer(state);
            }
            
            timer_begin(&c);
            task_dma_cycle(state);
            timer_split(&c);

            timer_begin(&d);
            task_cpu_m_cycle(state);
            timer_split(&d);
        }

        state->counter++; 
    }
    post = time(NULL);
    elapsed = difftime(post, pre);

    printf("Cycles: %lu\nSeconds: %f (%f MHz)\n", 
    state->counter, elapsed, (state->counter/elapsed)/exp2(20));
    printf("Frames: %d (%f frames per second)\n", 
    state->ppu->frame.n_frames, state->ppu->frame.n_frames / elapsed);
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
    state = initialize_gb(header);
    if (state == NULL) {
        printf("Error in initializing emulator state\n");
        fclose(fp);
        exit(1);
    }
    
    if (state->mem->read_rom(state, fp) != 0) {
        printf("Error reading ROM into memory\n");
        fclose(fp);
        exit(1);
    }
    
    fclose(fp);

    main_loop(state);
    
    teardown_gb(state);

    return 0;
}

#endif