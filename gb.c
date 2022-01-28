#include "base.h"
#include "cpu.h"
#include "mem.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/*
void execute_program(CPUState *cpu) {

    char offset, input;
    BYTE scratch;

    BYTE *opcode;

    // Main loop
    for (;;) {

        if (reg_pc(cpu) > 0x0a) {
            input = getchar();
            print_state_info(cpu, (input=='i'));
        }

        opcode = &cpu->code[reg_pc(cpu)];
        reg_pc(cpu) = execute_instruction(cpu, opcode);

        if (cpu->flags.wants_ime > 0) {
            cpu->flags.wants_ime -= 1;
        }
        if (cpu->flags.wants_ime == 1) {
            cpu->flags.ime = 1;
            cpu->flags.wants_ime = 0;
        }
    }

}*/

GBState *initialize_gb(void) {
    GBState *state = malloc(sizeof(GBState));
    state->cpu = initialize_cpu();
    state->counter = 0;

    BYTE *code = malloc(0x10000);
    state->code = memset(code, 0xFF, 0x10000);

    state->io_regs = (IORegs *)&state->code[0xFF00];
    
    memcpy(&state->code[0x104], 
        &GAMEBOY_LOGO, 
        sizeof(GAMEBOY_LOGO));
    return state;
}

void teardown_gb(GBState *state) {
    teardown_cpu(state->cpu);
    free(state->code);
    free(state);
}

int _dummy(void) { return 1; }

void main_loop(GBState *state, int n_cycles) {
    
    while ((n_cycles < 0) || (state->counter < n_cycles)) {
        if (state->counter % 4 == 0) {
            cpu_m_cycle(state);
        }

        if (state->cpu->r.pc > 34) {
            _dummy();
        }

        // check clock % 4; fetch instruction
        // check clock % 4; execute current instruction 
        // run PPU code
        // check interrupts

        state->counter++;
    }
}

#ifdef GB_MAIN

int main(int argc, char *argv[]) {
    GBState *state = initialize_gb();
    
    FILE *fp;
    fp = fopen("gb-bootroms/bin/dmg.bin", "r");

    int n_read = fread(state->code, 1, 0x100, fp);
    printf("n read: %d\n", n_read);
    
    for (int i = 0; i < n_read; i++) {
        if (i % 16 == 0) {
            printf("\n%02x\t", i);
        }
        printf("%02x ", state->code[i]);
    }
    printf("\n");

    for (int i = 0x100; i < 0x133; i++) {
        if (i % 16 == 0) {
            printf("\n%02x\t", i);
        }
        printf("%02x ", state->code[i]); 
    }

    fclose(fp);

    print_state_info(state, 1);

    main_loop(state, -1);
    

    teardown_gb(state);
    return 0;
}

#endif