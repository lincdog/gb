#include "base.h"
#include "mem.h"
#include "video.h"
#include <stdlib.h>

#include <SDL.h>
//#include <SDL_image.h>
//#include <SDL_timer.h>



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

int unpack_tile(
    const BYTE *data, 
    BYTE *data_unpacked, 
    BYTE flags,
    int x_offset,
    int y_offset,
    int x_extent
) {

    int x_flip = 0, y_flip = 0, p_num = 0;
    y_flip = bit_6(flags);
    x_flip = bit_5(flags);

    p_num = bit_4(flags);

    BYTE tmp1, tmp2;
    int x_base, y_base, row_base;
    char x_add, y_add;
    if (x_flip) {
        x_base = x_offset + 7;
        x_add = -1;
    } else {
        x_base = x_offset + 0;
        x_add = 1;
    }

    if (y_flip) {
        y_base = y_offset + 7;
        y_add = -1;
    } else {
        y_base = y_offset + 0;
        y_add = 1;
    }

    for (int i = 0; i < 8; i++) {
        tmp1 = data[2*i];
        tmp2 = data[2*i+1];
        row_base = x_extent*(y_base + y_add*i) + x_base;

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
    ppu->lcdc.win_map_area = MAP_AREA0;
    ppu->lcdc.window_enable = OFF;
    ppu->lcdc.bg_win_data_area = DATA_AREA1;
    ppu->lcdc.bg_map_area = MAP_AREA0;
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

    ppu->bg_pixelbuf = malloc(GB_FULL_SIZE * GB_FULL_SIZE);
    memset(ppu->bg_pixelbuf, 0, GB_FULL_SIZE * GB_FULL_SIZE);
    ppu->win_pixelbuf = malloc(GB_FULL_SIZE * GB_FULL_SIZE);
    memset(ppu->win_pixelbuf, 0, GB_FULL_SIZE * GB_FULL_SIZE);
    ppu->obj_pixelbuf = malloc((GB_HEIGHT_PX+16)*(GB_WIDTH_PX+8));
    memset(ppu->obj_pixelbuf, 0, (GB_HEIGHT_PX+16)*(GB_WIDTH_PX+8));

    return ppu;
}

void teardown_ppu(PPUState *ppu) {
    free(ppu->bg_pixelbuf);
    free(ppu->win_pixelbuf);
    free(ppu);
}


void print_unpacked(const BYTE *packed) {
    BYTE *unpacked;
    unpacked = malloc(64 * sizeof(BYTE));
    if (unpacked == NULL) {
        printf("Error on allocating unpacked buffer");
        exit(1);
    }

    unpack_tile(packed, unpacked, 0, 0, 0, 8);

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

void set_8bit_colors(SDL_Surface *surface, BYTE palette, BYTE color_source) {
    if (color_source != COLORS_NONE) {
        SDL_Color colors[4];
        if (color_source == COLORS_BGWIN) {
            colors[0] = bgwin_colors[palette & 0x3];
            colors[1] = bgwin_colors[(palette & 0xC)>>2];
            colors[2] = bgwin_colors[(palette & 0x30)>>4];
            colors[3] = bgwin_colors[(palette & 0xC0)>>6];
        } else {
            colors[0] = obj_colors[palette & 0x3];
            colors[1] = obj_colors[(palette & 0xC)>>2];
            colors[2] = obj_colors[(palette & 0x30)>>4];
            colors[3] = obj_colors[(palette & 0xC0)>>6];
        }
    
        SDL_SetPaletteColors(surface->format->palette, colors, 0, 4);
    }
}

SDL_Surface *new_8bit_surface(
    int width, int height, 
    BYTE palette, BYTE color_source) {
    SDL_Surface *surface = SDL_CreateRGBSurface(
        0, width, height, 
        8, 0, 0, 0, 0);
    
    set_8bit_colors(surface, palette, color_source);
    return surface;
}

SDL_Surface *new_8bit_surface_from(
    BYTE *pixels, int width, int height, 
    BYTE palette, BYTE color_source) {
    SDL_Surface *surface = SDL_CreateRGBSurfaceFrom(
        pixels,
        width,
        height,
        8,
        width,
        0, 0, 0, 0
    );

    set_8bit_colors(surface, palette, color_source);
    return surface;
}

/* Fills in pixel buffer pixels (assumed to be 256x256) with values from
tilemap starting at map_area_base (0x9700==AREA0 or 0x9C00==AREA1) using
tile data at 0x8000-0x9000 (AREA1) or 0x8800-0x9800 (AREA0)
 */
void fill_tilemap_pixels(
    GBState *state, 
    BYTE *pixels, 
    TileMapArea map_area_base,
    TileDataArea data_area_base) {

    WORD map_addr, tile_addr;
    BYTE tile_index, tile_x, tile_y, sign_check;
    int16_t tile_addr_offset;
    BYTE *tile_mem_pointer;
    tile_x = tile_y = 0; 
    sign_check = (data_area_base == DATA_AREA1) ? 0 : 0x80;

    for (int i = 0; i < TILEMAP_SIZE_BYTES; i++) {
        map_addr = map_area_base + i;
        tile_index = read_mem(state, map_addr, 0);
        /* tile_index ^ sign_check = tile_index if sign_check is 0,
        or last 7 bits of tile_index if sign_check is 0x80 (0b1000000).
        tile_index & sign_check = 0 if sign_check is 0, 
        or 0x80 (0x10000000) if sign_check is 0x80 and tile_index < 0.
        */
        tile_addr_offset = TILE_SIZE_BYTES * 
            ((tile_index ^ sign_check) - (tile_index & sign_check));
        
        tile_addr = data_area_base + tile_addr_offset;
                    
        tile_mem_pointer = get_mem_pointer(state, tile_addr, 0);
        tile_x = 8*(i % 32);
        tile_y = 8*(i >> 5);
        unpack_tile(
            tile_mem_pointer, 
            pixels, 
            0, tile_x, tile_y, GB_FULL_SIZE
        );
    }
}

SDL_Surface *ppu_make_surface(GBState *state) {
    /*TODO: Given the current PPU state and an SDL Renderer, 
    * produce the entire 160x144 video image and render it.
    * A prelude to breaking this process into cycle-based timing
    * that respects the various PPU modes, pixel FIFOs, and memory
    * access restrictions.
    */
    PPUState *ppu = state->ppu;
    LCDStatus stat = ppu->stat;
    LCDControl lcdc = ppu->lcdc;
    SDL_Rect bg_r, win_r;
    SDL_Surface *tile_surface, *bg_surface, *win_surface, *obj_surface;
    
    if (lcdc.lcd_enable == OFF)
        goto cleanup_end;

    if (lcdc.bg_window_enable == ON) {
        
        bg_r.x = ppu->misc.scx;
        bg_r.y = ppu->misc.scy;
        bg_r.w = GB_WIDTH_PX;
        bg_r.h = GB_HEIGHT_PX;
        
        fill_tilemap_pixels(
            state, ppu->bg_pixelbuf,
            lcdc.bg_map_area, lcdc.bg_win_data_area
        );
        
        bg_surface = new_8bit_surface_from(
            ppu->bg_pixelbuf,
            GB_FULL_SIZE, GB_FULL_SIZE, 
            ppu->misc.bgp, COLORS_BGWIN
        );
        
        SDL_BlitSurface(bg_surface, &bg_r, ppu->gb_surface, NULL);
        SDL_FreeSurface(bg_surface);

        if (lcdc.window_enable == ON) {
            
            win_r.x = ppu->misc.wx - 7;
            win_r.y = ppu->misc.wy;
            win_r.w = GB_WIDTH_PX;
            win_r.h = GB_HEIGHT_PX;

            fill_tilemap_pixels(
                state, ppu->win_pixelbuf,
                lcdc.win_map_area, lcdc.bg_win_data_area
            );
            win_surface = new_8bit_surface_from(
                ppu->win_pixelbuf,
                GB_FULL_SIZE, GB_FULL_SIZE, 
                ppu->misc.bgp, COLORS_BGWIN
            );

            SDL_BlitSurface(win_surface, &win_r, ppu->gb_surface, NULL);
            SDL_FreeSurface(win_surface);
        }
        
    }
    if (lcdc.obj_enable == ON) {
        obj_surface = new_8bit_surface(
            GB_FULL_SIZE, GB_FULL_SIZE, 
            PALETTE_DEFAULT, COLORS_OBJ);
        /* 
        - OAM scan: 0xFE00-0xFE9F
        - Per horizontal scanline, the first 10 objects found during the scan are displayed 
            on that line (even if they are off screen due to X coord).
        - 

        */

        SDL_FreeSurface(obj_surface);
    }

    cleanup_end:
    return ppu->gb_surface;

}

void ppu_render_surface(GBState *state) {
    PPUState *ppu = state->ppu;
    ppu->gb_texture = SDL_CreateTextureFromSurface(ppu->gb_renderer, ppu->gb_surface);
    
    SDL_RenderClear(ppu->gb_renderer);
    SDL_RenderCopy(ppu->gb_renderer, ppu->gb_texture, NULL, NULL);
    SDL_RenderPresent(ppu->gb_renderer);
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

