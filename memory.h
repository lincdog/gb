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
#define is_ioreg(x) REGION_CHECK(x, 0xFF00, 0xFF7F)
#define is_unusable(x) (is_eram(x) || REGION_CHECK(x, 0xFEA0, 0xFEFF))
#define is_hram(x) REGION_CHECK(x, 0xFF80, 0xFFFE)
#define is_ram(x) (REGION_CHECK(x, 0x8000, 0xDFFF) || is_hram(x) || is_ioreg(x))

#define BLANK(n) BYTE __space_ ## n
struct GBIORegs {
    BYTE buttons; //    00
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
    BYTE int_flag; //   0F 
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
    BLANK(1F); //        1F
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
    BYTE stack[127];
    BYTE int_enable;
};

/* Function prototypes */
BYTE read_8(WORD addr, BYTE *code);
WORD read_16(WORD addr, BYTE *code);
int write_8(WORD addr, BYTE *code, BYTE data);
int write_16(WORD addr, BYTE *code, WORD data);

#endif // GB_MEMORY