#include "base.h"
#include "mem.h"
#include "video.h"
#include <stdlib.h>

#include <SDL.h>
//#include <SDL_image.h>
//#include <SDL_timer.h>

int unpack_tile(const BYTE *data, BYTE *data_unpacked, BYTE flags) {

    int x_flip = 0, y_flip = 0, p_num = 0;
    y_flip = bit_6(flags);
    x_flip = bit_5(flags);

    p_num = bit_4(flags);

    BYTE tmp1, tmp2;
    char x_base, x_add, y_base, y_add, row_base;
    if (x_flip) {
        x_base = 7;
        x_add = -1;
    } else {
        x_base = 0;
        x_add = 1;
    }

    if (y_flip) {
        y_base = 7;
        y_add = -1;
    } else {
        y_base = 0;
        y_add = 1;
    }

    for (int i = 0; i < 8; i++) {
        tmp1 = data[2*i];
        tmp2 = data[2*i+1];
        row_base = 8*(y_base + y_add*i) + x_base;

        // This fills in one row per iteration of the 8x8 sprite
        // To flip Y, we start at the bottom of the 8x8 buffer
        // To flip X, we take the bits from right (lsb) to left (msb)
        // in each packed byte

        data_unpacked[row_base] = (bit_7(tmp1)>>7) | (bit_7(tmp2)>>6);
        data_unpacked[row_base + 1*x_add] = (bit_6(tmp1)>>6) | (bit_6(tmp2)>>5);
        data_unpacked[row_base + 2*x_add] = (bit_5(tmp1)>>5) | (bit_5(tmp2)>>4);
        data_unpacked[row_base + 3*x_add] = (bit_4(tmp1)>>4) | (bit_4(tmp2)>>3);
        data_unpacked[row_base + 4*x_add] = (bit_3(tmp1)>>3) | (bit_3(tmp2)>>2);
        data_unpacked[row_base + 5*x_add] = (bit_2(tmp1)>>2) | (bit_2(tmp2)>>1);
        data_unpacked[row_base + 6*x_add] = (bit_1(tmp1)>>1) | (bit_1(tmp2)>>0);
        data_unpacked[row_base + 7*x_add] = (bit_0(tmp1)>>0) | (bit_0(tmp2)<<1);
         
    }

    return 64;
}

BYTE *lookup_tile(BYTE ind, BYTE *mem) {
    WORD addr;
    addr = TILE_INDEX_TO_ADDR_8000(ind);

    return &mem[addr];
}

BYTE *lookup_obj_tile(SpriteAttr *attr, BYTE *mem) {
    return lookup_tile(attr->index, mem);
}

PPUState *initialize_ppu(void) {
    PPUState *ppu = malloc(sizeof(PPUState));
    if (ppu == NULL) {
        printf("Failed to allocate PPUState\n");
        return NULL;
    }
    ppu->state = INIT;
    ppu->fifo_bg.state = SLEEP;
    ppu->fifo_obj.state = SLEEP;

    for (int i = 0; i < 16; i++) {
        ppu->fifo_bg.data[i] = 0;
        ppu->fifo_obj.data[i] = 0;
    }

    return ppu;
}

void teardown_ppu(PPUState *ppu) {
    free(ppu);
}


void print_unpacked(const BYTE *packed) {
    BYTE *unpacked;
    unpacked = malloc(64 * sizeof(BYTE));
    if (unpacked == NULL) {
        printf("Error on allocating unpacked buffer");
        exit(1);
    }

    unpack_tile(packed, unpacked, 0x20);

    for (int i = 0; i < 64; i++) {
        if (i % 8 == 0) {
            printf("\n");
        }
        printf("%x ", unpacked[i]);
    }
    printf("\n");

    free(unpacked);
}

SDL_Surface *make_tile_surface(const BYTE *packed) {
    BYTE *unpacked;

    unpacked = malloc(64 * sizeof(BYTE));
    if (unpacked == NULL) {
        printf("Error on allocating unpacked buffer");
        exit(1);
    }

    unpack_tile(packed, unpacked, 0x40); 

    SDL_Surface *surface;
    surface = SDL_CreateRGBSurfaceFrom(
        unpacked,
        8, // width (px)
        8, // height (px)
        8, // bit depth (bits)
        8, // pitch (bytes per row)
        0, 0, 0, 0 // masks, not used for 8 bit depth
    );

    if (surface == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
            "Couldn't allocate surface: %s\n", SDL_GetError());
        
        return NULL;
    }

    if (SDL_SetPaletteColors(surface->format->palette, colors, 0, 4) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
            "Couldn't set palette colors: %s\n", SDL_GetError());
        
        return NULL;
    }

    return surface;
}

int main(int argc, char *argv[]) {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture, *test_texture;
    SDL_Surface *test_surface;
    SDL_Event event;
    SDL_Rect r;

    print_unpacked(test_tile);

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
            "Couldn't initialize SDL: %s", SDL_GetError());
        return 3;
    }

    window = SDL_CreateWindow("SDL_CreateTexture",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        512, 512,
        SDL_WINDOW_RESIZABLE
    );

    r.w = 10;
    r.h = 5;

    renderer = SDL_CreateRenderer(window, -1, 0);
    /*
    texture = SDL_CreateTexture(
        renderer, 
        SDL_PIXELFORMAT_RGBA8888, 
        SDL_TEXTUREACCESS_TARGET, 
        160, 
        144
    );
    */

    test_surface = make_tile_surface(test_tile);
    test_texture = SDL_CreateTextureFromSurface(renderer, test_surface);

    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0x00);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, test_texture, NULL, NULL);
    SDL_RenderPresent(renderer);

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

    free(test_surface->pixels);
    
    SDL_DestroyRenderer(renderer);
    SDL_Quit();
 
    return 0;
}