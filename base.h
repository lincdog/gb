#ifndef GB_BASE
#define GB_BASE

#include <stdint.h>
#include <assert.h>
#ifdef NDEBUG
#   define assert(...)
#endif

#include <SDL.h>

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

typedef enum {
    INIT=-1,
    HBLANK=0, 
    VBLANK=1, 
    OAMSCAN=2, 
    DRAW=3
} PPUMode;
typedef enum {
    FETCH_TILE, 
    FETCH_DATA_LOW, 
    FETCH_DATA_HIGH, 
    SLEEP, 
    PUSH
} PPUFifoState;

typedef enum {OFF=0, ON=1} ToggleEnum;
typedef enum {DATA_AREA0=0x9000, DATA_AREA1=0x8000} TileDataArea;
typedef enum {MAP_AREA0=0x9800, MAP_AREA1=0x9C00} TileMapArea;
typedef enum {OBJ_8x8=8, OBJ_8x16=16} ObjectSize;
typedef struct {
    ToggleEnum lcd_enable;
    TileMapArea win_map_area;
    ToggleEnum window_enable;
    TileDataArea bg_win_data_area;
    TileMapArea bg_map_area;
    ObjectSize obj_size;
    ToggleEnum obj_enable;
    ToggleEnum bg_window_enable;
} LCDControl;

typedef struct {
    ToggleEnum lyc_ly_interrupt;
    ToggleEnum mode_2_interrupt;
    ToggleEnum mode_1_interrupt;
    ToggleEnum mode_0_interrupt;
    ToggleEnum lyc_ly_equal;
    PPUMode mode;
} LCDStatus;

typedef struct {
    BYTE scy;
    BYTE scx;
    BYTE ly;
    BYTE lyc;
    BYTE wy;
    BYTE wx;
    /* FIXME: SDL_Color types for each index?
     * 0: white (bg) or transparent (obj)
     * 1: light gray
     * 2: dark gray
     * 3: black
     */
    BYTE bgp;
    BYTE obp0;
    BYTE obp1;
} PPUMisc;

typedef struct {
    PPUFifoState state;
    struct {
        BYTE color;
        BYTE palette;
        BYTE sprite_prio;
        BYTE bg_prio;
    } data[16];
    int head;
    int counter;
} PPUFifo;

typedef struct __attribute__ ((packed)) {
    BYTE y;
    BYTE x;
    BYTE index;
    BYTE flags;
} OAMEntry;

typedef struct {
    WORD entry_addr;
    OAMEntry oam;
    BYTE lsb;
    BYTE msb;
} OAMRow_t;

typedef struct {
    int counter;
    int n_sprites_row;
    OAMRow_t current_row_sprites[10];
} OAMScan_t;

typedef struct {
    BYTE fetcher_x;
    BYTE fetcher_y;
    PPUFifo fifo_bg;
    PPUFifo fifo_obj;
} Drawing_t;

typedef struct {
    unsigned int counter;
    ToggleEnum in_window;
    int n_sprites_total;
    BYTE win_y;
} Frame_t;

/* Location for unpacking rows of pixels to be rendered. 

 - x_pos: the X coordinate in *screen* pixels that is currently being 
   fetched into the buffer. x_pos can go between 0 and 160. 
   For the BG, it always covers 0-160 fully. For the window, it starts at 
   max(misc.wx - 7, 0). For objects, it jumps around according to X coordinates
   in the OAM entries in the sorted list for this scanline.
 - offset: the start index in the buffer to use for rendering. This is used because
   the BG or window can be scrolled by individual pixels, but tiles are 8 pixels wide.
   The offset means that we can still copy the full first tile to the buffer, but only
   render its last few pixels, according to SCX or WX. 

*/
typedef struct {
    BYTE buf[176];
    BYTE x_pos;
    unsigned int offset;
} Pixelbuf_t;

typedef struct {
    unsigned int counter;
    BYTE priority[160];
    Pixelbuf_t obj;
    Pixelbuf_t bg;
    Pixelbuf_t win;
} Scanline_t;

typedef struct {
    LCDControl lcdc;
    LCDStatus stat;
    PPUMisc misc;
    Frame_t frame;
    Scanline_t scanline;
    unsigned int mode_counter;

    //BYTE *pixelbuf;
    //BYTE *win_pixelbuf;
    //BYTE *obj_pixelbuf;
    //BYTE *current_bg_tile;
    //BYTE *current_win_tile;
    //BYTE *current_obj_tile;
    
    Drawing_t draw;
    OAMScan_t oamscan;
} PPUState;


/* Represents a contiguous region of address space for the GB.
Does not actually contain the data of this region; rather, the 
read and write functions are responsible for interacting with 
MBC/system memory state to map to the correct data.
Flags contains priority information as well as ownership/locks.

Function pointers:
    Note: All function pointers take as args 1 and 2 a GBState pointer (here void *
    because GBState is not defined yet) and a WORD relative address 
    (= abs_addr - region.base), and a BYTE flags argument as the last argument. 
    write takes a BYTE data as its 3rd argument.
    
    These functions handle the internal details of accessing different memory regions
    on the game boy, which may represent different physical chips on the cartridge or system,
    and may have different access restrictions for the CPU and PPU, e.g. video RAM, or other
    connections to hardware like most of the IO registers at 0xFF00-0xFF7F and 0xFFFF.
    Some regions may interpret writes as special behaviors (e.g. memory bank switch) or ignore writes.

 - check_access: returns 1 if the caller can access the given memory address based on 
    the supplied BYTE flags argument, which contains a MEM_SOURCE_* constant.
    Of course, the caller must be honest about their source for this to work.
    Returns 0 if the caller cannot access the memory.
 - read: Reads the given (relative) memory address and returns the value.
 - write: Writes the given BYTE data to the given (relative) memory address. Returns
    status.
- get_ptr: Returns a *raw* pointer to the specified memory address, if possible. Some regions 
    may not be implemented as an actual BYTE array under the hood (mainly IO regs), and they
    can return NULL here, so beware if dereferencing. 
*/
typedef struct {
    char name[8];
    WORD base;
    WORD end;
    unsigned int len;
    BYTE flags; // owner / lock status? / priority
    int (*check_access)(void *, WORD, BYTE);
    BYTE (*read)(void *, WORD, BYTE);
    BYTE (*write)(void *, WORD, BYTE, BYTE);
    BYTE *(*get_ptr)(void *, WORD, BYTE);
} MemoryRegion;

/* Represents a generic memory mapping system on the GB.
regions is a pointer to an array of MemoryRegion's. The GB's total memory
consists of one Memmap_t for the system and one Memmap_t for the cartridge's
onboard memory, which may be of different types.
*/
typedef struct {
    //char name[6];
    MemoryRegion *regions;
    int n_regions;
    void (*initialize)(void);
    void (*teardown)(void *);
    void *state;
} Memmap_t;

typedef enum {
    BASIC,
    MBC1,
    MBC3,
    DEBUG
} MemInitFlag;

/* The total memory system of the Game Boy consists of a system memory
map and a cartridge memory map. The configuration of these is specified 
by a MemInitFlag mode. This is determined from the cartridge header. */
typedef struct {
    MemInitFlag mode;
    Memmap_t *system;
    Memmap_t *cartridge;
} MemoryState;

typedef struct {
    BYTE reg_div;
    BYTE reg_tima;
    BYTE reg_tma;
    BYTE reg_tac;
    ToggleEnum timer_enabled;
    enum {
        _00=1024,
        _01=16,
        _10=64,
        _11=256
    } tima_period_cycles;

} TimerState;

typedef struct {
    SDL_Event event;
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Surface *surface;
} SDLComponents;

/* The top-level Game Boy state structure. This is passed around to all the 
important functions. Contains pointers to the various subsystems. */
typedef struct {
    unsigned long counter;
    CPUState *cpu;
    PPUState *ppu;
    MemoryState *mem;
    TimerState *timer;
    SDLComponents *sdl;
    ToggleEnum should_quit;
} GBState;

/* The main loop executes calls for the various subsystems using these 
entries to determine their timing. */
typedef struct {
    unsigned int period;
    int mask;
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

GBState *initialize_gb(MemInitFlag);
void teardown_gb(GBState *);

#endif // GB_BASE