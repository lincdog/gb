#ifndef GB_BASE
#define GB_BASE

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

enum EXEC_STATE {READY, IM_8, IM_16_LSB, IM_16_MSB, CB};

typedef struct {
    BYTE z;
    BYTE n;
    BYTE h;
    BYTE c;
    BYTE ime;
    BYTE wants_ime;
} GBFlags;

typedef struct {
    BYTE a;
    reg bc;
    reg de;
    reg hl;
    WORD sp;
    WORD pc;
    GBFlags flags;
    
    BYTE *code;
} GBState;

void print_state_info(GBState *, char);

#endif // GB_BASE