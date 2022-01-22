#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "base.h"
#include "mem.h"
#include "cpu.h"

const int TIMA_FREQS[] = {
    CPU_FREQ >> 10,
    CPU_FREQ >> 4,
    CPU_FREQ >> 6,
    CPU_FREQ >> 8
};
const int TIMA_PERIODS_CPUS[] = {
    1024,
    16,
    64,
    256
};

#define IRQ_VBLANK(x) (x & 0x1)
#define IRQ_LCDSTAT(x) (x & 0x2)
#define IRQ_TIMER(x) (x & 0x4)
#define IRQ_SERIAL(x) (x & 0x8)
#define IRQ_JOYPAD(x) (x & 0x10)

const WORD irq_addrs[] = {
    0x40, 0x48, 0x50, 0x58, 0x60
};

void reset_registers(CPUState *cpu) {
    reg_sp(cpu) = 0xFFFE;
    reg_pc(cpu) = 0;

    reg_a(cpu) = 1;
    reg_b(cpu) = 0;
    reg_c(cpu) = 0x13;
    reg_d(cpu) = 0;
    reg_e(cpu) = 0xD8;
    reg_h(cpu) = 0x01;
    reg_l(cpu) = 0x4D;

    cpu->flags.z = 0;
    cpu->flags.n = 0;
    cpu->flags.h = 0;
    cpu->flags.z = 0;
    cpu->flags.ime = 0;
    cpu->flags.wants_ime = 0;

}

void reset_pipeline(CPUState *cpu) {
    //cpu->state = READY;
    cpu->check_flags = (CPUFlags){
        .z=NOCHANGE, 
        .n=NOCHANGE, 
        .h=NOCHANGE, 
        .c=NOCHANGE, 
        .ime=NOCHANGE, 
        .wants_ime=0
    };
    cpu->result = 0;
    cpu->is_16_bit = 0;
    cpu->opcode = 0;

    cpu->reg_dest = NULL;
    cpu->reg_src = NULL;
    cpu->data1 = 0;
    cpu->data2 = 0;
    cpu->offset = 0;

    cpu->addr = reg_pc(cpu);
    cpu->counter = 0;
    
    for (int i = 0; i < 8; i++) {
        cpu->pipeline[i] = NULL;
    }
}

CPUState *initialize_cpu(void) {
    CPUState *cpu = malloc(sizeof(CPUState));
    reset_registers(cpu);
    reset_pipeline(cpu);
    cpu->state = PREINIT;
        
    return cpu;
}

void teardown_cpu(CPUState *cpu) {
    free(cpu);
}

void print_state_info(GBState *state, char print_io_reg) {
    CPUState *cpu = state->cpu;
    printf("pc: %02x \ninst: %02x \nsp: %02x\n", 
        reg_pc(cpu), cpu->opcode, reg_sp(cpu));
    printf("\ta: %02x \n\tb: %02x \n\tc: %02x\n", 
        reg_a(cpu), reg_b(cpu), reg_c(cpu));
    printf("\td: %02x \n\te: %02x\n",
        reg_d(cpu), reg_e(cpu));
    printf("\th: %02x \n\tl: %02x\n",
        reg_h(cpu), reg_l(cpu));
    printf("z n h c ime\n");
    printf("%d %d %d %d %d\n", 
        cpu->flags.z, cpu->flags.n, 
        cpu->flags.h, cpu->flags.c,
        cpu->flags.ime
        );
    
    if (print_io_reg) {
        for (int i = 0xFF00; i <= 0xFFFF; i++) {
            if ((i & 0xFF)%16 == 0) {
                printf("\n%x\t", i);
            }
            printf("%02x ", state->code[i]);
        }
        printf("\n");
    }
    
}

/*
 - write: (reg)pointer dest or (r.addr)mem dest; byte data
 - read: addr/pointer dest; addr/pointer src
 - check condition
 - nop

*/

/* 8-bit arithmetic and bitwise operations */
CYCLE_FUNC(_do_cpl) {
    reg_a(state->cpu) = (BYTE)(reg_a(state->cpu) ^ 0xFF);
    _check_flags(state);
}
CYCLE_FUNC(_add_reg_reg) {
    state->cpu->result = *(state->cpu->reg_dest) + *(state->cpu->reg_src);
    _check_flags(state);
    *(state->cpu->reg_dest) += *(state->cpu->reg_src);
}
CYCLE_FUNC(_sub_reg_reg) {
    state->cpu->result = *(state->cpu->reg_dest) - *(state->cpu->reg_src);
    _check_flags(state);
    *(state->cpu->reg_dest) -= *(state->cpu->reg_src);
}
CYCLE_FUNC(_adc_reg_reg) {
    int carry = (state->cpu->flags.c == SET) ? 1 : 0;
    state->cpu->result = *(state->cpu->reg_dest) + *(state->cpu->reg_src) + carry;
    _check_flags(state);
    *(state->cpu->reg_dest) += (*(state->cpu->reg_src) + carry);
}
CYCLE_FUNC(_sbc_reg_reg) {
    int carry = (state->cpu->flags.c == SET) ? 1 : 0;
    state->cpu->result = *(state->cpu->reg_dest) - *(state->cpu->reg_src) - carry;
    _check_flags(state);
    *(state->cpu->reg_dest) -= (*(state->cpu->reg_src) + carry);
}
CYCLE_FUNC(_and_reg_reg) {
    state->cpu->result = *(state->cpu->reg_dest) & *(state->cpu->reg_src);
    _check_flags(state);
    *(state->cpu->reg_dest) &= *(state->cpu->reg_src);
}
CYCLE_FUNC(_or_reg_reg) {
    state->cpu->result = *(state->cpu->reg_dest) | *(state->cpu->reg_src);
    _check_flags(state);
    *(state->cpu->reg_dest) |= *(state->cpu->reg_src);
}
CYCLE_FUNC(_xor_reg_reg) {
    state->cpu->result = *(state->cpu->reg_dest) ^ *(state->cpu->reg_src);
    _check_flags(state);
    *(state->cpu->reg_dest) ^= *(state->cpu->reg_src);
}
CYCLE_FUNC(_cp_reg_reg) {
    state->cpu->result = *(state->cpu->reg_dest) - *(state->cpu->reg_src);
    _check_flags(state);
}
CYCLE_FUNC(_rlc_reg) {
    BYTE bit7 = bit_7(*(state->cpu->reg_dest)) >> 7;
    state->cpu->result = (*(state->cpu->reg_dest)<<1) | bit7;
    if (bit7)
        state->cpu->check_flags.c = SET;
    else
        state->cpu->check_flags.c = CLEAR;
    _check_flags(state);
    *(state->cpu->reg_dest) = (BYTE)state->cpu->result;
}
CYCLE_FUNC(_rl_reg) {
    BYTE carry = (state->cpu->flags.c == SET) ? 1 : 0;
    BYTE bit7 = bit_7(*(state->cpu->reg_dest)) >> 7;
    state->cpu->result = (*(state->cpu->reg_dest)<<1) | carry;
    if (bit7)
        state->cpu->check_flags.c = SET;
    else
        state->cpu->check_flags.c = CLEAR;
    _check_flags(state);
    *(state->cpu->reg_dest) = (BYTE)state->cpu->result;
}
CYCLE_FUNC(_rrc_reg) {
    BYTE bit0 = bit_0(*(state->cpu->reg_dest));
    state->cpu->result = (*(state->cpu->reg_dest)>>1) | (bit0<<7);
    if (bit0)
        state->cpu->check_flags.c = SET;
    else
        state->cpu->check_flags.c = CLEAR;
    _check_flags(state);
    *(state->cpu->reg_dest) = (BYTE)state->cpu->result;
}
CYCLE_FUNC(_rr_reg) {
    BYTE carry = (state->cpu->flags.c == SET) ? 1 : 0;
    BYTE bit0 = bit_0(*(state->cpu->reg_dest));
    state->cpu->result = (*(state->cpu->reg_dest)>>1) | (carry<<7);
    if (bit0)
        state->cpu->check_flags.c = SET;
    else
        state->cpu->check_flags.c = CLEAR;
    _check_flags(state);
    *(state->cpu->reg_dest) = (BYTE)state->cpu->result;
}
CYCLE_FUNC(_sla_reg) {
    BYTE bit7 = bit_7(*(state->cpu->reg_dest)) >> 7;
    state->cpu->result = *(state->cpu->reg_dest)<<1;
    if (bit7)
        state->cpu->check_flags.c = SET;
    else
        state->cpu->check_flags.c = CLEAR;
    _check_flags(state);
    *(state->cpu->reg_dest) = (BYTE)state->cpu->result;
}
CYCLE_FUNC(_swap_reg) {
    BYTE lsn = ls_nib(*(state->cpu->reg_dest));
    BYTE msn = ms_nib(*(state->cpu->reg_dest));
    state->cpu->result = (lsn << 4) | msn;
    _check_flags(state);
    *(state->cpu->reg_dest) = (BYTE)state->cpu->result;
}
CYCLE_FUNC(_sra_reg) {
    BYTE bit7 = bit_7(*(state->cpu->reg_dest));
    BYTE bit0 = bit_0(*(state->cpu->reg_dest));
    state->cpu->result = bit7 | (*(state->cpu->reg_dest)>>1);
    if (bit0)
        state->cpu->check_flags.c = SET;
    else
        state->cpu->check_flags.c = CLEAR;
    _check_flags(state);
    *(state->cpu->reg_dest) = (BYTE)state->cpu->result;
}
CYCLE_FUNC(_srl_reg) {
    BYTE bit0 = bit_0(*(state->cpu->reg_dest));
    state->cpu->result = *(state->cpu->reg_dest) >> 1;
    if (bit0)
        state->cpu->check_flags.c = SET;
    else
        state->cpu->check_flags.c = CLEAR;
    _check_flags(state);
    *(state->cpu->reg_dest) = (BYTE)state->cpu->result;
}
CYCLE_FUNC(_test_bit_0_reg) {
    if (bit_0(*(state->cpu->reg_dest)))
        state->cpu->check_flags.z = CLEAR;
    else
        state->cpu->check_flags.z = SET;
    _check_flags(state);
}
CYCLE_FUNC(_test_bit_1_reg) {
    if (bit_1(*(state->cpu->reg_dest)))
        state->cpu->check_flags.z = CLEAR;
    else
        state->cpu->check_flags.z = SET;
    _check_flags(state);
}
CYCLE_FUNC(_test_bit_2_reg) {
    if (bit_2(*(state->cpu->reg_dest)))
        state->cpu->check_flags.z = CLEAR;
    else
        state->cpu->check_flags.z = SET;
    _check_flags(state);
}
CYCLE_FUNC(_test_bit_3_reg) {
    if (bit_3(*(state->cpu->reg_dest)))
        state->cpu->check_flags.z = CLEAR;
    else
        state->cpu->check_flags.z = SET;
    _check_flags(state);
}
CYCLE_FUNC(_test_bit_4_reg) {
    if (bit_4(*(state->cpu->reg_dest)))
        state->cpu->check_flags.z = CLEAR;
    else
        state->cpu->check_flags.z = SET;
    _check_flags(state);
}
CYCLE_FUNC(_test_bit_5_reg) {
    if (bit_5(*(state->cpu->reg_dest)))
        state->cpu->check_flags.z = CLEAR;
    else
        state->cpu->check_flags.z = SET;
    _check_flags(state);
}
CYCLE_FUNC(_test_bit_6_reg) {
    if (bit_6(*(state->cpu->reg_dest)))
        state->cpu->check_flags.z = CLEAR;
    else
        state->cpu->check_flags.z = SET;
    _check_flags(state);
}
CYCLE_FUNC(_test_bit_7_reg) {
    if (bit_7(*(state->cpu->reg_dest)))
        state->cpu->check_flags.z = CLEAR;
    else
        state->cpu->check_flags.z = SET;
    _check_flags(state);
}
CYCLE_FUNC(_set_bit_0_reg) {
    *(state->cpu->reg_dest) |= (1<<0);
}
CYCLE_FUNC(_res_bit_0_reg) {
    *(state->cpu->reg_dest) &= (BYTE)(~(1<<0));
}
CYCLE_FUNC(_set_bit_1_reg) {
    *(state->cpu->reg_dest) |= (1<<1);
}
CYCLE_FUNC(_res_bit_1_reg) {
    *(state->cpu->reg_dest) &= (BYTE)(~(1<<1));
}
CYCLE_FUNC(_set_bit_2_reg) {
    *(state->cpu->reg_dest) |= (1<<2);
}
CYCLE_FUNC(_res_bit_2_reg) {
    *(state->cpu->reg_dest) &= (BYTE)(~(1<<2));
}
CYCLE_FUNC(_set_bit_3_reg) {
    *(state->cpu->reg_dest) |= (1<<3);
}
CYCLE_FUNC(_res_bit_3_reg) {
    *(state->cpu->reg_dest) &= (BYTE)(~(1<<3));
}
CYCLE_FUNC(_set_bit_4_reg) {
    *(state->cpu->reg_dest) |= (1<<4);
}
CYCLE_FUNC(_res_bit_4_reg) {
    *(state->cpu->reg_dest) &= (BYTE)(~(1<<4));
}
CYCLE_FUNC(_set_bit_5_reg) {
    *(state->cpu->reg_dest) |= (1<<5);
}
CYCLE_FUNC(_res_bit_5_reg) {
    *(state->cpu->reg_dest) &= (BYTE)(~(1<<5));
}
CYCLE_FUNC(_set_bit_6_reg) {
    *(state->cpu->reg_dest) |= (1<<6);
}
CYCLE_FUNC(_res_bit_6_reg) {
    *(state->cpu->reg_dest) &= (BYTE)(~(1<<6));
}
CYCLE_FUNC(_set_bit_7_reg) {
    *(state->cpu->reg_dest) |= (1<<7);
}
CYCLE_FUNC(_res_bit_7_reg) {
    *(state->cpu->reg_dest) &= (BYTE)(~(1<<7));
}

/* misc */
CYCLE_FUNC(_inc_data1) {
    state->cpu->data1++;
}
CYCLE_FUNC(_dec_data1) {
    state->cpu->data1--;
}
CYCLE_FUNC(_do_da) {
    BYTE a_val = reg_a(state->cpu);
    CPUFlags *flags = &state->cpu->flags;
    if (ls_nib(a_val) > 9 || flags->h == SET) {
        flags->c = (check_carry_8(a_val, 6)) ? SET : CLEAR;
        a_val += 6;
    }
    if (ms_nib(a_val) > 9) {
        flags->c = (check_carry_8(a_val, 0x60)) ? SET : CLEAR;
        a_val += 0x60;
    }
    if (a_val == 0)
        flags->z = SET;
    else
        flags->z = CLEAR;
    
    flags->h = CLEAR;

    reg_a(state->cpu) = a_val;
}

/* Copy data from a src register to a dest register */
CYCLE_FUNC(_copy_reg) {
    *(state->cpu->reg_dest) = *(state->cpu->reg_src);
}
CYCLE_FUNC(_copy_reg_16) {
    *(WORD*)state->cpu->reg_dest = *(WORD*)state->cpu->reg_src;
}

/* Copy data from intermediate store to a register */
CYCLE_FUNC(_write_reg_data) {
    if (state->cpu->is_16_bit)
        *(WORD*)(state->cpu->reg_dest) = b2w(state->cpu->data1, state->cpu->data2);
    else
        *(state->cpu->reg_dest) = state->cpu->data1;
}
CYCLE_FUNC(_write_reg_data2) {
    *(state->cpu->reg_dest) = state->cpu->data2;
}
CYCLE_FUNC(_write_reg_data_inc_hl) {
    _write_reg_data(state);
    _inc_hl(state);
}
CYCLE_FUNC(_write_reg_data_dec_hl) {
    _write_reg_data(state);
    _dec_hl(state);
}

/* Memory writes */
/* Write data from a src register to a memory address */
CYCLE_FUNC(_write_mem_reg_l) {
    write_mem(state, state->cpu->addr, *(state->cpu->reg_src));
}
CYCLE_FUNC(_write_mem_reg_l_inc_hl) {
    write_mem(state, state->cpu->addr, *(state->cpu->reg_src));
    _inc_hl(state);
}
CYCLE_FUNC(_write_mem_reg_l_dec_hl) {
    write_mem(state, state->cpu->addr, *(state->cpu->reg_src));
    _dec_hl(state);
}
CYCLE_FUNC(_write_mem_reg_h) {
   write_mem(state, state->cpu->addr+1, *(state->cpu->reg_src + 1)); 
}
CYCLE_FUNC(_write_mem_data1) {
    write_mem(state, state->cpu->addr, state->cpu->data1);
}
CYCLE_FUNC(_write_mem_data2) {
    write_mem(state, state->cpu->addr+1, state->cpu->data2);
}
CYCLE_FUNC(_write_mem_reg_16) {
    BYTE lsb = *(WORD*)(state->cpu->reg_src) & 0xFF;
    BYTE msb = (*(WORD*)(state->cpu->reg_src) & 0xFF00) >> 8;
    write_mem(state, state->cpu->addr, lsb);
    write_mem(state, state->cpu->addr+1, msb);
}
CYCLE_FUNC(_set_addr_from_data) {
    state->cpu->addr = b2w(state->cpu->data1, state->cpu->data2);
}
CYCLE_FUNC(_set_addr_from_data_io) {
    state->cpu->addr = 0xFF00 + state->cpu->data1;
}
/* Read data from src register to intermediate store */
CYCLE_FUNC(_read_reg_l) {
    state->cpu->data1 = *(state->cpu->reg_src);
}
/* Read data from src register to high byte of intermediate store */
CYCLE_FUNC(_read_reg_h) {
    state->cpu->data2 = *(state->cpu->reg_src + 1);
}
/* Read data from memory to intermediate store */
CYCLE_FUNC(_read_mem_l) {
    state->cpu->data1 = read_mem(state, state->cpu->addr);
}
CYCLE_FUNC(_read_mem_h) {
    state->cpu->data2 = read_mem(state, state->cpu->addr + 1);
}
CYCLE_FUNC(_read_mem_l_into_flags) {
    set_flags_from_byte(
        state->cpu, 
        read_mem(state, state->cpu->addr)
    );
}
CYCLE_FUNC(_read_mem_l_and_store) {
    _read_mem_l(state);
    _write_reg_data(state);
}
CYCLE_FUNC(_read_mem_h_and_store) {
    _read_mem_h(state);
    _write_reg_data2(state); 
}

CYCLE_FUNC(_inc_bc) {
    reg_bc(state->cpu)++;
}
CYCLE_FUNC(_dec_bc) {
    reg_bc(state->cpu)--;
}
CYCLE_FUNC(_inc_de) {
    reg_de(state->cpu)++;
}
CYCLE_FUNC(_dec_de) {
    reg_de(state->cpu)--;
}
CYCLE_FUNC(_dec_sp_2) {
    reg_sp(state->cpu) -= 2;
}
CYCLE_FUNC(_dec_sp) {
    reg_sp(state->cpu)--;
}
CYCLE_FUNC(_inc_sp_2) {
    reg_sp(state->cpu) += 2;
}
CYCLE_FUNC(_inc_sp_2_enable_ints) {
    reg_sp(state->cpu) += 2;
    state->cpu->flags.ime = SET;
}
CYCLE_FUNC(_inc_sp) {
    reg_sp(state->cpu)++;
}
CYCLE_FUNC(_inc_pc_2) {
    reg_pc(state->cpu) += 2;
}
CYCLE_FUNC(_set_pc_from_data) {
    reg_pc(state->cpu) = b2w(state->cpu->data1, state->cpu->data2);
}
CYCLE_FUNC(_add_reg_signed_data) {
    state->cpu->result = *(WORD*)(state->cpu->reg_dest) 
        + (signed)state->cpu->offset; 
    state->cpu->is_16_bit = 1;
    _check_flags(state);
    *(WORD*)(state->cpu->reg_dest) += (signed)state->cpu->offset;
}
CYCLE_FUNC(_inc_hl) {
    reg_hl(state->cpu)++;
}
CYCLE_FUNC(_dec_hl) {
    reg_hl(state->cpu)--;
}
CYCLE_FUNC(_nop) {
    
}
CYCLE_FUNC(_disable_interrupts) {
    state->cpu->flags.ime = CLEAR;
}
CYCLE_FUNC(_enable_interrupts) {
    state->cpu->flags.wants_ime = 2;
}
CYCLE_FUNC(_do_halt) {
    state->cpu->state = HALT;
}
CYCLE_FUNC(_do_stop) {
    state->cpu->state = STOP;
}
CYCLE_FUNC(_read_imm_offset) {
    state->cpu->addr = reg_pc(state->cpu);
    state->cpu->offset = (signed)read_mem(state, state->cpu->addr);
    reg_pc(state->cpu)++;
}
CYCLE_FUNC(_read_imm_l) {
    state->cpu->addr = reg_pc(state->cpu);
    state->cpu->data1 = read_mem(state, state->cpu->addr);
    reg_pc(state->cpu)++;
}
CYCLE_FUNC(_read_imm_h) {
    state->cpu->addr = reg_pc(state->cpu);
    state->cpu->data2 = read_mem(state, state->cpu->addr);
    reg_pc(state->cpu)++;
}

CYCLE_FUNC(_fetch_inst) {
    state->cpu->addr = reg_pc(state->cpu);
    state->cpu->opcode = read_mem(state, state->cpu->addr);
    reg_pc(state->cpu)++;
}

CYCLE_FUNC(_check_flags) {
    CPUState *cpu = state->cpu;

    if (cpu->changes_flags != 1)
        goto _end_check_flags;

    CPUFlags cur_flags = cpu->flags;
    CPUFlags chk_flags = cpu->check_flags;

    if (chk_flags.z == SET || chk_flags.z == CLEAR) {
        cpu->flags.z = chk_flags.z;
    } else if (chk_flags.z == CHECK) {
        cpu->flags.z = (cpu->result == 0) ? SET : CLEAR;
    } else if (chk_flags.z == FLIP) {
        cpu->flags.z = (cur_flags.z == SET) ? CLEAR : SET;
    }
    if (chk_flags.n == SET || chk_flags.n == CLEAR) {
        cur_flags.n = chk_flags.n;
    } else if (chk_flags.n == CHECK) {
        // should be unreachable
    } else if (chk_flags.n == FLIP) {
        cpu->flags.n = (cur_flags.n == SET) ? CLEAR : SET;
    }
    if (chk_flags.h == SET || chk_flags.h == CLEAR) {
        cpu->flags.h = chk_flags.h;
    } else if (chk_flags.h == CHECK) {
        cpu->flags.h = ((cpu->result & 0xF)>9) ? SET : CLEAR;
    } else if (chk_flags.h == FLIP) {
        cpu->flags.h = (cur_flags.h == SET) ? CLEAR : SET;
    }
    if (chk_flags.c == SET || chk_flags.c == CLEAR) {
        cpu->flags.c = chk_flags.c;
    } else if (chk_flags.c == CHECK) {
        if (cpu->is_16_bit)
            cpu->flags.c = (cpu->result > (int)0xFFFF) ? SET : CLEAR;
        else
            cpu->flags.c = (cpu->result > (int)0xFF) ? SET : CLEAR;
        
        if (cpu->result < 0)
            cpu->flags.c = SET;
    } else if (chk_flags.c == FLIP) {
        cpu->flags.c = (cur_flags.c == SET) ? CLEAR : SET;
    }
    _end_check_flags:
    1;

}

/* Execute a single m-cycle (4 clock ticks) of CPU operation. 
Executes the current pipelined action first, unless in PREINIT,
increments the pipeline counter, and fetches the next instruction if there
are no further pipelined actions after this.
*/
void cpu_m_cycle(GBState *state) {
    CPUState *cpu = state->cpu;

    if (cpu->state != PREINIT) {
        // Execute the next cycle step of the current queue
        (*cpu->pipeline[cpu->counter])(state);
        // Increment the cycle counter
        cpu->counter++;
    } else {
        cpu->state = READY;
    }

    if (cpu->flags.wants_ime == 2) {
            cpu->flags.wants_ime--;
    } else if (cpu->flags.wants_ime == 1) {
        cpu->flags.ime = SET;
        cpu->flags.wants_ime = 0;
    }
    
    // counter is index into pipeline; if NULL,
    // no further actions are queued and we fetch an opcode.
    if (cpu->pipeline[cpu->counter] == NULL) {
        // We are at the end of an instruction's execution
        
        // Clear the counter, set all func ptrs to NULL, 
        // clear result, 16 bit flag, clears check_flags
        reset_pipeline(cpu);
        // Read next instruction from memory at current PC location
        // load it into cpu->opcode and *increment PC*
        _fetch_inst(state);
        if (cpu->state == READY) {
            // Set up cycle queue based on this opcode
            cpu_setup_pipeline(state, cpu->opcode);
        } else if (cpu->state == PREFIX) {
            cpu_setup_prefix_pipeline(state, cpu->opcode);
            cpu->state = READY;
        }
    } 
}

void cpu_setup_pipeline(GBState *state, BYTE opcode) {
    /* Fill the CPU cycle queue with function pointers and set 
    emulator-level flags needed to execute the opcode argument.
    Note that this is done right after fetching the instruction,
    in the same "cpu M cycle", but the first pipeline function is 
    executed in the next cpu M cycle.
    */
    CPUState *cpu = state->cpu;

    switch (opcode) {
        case 0x00:
            cpu->pipeline[0] = &_nop;
            break;
        case 0x01:
            cpu->reg_dest = &reg_bc(cpu);
            cpu->is_16_bit = 1;
            cpu->pipeline[0] = &_read_imm_l;
            cpu->pipeline[1] = &_read_imm_h;
            cpu->pipeline[2] = &_write_reg_data;
            break;
        case 0x02:
            cpu->addr = reg_bc(cpu);
            cpu->reg_src = &reg_a(cpu);
            cpu->pipeline[0] = &_nop; // TODO use cycle loading BC val to addr
            cpu->pipeline[1] = &_write_mem_reg_l;
            break;
        case 0x03:
            cpu->pipeline[0] = &_nop;
            cpu->pipeline[1] = &_inc_bc;
            break;
        case 0x04:
            cpu->reg_dest = &reg_b(cpu);
            cpu->reg_src = &cpu->data1;
            cpu->data1 = 1;
            CHECK_FLAGS(cpu, CHECK, CLEAR, CHECK, NOCHANGE);

            cpu->pipeline[0] = &_add_reg_reg;
            break;
        case 0x05:
            cpu->reg_dest = &reg_b(cpu);
            cpu->reg_src = &cpu->data1;
            cpu->data1 = 1;
            CHECK_FLAGS(cpu, CHECK, SET, CHECK, NOCHANGE);
            cpu->pipeline[0] = &_sub_reg_reg;
            break;
        case 0x06:
            cpu->reg_dest = &reg_b(cpu);
            cpu->pipeline[0] = &_read_imm_l;
            cpu->pipeline[1] = &_write_reg_data;
            break;
        case 0x07:
            cpu->reg_dest = &reg_a(cpu);
            CHECK_FLAGS(cpu, CLEAR, CLEAR, CLEAR, CHECK);
            cpu->pipeline[0] = &_rlc_reg;
            break;
        case 0x08:
            // FIXME ld (nn), SP
            cpu->reg_src = &reg_sp(cpu);
            cpu->is_16_bit = 1;
            cpu->pipeline[0] = &_read_imm_l;
            cpu->pipeline[1] = &_read_imm_h;
            cpu->pipeline[2] = &_set_addr_from_data;
            cpu->pipeline[3] = &_write_mem_reg_l;
            cpu->pipeline[4] = &_write_mem_reg_h;
            break;
        case 0x09:
            cpu->reg_dest = &reg_hl(cpu);
            cpu->reg_src = &reg_bc(cpu);
            cpu->is_16_bit = 1;
            CHECK_FLAGS(cpu, NOCHANGE, CLEAR, CHECK, CHECK);
            cpu->pipeline[0] = &_nop; // TODO Actually do two 8 bit ops?
            cpu->pipeline[1] = &_add_reg_reg;
            break;
        case 0x0A:
            cpu->reg_dest = &reg_a(cpu);
            cpu->addr = reg_bc(cpu);
            cpu->pipeline[0] = &_read_mem_l;
            cpu->pipeline[1] = &_write_reg_data;
            break;
        case 0x0B:
            cpu->pipeline[0] = &_nop;
            cpu->pipeline[1] = &_dec_bc;
            break;
        case 0x0C:
            cpu->reg_dest = &reg_c(cpu);
            cpu->reg_src = &cpu->data1;
            cpu->data1 = 1;
            CHECK_FLAGS(cpu, CHECK, CLEAR, CHECK, NOCHANGE);
            cpu->pipeline[0] = &_add_reg_reg;
            break;
        case 0x0D:
            cpu->reg_dest = &reg_c(cpu);
            cpu->reg_src = &cpu->data1;
            cpu->data1 = 1;
            CHECK_FLAGS(cpu, CHECK, SET, CHECK, NOCHANGE);
            cpu->pipeline[0] = &_sub_reg_reg;
            break;
        case 0x0E:
            cpu->reg_dest = &reg_c(cpu);
            cpu->pipeline[0] = &_read_imm_l;
            cpu->pipeline[1] = &_write_reg_data;
            break;
        case 0x0F:
            cpu->reg_dest = &reg_a(cpu);
            CHECK_FLAGS(cpu, CLEAR, CLEAR, CLEAR, CHECK);
            cpu->pipeline[0] = &_rrc_reg;
            break;
        case 0x10:
            cpu->pipeline[0] = &_do_stop;
            cpu->pipeline[1] = &_read_imm_l;
            break;
        case 0x11:
            cpu->is_16_bit = 1;
            cpu->reg_dest = &reg_de(cpu);
            cpu->pipeline[0] = &_read_imm_l;
            cpu->pipeline[1] = &_read_imm_h;
            cpu->pipeline[2] = &_write_reg_data;
            break;
        case 0x12:
            cpu->addr = reg_de(cpu);
            cpu->reg_src = &reg_a(cpu);
            cpu->pipeline[0] = &_nop; // TODO use cycle loading BC val to addr
            cpu->pipeline[1] = &_write_mem_reg_l;
            break;
        case 0x13:
            cpu->pipeline[0] = &_nop;
            cpu->pipeline[1] = &_inc_de;
            break;
        case 0x14:
            cpu->reg_dest = &reg_d(cpu);
            cpu->reg_src = &cpu->data1;
            cpu->data1 = 1;
            CHECK_FLAGS(cpu, CHECK, CLEAR, CHECK, NOCHANGE);
            cpu->pipeline[0] = &_add_reg_reg;
            break;
        case 0x15:
            cpu->reg_dest = &reg_d(cpu);
            cpu->reg_src = &cpu->data1;
            cpu->data1 = 1;
            CHECK_FLAGS(cpu, CHECK, SET, CHECK, NOCHANGE);
            cpu->pipeline[0] = &_sub_reg_reg;
            break;
        case 0x16:
            cpu->reg_dest = &reg_d(cpu);
            cpu->pipeline[0] = &_read_imm_l;
            cpu->pipeline[1] = &_write_reg_data;
            break;
        case 0x17:
            cpu->reg_dest = &reg_a(cpu);
            CHECK_FLAGS(cpu, CLEAR, CLEAR, CLEAR, CHECK);
            cpu->pipeline[0] = &_rl_reg;
            break;
        case 0x18:
            cpu->reg_dest = &reg_pc(cpu);
            cpu->pipeline[0] = &_read_imm_offset;
            cpu->pipeline[1] = &_nop;
            cpu->pipeline[2] = &_add_reg_signed_data;
            break;
       case 0x19:
            cpu->reg_dest = &reg_hl(cpu);
            cpu->reg_src = &reg_de(cpu);
            cpu->is_16_bit = 1;
            CHECK_FLAGS(cpu, NOCHANGE, CLEAR, CHECK, CHECK);
            cpu->pipeline[0] = &_nop; // TODO Actually do two 8 bit ops?
            cpu->pipeline[1] = &_add_reg_reg;
            break;
        case 0x1A:
            cpu->reg_dest = &reg_a(cpu);
            cpu->addr = reg_de(cpu);
            cpu->pipeline[0] = &_read_mem_l;
            cpu->pipeline[1] = &_write_reg_data;
            break;
        case 0x1B:
            cpu->pipeline[0] = &_nop;
            cpu->pipeline[1] = &_dec_de;
            break;
        case 0x1C:
            cpu->reg_dest = &reg_e(cpu);
            cpu->reg_src = &cpu->data1;
            cpu->data1 = 1;
            CHECK_FLAGS(cpu, CHECK, CLEAR, CHECK, NOCHANGE);
            cpu->pipeline[0] = &_add_reg_reg;
            break;
        case 0x1D:
            cpu->reg_dest = &reg_c(cpu);
            cpu->reg_src = &cpu->data1;
            cpu->data1 = 1;
            CHECK_FLAGS(cpu, CHECK, SET, CHECK, NOCHANGE);
            cpu->pipeline[0] = &_sub_reg_reg;
            break;
        case 0x1E:
            cpu->reg_dest = &reg_e(cpu);
            cpu->pipeline[0] = &_read_imm_l;
            cpu->pipeline[1] = &_write_reg_data;
            break;
        case 0x1F:
            cpu->reg_dest = &reg_a(cpu);
            CHECK_FLAGS(cpu, CLEAR, CLEAR, CLEAR, CHECK);
            cpu->pipeline[0] = &_rr_reg;
            break; 
        case 0x20:
            // Conditional jump: JR NZ
            cpu->reg_dest = &reg_pc(cpu);
            cpu->is_16_bit = 1;
            cpu->pipeline[0] = &_read_imm_offset;
            cpu->pipeline[1] = &_nop;
            if (cpu->flags.z == CLEAR) {
                cpu->pipeline[2] = &_add_reg_signed_data;
            }
            break;
         case 0x21:
            cpu->is_16_bit = 1;
            cpu->reg_dest = &reg_hl(cpu);
            cpu->pipeline[0] = &_read_imm_l;
            cpu->pipeline[1] = &_read_imm_h;
            cpu->pipeline[2] = &_write_reg_data;
            break;
        case 0x22:
            cpu->addr = reg_hl(cpu);
            cpu->reg_src = &reg_a(cpu);
            cpu->pipeline[0] = &_nop;
            cpu->pipeline[1] = &_write_mem_reg_l_inc_hl;
            break;
        case 0x23:
            cpu->pipeline[0] = &_nop;
            cpu->pipeline[1] = &_inc_hl;
            break;
        case 0x24:
            cpu->reg_dest = &reg_h(cpu);
            cpu->reg_src = &cpu->data1;
            cpu->data1 = 1;
            CHECK_FLAGS(cpu, CHECK, CLEAR, CHECK, NOCHANGE);

            cpu->pipeline[0] = &_add_reg_reg;
            break;
        case 0x25:
            cpu->reg_dest = &reg_h(cpu);
            cpu->reg_src = &cpu->data1;
            cpu->data1 = 1;
            CHECK_FLAGS(cpu, CHECK, SET, CHECK, NOCHANGE);
            cpu->pipeline[0] = &_sub_reg_reg;
            break;
        case 0x26:
            cpu->reg_dest = &reg_h(cpu);
            cpu->pipeline[0] = &_read_imm_l;
            cpu->pipeline[1] = &_write_reg_data;
            break;
        case 0x27:
            //DAA
            cpu->reg_dest = &reg_a(cpu);
            CHECK_FLAGS(cpu, CHECK, NOCHANGE, CLEAR, CHECK);
            cpu->pipeline[0] = &_do_da;
            break;
        case 0x28:
            cpu->reg_dest = &reg_pc(cpu);
            cpu->pipeline[0] = &_read_imm_offset;
            cpu->pipeline[1] = &_nop;
            if (cpu->flags.z == SET) {
                cpu->pipeline[2] = &_add_reg_signed_data;
            }
            break; 
        case 0x29:
            cpu->reg_dest = &reg_hl(cpu);
            cpu->reg_src = &reg_hl(cpu);
            cpu->is_16_bit = 1;
            CHECK_FLAGS(cpu, NOCHANGE, CLEAR, CHECK, CHECK);
            cpu->pipeline[0] = &_nop; // TODO Actually do two 8 bit ops?
            cpu->pipeline[1] = &_add_reg_reg;
            break;
        case 0x2A:
            cpu->reg_dest = &reg_a(cpu);
            cpu->addr = reg_hl(cpu);
            cpu->pipeline[0] = &_read_mem_l;
            cpu->pipeline[1] = &_write_reg_data_inc_hl;
            break;
        case 0x2B:
            cpu->pipeline[0] = &_nop;
            cpu->pipeline[1] = &_dec_hl;
            break;
        case 0x2C:
            cpu->reg_dest = &reg_l(cpu);
            cpu->reg_src = &cpu->data1;
            cpu->data1 = 1;
            CHECK_FLAGS(cpu, CHECK, CLEAR, CHECK, NOCHANGE);
            cpu->pipeline[0] = &_add_reg_reg;
            break;
        case 0x2D:
            cpu->reg_dest = &reg_l(cpu);
            cpu->reg_src = &cpu->data1;
            cpu->data1 = 1;
            CHECK_FLAGS(cpu, CHECK, SET, CHECK, NOCHANGE);
            cpu->pipeline[0] = &_sub_reg_reg;
            break;
        case 0x2E:
            cpu->reg_dest = &reg_l(cpu);
            cpu->pipeline[0] = &_read_imm_l;
            cpu->pipeline[1] = &_write_reg_data;
            break;
        case 0x2F:
            cpu->reg_dest = &reg_a(cpu);
            CHECK_FLAGS(cpu, NOCHANGE, SET, SET, NOCHANGE);
            cpu->pipeline[0] = &_do_cpl;
            break;
        case 0x30:
            cpu->reg_dest = &reg_pc(cpu);
            cpu->pipeline[0] = &_read_imm_offset;
            cpu->pipeline[1] = &_nop;
            if (cpu->flags.c == CLEAR) {
                cpu->pipeline[2] = &_add_reg_signed_data;
            }
            break;
        case 0x31:
            cpu->is_16_bit = 1;
            cpu->reg_dest = &reg_sp(cpu);
            cpu->pipeline[0] = &_read_imm_l;
            cpu->pipeline[1] = &_read_imm_h;
            cpu->pipeline[2] = &_write_reg_data;
            break;
        case 0x32:
            cpu->addr = reg_hl(cpu);
            cpu->reg_src = &reg_a(cpu);
            cpu->pipeline[0] = &_nop; // TODO use cycle loading BC val to addr
            cpu->pipeline[1] = &_write_mem_reg_l_dec_hl;
            break;
        case 0x33:
            cpu->pipeline[0] = &_nop;
            cpu->pipeline[1] = &_inc_sp;
            break;
        case 0x34:
            cpu->addr = reg_hl(cpu);
            cpu->reg_dest = &cpu->data1;
            cpu->reg_src = &cpu->data2;
            cpu->data2 = 1;
            CHECK_FLAGS(cpu, CHECK, CLEAR, CHECK, NOCHANGE);
            cpu->pipeline[0] = &_read_mem_l;
            cpu->pipeline[1] = &_add_reg_reg;
            cpu->pipeline[2] = &_write_mem_data1;
            break;
        case 0x35:
            cpu->addr = &reg_hl(cpu);
            cpu->reg_dest = &cpu->data1;
            cpu->reg_src = &cpu->data2;
            cpu->data2 = 1;
            CHECK_FLAGS(cpu, CHECK, SET, CHECK, NOCHANGE);
            cpu->pipeline[0] = &_read_mem_l;
            cpu->pipeline[1] = &_sub_reg_reg;
            cpu->pipeline[2] = &_write_mem_data1; 
            break;
        case 0x36:
            cpu->addr = reg_hl(cpu);
            cpu->pipeline[0] = &_nop;
            cpu->pipeline[1] = &_read_imm_l;
            cpu->pipeline[2] = &_write_mem_data1;
            break;
        case 0x37:
            //SCF
            CHECK_FLAGS(cpu, NOCHANGE, CLEAR, CLEAR, SET);
            cpu->pipeline[0] = &_check_flags;
            break;
        case 0x38:
            cpu->reg_dest = &reg_pc(cpu);
            cpu->pipeline[0] = &_read_imm_offset;
            cpu->pipeline[1] = &_nop;
            if (cpu->flags.c == SET) {
                cpu->pipeline[2] = &_add_reg_signed_data;
            }
            break; 
        case 0x39:
            cpu->reg_dest = &reg_hl(cpu);
            cpu->reg_src = &reg_sp(cpu);
            cpu->is_16_bit = 1;
            CHECK_FLAGS(cpu, NOCHANGE, CLEAR, CHECK, CHECK);
            cpu->pipeline[0] = &_nop; // TODO Actually do two 8 bit ops?
            cpu->pipeline[1] = &_add_reg_reg;
            break;
        case 0x3A:
            cpu->reg_dest = &reg_a(cpu);
            cpu->addr = reg_hl(cpu);
            cpu->pipeline[0] = &_read_mem_l;
            cpu->pipeline[1] = &_write_reg_data_dec_hl;
            break;
        case 0x3B:
            cpu->pipeline[0] = &_nop;
            cpu->pipeline[1] = &_dec_sp;
            break;
        case 0x3C:
            cpu->reg_dest = &reg_a(cpu);
            cpu->reg_src = &cpu->data1;
            cpu->data1 = 1;
            CHECK_FLAGS(cpu, CHECK, CLEAR, CHECK, NOCHANGE);
            cpu->pipeline[0] = &_add_reg_reg;
            break;
        case 0x3D:
            cpu->reg_dest = &reg_a(cpu);
            cpu->reg_src = &cpu->data1;
            cpu->data1 = 1;
            CHECK_FLAGS(cpu, CHECK, SET, CHECK, NOCHANGE);
            cpu->pipeline[0] = &_sub_reg_reg;
            break;
        case 0x3E:
            cpu->reg_dest = &reg_a(cpu);
            cpu->pipeline[0] = &_read_imm_l;
            cpu->pipeline[1] = &_write_reg_data;
            break;
        case 0x3F:
            // CCF
            CHECK_FLAGS(cpu, NOCHANGE, CLEAR, CLEAR, FLIP);
            cpu->pipeline[0] = &_check_flags;
            break;
        case 0x40:
            LD_REG(cpu, b, b);
            break;
        case 0x41:
            LD_REG(cpu, b, c);
            break;
        case 0x42:
            LD_REG(cpu, b, d);
            break;
        case 0x43:
            LD_REG(cpu, b, e);
            break;
        case 0x44:
            LD_REG(cpu, b, h);
            break;
        case 0x45:
            LD_REG(cpu, b, l);
            break;
        case 0x46:
            LD_REG_HL(cpu, b);
            break;
        case 0x47:
            LD_REG(cpu, b, a);
            break;
        case 0x48:
            LD_REG(cpu, c, b);
            break;
        case 0x49:
            LD_REG(cpu, c, c);
            break;
        case 0x4A:
            LD_REG(cpu, c, d);
            break;
        case 0x4B:
            LD_REG(cpu, c, e);
            break;
        case 0x4C:
            LD_REG(cpu, c, h);
            break;
        case 0x4D:
            LD_REG(cpu, c, l);
            break;
        case 0x4E:
            LD_REG_HL(cpu, c);
            break;
        case 0x4F:
            LD_REG(cpu, c, a);
            break;
        case 0x50:
            LD_REG(cpu, d, b);
            break;
        case 0x51:
            LD_REG(cpu, d, c);
            break;
        case 0x52:
            LD_REG(cpu, d, d);
            break;
        case 0x53:
            LD_REG(cpu, d, e);
            break;
        case 0x54:
            LD_REG(cpu, d, h);
            break;
        case 0x55:
            LD_REG(cpu, d, l);
            break;
        case 0x56:
            LD_REG_HL(cpu, d);
            break;
        case 0x57:
            LD_REG(cpu, d, a);
            break;
        case 0x58:
            LD_REG(cpu, e, b);
            break;
        case 0x59:
            LD_REG(cpu, e, c);
            break;
        case 0x5A:
            LD_REG(cpu, e, d);
            break;
        case 0x5B:
            LD_REG(cpu, e, e);
            break;
        case 0x5C:
            LD_REG(cpu, e, h);
            break;
        case 0x5D:
            LD_REG(cpu, e, l);
            break;
        case 0x5E:
            LD_REG_HL(cpu, e);
            break;
        case 0x5F:
            LD_REG(cpu, e, a);
            break;
        case 0x60:
            LD_REG(cpu, h, b);
            break;
        case 0x61:
            LD_REG(cpu, h, c);
            break;
        case 0x62:
            LD_REG(cpu, h, d);
            break;
        case 0x63:
            LD_REG(cpu, h, e);
            break;
        case 0x64:
            LD_REG(cpu, h, h);
            break;
        case 0x65:
            LD_REG(cpu, h, l);
            break;
        case 0x66:
            LD_REG_HL(cpu, h);
            break;
        case 0x67:
            LD_REG(cpu, h, a);
            break;
        case 0x68:
            LD_REG(cpu, l, b);
            break;
        case 0x69:
            LD_REG(cpu, l, c);
            break;
        case 0x6A:
            LD_REG(cpu, l, d);
            break;
        case 0x6B:
            LD_REG(cpu, l, e);
            break;
        case 0x6C:
            LD_REG(cpu, l, h);
            break;
        case 0x6D:
            LD_REG(cpu, l, l);
            break;
        case 0x6E:
            LD_REG_HL(cpu, l);
            break;
        case 0x6F:
            LD_REG(cpu, l, a);
            break;
        case 0x70:
            LD_HL_REG(cpu, b);
            break;
        case 0x71:
            LD_HL_REG(cpu, c);
            break;
        case 0x72:
            LD_HL_REG(cpu, d);
            break;
        case 0x73:
            LD_HL_REG(cpu, e);
            break;
        case 0x74:
            LD_HL_REG(cpu, h);
            break;
        case 0x75:
            LD_HL_REG(cpu, l);
            break;
        case 0x76:
            // TODO HALT
            cpu->pipeline[0] = &_do_halt;
            break;
        case 0x77:
            LD_HL_REG(cpu, a);
            break;
        case 0x78:
            LD_REG(cpu, a, b);
            break;
        case 0x79:
            LD_REG(cpu, a, c);
            break;
        case 0x7A:
            LD_REG(cpu, a, d);
            break;
        case 0x7B:
            LD_REG(cpu, a, e);
            break;
        case 0x7C:
            LD_REG(cpu, a, h);
            break;
        case 0x7D:
            LD_REG(cpu, a, l);
            break;
        case 0x7E:
            LD_REG_HL(cpu, a);
            break;
        case 0x7F:
            LD_REG(cpu, a, a);
            break;
        case 0x80:
            ADD_8_REG(cpu, a, b);
            break;
        case 0x81:
            ADD_8_REG(cpu, a, c);
            break;
        case 0x82:
            ADD_8_REG(cpu, a, d);
            break;
        case 0x83:
            ADD_8_REG(cpu, a, e);
            break;
        case 0x84:
            ADD_8_REG(cpu, a, h);
            break;
        case 0x85:
            ADD_8_REG(cpu, a, l);
            break;
        case 0x86:
            ADD_8_HL(cpu, a);
            break;
        case 0x87:
            ADD_8_REG(cpu, a, a);
            break;
        case 0x88:
            ADC_8_REG(cpu, a, b);
            break;
        case 0x89:
            ADC_8_REG(cpu, a, c);
            break;
        case 0x8A:
            ADC_8_REG(cpu, a, d);
            break;
        case 0x8B:
            ADC_8_REG(cpu, a, e);
            break;
        case 0x8C:
            ADC_8_REG(cpu, a, h);
            break;
        case 0x8D:
            ADC_8_REG(cpu, a, l);
        case 0x8E:
            ADC_8_HL(cpu, a);
            break;
        case 0x8F:
            ADC_8_REG(cpu, a, a);
        case 0x90:
            SUB_8_REG(cpu, a, b);
            break;
        case 0x91:
            SUB_8_REG(cpu, a, c);
            break;
        case 0x92:
            SUB_8_REG(cpu, a, d);
            break;
        case 0x93:
            SUB_8_REG(cpu, a, e);
            break;
        case 0x94:
            SUB_8_REG(cpu, a, h);
            break;
        case 0x95:
            SUB_8_REG(cpu, a, l);
        case 0x96:
            SUB_8_HL(cpu, a);
            break;
        case 0x97:
            SUB_8_REG(cpu, a, a);
            break;
        case 0x98:
            SBC_8_REG(cpu, a, b);
            break;
        case 0x99:
            SBC_8_REG(cpu, a, c);
            break;
        case 0x9A:
            SBC_8_REG(cpu, a, d);
            break;
        case 0x9B:
            SBC_8_REG(cpu, a, e);
            break;
        case 0x9C:
            SBC_8_REG(cpu, a, h);
            break;
        case 0x9D:
            SBC_8_REG(cpu, a, l);
            break;
        case 0x9E:
            SBC_8_HL(cpu, a);
            break;
        case 0x9F:
            SBC_8_REG(cpu, a, a);
            break;
        case 0xA0:
            AND_8_REG(cpu, a, b);
            break;
        case 0xA1:
            AND_8_REG(cpu, a, c);
            break;
        case 0xA2:
            AND_8_REG(cpu, a, d);
            break;
        case 0xA3:
            AND_8_REG(cpu, a, e);
            break;
        case 0xA4:
            AND_8_REG(cpu, a, h);
            break;
        case 0xA5:
            AND_8_REG(cpu, a, l);
            break;
        case 0xA6:
            AND_8_HL(cpu, a);
            break;
        case 0xA7:
            AND_8_REG(cpu, a, a);
            break;
        case 0xA8:
            XOR_8_REG(cpu, a, b);
            break;
        case 0xA9:
            XOR_8_REG(cpu, a, c);
            break;
        case 0xAA:
            XOR_8_REG(cpu, a, d);
            break;
        case 0xAB:
            XOR_8_REG(cpu, a, e);
            break;
        case 0xAC:
            XOR_8_REG(cpu, a, h);
            break;
        case 0xAD:
            XOR_8_REG(cpu, a, l);
            break;
        case 0xAE:
            XOR_8_HL(cpu, a);
            break;
        case 0xAF:
            XOR_8_REG(cpu, a, a);
            break;
        case 0xB0:
            OR_8_REG(cpu, a, b);
            break;
        case 0xB1:
            OR_8_REG(cpu, a, c);
            break;
        case 0xB2:
            OR_8_REG(cpu, a, d);
            break;
        case 0xB3:
            OR_8_REG(cpu, a, e);
            break;
        case 0xB4:
            OR_8_REG(cpu, a, h);
            break;
        case 0xB5:
            OR_8_REG(cpu, a, l);
            break;
        case 0xB6:
            OR_8_HL(cpu, a);
            break;
        case 0xB7:
            OR_8_REG(cpu, a, a);
            break;
        case 0xB8:
            CP_8_REG(cpu, a, b);
            break;
        case 0xB9:
            CP_8_REG(cpu, a, c);
            break;
        case 0xBA:
            CP_8_REG(cpu, a, d);
            break;
        case 0xBB:
            CP_8_REG(cpu, a, e);
            break;
        case 0xBC:
            CP_8_REG(cpu, a, h);
            break;
        case 0xBD:
            CP_8_REG(cpu, a, l);
            break;
        case 0xBE:
            CP_8_HL(cpu, a);
            break;
        case 0xBF:
            CP_8_REG(cpu, a, a); 
            break;
        case 0xC0:
            cpu->reg_dest = &reg_pc(cpu);
            cpu->addr = reg_sp(cpu);
            cpu->is_16_bit = 1;
            cpu->pipeline[0] = &_nop;
            if (cpu->flags.z == CLEAR) {
                cpu->pipeline[1] = &_read_mem_l;
                cpu->pipeline[2] = &_read_mem_h;
                cpu->pipeline[3] = &_inc_sp_2;
                cpu->pipeline[4] = &_write_reg_data;
            } else {
                cpu->pipeline[1] = &_nop;
            }
            break;
        case 0xC1:
            cpu->reg_dest = &reg_bc(cpu);
            cpu->addr = reg_sp(cpu);
            cpu->is_16_bit = 1;
            cpu->pipeline[0] = &_read_mem_l;
            cpu->pipeline[1] = &_read_mem_h_and_store;
            cpu->pipeline[2] = &_inc_sp_2;
            break;
        case 0xC2:
            cpu->reg_dest = &reg_pc(cpu);
            cpu->is_16_bit = 1;
            cpu->pipeline[0] = &_read_imm_l;
            cpu->pipeline[1] = &_read_imm_h;
            cpu->pipeline[2] = &_nop;
            if (cpu->flags.z == CLEAR) {
                cpu->pipeline[3] = &_write_reg_data;
            }
            break;
        case 0xC3:
            cpu->reg_dest = &reg_pc(cpu);
            cpu->is_16_bit = 1;
            cpu->pipeline[0] = &_read_imm_l;
            cpu->pipeline[1] = &_read_imm_h;
            cpu->pipeline[2] = &_nop;
            cpu->pipeline[3] = &_write_reg_data;
            break;
        case 0xC4:
            cpu->reg_dest = &reg_pc(cpu);
            cpu->is_16_bit = 1;
            cpu->pipeline[0] = &_read_imm_l;
            cpu->pipeline[1] = &_read_imm_h;
            cpu->pipeline[2] = &_nop;
            if (cpu->flags.z == CLEAR) {
                cpu->pipeline[3] = &_dec_sp_2;
                cpu->addr = reg_sp(cpu) - 2;
                cpu->pipeline[4] = &_write_mem_reg_16;
                cpu->pipeline[5] = &_write_reg_data;
            }
            break;
        case 0xC5:
            cpu->addr = reg_sp(cpu) - 2;
            cpu->reg_src = &reg_bc(cpu);
            cpu->is_16_bit = 1;
            cpu->pipeline[0] = &_dec_sp_2;
            cpu->pipeline[1] = &_nop;
            cpu->pipeline[2] = &_nop;
            cpu->pipeline[3] = &_write_mem_reg_16;
            break;
        case 0xC6:
            cpu->reg_dest = &reg_a(cpu);
            cpu->reg_src = &cpu->data1;
            cpu->pipeline[0] = &_read_imm_l;
            CHECK_FLAGS(cpu, CHECK, CLEAR, CHECK, CHECK);
            cpu->pipeline[1] = &_add_reg_reg;
            break;
        case 0xC7:
            // RST 00
            cpu->reg_dest = &reg_pc(cpu);
            cpu->is_16_bit = 1;
            cpu->data1 = 0x00;
            cpu->data2 = 0x00;
            cpu->pipeline[0] = &_dec_sp_2;
            cpu->addr = reg_sp(cpu) - 2;
            cpu->pipeline[1] = &_write_mem_reg_16;
            cpu->pipeline[2] = &_nop;
            cpu->pipeline[3] = &_write_reg_data;
            break;
        case 0xC8:
            // RET Z
            cpu->reg_dest = &reg_pc(cpu);
            cpu->addr = reg_sp(cpu);
            cpu->is_16_bit = 1;
            cpu->pipeline[0] = &_nop;
            if (cpu->flags.z == SET) {
                cpu->pipeline[1] = &_read_mem_l;
                cpu->pipeline[2] = &_read_mem_h;
                cpu->pipeline[3] = &_inc_sp_2;
                cpu->pipeline[4] = &_write_reg_data;
            } else {
                cpu->pipeline[1] = &_nop;
            }
            break;
        case 0xC9:
            cpu->reg_dest = &reg_pc(cpu);
            cpu->addr = reg_sp(cpu);
            cpu->is_16_bit = 1;
            cpu->pipeline[0] = &_read_mem_l;
            cpu->pipeline[1] = &_read_mem_h;
            cpu->pipeline[2] = &_write_reg_data;
            cpu->pipeline[3] = &_inc_sp_2;
            break;
        case 0xCA:
            cpu->reg_dest = &reg_pc(cpu);
            cpu->is_16_bit = 1;
            cpu->pipeline[0] = &_read_imm_l;
            cpu->pipeline[1] = &_read_imm_h;
            cpu->pipeline[2] = &_nop;
            if (cpu->flags.z == SET) {
                cpu->pipeline[3] = &_write_reg_data;
            }
            break; 
        case 0xCB:
            //PREFIX
            cpu->state = PREFIX;
            cpu->pipeline[0] = &_nop;
            break;
        case 0xCC:
            cpu->reg_dest = &reg_pc(cpu);
            cpu->is_16_bit = 1;
            cpu->pipeline[0] = &_read_imm_l;
            cpu->pipeline[1] = &_read_imm_h;
            cpu->pipeline[2] = &_nop;
            if (cpu->flags.z == SET) {
                cpu->pipeline[3] = &_dec_sp_2;
                cpu->addr = reg_sp(cpu) - 2;
                cpu->pipeline[4] = &_write_mem_reg_16;
                cpu->pipeline[5] = &_write_reg_data;
            }
            break;
        case 0xCD:
            cpu->reg_dest = &reg_pc(cpu);
            cpu->is_16_bit = 1;
            cpu->pipeline[0] = &_read_imm_l;
            cpu->pipeline[1] = &_read_imm_h;
            cpu->pipeline[2] = &_nop;     
            cpu->pipeline[3] = &_dec_sp_2;
            cpu->addr = reg_sp(cpu) - 2;
            cpu->pipeline[4] = &_write_mem_reg_16;
            cpu->pipeline[5] = &_write_reg_data;
            break;
        case 0xCE:
            cpu->reg_dest = &reg_a(cpu);
            cpu->reg_src = &cpu->data1;
            cpu->pipeline[0] = &_read_imm_l;
            CHECK_FLAGS(cpu, CHECK, CLEAR, CHECK, CHECK);
            cpu->pipeline[1] = &_adc_reg_reg;
            break;
        case 0xCF:
            // RST 08
            cpu->reg_dest = &reg_pc(cpu);
            cpu->is_16_bit = 1;
            cpu->data1 = 0x08;
            cpu->data2 = 0x00;
            cpu->pipeline[0] = &_dec_sp_2;
            cpu->addr = reg_sp(cpu) - 2;
            cpu->pipeline[1] = &_write_mem_reg_16;
            cpu->pipeline[2] = &_nop;
            cpu->pipeline[3] = &_write_reg_data;
            break;
        case 0xD0:
            cpu->reg_dest = &reg_pc(cpu);
            cpu->addr = reg_sp(cpu);
            cpu->is_16_bit = 1;
            cpu->pipeline[0] = &_nop;
            if (cpu->flags.c == CLEAR) {
                cpu->pipeline[1] = &_read_mem_l;
                cpu->pipeline[2] = &_read_mem_h;
                cpu->pipeline[3] = &_inc_sp_2;
                cpu->pipeline[4] = &_write_reg_data;
            } else {
                cpu->pipeline[1] = &_nop;
            }
            break;
        case 0xD1:
            cpu->reg_dest = &reg_de(cpu);
            cpu->addr = reg_sp(cpu);
            cpu->is_16_bit = 1;
            cpu->pipeline[0] = &_read_mem_l;
            cpu->pipeline[1] = &_read_mem_h_and_store;
            cpu->pipeline[2] = &_inc_sp_2;
            break;
        case 0xD2:
            cpu->reg_dest = &reg_pc(cpu);
            cpu->is_16_bit = 1;
            cpu->pipeline[0] = &_read_imm_l;
            cpu->pipeline[1] = &_read_imm_h;
            cpu->pipeline[2] = &_nop;
            if (cpu->flags.c == CLEAR) {
                cpu->pipeline[3] = &_write_reg_data;
            }
            break; 
        case 0xD3:
            ILLEGAL_INST(cpu, 0xD3);
            break;
        case 0xD4:
            cpu->reg_dest = &reg_pc(cpu);
            cpu->is_16_bit = 1;
            cpu->pipeline[0] = &_read_imm_l;
            cpu->pipeline[1] = &_read_imm_h;
            cpu->pipeline[2] = &_nop;
            if (cpu->flags.c == CLEAR) {
                cpu->pipeline[3] = &_dec_sp_2;
                cpu->addr = reg_sp(cpu) - 2;
                cpu->pipeline[4] = &_write_mem_reg_16;
                cpu->pipeline[5] = &_write_reg_data;
            }
            break; 
        case 0xD5:
            cpu->addr = reg_sp(cpu) - 2;
            cpu->reg_src = &reg_de(cpu);
            cpu->is_16_bit = 1;
            cpu->pipeline[0] = &_dec_sp_2;
            cpu->pipeline[1] = &_nop;
            cpu->pipeline[2] = &_nop;
            cpu->pipeline[3] = &_write_mem_reg_16;
            break;
        case 0xD6:
            cpu->reg_dest = &reg_a(cpu);
            cpu->reg_src = &cpu->data1;
            cpu->pipeline[0] = &_read_imm_l;
            CHECK_FLAGS(cpu, CHECK, SET, CHECK, CHECK);
            cpu->pipeline[1] = &_sub_reg_reg;
            break;
        case 0xD7:
            // RST 10
            cpu->reg_dest = &reg_pc(cpu);
            cpu->is_16_bit = 1;
            cpu->data1 = 0x10;
            cpu->data2 = 0x00;
            cpu->pipeline[0] = &_dec_sp_2;
            cpu->addr = reg_sp(cpu) - 2;
            cpu->pipeline[1] = &_write_mem_reg_16;
            cpu->pipeline[2] = &_nop;
            cpu->pipeline[3] = &_write_reg_data;
            break;
        case 0xD8:
            // RET Z
            cpu->reg_dest = &reg_pc(cpu);
            cpu->addr = reg_sp(cpu);
            cpu->is_16_bit = 1;
            cpu->pipeline[0] = &_nop;
            if (cpu->flags.c == SET) {
                cpu->pipeline[1] = &_read_mem_l;
                cpu->pipeline[2] = &_read_mem_h;
                cpu->pipeline[3] = &_inc_sp_2;
                cpu->pipeline[4] = &_write_reg_data;
            } else {
                cpu->pipeline[1] = &_nop;
            }
            break;
        case 0xD9:
            // RETI
            cpu->reg_dest = &reg_pc(cpu);
            cpu->addr = reg_sp(cpu);
            cpu->is_16_bit = 1;
            cpu->pipeline[0] = &_read_mem_l;
            cpu->pipeline[1] = &_read_mem_h;
            cpu->pipeline[2] = &_write_reg_data;
            cpu->pipeline[3] = &_inc_sp_2_enable_ints;
            break;
        case 0xDA:
            cpu->reg_dest = &reg_pc(cpu);
            cpu->is_16_bit = 1;
            cpu->pipeline[0] = &_read_imm_l;
            cpu->pipeline[1] = &_read_imm_h;
            cpu->pipeline[2] = &_nop;
            if (cpu->flags.c == SET) {
                cpu->pipeline[3] = &_write_reg_data;
            }
            break;  
        case 0xDB:
            ILLEGAL_INST(cpu, 0xDB);
            break;
        case 0xDC:
            cpu->reg_dest = &reg_pc(cpu);
            cpu->is_16_bit = 1;
            cpu->pipeline[0] = &_read_imm_l;
            cpu->pipeline[1] = &_read_imm_h;
            cpu->pipeline[2] = &_nop;
            if (cpu->flags.c == SET) {
                cpu->pipeline[3] = &_dec_sp_2;
                cpu->addr = reg_sp(cpu) - 2;
                cpu->pipeline[4] = &_write_mem_reg_16;
                cpu->pipeline[5] = &_write_reg_data;
            }
            break;
        case 0xDD:
            ILLEGAL_INST(cpu, 0xDD);
            break;
        case 0xDE:
            cpu->reg_dest = &reg_a(cpu);
            cpu->reg_src = &cpu->data1;
            cpu->pipeline[0] = &_read_imm_l;
            CHECK_FLAGS(cpu, CHECK, SET, CHECK, CHECK);
            cpu->pipeline[1] = &_sbc_reg_reg;
            break;
        case 0xDF:
            // RST 18
            cpu->reg_dest = &reg_pc(cpu);
            cpu->is_16_bit = 1;
            cpu->data1 = 0x18;
            cpu->data2 = 0x00;
            cpu->pipeline[0] = &_dec_sp_2;
            cpu->addr = reg_sp(cpu) - 2;
            cpu->pipeline[1] = &_write_mem_reg_16;
            cpu->pipeline[2] = &_nop;
            cpu->pipeline[3] = &_write_reg_data; 
            break;
        case 0xE0:
            // LDH (a8), A 3 cycles
            cpu->reg_src = &reg_a(cpu);
            cpu->pipeline[0] = &_read_imm_l;
            // cpu->addr = 0xFF00 + cpu->data1
            cpu->pipeline[1] = &_set_addr_from_data_io;
            cpu->pipeline[2] = &_write_mem_reg_l;
            break;
        case 0xE1:
            cpu->reg_dest = &reg_hl(cpu);
            cpu->addr = reg_sp(cpu);
            cpu->is_16_bit = 1;
            cpu->pipeline[0] = &_read_mem_l;
            cpu->pipeline[1] = &_read_mem_h_and_store;
            cpu->pipeline[2] = &_inc_sp_2; 
            break;
        case 0xE2:
            cpu->reg_src = &reg_a(cpu);
            cpu->data1 = reg_c(cpu);
            cpu->pipeline[0] = &_set_addr_from_data_io;
            cpu->pipeline[1] = &_write_mem_reg_l;
            break;
        case 0xE3:
            ILLEGAL_INST(cpu, 0xE3);
            break;
        case 0xE4:
            ILLEGAL_INST(cpu, 0xE4);
            break;
        case 0xE5:
            cpu->addr = reg_sp(cpu) - 2;
            cpu->reg_src = &reg_hl(cpu);
            cpu->is_16_bit = 1;
            cpu->pipeline[0] = &_dec_sp_2;
            cpu->pipeline[1] = &_nop;
            cpu->pipeline[2] = &_nop;
            cpu->pipeline[3] = &_write_mem_reg_16;
            break;
        case 0xE6:
            cpu->reg_dest = &reg_a(cpu);
            cpu->reg_src = &cpu->data1;
            cpu->pipeline[0] = &_read_imm_l;
            CHECK_FLAGS(cpu, CHECK, CLEAR, SET, CLEAR);
            cpu->pipeline[1] = &_and_reg_reg;
            break;
        case 0xE7:
            // RST 20
            cpu->reg_dest = &reg_pc(cpu);
            cpu->is_16_bit = 1;
            cpu->data1 = 0x20;
            cpu->data2 = 0x00;
            cpu->pipeline[0] = &_dec_sp_2;
            cpu->addr = reg_sp(cpu) - 2;
            cpu->pipeline[1] = &_write_mem_reg_16;
            cpu->pipeline[2] = &_nop;
            cpu->pipeline[3] = &_write_reg_data;
            break;
        case 0xE8:
            // ADD SP, r8
            // 4 cycles
            cpu->reg_dest = &reg_sp(cpu);
            cpu->pipeline[0] = &_read_imm_offset;
            CHECK_FLAGS(cpu, CLEAR, CLEAR, CHECK, CHECK);
            cpu->pipeline[1] = &_nop;
            cpu->pipeline[2] = &_nop;
            cpu->pipeline[3] = &_add_reg_signed_data;
            break;
        case 0xE9:
            cpu->reg_dest = &reg_pc(cpu);
            cpu->reg_src = &reg_hl(cpu);
            cpu->pipeline[0] = &_nop;
            cpu->pipeline[1] = &_copy_reg_16;
            break;
        case 0xEA:
            cpu->reg_src = &reg_a(cpu);
            cpu->pipeline[0] = &_read_imm_l;
            cpu->pipeline[1] = &_read_imm_h;
            cpu->pipeline[2] = &_set_addr_from_data;
            cpu->pipeline[3] = &_write_mem_reg_l;
            break;
        case 0xEB:
            ILLEGAL_INST(cpu, 0xEB);
            break;
        case 0xEC:
            ILLEGAL_INST(cpu, 0xEC);
            break;
        case 0xED:
            ILLEGAL_INST(cpu, 0xED);
            break;
        case 0xEE:
            cpu->reg_dest = &reg_a(cpu);
            cpu->reg_src = &cpu->data1;
            cpu->pipeline[0] = &_read_imm_l;
            CHECK_FLAGS(cpu, CHECK, CLEAR, CLEAR, CLEAR);
            cpu->pipeline[1] = &_xor_reg_reg;
            break;
        case 0xEF:
            // RST 28
            cpu->reg_dest = &reg_pc(cpu);
            cpu->is_16_bit = 1;
            cpu->data1 = 0x28;
            cpu->data2 = 0x00;
            cpu->pipeline[0] = &_dec_sp_2;
            cpu->addr = reg_sp(cpu) - 2;
            cpu->pipeline[1] = &_write_mem_reg_16;
            cpu->pipeline[2] = &_nop;
            cpu->pipeline[3] = &_write_reg_data; 
            break;
        case 0xF0:
            cpu->reg_dest = &reg_a(cpu);
            cpu->pipeline[0] = &_read_imm_l;
            cpu->pipeline[1] = &_set_addr_from_data_io;
            cpu->pipeline[2] = &_read_mem_l_and_store;
            break;
        case 0xF1:
            // POP AF
            cpu->reg_dest = &reg_a(cpu);
            cpu->addr = reg_sp(cpu);
            cpu->is_16_bit = 0;
            // Data1 not used
            cpu->data1 = 0;
            cpu->pipeline[0] = &_read_mem_l_into_flags;
            cpu->pipeline[1] = &_read_mem_h_and_store;
            cpu->pipeline[2] = &_inc_sp_2;  
            break;
        case 0xF2:
            cpu->reg_dest = &reg_a(cpu);
            cpu->data1 = reg_c(cpu);
            cpu->pipeline[0] = &_set_addr_from_data_io;
            cpu->pipeline[1] = &_read_mem_l_and_store;
            break;
        case 0xF3:
            // DI
            cpu->pipeline[0] = &_disable_interrupts;
            break;
        case 0xF4:
            ILLEGAL_INST(cpu, 0xF4);
            break;
        case 0xF5:
            //PUSH AF
            cpu->addr = reg_sp(cpu) - 2;
            cpu->reg_src = &reg_a(cpu);
            cpu->is_16_bit = 0;
            cpu->data1 = flags_to_byte(cpu);
            cpu->data2 = reg_a(cpu);

            cpu->pipeline[0] = &_dec_sp_2;
            cpu->pipeline[1] = &_nop;
            cpu->pipeline[2] = &_write_mem_data1;
            cpu->pipeline[3] = &_write_mem_data2;
            break;
        case 0xF6:
            cpu->reg_dest = &reg_a(cpu);
            cpu->reg_src = &cpu->data1;
            cpu->pipeline[0] = &_read_imm_l;
            CHECK_FLAGS(cpu, CHECK, CLEAR, CLEAR, CLEAR);
            cpu->pipeline[1] = &_or_reg_reg;
            break;
        case 0xF7:
            // RST 30
            cpu->reg_dest = &reg_pc(cpu);
            cpu->is_16_bit = 1;
            cpu->data1 = 0x30;
            cpu->data2 = 0x00;
            cpu->pipeline[0] = &_dec_sp_2;
            cpu->addr = reg_sp(cpu) - 2;
            cpu->pipeline[1] = &_write_mem_reg_16;
            cpu->pipeline[2] = &_nop;
            cpu->pipeline[3] = &_write_reg_data;
            break; 
        case 0xF8:
            cpu->reg_dest = &reg_hl(cpu);
            cpu->reg_src = &reg_sp(cpu);
            cpu->is_16_bit = 1;
            CHECK_FLAGS(cpu, CLEAR, CLEAR, CHECK, CHECK);
            cpu->pipeline[0] = &_read_imm_offset;
            cpu->pipeline[1] = &_copy_reg_16;
            cpu->pipeline[2] = &_add_reg_signed_data;
            break;
        case 0xF9:
            cpu->reg_dest = &reg_sp(cpu);
            cpu->reg_src = &reg_hl(cpu);
            cpu->is_16_bit = 1;
            cpu->pipeline[0] = &_nop;
            cpu->pipeline[1] = &_copy_reg_16;
            break;
        case 0xFA:
            cpu->reg_dest = &reg_a(cpu);
            cpu->pipeline[0] = &_read_imm_l;
            cpu->pipeline[1] = &_read_imm_h;
            cpu->pipeline[2] = &_set_addr_from_data;
            cpu->pipeline[3] = _read_mem_l_and_store;
            break;
        case 0xFB:
            //EI
            cpu->pipeline[0] = &_enable_interrupts;
            break;
        case 0xFC:
            ILLEGAL_INST(cpu, 0xFC);
            break;
        case 0xFD:
            ILLEGAL_INST(cpu, 0xFD);
            break;
        case 0xFE:
            cpu->reg_dest = &reg_a(cpu);
            cpu->reg_src = &cpu->data1;
            cpu->pipeline[0] = &_read_imm_l;
            CHECK_FLAGS(cpu, CHECK, SET, CHECK, CHECK);
            cpu->pipeline[1] = &_cp_reg_reg;
            break;
        case 0xFF:
            // RST 38
            cpu->reg_dest = &reg_pc(cpu);
            cpu->is_16_bit = 1;
            cpu->data1 = 0x38;
            cpu->data2 = 0x00;
            cpu->pipeline[0] = &_dec_sp_2;
            cpu->addr = reg_sp(cpu) - 2;
            cpu->pipeline[1] = &_write_mem_reg_16;
            cpu->pipeline[2] = &_nop;
            cpu->pipeline[3] = &_write_reg_data; 
            break; 
    }

    
}

BYTE flags_to_byte(CPUState *cpu) {
    BYTE result = 0;
    if (cpu->flags.z == SET)
        result |= 0x80;
    if (cpu->flags.n == SET)
        result |= 0x40;
    if (cpu->flags.h == SET)
        result |= 0x20;
    if (cpu->flags.c == SET)
        result |= 0x10;

    return result;
}

void set_flags_from_byte(CPUState *cpu, BYTE f) {
    if (f & 0x80)
        cpu->flags.z = SET;
    else 
        cpu->flags.z = CLEAR;

    if (f & 0x40)
        cpu->flags.n = SET;
    else
        cpu->flags.n = CLEAR;

    if (f & 0x20) 
        cpu->flags.h = SET; 
    else
        cpu->flags.h = CLEAR;
    
    if (f & 0x10)
        cpu->flags.c = SET;
    else
        cpu->flags.c = CLEAR;
}

void *_get_prefix_exec_ptr(BYTE opcode) {
    BYTE op_ls = ls_nib(opcode);
    BYTE op_ms = ms_nib(opcode);

    if (op_ls < 8) {
        switch (op_ms) {
            case 0x0: return &_rlc_reg;
            case 0x1: return &_rl_reg;
            case 0x2: return &_sla_reg;
            case 0x3: return &_swap_reg;
            case 0x4: return &_test_bit_0_reg;
            case 0x5: return &_test_bit_2_reg;
            case 0x6: return &_test_bit_4_reg;
            case 0x7: return &_test_bit_6_reg;
            case 0x8: return &_res_bit_0_reg;
            case 0x9: return &_res_bit_2_reg;
            case 0xA: return &_res_bit_4_reg;
            case 0xB: return &_res_bit_6_reg;
            case 0xC: return &_set_bit_0_reg;
            case 0xD: return &_set_bit_2_reg;
            case 0xE: return &_set_bit_4_reg;
            case 0xF: return &_set_bit_6_reg;
        }
    } else {
        switch (op_ms) {
            case 0x0: return &_rrc_reg;
            case 0x1: return &_rr_reg;
            case 0x2: return &_sra_reg;
            case 0x3: return &_srl_reg;
            case 0x4: return &_test_bit_1_reg;
            case 0x5: return &_test_bit_3_reg;
            case 0x6: return &_test_bit_5_reg;
            case 0x7: return &_test_bit_7_reg;
            case 0x8: return &_res_bit_1_reg;
            case 0x9: return &_res_bit_3_reg;
            case 0xA: return &_res_bit_5_reg;
            case 0xB: return &_res_bit_7_reg;
            case 0xC: return &_set_bit_1_reg;
            case 0xD: return &_set_bit_3_reg;
            case 0xE: return &_set_bit_5_reg;
            case 0xF: return &_set_bit_7_reg;
        }
    }
}

void cpu_setup_prefix_pipeline(GBState *state, BYTE opcode) {
    CPUState *cpu = state->cpu;
    int act_on_hl_addr = 0;
    BYTE op_ls = ls_nib(opcode);
    BYTE op_ms = ms_nib(opcode);

    void (*exec_ptr)(GBState *) = _get_prefix_exec_ptr(opcode);

    switch (op_ls) {
        case 0x0:
        case 0x8:
            cpu->reg_dest = &reg_b(cpu);
            cpu->data1 = reg_b(cpu);
            cpu->pipeline[0] = exec_ptr;
            break;
        case 0x1:
        case 0x9:
            cpu->reg_dest = &reg_c(cpu);
            cpu->data1 = reg_c(cpu);
            cpu->pipeline[0] = exec_ptr;
            break;
        case 0x2:
        case 0xA:
            cpu->reg_dest = &reg_d(cpu);
            cpu->data1 = reg_d(cpu);
            cpu->pipeline[0] = exec_ptr;
            break;
        case 0x3:
        case 0xB:
            cpu->reg_dest = &reg_e(cpu);
            cpu->data1 = reg_e(cpu);
            cpu->pipeline[0] = exec_ptr;
            break;
        case 0x4:
        case 0xC:
            cpu->reg_dest = &reg_h(cpu);
            cpu->data1 = reg_h(cpu);
            cpu->pipeline[0] = exec_ptr;
            break;
        case 0x5:
        case 0xD:
            cpu->reg_dest = &reg_l(cpu);
            cpu->data1 = reg_l(cpu);
            cpu->pipeline[0] = exec_ptr;
            break;
        case 0x6:
        case 0xE:
            cpu->addr = reg_hl(cpu);
            cpu->reg_dest = &cpu->data1;
            act_on_hl_addr = 1;
            cpu->pipeline[0] = &_read_mem_l;
            cpu->pipeline[1] = exec_ptr;
            if (op_ms >= 4 && op_ms <= 7) {
                cpu->pipeline[2] = &_write_mem_data1;
            } else {
                cpu->pipeline[2] = &_nop;
                cpu->pipeline[3] = &_write_mem_data1;
            }
            break;
        case 0x7:
        case 0xF:
            cpu->reg_dest = &reg_a(cpu);
            cpu->data1 = reg_a(cpu);
            cpu->pipeline[0] = exec_ptr;
            break;
    }
    switch (op_ms) {
        case 0x0:
            CHECK_FLAGS(cpu, CHECK, CLEAR, CLEAR, CHECK);
            break;
        case 0x1:
            CHECK_FLAGS(cpu, CHECK, CLEAR, CLEAR, CHECK);
            break;
        case 0x2:
            CHECK_FLAGS(cpu, CHECK, CLEAR, CLEAR, CHECK);
            break;
        case 0x3:
            CHECK_FLAGS(cpu, CHECK, CLEAR, CLEAR, CLEAR);
            break;
        case 0x4:
            CHECK_FLAGS(cpu, CHECK, CLEAR, SET, NOCHANGE);
            break;
        case 0x5:
            CHECK_FLAGS(cpu, CHECK, CLEAR, SET, NOCHANGE);
            break;
        case 0x6:
            CHECK_FLAGS(cpu, CHECK, CLEAR, SET, NOCHANGE);
            break;
        case 0x7:
            CHECK_FLAGS(cpu, CHECK, CLEAR, SET, NOCHANGE);
            break;
        default:
            break;
    }

}