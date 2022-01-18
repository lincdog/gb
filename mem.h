#ifndef GB_MEMORY
#define GB_MEMORY

#include "base.h"

/* Memory mapping */
#define REGION_CHECK(x, lb, ub) (((WORD)(x)>=lb)&&((WORD)(x)<=ub)))

#define is_fixed_rom(x) REGION_CHECK(x, 0x0000, 0x3FFF)
#define is_rom_selector(x) REGION_CHECK(x, 0x2000, 0x3FFF)
#define is_banked_rom(x) REGION_CHECK(x, 0x4000, 0x7FFF)
#define is_rom(x) REGION_CHECK(x, 0x0000, 0x7FFF)
#define is_ram_selector(x) REGION_CHECK(x, 0x4000, 0x5FFF)
#define is_vram(x) REGION_CHECK(x, 0x8000, 0x9FFF)
#define is_banked_ram(x) REGION_CHECK(x, 0xA000, 0xBFFF)
#define is_wram(x) REGION_CHECK(x, 0xC000, 0xDFFF)
#define is_eram(x) REGION_CHECK(x, 0xE000, 0xFDFF)
#define is_oam(x) REGION_CHECK(x, 0xFE00, 0xFE9F)
#define is_ioreg(x) REGION_CHECK(x, 0xFF00, 0xFF7F)
#define is_unusable(x) (is_eram(x) || REGION_CHECK(x, 0xFEA0, 0xFEFF))
#define is_hram(x) REGION_CHECK(x, 0xFF80, 0xFFFE)
#define is_ram(x) (REGION_CHECK(x, 0x8000, 0xDFFF) || is_hram(x) || is_ioreg(x))

/* Function prototypes */

BYTE read_8(WORD, BYTE *);
WORD read_16(WORD, BYTE *);
int write_8(WORD, BYTE *, BYTE);
int write_16(WORD, BYTE *, WORD);

BYTE read_mem(GBState *, WORD);
BYTE write_mem(GBState *, WORD, BYTE);

#endif // GB_MEMORY