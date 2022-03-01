#include "base.h"
#include "mem.h"
#include "video.h"
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>

#include <SDL.h>
//#include <SDL_image.h>
//#include <SDL_timer.h>


const SDL_Color obj_colors[] = {
    {.r = 0xFF, .g = 0xFF, .b = 0xFF, .a = 0x00 },
    {.r = 0x90, .g = 0x90, .b = 0x90, .a = 0xFF },
    {.r = 0x50, .g = 0x50, .b = 0x50, .a = 0xFF },
    {.r = 0x00, .g = 0x00, .b = 0x00, .a = 0xFF }
};

const SDL_Color bgwin_colors[] = {
    {.r = 0xFF, .g = 0xFF, .b = 0xFF, .a = 0xFF },
    {.r = 0x90, .g = 0x90, .b = 0x90, .a = 0xFF },
    {.r = 0x50, .g = 0x50, .b = 0x50, .a = 0xFF },
    {.r = 0x00, .g = 0x00, .b = 0x00, .a = 0xFF }
};

const BYTE color_table[] = { 0xFF, 0x90, 0x50, 0x00 };


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

/* Unpacks an 8x8 or 8x16 tile starting at addr pointed to by data,
into target buffer data_unpacked, at the specified x and y offsets given
x_extent bytes per row in the target. Flags specify X or Y flip (as found
in OAM memory).
Tiles are stored in 2-bit-per pixel interleaved format (2 bytes per 8-pixel row,
16 bytes per 8x8 tile). The earlier byte gives the least significant bit
of each value in the row, the later byte the most significant bit.
 If the first two bytes are 0xB3 0x1C, the unpacked value will be:
0xB3 ==  1  |  0  |  1  |  1  |  0  |  0  |  1  |  1
0x1C ==  0  |  0  |  0  |  1  |  1  |  1  |  0  |  0
        --------------------------------------------
         01 | 00  | 01  | 11  | 10  | 10  | 01  | 01
    = 1 0 1 3 2 2 1 1

The unpacked values are then used to index into palette to convert them
to their final form. Bits 0-1 specify the color for index 0, 2-3 for index 1,
4-5 for index 2, and 6-7 for index 3. So palette of 10|11|00|11 would 
map 0 to 3, 1 to 0, 2 to 3, and 3 to 2.
*/
void unpack_tile(
    const BYTE *data, 
    BYTE *data_unpacked, 
    BYTE flags,
    BYTE palette,
    ObjectSize size,
    int x_offset,
    int y_offset,
    int x_extent
) {

    int x_flip = 0, y_flip = 0, p_num = 0;
    y_flip = bit_6(flags);
    x_flip = bit_5(flags);

    BYTE colors[4];
    colors[0] = palette & 0x3;
    colors[1] = (palette & 0xC) >> 2;
    colors[2] = (palette & 0x30) >> 4;
    colors[3] = (palette & 0xC0) >> 6;
    
    /* If 8x16, the second packed tile immediately after the first is
    added below the first (8 columns, 16 rows), so we can just extend
    our loop to continue through memory. 
    FIXME does Y flip for 8x16 flip entire 16 rows as one? Or does it
    flip each 8x8 constituent individually? Current implementation flips
    entire 16 rows. 
    */
    int n_rows = (size == OBJ_8x8) ? 8 : 16;

    BYTE tmp1, tmp2;
    int x_base, y_base, row_base;
    char x_add, y_add;
    if (x_flip) {
        x_base = x_offset + n_rows - 1;
        x_add = -1;
    } else {
        x_base = x_offset;
        x_add = 1;
    }

    if (y_flip) {
        y_base = y_offset + n_rows - 1;
        y_add = -1;
    } else {
        y_base = y_offset;
        y_add = 1;
    }

    for (int i = 0; i < n_rows; i++) {
        tmp1 = data[2*i];
        tmp2 = data[2*i+1];
        row_base = x_extent*(y_base + y_add*i) + x_base;

        // This fills in one row per iteration of the 8x8 sprite
        // To flip Y, we start at the bottom of the 8x8 buffer
        // To flip X, we take the bits from right (lsb) to left (msb)
        // in each packed byte

        data_unpacked[row_base] = colors[(bit_7(tmp1)>>7) | (bit_7(tmp2)>>6)];
        data_unpacked[row_base + 1*x_add] = colors[(bit_6(tmp1)>>6) | (bit_6(tmp2)>>5)];
        data_unpacked[row_base + 2*x_add] = colors[(bit_5(tmp1)>>5) | (bit_5(tmp2)>>4)];
        data_unpacked[row_base + 3*x_add] = colors[(bit_4(tmp1)>>4) | (bit_4(tmp2)>>3)];
        data_unpacked[row_base + 4*x_add] = colors[(bit_3(tmp1)>>3) | (bit_3(tmp2)>>2)];
        data_unpacked[row_base + 5*x_add] = colors[(bit_2(tmp1)>>2) | (bit_2(tmp2)>>1)];
        data_unpacked[row_base + 6*x_add] = colors[(bit_1(tmp1)>>1) | (bit_1(tmp2)>>0)];
        data_unpacked[row_base + 7*x_add] = colors[(bit_0(tmp1)>>0) | (bit_0(tmp2)<<1)];
         
    }
}

void reset_ppu_fifo(PPUFifo *fifo) {
    fifo->state = FETCH_TILE;
    fifo->counter = 0;
    for (int i = 0; i < 16; i++) {
        fifo->data[i].color = 0;
        fifo->data[i].palette = PALETTE_DEFAULT;
        fifo->data[i].sprite_prio = 0;
        fifo->data[i].bg_prio = 0;
    }
}

void reset_oam_scan(OAMScan_t *oam_scan) {
    oam_scan->counter = 0;
    oam_scan->n_sprites_total = 0;
    oam_scan->n_sprites_row = 0;

    OAMEntry *spr;

    for (int i = 0; i < 10; i++) {
        spr = &oam_scan->current_row_sprites[i];
        spr->x = 0;
        spr->y = 0;
        spr->flags = 0;
        spr->index = 0;
    }
}

PPUState *initialize_ppu(void) {
    PPUState *ppu = malloc(sizeof(PPUState));
    if (ppu == NULL) {
        printf("Failed to allocate PPUState\n");
        return NULL;
    }
    // 0x91: 1001 0001
    ppu->lcdc.lcd_enable = ON;
    ppu->lcdc.win_map_area = MAP_AREA0;
    ppu->lcdc.window_enable = OFF;
    ppu->lcdc.bg_win_data_area = DATA_AREA1;
    ppu->lcdc.bg_map_area = MAP_AREA0;
    ppu->lcdc.obj_size = OBJ_8x8;
    ppu->lcdc.obj_enable = OFF;
    ppu->lcdc.bg_window_enable = ON;

    // 0x85: 1000 0101
    ppu->stat.lyc_ly_interrupt = OFF;
    ppu->stat.mode_2_interrupt = OFF;
    ppu->stat.mode_1_interrupt = OFF;
    ppu->stat.mode_0_interrupt = OFF;
    ppu->stat.lyc_ly_equal = ON;
    ppu->stat.mode = VBLANK;

    ppu->frame.counter = PPU_PER_FRAME - COUNTER_VBLANK_LENGTH;
    ppu->frame.win_y = 0;
    ppu->frame.pixelbuf = malloc(GB_FULL_SIZE * GB_FULL_SIZE);
    if (ppu->frame.pixelbuf == NULL) {
        printf("Failed to allocate frame pixelbuf\n");
        exit(1);
    } 
    memset(ppu->frame.pixelbuf, 0, GB_FULL_SIZE * GB_FULL_SIZE);

    ppu->scanline.counter = 0;
    ppu->scanline.mode_counter = COUNTER_VBLANK_LENGTH;
    ppu->scanline.x_pos = 0;
    ppu->scanline.bg.buf = malloc(SCANLINE_PIXELBUF_SIZE);
    if (ppu->scanline.bg.buf == NULL) {
        printf("Failed to allocate scanline bg pixelbuf\n");
        exit(1);
    }
    memset(ppu->scanline.bg.buf, 0, SCANLINE_PIXELBUF_SIZE);
    ppu->scanline.bg.offset = 0;
    ppu->scanline.win.buf = malloc(SCANLINE_PIXELBUF_SIZE);
    if (ppu->scanline.win.buf == NULL) {
        printf("Failed to allocate scanline win pixelbuf\n");
        exit(1);
    }
    memset(ppu->scanline.win.buf, 0, SCANLINE_PIXELBUF_SIZE);
    ppu->scanline.win.offset = 0;
    ppu->scanline.obj.buf = malloc(SCANLINE_PIXELBUF_SIZE);
    if (ppu->scanline.obj.buf == NULL) {
        printf("Failed to allocate scanline obj pixelbuf\n");
        exit(1);
    }
    memset(ppu->scanline.obj.buf, 0, SCANLINE_PIXELBUF_SIZE);
    ppu->scanline.obj.offset = 0;

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
    
    reset_ppu_fifo(&ppu->draw.fifo_bg);
    reset_ppu_fifo(&ppu->draw.fifo_obj);
    reset_oam_scan(&ppu->oam_scan);

    /*ppu->win_pixelbuf = malloc(GB_FULL_SIZE * GB_FULL_SIZE);
    memset(ppu->win_pixelbuf, 0, GB_FULL_SIZE * GB_FULL_SIZE);
    ppu->obj_pixelbuf = malloc(GB_FULL_SIZE * GB_FULL_SIZE);
    memset(ppu->obj_pixelbuf, 0, GB_FULL_SIZE * GB_FULL_SIZE);
    */

    return ppu;
}

void teardown_ppu(PPUState *ppu) {
    free(ppu->frame.pixelbuf);
    free(ppu->scanline.bg.buf);
    free(ppu->scanline.win.buf);
    free(ppu->scanline.obj.buf);
    free(ppu);
}


void print_unpacked(const BYTE *packed) {
    BYTE *unpacked;
    unpacked = malloc(64 * sizeof(BYTE));
    if (unpacked == NULL) {
        printf("Error on allocating unpacked buffer");
        exit(1);
    }

    unpack_tile(packed, unpacked, 0, PALETTE_DEFAULT, OBJ_8x8, 0, 0, 8);

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
        printf("0x%02X, ", packed[i]);
    }

    free(packed);
}

void convert_8bit_colors(BYTE *pixels, BYTE palette, int nbytes) {
    BYTE colors[4];
    BYTE tmp;
    colors[0] = palette & 0x3;
    colors[1] = (palette & 0xC) >> 2;
    colors[2] = (palette & 0x30) >> 4;
    colors[3] = (palette & 0xC0) >> 6;

    for (int i = 0; i < nbytes; i++) {
        tmp = pixels[i];
        pixels[i] = colors[tmp];
    }
}

void set_8bit_colors(SDL_Surface *surface, BYTE color_source) {
    if (color_source != COLORS_NONE) {
        SDL_Color *colors;
        SDL_BlendMode blend_mode;
        if (color_source == COLORS_BGWIN) {
            colors = &bgwin_colors;
            blend_mode = SDL_BLENDMODE_NONE;
        } else {
            colors = &obj_colors;
            blend_mode = SDL_BLENDMODE_NONE;
        }

        SDL_SetSurfaceBlendMode(surface, blend_mode);
        SDL_SetPaletteColors(surface->format->palette, colors, 0, 4);
    }
}

SDL_Surface *new_8bit_surface(
    int width, int height, 
    BYTE color_source) {
    SDL_Surface *surface = SDL_CreateRGBSurface(
        0, width, height, 
        8, 0, 0, 0, 0);
    
    set_8bit_colors(surface, color_source);
    return surface;
}

SDL_Surface *new_8bit_surface_from(
    BYTE *pixels, int width, int height, 
    BYTE color_source) {
    SDL_Surface *surface = SDL_CreateRGBSurfaceFrom(
        pixels,
        width,
        height,
        8,
        width,
        0, 0, 0, 0
    );

    set_8bit_colors(surface, color_source);
    return surface;
}

void ppu_render_scanline(GBState *state) {
    PPUState *ppu = state->ppu;
    SDL_Rect r;
    BYTE color;
    //SDL_RenderGetViewport(state->gb_renderer, &r);

    r.y = ppu->misc.ly;
    r.x = 0;//ppu->scanline.x_pos;
    r.w = 1; 
    r.h = 1;
    int start_ind = ppu->scanline.bg.offset;

    for (; r.x < GB_WIDTH_PX; r.x++) {
        color = color_table[ppu->scanline.bg.buf[r.x + start_ind]];
        SDL_SetRenderDrawColor(state->gb_renderer, color, color, color, 0xFF);
        SDL_RenderFillRect(state->gb_renderer, &r);
    }
   
}


void ppu_oamscan(GBState *state) {
    PPUState *ppu = state->ppu;
    LCDControl lcdc = ppu->lcdc;
    LCDStatus stat = ppu->stat;
    PPUMisc misc = ppu->misc;
    OAMScan_t *oam_scan = &ppu->oam_scan;
    BYTE ly;
    int row_overlap;
    ly = misc.ly;
    OAMEntry *oam_table_ptr;
    OAMEntry current_entry;

    if ((oam_scan->counter & 1)
        ||lcdc.obj_enable == OFF 
        || oam_scan->n_sprites_row == SPRITES_ROW_MAX
        || oam_scan->n_sprites_total == SPRITES_TOTAL_MAX
    ) goto ppu_oamscan_end;

    oam_table_ptr = ppu_get_mem_pointer(state, OAM_BASE);
    current_entry = oam_table_ptr[oam_scan->counter];

    row_overlap = (int)ly - ((int)current_entry.y - 16);

    if (row_overlap > 0 && row_overlap < (int)lcdc.obj_size) {
        oam_scan->n_sprites_row++;
        oam_scan->n_sprites_total++;
    }

    ppu_oamscan_end:
    oam_scan->counter++;
}

void ppu_do_mode_switch(GBState *state) {
    PPUState *ppu = state->ppu;
    PPUMode next_mode;
    unsigned int next_mode_counter;

    switch (ppu->stat.mode) {
        case OAMSCAN:
            next_mode_counter = 172; // at minimum
            next_mode = DRAW;
            reset_ppu_fifo(&ppu->draw.fifo_bg);
            reset_ppu_fifo(&ppu->draw.fifo_obj);
            break;
        case DRAW:
            next_mode_counter = PPU_PER_SCANLINE - ppu->scanline.counter;
            next_mode = HBLANK;
            
            break;
        case HBLANK:
            if (ppu->misc.ly < 144) {
                next_mode = OAMSCAN;
                next_mode_counter = 80;
            } else {
                next_mode = VBLANK;
                next_mode_counter = 4560;
            }

            break;
        case VBLANK:
            next_mode_counter = 80;
            next_mode = OAMSCAN;
            break;
    }
    ppu->scanline.mode_counter = next_mode_counter;
    ppu->stat.mode = next_mode;
}

/* Returns the address (in the tilemap at base) at which the tile index 
 for the current tile can be found. "Current tile" means the tile that needs to be
 rendered given x scroll scx (pixels), y scroll scy (pixels), cur_x X offset in row (pixels,
 0-160), ly current scanline (pixels, 0-144)
 */
WORD get_tilemap_addr(WORD base, BYTE scx, BYTE scy, BYTE cur_x, BYTE ly) {
    BYTE tile_x, tile_y;
    tile_x = compute_tile_x(scx, cur_x);
    tile_y = compute_tile_y(scy, ly);

    return compute_tilemap_addr(base, tile_x, tile_y);
}

void unpack_row(
    const BYTE lsb, 
    const BYTE msb, 
    BYTE *dest,
    const BYTE flags,
    const BYTE palette
) {
    BYTE x_flip;
    x_flip = flags & TILE_X_FLIP;

    BYTE colors[4];
    colors[0] = palette & 0x3;
    colors[1] = (palette & 0xC) >> 2;
    colors[2] = (palette & 0x30) >> 4;
    colors[3] = (palette & 0xC0) >> 6;
    
    if (x_flip) {
        dest[7] = (bit_7(lsb) >> 7) | (bit_7(msb) >> 6);
        dest[6] = (bit_6(lsb) >> 6) | (bit_6(msb) >> 5);
        dest[5] = (bit_5(lsb) >> 5) | (bit_5(msb) >> 4);
        dest[4] = (bit_4(lsb) >> 4) | (bit_4(msb) >> 3);
        dest[3] = (bit_3(lsb) >> 3) | (bit_3(msb) >> 2);
        dest[2] = (bit_2(lsb) >> 2) | (bit_2(msb) >> 1);
        dest[1] = (bit_1(lsb) >> 1) | (bit_1(msb) >> 0);
        dest[0] = (bit_0(lsb) >> 0) | (bit_0(msb) << 1);
    } else {
        dest[0] = (bit_7(lsb) >> 7) | (bit_7(msb) >> 6);
        dest[1] = (bit_6(lsb) >> 6) | (bit_6(msb) >> 5);
        dest[2] = (bit_5(lsb) >> 5) | (bit_5(msb) >> 4);
        dest[3] = (bit_4(lsb) >> 4) | (bit_4(msb) >> 3);
        dest[4] = (bit_3(lsb) >> 3) | (bit_3(msb) >> 2);
        dest[5] = (bit_2(lsb) >> 2) | (bit_2(msb) >> 1);
        dest[6] = (bit_1(lsb) >> 1) | (bit_1(msb) >> 0);
        dest[7] = (bit_0(lsb) >> 0) | (bit_0(msb) << 1);
    }
}

/* Determine, given the whole current state, what row of 8 pixels from the 
background tilemap to render (based on current scanline and scy), fetch these 
two bytes and unpack them into the scanline pixelbuf, shifted by the scx value.
This involves 3 memory reads: 1 to read the tilemap to determine the index of the 
current tile, then two to read the two bytes of the row from the computed tile data
address. The tile map and tile data area used are taken from the lcdc register, and 

In the final implementation the three memory reads should each take 1 dot. The state must
describe what the next step to do is, and store the incremental information like the
current tile index to read, then the tile data address. The data needs to be pushed to a
FIFO then to be maximally accurate.
*/
void get_current_tile_row(GBState *state) {
    PPUState *ppu = state->ppu;
    LCDControl lcdc = ppu->lcdc;
    LCDStatus stat = ppu->stat;
    PPUMisc misc = ppu->misc;
    Drawing_t draw = ppu->draw;
    Scanline_t *scanline = &ppu->scanline;

    WORD map_area, data_area, map_addr, tile_addr;
    BYTE tile_index, x_pixel_offset, y_pixel_offset;
    BYTE lsb, msb;
    lsb = msb = 0;

    map_area = (WORD)lcdc.bg_map_area;
    data_area = (WORD)lcdc.bg_win_data_area;

    map_addr = get_tilemap_addr(map_area, misc.scx, misc.scy, scanline->x_pos, misc.ly);

    tile_index = ppu_read_mem(state, map_addr);
    y_pixel_offset = compute_y_pixel_offset(misc.scy, misc.ly);
        
    // Gets us to the top left corner of the tile we want
    tile_addr = compute_tiledata_addr(data_area, tile_index);
    
    // Add 2 for each row down in the tile we are (2 bytes per row)
    tile_addr += (TILE_BYTES_PER_ROW * y_pixel_offset);

    lsb = ppu_read_mem(state, tile_addr);
    msb = ppu_read_mem(state, tile_addr+1);

    unpack_row(lsb, msb, &scanline->bg.buf[scanline->x_pos], 0, PALETTE_DEFAULT);
}

void ppu_draw_cycle(GBState *state) {
    PPUState *ppu = state->ppu;
    get_current_tile_row(state);
    ppu->scanline.x_pos += 8;

    if (ppu->scanline.x_pos == GB_WIDTH_PX) {
        ppu->scanline.mode_counter = 1;
        //for (int i = 0; i < GB_WIDTH_PX; i++) {
        //    printf("%02x ", ppu->scanline.pixelbuf[i]);
        //}
        //printf("\n");
    }
}

void ppu_next_scanline(PPUState *ppu) {
    ppu->scanline.counter = 0;
    memset(ppu->scanline.bg.buf, 0, SCANLINE_PIXELBUF_SIZE);
    ppu->scanline.bg.offset = ppu->misc.scx & 0x7;
    memset(ppu->scanline.win.buf, 0, SCANLINE_PIXELBUF_SIZE);
    ppu->scanline.win.offset = ppu->misc.wx & 0x7;
    memset(ppu->scanline.obj.buf, 0, SCANLINE_PIXELBUF_SIZE);
    ppu->scanline.x_pos = 0;
    ppu->misc.ly++;
}

void ppu_next_frame(PPUState *ppu) {
    ppu->frame.counter = 0;
    ppu->frame.win_y = 0;
    ppu->misc.ly = 0;
}

void task_ppu_cycle(GBState *state) {
    PPUState *ppu = state->ppu;
    LCDControl lcdc = ppu->lcdc;
    LCDStatus stat = ppu->stat;
    PPUMisc misc = ppu->misc;

    if (lcdc.lcd_enable == OFF)
        goto ppu_cycle_end;
    
    switch (stat.mode) {
        case HBLANK:
        case VBLANK:
            /* Do nothing */
            break;
        case OAMSCAN:

            break;
        case DRAW:
            ppu_draw_cycle(state);
            break;
    }

    ppu_cycle_end:
    ppu->scanline.counter++;
    ppu->frame.counter++;

    ppu->scanline.mode_counter--;
    if (ppu->scanline.mode_counter == 0)
        ppu_do_mode_switch(state);

    if (ppu->scanline.counter == PPU_PER_SCANLINE) {
        ppu_render_scanline(state);
        ppu_next_scanline(ppu);
    }

    if (ppu->frame.counter == PPU_PER_FRAME) {
        SDL_UpdateWindowSurface(state->gb_window);
        ppu_next_frame(ppu);
        /*ppu->frame.counter = 0;
        ppu->misc.ly = 0;*/
    }
}

