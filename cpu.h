#ifndef GB_CPU
#define GB_CPU
#include "base.h"
#include "mem.h"

CPUState *initialize_cpu(void);
void teardown_cpu(CPUState *);
void reset_registers(CPUState *);
void reset_pipeline(CPUState *);
void cpu_m_cycle(GBState *);
void cpu_setup_pipeline(GBState *, BYTE);
void cpu_setup_prefix_pipeline(GBState *, BYTE);
void print_state_info(GBState *, char);

#define CYCLE_FUNC(__name) static inline void __name(GBState *state)
/* IMPLEMENTED */
CYCLE_FUNC(_copy_reg);
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
CYCLE_FUNC(_read_mem_h_and_store_8);
CYCLE_FUNC(_read_mem_h_and_store_16);
CYCLE_FUNC(_decrement_sp);
CYCLE_FUNC(_xor_reg_reg);
CYCLE_FUNC(_and_reg_reg);
CYCLE_FUNC(_or_reg_reg);
CYCLE_FUNC(_adc_reg_reg);
CYCLE_FUNC(_sbc_reg_reg);
CYCLE_FUNC(_do_stop);
CYCLE_FUNC(_rr_reg);
CYCLE_FUNC(_rrc_reg);
CYCLE_FUNC(_rl_reg);
CYCLE_FUNC(_rlc_reg);

CYCLE_FUNC(_inc_hl);
CYCLE_FUNC(_dec_hl);
CYCLE_FUNC(_inc_sp);
CYCLE_FUNC(_dec_sp);
CYCLE_FUNC(_inc_bc);
CYCLE_FUNC(_dec_bc);
CYCLE_FUNC(_inc_de);
CYCLE_FUNC(_dec_de);
CYCLE_FUNC(_nop);
CYCLE_FUNC(_fetch_inst);
CYCLE_FUNC(_check_flags);
CYCLE_FUNC(_do_da);
CYCLE_FUNC(_do_cpl);
CYCLE_FUNC(_add_reg_signed_data);
CYCLE_FUNC(_set_addr_from_data_io);
CYCLE_FUNC(_write_reg16_to_stack);

/* NOT IMPLEMENTED */

CYCLE_FUNC(_write_mem_l_inc_dest);
CYCLE_FUNC(_write_mem_l_dec_dest);
CYCLE_FUNC(_write_data1_to_dest);

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
#define reg_flags(__cpu, __f) (__cpu)->flags.__f
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

#define cpu_write_mem(__state, __addr, __data) write_mem(__state, __addr, __data, MEM_SOURCE_CPU)
#define cpu_read_mem(__state, __addr) read_mem(__state, __addr, MEM_SOURCE_CPU)

/* instruction macros */
#define ILLEGAL_INST(__cpu, __inst) \
    printf("ILLEGAL INSTRUCTION " # __inst "\n"); \
    __cpu->pipeline[0] = &_nop;

// An operation between 2 8-bit registers
#define REG_OP(__cpu, __op, __dest, __src) \
    __cpu->reg_dest = &reg_ ## __dest(__cpu); \
    __cpu->reg_src = &reg_ ## __src(__cpu); \
    __cpu->pipeline[0] = &__op; 

// An operation between an 8 bit register and a byte from the memory addr (HL)
#define OP_REG_HL(__cpu, __op, __dest) \
    __cpu->reg_dest = &reg_ ## __dest(__cpu); \
    __cpu->reg_src = &cpu->data1; \
    __cpu->addr = reg_hl(__cpu); \
    __cpu->pipeline[0] = &_read_mem_l; \
    __cpu->pipeline[1] = &__op; 

// An operation between a byte at the memory addr (HL) and an 8-bit register
#define OP_HL_REG(__cpu, __op, __src) \
    __cpu->reg_dest = &cpu->data1; \
    __cpu->reg_src = &reg_ ## __src(__cpu); \
    __cpu->addr = reg_hl(__cpu); \
    __cpu->pipeline[0] = &_read_mem_l; \
    __cpu->pipeline[1] = &__op; \
    __cpu->pipeline[2] = &_write_mem_data1;

#define LD_REG(__cpu, __dest, __src) REG_OP(__cpu, _copy_reg, __dest, __src)

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
    OP_REG_HL(__cpu, _add_reg_reg, __dest) \
    CHECK_FLAGS(__cpu, CHECK, CLEAR, CHECK, CHECK);

#define ADD_8_REG(__cpu, __dest, __src) \
    REG_OP(__cpu, _add_reg_reg, __dest, __src); \
    CHECK_FLAGS(__cpu, CHECK, CLEAR, CHECK, CHECK);

#define ADC_8_HL(__cpu, __dest) \
    OP_REG_HL(__cpu, _adc_reg_reg, __dest) \
    CHECK_FLAGS(__cpu, CHECK, CLEAR, CHECK, CHECK);

#define ADC_8_REG(__cpu, __dest, __src) \
    REG_OP(__cpu, _adc_reg_reg, __dest, __src); \
    CHECK_FLAGS(__cpu, CHECK, CLEAR, CHECK, CHECK);

#define SUB_8_HL(__cpu, __dest) \
    OP_REG_HL(__cpu, _sub_reg_reg, __dest) \
    CHECK_FLAGS(__cpu, CHECK, SET, CHECK, CHECK);

#define SUB_8_REG(__cpu, __dest, __src) \
    REG_OP(__cpu, _sub_reg_reg, __dest, __src); \
    CHECK_FLAGS(__cpu, CHECK, SET, CHECK, CHECK);

#define SBC_8_HL(__cpu, __dest) \
    OP_REG_HL(__cpu, _sbc_reg_reg, __dest) \
    CHECK_FLAGS(__cpu, CHECK, SET, CHECK, CHECK);

#define SBC_8_REG(__cpu, __dest, __src) \
    REG_OP(__cpu, _sbc_reg_reg, __dest, __src); \
    CHECK_FLAGS(__cpu, CHECK, SET, CHECK, CHECK);

#define AND_8_HL(__cpu, __dest) \
    OP_REG_HL(__cpu, _and_reg_reg, __dest) \
    CHECK_FLAGS(__cpu, CHECK, CLEAR, SET, CLEAR);

#define AND_8_REG(__cpu, __dest, __src) \
    REG_OP(__cpu, _and_reg_reg, __dest, __src); \
    CHECK_FLAGS(__cpu, CHECK, CLEAR, SET, CLEAR); \

#define OR_8_HL(__cpu, __dest) \
    OP_REG_HL(__cpu, _or_reg_reg, __dest) \
    CHECK_FLAGS(__cpu, CHECK, CLEAR, CLEAR, CLEAR);

#define OR_8_REG(__cpu, __dest, __src) \
    REG_OP(__cpu, _or_reg_reg, __dest, __src); \
    CHECK_FLAGS(__cpu, CHECK, CLEAR, CLEAR, CLEAR);

#define XOR_8_HL(__cpu, __dest) \
    OP_REG_HL(__cpu, _xor_reg_reg, __dest) \
    CHECK_FLAGS(__cpu, CHECK, CLEAR, CLEAR, CLEAR);

#define XOR_8_REG(__cpu, __dest, __src) \
    REG_OP(__cpu, _xor_reg_reg, __dest, __src); \
    CHECK_FLAGS(__cpu, CHECK, CLEAR, CLEAR, CLEAR);

#define CP_8_HL(__cpu, __dest) \
    OP_REG_HL(__cpu, _cp_reg_reg, __dest) \
    CHECK_FLAGS(__cpu, CHECK, SET, CHECK, CHECK);

#define CP_8_REG(__cpu, __dest, __src) \
    REG_OP(__cpu, _cp_reg_reg, __dest, __src); \
    CHECK_FLAGS(__cpu, CHECK, SET, CHECK, CHECK); \

#define CONDITIONAL_CALL(__cpu, __condition) \
    __cpu->reg_src = &reg_pc(__cpu); \
    __cpu->reg_dest = &reg_pc(__cpu); \
    __cpu->is_16_bit = 1; \
    __cpu->pipeline[0] = &_read_imm_l; \
    __cpu->pipeline[1] = &_read_imm_h; \
    __cpu->pipeline[2] = &_nop; \
    if (__condition) { \
        __cpu->pipeline[3] = &_dec_sp_2; \
        __cpu->pipeline[4] = &_write_reg16_to_stack; \
        __cpu->pipeline[5] = &_write_reg_data; \
    }

#define RST(__cpu, __addr) \
    __cpu->reg_src = &reg_pc(__cpu); \
    __cpu->reg_dest = &reg_pc(__cpu); \
    __cpu->is_16_bit = 1; \
    __cpu->data1 = __addr; \
    __cpu->data2 = 0x0; \
    __cpu->pipeline[0] = &_nop; \
    __cpu->pipeline[1] = &_dec_sp_2; \
    __cpu->pipeline[2] = &_write_reg16_to_stack; \
    __cpu->pipeline[3] = &_write_reg_data;


#define CONDITIONAL_RET(__cpu, __condition) \
    __cpu->reg_dest = &reg_pc(cpu); \
    __cpu->addr = reg_sp(__cpu); \
    __cpu->is_16_bit = 1; \
    __cpu->pipeline[0] = &_nop; \
    if (__condition) { \
        __cpu->pipeline[1] = &_read_mem_l; \
        __cpu->pipeline[2] = &_read_mem_h; \
        __cpu->pipeline[3] = &_inc_sp_2; \
        __cpu->pipeline[4] = &_write_reg_data; \
    } else { \
        __cpu->pipeline[1] = &_nop; \
    }

#define COND_REL_JMP(__cpu, __condition) \
    __cpu->reg_dest = &reg_pc(__cpu); \
    __cpu->is_16_bit = 1; \
    __cpu->pipeline[0] = &_nop; \
    __cpu->pipeline[1] = &_read_imm_offset; \
    if (__condition) { \
        __cpu->pipeline[2] = &_add_reg_signed_data; \
    }

#define COND_ABS_JMP(__cpu, __condition) \
    __cpu->reg_dest = &reg_pc(__cpu); \
    __cpu->is_16_bit = 1; \
    __cpu->pipeline[0] = &_nop; \
    __cpu->pipeline[1] = &_read_imm_l; \
    __cpu->pipeline[2] = &_read_imm_h; \
    if (__condition) { \
        __cpu->pipeline[3] = &_write_reg_data; \
    }


#endif // GB_CPU