#ifndef GB_VIDEO
#define GB_VIDEO

#include "base.h"
#include "mem.h"
#include <SDL.h>

#define GB_HEIGHT_PX 160
#define GB_WIDTH_PX 144
#define EMU_HEIGHT_PX (4*GB_HEIGHT_PX)
#define EMU_WIDTH_PX (4*GB_WIDTH_PX)
#define GB_FULL_SIZE 256
#define TILE_SIZE_BYTES 16
#define TILEMAP_SIZE_BYTES 1024
#define TILEMAP_AREA0 0x9800
#define TILEMAP_AREA1 0x9C00
#define TILEDATA_AREA0 0x9000
#define TILEDATA_AREA1 0x8000
#define COLORS_BGWIN 0
#define COLORS_OBJ 1
#define COLORS_NONE 0xFF
#define PALETTE_DEFAULT 0xE4

typedef struct __attribute__ ((packed)) {
    BYTE y;
    BYTE x;
    BYTE index;
    BYTE flags;
} SpriteAttr;

typedef struct {
    SpriteAttr oam_attrs[40];
} OAMAttrTable;

// If LCDC.4 == 1:
#define TILE_INDEX_TO_ADDR_8000(__i) (WORD)(0x8000 + 0x10 * (BYTE)__i)
// If LCDC.4 == 0:
#define TILE_INDEX_TO_ADDR_8800(__i) (WORD)(0x9000 + 0x10 * (char)__i)

PPUState *initialize_ppu(void);
void teardown_ppu(PPUState *);
SDL_Surface *new_8bit_surface(int, int, BYTE, BYTE);
SDL_Surface *new_8bit_surface_from(BYTE *, int, int, BYTE, BYTE);

#endif // GB_VIDEO