#ifndef GB_CPU
#define GB_CPU
#include "base.h"

void execute_prefix_inst(CPUState *, BYTE);
WORD execute_instruction(CPUState *, BYTE *);
CPUState *initialize_state(void);
void reset_registers(CPUState *);
void print_state_info(CPUState *, char);

static char GAMEBOY_LOGO[] = {
    0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
    0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E, 0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
    0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E
};

#define check_half(a, b) ((((a)+(b)) & 0x1F) > 9)
#define check_carry_16(a, b) (((int)(a) + (int)(b)) > 0xFFFF)
#define check_carry_8(a, b) (((WORD)(a) + (WORD)(b)) > 0xFF)
#define check_sub(a, b) ((b) > (a));

/* Instruction macros */
#define INC_8(r) r += 1; \
                state->flags.z = (r == 0); \
                state->flags.h = check_half(r, 0); \
                state->flags.n = 0; 

#define DEC_8(r) r -= 1; \
                state->flags.z = (r == 0); \
                state->flags.h = check_half(r, 0); \
                state->flags.n = 0;

#define ADD_16_REG(dest, src) \
    state->flags.c = check_carry_16(state->dest, state->src); \
    state->flags.h = check_half(state->dest, state->src); \
    state->flags.n = 0; state->dest += state->src;

#define LD_REG(dest, src) state->dest = state->src;
#define MEM_REG(dest, src) write_8(state->dest, state->code, state->src);

#define ADD_8(dest, src) state->flags.c = check_carry_8(state->dest, src); \
                        state->dest += src; \
                        state->flags.z = (state->dest == 0); \
                        state->flags.h = check_half(state->dest, 0); \
                        state->flags.n = 0;

#define ADD_8_REG(dest, rsrc) ADD_8(dest, state->rsrc)
#define ADC_8(dest, src) ADD_8(dest, src + state->flags.c)
#define ADC_8_REG(dest, rsrc) ADC_8(dest, state->rsrc)

#define SUB_8(dest, src) state->flags.c = check_carry_8(state->dest, -(src)); \
                        state->dest -= src; \
                        state->flags.z = (state->dest == 0); \
                        state->flags.h = check_half(state->dest, 0); \
                        state->flags.n = 1;
                        
#define SUB_8_REG(dest, rsrc) SUB_8(dest, state->rsrc)
#define SBC_8(dest, src) SUB_8(dest, src + state->flags.c)
#define SBC_8_REG(dest, rsrc) SBC_8(dest, state->rsrc)

#define AND_8(dest, src) state->dest &= src; \
                        state->flags.z = (state->dest == 0); \
                        state->flags.n = 0; \
                        state->flags.h = 1; \
                        state->flags.c = 0;

#define AND_8_REG(dest, rsrc) AND_8(dest, state->rsrc);

#define OR_8(dest, src) state->dest |= src; \
                        state->flags.z = (state->dest == 0); \
                        state->flags.n = 0; \
                        state->flags.h = 0; \
                        state->flags.c = 0;

#define OR_8_REG(dest, rsrc) OR_8(dest, state->rsrc);

#define XOR_8(dest, src) state->dest ^= src; \
                        state->flags.z = (state->dest == 0); \
                        state->flags.n = 0; \
                        state->flags.h = 0; \
                        state->flags.c = 0; \

#define XOR_8_REG(dest, rsrc) XOR_8(dest, state->rsrc);

#define CP_8(dest, src) offset = state->dest - src; \
                        state->flags.n = 1; \
                        state->flags.z = (offset == 0); \
                        state->flags.c = (offset < 0); \
                        state->flags.h = check_half(offset, 0);

#define CP_8_REG(dest, rsrc) CP_8(dest, state->rsrc);
                        
#define PUSH_16(data) state->sp -= 2; \
                      write_16(state->sp, state->code, data); \

#define PUSH_8(data) state->sp -= 1; \
                     write_8(state->sp, state->code, data); \

#define POP_8(dest) dest = read_8(state->sp, state->code); \
                    state->sp += 1;

#define POP_16(dest) dest = read_16(state->sp, state->code); \
                     state->sp += 2;

#define CALL(dest) PUSH_16(state->pc + 1); return dest; 

#define RET() POP_16(state->pc); return state->pc;

#define L_ROT(data, ...) state->flags.c = (bit_7(data)>>7); \
                    data = ((data << 1) | state->flags.c); \
                    state->flags.z = (data == 0); \
                    state->flags.n = 0; \
                    state->flags.h = 0;

#define L_ROT_CAR(data, ...) scratch = state->flags.c; \
                        state->flags.c = (bit_7(data)>>7); \
                        data = (data << 1) | scratch; \
                        state->flags.z = (data == 0); \
                        state->flags.n = 0; \
                        state->flags.h = 0;
                        
#define R_ROT(data, ...) state->flags.c = (bit_0(data)); \
                    data = ((data >> 1) | (state->flags.c << 7)); \
                    state->flags.z = (data == 0); \
                    state->flags.n = 0; \
                    state->flags.h = 0;
                    
#define R_ROT_CAR(data, ...) scratch = state->flags.c; \
                        state->flags.c = bit_0(data); \
                        data = ((data >> 1) | (scratch << 7)); \
                        state->flags.z = (data == 0); \
                        state->flags.n = 0; \
                        state->flags.h = 0;
                        

#define SET_BIT(data, bit, ...) data |= (1 << bit);
#define CLEAR_BIT(data, bit, ...) data &= (BYTE)(~(1 << bit));
#define TEST_BIT(data, bit, ...) state->flags.z = ((data & (1 << bit)) == 0); \
                            state->flags.n = 0; \
                            state->flags.h = 1;
                            
#define L_SHIFT_A(data, ...) state->flags.c = (bit_7(data)>>7); \
                        data = (BYTE)(data << 1); \
                        state->flags.z = (data == 0); \
                        state->flags.n = 0; \
                        state->flags.h = 0;

#define R_SHIFT_A(data, ...) state->flags.c = bit_0(data); \
                        data = (bit_7(data)) | (data >> 1); \
                        state->flags.z = (data == 0); \
                        state->flags.n = 0; \
                        state->flags.h = 0;

#define R_SHIFT_L(data, ...) state->flags.c = bit_0(data); \
                        data = data >> 1; \
                        state->flags.z = (data == 0); \
                        state->flags.n = 0; \
                        state->flags.h = 0;

#define SWAP(data, ...) data = (BYTE) (((BYTE)data<<4) | ((BYTE)data>>4)); \
                    state->flags.z = (data == 0); \
                    state->flags.n = 0; \
                    state->flags.h = 0; \
                    state->flags.c = 0;

#define OP_ON_HL(OP, ...) scratch = read_8(state->hl.dw, state->code); \
                    OP(scratch, __VA_ARGS__); \
                    write_8(state->hl.dw, state->code, scratch); \

#define ILLEGAL(inst) printf("Illegal instruction %x\n", inst); exit(1);


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


#define HANDLE_INTERRUPT(bit_index) if (state->flags.ime && \
                            (read_8(0xFFFF, state->code)&(1<<bit_index))) { \
                            scratch = read_8(0xFF0F, state->code); \
                            write_8(0xFF0F, state->code, scratch ^ (1 << bit_index)); \
                            state->flags.ime = 0; state->flags.wants_ime = 0; \
                            CALL(irq_addrs[bit_index]); }



#endif // GB_CPU