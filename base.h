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

enum CPU_STATE {
    PREINIT,
    READY,
    PREFIX,
    INTERRUPT,
    HALT,
    STOP
};
enum PPU_STATE {
    INIT,
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

enum CPU_FLAG {
    CLEAR=0,
    SET=1,
    CHECK,
    NOCHANGE,
    FLIP
};

typedef struct {
    enum CPU_FLAG z;
    enum CPU_FLAG n;
    enum CPU_FLAG h;
    enum CPU_FLAG c;
    enum CPU_FLAG ime;
    char wants_ime;
} CPUFlags;

typedef struct __attribute__ ((packed)) {
    BYTE f;
    BYTE a;
    BYTE c;
    BYTE b;
    BYTE e;
    BYTE d;
    BYTE l;
    BYTE h;
    WORD sp;
    WORD pc;
} CPURegs;

#define rr(__x) *(WORD *)__x
#define w(__x) ((WORD *)&__x)
#define w_v(__x) (*w(__x))

typedef struct __attribute__ ((packed)) {
    CPURegs r;
    CPUFlags flags;

    enum CPU_STATE state;
    BYTE changes_flags; // Flag to indicate check_flags should be examined
    CPUFlags check_flags;
    int result; // Result of operation, for flag checks
    BYTE is_16_bit; // Flag to indicate 16 bit store/load
    BYTE opcode; // Opcode pending execution
    BYTE *reg_dest; // Pointer to destination data
    BYTE *reg_src; // Pointer to source data
    BYTE data1; // 8 bit data (lsb)
    BYTE data2; // 8 bit data (msb)
    int8_t offset;
    WORD addr; // 16 bit address to read/write from/to
    BYTE counter; // Pipeline stage counter
    void (*pipeline[8]) (void *); // List of function pointers
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
    enum PPU_STATE state;
    struct {
        enum PPU_FIFO_STATE state;
        BYTE data[16];
    } fifo_bg;
    struct {
        enum PPU_FIFO_STATE state;
        BYTE data[16];
    } fifo_obj;
} PPUState;


/* Represents a contiguous region of address space for the GB.
Does not actually contain the data of this region; rather, the 
read and write functions are responsible for interacting with 
MBC/system memory state to map to the correct data.
Flags contains priority information as well as ownership/locks.
*/
typedef struct {
    char name[8];
    WORD base;
    WORD end;
    WORD len;
    BYTE flags; // owner / lock status? / priority
    BYTE (*read)(GBState *, WORD, BYTE);
    BYTE (*write)(GBState *, WORD, BYTE, BYTE);
} MemoryRegion;

/* Represents a generic memory mapping system on the GB.
regions is a pointer to an array of MemoryRegion's.
*/
typedef struct {
    char name[6];
    MemoryRegion *regions;
    int n_regions;
    void (*initialize)(void);
    void (*teardown)(void *);
    void *state;
} Memmap_t;

typedef struct {
    Memmap_t *system;
    Memmap_t *cartridge;
} MemoryState;

typedef struct {
    unsigned long counter;
    CPUState *cpu;
    PPUState *ppu;
    MemoryState *mem;
} GBState;

typedef struct {
    unsigned int period;
    void (*run_task)(GBState *);
} GBTask;

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

GBState *initialize_gb(void);
void teardown_gb(GBState *);

#endif // GB_BASE