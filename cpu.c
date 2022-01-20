#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "base.h"
#include "mem.h"
#include "cpu.h"

/*
union {
    struct {
        unsigned char FIXED[32767];
        unsigned char HOME[32767];
        unsigned char VRAM[16383];
    } section;
    struct {

    }
} ADDR;*/

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
static inline void _copy_reg(GBState *state) {
    *(state->cpu->reg_dest) = *(state->cpu->reg_src);
}
static inline void _add_reg_data1(GBState *state) {
    state->cpu->result = *(state->cpu->reg_dest) + state->cpu->data1;
    *(state->cpu->reg_dest) += state->cpu->data1;
}
static inline void _sub_reg_data1(GBState *state) {
    state->cpu->result = *(state->cpu->reg_dest) - state->cpu->data1;
    *(state->cpu->reg_dest) -= state->cpu->data1;
}
static inline void _add_reg_reg(GBState *state) {
    state->cpu->result = *(state->cpu->reg_dest) + *(state->cpu->reg_src);
    *(state->cpu->reg_dest) += *(state->cpu->reg_src);
}
static inline void _sub_reg_reg(GBState *state) {
    state->cpu->result = *(state->cpu->reg_dest) - *(state->cpu->reg_src);
    *(state->cpu->reg_dest) -= *(state->cpu->reg_src);
}

/* Copy data from intermediate store to a register */
static inline void _write_reg_data(GBState *state) {
    if (state->cpu->is_16_bit)
        *(WORD*)(state->cpu->reg_dest) = b2w(state->cpu->data1, state->cpu->data2);
    else
        *(state->cpu->reg_dest) = state->cpu->data1;
}

/* Write data from a src register to a memory address */
static inline void _write_mem_reg_l(GBState *state) {
    write_mem(state, state->cpu->addr, *(state->cpu->reg_src));
}
static inline void _write_mem_reg_h(GBState *state) {
   write_mem(state, state->cpu->addr, *(state->cpu->reg_src + 1)); 
}
static inline void _set_addr_from_data(GBState *state) {
    state->cpu->addr = b2w(state->cpu->data1, state->cpu->data2);
}
/* Read data from src register to intermediate store */
static inline void _read_reg_l(GBState *state) {
    state->cpu->data1 = *(state->cpu->reg_src);
}
/* Read data from src register to high byte of intermediate store */
static inline void _read_reg_h(GBState *state) {
    state->cpu->data2 = *(state->cpu->reg_src + 1);
}
/* Read data from memory to intermediate store */
static inline void _read_mem_l(GBState *state) {
    state->cpu->data1 = read_mem(state, state->cpu->addr);
}
static inline void _read_mem_h(GBState *state) {
    state->cpu->data2 = read_mem(state, state->cpu->addr + 1);
}
static inline void _read_mem_h_and_store(GBState *state) {
    _read_mem_h(state);
    _write_reg_data(state); 
}
/* Increment the program counter */
static inline void _advance_pc(GBState *state) {
    reg_pc(state->cpu)++;
}
static inline void _decrement_sp(GBState *state) {
    reg_sp(state->cpu)--;
}

static inline void _nop(GBState *state) {
    
}

static inline void _fetch_inst(GBState *state) {
    state->cpu->addr = reg_pc(state->cpu);
    state->cpu->opcode = read_mem(state, state->cpu->addr);
    reg_pc(state->cpu)++;
}

static inline void _check_flags(GBState *state) {
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
            cpu->pipeline[1] = &_write_mem_data;
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
            cpu->pipeline[0] = &_rr_reg;
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

        case 0x1A:

        case 0x1B:

        case 0x1C:

        case 0x1D:

        case 0x1E:

        case 0x1F:
    }
    
}