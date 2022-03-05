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

void reset_pixelbuf(Pixelbuf_t *px) {
    px->offset = 0;
    px->x_pos = 0;
    memset(px->buf, 0, SCANLINE_PIXELBUF_SIZE);
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
    ppu->lcdc.lcd_enable = OFF;
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

    ppu->frame.counter = PPU_PER_FRAME + COUNTER_VBLANK_LENGTH;
    ppu->frame.win_y = 0;
    ppu->frame.in_window = OFF;

    ppu->scanline.counter = PPU_PER_SCANLINE;
    memset(ppu->scanline.priority, PX_PRIO_NULL, GB_WIDTH_PX);
    ppu->mode_counter = PPU_PER_FRAME + COUNTER_VBLANK_LENGTH;

    reset_pixelbuf(&ppu->scanline.bg);
    reset_pixelbuf(&ppu->scanline.win);
    reset_pixelbuf(&ppu->scanline.obj);
    ppu->scanline.obj.offset = 8;

    ppu->misc.scy = 0x00;
    ppu->misc.scx = 0x00;
    ppu->misc.ly = 0x00;
    ppu->misc.lyc = 0x00;
    ppu->misc.wy = 0x00;
    ppu->misc.wx = 0x00;
    // 11 11 11 00 
    ppu->misc.bgp = 0xFC;
    ppu->misc.obp0 = PALETTE_DEFAULT;
    ppu->misc.obp1 = PALETTE_DEFAULT;
    
    reset_ppu_fifo(&ppu->draw.fifo_bg);
    reset_ppu_fifo(&ppu->draw.fifo_obj);
    reset_oamscan(&ppu->oamscan);

    ppu->stat_interrupt = OFF;

    return ppu;
}

void teardown_ppu(PPUState *ppu) {
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
        dest[7] = colors[(bit_7(lsb) >> 7) | (bit_7(msb) >> 6)];
        dest[6] = colors[(bit_6(lsb) >> 6) | (bit_6(msb) >> 5)];
        dest[5] = colors[(bit_5(lsb) >> 5) | (bit_5(msb) >> 4)];
        dest[4] = colors[(bit_4(lsb) >> 4) | (bit_4(msb) >> 3)];
        dest[3] = colors[(bit_3(lsb) >> 3) | (bit_3(msb) >> 2)];
        dest[2] = colors[(bit_2(lsb) >> 2) | (bit_2(msb) >> 1)];
        dest[1] = colors[(bit_1(lsb) >> 1) | (bit_1(msb) >> 0)];
        dest[0] = colors[(bit_0(lsb) >> 0) | (bit_0(msb) << 1)];
    } else {
        dest[0] = colors[(bit_7(lsb) >> 7) | (bit_7(msb) >> 6)];
        dest[1] = colors[(bit_6(lsb) >> 6) | (bit_6(msb) >> 5)];
        dest[2] = colors[(bit_5(lsb) >> 5) | (bit_5(msb) >> 4)];
        dest[3] = colors[(bit_4(lsb) >> 4) | (bit_4(msb) >> 3)];
        dest[4] = colors[(bit_3(lsb) >> 3) | (bit_3(msb) >> 2)];
        dest[5] = colors[(bit_2(lsb) >> 2) | (bit_2(msb) >> 1)];
        dest[6] = colors[(bit_1(lsb) >> 1) | (bit_1(msb) >> 0)];
        dest[7] = colors[(bit_0(lsb) >> 0) | (bit_0(msb) << 1)];
    }
}

/* Given an OAM entry pointer, the current scanline ly, and the sprite object 
size (set in LCDC, 8 or 16 rows), return the address for the 2 byte row of 8 
pixels to be rendered for this sprite, or 0 if it does not overlap this line.
*/
WORD get_sprite_row_addr(OAMEntry *oam, BYTE ly, ObjectSize obj_size) {
    WORD sprite_base_addr, sprite_row_addr, base_offset;
    BYTE index;

    if (obj_size == OBJ_8x16)
        index = oam->index & 0xFE;
    else
        index = oam->index;

    sprite_base_addr = TILEDATA_OBJ + TILE_SIZE_BYTES * index;

    int row_overlap = (int)ly - ((int)oam->y - 16);
    if (row_overlap >= 0 && row_overlap < (int)obj_size) {
        if (oam->flags & TILE_Y_FLIP)
            base_offset = TILE_BYTES_PER_ROW * (((BYTE)obj_size - 1) - row_overlap);
        else
            base_offset = TILE_BYTES_PER_ROW * row_overlap;
        
        sprite_row_addr = sprite_base_addr + base_offset;
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
        || ppu->frame.n_sprites_total == SPRITES_TOTAL_MAX
    ) goto ppu_oamscan_end;

    current_entry_addr = get_current_entry_addr(oamscan);
    current_entry = ppu_get_mem_pointer(state, current_entry_addr);

    tile_row_addr = get_sprite_row_addr(current_entry, misc.ly, lcdc.obj_size);

    if (tile_row_addr != 0) {
        /*printf("ly = %03d: OAM entry #%d, sprite index 0x%02x: row addr 0x%04x\n", 
            misc.ly,
            oamscan->counter >> 1,
            current_entry->index,
            tile_row_addr
        );*/
        lsb = ppu_read_mem(state, tile_row_addr);
        msb = ppu_read_mem(state, tile_row_addr+1);
        oamscan->current_row_sprites[oamscan->n_sprites_row].entry_addr = current_entry_addr;
        oamscan->current_row_sprites[oamscan->n_sprites_row].oam = *current_entry;
        oamscan->current_row_sprites[oamscan->n_sprites_row].lsb = lsb;
        oamscan->current_row_sprites[oamscan->n_sprites_row].msb = msb;

        oamscan->n_sprites_row++;
        ppu->frame.n_sprites_total++;
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

void fetch_current_obj_row(GBState *state) {
    PPUState *ppu = state->ppu;
    LCDControl lcdc = ppu->lcdc;
    LCDStatus stat = ppu->stat;
    PPUMisc misc = ppu->misc;
    Pixelbuf_t *obj = &ppu->scanline.obj;
    OAMScan_t *oamscan = &ppu->oamscan;
    OAMRow_t *entry;

    assert(stat.mode == DRAW);
    assert(lcdc.obj_enable == ON);

    if (obj->x_pos >= oamscan->n_sprites_row) 
        goto obj_fetch_end;

    int x_start, prio_start, prio_stop;
    /* XXX we are using "x_pos" as the index of the current sprite to be rendered, not
    anything to do with actual x position */
    entry = &oamscan->current_row_sprites[obj->x_pos];
    x_start = entry->oam.x;

    if (x_start >= GB_WIDTH_PX+8)
        goto obj_fetch_end;
    
    BYTE palette = (entry->oam.flags & TILE_PALETTE_NUM) ? misc.obp1 : misc.obp0;
    unpack_row(entry->lsb, entry->msb,
        &obj->buf[x_start], entry->oam.flags, palette);
    
    prio_start = (x_start < 8) ? 0 : x_start - 8;
    prio_stop = 8 - (8 - x_start);

    if (! (entry->oam.flags & TILE_BG_OVER_OBJ)) {
        for (int i = prio_start; i < prio_stop; i++)
            ppu->scanline.priority[i] = PX_PRIO_OBJ;
    }

    obj_fetch_end:
    obj->x_pos++;
}


/* Returns the address (in the tilemap at base) at which the tile index 
 for the current tile can be found. "Current tile" means the tile that needs to be
 rendered given x scroll scx (pixels), y scroll scy (pixels), cur_x X offset in row (pixels,
 0-160), ly current scanline (pixels, 0-144)
 */
WORD get_bg_tilemap_addr(WORD base, BYTE scx, BYTE scy, BYTE cur_x, BYTE ly) {
    WORD tile_x, tile_y;
    tile_x = compute_bg_tile_x(scx, cur_x);
    tile_y = compute_bg_tile_y(scy, ly);

    return compute_tilemap_addr(base, tile_x, tile_y);
}

WORD compute_tiledata_addr(WORD base, BYTE ind) {
    BYTE mask = (base == TILEDATA_AREA0) ? 0x7F : 0xFF;
    WORD offset = TILE_SIZE_BYTES * (ind & mask) - TILE_SIZE_BYTES * (ind & ~mask);

    return base + offset;
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
    //Drawing_t draw = ppu->draw;
    Pixelbuf_t *bg = &ppu->scanline.bg;

    assert(stat.mode == DRAW);
    assert(lcdc.bg_window_enable == ON);

    if (bg->x_pos >= SCANLINE_PIXELBUF_SIZE)
        goto bg_fetch_end;

    WORD map_area, data_area, map_addr, tile_addr;
    BYTE tile_index, x_pixel_offset, y_pixel_offset;
    BYTE lsb, msb;
    lsb = msb = 0;

    map_area = (WORD)lcdc.bg_map_area;
    data_area = (WORD)lcdc.bg_win_data_area;

    map_addr = get_bg_tilemap_addr(map_area, misc.scx, misc.scy, bg->x_pos, misc.ly);

    tile_index = ppu_read_mem(state, map_addr);

    y_pixel_offset = compute_y_pixel_offset(misc.scy, misc.ly);
    
    // Gets us to the top left corner of the tile we want
    tile_addr = compute_tiledata_addr(data_area, tile_index);

    // Add 2 for each row down in the tile we are (2 bytes per row)
    tile_addr += (TILE_BYTES_PER_ROW * y_pixel_offset);

    lsb = ppu_read_mem(state, tile_addr);
    msb = ppu_read_mem(state, tile_addr+1);

    assert(bg->x_pos < SCANLINE_PIXELBUF_SIZE);
    unpack_row(lsb, msb, &bg->buf[bg->x_pos], 0, misc.bgp);

    bg_fetch_end:
    bg->x_pos += 8;
}

/* Returns 1 if the current X position and Y position (scanline) are inside 
the window, whose top left corner in screen coords is given by wx-7, wy.
If wx <= 7, we are always inside the window in X.
*/
int in_window_p(BYTE wx, BYTE wy, BYTE cur_x, BYTE ly) {
    int result = 1;

    if (ly < wy)
        result = 0;

    if (cur_x < less_7_or_0(wx))
        result = 0;
    
    return result;
}

/* Returns the address of the current window tile given the current X position,
current Y position (scanline), and window's top left corner specified by wx and wy.
Assumes (asserts) that we are indeed inside the window.
*/
WORD get_win_tilemap_addr(WORD base, BYTE wx, BYTE wy, BYTE cur_x, BYTE ly) {
    BYTE tile_x, tile_y;

    assert(in_window_p(wx, wy, cur_x, ly));

    tile_y = (ly - wy) >> 3;

    tile_x = (cur_x - less_7_or_0(wx)) >> 3;

    return compute_tilemap_addr(base, tile_x, tile_y);
}

void fetch_current_win_row(GBState *state) {
    PPUState *ppu = state->ppu;
    LCDControl lcdc = ppu->lcdc;
    LCDStatus stat = ppu->stat;
    PPUMisc misc = ppu->misc;
    //Drawing_t draw = ppu->draw;
    Pixelbuf_t *win = &ppu->scanline.win;
    Frame_t *frame = &ppu->frame;

    assert(stat.mode == DRAW);
    assert(lcdc.bg_window_enable == ON && lcdc.window_enable == ON);
    
    WORD map_area, data_area, map_addr, tile_addr;
    BYTE tile_index, x_pixel_offset, y_pixel_offset;
    BYTE lsb, msb;
    lsb = msb = 0;

    map_area = (WORD)lcdc.win_map_area;
    data_area = (WORD)lcdc.bg_win_data_area;

    if ( ! in_window_p(misc.wx, misc.wy, win->x_pos, misc.ly)
        || win->x_pos >= GB_WIDTH_PX
    ) goto win_fetch_end;

    map_addr = get_win_tilemap_addr(map_area, misc.wx, misc.wy, win->x_pos, misc.ly);

    tile_index = ppu_read_mem(state, map_addr);

    y_pixel_offset = (misc.ly - misc.wy) & 0x7;
    x_pixel_offset = (win->x_pos - less_7_or_0(misc.wx)) & 0x7;
        
    // Gets us to the top left corner of the tile we want
    tile_addr = compute_tiledata_addr(data_area, tile_index);
    
    // Add 2 for each row down in the tile we are (2 bytes per row)
    tile_addr += (TILE_BYTES_PER_ROW * y_pixel_offset);

    lsb = ppu_read_mem(state, tile_addr);
    msb = ppu_read_mem(state, tile_addr+1);

    win->offset = win_x_offset(misc.wx);

    assert(win->x_pos < SCANLINE_PIXELBUF_SIZE);
    unpack_row(lsb, msb, &win->buf[win->x_pos], 0, misc.bgp);
    int prio_start, prio_stop;
    prio_start = win->x_pos;
    prio_stop = (win->x_pos + 8) - win->offset;
    for (int i = prio_start; i < prio_stop; i++)
        ppu->scanline.priority[i] = PX_PRIO_WIN;

    win_fetch_end:
    win->x_pos += 8;
}

void ppu_draw_cycle(GBState *state) {
    PPUState *ppu = state->ppu;

    if (ppu->lcdc.bg_window_enable == ON) {
        fetch_current_bg_row(state);

        if (ppu->lcdc.window_enable == ON)
            fetch_current_win_row(state);
    }

    if (ppu->lcdc.obj_enable == ON)
        fetch_current_obj_row(state);
}

void ppu_draw_cleanup(PPUState *ppu) {

}


void ppu_render_scanline(GBState *state) {
    PPUState *ppu = state->ppu;
    PPUMisc misc = ppu->misc;
    LCDControl lcdc = ppu->lcdc;
    Scanline_t *scanline = &ppu->scanline;
    Pixelbuf_t *current_px;
    SDLComponents *sdl = state->sdl;
    SDL_Rect r;
    BYTE color;

    r.y = ppu->misc.ly;
    r.x = 0;//ppu->scanline.x_pos;
    r.w = 1; 
    r.h = 1;

    BYTE prio, alpha;

    for (; r.x < GB_WIDTH_PX; r.x++) {
        
        prio = scanline->priority[r.x];
        
        switch (prio) {
            case PX_PRIO_NULL:
                current_px = NULL;
                alpha = 0xFF;
                break;
            case PX_PRIO_BG:
                current_px = &scanline->bg;
                alpha = 0xFF;
                break;
            case PX_PRIO_WIN:
                current_px = &scanline->win;
                alpha = 0xFF;
                break;
            case PX_PRIO_OBJ:
                current_px = &scanline->obj;
                alpha = 0xFF;
                break;
            default:
                assert(0);
                break;
        }

        if (current_px != NULL) {
            assert(r.x + current_px->offset < SCANLINE_PIXELBUF_SIZE);
            color = color_table[current_px->buf[r.x + current_px->offset]];
            SDL_SetRenderDrawColor(sdl->renderer, color, color, color, alpha);
            SDL_RenderFillRect(sdl->renderer, &r);
        }
    }
   
}

void ppu_next_scanline(PPUState *ppu) {
    ppu->scanline.counter = PPU_PER_SCANLINE;
    memset(ppu->scanline.bg.buf, 0, SCANLINE_PIXELBUF_SIZE);
    ppu->scanline.bg.offset = ppu->misc.scx & 0x7;
    ppu->scanline.bg.x_pos = 0;

    memset(ppu->scanline.win.buf, 0, SCANLINE_PIXELBUF_SIZE);
    ppu->scanline.win.offset = ppu->misc.wx & 0x7;
    ppu->scanline.win.x_pos = less_7_or_0(ppu->misc.wx);

    memset(ppu->scanline.obj.buf, 0, SCANLINE_PIXELBUF_SIZE);
    ppu->scanline.obj.x_pos = 0;
    ppu->scanline.obj.offset = 8;
    
    if (ppu->lcdc.bg_window_enable == ON)
        memset(ppu->scanline.priority, PX_PRIO_BG, GB_WIDTH_PX);
    else
        memset(ppu->scanline.priority, PX_PRIO_NULL, GB_WIDTH_PX);

    ppu->misc.ly++;

    reset_oamscan(&ppu->oamscan);
}

void ppu_next_frame(PPUState *ppu) {
    ppu->frame.counter = PPU_PER_FRAME;
    ppu->frame.win_y = 0;
    ppu->frame.in_window = OFF;
    ppu->frame.n_sprites_total = 0;
    
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

ToggleEnum ppu_check_stat_interrupts(PPUState *ppu) {
    ToggleEnum new_level, old_level;
    old_level = ppu->stat_interrupt;

    LCDStatus stat = ppu->stat;
    new_level = (stat.lyc_ly_equal & stat.lyc_ly_interrupt)
        | (stat.mode_0_interrupt & (stat.mode == HBLANK))
        | (stat.mode_1_interrupt & (stat.mode == VBLANK))
        | (stat.mode_2_interrupt & (stat.mode == OAMSCAN));
    
    if (new_level == old_level)
        return STABLE;
    else
        return new_level;
}

void task_ppu_cycle(GBState *state) {
    PPUState *ppu = state->ppu;
    LCDControl lcdc = ppu->lcdc;
    LCDStatus stat = ppu->stat;
    PPUMisc misc = ppu->misc;
    ToggleEnum stat_interrupt;

    if (misc.ly == misc.lyc) {
        ppu->stat.lyc_ly_equal = ON;
    } else {
        ppu->stat.lyc_ly_equal = OFF;
    }
        
    if (lcdc.lcd_enable == OFF)
        goto ppu_lcd_off;
    
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

    if (ppu->mode_counter == 0) {
        ppu_do_mode_switch(ppu);
        stat_interrupt = ppu_check_stat_interrupts(ppu);
        switch (stat_interrupt) {
            case ON:
                REQUEST_INTERRUPT(state, INT_STAT);
                /* Fall through */
            case OFF:
                ppu->stat_interrupt = stat_interrupt;
                break;
            case STABLE:
            default:
                break;
        }
        if (ppu->stat.mode == VBLANK)
            REQUEST_INTERRUPT(state, INT_VBLANK);
    }

    if (ppu->scanline.counter == 0) {
        ppu_render_scanline(state);
        ppu_next_scanline(ppu);
    }

    if (ppu->frame.counter == 0) {
        SDL_UpdateWindowSurface(state->sdl->window);
        ppu_next_frame(ppu);
    }

    ppu_lcd_off:
    1;
}

