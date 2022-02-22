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
#define TILEMAP_SIZE_BYTES 1024
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

#define COUNTER_OAMSEARCH_START 0
#define COUNTER_OAMSEARCH_STOP 80
#define counter_to_scanline(__c) (int)(__c / 456)
#define COUNTER_VBLANK_START 65664 // 70224 - 4560
#define COUNTER_VBLANK_STOP 70224


typedef struct __attribute__ ((packed)) {
    BYTE y;
    BYTE x;
    BYTE index;
    BYTE flags;
} SpriteAttr;

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