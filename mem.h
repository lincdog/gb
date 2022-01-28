#ifndef GB_MEMORY
#define GB_MEMORY

#include "base.h"

/* Memory mapping */
#define REGION_CHECK(x, lb, ub) (((WORD)(x)>=lb)&&((WORD)(x)<=ub))

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
#define is_stack(x) REGION_CHECK(x, 0xFF80, 0xFFFE)
#define is_ram(x) (REGION_CHECK(x, 0x8000, 0xDFFF) || is_stack(x) || is_ioreg(x))


/* Function prototypes */

BYTE read_mem(GBState *, WORD, BYTE);
int write_mem(GBState *, WORD, BYTE, BYTE);

#define READ_FUNC(__name) static inline BYTE __name(GBState *state, WORD addr, BYTE flags)
#define WRITE_FUNC(__name) static inline int __name(GBState *state, WORD addr, BYTE data, BYTE flags)

/* Memory access flags */
#define mem_source(__f) ((__f) & 0x3)
#define MEM_SOURCE_CPU 0x0
#define MEM_SOURCE_PPU 0x1
#define MEM_SOURCE_BUTTONS 0x2
#define MEM_SOURCE_TIMER 0x3
#define MEM_DEBUG 0x80

typedef struct {
    char name[6];
    WORD addr;
    BYTE (*read)(GBState *, WORD, BYTE);
    BYTE (*write)(GBState *, WORD, BYTE, BYTE);
} IOReg_t;

READ_FUNC(_read_unimplemented);
WRITE_FUNC(_write_unimplemented);
READ_FUNC(_read_p1);
WRITE_FUNC(_write_p1);

#define unused_ioreg(__addr) \
    (IOReg_t){.name="none\0", .addr=__addr, .read=&_read_unimplemented, .write=&_write_unimplemented}
#define named_ioregs(__ptr) (IORegs *)(__ptr)

#endif // GB_MEMORY