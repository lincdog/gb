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
    cpu->state = READY;
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

/* Copy data from a src register to a dest register */
CYCLE_FUNC(_copy_reg) {
    *(state->cpu->reg_dest) = *(state->cpu->reg_src);
}
CYCLE_FUNC(_add_reg_data1) {
    state->cpu->result = *(state->cpu->reg_dest) + state->cpu->data1;
    *(state->cpu->reg_dest) += state->cpu->data1;
}
CYCLE_FUNC(_sub_reg_data1) {
    state->cpu->result = *(state->cpu->reg_dest) - state->cpu->data1;
    *(state->cpu->reg_dest) -= state->cpu->data1;
}
CYCLE_FUNC(_add_reg_reg) {
    state->cpu->result = *(state->cpu->reg_dest) + *(state->cpu->reg_src);
    *(state->cpu->reg_dest) += *(state->cpu->reg_src);
}
CYCLE_FUNC(_sub_reg_reg) {
    state->cpu->result = *(state->cpu->reg_dest) - *(state->cpu->reg_src);
    *(state->cpu->reg_dest) -= *(state->cpu->reg_src);
}
CYCLE_FUNC(_inc_data1) {
    state->cpu->data1++;
}
CYCLE_FUNC(_dec_data1) {
    state->cpu->data1--;
}
/* Copy data from intermediate store to a register */
CYCLE_FUNC(_write_reg_data) {
    if (state->cpu->is_16_bit)
        *(WORD*)(state->cpu->reg_dest) = b2w(state->cpu->data1, state->cpu->data2);
    else
        *(state->cpu->reg_dest) = state->cpu->data1;
}

/* Write data from a src register to a memory address */
CYCLE_FUNC(_write_mem_reg_l) {
    write_mem(state, state->cpu->addr, *(state->cpu->reg_src));
}
CYCLE_FUNC(_write_mem_reg_h) {
   write_mem(state, state->cpu->addr, *(state->cpu->reg_src + 1)); 
}
CYCLE_FUNC(_write_mem_data1) {
    write_mem(state, state->cpu->addr, state->cpu->data1);
}
CYCLE_FUNC(_set_addr_from_data) {
    state->cpu->addr = b2w(state->cpu->data1, state->cpu->data2);
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
CYCLE_FUNC(_read_mem_h_and_store) {
    _read_mem_h(state);
    _write_reg_data(state); 
}

CYCLE_FUNC(_decrement_sp) {
    reg_sp(state->cpu)--;
}

CYCLE_FUNC(_nop) {
    
}

CYCLE_FUNC(_fetch_inst) {
    state->cpu->addr = reg_pc(state->cpu);
    state->cpu->opcode = read_mem(state, state->cpu->addr);
    reg_pc(state->cpu)++;
}

CYCLE_FUNC(_check_flags) {
    CPUState *cpu = state->cpu;
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
            cpu->flags.c = (cpu->result > 0xFFFF) ? SET : CLEAR;
        else
            cpu->flags.c = (cpu->result > 0xFF) ? SET : CLEAR;
    } else if (chk_flags.c == FLIP) {
        cpu->flags.c = (cur_flags.c == SET) ? CLEAR : SET;
    }
}

void cpu_m_cycle(GBState *state) {
    CPUState *cpu = state->cpu;
    // Make a copy of the pending opcode before fetching the next one
    BYTE opcode = cpu->opcode;
    // counter is index into pipeline; if NULL,
    // no further actions are queued and we fetch an opcode.
    if (cpu->pipeline[cpu->counter] == NULL) {
        // Clear the counter, set all func ptrs to NULL, 
        // clear result, 16 bit flag
        reset_pipeline(cpu);
        // Read next instructin from memory at current PC location
        // load it into cpu->opcode and *increment PC*
        _fetch_inst(state);
        // Set up cycle queue based on this opcode
        cpu_setup_pipeline(state, cpu->opcode);
    } else {
        // Execute the next cycle step of the current queue
        (*cpu->pipeline[cpu->counter])(state);
        // Increment the cycle counter
        cpu->counter++;
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
            cpu->addr = reg_pc(cpu);
            cpu->reg_dest = &reg_bc(cpu);
            cpu->is_16_bit = 1;
            cpu->pipeline[0] = &_read_mem_l;
            cpu->pipeline[1] = &_read_mem_h;
            cpu->pipeline[2] = &_write_reg_data;
            break;
        case 0x02:
            cpu->addr = reg_bc(cpu);
            cpu->reg_src = &reg_a(cpu);
            cpu->pipeline[0] = &_nop; // TODO use cycle loading BC val to addr
            cpu->pipeline[1] = &_write_mem_l;
            break;
        case 0x03:
            cpu->reg_dest = &reg_bc(cpu);
            cpu->is_16_bit = 1;
            cpu->data1 = 1;
            
            cpu->pipeline[0] = &_nop;
            cpu->pipeline[1] = &_add_reg_data1;
            break;
        case 0x04:
            cpu->reg_dest = &reg_b(cpu);
            cpu->data1 = 1;
            CHECK_FLAGS(cpu, CHECK, CLEAR, CHECK, NOCHANGE);

            cpu->pipeline[0] = &_add_reg_data1;
            break;
        case 0x05:
            cpu->reg_dest = &reg_b(cpu);
            cpu->data1 = 1;
            CHECK_FLAGS(cpu, CHECK, SET, CHECK, NOCHANGE);
            
            cpu->pipeline[0] = &_sub_reg_data1;
            break;
        case 0x06:
            cpu->reg_dest = &reg_b(cpu);
            cpu->addr = reg_pc(cpu);
            cpu->pipeline[0] = &_read_mem_l;
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
            cpu->addr = reg_pc(cpu);
            cpu->is_16_bit = 1;
            cpu->pipeline[0] = &_read_mem_l;
            cpu->pipeline[1] = &_read_mem_h;
            cpu->pipeline[2] = &_set_addr_from_data;
            cpu->pipeline[3] = &_write_mem_l;
            cpu->pipeline[4] = &_write_mem_h;
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
            cpu->reg_dest = &reg_bc(cpu);
            cpu->data1 = 1;
            cpu->is_16_bit = 1;
            cpu->pipeline[0] = &_nop;
            cpu->pipeline[1] = &_sub_reg_reg;
            break;
        case 0x0C:
            cpu->reg_dest = &reg_c(cpu);
            cpu->data1 = 1;
            CHECK_FLAGS(cpu, CHECK, CLEAR, CHECK, NOCHANGE);
            cpu->pipeline[0] = &_add_reg_data1;
            break;
        case 0x0D:
            cpu->reg_dest = &reg_c(cpu);
            cpu->data1 = 1;
            CHECK_FLAGS(cpu, CHECK, SET, CHECK, NOCHANGE);
            cpu->pipeline[0] = &_sub_reg_data1;
            break;
        case 0x0E:
            cpu->reg_dest = &reg_c(cpu);
            cpu->addr = reg_pc(cpu);
            cpu->pipeline[0] = &_read_mem_l;
            cpu->pipeline[1] = &_write_reg_data;
            break;
        case 0x0F:
            cpu->reg_dest = &reg_a(cpu);
            cpu->pipeline[0] = &_rrc_reg;
            break;
        case 0x10:
            cpu->pipeline[0] = &_do_stop;
            break;
        case 0x11:
            cpu->addr = reg_pc(cpu);
            cpu->is_16_bit = 1;
            cpu->reg_dest = &reg_de(cpu);
            cpu->pipeline[0] = &_read_mem_l;
            cpu->pipeline[1] = &_read_mem_h;
            cpu->pipeline[2] = &_write_reg_data;
            break;
        case 0x12:
            cpu->addr = reg_de(cpu);
            cpu->reg_src = &reg_a(cpu);
            cpu->pipeline[0] = &_nop; // TODO use cycle loading BC val to addr
            cpu->pipeline[1] = &_write_mem_l;
            break;
        case 0x13:
            cpu->reg_dest = &reg_de(cpu);
            cpu->is_16_bit = 1;
            cpu->data1 = 1;
            cpu->pipeline[0] = &_nop;
            cpu->pipeline[1] = &_add_reg_data1;
            break;
        case 0x14:
            cpu->reg_dest = &reg_d(cpu);
            cpu->data1 = 1;
            CHECK_FLAGS(cpu, CHECK, CLEAR, CHECK, NOCHANGE);

            cpu->pipeline[0] = &_add_reg_data1;
            break;
        case 0x15:
            cpu->reg_dest = &reg_d(cpu);
            cpu->data1 = 1;
            CHECK_FLAGS(cpu, CHECK, CLEAR, CHECK, NOCHANGE);

            cpu->pipeline[0] = &_sub_reg_data1;
            break;
        case 0x16:
            cpu->reg_dest = &reg_d(cpu);
            cpu->addr = reg_pc(cpu);
            cpu->pipeline[0] = &_read_mem_l;
            cpu->pipeline[1] = &_write_reg_data;
            break;
        case 0x17:
            cpu->reg_dest = &reg_a(cpu);
            CHECK_FLAGS(cpu, CLEAR, CLEAR, CLEAR, CHECK);
            cpu->pipeline[0] = &_rl_reg;
            break;

        case 0x18:
            cpu->reg_dest = &reg_pc(cpu);
            cpu->addr = reg_pc(cpu);
            cpu->is_16_bit = 1;
            cpu->pipeline[0] = &_read_mem_l;
            cpu->pipeline[1] = &_nop;
            cpu->pipeline[2] = &_add_reg_data_signed;
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
            cpu->reg_dest = &reg_de(cpu);
            cpu->data1 = 1;
            cpu->is_16_bit = 1;
            cpu->pipeline[0] = &_nop;
            cpu->pipeline[1] = &_sub_reg_reg;
            break;
        case 0x1C:
            cpu->reg_dest = &reg_e(cpu);
            cpu->data1 = 1;
            CHECK_FLAGS(cpu, CHECK, CLEAR, CHECK, NOCHANGE);
            cpu->pipeline[0] = &_add_reg_data1;
            break;
        case 0x1D:
            cpu->reg_dest = &reg_c(cpu);
            cpu->data1 = 1;
            CHECK_FLAGS(cpu, CHECK, SET, CHECK, NOCHANGE);
            cpu->pipeline[0] = &_sub_reg_data1;
            break;
        case 0x1E:
            cpu->reg_dest = &reg_e(cpu);
            cpu->addr = reg_pc(cpu);
            cpu->pipeline[0] = &_read_mem_l;
            cpu->pipeline[1] = &_write_reg_data;
            break;
        case 0x1F:
            cpu->reg_dest = &reg_a(cpu);
            cpu->pipeline[0] = &_rr_reg;
            break; 
        case 0x20:
            // Conditional jump: JR NZ
            cpu->reg_dest = &reg_pc(cpu);
            cpu->addr = reg_pc(cpu);
            cpu->is_16_bit = 1;
            cpu->pipeline[0] = &_read_mem_l;
            cpu->pipeline[1] = &_nop;
            if (cpu->flags.z == CLEAR) {
                cpu->pipeline[2] = &_add_reg_data_signed;
            }
            break;
         case 0x21:
            cpu->addr = reg_pc(cpu);
            cpu->is_16_bit = 1;
            cpu->reg_dest = &reg_hl(cpu);
            cpu->pipeline[0] = &_read_mem_l;
            cpu->pipeline[1] = &_read_mem_h;
            cpu->pipeline[2] = &_write_reg_data;
            break;
        case 0x22:
            cpu->addr = reg_hl(cpu);
            cpu->reg_dest = &reg_hl(cpu);
            cpu->reg_src = &reg_a(cpu);
            cpu->pipeline[0] = &_nop; // TODO use cycle loading BC val to addr
            cpu->pipeline[1] = &_write_mem_l_inc_dest;
            break;
        case 0x23:
            cpu->reg_dest = &reg_hl(cpu);
            cpu->is_16_bit = 1;
            cpu->data1 = 1;
            cpu->pipeline[0] = &_nop;
            cpu->pipeline[1] = &_add_reg_data1;
            break;
        case 0x24:
            cpu->reg_dest = &reg_h(cpu);
            cpu->data1 = 1;
            CHECK_FLAGS(cpu, CHECK, CLEAR, CHECK, NOCHANGE);

            cpu->pipeline[0] = &_add_reg_data1;
            break;
        case 0x25:
            cpu->reg_dest = &reg_h(cpu);
            cpu->data1 = 1;
            CHECK_FLAGS(cpu, CHECK, CLEAR, CHECK, NOCHANGE);

            cpu->pipeline[0] = &_sub_reg_data1;
            break;
        case 0x26:
            cpu->reg_dest = &reg_h(cpu);
            cpu->addr = reg_pc(cpu);
            cpu->pipeline[0] = &_read_mem_l;
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
            cpu->addr = reg_pc(cpu);
            cpu->is_16_bit = 1;
            cpu->pipeline[0] = &_read_mem_l;
            cpu->pipeline[1] = &_nop;
            if (cpu->flags.z == SET) {
                cpu->pipeline[2] = &_add_reg_data_signed;
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
            cpu->reg_src = &reg_hl(cpu);
            cpu->addr = reg_hl(cpu);
            cpu->pipeline[0] = &_read_mem_l;
            cpu->pipeline[1] = &_write_reg_data_inc_src;
            break;
        case 0x2B:
            cpu->reg_dest = &reg_hl(cpu);
            cpu->data1 = 1;
            cpu->is_16_bit = 1;
            cpu->pipeline[0] = &_nop;
            cpu->pipeline[1] = &_sub_reg_reg;
            break;
        case 0x2C:
            cpu->reg_dest = &reg_l(cpu);
            cpu->data1 = 1;
            CHECK_FLAGS(cpu, CHECK, CLEAR, CHECK, NOCHANGE);
            cpu->pipeline[0] = &_add_reg_data1;
            break;
        case 0x2D:
            cpu->reg_dest = &reg_l(cpu);
            cpu->data1 = 1;
            CHECK_FLAGS(cpu, CHECK, SET, CHECK, NOCHANGE);
            cpu->pipeline[0] = &_sub_reg_data1;
            break;
        case 0x2E:
            cpu->reg_dest = &reg_l(cpu);
            cpu->addr = reg_pc(cpu);
            cpu->pipeline[0] = &_read_mem_l;
            cpu->pipeline[1] = &_write_reg_data;
            break;
        case 0x2F:
            cpu->reg_dest = &reg_a(cpu);
            CHECK_FLAGS(cpu, NOCHANGE, SET, SET, NOCHANGE);
            cpu->pipeline[0] = &_do_cpl;
            break;
        case 0x30:
            cpu->reg_dest = &reg_pc(cpu);
            cpu->addr = reg_pc(cpu);
            cpu->is_16_bit = 1;
            cpu->pipeline[0] = &_read_mem_l;
            cpu->pipeline[1] = &_nop;
            if (cpu->flags.c == CLEAR) {
                cpu->pipeline[2] = &_add_reg_data_signed;
            }
            break;
        case 0x31:
            cpu->addr = reg_pc(cpu);
            cpu->is_16_bit = 1;
            cpu->reg_dest = &reg_sp(cpu);
            cpu->pipeline[0] = &_read_mem_l;
            cpu->pipeline[1] = &_read_mem_h;
            cpu->pipeline[2] = &_write_reg_data;
            break;
        case 0x32:
            cpu->addr = reg_hl(cpu);
            cpu->reg_dest = &reg_hl(cpu);
            cpu->reg_src = &reg_a(cpu);
            cpu->pipeline[0] = &_nop; // TODO use cycle loading BC val to addr
            cpu->pipeline[1] = &_write_mem_l_dec_dest;
            break;
        case 0x33:
            cpu->reg_dest = &reg_sp(cpu);
            cpu->is_16_bit = 1;
            cpu->data1 = 1;
            cpu->pipeline[0] = &_nop;
            cpu->pipeline[1] = &_add_reg_data1;
            break;
        case 0x34:
            cpu->addr = reg_hl(cpu);
            CHECK_FLAGS(cpu, CHECK, CLEAR, CHECK, NOCHANGE);
            cpu->pipeline[0] = &_read_mem_l;
            cpu->pipeline[1] = &_inc_data1;
            cpu->pipeline[2] = &_write_mem_data1;
            break;
        case 0x35:
            cpu->addr = &reg_hl(cpu);
            cpu->data1 = 1;
            CHECK_FLAGS(cpu, CHECK, CLEAR, CHECK, NOCHANGE);

            cpu->pipeline[0] = &_read_mem_l;
            cpu->pipeline[1] = &_dec_data1;
            cpu->pipeline[2] = &_write_mem_data1; 
            break;
        case 0x36:
            cpu->reg_dest = &reg_hl(cpu);
            cpu->addr = reg_pc(cpu);
            cpu->pipeline[0] = &_nop;
            cpu->pipeline[1] = &_read_mem_l;
            // Writes data1 to the address specified by *(cpu->reg_dest)
            cpu->pipeline[2] = &_write_data1_to_dest;
            break;
        case 0x37:
            //SCF
            CHECK_FLAGS(cpu, NOCHANGE, CLEAR, CLEAR, SET);
            cpu->pipeline[0] = &_check_flags;
            break;
        case 0x38:
            cpu->reg_dest = &reg_pc(cpu);
            cpu->addr = reg_pc(cpu);
            cpu->is_16_bit = 1;
            cpu->pipeline[0] = &_read_mem_l;
            cpu->pipeline[1] = &_nop;
            if (cpu->flags.c == SET) {
                cpu->pipeline[2] = &_add_reg_data_signed;
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
            cpu->reg_src = &reg_hl(cpu);
            cpu->addr = reg_hl(cpu);
            cpu->pipeline[0] = &_read_mem_l;
            cpu->pipeline[1] = &_write_reg_data_dec_src;
            break;
        case 0x3B:
            cpu->reg_dest = &reg_sp(cpu);
            cpu->data1 = 1;
            cpu->is_16_bit = 1;
            cpu->pipeline[0] = &_nop;
            cpu->pipeline[1] = &_sub_reg_reg;
            break;
        case 0x3C:
            cpu->reg_dest = &reg_a(cpu);
            cpu->data1 = 1;
            CHECK_FLAGS(cpu, CHECK, CLEAR, CHECK, NOCHANGE);
            cpu->pipeline[0] = &_add_reg_data1;
            break;
        case 0x3D:
            cpu->reg_dest = &reg_a(cpu);
            cpu->data1 = 1;
            CHECK_FLAGS(cpu, CHECK, SET, CHECK, NOCHANGE);
            cpu->pipeline[0] = &_sub_reg_data1;
            break;
        case 0x3E:
            cpu->reg_dest = &reg_a(cpu);
            cpu->addr = reg_pc(cpu);
            cpu->pipeline[0] = &_read_mem_l;
            cpu->pipeline[1] = &_write_reg_data;
            break;
        case 0x3F:
            // CCF
            CHECK_FLAGS(cpu, NOCHANGE, CLEAR, CLEAR, FLIP);
            cpu->pipeline[0] = &_do_ccf;
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
            

    }

    
}