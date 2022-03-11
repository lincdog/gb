#ifndef GB_BASE
#define GB_BASE

#include <stdint.h>
#include <assert.h>

int _debug_dummy(void);

#ifdef NDEBUG
#   define assert(...)
#   define DEBUGHOOK(__cond)
#else
#   define DEBUGHOOK(__cond) if (__cond) {  }
#endif

#include <SDL.h>

#define WORD uint16_t
#define BYTE uint8_t
#define START_ADDR 0x100

/* Utility macros */
#define ms_nib(x) (((x) & 0xF0)>>4)
#define ls_nib(x) (((x) & 0x0F))

#define set_bit_7(x) ((x) |= 0x80)
#define set_bit_6(x) ((x) |= 0x40)
#define set_bit_5(x) ((x) |= 0x20)
#define set_bit_4(x) ((x) |= 0x10)
#define set_bit_3(x) ((x) |= 0x08)
#define set_bit_2(x) ((x) |= 0x04)
#define set_bit_1(x) ((x) |= 0x02)
#define set_bit_0(x) ((x) |= 0x01)
#define res_bit_7(x) ((x) &= 0x7F)
#define res_bit_6(x) ((x) &= 0xBF)
#define res_bit_5(x) ((x) &= 0xDF)
#define res_bit_4(x) ((x) &= 0xEF)
#define res_bit_3(x) ((x) &= 0xF7)
#define res_bit_2(x) ((x) &= 0xFB)
#define res_bit_1(x) ((x) &= 0xFD)
#define res_bit_0(x) ((x) &= 0xFE)

#define bit_7(x) ((x) & 0x80)
#define bit_6(x) ((x) & 0x40)
#define bit_5(x) ((x) & 0x20)
#define bit_4(x) ((x) & 0x10)
#define bit_3(x) ((x) & 0x08)
#define bit_2(x) ((x) & 0x04)
#define bit_1(x) ((x) & 0x02)
#define bit_0(x) ((x) & 0x01)

#define get_le_word(ptr) (WORD)(ptr[1] | (ptr[2]<<8))

#define MEM_SOURCE_INTERRUPT 4
#define INT_FLAG_ADDR 0xFF0F
#define INT_VBLANK 0x1
#define INT_STAT 0x2
#define INT_TIMER 0x4
#define INT_SERIAL 0x8
#define INT_JOYPAD 0x10

#define KEY_MAP_DOWN SDL_SCANCODE_S
#define KEY_MAP_UP SDL_SCANCODE_W
#define KEY_MAP_LEFT SDL_SCANCODE_A
#define KEY_MAP_RIGHT SDL_SCANCODE_D
#define KEY_MAP_A SDL_SCANCODE_PERIOD
#define KEY_MAP_B SDL_SCANCODE_SLASH
#define KEY_MAP_SELECT SDL_SCANCODE_Z
#define KEY_MAP_START SDL_SCANCODE_SPACE

#define joypad_action_selected(x) (!(bit_5(x)))
#define joypad_direction_selected(x) (!(bit_4(x)))

#define joypad_down_press(x) res_bit_3(x)
#define joypad_down_release(x) set_bit_3(x)
#define joypad_start_press(x) res_bit_3(x)
#define joypad_start_release(x) set_bit_3(x)
#define joypad_up_press(x) res_bit_2(x)
#define joypad_up_release(x) set_bit_2(x)
#define joypad_select_press(x) res_bit_2(x)
#define joypad_select_release(x) set_bit_2(x)
#define joypad_left_press(x) res_bit_1(x)
#define joypad_left_release(x) set_bit_1(x)
#define joypad_b_press(x) res_bit_1(x)
#define joypad_b_release(x) set_bit_1(x)
#define joypad_right_press(x) res_bit_0(x)
#define joypad_right_release(x) set_bit_0(x)
#define joypad_a_press(x) res_bit_0(x)
#define joypad_a_release(x) set_bit_0(x)


#define REQUEST_INTERRUPT(__state, __bit) \
    __state->cpu->int_flag |= __bit;

#define CPU_STATE_FETCH 0x1
#define CPU_STATE_EXECUTE 0x2
#define CPU_STATE_PREFIX 0x4
#define CPU_STATE_BRANCH 0x8
#define CPU_STATE_INTERRUPT 0x10
#define CPU_STATE_HALT 0x40
#define CPU_STATE_STOP 0x80
enum CPU_STATE {
    PREINIT,
    READY,
    READY_PREFIX,
    EXECUTE,
    BRANCH,
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

typedef enum {STABLE=-1, OFF=0, ON=1} ToggleEnum;

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
    CPURegs r; // registers
    CPUFlags flags; // flags

    enum CPU_STATE state;
    BYTE changes_flags; // Flag to indicate check_flags should be examined
    CPUFlags check_flags;
    ToggleEnum branch_taken; // Flag to indicate branch 
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
    BYTE int_enable;
    BYTE int_flag;
} CPUState;

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
    LCDControl lcdc; // 0xFF40: LCD Control
    LCDStatus stat; // 0xFF41: LCD Status
    PPUMisc misc; // 0xFF42-0xFF4B: Scrolling, palettes, LY,  LY compare
    Frame_t frame; // Per-frame data structure
    Scanline_t scanline; // Per-scanline data structure
    OAMScan_t oamscan; // Per-scanline OAM scan data structure
    unsigned int mode_counter; // Counts down to the next mode switch
    ToggleEnum stat_interrupt;
    Drawing_t draw;
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

typedef struct {
    enum {DMA_OFF, DMA_INIT, DMA_ON} status;
    WORD addr;
} DMAState;

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
    BYTE button_select;
    BYTE action_buttons;
    BYTE direction_buttons;
    SDL_Event event;
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Surface *surface;
} SDLComponents;

typedef enum {DECREASE=0, INCREASE=1} DirEnum;

typedef struct {
    BYTE sweep_time; // FF10 bit 4-6
    DirEnum sweep_dir; // FF10 bit 3
    BYTE sweep_shift; // FF10 bit 0-2
    BYTE wave_duty; // FF11 bit 6-7
    BYTE sound_length; // FF11 bit 0-5
    BYTE env_init; // FF12 bit 4-7
    DirEnum env_dir; // FF12 bit 3
    BYTE n_sweeps; // FF12 bit 0-2
    WORD frequency; // FF13 | ((FF14 & 0x7) << 8) - 11 bits
    ToggleEnum stop_counter;
    ToggleEnum init;
} APUChannel1;

typedef struct {
    BYTE wave_duty; // FF16 bit 6-7
    BYTE sound_length; // FF16 bit 0-5
    BYTE env_init; // FF17 bits 4-7
    DirEnum env_dir; // FF17 bit 3
    BYTE n_sweeps; // FF17 bits 0-2
    WORD frequency; // FF18 | ((FF19 & 0x7) << 8)
    ToggleEnum stop_counter;
    ToggleEnum init;
} APUChannel2;
 
typedef struct {
    ToggleEnum on_off; // FF1A bit 7
    BYTE sound_length; // FF1B
    BYTE volume; // FF1C bits 5-6
    WORD frequency; // FF1D | ((FF1E & 0x7) << 8) - 11 bits
    ToggleEnum stop_counter;
    ToggleEnum init;
    BYTE wave_ram[32]; // FF30-FF3F (32 * 4 bit samples)
 } APUChannel3;

typedef enum {Step_15=0x7FFF, Step_7=0x7F} Ch4StepEnum;
typedef struct {
    BYTE sound_length; // FF20 bit 0-5
    BYTE env_init; // FF21 bit 4-7
    DirEnum env_dir; // FF21 bit 3
    BYTE n_sweeps; // FF21 bit 0-2
    BYTE shift_frequency; // FF22 bit 4-7
    Ch4StepEnum shift_step; // FF22 bit 3
    BYTE shift_ratio; // FF22 bit 0-2
    ToggleEnum stop_counter;
    ToggleEnum init;
} APUChannel4;

typedef struct {
    ToggleEnum vin_so2; // FF24 bit 7
    BYTE vol_so2; // FF24 bit 4-6
    ToggleEnum vin_so1; // FF24 bit 3
    BYTE vol_so1; // FF24 bit 0-2
} MasterControl;

// FF25
typedef struct {
    ToggleEnum ch4_so2;
    ToggleEnum ch3_so2;
    ToggleEnum ch2_so2;
    ToggleEnum ch1_so2;
    ToggleEnum ch4_so1;
    ToggleEnum ch3_so1;
    ToggleEnum ch2_so1;
    ToggleEnum ch1_so1;
} OutputControl;

// FF26
typedef struct {
    ToggleEnum sound_on; // bit 7
    ToggleEnum ch4_flag; // bit 3
    ToggleEnum ch3_flag; // bit 2
    ToggleEnum ch2_flag; // bit 1
    ToggleEnum ch1_flag; // bit 0
} SoundSwitch;

typedef struct {
    APUChannel1 ch1;
    APUChannel2 ch2;
    APUChannel3 ch3;
    APUChannel4 ch4;
    MasterControl master;
    OutputControl output;
    SoundSwitch sound_switch;
} APUState;

/* The top-level Game Boy state structure. This is passed around to all the 
important functions. Contains pointers to the various subsystems. */
typedef struct {
    unsigned long counter;
    CPUState *cpu;
    PPUState *ppu;
    MemoryState *mem;
    DMAState *dma;
    TimerState *timer;
    SDLComponents *sdl;
    ToggleEnum should_quit;
} GBState;

/* The main loop executes calls for the various subsystems using these 
entries to determine their timing. */
typedef struct {
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