#include "base.h"
#include "macros.h"
#include "gb.c"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define TEST_EQUALS(__r) printf("test->" # __r " %x post->" # __r ": %x\n", \
    test_state->__r, post_state->__r); \
    if (test_state->__r != post_state->__r) { \
        failure = 1; \
        printf("FAILURE: " # __r "\n"); \
    }

int test_state(GBState *test_state, GBState *post_state, BYTE *opcode, WORD post_pc) {
    int new_pc = 0;
    int failure = 0;

    printf("Testing opcode %x", opcode[0]);
    if (strlen(opcode) == 2) {
        printf("[%x]", opcode[1]);
    } else if (strlen(opcode) == 3) {
        printf("[%x %x]", opcode[1], opcode[2]);
    }
    printf("\n");

    new_pc = execute_instruction(test_state, opcode);

    printf("next PC: %x, specified PC: %x\n", new_pc, post_pc);
    if (new_pc != post_pc) {
        printf("FAILURE: PC\n");
    }

    TEST_EQUALS(a);
    TEST_EQUALS(pc);
    TEST_EQUALS(sp);
    TEST_EQUALS(bc.w.h);
    TEST_EQUALS(bc.w.l);
    TEST_EQUALS(bc.dw);
    TEST_EQUALS(de.w.h);
    TEST_EQUALS(de.w.l);
    TEST_EQUALS(de.dw);
    TEST_EQUALS(hl.w.h);
    TEST_EQUALS(hl.w.l);
    TEST_EQUALS(hl.dw);
    TEST_EQUALS(flags.z);
    TEST_EQUALS(flags.n);
    TEST_EQUALS(flags.h);
    TEST_EQUALS(flags.c);
    TEST_EQUALS(flags.ime);
    TEST_EQUALS(flags.wants_ime);

    return failure;
}

int main(void) {
    BYTE opcode[3] = {0, 0, 0};
    
    GBState *test_state, *post_state;
    test_state = initialize_state();
    post_state = initialize_state();


    return 0;
}
