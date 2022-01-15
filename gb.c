#include "base.h"
#include "cpu.h"
#include <stdio.h>
#include <stdlib.h>


void execute_program(GBState *state) {

    char offset, input;
    BYTE scratch;

    BYTE *opcode;

    // Main loop
    for (;;) {

        if (state->pc > 0x0a) {
            input = getchar();
            print_state_info(state, (input=='i'));
        }

        opcode = &state->code[state->pc];
        state->pc = execute_instruction(state, opcode);

        if (state->flags.wants_ime > 0) {
            state->flags.wants_ime -= 1;
        }
        if (state->flags.wants_ime == 1) {
            state->flags.ime = 1;
            state->flags.wants_ime = 0;
        }
    }

}


#ifdef GB_MAIN

int main(int argc, char *argv[]) {
    GBState *state = initialize_state();

    FILE *fp;
    fp = fopen("gb-bootroms/bin/dmg.bin", "r");

    int n_read = fread(state->code, 1, 0x100, fp);
    printf("n read: %d\n",
        n_read);
    
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

    execute_program(state);

    return 0;
}

#endif