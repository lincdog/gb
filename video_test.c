#include "base.h"
#include "mem.h"
#include "video.h"
#include "video.c"
#include <stdlib.h>
#include <string.h>
#include <SDL.h>



const BYTE test_tiles_packed[][16] = {
    {
        0xFF, 0x00, 0x7E, 0xFF, 
        0x85, 0x81, 0x89, 0x83,
        0x93, 0x85, 0xA5, 0x8B, 
        0xC9, 0x97, 0x7E, 0xFF
    },
    {
        0x55, 0x33, 0x55, 0x33,
        0x55, 0x33, 0x55, 0x33,
        0x55, 0x33, 0x55, 0x33,
        0x55, 0x33, 0x55, 0x33
    },
    {
        0xF0, 0xF0, 0x00, 0xF0,
        0xF0, 0x00, 0x00, 0x00,
        0xF0, 0x00, 0x00, 0xF0,
        0xF0, 0xF0, 0x00, 0x00
    }
};

const BYTE test_tilemap[] = {
    0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0,
    1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1,
    2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2,
    0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0,
    1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1,
    2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2,
    0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0,
    1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1,
    2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2,
    0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0,
    1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1,
    2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2,
    0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0,
    1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1,
    2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2,
    0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0
};

const BYTE test_tiles_unpacked[][64] = {
    {
        1, 1, 1, 1, 1, 1, 1, 1,
        2, 3, 3, 3, 3, 3, 3, 2,
        3, 0, 0, 0, 0, 1, 0, 3,
        3, 0, 0, 0, 1, 0, 2, 3,
        3, 0, 0, 1, 0, 2, 1, 3,
        3, 0, 1, 0, 2, 1, 2, 3,
        3, 1, 0, 2, 1, 2, 2, 3,
        2, 3, 3, 3, 3, 3, 3, 2
    },
    {
        0, 1, 2, 3, 0, 1, 2, 3,
        0, 1, 2, 3, 0, 1, 2, 3,
        0, 1, 2, 3, 0, 1, 2, 3,
        0, 1, 2, 3, 0, 1, 2, 3,
        0, 1, 2, 3, 0, 1, 2, 3,
        0, 1, 2, 3, 0, 1, 2, 3,
        0, 1, 2, 3, 0, 1, 2, 3,
        0, 1, 2, 3, 0, 1, 2, 3,
    },
    {
        3, 3, 3, 3, 0, 0, 0, 0,
        2, 2, 2, 2, 0, 0, 0, 0,
        1, 1, 1, 1, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        1, 1, 1, 1, 0, 0, 0, 0,
        2, 2, 2, 2, 0, 0, 0, 0,
        3, 3, 3, 3, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0
    }
};

typedef struct {
    GBState *state;
    BYTE *mem;
    BYTE *final_pixels;
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Surface *gb_surface;
    SDL_Texture *gb_texture;
} VideoTestState;

VideoTestState *initialize_video_tests(GBState *state) {
    VideoTestState *vtstate = malloc(sizeof(VideoTestState));
    vtstate->state = state;
    vtstate->mem = ((DebugMemState *)state->mem->system->state)->mem;
    
    vtstate->final_pixels = (BYTE *)malloc(GB_HEIGHT_PX * GB_WIDTH_PX);
    memset(vtstate->final_pixels, 0, GB_HEIGHT_PX*GB_WIDTH_PX);

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
            "Couldn't initialize SDL: %s", SDL_GetError());
        return NULL;
    }

    vtstate->window = SDL_CreateWindow("Test",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        EMU_WIDTH_PX, EMU_HEIGHT_PX,
        SDL_WINDOW_RESIZABLE
    );
    vtstate->renderer = SDL_CreateRenderer(vtstate->window, -1, 0);
    vtstate->gb_surface = SDL_CreateRGBSurface(
        0, 
        GB_WIDTH_PX, 
        GB_HEIGHT_PX, 
        8,
        0,
        0,
        0,
        0
    );

    SDL_SetPaletteColors(vtstate->gb_surface->format->palette, bgwin_colors, 0, 4);
    vtstate->gb_texture = NULL;

    return vtstate;
}

SDL_Surface *run_test(VideoTestState *vtstate) {
    GBState *state = vtstate->state;
    PPUState *ppu = state->ppu;
    /* Set LCDC to known state */
    ppu->lcdc.lcd_enable = ON;
    ppu->lcdc.win_map_area = AREA0;
    ppu->lcdc.window_enable = OFF;
    ppu->lcdc.bg_win_data_area = AREA1;
    ppu->lcdc.bg_map_area = AREA0;
    ppu->lcdc.obj_size = _8x8;
    ppu->lcdc.obj_enable = OFF;
    ppu->lcdc.bg_window_enable = ON;

    ppu->misc.scx = 0;
    ppu->misc.scy = 0;
    ppu->misc.wx = 7;
    ppu->misc.wy = 10;

    BYTE *mem = vtstate->mem;
    /* Set up tile data at area 1 */
    memcpy(&mem[0x8000], &test_tiles_packed[0], 16);
    memcpy(&mem[0x8010], &test_tiles_packed[1], 16);
    memcpy(&mem[0x8020], &test_tiles_packed[2], 16);

    /* Set up tile map at area 0 */
    memcpy(&mem[0x9800], &test_tilemap, 32*32);

    return ppu_render_picture(state);
}

void teardown_video_tests(VideoTestState *vtstate) {
    free(vtstate->final_pixels);
    SDL_DestroyTexture(vtstate->gb_texture);
    SDL_FreeSurface(vtstate->gb_surface);
    SDL_DestroyRenderer(vtstate->renderer);
    SDL_DestroyWindow(vtstate->window);
    free(vtstate);
}

int main(int argc, char *argv[]) {
   
    SDL_Texture *texture, *test_texture;
    SDL_Surface *gb_surface, *test_tile_surface;
    SDL_Event event;

    print_unpacked(test_tiles_packed[0]);
    print_packed(test_tiles_unpacked[0]);
    print_packed(test_tiles_unpacked[1]);
    print_packed(test_tiles_unpacked[2]);

    GBState *state = initialize_gb(DEBUG);
    VideoTestState *vtstate = initialize_video_tests(state);

    /*
    SDL_Rect r;
    r.x = 0;
    r.y = 0;
    r.w = 8;
    r.h = 8;

    test_tile_surface = make_tile_surface(test_tiles_packed[0]);

    for (int i = 0; i < GB_HEIGHT_PX; i += 8) {
        for (int j = 0; j < GB_WIDTH_PX; j += 8) {
            r.x = j; 
            r.y = i;
            SDL_BlitSurface(test_tile_surface, NULL, vtstate->gb_surface, &r);
        }
    }
    */
    vtstate->gb_surface = run_test(vtstate);
    vtstate->gb_texture = SDL_CreateTextureFromSurface(vtstate->renderer, vtstate->gb_surface);
    
    SDL_RenderClear(vtstate->renderer);
    SDL_RenderCopy(vtstate->renderer, vtstate->gb_texture, NULL, NULL);
    SDL_RenderPresent(vtstate->renderer);
    while (1) {
        SDL_PollEvent(&event);
        if(event.type == SDL_QUIT)
            break;

        //r.x = rand() % 500;
        //r.y = rand() % 500;

        //SDL_SetRenderTarget(renderer, test_texture);
        //SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        //SDL_RenderClear(renderer);

        //SDL_RenderDrawRect(renderer,&r);
        //SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0x00);
        //SDL_RenderFillRect(renderer, &r);
        //SDL_SetRenderTarget(renderer, NULL);
    }
    
    teardown_video_tests(vtstate);
    teardown_gb(state);

    SDL_Quit();
 
    return 0;
}
