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

void reset_oamscan(OAMScan_t *oamscan) {
    oamscan->counter = 0;
    oamscan->n_sprites_total = 0;
    oamscan->n_sprites_row = 0;

    OAMRow_t *row;

    for (int i = 0; i < 10; i++) {
        row = &oamscan->current_row_sprites[i];
        row->entry_addr = 0;
        row->lsb = UNINIT;
        row->msb = UNINIT;
        row->oam.flags = 0;
        row->oam.index = 0;
        row->oam.x = 0;
        row->oam.y = 0;
    }
}

int _ppu_dummy(void) {
    return 1;
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

    ppu->frame.counter = COUNTER_VBLANK_LENGTH;
    ppu->frame.win_y = 0;
    ppu->frame.pixelbuf = malloc(GB_FULL_SIZE * GB_FULL_SIZE);
    if (ppu->frame.pixelbuf == NULL) {
        printf("Failed to allocate frame pixelbuf\n");
        exit(1);
    } 
    memset(ppu->frame.pixelbuf, 0, GB_FULL_SIZE * GB_FULL_SIZE);

    ppu->scanline.counter = PPU_PER_SCANLINE;
    ppu->mode_counter = COUNTER_VBLANK_LENGTH;
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
    reset_oamscan(&ppu->oamscan);

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

void ppu_render_scanline(GBState *state) {
    PPUState *ppu = state->ppu;
    SDLComponents *sdl = state->sdl;
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
        SDL_SetRenderDrawColor(sdl->renderer, color, color, color, 0xFF);
        SDL_RenderFillRect(sdl->renderer, &r);
    }
   
}

/* Given an OAM entry pointer, the current scanline ly, and the sprite object 
size (set in LCDC, 8 or 16 rows), return the address for the 2 byte row of 8 
pixels to be rendered for this sprite, or 0 if it does not overlap this line.
*/
WORD get_sprite_row_addr(OAMEntry *oam, BYTE ly, ObjectSize obj_size) {
    WORD sprite_base_addr, sprite_row_addr;
    BYTE index;

    if (obj_size == OBJ_8x16)
        index = oam->index & 0xFE;
    else
        index = oam->index;

    sprite_base_addr = TILEDATA_OBJ + TILE_SIZE_BYTES * index;

    int row_overlap = (int)ly - ((int)oam->y - 16);
    if (row_overlap >= 0 && row_overlap < (int)obj_size) {
        sprite_row_addr = sprite_base_addr + (TILE_BYTES_PER_ROW * row_overlap);
    } else {
        sprite_row_addr = 0;
    }

    return sprite_row_addr;
}

WORD get_current_entry_addr(OAMScan_t *oamscan) {
    return OAM_BASE + sizeof(OAMEntry) * (oamscan->counter >> 1);
}

void ppu_oamscan_cycle(GBState *state) {
    PPUState *ppu = state->ppu;
    LCDControl lcdc = ppu->lcdc;
    LCDStatus stat = ppu->stat;
    PPUMisc misc = ppu->misc;
    OAMScan_t *oamscan = &ppu->oamscan;
    OAMEntry *oam_table_ptr, *current_entry;

    WORD current_entry_addr, tile_row_addr;
    BYTE lsb, msb;

    assert(stat.mode == OAMSCAN);
    assert(misc.ly < 144);
    assert(oamscan->counter < 80);

    if ((oamscan->counter & 1)
        ||lcdc.obj_enable == OFF 
        || oamscan->n_sprites_row == SPRITES_ROW_MAX
        || oamscan->n_sprites_total == SPRITES_TOTAL_MAX
    ) goto ppu_oamscan_end;

    current_entry_addr = get_current_entry_addr(oamscan);
    current_entry = ppu_get_mem_pointer(state, current_entry_addr);

    tile_row_addr = get_sprite_row_addr(current_entry, misc.ly, lcdc.obj_size);

    if (tile_row_addr != 0) {
        printf("ly = %03d: OAM entry #%d, sprite index 0x%02x: row addr 0x%04x\n", 
            misc.ly,
            oamscan->counter >> 1,
            current_entry->index,
            tile_row_addr
        );
        lsb = ppu_read_mem(state, tile_row_addr);
        msb = ppu_read_mem(state, tile_row_addr+1);
        oamscan->current_row_sprites[oamscan->n_sprites_row].entry_addr = current_entry_addr;
        oamscan->current_row_sprites[oamscan->n_sprites_row].oam = *current_entry;
        oamscan->current_row_sprites[oamscan->n_sprites_row].lsb = lsb;
        oamscan->current_row_sprites[oamscan->n_sprites_row].msb = msb;

        oamscan->n_sprites_row++;
        oamscan->n_sprites_total++;
    }

    ppu_oamscan_end:
    oamscan->counter++;
}

/* Sorts the current row sprites array in *descending* order by OAM X coord,
or OAM memory order if same X. We use descending order because we want to 
draw the higher priority sprite pixels last so that they overwrite the 
lower priority ones.
*/
int _oamscan_sort_key(const void *a, const void *b) {
    int diff = ((OAMRow_t*)b)->oam.x - ((OAMRow_t*)a)->oam.x;

    if (diff != 0)
        return diff;
    else
        return ((OAMRow_t*)b)->entry_addr - ((OAMRow_t*)a)->entry_addr;
}
void ppu_oamscan_cleanup(PPUState *ppu) {
    qsort(
        &ppu->oamscan.current_row_sprites,
        ppu->oamscan.n_sprites_row,
        sizeof(OAMRow_t),
        _oamscan_sort_key
    );

    reset_ppu_fifo(&ppu->draw.fifo_bg);
    reset_ppu_fifo(&ppu->draw.fifo_obj);
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
void fetch_current_bg_row(GBState *state) {
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

    if (ppu->scanline.x_pos < GB_WIDTH_PX) {
        fetch_current_bg_row(state);
        
        ppu->scanline.x_pos += 8;
    }
}

void ppu_draw_cleanup(PPUState *ppu) {

}

void ppu_next_scanline(PPUState *ppu) {
    ppu->scanline.counter = PPU_PER_SCANLINE;
    memset(ppu->scanline.bg.buf, 0, SCANLINE_PIXELBUF_SIZE);
    ppu->scanline.bg.offset = ppu->misc.scx & 0x7;
    memset(ppu->scanline.win.buf, 0, SCANLINE_PIXELBUF_SIZE);
    ppu->scanline.win.offset = ppu->misc.wx & 0x7;
    memset(ppu->scanline.obj.buf, 0, SCANLINE_PIXELBUF_SIZE);
    ppu->scanline.x_pos = 0;
    
    ppu->misc.ly++;

    reset_oamscan(&ppu->oamscan);
}

void ppu_next_frame(PPUState *ppu) {
    ppu->frame.counter = PPU_PER_FRAME;
    ppu->frame.win_y = 0;
    ppu->misc.ly = 0;
}

void ppu_do_mode_switch(PPUState *ppu) {
    PPUMode next_mode;
    unsigned int next_mode_counter;
    assert(ppu->mode_counter == 0);

    switch (ppu->stat.mode) {
        case OAMSCAN:
            next_mode_counter = COUNTER_DRAW_MIN_LENGTH; // at minimum
            next_mode = DRAW;
            ppu_oamscan_cleanup(ppu);
            break;
        case DRAW:
            next_mode_counter = ppu->scanline.counter;
            assert(next_mode_counter >= COUNTER_HBLANK_MIN_LENGTH 
                && next_mode_counter <= COUNTER_HBLANK_MAX_LENGTH);
            next_mode = HBLANK;
            ppu_draw_cleanup(ppu);
            break;
        case HBLANK:
            assert(ppu->scanline.counter == 0);
            if (ppu->misc.ly < (N_DRAW_SCANLINES - 1)) {
                next_mode = OAMSCAN;
                next_mode_counter = COUNTER_OAMSCAN_LENGTH;
            } else {
                /* End of HBLANK for scanline 143. We have not yet incremented
                LY - which happens in ppu_next_scanline() - 
                */
                assert(ppu->frame.counter == COUNTER_VBLANK_LENGTH);
                assert(ppu->misc.ly == 143);
                next_mode = VBLANK;
                next_mode_counter = COUNTER_VBLANK_LENGTH;
            }

            break;
        case VBLANK:
            assert(ppu->frame.counter == 0);
            next_mode_counter = COUNTER_OAMSCAN_LENGTH;
            next_mode = OAMSCAN;
            break;
        default:
            assert(0);
            break;
    }
    assert(next_mode_counter <= 10*PPU_PER_SCANLINE);
    ppu->mode_counter = next_mode_counter;
    ppu->stat.mode = next_mode;
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
            ppu_oamscan_cycle(state);
            break;
        case DRAW:
            ppu_draw_cycle(state);
            break;
    }

    ppu_cycle_end:
    ppu->scanline.counter--;
    ppu->frame.counter--;
    ppu->mode_counter--;

    if (ppu->mode_counter == 0)
        ppu_do_mode_switch(ppu);

    if (ppu->scanline.counter == 0) {
        ppu_render_scanline(state);
        ppu_next_scanline(ppu);
    }

    if (ppu->frame.counter == 0) {
        SDL_UpdateWindowSurface(state->sdl->window);
        ppu_next_frame(ppu);
    }
}

