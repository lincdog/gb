#include "base.h"
#include "mem.h"
#include "video.h"
#include <stdlib.h>

#include <SDL.h>
//#include <SDL_image.h>
//#include <SDL_timer.h>


const BYTE test_tile[] = {
    0xFF, 0x00, 0x7E, 0xFF, 
    0x85, 0x81, 0x89, 0x83,
    0x93, 0x85, 0xA5, 0x8B, 
    0xC9, 0x97, 0x7E, 0xFF
};

const BYTE test_tile_unpacked[] = {
    1, 1, 1, 1, 1, 1, 1, 1,
    2, 3, 3, 3, 3, 3, 3, 2,
    3, 0, 0, 0, 0, 1, 0, 3,
    3, 0, 0, 0, 1, 0, 2, 3,
    3, 0, 0, 1, 0, 2, 1, 3,
    3, 0, 1, 0, 2, 1, 2, 3,
    3, 1, 0, 2, 1, 2, 2, 3,
    2, 3, 3, 3, 3, 3, 3, 2
};

const SDL_Color obj_colors[] = {
    {.r = 0xFF, .g = 0xFF, .b = 0xFF, .a = 0xFF },
    {.r = 0x90, .g = 0x90, .b = 0x90, .a = 0x00 },
    {.r = 0x50, .g = 0x50, .b = 0x50, .a = 0x00 },
    {.r = 0x00, .g = 0x00, .b = 0x00, .a = 0x00 }
};

const SDL_Color bgwin_colors[] = {
    {.r = 0xFF, .g = 0xFF, .b = 0xFF, .a = 0x00 },
    {.r = 0x90, .g = 0x90, .b = 0x90, .a = 0x00 },
    {.r = 0x50, .g = 0x50, .b = 0x50, .a = 0x00 },
    {.r = 0x00, .g = 0x00, .b = 0x00, .a = 0x00 }
};

int pack_tile(const BYTE *data_unpacked, BYTE *data, BYTE flags) {
    BYTE packed_byte_1 = 0;
    BYTE packed_byte_2 = 0;

    for (int i = 0, j = 0; i < 64; i += 8, j += 2) {
        printf("i: %d j: %d\n", i, j);
        packed_byte_1 = ((data_unpacked[i] & 0x1) << 7) |
                        ((data_unpacked[i+1] & 0x1) << 6) |
                        ((data_unpacked[i+2] & 0x1) << 5) |
                        ((data_unpacked[i+3] & 0x1) << 4) |
                        ((data_unpacked[i+4] & 0x1) << 3) |
                        ((data_unpacked[i+5] & 0x1) << 2) |
                        ((data_unpacked[i+6] & 0x1) << 1) |
                        ((data_unpacked[i+7] & 0x1) << 0);
        packed_byte_2 = ((data_unpacked[i] & 0x2) << 6) |
                        ((data_unpacked[i+1] & 0x2) << 5) |
                        ((data_unpacked[i+2] & 0x2) << 4) |
                        ((data_unpacked[i+3] & 0x2) << 3) |
                        ((data_unpacked[i+4] & 0x2) << 2) |
                        ((data_unpacked[i+5] & 0x2) << 1) |
                        ((data_unpacked[i+6] & 0x2) << 0) |
                        ((data_unpacked[i+7] & 0x2) >> 1);
        data[j] = packed_byte_1;
        data[j+1] = packed_byte_2;
    }
}

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
    ppu->counter = 0;
    // 0x91: 1001 0001
    ppu->lcdc.lcd_enable = ON;
    ppu->lcdc.win_map_area = AREA0;
    ppu->lcdc.window_enable = OFF;
    ppu->lcdc.bg_win_data_area = AREA1;
    ppu->lcdc.bg_map_area = AREA0;
    ppu->lcdc.obj_size = _8x8;
    ppu->lcdc.obj_enable = OFF;
    ppu->lcdc.bg_window_enable = ON;

    // 0x85: 1000 0101
    ppu->stat.lyc_ly_interrupt = OFF;
    ppu->stat.mode_2_interrupt = OFF;
    ppu->stat.mode_1_interrupt = OFF;
    ppu->stat.mode_0_interrupt = OFF;
    ppu->stat.lyc_ly_equal = ON;
    ppu->stat.mode = VBLANK;

    ppu->misc.scy = 0x00;
    ppu->misc.scx = 0x00;
    ppu->misc.ly = 0x00;
    ppu->misc.lyc = 0x00;
    ppu->misc.wy = 0x00;
    ppu->misc.wx = 0x00;
    // 11 11 11 00 
    ppu->misc.bgp = 0xFC;
    ppu->misc.obp0 = UNINIT;
    ppu->misc.obp1 = UNINIT;
    
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

    unpack_tile(packed, unpacked, 0);

    for (int i = 0; i < 64; i++) {
        if (i % 8 == 0) {
            printf("\n");
        }
        printf("%x ", unpacked[i]);
    }
    printf("\n");

    free(unpacked);
}

void print_packed(const BYTE *unpacked) {
    BYTE *packed;
    packed = malloc(16 * sizeof(BYTE));
    if (packed == NULL) {
        printf("Error on allocating packed buffer");
        exit(1);
    }

    pack_tile(unpacked, packed, 0);

    for (int i = 0; i < 16; i++) {
        if (i % 4 == 0)
            printf("\n");
        printf("%02X ", packed[i]);
    }
    printf("\n");

    free(packed);
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

    if (SDL_SetPaletteColors(surface->format->palette, obj_colors, 0, 4) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
            "Couldn't set palette colors: %s\n", SDL_GetError());
        
        return NULL;
    }

    return surface;
}

SDL_Surface *make_tile_surface_from_index(GBState *state, BYTE index) {

}

SDL_Surface *ppu_render_picture(GBState *state, SDL_Renderer *renderer) {
    /*TODO: Given the current PPU state and an SDL Renderer, 
    * produce the entire 160x144 video image and render it.
    * A prelude to breaking this process into cycle-based timing
    * that respects the various PPU modes, pixel FIFOs, and memory
    * access restrictions.
    */
    PPUState *ppu = state->ppu;
    LCDStatus stat = ppu->stat;
    LCDControl lcdc = ppu->lcdc;
    SDL_Rect r;
    r.x = 0;
    r.y = 0;
    r.h = 8;
    r.w = 8;

    SDL_Surface *tile_surface, *bg_surface, *win_surface, *obj_surface, *final_surface;
    
    final_surface = SDL_CreateRGBSurface(
        0, GB_WIDTH_PX, GB_HEIGHT_PX, 
        8, 0, 0, 0, 0);
    
    if (lcdc.lcd_enable == OFF)
        goto cleanup_end;
    
    WORD bg_win_tile_base, bg_map_base, win_map_base;
    WORD map_addr, tile_addr;
    char signed_offset;
    BYTE unsigned_offset;
    BYTE *tile_pointer;

    if (lcdc.bg_window_enable == ON) {
        bg_surface = SDL_CreateRGBSurface(
            0, GB_FULL_SIZE, GB_FULL_SIZE, 
            8, 0, 0, 0, 0);
        
        if (lcdc.bg_map_area == AREA1)
            bg_map_base = 0x9C00;
        else
            bg_map_base = 0x9800;
        
        if (lcdc.bg_win_data_area == AREA1) {
            bg_win_tile_base = 0x8000;
            for (int i = 0; i < 0x400; i++) {
                map_addr = bg_map_base + i;
                unsigned_offset = read_mem(state, map_addr, 0);
                tile_addr = bg_win_tile_base + (TILE_SIZE_BYTES * unsigned_offset);
                tile_pointer = get_mem_pointer(state, tile_addr, 0);
                tile_surface = make_tile_surface(tile_pointer);
                r.x = (i & 0x3F) << 3;
                r.y =  (i >> 5) << 3;
                SDL_BlitSurface(tile_surface, NULL, bg_surface, &r);
            }
        } else {
            bg_win_tile_base = 0x9000;
            for (int i = 0; i < 0x400; i++) {
                map_addr = bg_map_base + i;
                signed_offset = read_mem(state, map_addr, 0);
                tile_addr = bg_win_tile_base + (TILE_SIZE_BYTES * signed_offset);
                tile_surface = make_tile_surface(tile_pointer);
                r.x = (i & 0x3F) << 3;
                r.y =  (i >> 5) << 3;
                SDL_BlitSurface(tile_surface, NULL, bg_surface, &r);
            }
        }
        
        

        if (lcdc.window_enable == ON) {
            win_surface = SDL_CreateRGBSurface(
                0, GB_FULL_SIZE, GB_FULL_SIZE, 
                8, 0, 0, 0, 0);

            if (lcdc.win_map_area == AREA1)
                win_map_base = 0x9C00;
            else
                win_map_base = 0x9800;


            SDL_FreeSurface(win_surface);
        }
        SDL_FreeSurface(bg_surface);
    }
    if (lcdc.obj_enable == ON) {
        obj_surface = SDL_CreateRGBSurface(
            0, GB_FULL_SIZE, GB_FULL_SIZE, 
            8, 0, 0, 0, 0);


        SDL_FreeSurface(obj_surface);
    }

    cleanup_end:
    return final_surface;

}


void task_ppu_cycle(GBState *state) {
    PPUState *ppu = state->ppu;

    if (ppu->lcdc.lcd_enable == OFF)
        goto ppu_cycle_end;
    
    if (ppu->counter <= 80)
    switch (ppu->stat.mode) {
        case HBLANK:
            break;
        case VBLANK:
            break;
        case OAMSCAN:
            break;
        case DRAW:
            break;
    }

    ppu_cycle_end:
    1;
}

