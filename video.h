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

#define COUNTER_OAMSEARCH_LENGTH 80
#define COUNTER_DRAW_MIN_LENGTH 172
#define COUNTER_DRAW_MAX_LENGTH 289
#define COUNTER_HBLANK_MIN_LENGTH 87
#define COUNTER_HBLANK_MAX_LENGTH 204
#define PPU_PER_SCANLINE 456
#define SCANLINE_PER_FRAME 154
#define SCANLINE_PIXELBUF_SIZE (GB_WIDTH_PX + 16)
#define PPU_PER_FRAME (PPU_PER_SCANLINE * SCANLINE_PER_FRAME)
#define COUNTER_VBLANK_LENGTH 4560
#define SPRITES_ROW_MAX 10
#define SPRITES_TOTAL_MAX 40

#define compute_tile_x(__scx, __cur_x) (((__scx >> 3) + (__cur_x >> 3)) & 0x1F)
#define compute_tile_y(__scy, __ly) (((__scy >> 3) + (__ly >> 3)) & 0x1F)
#define compute_x_pixel_offset(__scx, __cur_x) ((__scx) & 0x7)
#define compute_y_pixel_offset(__scy, __ly) ((__scy + __ly) & 0x7)
#define compute_tilemap_addr(__base, __tx, __ty) (__base + (TILEMAP_BYTES_PER_ROW * __ty) + __tx)
#define sign_check(__darea) (((TileDataArea)__darea == DATA_AREA1) ? 0 : 0x80)
#define compute_tiledata_addr(__base, __ind) __base + (TILE_SIZE_BYTES * \
    (__ind ^ sign_check(__base)) - (__ind & sign_check(__base)))

// If LCDC.4 == 1:
#define TILE_INDEX_TO_ADDR_8000(__i) (WORD)(0x8000 + 0x10 * (BYTE)__i)
// If LCDC.4 == 0:
#define TILE_INDEX_TO_ADDR_8800(__i) (WORD)(0x9000 + 0x10 * (char)__i)

#define ppu_write_mem(__state, __addr, __data) write_mem(__state, __addr, __data, MEM_SOURCE_PPU)
#define ppu_read_mem(__state, __addr) read_mem(__state, __addr, MEM_SOURCE_PPU)
#define ppu_get_mem_pointer(__state, __addr) get_mem_pointer(__state, __addr,  MEM_SOURCE_PPU)

PPUState *initialize_ppu(void);
void teardown_ppu(PPUState *);
SDL_Surface *new_8bit_surface(int, int, BYTE);
SDL_Surface *new_8bit_surface_from(BYTE *, int, int, BYTE);

#endif // GB_VIDEO