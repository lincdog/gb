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

const BYTE UNINIT = 0xFF;

/* Function prototypes */

BYTE read_mem(GBState *, WORD, BYTE);
int write_mem(GBState *, WORD, BYTE, BYTE);

#define __READ_ARGS_DECL GBState *state, WORD rel_addr, BYTE flags
#define __READ_ARGS state, rel_addr, flags
#define __WRITE_ARGS_DECL GBState *state, WORD rel_addr, BYTE data, BYTE flags
#define __WRITE_ARGS state, rel_addr, data, flags
#define READ_FUNC(__name) static inline BYTE __name(__READ_ARGS_DECL)
#define WRITE_FUNC(__name) static inline int __name(__WRITE_ARGS_DECL)


/* Memory access flags */
#define mem_source(__f) ((__f) & 0x3)
#define MEM_SOURCE_CPU 0x0
#define MEM_SOURCE_PPU 0x1
#define MEM_SOURCE_BUTTONS 0x2
#define MEM_SOURCE_TIMER 0x3
#define MEM_DEBUG 0x80
#define MEM_UNMAPPED 0x40

typedef struct {
    char name[6];
    WORD addr;
    BYTE (*read)(GBState *, WORD, BYTE);
    BYTE (*write)(GBState *, WORD, BYTE, BYTE);
} IOReg_t;

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

READ_FUNC(_read_unimplemented);
WRITE_FUNC(_write_unimplemented);
READ_FUNC(_read_p1);
WRITE_FUNC(_write_p1);

IORegs *initialize_ioregs(void);
MemoryState *initialize_memory(void);

#define unused_ioreg(__addr) \
    (IOReg_t){.name="none\0", .addr=__addr, .read=&_read_unimplemented, .write=&_write_unimplemented}
#define named_ioregs(__ptr) (IORegs *)(__ptr)

/* DMG BootRom */
const BYTE DMG_boot_rom[] = {
    0x31, 0xFE, 0xFF, 0xAF, 0x21, 0xFF, 0x9F, 0x32,
    0xCB, 0x7C, 0x20, 0xFB, 0x21, 0x26, 0xFF, 0x0E,
    0x11, 0x3E, 0x80, 0x32, 0xE2, 0x0C, 0x3E, 0xF3,
    0xE2, 0x32, 0x3E, 0x77, 0x77, 0x3E, 0xFC, 0xE0,
    0x47, 0x11, 0x04, 0x01, 0x21, 0x10, 0x80, 0x1A,
    0xCD, 0x95, 0x00, 0xCD, 0x96, 0x00, 0x13, 0x7B,
    0xFE, 0x34, 0x20, 0xF3, 0x11, 0xD8, 0x00, 0x06,
    0x08, 0x1A, 0x13, 0x22, 0x23, 0x05, 0x20, 0xF9,
    0x3E, 0x19, 0xEA, 0x10, 0x99, 0x21, 0x2F, 0x99,
    0x0E, 0x0C, 0x3D, 0x28, 0x08, 0x32, 0x0D, 0x20,
    0xF9, 0x2E, 0x0F, 0x18, 0xF3, 0x67, 0x3E, 0x64,
    0x57, 0xE0, 0x42, 0x3E, 0x91, 0xE0, 0x40, 0x04,
    0x1E, 0x02, 0x0E, 0x0C, 0xF0, 0x44, 0xFE, 0x90,
    0x20, 0xFA, 0x0D, 0x20, 0xF7, 0x1D, 0x20, 0xF2,
    0x0E, 0x13, 0x24, 0x7C, 0x1E, 0x83, 0xFE, 0x62,
    0x28, 0x06, 0x1E, 0xC1, 0xFE, 0x64, 0x20, 0x06,
    0x7B, 0xE2, 0x0C, 0x3E, 0x87, 0xE2, 0xF0, 0x42,
    0x90, 0xE0, 0x42, 0x15, 0x20, 0xD2, 0x05, 0x20,
    0x4F, 0x16, 0x20, 0x18, 0xCB, 0x4F, 0x06, 0x04,
    0xC5, 0xCB, 0x11, 0x17, 0xC1, 0xCB, 0x11, 0x17,
    0x05, 0x20, 0xF5, 0x22, 0x23, 0x22, 0x23, 0xC9,
    0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B,
    0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
    0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E,
    0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
    0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC,
    0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E,
    0x3C, 0x42, 0xB9, 0xA5, 0xB9, 0xA5, 0x42, 0x3C,
    0x21, 0x04, 0x01, 0x11, 0xA8, 0x00, 0x1A, 0x13,
    0xBE, 0x20, 0xFE, 0x23, 0x7D, 0xFE, 0x34, 0x20,
    0xF5, 0x06, 0x19, 0x78, 0x86, 0x23, 0x05, 0x20,
    0xFB, 0x86, 0x20, 0xFE, 0x3E, 0x01, 0xE0, 0x50
};

const BYTE GAMEBOY_LOGO[] = {
    0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 
    0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
    0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E, 
    0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
    0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 
    0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E
};

#endif // GB_MEMORY