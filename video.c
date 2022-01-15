#include "base.h"
#include <stdlib.h>

#include <SDL.h>
//#include <SDL_image.h>
//#include <SDL_timer.h>

int unpack_tile(BYTE *data, BYTE *data_unpacked) {

    BYTE tmp1, tmp2;

    for (int i = 0; i < 16; i += 2) {
        tmp1 = data[i];
        tmp2 = data[i+1];

        data_unpacked[4*i + 0] = (bit_7(tmp1)>>7) | (bit_7(tmp2)>>6);
        data_unpacked[4*i + 1] = (bit_6(tmp1)>>6) | (bit_6(tmp2)>>5);
        data_unpacked[4*i + 2] = (bit_5(tmp1)>>5) | (bit_5(tmp2)>>4);
        data_unpacked[4*i + 3] = (bit_4(tmp1)>>4) | (bit_4(tmp2)>>3);
        data_unpacked[4*i + 4] = (bit_3(tmp1)>>3) | (bit_3(tmp2)>>2);
        data_unpacked[4*i + 5] = (bit_2(tmp1)>>2) | (bit_2(tmp2)>>1);
        data_unpacked[4*i + 6] = (bit_1(tmp1)>>1) | (bit_1(tmp2)>>0);
        data_unpacked[4*i + 7] = (bit_0(tmp1)>>0) | (bit_0(tmp2)<<1);
         
    }

    return 64;
}

int main(int argc, char *argv[]) {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    SDL_Palette *palette;
    SDL_Event event;
    SDL_Rect r;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
            "Couldn't initialize SDL: %s", SDL_GetError());
        return 3;
    }

    window = SDL_CreateWindow("SDL_CreateTexture",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        160, 144,
        SDL_WINDOW_RESIZABLE
    );

    r.w = 10;
    r.h = 5;

    renderer = SDL_CreateRenderer(window, -1, 0);

    texture = SDL_CreateTexture(
        renderer, 
        SDL_PIXELFORMAT_RGBA8888, 
        SDL_TEXTUREACCESS_TARGET, 
        160, 
        144
    );

    palette = SDL_AllocPalette(4);
    const SDL_Color colors[4] = {
        {.r = 255, .g = 255, .b = 255, .a = 255 },
        {.r = 255, .g = 255, .b = 255, .a = 0   },
        {.r = 100, .g = 100, .b = 100, .a = 0   },
        {.r = 0,   .g = 0,   .b = 0,   .a = 0   }
    };
    if (SDL_SetPaletteColors(palette, colors, 0, 4) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
            "Couldn't set palette colors: %s\n", SDL_GetError());
        
        return 4;
    }

    while (1) {
        SDL_PollEvent(&event);
        if(event.type == SDL_QUIT)
            break;

        r.x = rand() % 500;
        r.y = rand() % 500;

        SDL_SetRenderTarget(renderer, texture);
        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
        //SDL_RenderClear(renderer);

        SDL_RenderDrawRect(renderer,&r);
        SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0x00);
        SDL_RenderFillRect(renderer, &r);
        SDL_SetRenderTarget(renderer, NULL);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_FreePalette(palette);
    SDL_Quit();
 
    return 0;
}