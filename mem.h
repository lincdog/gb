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
#define __CHECK_ACCESS_ARGS_DECL GBState *state, WORD rel_addr, BYTE flags
#define __CHECK_ACCESS_ARGS state, rel_addr, flags
#define CHECK_ACCESS_FUNC(__name) static inline int __name(__CHECK_ACCESS_ARGS_DECL)

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

/* Pretty much the same as MemoryRegion_t from base.h; the sys_*_ioreg
functions locate the IOReg_t entry for the given address and call its 
access functions. */
typedef struct {
    char name[6];
    WORD addr;
    int (*check_access)(GBState *, WORD, BYTE);
    BYTE (*read)(GBState *, WORD, BYTE);
    BYTE (*write)(GBState *, WORD, BYTE, BYTE);
} IOReg_t;

/* A packed structure representing the 128 IO registers by name.
They are contiguous from 0 to 0x7F, with blank spaces inserted where needed,
but 0xFFFF (interrupt enable) is placed at 0x80 to avoid having 126 blanks 
where high RAM would be (0xFF80-0xFFFE). The sys_*_ioreg functions are
aware of this and map 0xFFFF to the relative address 0x80. */
#define BLANK(n) BYTE __space_ ## n
typedef struct __attribute__ ((packed)) {
    BYTE p1; //    00
    BYTE sb; //         01
    BYTE sc; //         02
    BLANK(03); //       03
    BYTE div; //        04
    BYTE tima; //       05
    BYTE tma; //        06
    BYTE tac; //        07
    BLANK(08); //       08
    BLANK(09); //       09
    BLANK(0A); //       0A
    BLANK(0B); //       0B
    BLANK(0C); //       0C
    BLANK(0D); //       0D
    BLANK(0E); //       0E
    BYTE if_; //   0F 
    BYTE nr10; //       10
    BYTE nr11; //       11
    BYTE nr12; //       12
    BYTE nr13; //       13
    BYTE nr14; //       14
    BLANK(15); //       15
    BYTE nr21; //       16
    BYTE nr22; //       17;
    BYTE nr23; //       18;
    BYTE nr24; //       19;
    BYTE nr30; //       1A;
    BYTE nr31; //       1B
    BYTE nr32; //       1C
    BYTE nr33; //       1D
    BYTE nr34; //       1E
    BLANK(1F); //       1F
    BYTE nr41; //       20
    BYTE nr42; //       21
    BYTE nr43; //       22
    BYTE nr44; //       23
    BYTE nr50; //       24
    BYTE nr51; //       25
    BYTE nr52; //       26
    BLANK(27); //       27
    BLANK(28); //       28
    BLANK(29); //       29
    BLANK(2A); //       2A
    BLANK(2B); //       2B
    BLANK(2C); //       2C
    BLANK(2D); //       2D
    BLANK(2E); //       2E
    BLANK(2F); //       2F
    BYTE wav00; //      30
    BYTE wav01; //      31
    BYTE wav02; //      32
    BYTE wav03; //      33
    BYTE wav04; //      34
    BYTE wav05; //      35
    BYTE wav06; //      36
    BYTE wav07; //      37
    BYTE wav08; //      38
    BYTE wav09; //      39
    BYTE wav10; //      3A
    BYTE wav11; //      3B
    BYTE wav12; //      3C
    BYTE wav13; //      3D
    BYTE wav14; //      3E
    BYTE wav15; //      3F
    BYTE lcdc; //       40
    BYTE stat; //       41
    BYTE scy; //        42
    BYTE scx; //        43
    BYTE ly; //         44
    BYTE lyc; //        45
    BYTE dma; //        46
    BYTE bgp; //        47
    BYTE obp0; //       48
    BYTE obp1; //       49
    BYTE wy; //         4A
    BYTE wx; //         4B
    BLANK(4C); //       4C
    BYTE key1; //       4D
    BLANK(4E); //       4E
    BYTE vbk; //        4F
    BYTE boot; //       50
    BYTE hdma1; 
    BYTE hdma2;
    BYTE hdma3;
    BYTE hdma4;
    BYTE hdma5;
    BYTE rp;
    BLANK(57);
    BLANK(58);
    BLANK(59);
    BLANK(5A);
    BLANK(5B);
    BLANK(5C);
    BLANK(5D);
    BLANK(5E);
    BLANK(5F);
    BLANK(60);
    BLANK(61);
    BLANK(62);
    BLANK(63);
    BLANK(64);
    BLANK(65);
    BLANK(66);
    BLANK(67);
    BYTE bcps;
    BYTE bcpd;
    BYTE ocps;
    BYTE ocpd;
    BLANK(6C);
    BLANK(6D);
    BLANK(6E);
    BLANK(6F);
    BYTE svbk;
    BLANK(71);
    BLANK(72);
    BLANK(73);
    BLANK(74);
    BLANK(75);
    BYTE pcm12;
    BYTE pcm34;
    BLANK(78);
    BLANK(79);
    BLANK(7A);
    BLANK(7B);
    BLANK(7C);
    BLANK(7D);
    BLANK(7E);
    BLANK(7F);
    //BYTE stack[127];
    BYTE ie_;
} IORegs;

typedef struct __attribute__((packed)) {
    // 0x100-0x103
    BYTE entry_point[3];
    // 0x104-0x133
    BYTE nintendo_logo[48];
    /* 0x134-0x143 in early cartridges;
     * later ones use 13F-142 for manufacturer,
     * and 143 bit 7 for CGB flag
    */
    BYTE title_or_mfc[16];
    // 0x144-0x145
    BYTE new_licensee[2];
    // 0x146
    BYTE sgb;
    // 0x147
    BYTE cartridge_type;
    // 0x148
    BYTE rom_size;
    // 0x149
    BYTE ram_size;
    // 0x14A
    BYTE destination;
    // 0x14B
    BYTE old_licensee;
    // 0x14C
    BYTE version;
    // 0x14D
    BYTE header_cksum;
    // 0x14E-0x14F
    BYTE global_cksum[2];
} CartridgeHeader;

typedef enum {
    SIMPLE_LARGERAM=0,
    SIMPLE_LARGEROM=1,
    ADVANCED_LARGERAM=2,
    ADVANCED_LARGEROM=3
} BANK_MODE;
/* Contains the state needed to manage the MBC1 ROM/RAM 
bank controller. rom_banks and ram_banks are pointers to 
the actual memory data. This basically represents the cartridge.*/
typedef struct {
    int ram_enabled;
    BYTE rom_bank_number;
    BYTE n_rom_banks;
    BYTE *rom_banks;
    BYTE ram_bank_number;
    BYTE n_ram_banks;
    BYTE *ram_banks;
    BANK_MODE bank_mode;
} MBC1CartState;

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
    // 0xFF00-0xFF7F and 0xFFFF
    IORegs *ioregs;
    // 0xFF80-0xFFFE
    BYTE *hram;
} SysMemState;

IORegs *initialize_ioregs(void);
MemoryState *initialize_memory(MemInitFlag);
void teardown_memory(MemoryState *);
TimerState *initialize_timer(void);
void teardown_timer(TimerState *);

void task_div_timer(GBState *);
void task_tima_timer(GBState *);

#define unused_ioreg(__addr) \
    (IOReg_t){.name="none\0", .addr=__addr, .check_access=&_check_always_yes, .read=&_read_unimplemented, .write=&_write_unimplemented}
#define named_ioregs(__ptr) (IORegs *)(__ptr)


#endif // GB_MEMORY