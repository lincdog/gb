#ifndef GB_VIDEO
#define GB_VIDEO

#include "base.h"
#include "memory.h"
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

const BYTE test_tile[] = {
    0xFF, 0x00, 0x7E, 0xFF, 
    0x85, 0x81, 0x89, 0x83,
    0x93, 0x85, 0xA5, 0x8B, 
    0xC9, 0x97, 0x7E, 0xFF
};

const SDL_Color colors[] = {
    {.r = 0xFF, .g = 0xFF, .b = 0xFF, .a = 0xFF },
    {.r = 0x90, .g = 0x90, .b = 0x90, .a = 0x00 },
    {.r = 0x50, .g = 0x50, .b = 0x50, .a = 0x00 },
    {.r = 0x00, .g = 0x00, .b = 0x00, .a = 0x00 }
};

typedef enum {
    HBlank=0,
    VBlank=1,
    SearchingOAM=2,
    TransferringToLCD=3
} LCDStatus;

// If LCDC.4 == 1:
#define TILE_INDEX_TO_ADDR_8000(__i) (WORD)(0x8000 + 0x10 * (BYTE)__i)
// If LCDC.4 == 0:
#define TILE_INDEX_TO_ADDR_8800(__i) (WORD)(0x9000 + 0x10 * (char)__i)

PPUState *initialize_ppu(void);
void teardown_ppu(PPUState *void);

#endif // GB_VIDEO