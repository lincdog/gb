#ifndef GB_VIDEO
#define GB_VIDEO

#include "base.h"
#include "mem.h"
#include <SDL.h>

typedef struct {
    BYTE x;
    BYTE y;
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

#endif // GB_VIDEO