#ifndef GB_VIDEO
#define GB_VIDEO

#include "base.h"
#include "mem.h"
#include <SDL.h>

#define GB_HEIGHT_PX 144
#define GB_WIDTH_PX 160 
#define EMU_SCALE_FACTOR 4
#define EMU_HEIGHT_PX (EMU_SCALE_FACTOR*GB_HEIGHT_PX)
#define EMU_WIDTH_PX (EMU_SCALE_FACTOR*GB_WIDTH_PX)
#define GB_FULL_SIZE 256
#define TILE_SIZE_BYTES 16
#define TILE_BYTES_PER_ROW 2
#define TILEMAP_SIZE_BYTES 1024
#define TILEMAP_BYTES_PER_ROW 32
#define TILEMAP_AREA0 0x9800
#define TILEMAP_AREA1 0x9C00
#define TILEDATA_AREA0 0x9000
#define TILEDATA_AREA1 0x8000
#define TILEDATA_OBJ 0x8000
#define OAM_BASE 0xFE00
#define COLORS_BGWIN 0
#define COLORS_OBJ 1
#define COLORS_NONE 0xFF
#define PALETTE_DEFAULT 0xE4
#define TILE_PALETTE_NUM 0x10
#define TILE_X_FLIP 0x20
#define TILE_Y_FLIP 0x40
#define TILE_BG_OVER_OBJ 0x80

#define COUNTER_OAMSCAN_LENGTH 80
#define COUNTER_DRAW_MIN_LENGTH 172
#define COUNTER_DRAW_MAX_LENGTH 289
#define COUNTER_HBLANK_MIN_LENGTH 87
#define COUNTER_HBLANK_MAX_LENGTH 204
#define COUNTER_VBLANK_LENGTH 4560

#define N_DRAW_SCANLINES 144
#define N_VBLANK_SCANLINES 10
#define PPU_PER_SCANLINE 456
#define SCANLINE_PER_FRAME 154
#define SCANLINE_PIXELBUF_SIZE (GB_WIDTH_PX + 16)
#define PPU_PER_FRAME (PPU_PER_SCANLINE * SCANLINE_PER_FRAME)
#define SPRITES_ROW_MAX 10
#define SPRITES_TOTAL_MAX 40
#define PX_PRIO_NULL 0
#define PX_PRIO_BG 1
#define PX_PRIO_WIN 2
#define PX_PRIO_OBJ 3

#define compute_bg_tile_x(__scx, __cur_x) ((((__scx + __cur_x) >> 3)) & 0x1F)
#define compute_bg_tile_y(__scy, __ly) ((((__scy + __ly) >> 3)) & 0x1F)
#define compute_x_pixel_offset(__scx, __cur_x) ((__scx) & 0x7)
#define compute_y_pixel_offset(__scy, __ly) ((__scy + __ly) & 0x7)
#define compute_tilemap_addr(__base, __tx, __ty) (__base + (TILEMAP_BYTES_PER_ROW * __ty) + __tx)
#define sign_check(__darea) (((TileDataArea)__darea == DATA_AREA1) ? 0 : 0x80)
#define compute_tiledata_addr(__base, __ind) (__base + TILE_SIZE_BYTES * \
    (__ind ^ sign_check(__base))) - (TILE_SIZE_BYTES * (__ind & sign_check(__base)))
#define less_7_or_0(__wx) ((__wx < 7) ? 0 : __wx - 7)
#define win_x_offset(__wx) ((__wx < 7) ? (7 - __wx) : 0)

#define ppu_write_mem(__state, __addr, __data) write_mem(__state, __addr, __data, MEM_SOURCE_PPU)
#define ppu_read_mem(__state, __addr) read_mem(__state, __addr, MEM_SOURCE_PPU)
#define ppu_get_mem_pointer(__state, __addr) get_mem_pointer(__state, __addr,  MEM_SOURCE_PPU)

PPUState *initialize_ppu(void);
void teardown_ppu(PPUState *);
void task_ppu_cycle(GBState *);

#endif // GB_VIDEO