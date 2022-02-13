#include "base.h"
#include "mem.h"
#include "video.h"
#include "video.c"
#include <stdlib.h>
#include <SDL.h>

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

    SDL_SetPaletteColors(vtstate->gb_surface->format->palette, obj_colors, 0, 4);
    vtstate->gb_texture = NULL;

    return vtstate;
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
    /*SDL_Window *window;
    SDL_Renderer *renderer;*/
    SDL_Texture *texture, *test_texture;
    SDL_Surface *gb_surface, *test_tile_surface;
    SDL_Event event;

    print_unpacked(test_tile);
    print_packed(test_tile_unpacked);

    GBState *state = initialize_gb(DEBUG);
    VideoTestState *vtstate = initialize_video_tests(state);

    SDL_Rect r;
    r.x = 0;
    r.y = 0;
    r.w = 8;
    r.h = 8;
    SDL_SetPaletteColors(vtstate->gb_surface->format->palette, obj_colors, 0, 4);

    test_tile_surface = make_tile_surface(test_tile);

    for (int i = 0; i < GB_HEIGHT_PX; i += 8) {
        for (int j = 0; j < GB_WIDTH_PX; j += 8) {
            r.x = j; 
            r.y = i;
            SDL_BlitSurface(test_tile_surface, NULL, vtstate->gb_surface, &r);
        }
    }

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
