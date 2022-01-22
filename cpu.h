#ifndef GB_CPU
#define GB_CPU
#include "base.h"

void execute_prefix_inst(CPUState *, BYTE);
WORD execute_instruction(CPUState *, BYTE *);
CPUState *initialize_cpu(void);
void teardown_cpu(CPUState *);
void reset_registers(CPUState *);
void reset_pipeline(CPUState *);
void cpu_m_cycle(GBState *);
void cpu_setup_pipeline(GBState *, BYTE);
void print_state_info(GBState *, char);

#define CYCLE_FUNC(__name) static inline void __name(GBState *state)
/* IMPLEMENTED */
CYCLE_FUNC(_copy_reg);
CYCLE_FUNC(_add_reg_data1);
CYCLE_FUNC(_sub_reg_data1);
CYCLE_FUNC(_add_reg_reg);
CYCLE_FUNC(_sub_reg_reg);
CYCLE_FUNC(_inc_data1);
CYCLE_FUNC(_dec_data1);
CYCLE_FUNC(_write_reg_data);
CYCLE_FUNC(_write_mem_reg_l);
CYCLE_FUNC(_write_mem_reg_h);
CYCLE_FUNC(_write_mem_data1);
CYCLE_FUNC(_set_addr_from_data);
CYCLE_FUNC(_read_reg_l);
CYCLE_FUNC(_read_reg_h);
CYCLE_FUNC(_read_mem_l);
CYCLE_FUNC(_read_mem_h);
CYCLE_FUNC(_read_mem_h_and_store);
CYCLE_FUNC(_decrement_sp);

CYCLE_FUNC(_nop);
CYCLE_FUNC(_fetch_inst);
CYCLE_FUNC(_check_flags);

/* NOT IMPLEMENTED */
CYCLE_FUNC(_write_mem_l);
CYCLE_FUNC(_write_mem_h);
CYCLE_FUNC(_do_stop);
CYCLE_FUNC(_rr_reg);
CYCLE_FUNC(_rrc_reg);
CYCLE_FUNC(_rl_reg);
CYCLE_FUNC(_rlc_reg);
CYCLE_FUNC(_do_da);
CYCLE_FUNC(_write_reg_data_inc_src);
CYCLE_FUNC(_do_cpl);
CYCLE_FUNC(_add_reg_data_signed);
CYCLE_FUNC(_write_mem_l_inc_dest);
CYCLE_FUNC(_write_mem_l_dec_dest);
CYCLE_FUNC(_write_data1_to_dest);
CYCLE_FUNC(_set_addr_from_data_io);

static char GAMEBOY_LOGO[] = {
    0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
    0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E, 0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
    0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E
};

#define reg_a(__cpu) (__cpu)->r.a
#define reg_b(__cpu) (__cpu)->r.b
#define reg_c(__cpu) (__cpu)->r.c
#define reg_bc(__cpu) w_v((__cpu)->r.c)
#define reg_d(__cpu) (__cpu)->r.d
#define reg_e(__cpu) (__cpu)->r.e
#define reg_de(__cpu) w_v((__cpu)->r.e)
#define reg_h(__cpu) (__cpu)->r.h
#define reg_l(__cpu) (__cpu)->r.l
#define reg_hl(__cpu) w_v((__cpu)->r.l)
#define reg_sp(__cpu) (__cpu)->r.sp
#define reg_pc(__cpu) (__cpu)->r.pc
#define b2w(__lsb, __msb) ((WORD)__lsb | (((WORD)__msb)<<8))

#define CHECK_FLAGS(__cpu, __z, __n, __h, __c) \
    __cpu->changes_flags = 1; \
    __cpu->check_flags.z = __z; \
    __cpu->check_flags.n = __n; \
    __cpu->check_flags.h = __h; \
    __cpu->check_flags.c = __c;

#define check_half(a, b) ((((a)+(b)) & 0x1F) > 9)
#define check_carry_16(a, b) (((int)(a) + (int)(b)) > 0xFFFF)
#define check_carry_8(a, b) (((WORD)(a) + (WORD)(b)) > 0xFF)
#define check_sub(a, b) ((b) > (a));

BYTE flags_to_byte(CPUState *);
void set_flags_from_byte(CPUState *, BYTE);

/* NEW instruction macros */
#define ILLEGAL_INST(__cpu, __inst) \
    printf("ILLEGAL INSTRUCTION " # __inst "\n"); \
    __cpu->pipeline[0] = &_nop;


#define REG_OP(__cpu, __op, __dest, __src) \
    __cpu->reg_dest = &reg_ ## __dest(__cpu); \
    __cpu->reg_src = &reg_ ## __src(__cpu); \
    __cpu->pipeline[0] = &__op; 

#define OP_REG_HL(__cpu, __op, __dest) \
    __cpu->reg_dest = &reg_ ## __dest(__cpu); \
    __cpu->addr = reg_hl(__cpu); \
    __cpu->pipeline[0] = &_read_mem_l; \
    __cpu->pipeline[1] = &__op; 

#define OP_HL_REG(__cpu, __op, __src) \
    __cpu->reg_src = &reg_ ## __src(__cpu); \
    __cpu->addr = reg_hl(__cpu); \
    __cpu->pipeline[0] = &_nop; \
    __cpu->pipeline[1] = &__op;

#define LD_REG(__cpu, __dest, __src) REG_OP(__cpu, _copy_reg, __dest, __src)
/* \
    __cpu->reg_dest = &reg_ ## __dest(__cpu); \
    __cpu->reg_src = &reg_ ## __src(__cpu); \
    __cpu->pipeline[0] = &_copy_reg;
*/

#define LD_REG_HL(__cpu, __dest) \
    __cpu->reg_dest = &reg_ ## __dest(__cpu); \
    __cpu->addr = reg_hl(__cpu); \
    __cpu->pipeline[0] = &_read_mem_l; \
    __cpu->pipeline[1] = &_write_reg_data;

#define LD_HL_REG(__cpu, __src) \
    __cpu->reg_src = &reg_ ## __src(__cpu); \
    __cpu->addr = reg_hl(__cpu); \
    __cpu->pipeline[0] = &_nop; \
    __cpu->pipeline[1] = &_write_mem_reg_l;

#define ADD_8_HL(__cpu, __dest) \
    OP_REG_HL(__cpu, _add_reg_data1, __dest) \
    CHECK_FLAGS(__cpu, CHECK, CLEAR, CHECK, CHECK);

#define ADD_8_REG(__cpu, __src, __dest) \
    REG_OP(__cpu, _add_reg_reg, __src, __dest); \
    CHECK_FLAGS(__cpu, CHECK, CLEAR, CHECK, CHECK);

#define ADC_8_HL(__cpu, __dest) \
    OP_REG_HL(__cpu, _adc_reg_data1, __dest) \
    CHECK_FLAGS(__cpu, CHECK, CLEAR, CHECK, CHECK);

#define ADC_8_REG(__cpu, __src, __dest) \
    REG_OP(__cpu, _adc_reg_reg, __src, __dest); \
    CHECK_FLAGS(__cpu, CHECK, CLEAR, CHECK, CHECK);

#define SUB_8_HL(__cpu, __dest) \
    OP_REG_HL(__cpu, _sub_reg_data1, __dest) \
    CHECK_FLAGS(__cpu, CHECK, SET, CHECK, CHECK);

#define SUB_8_REG(__cpu, __src, __dest) \
    REG_OP(__cpu, _sub_reg_reg, __src, __dest); \
    CHECK_FLAGS(__cpu, CHECK, SET, CHECK, CHECK);

#define SBC_8_HL(__cpu, __dest) \
    OP_REG_HL(__cpu, _sbc_reg_data1, __dest) \
    CHECK_FLAGS(__cpu, CHECK, SET, CHECK, CHECK);

#define SBC_8_REG(__cpu, __src, __dest) \
    REG_OP(__cpu, _sbc_reg_reg, __src, __dest); \
    CHECK_FLAGS(__cpu, CHECK, SET, CHECK, CHECK);

#define AND_8_HL(__cpu, __dest) \
    OP_REG_HL(__cpu, _and_reg_data1, __dest) \
    CHECK_FLAGS(__cpu, CHECK, CLEAR, SET, CLEAR);

#define AND_8_REG(__cpu, __src, __dest) \
    REG_OP(__cpu, _and_reg_reg, __src, __dest); \
    CHECK_FLAGS(__cpu, CHECK, CLEAR, SET, CLEAR); \

#define OR_8_HL(__cpu, __dest) \
    OP_REG_HL(__cpu, _or_reg_data1, __dest) \
    CHECK_FLAGS(__cpu, CHECK, CLEAR, CLEAR, CLEAR);

#define OR_8_REG(__cpu, __src, __dest) \
    REG_OP(__cpu, _or_reg_reg, __src, __dest); \
    CHECK_FLAGS(__cpu, CHECK, CLEAR, CLEAR, CLEAR);

#define XOR_8_HL(__cpu, __dest) \
    OP_REG_HL(__cpu, _xor_reg_data1, __dest) \
    CHECK_FLAGS(__cpu, CHECK, CLEAR, CLEAR, CLEAR);

#define XOR_8_REG(__cpu, __src, __dest) \
    REG_OP(__cpu, _xor_reg_reg, __src, __dest); \
    CHECK_FLAGS(__cpu, CHECK, CLEAR, CLEAR, CLEAR);

#define ADD_8_HL(__cpu, __dest) \
    OP_REG_HL(__cpu, _cp_reg_data1, __dest) \
    CHECK_FLAGS(__cpu, CHECK, SET, CHECK, CHECK);

#define CP_8_REG(__cpu, __src, __dest) \
    REG_OP(__cpu, _cp_reg_reg, __src, __dest); \
    CHECK_FLAGS(__cpu, CHECK, SET, CHECK, CHECK); \

/* OLD Instruction macros 
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
                        
#define PUSH_16(data) state->r.sp -= 2; \
                      write_16(state->r.sp, state->code, data); \

#define PUSH_8(data) state->r.sp -= 1; \
                     write_8(state->r.sp, state->code, data); \

#define POP_8(dest) dest = read_8(state->r.sp, state->code); \
                    state->r.sp += 1;

#define POP_16(dest) dest = read_16(state->r.sp, state->code); \
                     state->r.sp += 2;

#define CALL(dest) PUSH_16(state->r.pc + 1); return dest; 

#define RET() POP_16(state->r.pc); return state->r.pc;

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

#define OP_ON_HL(OP, ...) scratch = read_8(dw_v(state->r.l), state->code); \
                    OP(scratch, __VA_ARGS__); \
                    write_8(dw_v(state->r.l), state->code, scratch); \

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


**** END OLD instruction macros */

#endif // GB_CPU