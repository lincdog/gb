#ifndef GB_MACROS
#define GB_MACROS

#define WORD uint16_t
#define BYTE uint8_t
#define START_ADDR 0x100

static char GAMEBOY_LOGO[] = {
    0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
    0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E, 0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
    0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E
};

typedef union {
    struct {
        BYTE l;
        BYTE h;
    } w;
    WORD dw;
} reg;

enum EXEC_STATE {READY, IM_8, IM_16_LSB, IM_16_MSB, CB};

typedef struct GBFlags {
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
    struct GBFlags flags;
    
    BYTE *code;
} GBState;

/* Function prototypes */
BYTE read_8(WORD addr, BYTE *code);
WORD read_16(WORD addr, BYTE *code);
int write_8(WORD addr, BYTE *code, BYTE data);
int write_16(WORD addr, BYTE *code, WORD data);

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

#define check_half(a, b) ((((a)+(b)) & 0x1F) > 9)
#define check_carry_16(a, b) (((int)(a) + (int)(b)) > 0xFFFF)
#define check_carry_8(a, b) (((WORD)(a) + (WORD)(b)) > 0xFF)
#define check_sub(a, b) ((b) > (a));

/* Instruction macros */
#define INC_8(r) r += 1; if (r == 0) state->flags.z = 1; \
                if (check_half(r, 0)) state->flags.h = 1; \
                state->flags.n = 0; \
                break;
#define DEC_8(r) r -= 1; if (r == 0) state->flags.z = 1; \
                if (check_half(r, 0)) state->flags.h = 1; \
                state->flags.n = 0; \
                break;
#define ADD_16_REG(dest, src) \
    if (check_carry_16(state->hl.dw, state->bc.dw)) state->flags.c = 1; \
    if (check_half(state->hl.dw, state->bc.dw)) state->flags.h = 1; \
    state->flags.n = 0; state->hl.dw += state->bc.dw; \
    break;

#define LD_REG(dest, src) state->dest = state->src; break;
#define MEM_REG(dest, src) write_8(state->dest, state->code, state->src); break;

#define ADD_8(dest, src) if (check_carry_8(state->dest, src)) state->flags.c = 1; \
                        state->dest += src; \
                        if (state->dest == 0) state->flags.z = 1; \
                        if (check_half(state->dest, 0)) state->flags.h = 1; \
                        state->flags.n = 0; \
                        break;
#define ADD_8_REG(dest, rsrc) ADD_8(dest, state->rsrc)
#define ADC_8(dest, src) ADD_8(dest, src + state->flags.c)
#define ADC_8_REG(dest, rsrc) ADC_8(dest, state->rsrc)

#define SUB_8(dest, src) if (check_carry_8(state->dest, -(src))) state->flags.c = 1; \
                        state->dest -= src; \
                        if (state->dest == 0) state->flags.z = 1; \
                        if (check_half(state->dest, 0)) state->flags.h = 1; \
                        state->flags.n = 1; \
                        break;
#define SUB_8_REG(dest, rsrc) SUB_8(dest, state->rsrc)
#define SBC_8(dest, src) SUB_8(dest, src + state->flags.c)
#define SBC_8_REG(dest, rsrc) SBC_8(dest, state->rsrc)

#define AND_8(dest, src) state->dest &= src; \
                        if (state->dest == 0) state->flags.z = 1; \
                        state->flags.n = 0; \
                        state->flags.h = 1; \
                        state->flags.c = 0; \
                        break;
#define AND_8_REG(dest, rsrc) AND_8(dest, state->rsrc);

#define OR_8(dest, src) state->dest |= src; \
                        if (state->dest == 0) state->flags.z = 1; \
                        state->flags.n = 0; \
                        state->flags.h = 0; \
                        state->flags.c = 0; \
                        break;
#define OR_8_REG(dest, rsrc) OR_8(dest, state->rsrc);

#define XOR_8(dest, src) state->dest ^= src; \
                        if (state->dest == 0) state->flags.z = 1; \
                        state->flags.n = 0; \
                        state->flags.h = 0; \
                        state->flags.c = 0; \
                        break;
#define XOR_8_REG(dest, rsrc) XOR_8(dest, state->rsrc);

#define CP_8(dest, src) offset = state->dest - src; \
                        state->flags.n = 1; \
                        if (offset == 0) state->flags.z = 1; \
                        if (offset < 0) state->flags.c = 1; \
                        if (check_half(offset, 0)) state->flags.h = 1; \
                        break;
#define CP_8_REG(dest, rsrc) CP_8(dest, state->rsrc);
                        
#define PUSH_16(data) state->sp -= 2; \
                      write_16(state->sp, state->code, data); \
                      break;
#define PUSH_8(data) state->sp -= 1; \
                     write_8(state->sp, state->code, data); \
                     break;

#define POP_8(dest) dest = read_8(state->sp, state->code); \
                    state->sp += 1; \
                    break;

#define POP_16(dest) dest = read_16(state->sp, state->code); \
                     state->sp += 2; \
                     break;

#define CALL(dest) PUSH_16(state->pc + 1); state->pc = dest; \
                   break;
#define RET() POP_16(state->pc); break;

#define L_ROT(data, ...) state->flags.c = (bit_7(data)>>7); \
                    data = ((data << 1) | state->flags.c); \
                    if (data == 0) state->flags.z = 1; \
                    state->flags.n = 0; \
                    state->flags.h = 0; \
                    

#define L_ROT_CAR(data, ...) scratch = state->flags.c; \
                        state->flags.c = (bit_7(data)>>7); \
                        data = (data << 1) | scratch; \
                        if (data == 0) state->flags.z = 1; \
                        state->flags.n = 0; \
                        state->flags.h = 0; \
                        
#define R_ROT(data, ...) state->flags.c = (bit_0(data)); \
                    data = ((data >> 1) | (state->flags.c << 7)); \
                    if (data == 0) state->flags.z = 1; \
                    state->flags.n = 0; \
                    state->flags.h = 0; \
                    
#define R_ROT_CAR(data, ...) scratch = state->flags.c; \
                        state->flags.c = bit_0(data); \
                        data = ((data >> 1) | (scratch << 7)); \
                        if (data == 0) state->flags.z = 1; \
                        state->flags.n = 0; \
                        state->flags.h = 0; \
                        

#define SET_BIT(data, bit, ...) data |= (1 << bit); break;
#define CLEAR_BIT(data, bit, ...) data &= (BYTE)(~(1 << bit)); break;
#define TEST_BIT(data, bit, ...) if ((data & (1 << bit)) == 0) state->flags.z = 1; \
                            state->flags.n = 0; \
                            state->flags.h = 1; \
                            

#define L_SHIFT_A(data, ...) state->flags.c = (bit_7(data)>>7); \
                        data = (BYTE)(data << 1); \
                        if (data == 0) state->flags.z = 1; \
                        state->flags.n = 0; \
                        state->flags.h = 0; \

#define R_SHIFT_A(data, ...) state->flags.c = bit_0(data); \
                        data = (bit_7(data)) | (data >> 1); \
                        if (data == 0) state->flags.z = 1; \
                        state->flags.n = 0; \
                        state->flags.h = 0; \

#define R_SHIFT_L(data, ...) state->flags.c = bit_0(data); \
                        data = data >> 1; \
                        if (data == 0) state->flags.z = 1; \
                        state->flags.n = 0; \
                        state->flags.h = 0; \

#define SWAP(data, ...) data = (BYTE) (((BYTE)data<<4) | ((BYTE)data>>4)); \
                    if (data == 0) state->flags.z = 1; \
                    state->flags.n = 0; \
                    state->flags.h = 0; \
                    state->flags.c = 0; \

#define OP_ON_HL(OP, ...) scratch = read_8(state->hl.dw, state->code); \
                    OP(scratch, __VA_ARGS__); \
                    write_8(state->hl.dw, state->code, scratch); \
                    break;

#define ILLEGAL(inst) printf("Illegal instruction %x\n", inst); exit(1); break;


#define PREFIX_LEFT_EVEN_SWITCH(opcode, target) \
    switch (ms_nib(opcode)) { \
        case 0x0: L_ROT(target); break; \
        case 0x1: L_ROT_CAR(target); break;\
        case 0x2: L_SHIFT_A(target); break;\
        case 0x3: SWAP(target); break;\
        case 0x4: TEST_BIT(target, 0); break;\
        case 0x5: TEST_BIT(target, 2); break;\
        case 0x6: TEST_BIT(target, 4); break;\
        case 0x7: TEST_BIT(target, 6); break;\
        case 0x8: CLEAR_BIT(target, 0); break;\
        case 0x9: CLEAR_BIT(target, 2); break;\
        case 0xA: CLEAR_BIT(target, 4); break;\
        case 0xB: CLEAR_BIT(target, 6); break;\
        case 0xC: SET_BIT(target, 0); break;\
        case 0xD: SET_BIT(target, 2); break;\
        case 0xE: SET_BIT(target, 4); break;\
        case 0xF: SET_BIT(target, 6) break;\
    }

#define PREFIX_RIGHT_ODD_SWITCH(opcode, target) \
    switch (ms_nib(opcode)) { \
        case 0x0: R_ROT(target); break;\
        case 0x1: R_ROT_CAR(target); break;\
        case 0x2: R_SHIFT_A(target); break;\
        case 0x3: R_SHIFT_L(target); break;\
        case 0x4: TEST_BIT(target, 1); break;\
        case 0x5: TEST_BIT(target, 3); break;\
        case 0x6: TEST_BIT(target, 5); break;\
        case 0x7: TEST_BIT(target, 7); break;\
        case 0x8: CLEAR_BIT(target, 1); break;\
        case 0x9: CLEAR_BIT(target, 3); break;\
        case 0xA: CLEAR_BIT(target, 5); break;\
        case 0xB: CLEAR_BIT(target, 7); break;\
        case 0xC: SET_BIT(target, 1); break;\
        case 0xD: SET_BIT(target, 3); break;\
        case 0xE: SET_BIT(target, 5); break;\
        case 0xF: SET_BIT(target, 7) break;\
    }

#endif // GB_MACROS
  