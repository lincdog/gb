#ifndef GB_BASE
#define GB_BASE

#include <stdint.h>

#define WORD uint16_t
#define BYTE uint8_t
#define START_ADDR 0x100

/* Utility macros */
#define ms_nib(x) (((x) & 0xF0)>>4)
#define ls_nib(x) (((x) & 0x0F))

#define bit_7(x) ((x) & 0x80)
#define bit_6(x) ((x) & 0x40)
#define bit_5(x) ((x) & 0x20)
#define bit_4(x) ((x) & 0x10)
#define bit_3(x) ((x) & 0x08)
#define bit_2(x) ((x) & 0x04)
#define bit_1(x) ((x) & 0x02)
#define bit_0(x) ((x) & 0x01)

#define get_le_word(ptr) (WORD)(ptr[1] | (ptr[2]<<8))

typedef union {
    struct {
        BYTE l;
        BYTE h;
    } w;
    WORD dw;
} reg;

enum CPU_STATE {
    READY,
    IM_8_UNSIGNED,
    IM_8_SIGNED,
    IM_16_LSB, 
    IM_16_MSB, 
    PREFIX,
};
enum PPU_STATE {
    HBLANK, 
    VBLANK, 
    OAMSCAN, 
    DRAW
};
enum PPU_FIFO_STATE {
    FETCH_TILE, 
    FETCH_DATA_LOW, 
    FETCH_DATA_HIGH, 
    SLEEP, 
    PUSH
};

typedef struct {
    BYTE z;
    BYTE n;
    BYTE h;
    BYTE c;
    BYTE ime;
    BYTE wants_ime;
} CPUFlags;

typedef struct {
    BYTE a;
    reg bc;
    reg de;
    reg hl;
    WORD sp;
    WORD pc;
    CPUFlags flags;
    enum CPU_STATE state;
    BYTE opcode; // Opcode pending execution
    BYTE counter; // Pipeline stage counter
    BYTE *reg_dest; // Pointer to destination register
    BYTE *src; // Pointer to source data
    reg _data; // 8 or 16 bit data
    WORD addr; // 16 bit address to read/write from/to
    BYTE *code;
    void **pipeline; // List of function pointers
} CPUState;

typedef struct {
    void *next;
    int timer;
    void (*execute)(void *);
} CPUStep;

typedef struct {
    int timer;
    void (*execute)(void *);
} GBEvent;

typedef struct {
    enum PPU_STATE s;
    struct {
        enum PPU_FIFO_STATE s;
        BYTE data[16];
    } fifo_bg;
    struct {
        enum PPU_FIFO_STATE s;
        BYTE data[16];
    } fifo_obj;
} PPUState;


#define BLANK(n) BYTE __space_ ## n
typedef struct __attribute__ ((packed)) {
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
    BYTE stack[127];
    BYTE int_enable;
} IORegs;

typedef struct {
    unsigned long counter;
    CPUState *cpu;
    //void **cpu_pipeline;
    //CPUStep **cpu_pipeline;
    PPUState *ppu;
    //GBEvent **events;
    IORegs *io_regs;
    BYTE *code;
    WORD (*read_mem)(void*, WORD, int);
    int (*write_mem)(void*, WORD, WORD, int);
} GBState;

#define CPU_FREQ 4194304
#define M_CYCLE 1048576
#define CPU_PER_M 4
#define CPU_PERIOD_S  (float)(1.0 / (float)CPU_FREQ)
#define CPU_PERIOD_MS (float)(1000.0 / (float)CPU_FREQ)
#define CPU_PERIOD_US (float)(1000000.0 / (float)CPU_FREQ)

#define PPU_FREQ CPU_FREQ
#define PPU_PERIOD_S CPU_PERIOD_S
#define PPU_PERIOD_MS CPU_PERIOD_MS
#define PPU_PERIOD_US CPU_PERIOD_US
#define PPU_PER_SCANLINE 456
#define SCANLINE_PER_FRAME 154

#define HSYNC_FREQ 9198
#define HSYNC_PERIOD_S  (float)(1.0 / (float)HSYNC_FREQ)
#define HSYNC_PERIOD_MS (float)(1000.0 / (float)HSYNC_FREQ)
#define HSYNC_PERIOD_US (float)(1000000.0 / (float)HSYNC_FREQ)
#define CPU_PER_HSYNC (float)(HSYNC_PERIOD_US / CPU_PERIOD_US)

#define VSYNC_FREQ 59.73
#define VSYNC_PERIOD_S  (float)(1.0 / (float)VSYNC_FREQ)
#define VSYNC_PERIOD_MS (float)(1000.0 / (float)VSYNC_FREQ)
#define VSYNC_PERIOD_US (float)(1000000.0 / (float)VSYNC_FREQ)
#define CPU_PER_VSYNC (float)(VSYNC_PERIOD_US / CPU_PERIOD_US)
#define HSYNC_PER_VSYNC (float)(VSYNC_PERIOD_US / HSYNC_PERIOD_US)

#define DIV_REG_FREQ 16384
#define DIV_REG_PERIOD_S  (float)(1.0 / (float)DIV_REG_FREQ)
#define DIV_REG_PERIOD_MS (float)(1000.0 / (float)DIV_REG_FREQ)
#define DIV_REG_PERIOD_US (float)(1000000.0 / (float)DIV_REG_FREQ)
#define CPU_PER_DIV_REG (float)(DIV_REG_PERIOD_US / CPU_PERIOD_US)

#define BASE_FREQ CPU_FREQ
#define BASE_PERIOD_S CPU_PERIOD_S
#define BASE_PERIOD_MS CPU_PERIOD_MS
#define BASE_PERIOD_US CPU_PERIOD_US
#define BASE_PER_CPU 1
#define BASE_PER_PPU 1
#define BASE_PER_SCANLINE PPU_PER_SCANLINE
#define BASE_PER_HSYNC (BASE_PER_CPU * CPU_PER_HSYNC)
#define BASE_PER_VSYNC (BASE_PER_PPU * CPU_PER_VSYNC)
#define BASE_PER_M 4


#endif // GB_BASE