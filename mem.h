#ifndef GB_MEMORY
#define GB_MEMORY

#include "base.h"
#include <time.h>

#define UNINIT 0xFF

/* Function prototypes */

BYTE read_mem(GBState *, WORD, BYTE);
int write_mem(GBState *, WORD, BYTE, BYTE);
BYTE *get_mem_pointer(GBState *, WORD, BYTE);

#define __READ_ARGS_DECL GBState *state, WORD rel_addr, BYTE flags
#define __READ_ARGS state, rel_addr, flags
#define __WRITE_ARGS_DECL GBState *state, WORD rel_addr, BYTE data, BYTE flags
#define __WRITE_ARGS state, rel_addr, data, flags
#define READ_FUNC(__name) static inline BYTE __name(__READ_ARGS_DECL)
#define WRITE_FUNC(__name) static inline int __name(__WRITE_ARGS_DECL)
#define __GET_PTR_ARGS_DECL GBState *state, WORD rel_addr, BYTE flags
#define __GET_PTR_ARGS state, rel_addr, flags
#define GET_PTR_FUNC(__name) static inline BYTE *__name(__GET_PTR_ARGS_DECL)

/* Memory access flags
7 | 6 | 5 | 4 | 3 | 2  1  0
D | U | L | A | R | S  S  S
D: MEM_DEBUG - bypass locks/ownership
U: MEM_UNMAPPED - used for boot ROM, this region is inaccessible
L: MEM_LOCKED - this region is locked by something
A: MEM_TRY_ACQUIRE - try to acquire a lock on this region now
R: MEM_TRY_RELEASE - try to release a lock on this region now
S: MEM_SOURCE_* - the owner of this region, or the source of this memory access
 */

#define get_mem_source(__f) ((__f) & 0x7)
#define MEM_SOURCE_CPU 0x0
#define MEM_SOURCE_PPU 0x1
#define MEM_SOURCE_BUTTONS 0x2
#define MEM_SOURCE_TIMER 0x3
#define MEM_SOURCE_INTERRUPT 0x4
#define MEM_SOURCE_DMA 0x5

#define MEM_LOCKED 0x20
#define MEM_DEBUG 0x80
#define MEM_UNMAPPED 0x40

#define SYS_OAM_BASE 0xFE00
#define SYS_VRAM_BASE 0x8000
#define SYS_WRAM_BASE 0xC000
#define SYS_IOREG_BASE 0xFF00
#define SYS_HRAM_BASE 0xFF80

/* Pretty much the same as MemoryRegion_t from base.h; the sys_*_ioreg
functions locate the IOReg_t entry for the given address and call its 
access functions. */
typedef struct {
    char name[6];
    WORD addr;
    BYTE (*read)(GBState *, WORD, BYTE);
    BYTE (*write)(GBState *, WORD, BYTE, BYTE);
} IOReg_t;

enum cart_type {
    CART_ROM=0,
    CART_MBC1=1,
    CART_MBC1_RAM=2,
    CART_MBC1_RAM_BAT=3,
    CART_MBC2=5,
    CART_MBC2_BAT=6,
    CART_ROM_RAM=8,
    CART_ROM_RAM_BAT=9,
    CART_MMM01=0xB,
    CART_MMM01_RAM=0xC,
    CART_MMM01_RAM_BAT=0xD,
    CART_MBC3_TIMER_BAT=0xF,
    CART_MBC3_TIMER_RAM_BAT=0x10,
    CART_MBC3=0x11,
    CART_MBC3_RAM=0x12,
    CART_MBC3_RAM_BAT=0x13,
    CART_MBC5=0x19,
    CART_MBC5_RAM=0x1A,
    CART_MBC5_RAM_BAT=0x1B,
    CART_MBC5_RUM=0x1C,
    CART_MBC5_RUM_RAM=0x1D,
    CART_MBC5_RUM_RAM_BAT=0x1E,
    CART_MBC6=0x20,
    CART_MBC7_SENS_RUM_RAM_BAT=0x22,
    CART_CAMERA=0xFC,
    CART_TAMA5=0xFD,
    CART_HUC3=0xFE,
    CART_HUC1_RAM_BAT=0xFF
};

enum rom_size {
    ROM_32KB=0,
    ROM_64KB=1,
    ROM_128KB=2,
    ROM_256KB=3,
    ROM_512KB=4,
    ROM_1MB=5,
    ROM_2MB=6,
    ROM_4MB=7,
    ROM_8MB=8,
    ROM_1_1MB=0x52,
    ROM_1_2MB=0x53,
    ROM_1_5MB=0x54
};

enum ram_size {
    RAM_NONE=0,
    RAM_2KB=1,
    RAM_8KB=2,
    RAM_32KB=3,
    RAM_128KB=4,
    RAM_64KB=5
};

#define ROM_BANK_SIZE 0x4000
#define RAM_BANK_SIZE 0x2000
/* Contains the state needed to manage the MBC1 ROM/RAM 
bank controller. rom_banks and ram_banks are pointers to 
the actual memory data. This basically represents the cartridge.*/
typedef struct {
    int ram_enabled;
    int active_rom_bank;
    int reg_1_5bits;
    int rom_bank_mask;
    int n_rom_banks;
    BYTE *rom_banks;
    int reg_2_2bits;
    int active_ram_bank;
    int n_ram_banks;
    BYTE *ram_banks;
    enum {DEFAULT=0, LARGE_RAM=1, LARGE_ROM=2} cart_type;
    enum {MODE_SIMPLE=0, MODE_ADVANCED=1} bank_mode;
} MBC1CartState;

typedef struct {
    int ram_rtc_enabled;
    int active_rom_bank;
    int reg_1_7bits;
    int rom_bank_mask;
    int n_rom_banks;
    BYTE *rom_banks;
    int reg_2_4bits;
    int active_ram_bank;
    int active_rtc_bank;
    int n_ram_banks;
    BYTE *ram_banks;
    enum {MODE_RAM=0, MODE_RTC=1} bank_mode; 
    struct {
        enum {LATCH_0=0, LATCH_1=1, LATCHED=2} latch;
        time_t time_epoch;
        struct tm *time_struct;
        int halt;
        int day_carry;
    } rtc;
} MBC3CartState;

typedef struct {
    BYTE rom[0x8000];
} BasicCartState;

typedef struct {
    BYTE mem[0x10000];
}   DebugMemState;

/* This is the memory structure for components present on the Game Boy system
itself. bootrom_mapped is a flag. */
typedef struct {
    int bootrom_mapped;
    // 0x00-0x100
    BYTE *bootrom;
    // 0x8000-0x9FFF
    BYTE *vram;
    // 0xA000-DFFF
    BYTE *wram;
    // 0xFE00-0xFE9F
    BYTE *oam_table;
    
    // 0xFF80-0xFFFE
    BYTE *hram;
} SysMemState;

MemoryState *initialize_memory(CartridgeHeader *);
void replace_mem_region(const MemoryRegion *, MemoryRegion **);

void teardown_memory(MemoryState *);
TimerState *initialize_timer(void);
void teardown_timer(TimerState *);
DMAState *initialize_dma(void);
void teardown_dma(DMAState *);

void task_div_timer(GBState *);
void task_tima_timer(GBState *);
void task_dma_cycle(GBState *);

#define unused_ioreg(__addr) \
    (IOReg_t){.name="none\0", .addr=__addr, .check_access=&_check_always_yes, .read=&_read_unimplemented, .write=&_write_unimplemented}
#define named_ioregs(__ptr) (IORegs *)(__ptr)

#define mem_cart(__state, __type) ((__type*)__state->mem->cartridge->state)
#define mem_sys(__state, __type) ((__type*)__state->mem->system->state)

#endif // GB_MEMORY