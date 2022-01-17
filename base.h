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
enum PPU_STATE {HBLANK, VBLANK, OAMSCAN, DRAW};
enum PPU_FIFO_STATE {FETCH_TILE, FETCH_DATA_LOW, FETCH_DATA_HIGH, SLEEP, PUSH};

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
    enum CPU_STATE s;
    BYTE *code;
} CPUState;

typedef struct {
    int timer;
    void (*execute)(void *);
} GBEvent;

typedef struct {
    enum PPU_STATE s;
    enum PPU_FIFO_STATE fs;
} PPUState;

typedef struct {
    CPUState *cpu_state;
    PPUState *ppu_state;
    GBEvent **events;
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