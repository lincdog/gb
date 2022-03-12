#include "base.h"
#include "cpu.h"
#include "cpu.c"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define IO_CHECK_START 0xFF00
#define IO_CHECK_STOP 0xFF80
#define STACK_CHECK_START 0xFF80
#define STACK_CHECK_STOP 0xFFFF
#define STACK_CHECK_SIZE (STACK_CHECK_STOP - STACK_CHECK_START)
#define MEM_CHECK_START 0x8000
#define MEM_CHECK_STOP  0xDFFF
#define MEM_CHECK_SIZE (MEM_CHECK_STOP - MEM_CHECK_START)

typedef struct {
    GBState *test_state;
    GBState *post_state;
    BYTE *test_mem;
    BYTE *post_mem;
    CPUState *test_cpu;
    CPUState *post_cpu;
    BYTE opcode[3];
    int opcode_length;
    int n_m_cycles;
    int n_tests;
    int n_prefix_tests;
    int n_failures;
    int n_prefix_failures;
    int status;
    BYTE *covered_opcodes;
    BYTE *covered_prefix_opcodes;
    BYTE *failed_opcodes;
    BYTE *failed_prefix_opcodes;
} CPUTestState;

#define TEST_EQUALS(__test, __des, __r) \
    if (reg_ ## __r(__test) != reg_ ## __r(__des)) { \
        printf("FAILURE: test->" # __r ": %04x\t post->" # __r ": %04x\n", \
        reg_ ## __r(__test), reg_ ## __r(__des)); \
        failure = 1; \
    }

#define TEST_FLAG_EQUALS(__test, __des, __f) \
    if (__test->flags.__f != __des->flags.__f) { \
        printf("FLAG FAILURE: test->flags." # __f": %s\t post->" #__f ": %s\n", \
    __test->flags.__f == SET ? "SET" : "CLEAR", \
    __des->flags.__f == SET ? "SET" : "CLEAR"); \
        failure = 1; \
    }

void test_instruction(CPUTestState *tstate) {
    int failure = 0;

    
    printf("Testing opcode %02x", tstate->opcode[0]);
    if (tstate->opcode_length == 2) {
        printf(" [%02x]", tstate->opcode[1]);
    } else if (tstate->opcode_length == 3) {
        printf(" [%02x %02x]", tstate->opcode[1], tstate->opcode[2]);
    }
    printf("\n");
    
    CPUState *test_cpu = tstate->test_cpu;
    CPUState *post_cpu = tstate->post_cpu;

    /* Note we add 1 to n_m_cycles because the first cycle is only fetching
    the first opcode. */
    for (int i = 0; i < tstate->n_m_cycles + 1; i++) {
        task_cpu_m_cycle(tstate->test_state);
    }

    TEST_EQUALS(test_cpu, post_cpu, a);
    TEST_EQUALS(test_cpu, post_cpu, pc);
    TEST_EQUALS(test_cpu, post_cpu, sp);
    TEST_EQUALS(test_cpu, post_cpu, b);
    TEST_EQUALS(test_cpu, post_cpu, c);
    TEST_EQUALS(test_cpu, post_cpu, bc);
    TEST_EQUALS(test_cpu, post_cpu, d);
    TEST_EQUALS(test_cpu, post_cpu, e);
    TEST_EQUALS(test_cpu, post_cpu, de);
    TEST_EQUALS(test_cpu, post_cpu, hl);
    TEST_EQUALS(test_cpu, post_cpu, h);
    TEST_EQUALS(test_cpu, post_cpu, l);
    TEST_FLAG_EQUALS(test_cpu, post_cpu, z);
    TEST_FLAG_EQUALS(test_cpu, post_cpu, n);
    TEST_FLAG_EQUALS(test_cpu, post_cpu, h);
    TEST_FLAG_EQUALS(test_cpu, post_cpu, c);
    TEST_FLAG_EQUALS(test_cpu, post_cpu, ime);
    TEST_FLAG_EQUALS(test_cpu, post_cpu, wants_ime);

    for (WORD i = MEM_CHECK_START; i < MEM_CHECK_STOP; i++) {
        if (tstate->test_mem[i] != 
            tstate->post_mem[i]) {
            failure++;
            printf("MEM FAILURE: code[0x%04x]: (test) %02x (post) %02x\n",
                i, tstate->test_mem[i], tstate->post_mem[i]);
        }
    }

    for (WORD i = STACK_CHECK_START; i < STACK_CHECK_STOP; i++) {
        if (tstate->test_mem[i] != 
            tstate->post_mem[i]) {
            failure++;
            printf("STACK FAILURE: code[0x%04x]: (test) %02x (post) %02x\n",
                i, tstate->test_mem[i], tstate->post_mem[i]);
        }
    }

    for (WORD i = IO_CHECK_START; i < IO_CHECK_STOP; i++) {
        if (tstate->test_mem[i] != 
            tstate->post_mem[i]) {
            failure++;
            printf("IOREG FAILURE: code[0x%04x]: (test) %02x (post) %02x\n",
                i, tstate->test_mem[i], tstate->post_mem[i]);
        }
    }

    if (failure) {
        if (tstate->opcode[0] == 0xCB)
            tstate->failed_prefix_opcodes[tstate->n_prefix_failures++] = tstate->opcode[1];
        else
            tstate->failed_opcodes[tstate->n_failures++] = tstate->opcode[0];
    }
}

int _test_dummy(void) { return 1; }

void stage_test(
    CPUTestState *tstate, 
    BYTE op0, BYTE op1, BYTE op2, 
    int n_m_cycles, 
    int opcode_length
) {
    if (op0 == 0xCB && op1 == 0x06)
        _test_dummy();

    tstate->opcode_length = opcode_length;
    tstate->n_m_cycles = n_m_cycles;

    reset_registers(tstate->test_cpu);
    reset_pipeline(tstate->test_cpu);
    tstate->test_cpu->state = PREINIT;

    reset_registers(tstate->post_cpu);
    reset_pipeline(tstate->post_cpu);
    tstate->post_cpu->state = PREINIT;

    tstate->opcode[0] = op0;
    tstate->opcode[1] = op1;
    tstate->opcode[2] = op2;

    reg_pc(tstate->post_cpu) = opcode_length + 1;

    if (op0 == 0xCB)
        tstate->covered_prefix_opcodes[tstate->n_prefix_tests++] = op1;
    else
        tstate->covered_opcodes[tstate->n_tests++] = op0;

    memset(tstate->test_mem, UNINIT, 0x10000);
    memset(tstate->post_mem, UNINIT, 0x10000);

    for (int i = 0; i < tstate->opcode_length; i++) {
        tstate->test_mem[i] = tstate->opcode[i];
        tstate->post_mem[i] = tstate->opcode[i];
    }
    
}

/* Stage and run a test. __setup_code is any arbitrary statements that set up the
post state, to which the test state is compared after executing the cycles. */
#define DO_TEST(__tstate, op0, op1, op2, __len_op, __n_cyc, __setup_code) \
    stage_test(__tstate, op0, op1, op2, __n_cyc, __len_op); \
    __setup_code; \
    test_instruction(__tstate);

#define SET_ZNHC(__post, __z, __n, __h, __c) \
    __post->flags.z = __z; \
    __post->flags.n = __n; \
    __post->flags.h = __h; \
    __post->flags.c = __c;


CPUTestState *initialize_cpu_tests(GBState *test_state, GBState *post_state) {

    CPUTestState *tstate = malloc(sizeof(CPUTestState));
    if (tstate == NULL) {
        printf("FAILED TO ALLOCATE CPUTESTSTATE\n");
        return NULL;
    }

    tstate->test_state = test_state;
    tstate->post_state = post_state;
    tstate->test_cpu = test_state->cpu;
    tstate->post_cpu = post_state->cpu;
    tstate->test_mem = ((DebugMemState *)test_state->mem->system->state)->mem;
    tstate->post_mem = ((DebugMemState *)post_state->mem->system->state)->mem;
    tstate->opcode[0] = 0x0;
    tstate->opcode[1] = 0x0;
    tstate->opcode[2] = 0x0;
    tstate->opcode_length = 0;
    tstate->n_tests = 0;
    tstate->n_prefix_tests = 0;
    tstate->n_m_cycles = 0;
    tstate->n_failures = 0;
    tstate->n_prefix_failures = 0;
    tstate->status = 0;
    tstate->failed_opcodes = malloc(512);
    tstate->failed_prefix_opcodes = malloc(512);
    tstate->covered_opcodes = malloc(512);
    tstate->covered_prefix_opcodes = malloc(512);

    return tstate;
}

void teardown_cpu_tests(CPUTestState *tstate) {
    free(tstate->failed_opcodes);
    free(tstate->failed_prefix_opcodes);
    free(tstate->covered_opcodes);
    free(tstate->covered_prefix_opcodes);
    free(tstate);
}

void run_ld_16_imm(CPUTestState *tstate) {
    CPUState *test_cpu = tstate->test_cpu;
    CPUState *post_cpu = tstate->post_cpu;
    BYTE *test_mem = tstate->test_mem;
    BYTE *post_mem = tstate->post_mem; 
    DO_TEST(tstate, 0x01, 0xAD, 0xDE, 3, 3, 
        reg_bc(post_cpu) = 0xDEAD;
    );
    DO_TEST(tstate, 0x11, 0xAD, 0xDE, 3, 3, 
        reg_de(post_cpu) = 0xDEAD;
    );
    DO_TEST(tstate, 0x21, 0xAD, 0xDE, 3, 3, 
        reg_hl(post_cpu) = 0xDEAD;
    );
    DO_TEST(tstate, 0x31, 0xAD, 0xDE, 3, 3, 
        reg_sp(post_cpu) = 0xDEAD;
    );
}

void run_wr_r16_a(CPUTestState *tstate) {
    CPUState *test_cpu = tstate->test_cpu;
    CPUState *post_cpu = tstate->post_cpu;
    BYTE *test_mem = tstate->test_mem;
    BYTE *post_mem = tstate->post_mem; 

    DO_TEST(tstate, 0x02, 0x00, 0x00, 1, 2,
        reg_a(test_cpu) = 0xDA;
        reg_a(post_cpu) = 0xDA;
        reg_bc(test_cpu) = 0x8010;
        reg_bc(post_cpu) = 0x8010;
        post_mem[0x8010] = 0xDA;
    );
    DO_TEST(tstate, 0x12, 0x00, 0x00, 1, 2,
        reg_a(test_cpu) = 0xDA;
        reg_a(post_cpu) = 0xDA;
        reg_de(test_cpu) = 0x8010;
        reg_de(post_cpu) = 0x8010;
        post_mem[0x8010] = 0xDA;
    );
    DO_TEST(tstate, 0x22, 0x00, 0x00, 1, 2,
        reg_a(test_cpu) = 0xDA;
        reg_a(post_cpu) = 0xDA;
        reg_hl(test_cpu) = 0x8010;
        reg_hl(post_cpu) = 0x8011;
        post_mem[0x8010] = 0xDA;
    );
    DO_TEST(tstate, 0x32, 0x00, 0x00, 1, 2,
        reg_a(test_cpu) = 0xDA;
        reg_a(post_cpu) = 0xDA;
        reg_hl(test_cpu) = 0x8010;
        reg_hl(post_cpu) = 0x800F;
        post_mem[0x8010] = 0xDA;
    );
}

void run_inc_dec_r16(CPUTestState *tstate) {
    CPUState *test_cpu = tstate->test_cpu;
    CPUState *post_cpu = tstate->post_cpu;
    BYTE *test_mem = tstate->test_mem;
    BYTE *post_mem = tstate->post_mem;

    DO_TEST(tstate, 0x03, 0x00, 0x00, 1, 2,
        reg_bc(test_cpu) = 0x103;
        reg_bc(post_cpu) = 0x104;
    );
    DO_TEST(tstate, 0x13, 0x00, 0x00, 1, 2,
        reg_de(test_cpu) = 0x103;
        reg_de(post_cpu) = 0x104;
    );
    DO_TEST(tstate, 0x23, 0x00, 0x00, 1, 2,
        reg_hl(test_cpu) = 0x103;
        reg_hl(post_cpu) = 0x104;
    );
    DO_TEST(tstate, 0x33, 0x00, 0x00, 1, 2,
        reg_sp(test_cpu) = 0x103;
        reg_sp(post_cpu) = 0x104;
    );
    /* DEC <r16> */
    DO_TEST(tstate, 0x0B, 0x00, 0x00, 1, 2,
        reg_bc(test_cpu) = 0x104;
        reg_bc(post_cpu) = 0x103;
    );
    DO_TEST(tstate, 0x1B, 0x00, 0x00, 1, 2,
        reg_de(test_cpu) = 0x104;
        reg_de(post_cpu) = 0x103;
    );
    DO_TEST(tstate, 0x2B, 0x00, 0x00, 1, 2,
        reg_hl(test_cpu) = 0x104;
        reg_hl(post_cpu) = 0x103;
    );
    DO_TEST(tstate, 0x3B, 0x00, 0x00, 1, 2,
        reg_sp(test_cpu) = 0x104;
        reg_sp(post_cpu) = 0x103;
    );
}

void run_inc_dec_r8(CPUTestState *tstate) {
    CPUState *test_cpu = tstate->test_cpu;
    CPUState *post_cpu = tstate->post_cpu;
    BYTE *test_mem = tstate->test_mem;
    BYTE *post_mem = tstate->post_mem;

    DO_TEST(tstate, 0x04, 0x00, 0x00, 1, 1,
        reg_b(test_cpu) = 0x09;
        reg_b(post_cpu) = 0x0A;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, SET, CLEAR);
    );
    DO_TEST(tstate, 0x14, 0x00, 0x00, 1, 1,
        reg_d(test_cpu) = 0x09;
        reg_d(post_cpu) = 0x0A;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, SET, CLEAR);
    );
    DO_TEST(tstate, 0x24, 0x00, 0x00, 1, 1,
        reg_h(test_cpu) = 0x09;
        reg_h(post_cpu) = 0x0A;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, SET, CLEAR);
    );
    DO_TEST(tstate, 0x34, 0x00, 0x00, 1, 3,
        reg_hl(test_cpu) = 0x8010;
        test_mem[0x8010] = 0xA;
        reg_hl(post_cpu) = 0x8010;
        post_mem[0x8010] = 0xB;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, SET, CLEAR);
    );
    DO_TEST(tstate, 0x0C, 0x00, 0x00, 1, 1,
        reg_c(test_cpu) = 0x09;
        reg_c(post_cpu) = 0x0A;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, SET, CLEAR);
    );
    DO_TEST(tstate, 0x1C, 0x00, 0x00, 1, 1,
        reg_e(test_cpu) = 0x09;
        reg_e(post_cpu) = 0x0A;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, SET, CLEAR);
    );
    DO_TEST(tstate, 0x2C, 0x00, 0x00, 1, 1,
        reg_l(test_cpu) = 0x09;
        reg_l(post_cpu) = 0x0A;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, SET, CLEAR);
    );
    DO_TEST(tstate, 0x3C, 0x00, 0x00, 1, 1,
        reg_a(test_cpu) = 0x09;
        reg_a(post_cpu) = 0x0A;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, SET, CLEAR);
    );
    /* DEC <r8> */
    DO_TEST(tstate, 0x05, 0x00, 0x00, 1, 1,
        reg_b(test_cpu) = 0x0A;
        reg_b(post_cpu) = 0x09;
        SET_ZNHC(post_cpu, CLEAR, SET, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0x15, 0x00, 0x00, 1, 1,
        reg_d(test_cpu) = 0x0A;
        reg_d(post_cpu) = 0x09;
        SET_ZNHC(post_cpu, CLEAR, SET, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0x25, 0x00, 0x00, 1, 1,
        reg_h(test_cpu) = 0x0A;
        reg_h(post_cpu) = 0x09;
        SET_ZNHC(post_cpu, CLEAR, SET, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0x35, 0x00, 0x00, 1, 3,
        reg_hl(test_cpu) = 0x8010;
        test_mem[0x8010] = 0xF;
        reg_hl(post_cpu) = 0x8010;
        post_mem[0x8010] = 0xE;
        SET_ZNHC(post_cpu, CLEAR, SET, SET, CLEAR);
    );
    DO_TEST(tstate, 0x0D, 0x00, 0x00, 1, 1,
        reg_c(test_cpu) = 0x0A;
        reg_c(post_cpu) = 0x09;
        SET_ZNHC(post_cpu, CLEAR, SET, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0x1D, 0x00, 0x00, 1, 1,
        reg_e(test_cpu) = 0x0A;
        reg_e(post_cpu) = 0x09;
        SET_ZNHC(post_cpu, CLEAR, SET, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0x2D, 0x00, 0x00, 1, 1,
        reg_l(test_cpu) = 0x0A;
        reg_l(post_cpu) = 0x09;
        SET_ZNHC(post_cpu, CLEAR, SET, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0x3D, 0x00, 0x00, 1, 1,
        reg_a(test_cpu) = 0x0A;
        reg_a(post_cpu) = 0x09;
        SET_ZNHC(post_cpu, CLEAR, SET, CLEAR, CLEAR);
    );
}

void run_ld_imm8(CPUTestState *tstate) {
    CPUState *test_cpu = tstate->test_cpu;
    CPUState *post_cpu = tstate->post_cpu;
    BYTE *test_mem = tstate->test_mem;
    BYTE *post_mem = tstate->post_mem;

    /* LD <r8>, d8 */
    DO_TEST(tstate, 0x06, 0xDE, 0x00, 2, 2,
        reg_b(post_cpu) = 0xDE;
    );
    DO_TEST(tstate, 0x16, 0xDE, 0x00, 2, 2,
        reg_d(post_cpu) = 0xDE;
    );
    DO_TEST(tstate, 0x26, 0xDE, 0x00, 2, 2,
        reg_h(post_cpu) = 0xDE;
    );
    DO_TEST(tstate, 0x36, 0xDE, 0x00, 2, 3,
        reg_hl(test_cpu) = 0x8010;
        reg_hl(post_cpu) = 0x8010;
        post_mem[0x8010] = 0xDE;
    );
    DO_TEST(tstate, 0x0E, 0xDE, 0x00, 2, 2,
        reg_c(post_cpu) = 0xDE;
    );
    DO_TEST(tstate, 0x1E, 0xDE, 0x00, 2, 2,
        reg_e(post_cpu) = 0xDE;
    );
    DO_TEST(tstate, 0x2E, 0xDE, 0x00, 2, 2,
        reg_l(post_cpu) = 0xDE;
    );
    DO_TEST(tstate, 0x3E, 0xDE, 0x00, 2, 2,
        reg_a(post_cpu) = 0xDE;
    );
}

void run_add_hl_r16(CPUTestState *tstate) {
    CPUState *test_cpu = tstate->test_cpu;
    CPUState *post_cpu = tstate->post_cpu;
    BYTE *test_mem = tstate->test_mem;
    BYTE *post_mem = tstate->post_mem;

    DO_TEST(tstate, 0x09, 0x00, 0x00, 1, 2,
        reg_hl(test_cpu) = 0x2001;
        reg_bc(test_cpu) = 0x3019;
        reg_hl(post_cpu) = 0x501A;
        reg_bc(post_cpu) = 0x3019;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, SET, CLEAR);
    );
    DO_TEST(tstate, 0x19, 0x00, 0x00, 1, 2,
        reg_hl(test_cpu) = 0x2001;
        reg_de(test_cpu) = 0x3019;
        reg_hl(post_cpu) = 0x501A;
        reg_de(post_cpu) = 0x3019;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, SET, CLEAR);
    );
    DO_TEST(tstate, 0x29, 0x00, 0x00, 1, 2,
        reg_hl(test_cpu) = 0x3019;
        reg_hl(post_cpu) = 0x6032;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, SET, CLEAR);
    );
    DO_TEST(tstate, 0x39, 0x00, 0x00, 1, 2,
        reg_hl(test_cpu) = 0x2001;
        reg_sp(test_cpu) = 0x3019;
        reg_hl(post_cpu) = 0x501A;
        reg_sp(post_cpu) = 0x3019;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, SET, CLEAR);
    );

}

void run_ld_a_r16_mem(CPUTestState *tstate) {
    CPUState *test_cpu = tstate->test_cpu;
    CPUState *post_cpu = tstate->post_cpu;
    BYTE *test_mem = tstate->test_mem;
    BYTE *post_mem = tstate->post_mem; 

    DO_TEST(tstate, 0x0A, 0x00, 0x00, 1, 2,
        reg_bc(test_cpu) = 0x8010;
        test_mem[0x8010] = 0xDE;
        reg_bc(post_cpu) = 0x8010;
        post_mem[0x8010] = 0xDE;
        reg_a(post_cpu) = 0xDE;
    );
    DO_TEST(tstate, 0x1A, 0x00, 0x00, 1, 2,
        reg_de(test_cpu) = 0x8010;
        test_mem[0x8010] = 0xDE;
        reg_de(post_cpu) = 0x8010;
        post_mem[0x8010] = 0xDE;
        reg_a(post_cpu) = 0xDE;
    );
    DO_TEST(tstate, 0x2A, 0x00, 0x00, 1, 2,
        reg_hl(test_cpu) = 0x8010;
        test_mem[0x8010] = 0xDE;
        reg_hl(post_cpu) = 0x8011;
        post_mem[0x8010] = 0xDE;
        reg_a(post_cpu) = 0xDE;
    );
    DO_TEST(tstate, 0x3A, 0x00, 0x00, 1, 2,
        reg_hl(test_cpu) = 0x8010;
        test_mem[0x8010] = 0xDE;
        reg_hl(post_cpu) = 0x800F;
        post_mem[0x8010] = 0xDE;
        reg_a(post_cpu) = 0xDE;
    );
}

void run_rel_jmp(CPUTestState *tstate) {
    CPUState *test_cpu = tstate->test_cpu;
    CPUState *post_cpu = tstate->post_cpu;
    BYTE *test_mem = tstate->test_mem;
    BYTE *post_mem = tstate->post_mem;
    // JR NZ
    DO_TEST(tstate, 0x20, -3, 0x00, 2, 3, 
        test_cpu->flags.z = CLEAR;
        post_cpu->flags.z = CLEAR;
        reg_pc(post_cpu) = 0;
    );
    DO_TEST(tstate, 0x20, 0x10, 0x00, 2, 2, 
        test_cpu->flags.z = SET;
        post_cpu->flags.z = SET;
        reg_pc(post_cpu) = 0x03;
    );
    // JR NC
    DO_TEST(tstate, 0x30, -3, 0x00, 2, 3, 
        test_cpu->flags.c = CLEAR;
        post_cpu->flags.c = CLEAR;
        reg_pc(post_cpu) = 0;
    );
    DO_TEST(tstate, 0x30, 0x10, 0x00, 2, 2, 
        test_cpu->flags.c = SET;
        post_cpu->flags.c = SET;
        reg_pc(post_cpu) = 0x03;
    ); 
    // JR
    DO_TEST(tstate, 0x18, -3, 0x00, 2, 3, 
        reg_pc(post_cpu) = 0;
    ); 
    // JR Z
    DO_TEST(tstate, 0x28, -3, 0x00, 2, 3, 
        test_cpu->flags.z = SET;
        post_cpu->flags.z = SET;
        reg_pc(post_cpu) = 0x0;
    );
    DO_TEST(tstate, 0x28, 0x10, 0x00, 2, 2, 
        test_cpu->flags.z = CLEAR;
        post_cpu->flags.z = CLEAR;
        reg_pc(post_cpu) = 0x03;
    );
    // JR C
    DO_TEST(tstate, 0x38, -3, 0x00, 2, 3, 
        test_cpu->flags.c = SET;
        post_cpu->flags.c = SET;
        reg_pc(post_cpu) = 0x0;
    );
    DO_TEST(tstate, 0x38, 0x10, 0x00, 2, 2, 
        test_cpu->flags.c = CLEAR;
        post_cpu->flags.c = CLEAR;
        reg_pc(post_cpu) = 0x03;
    ); 
}

void run_abs_jmp(CPUTestState *tstate) {
    CPUState *test_cpu = tstate->test_cpu;
    CPUState *post_cpu = tstate->post_cpu;
    BYTE *test_mem = tstate->test_mem;
    BYTE *post_mem = tstate->post_mem; 
    // JP NZ, a16
    DO_TEST(tstate, 0xC2, 0x01, 0x10, 3, 4,
        test_cpu->flags.z = CLEAR;
        post_cpu->flags.z = CLEAR;
        reg_pc(post_cpu) = 0x1002;
    );
    DO_TEST(tstate, 0xC2, 0x01, 0x10, 3, 3,
        test_cpu->flags.z = SET;
        post_cpu->flags.z = SET;
        reg_pc(post_cpu) = 0x04;
    );
    // JP NC, a16
    DO_TEST(tstate, 0xD2, 0x01, 0x10, 3, 4,
        test_cpu->flags.c = CLEAR;
        post_cpu->flags.c = CLEAR;
        reg_pc(post_cpu) = 0x1002;
    );
    DO_TEST(tstate, 0xD2, 0x01, 0x10, 3, 3,
        test_cpu->flags.c = SET;
        post_cpu->flags.c = SET;
        reg_pc(post_cpu) = 0x04;
    );
    // JP a16
    DO_TEST(tstate, 0xC3, 0x01, 0x10, 3, 4,
        reg_pc(post_cpu) = 0x1002;
    ); 
    // JP Z, a16
    DO_TEST(tstate, 0xCA, 0x01, 0x10, 3, 4,
        test_cpu->flags.z = SET;
        post_cpu->flags.z = SET;
        reg_pc(post_cpu) = 0x1002;
    );
    DO_TEST(tstate, 0xCA, 0x01, 0x10, 3, 3,
        test_cpu->flags.z = CLEAR;
        post_cpu->flags.z = CLEAR;
        reg_pc(post_cpu) = 0x04;
    );
    // JP HL
    DO_TEST(tstate, 0xE9, 0x00, 0x00, 1, 1,
        reg_hl(test_cpu) = 0xDEAD;
        reg_hl(post_cpu) = 0xDEAD;
        reg_pc(post_cpu) = 0xDEAE;
    );
}

void run_ld_reg_8(CPUTestState *tstate) {
    CPUState *test_cpu = tstate->test_cpu;
    CPUState *post_cpu = tstate->post_cpu;
    BYTE *test_mem = tstate->test_mem;
    BYTE *post_mem = tstate->post_mem; 

    /* Row 1: Dest = B or C */
    DO_TEST(tstate, 0x40, 0x00, 0x00, 1, 1, 
        reg_b(test_cpu) = 0x23;
        reg_b(post_cpu) = 0x23;
    );
    DO_TEST(tstate, 0x41, 0x00, 0x00, 1, 1, 
        reg_b(test_cpu) = 0x23;
        reg_c(test_cpu) = 0x34;
        reg_b(post_cpu) = 0x34;
        reg_c(post_cpu) = 0x34;
    );
    DO_TEST(tstate, 0x42, 0x00, 0x00, 1, 1, 
        reg_b(test_cpu) = 0x23;
        reg_d(test_cpu) = 0x34;
        reg_b(post_cpu) = 0x34;
        reg_d(post_cpu) = 0x34;
    );
    DO_TEST(tstate, 0x43, 0x00, 0x00, 1, 1, 
        reg_b(test_cpu) = 0x23;
        reg_e(test_cpu) = 0x34;
        reg_b(post_cpu) = 0x34;
        reg_e(post_cpu) = 0x34;
    );
    DO_TEST(tstate, 0x44, 0x00, 0x00, 1, 1, 
        reg_b(test_cpu) = 0x23;
        reg_h(test_cpu) = 0x34;
        reg_b(post_cpu) = 0x34;
        reg_h(post_cpu) = 0x34;
    );
    DO_TEST(tstate, 0x45, 0x00, 0x00, 1, 1, 
        reg_b(test_cpu) = 0x23;
        reg_l(test_cpu) = 0x34;
        reg_b(post_cpu) = 0x34;
        reg_l(post_cpu) = 0x34;
    );
    DO_TEST(tstate, 0x46, 0x00, 0x00, 1, 2, 
        reg_b(test_cpu) = 0x23;
        reg_hl(test_cpu) = 0x8010;
        test_mem[0x8010] = 0xDE;
        reg_b(post_cpu) = 0xDE;
        reg_hl(post_cpu) = 0x8010;
        post_mem[0x8010] = 0xDE;
    );
    DO_TEST(tstate, 0x47, 0x00, 0x00, 1, 1, 
        reg_b(test_cpu) = 0x23;
        reg_a(test_cpu) = 0x34;
        reg_b(post_cpu) = 0x34;
        reg_a(post_cpu) = 0x34;
    );
    DO_TEST(tstate, 0x48, 0x00, 0x00, 1, 1, 
        reg_c(test_cpu) = 0x23;
        reg_b(test_cpu) = 0x34;
        reg_c(post_cpu) = 0x34;
        reg_b(post_cpu) = 0x34;
    );
    DO_TEST(tstate, 0x49, 0x00, 0x00, 1, 1, 
        reg_c(test_cpu) = 0x23;
        reg_c(test_cpu) = 0x34;
        reg_c(post_cpu) = 0x34;
        reg_c(post_cpu) = 0x34;
    );
    DO_TEST(tstate, 0x4A, 0x00, 0x00, 1, 1, 
        reg_c(test_cpu) = 0x23;
        reg_d(test_cpu) = 0x34;
        reg_c(post_cpu) = 0x34;
        reg_d(post_cpu) = 0x34;
    );
    DO_TEST(tstate, 0x4B, 0x00, 0x00, 1, 1, 
        reg_c(test_cpu) = 0x23;
        reg_e(test_cpu) = 0x34;
        reg_c(post_cpu) = 0x34;
        reg_e(post_cpu) = 0x34;
    );
    DO_TEST(tstate, 0x4C, 0x00, 0x00, 1, 1, 
        reg_c(test_cpu) = 0x23;
        reg_h(test_cpu) = 0x34;
        reg_c(post_cpu) = 0x34;
        reg_h(post_cpu) = 0x34;
    );
    DO_TEST(tstate, 0x4D, 0x00, 0x00, 1, 1, 
        reg_c(test_cpu) = 0x23;
        reg_l(test_cpu) = 0x34;
        reg_c(post_cpu) = 0x34;
        reg_l(post_cpu) = 0x34;
    );
    DO_TEST(tstate, 0x4E, 0x00, 0x00, 1, 2, 
        reg_c(test_cpu) = 0x23;
        reg_hl(test_cpu) = 0x8010;
        test_mem[0x8010] = 0xDE;
        reg_c(post_cpu) = 0xDE;
        reg_hl(post_cpu) = 0x8010;
        post_mem[0x8010] = 0xDE; 
    );
    DO_TEST(tstate, 0x4F, 0x00, 0x00, 1, 1, 
        reg_c(test_cpu) = 0x23;
        reg_a(test_cpu) = 0x34;
        reg_c(post_cpu) = 0x34;
        reg_a(post_cpu) = 0x34;
    );

    /* Row 2: dest = D or E */

    DO_TEST(tstate, 0x50, 0x00, 0x00, 1, 1, 
        reg_d(test_cpu) = 0x23;
        reg_b(test_cpu) = 0x34;
        reg_d(post_cpu) = 0x34;
        reg_b(post_cpu) = 0x34;
        
    );
    DO_TEST(tstate, 0x51, 0x00, 0x00, 1, 1, 
        reg_d(test_cpu) = 0x23;
        reg_c(test_cpu) = 0x34;
        reg_d(post_cpu) = 0x34;
        reg_c(post_cpu) = 0x34;
    );
    DO_TEST(tstate, 0x52, 0x00, 0x00, 1, 1, 
        reg_d(test_cpu) = 0x23;
        reg_d(test_cpu) = 0x34;
        reg_d(post_cpu) = 0x34;
        reg_d(post_cpu) = 0x34;
    );
    DO_TEST(tstate, 0x53, 0x00, 0x00, 1, 1, 
        reg_d(test_cpu) = 0x23;
        reg_e(test_cpu) = 0x34;
        reg_d(post_cpu) = 0x34;
        reg_e(post_cpu) = 0x34;
    );
    DO_TEST(tstate, 0x54, 0x00, 0x00, 1, 1, 
        reg_d(test_cpu) = 0x23;
        reg_h(test_cpu) = 0x34;
        reg_d(post_cpu) = 0x34;
        reg_h(post_cpu) = 0x34;
    );
    DO_TEST(tstate, 0x55, 0x00, 0x00, 1, 1, 
        reg_d(test_cpu) = 0x23;
        reg_l(test_cpu) = 0x34;
        reg_d(post_cpu) = 0x34;
        reg_l(post_cpu) = 0x34;
    );
    DO_TEST(tstate, 0x56, 0x00, 0x00, 1, 2, 
        reg_d(test_cpu) = 0x23;
        reg_hl(test_cpu) = 0x8010;
        test_mem[0x8010] = 0xDE;
        reg_d(post_cpu) = 0xDE;
        reg_hl(post_cpu) = 0x8010;
        post_mem[0x8010] = 0xDE;
    );
    DO_TEST(tstate, 0x57, 0x00, 0x00, 1, 1, 
        reg_d(test_cpu) = 0x23;
        reg_a(test_cpu) = 0x34;
        reg_d(post_cpu) = 0x34;
        reg_a(post_cpu) = 0x34;
    );
    DO_TEST(tstate, 0x58, 0x00, 0x00, 1, 1, 
        reg_e(test_cpu) = 0x23;
        reg_b(test_cpu) = 0x34;
        reg_e(post_cpu) = 0x34;
        reg_b(post_cpu) = 0x34;
    );
    DO_TEST(tstate, 0x59, 0x00, 0x00, 1, 1, 
        reg_e(test_cpu) = 0x23;
        reg_c(test_cpu) = 0x34;
        reg_e(post_cpu) = 0x34;
        reg_c(post_cpu) = 0x34;
    );
    DO_TEST(tstate, 0x5A, 0x00, 0x00, 1, 1, 
        reg_e(test_cpu) = 0x23;
        reg_d(test_cpu) = 0x34;
        reg_e(post_cpu) = 0x34;
        reg_d(post_cpu) = 0x34;
    );
    DO_TEST(tstate, 0x5B, 0x00, 0x00, 1, 1, 
        reg_e(test_cpu) = 0x23;
        reg_e(test_cpu) = 0x34;
        reg_e(post_cpu) = 0x34;
        reg_e(post_cpu) = 0x34;
    );
    DO_TEST(tstate, 0x5C, 0x00, 0x00, 1, 1, 
        reg_e(test_cpu) = 0x23;
        reg_h(test_cpu) = 0x34;
        reg_e(post_cpu) = 0x34;
        reg_h(post_cpu) = 0x34;
    );
    DO_TEST(tstate, 0x5D, 0x00, 0x00, 1, 1, 
        reg_e(test_cpu) = 0x23;
        reg_l(test_cpu) = 0x34;
        reg_e(post_cpu) = 0x34;
        reg_l(post_cpu) = 0x34;
    );
    DO_TEST(tstate, 0x5E, 0x00, 0x00, 1, 2, 
        reg_e(test_cpu) = 0x23;
        reg_hl(test_cpu) = 0x8010;
        test_mem[0x8010] = 0xDE;
        reg_e(post_cpu) = 0xDE;
        reg_hl(post_cpu) = 0x8010;
        post_mem[0x8010] = 0xDE; 
    );
    DO_TEST(tstate, 0x5F, 0x00, 0x00, 1, 1, 
        reg_e(test_cpu) = 0x23;
        reg_a(test_cpu) = 0x34;
        reg_e(post_cpu) = 0x34;
        reg_a(post_cpu) = 0x34;
    );

    /* Row 3: Dest = H or L */

    DO_TEST(tstate, 0x60, 0x00, 0x00, 1, 1, 
        reg_h(test_cpu) = 0x23;
        reg_b(test_cpu) = 0x34;
        reg_h(post_cpu) = 0x34;
        reg_b(post_cpu) = 0x34;
    );
    DO_TEST(tstate, 0x61, 0x00, 0x00, 1, 1, 
        reg_h(test_cpu) = 0x23;
        reg_c(test_cpu) = 0x34;
        reg_h(post_cpu) = 0x34;
        reg_c(post_cpu) = 0x34;
    );
    DO_TEST(tstate, 0x62, 0x00, 0x00, 1, 1, 
        reg_h(test_cpu) = 0x23;
        reg_d(test_cpu) = 0x34;
        reg_h(post_cpu) = 0x34;
        reg_d(post_cpu) = 0x34;
    );
    DO_TEST(tstate, 0x63, 0x00, 0x00, 1, 1, 
        reg_h(test_cpu) = 0x23;
        reg_e(test_cpu) = 0x34;
        reg_h(post_cpu) = 0x34;
        reg_e(post_cpu) = 0x34;
    );
    DO_TEST(tstate, 0x64, 0x00, 0x00, 1, 1, 
        reg_h(test_cpu) = 0x23;
        reg_h(test_cpu) = 0x34;
        reg_h(post_cpu) = 0x34;
        reg_h(post_cpu) = 0x34;
    );
    DO_TEST(tstate, 0x65, 0x00, 0x00, 1, 1, 
        reg_h(test_cpu) = 0x23;
        reg_l(test_cpu) = 0x34;
        reg_h(post_cpu) = 0x34;
        reg_l(post_cpu) = 0x34;
    );
    DO_TEST(tstate, 0x66, 0x00, 0x00, 1, 2, 
        reg_h(test_cpu) = 0x23;
        reg_hl(test_cpu) = 0x8010;
        test_mem[0x8010] = 0xDE;
        reg_hl(post_cpu) = 0x8010;
        reg_h(post_cpu) = 0xDE;
        post_mem[0x8010] = 0xDE;
    );
    DO_TEST(tstate, 0x67, 0x00, 0x00, 1, 1, 
        reg_h(test_cpu) = 0x23;
        reg_a(test_cpu) = 0x34;
        reg_h(post_cpu) = 0x34;
        reg_a(post_cpu) = 0x34;
    );
    DO_TEST(tstate, 0x68, 0x00, 0x00, 1, 1, 
        reg_l(test_cpu) = 0x23;
        reg_b(test_cpu) = 0x34;
        reg_l(post_cpu) = 0x34;
        reg_b(post_cpu) = 0x34;
    );
    DO_TEST(tstate, 0x69, 0x00, 0x00, 1, 1, 
        reg_l(test_cpu) = 0x23;
        reg_c(test_cpu) = 0x34;
        reg_l(post_cpu) = 0x34;
        reg_c(post_cpu) = 0x34;
    );
    DO_TEST(tstate, 0x6A, 0x00, 0x00, 1, 1, 
        reg_l(test_cpu) = 0x23;
        reg_d(test_cpu) = 0x34;
        reg_l(post_cpu) = 0x34;
        reg_d(post_cpu) = 0x34;
    );
    DO_TEST(tstate, 0x6B, 0x00, 0x00, 1, 1, 
        reg_l(test_cpu) = 0x23;
        reg_e(test_cpu) = 0x34;
        reg_l(post_cpu) = 0x34;
        reg_e(post_cpu) = 0x34;
    );
    DO_TEST(tstate, 0x6C, 0x00, 0x00, 1, 1, 
        reg_l(test_cpu) = 0x23;
        reg_h(test_cpu) = 0x34;
        reg_l(post_cpu) = 0x34;
        reg_h(post_cpu) = 0x34;
    );
    DO_TEST(tstate, 0x6D, 0x00, 0x00, 1, 1, 
        reg_l(test_cpu) = 0x23;
        reg_l(test_cpu) = 0x34;
        reg_l(post_cpu) = 0x34;
        reg_l(post_cpu) = 0x34;
    );
    DO_TEST(tstate, 0x6E, 0x00, 0x00, 1, 2, 
        reg_l(test_cpu) = 0x23;
        reg_hl(test_cpu) = 0x8010;
        test_mem[0x8010] = 0xDE;
        reg_hl(post_cpu) = 0x8010;
        reg_l(post_cpu) = 0xDE;
        post_mem[0x8010] = 0xDE; 
    );
    DO_TEST(tstate, 0x6F, 0x00, 0x00, 1, 1, 
        reg_l(test_cpu) = 0x23;
        reg_a(test_cpu) = 0x34;
        reg_l(post_cpu) = 0x34;
        reg_a(post_cpu) = 0x34;
    );

    /* Row 4: Dest is (HL) or A */
    DO_TEST(tstate, 0x70, 0x00, 0x00, 1, 2, 
        reg_b(test_cpu) = 0xDE;
        reg_hl(test_cpu) = 0x8010;
        reg_b(post_cpu) = 0xDE;
        reg_hl(post_cpu) = 0x8010;
        post_mem[0x8010] = 0xDE;
    );
    DO_TEST(tstate, 0x71, 0x00, 0x00, 1, 2, 
        reg_c(test_cpu) = 0xDE;
        reg_hl(test_cpu) = 0x8010;
        reg_c(post_cpu) = 0xDE;
        reg_hl(post_cpu) = 0x8010;
        post_mem[0x8010] = 0xDE;
    );
    DO_TEST(tstate, 0x72, 0x00, 0x00, 1, 2, 
        reg_d(test_cpu) = 0xDE;
        reg_hl(test_cpu) = 0x8010;
        reg_d(post_cpu) = 0xDE;
        reg_hl(post_cpu) = 0x8010;
        post_mem[0x8010] = 0xDE;
    );
    DO_TEST(tstate, 0x73, 0x00, 0x00, 1, 2, 
        reg_e(test_cpu) = 0xDE;
        reg_hl(test_cpu) = 0x8010;
        reg_e(post_cpu) = 0xDE;
        reg_hl(post_cpu) = 0x8010;
        post_mem[0x8010] = 0xDE;
    );
    DO_TEST(tstate, 0x74, 0x00, 0x00, 1, 2, 
        reg_hl(test_cpu) = 0x8010;
        reg_hl(post_cpu) = 0x8010;
        post_mem[0x8010] = 0x80;
    );
    DO_TEST(tstate, 0x75, 0x00, 0x00, 1, 2, 
        reg_hl(test_cpu) = 0x8010;
        reg_hl(post_cpu) = 0x8010;
        post_mem[0x8010] = 0x10;
    );

    /* 0x76 is HALT */

    DO_TEST(tstate, 0x77, 0x00, 0x00, 1, 2, 
        reg_a(test_cpu) = 0xDE;
        reg_hl(test_cpu) = 0x8010;
        reg_a(post_cpu) = 0xDE;
        reg_hl(post_cpu) = 0x8010;
        post_mem[0x8010] = 0xDE;
    );
    DO_TEST(tstate, 0x78, 0x00, 0x00, 1, 1, 
        reg_a(test_cpu) = 0x23;
        reg_b(test_cpu) = 0x34;
        reg_a(post_cpu) = 0x34;
        reg_b(post_cpu) = 0x34;
    );
    DO_TEST(tstate, 0x79, 0x00, 0x00, 1, 1, 
        reg_a(test_cpu) = 0x23;
        reg_c(test_cpu) = 0x34;
        reg_a(post_cpu) = 0x34;
        reg_c(post_cpu) = 0x34;
    );
    DO_TEST(tstate, 0x7A, 0x00, 0x00, 1, 1, 
        reg_a(test_cpu) = 0x23;
        reg_d(test_cpu) = 0x34;
        reg_a(post_cpu) = 0x34;
        reg_d(post_cpu) = 0x34;
    );
    DO_TEST(tstate, 0x7B, 0x00, 0x00, 1, 1, 
        reg_a(test_cpu) = 0x23;
        reg_e(test_cpu) = 0x34;
        reg_a(post_cpu) = 0x34;
        reg_e(post_cpu) = 0x34;
    );
    DO_TEST(tstate, 0x7C, 0x00, 0x00, 1, 1, 
        reg_a(test_cpu) = 0x23;
        reg_h(test_cpu) = 0x34;
        reg_a(post_cpu) = 0x34;
        reg_h(post_cpu) = 0x34;
    );
    DO_TEST(tstate, 0x7D, 0x00, 0x00, 1, 1, 
        reg_a(test_cpu) = 0x23;
        reg_l(test_cpu) = 0x34;
        reg_a(post_cpu) = 0x34;
        reg_l(post_cpu) = 0x34;
    );
    DO_TEST(tstate, 0x7E, 0x00, 0x00, 1, 2, 
        reg_a(test_cpu) = 0x23;
        reg_hl(test_cpu) = 0x8010;
        test_mem[0x8010] = 0xDE;
        reg_hl(post_cpu) = 0x8010;
        reg_a(post_cpu) = 0xDE;
        post_mem[0x8010] = 0xDE;  
    );
    DO_TEST(tstate, 0x7F, 0x00, 0x00, 1, 1, 
        reg_a(test_cpu) = 0x23;
        reg_a(test_cpu) = 0x34;
        reg_a(post_cpu) = 0x34;
        reg_a(post_cpu) = 0x34;
    );
}

void run_math_reg_8(CPUTestState *tstate) {
    CPUState *test_cpu = tstate->test_cpu;
    CPUState *post_cpu = tstate->post_cpu;
    BYTE *test_mem = tstate->test_mem;
    BYTE *post_mem = tstate->post_mem;

    /* Row 1: ADD, ADC */
    DO_TEST(tstate, 0x80, 0x00, 0x00, 1, 1,
        reg_b(test_cpu) = 0x32;
        reg_a(test_cpu) = 0x3A;
        reg_b(post_cpu) = 0x32;
        reg_a(post_cpu) = 0x6C;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, SET, CLEAR);
    );
    DO_TEST(tstate, 0x80, 0x00, 0x00, 1, 1,
        reg_b(test_cpu) = 0xEE;
        reg_a(test_cpu) = 0x12;
        reg_b(post_cpu) = 0xEE;
        reg_a(post_cpu) = 0x00;
        SET_ZNHC(post_cpu, SET, CLEAR, CLEAR, SET);
    )
    DO_TEST(tstate, 0x81, 0x00, 0x00, 1, 1,
        reg_c(test_cpu) = 0x32;
        reg_a(test_cpu) = 0x3A;
        reg_c(post_cpu) = 0x32;
        reg_a(post_cpu) = 0x6C;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, SET, CLEAR);
    );
    DO_TEST(tstate, 0x82, 0x00, 0x00, 1, 1,
        reg_d(test_cpu) = 0x32;
        reg_a(test_cpu) = 0x3A;
        reg_d(post_cpu) = 0x32;
        reg_a(post_cpu) = 0x6C;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, SET, CLEAR);
    );
    DO_TEST(tstate, 0x83, 0x00, 0x00, 1, 1,
        reg_e(test_cpu) = 0x32;
        reg_a(test_cpu) = 0x3A;
        reg_e(post_cpu) = 0x32;
        reg_a(post_cpu) = 0x6C;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, SET, CLEAR);
    );
    DO_TEST(tstate, 0x84, 0x00, 0x00, 1, 1,
        reg_h(test_cpu) = 0x32;
        reg_a(test_cpu) = 0x3A;
        reg_h(post_cpu) = 0x32;
        reg_a(post_cpu) = 0x6C;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, SET, CLEAR);
    );
    DO_TEST(tstate, 0x85, 0x00, 0x00, 1, 1,
        reg_l(test_cpu) = 0x32;
        reg_a(test_cpu) = 0x3A;
        reg_l(post_cpu) = 0x32;
        reg_a(post_cpu) = 0x6C;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, SET, CLEAR);
    );
    DO_TEST(tstate, 0x86, 0x00, 0x00, 1, 2,
        reg_hl(test_cpu) = 0x8010;
        test_mem[0x8010] = 0x11;
        reg_a(test_cpu) = 0x3A;
        reg_hl(post_cpu) = 0x8010;
        post_mem[0x8010] = 0x11;
        reg_a(post_cpu) = 0x4B;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, SET, CLEAR);
    );
    DO_TEST(tstate, 0x87, 0x00, 0x00, 1, 1,
        reg_a(test_cpu) = 0x3A;
        reg_a(post_cpu) = 0x74;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, SET, CLEAR);
    );
    DO_TEST(tstate, 0x88, 0x00, 0x00, 1, 1,
        reg_b(test_cpu) = 0x32;
        reg_a(test_cpu) = 0x3A;
        reg_b(post_cpu) = 0x32;
        reg_a(post_cpu) = 0x6C;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, SET, CLEAR);
    );
    DO_TEST(tstate, 0x88, 0x00, 0x00, 1, 1,
        test_cpu->flags.c = SET;
        reg_b(test_cpu) = 0x32;
        reg_a(test_cpu) = 0x3A;
        reg_b(post_cpu) = 0x32;
        reg_a(post_cpu) = 0x6D;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, SET, CLEAR);
    );
    DO_TEST(tstate, 0x89, 0x00, 0x00, 1, 1,
        test_cpu->flags.c = SET;
        reg_c(test_cpu) = 0x32;
        reg_a(test_cpu) = 0x3A;
        reg_c(post_cpu) = 0x32;
        reg_a(post_cpu) = 0x6D;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, SET, CLEAR);
    );
    DO_TEST(tstate, 0x8A, 0x00, 0x00, 1, 1,
        test_cpu->flags.c = SET;
        reg_d(test_cpu) = 0x32;
        reg_a(test_cpu) = 0x3A;
        reg_d(post_cpu) = 0x32;
        reg_a(post_cpu) = 0x6D;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, SET, CLEAR);
    );
    DO_TEST(tstate, 0x8B, 0x00, 0x00, 1, 1,
        test_cpu->flags.c = SET;
        reg_e(test_cpu) = 0x32;
        reg_a(test_cpu) = 0x3A;
        reg_e(post_cpu) = 0x32;
        reg_a(post_cpu) = 0x6D;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, SET, CLEAR);
    );
    DO_TEST(tstate, 0x8C, 0x00, 0x00, 1, 1,
        test_cpu->flags.c = SET;
        reg_h(test_cpu) = 0x32;
        reg_a(test_cpu) = 0x3A;
        reg_h(post_cpu) = 0x32;
        reg_a(post_cpu) = 0x6D;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, SET, CLEAR);
    );
    DO_TEST(tstate, 0x8D, 0x00, 0x00, 1, 1,
        test_cpu->flags.c = SET;
        reg_l(test_cpu) = 0x32;
        reg_a(test_cpu) = 0x3A;
        reg_l(post_cpu) = 0x32;
        reg_a(post_cpu) = 0x6D;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, SET, CLEAR);
    );
    DO_TEST(tstate, 0x8E, 0x00, 0x00, 1, 2,
        test_cpu->flags.c = SET;
        reg_hl(test_cpu) = 0x8010;
        test_mem[0x8010] = 0x11;
        reg_a(test_cpu) = 0x3A;
        reg_hl(post_cpu) = 0x8010;
        post_mem[0x8010] = 0x11;
        reg_a(post_cpu) = 0x4C;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, SET, CLEAR);
    );
    DO_TEST(tstate, 0x8F, 0x00, 0x00, 1, 1,
        test_cpu->flags.c = SET;
        reg_a(test_cpu) = 0x3A;
        reg_a(post_cpu) = 0x75;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, SET, CLEAR);
    );

    /* Row 2: SUB, SBC */
    DO_TEST(tstate, 0x90, 0x00, 0x00, 1, 1,
        reg_b(test_cpu) = 0x32;
        reg_a(test_cpu) = 0x3A;
        reg_b(post_cpu) = 0x32;
        reg_a(post_cpu) = 0x08;
        SET_ZNHC(post_cpu, CLEAR, SET, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0x90, 0x00, 0x00, 1, 1,
        reg_b(test_cpu) = 0xEE;
        reg_a(test_cpu) = 0x12;
        reg_b(post_cpu) = 0xEE;
        reg_a(post_cpu) = 0x24;
        SET_ZNHC(post_cpu, CLEAR, SET, CLEAR, SET);
    )
    DO_TEST(tstate, 0x91, 0x00, 0x00, 1, 1,
        reg_c(test_cpu) = 0x32;
        reg_a(test_cpu) = 0x3A;
        reg_c(post_cpu) = 0x32;
        reg_a(post_cpu) = 0x08;
        SET_ZNHC(post_cpu, CLEAR, SET, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0x92, 0x00, 0x00, 1, 1,
        reg_d(test_cpu) = 0x32;
        reg_a(test_cpu) = 0x3A;
        reg_d(post_cpu) = 0x32;
        reg_a(post_cpu) = 0x08;
        SET_ZNHC(post_cpu, CLEAR, SET, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0x93, 0x00, 0x00, 1, 1,
        reg_e(test_cpu) = 0x32;
        reg_a(test_cpu) = 0x3A;
        reg_e(post_cpu) = 0x32;
        reg_a(post_cpu) = 0x08;
        SET_ZNHC(post_cpu, CLEAR, SET, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0x94, 0x00, 0x00, 1, 1,
        reg_h(test_cpu) = 0x32;
        reg_a(test_cpu) = 0x3A;
        reg_h(post_cpu) = 0x32;
        reg_a(post_cpu) = 0x08;
        SET_ZNHC(post_cpu, CLEAR, SET, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0x95, 0x00, 0x00, 1, 1,
        reg_l(test_cpu) = 0x32;
        reg_a(test_cpu) = 0x3A;
        reg_l(post_cpu) = 0x32;
        reg_a(post_cpu) = 0x08;
        SET_ZNHC(post_cpu, CLEAR, SET, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0x96, 0x00, 0x00, 1, 2,
        reg_hl(test_cpu) = 0x8010;
        test_mem[0x8010] = 0x11;
        reg_a(test_cpu) = 0x3A;
        reg_hl(post_cpu) = 0x8010;
        post_mem[0x8010] = 0x11;
        reg_a(post_cpu) = 0x29;
        SET_ZNHC(post_cpu, CLEAR, SET, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0x97, 0x00, 0x00, 1, 1,
        reg_a(test_cpu) = 0x3A;
        reg_a(post_cpu) = 0x0;
        SET_ZNHC(post_cpu, SET, SET, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0x98, 0x00, 0x00, 1, 1,
        reg_b(test_cpu) = 0x32;
        reg_a(test_cpu) = 0x3A;
        reg_b(post_cpu) = 0x32;
        reg_a(post_cpu) = 0x08;
        SET_ZNHC(post_cpu, CLEAR, SET, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0x98, 0x00, 0x00, 1, 1,
        test_cpu->flags.c = SET;
        reg_b(test_cpu) = 0x32;
        reg_a(test_cpu) = 0x3A;
        reg_b(post_cpu) = 0x32;
        reg_a(post_cpu) = 0x07;
        SET_ZNHC(post_cpu, CLEAR, SET, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0x99, 0x00, 0x00, 1, 1,
        test_cpu->flags.c = SET;
        reg_c(test_cpu) = 0x32;
        reg_a(test_cpu) = 0x3A;
        reg_c(post_cpu) = 0x32;
        reg_a(post_cpu) = 0x07;
        SET_ZNHC(post_cpu, CLEAR, SET, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0x9A, 0x00, 0x00, 1, 1,
        test_cpu->flags.c = SET;
        reg_d(test_cpu) = 0x32;
        reg_a(test_cpu) = 0x3A;
        reg_d(post_cpu) = 0x32;
        reg_a(post_cpu) = 0x07;
        SET_ZNHC(post_cpu, CLEAR, SET, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0x9B, 0x00, 0x00, 1, 1,
        test_cpu->flags.c = SET;
        reg_e(test_cpu) = 0x32;
        reg_a(test_cpu) = 0x3A;
        reg_e(post_cpu) = 0x32;
        reg_a(post_cpu) = 0x07;
        SET_ZNHC(post_cpu, CLEAR, SET, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0x9C, 0x00, 0x00, 1, 1,
        test_cpu->flags.c = SET;
        reg_h(test_cpu) = 0x32;
        reg_a(test_cpu) = 0x3A;
        reg_h(post_cpu) = 0x32;
        reg_a(post_cpu) = 0x07;
        SET_ZNHC(post_cpu, CLEAR, SET, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0x9D, 0x00, 0x00, 1, 1,
        test_cpu->flags.c = SET;
        reg_l(test_cpu) = 0x32;
        reg_a(test_cpu) = 0x3A;
        reg_l(post_cpu) = 0x32;
        reg_a(post_cpu) = 0x07;
        SET_ZNHC(post_cpu, CLEAR, SET, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0x9E, 0x00, 0x00, 1, 2,
        test_cpu->flags.c = SET;
        reg_hl(test_cpu) = 0x8010;
        test_mem[0x8010] = 0x11;
        reg_a(test_cpu) = 0x3A;
        reg_hl(post_cpu) = 0x8010;
        post_mem[0x8010] = 0x11;
        reg_a(post_cpu) = 0x28;
        SET_ZNHC(post_cpu, CLEAR, SET, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0x9F, 0x00, 0x00, 1, 1,
        test_cpu->flags.c = SET;
        reg_a(test_cpu) = 0x3A;
        reg_a(post_cpu) = 0xFF;
        SET_ZNHC(post_cpu, CLEAR, SET, SET, SET);
    ); 
    DO_TEST(tstate, 0x9F, 0x00, 0x00, 1, 1,
        reg_a(test_cpu) = 0x3A;
        reg_a(post_cpu) = 0x00;
        SET_ZNHC(post_cpu, SET, SET, CLEAR, CLEAR);
    ); 

    /* Row 3: AND, XOR */

    DO_TEST(tstate, 0xA0, 0x00, 0x00, 1, 1,
        reg_a(test_cpu) = 0xFB;
        reg_b(test_cpu) = 0x0F;
        reg_b(post_cpu) = 0x0F;
        reg_a(post_cpu) = 0x0B;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, SET, CLEAR);
    );
    DO_TEST(tstate, 0xA1, 0x00, 0x00, 1, 1,
        reg_a(test_cpu) = 0xFB;
        reg_c(test_cpu) = 0x0F;
        reg_c(post_cpu) = 0x0F;
        reg_a(post_cpu) = 0x0B;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, SET, CLEAR);
    );
    DO_TEST(tstate, 0xA2, 0x00, 0x00, 1, 1,
        reg_a(test_cpu) = 0xFB;
        reg_d(test_cpu) = 0x0F;
        reg_d(post_cpu) = 0x0F;
        reg_a(post_cpu) = 0x0B;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, SET, CLEAR);
    );
    DO_TEST(tstate, 0xA3, 0x00, 0x00, 1, 1,
        reg_a(test_cpu) = 0xFB;
        reg_e(test_cpu) = 0x0F;
        reg_e(post_cpu) = 0x0F;
        reg_a(post_cpu) = 0x0B;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, SET, CLEAR);
    ); 
    DO_TEST(tstate, 0xA4, 0x00, 0x00, 1, 1,
        reg_a(test_cpu) = 0xFB;
        reg_h(test_cpu) = 0x0F;
        reg_h(post_cpu) = 0x0F;
        reg_a(post_cpu) = 0x0B;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, SET, CLEAR);
    );
    DO_TEST(tstate, 0xA5, 0x00, 0x00, 1, 1,
        reg_a(test_cpu) = 0xFB;
        reg_l(test_cpu) = 0x0F;
        reg_l(post_cpu) = 0x0F;
        reg_a(post_cpu) = 0x0B;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, SET, CLEAR);
    );
    DO_TEST(tstate, 0xA6, 0x00, 0x00, 1, 2,
        reg_a(test_cpu) = 0xFB;
        reg_hl(test_cpu) = 0x8010;
        test_mem[0x8010] = 0xF0;
        reg_hl(post_cpu) = 0x8010;
        post_mem[0x8010] = 0xF0;
        reg_a(post_cpu) = 0xF0;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, SET, CLEAR);
    );
    DO_TEST(tstate, 0xA7, 0x00, 0x00, 1, 1,
        reg_a(test_cpu) = 0xFB;
        reg_a(post_cpu) = 0xFB;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, SET, CLEAR);
    );
    DO_TEST(tstate, 0xA8, 0x00, 0x00, 1, 1,
        reg_a(test_cpu) = 0xAA;
        reg_b(test_cpu) = 0x55;
        reg_b(post_cpu) = 0x55;
        reg_a(post_cpu) = 0xFF;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0xA9, 0x00, 0x00, 1, 1,
        reg_a(test_cpu) = 0xAA;
        reg_c(test_cpu) = 0x55;
        reg_c(post_cpu) = 0x55;
        reg_a(post_cpu) = 0xFF;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0xAA, 0x00, 0x00, 1, 1,
        reg_a(test_cpu) = 0xAA;
        reg_d(test_cpu) = 0x55;
        reg_d(post_cpu) = 0x55;
        reg_a(post_cpu) = 0xFF;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0xAB, 0x00, 0x00, 1, 1,
        reg_a(test_cpu) = 0xAA;
        reg_e(test_cpu) = 0x55;
        reg_e(post_cpu) = 0x55;
        reg_a(post_cpu) = 0xFF;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0xAC, 0x00, 0x00, 1, 1,
        reg_a(test_cpu) = 0xAA;
        reg_h(test_cpu) = 0x55;
        reg_h(post_cpu) = 0x55;
        reg_a(post_cpu) = 0xFF;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0xAD, 0x00, 0x00, 1, 1,
        reg_a(test_cpu) = 0xAA;
        reg_l(test_cpu) = 0x55;
        reg_l(post_cpu) = 0x55;
        reg_a(post_cpu) = 0xFF;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0xAE, 0x00, 0x00, 1, 2,
        reg_a(test_cpu) = 0xAA;
        reg_hl(test_cpu) = 0x8010;
        test_mem[0x8010] = 0x55;
        reg_hl(post_cpu) = 0x8010;
        post_mem[0x8010] = 0x55;
        reg_a(post_cpu) = 0xFF;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0xAF, 0x00, 0x00, 1, 1,
        reg_a(test_cpu) = 0xAA;
        reg_a(post_cpu) = 0x00;
        SET_ZNHC(post_cpu, SET, CLEAR, CLEAR, CLEAR);
    );

    /* Row 3: OR and CP */
    DO_TEST(tstate, 0xB0, 0x00, 0x00, 1, 1,
        reg_a(test_cpu) = 0x03;
        reg_b(test_cpu) = 0x32;
        reg_b(post_cpu) = 0x32;
        reg_a(post_cpu) = 0x33;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0xB1, 0x00, 0x00, 1, 1,
        reg_a(test_cpu) = 0x03;
        reg_c(test_cpu) = 0x32;
        reg_c(post_cpu) = 0x32;
        reg_a(post_cpu) = 0x33;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0xB2, 0x00, 0x00, 1, 1,
        reg_a(test_cpu) = 0x03;
        reg_d(test_cpu) = 0x32;
        reg_d(post_cpu) = 0x32;
        reg_a(post_cpu) = 0x33;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0xB3, 0x00, 0x00, 1, 1,
        reg_a(test_cpu) = 0x03;
        reg_e(test_cpu) = 0x32;
        reg_e(post_cpu) = 0x32;
        reg_a(post_cpu) = 0x33;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0xB4, 0x00, 0x00, 1, 1,
        reg_a(test_cpu) = 0x03;
        reg_h(test_cpu) = 0x32;
        reg_h(post_cpu) = 0x32;
        reg_a(post_cpu) = 0x33;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0xB5, 0x00, 0x00, 1, 1,
        reg_a(test_cpu) = 0x03;
        reg_l(test_cpu) = 0x32;
        reg_l(post_cpu) = 0x32;
        reg_a(post_cpu) = 0x33;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0xB6, 0x00, 0x00, 1, 2,
        reg_a(test_cpu) = 0x03;
        reg_hl(test_cpu) = 0x8010;
        test_mem[0x8010] = 0x32;
        reg_hl(post_cpu) = 0x8010;
        post_mem[0x8010] = 0x32;
        reg_a(post_cpu) = 0x33;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0xB7, 0x00, 0x00, 1, 1,
        reg_a(test_cpu) = 0x03;
        reg_a(post_cpu) = 0x03;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0xB8, 0x00, 0x00, 1, 1,
        reg_a(test_cpu) = 0x03;
        reg_b(test_cpu) = 0x32;
        reg_b(post_cpu) = 0x32;
        reg_a(post_cpu) = 0x03;
        SET_ZNHC(post_cpu, CLEAR, SET, CLEAR, SET);
    );
    DO_TEST(tstate, 0xB8, 0x00, 0x00, 1, 1,
        reg_a(test_cpu) = 0x03;
        reg_b(test_cpu) = 0x03;
        reg_b(post_cpu) = 0x03;
        reg_a(post_cpu) = 0x03;
        SET_ZNHC(post_cpu, SET, SET, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0xB9, 0x00, 0x00, 1, 1,
        reg_a(test_cpu) = 0x03;
        reg_c(test_cpu) = 0x32;
        reg_c(post_cpu) = 0x32;
        reg_a(post_cpu) = 0x03;
        SET_ZNHC(post_cpu, CLEAR, SET, CLEAR, SET);
    );
    DO_TEST(tstate, 0xBA, 0x00, 0x00, 1, 1,
        reg_a(test_cpu) = 0x03;
        reg_d(test_cpu) = 0x32;
        reg_d(post_cpu) = 0x32;
        reg_a(post_cpu) = 0x03;
        SET_ZNHC(post_cpu, CLEAR, SET, CLEAR, SET);
    );
    DO_TEST(tstate, 0xBB, 0x00, 0x00, 1, 1,
        reg_a(test_cpu) = 0x03;
        reg_e(test_cpu) = 0x32;
        reg_e(post_cpu) = 0x32;
        reg_a(post_cpu) = 0x03;
        SET_ZNHC(post_cpu, CLEAR, SET, CLEAR, SET);
    );
    DO_TEST(tstate, 0xBC, 0x00, 0x00, 1, 1,
        reg_a(test_cpu) = 0x03;
        reg_h(test_cpu) = 0x32;
        reg_h(post_cpu) = 0x32;
        reg_a(post_cpu) = 0x03;
        SET_ZNHC(post_cpu, CLEAR, SET, CLEAR, SET);
    );
    DO_TEST(tstate, 0xBD, 0x00, 0x00, 1, 1,
        reg_a(test_cpu) = 0x03;
        reg_l(test_cpu) = 0x32;
        reg_l(post_cpu) = 0x32;
        reg_a(post_cpu) = 0x03;
        SET_ZNHC(post_cpu, CLEAR, SET, CLEAR, SET);
    );
    DO_TEST(tstate, 0xBE, 0x00, 0x00, 1, 2,
        reg_a(test_cpu) = 0x03;
        reg_hl(test_cpu) = 0x8010;
        test_mem[0x8010] = 0x32;
        reg_hl(post_cpu) = 0x8010;
        post_mem[0x8010] = 0x32;
        reg_a(post_cpu) = 0x03;
        SET_ZNHC(post_cpu, CLEAR, SET, CLEAR, SET);
    );
    DO_TEST(tstate, 0xBF, 0x00, 0x00, 1, 1,
        reg_a(test_cpu) = 0x03;
        reg_a(post_cpu) = 0x03;
        SET_ZNHC(post_cpu, SET, SET, CLEAR, CLEAR);
    );
}

void run_math_imm_8(CPUTestState *tstate) {
    CPUState *test_cpu = tstate->test_cpu;
    CPUState *post_cpu = tstate->post_cpu;
    BYTE *test_mem = tstate->test_mem;
    BYTE *post_mem = tstate->post_mem; 

    /* ADD d8 */
    DO_TEST(tstate, 0xC6, 0x10, 0x00, 2, 2,
        reg_a(test_cpu) = 0x10;
        reg_a(post_cpu) = 0x20;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    );
    /* SUB d8 */
    DO_TEST(tstate, 0xD6, 0x10, 0x00, 2, 2,
        reg_a(test_cpu) = 0x10;
        reg_a(post_cpu) = 0x00;
        SET_ZNHC(post_cpu, SET, SET, CLEAR, CLEAR);
    );
    /* AND d8 */
    DO_TEST(tstate, 0xE6, 0xF, 0x00, 2, 2,
        reg_a(test_cpu) = 0b10011100;
        reg_a(post_cpu) = 0b00001100;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, SET, CLEAR);
    );
    /* OR d8 */
    DO_TEST(tstate, 0xF6, 0xF0, 0x00, 2, 2,
        reg_a(test_cpu) = 0x0E;
        reg_a(post_cpu) = 0xFE;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    );
    /* ADC d8 */
    DO_TEST(tstate, 0xCE, 0x10, 0x00, 2, 2,
        reg_a(test_cpu) = 0x10;
        reg_a(post_cpu) = 0x20;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0xCE, 0x10, 0x00, 2, 2,
        reg_a(test_cpu) = 0x10;
        test_cpu->flags.c = SET;
        reg_a(post_cpu) = 0x21;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    );
    /* SBC d8 */
    DO_TEST(tstate, 0xDE, 0x10, 0x00, 2, 2,
        reg_a(test_cpu) = 0x20;
        reg_a(post_cpu) = 0x10;
        SET_ZNHC(post_cpu, CLEAR, SET, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0xDE, 0x10, 0x00, 2, 2,
        reg_a(test_cpu) = 0x20;
        test_cpu->flags.c = SET;
        reg_a(post_cpu) = 0x0F;
        SET_ZNHC(post_cpu, CLEAR, SET, SET, CLEAR);
    );
    /* XOR d8 */
    DO_TEST(tstate, 0xEE, 0xF, 0x00, 2, 2,
        reg_a(test_cpu) = 0b10011100;
        reg_a(post_cpu) = 0b10010011;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    );
    /* CP d8 */
    DO_TEST(tstate, 0xFE, 0xF0, 0x00, 2, 2,
        reg_a(test_cpu) = 0x0E;
        reg_a(post_cpu) = 0x0E;
        SET_ZNHC(post_cpu, CLEAR, SET, SET, SET);
    );
}

void run_push_reg_16(CPUTestState *tstate) {
    CPUState *test_cpu = tstate->test_cpu;
    CPUState *post_cpu = tstate->post_cpu;
    BYTE *test_mem = tstate->test_mem;
    BYTE *post_mem = tstate->post_mem;

    DO_TEST(tstate, 0xC5, 0x00, 0x00, 1, 4, 
        reg_bc(test_cpu) = 0xDEAD;
        reg_bc(post_cpu) = 0xDEAD;
        reg_sp(post_cpu) = 0xFFFC;
        post_mem[0xFFFC] = 0xAD;
        post_mem[0xFFFD] = 0xDE;
    );
    DO_TEST(tstate, 0xD5, 0x00, 0x00, 1, 4, 
        reg_de(test_cpu) = 0xDEAD;
        reg_de(post_cpu) = 0xDEAD;
        reg_sp(post_cpu) = 0xFFFC;
        post_mem[0xFFFC] = 0xAD;
        post_mem[0xFFFD] = 0xDE;
    );
    DO_TEST(tstate, 0xE5, 0x00, 0x00, 1, 4, 
        reg_hl(test_cpu) = 0xDEAD;
        reg_hl(post_cpu) = 0xDEAD;
        reg_sp(post_cpu) = 0xFFFC;
        post_mem[0xFFFC] = 0xAD;
        post_mem[0xFFFD] = 0xDE;
    );
    DO_TEST(tstate, 0xF5, 0x00, 0x00, 1, 4, 
        reg_a(test_cpu) = 0xDE;
        test_cpu->flags.z = SET;
        test_cpu->flags.n = CLEAR;
        test_cpu->flags.h = SET;
        test_cpu->flags.c = CLEAR;
        reg_a(post_cpu) = 0xDE;
        post_cpu->flags.z = SET;
        post_cpu->flags.n = CLEAR;
        post_cpu->flags.h = SET;
        post_cpu->flags.c = CLEAR;
        reg_sp(post_cpu) = 0xFFFC;
        // Flags
        post_mem[0xFFFC] = 0b10100000;
        // reg A
        post_mem[0xFFFD] = 0xDE;
    );
}

void run_pop_reg_16(CPUTestState *tstate) {
    CPUState *test_cpu = tstate->test_cpu;
    CPUState *post_cpu = tstate->post_cpu;
    BYTE *test_mem = tstate->test_mem;
    BYTE *post_mem = tstate->post_mem;

    DO_TEST(tstate, 0xC1, 0x00, 0x00, 1, 3,
        reg_bc(test_cpu) = 0xBEEF;
        reg_sp(test_cpu) = 0xFFFC;
        test_mem[0xFFFC] = 0xAD;
        test_mem[0xFFFD] = 0xDE;

        reg_bc(post_cpu) = 0xDEAD;
        reg_sp(post_cpu) = 0xFFFE;
        post_mem[0xFFFC] = 0xAD;
        post_mem[0xFFFD] = 0xDE;
    );
    DO_TEST(tstate, 0xD1, 0x00, 0x00, 1, 3,
        reg_de(test_cpu) = 0xBEEF;
        reg_sp(test_cpu) = 0xFFFC;
        test_mem[0xFFFC] = 0xAD;
        test_mem[0xFFFD] = 0xDE;

        reg_de(post_cpu) = 0xDEAD;
        reg_sp(post_cpu) = 0xFFFE;
        post_mem[0xFFFC] = 0xAD;
        post_mem[0xFFFD] = 0xDE;
    );
    DO_TEST(tstate, 0xE1, 0x00, 0x00, 1, 3,
        reg_hl(test_cpu) = 0xBEEF;
        reg_sp(test_cpu) = 0xFFFC;
        test_mem[0xFFFC] = 0xAD;
        test_mem[0xFFFD] = 0xDE;

        reg_hl(post_cpu) = 0xDEAD;
        reg_sp(post_cpu) = 0xFFFE;
        post_mem[0xFFFC] = 0xAD;
        post_mem[0xFFFD] = 0xDE;
    );

    DO_TEST(tstate, 0xF1, 0x00, 0x00, 1, 3,
        reg_a(test_cpu) = 0xBE;
        reg_sp(test_cpu) = 0xFFFC;
        test_mem[0xFFFC] = 0b10100000;
        test_mem[0xFFFD] = 0xDE;

        reg_a(post_cpu) = 0xDE;
        reg_sp(post_cpu) = 0xFFFE;
        post_mem[0xFFFC] = 0b10100000;
        post_mem[0xFFFD] = 0xDE;
        SET_ZNHC(post_cpu, SET, CLEAR, SET, CLEAR);
    );
}

void run_ret(CPUTestState *tstate) {
    CPUState *test_cpu = tstate->test_cpu;
    CPUState *post_cpu = tstate->post_cpu;
    BYTE *test_mem = tstate->test_mem;
    BYTE *post_mem = tstate->post_mem;

    // RET NZ
    DO_TEST(tstate, 0xC0, 0x00, 0x00, 1, 5,
        test_cpu->flags.z = CLEAR;
        post_cpu->flags.z = CLEAR;
        reg_sp(test_cpu) = 0xFFFC;
        test_mem[0xFFFC] = 0xAD;
        test_mem[0xFFFD] = 0xDE;
        reg_sp(post_cpu) = 0xFFFE;
        post_mem[0xFFFC] = 0xAD;
        post_mem[0xFFFD]= 0xDE;
        reg_pc(post_cpu) = 0xDEAE;
    );
    DO_TEST(tstate, 0xC0, 0x00, 0x00, 1, 2,
        test_cpu->flags.z = SET;
        post_cpu->flags.z = SET;
        reg_sp(test_cpu) = 0xFFFC;
        test_mem[0xFFFC] = 0xAD;
        test_mem[0xFFFD] = 0xDE;
        reg_sp(post_cpu) = 0xFFFC;
        post_mem[0xFFFC] = 0xAD;
        post_mem[0xFFFD]= 0xDE;
        reg_pc(post_cpu) = 0x02;
    );
    // RET NC
    DO_TEST(tstate, 0xD0, 0x00, 0x00, 1, 5,
        test_cpu->flags.c = CLEAR;
        post_cpu->flags.c = CLEAR;
        reg_sp(test_cpu) = 0xFFFC;
        test_mem[0xFFFC] = 0xAD;
        test_mem[0xFFFD] = 0xDE;
        reg_sp(post_cpu) = 0xFFFE;
        post_mem[0xFFFC] = 0xAD;
        post_mem[0xFFFD]= 0xDE;
        reg_pc(post_cpu) = 0xDEAE;
    );
    DO_TEST(tstate, 0xD0, 0x00, 0x00, 1, 2,
        test_cpu->flags.c = SET;
        post_cpu->flags.c = SET;
        reg_sp(test_cpu) = 0xFFFC;
        test_mem[0xFFFC] = 0xAD;
        test_mem[0xFFFD] = 0xDE;
        reg_sp(post_cpu) = 0xFFFC;
        post_mem[0xFFFC] = 0xAD;
        post_mem[0xFFFD]= 0xDE;
        reg_pc(post_cpu) = 0x02;
    ); 
    // RET Z
    DO_TEST(tstate, 0xC8, 0x00, 0x00, 1, 5,
        test_cpu->flags.z = SET;
        post_cpu->flags.z = SET;
        reg_sp(test_cpu) = 0xFFFC;
        test_mem[0xFFFC] = 0xAD;
        test_mem[0xFFFD] = 0xDE;
        reg_sp(post_cpu) = 0xFFFE;
        post_mem[0xFFFC] = 0xAD;
        post_mem[0xFFFD]= 0xDE;
        reg_pc(post_cpu) = 0xDEAE;
    );
    DO_TEST(tstate, 0xC8, 0x00, 0x00, 1, 2,
        test_cpu->flags.z = CLEAR;
        post_cpu->flags.z = CLEAR;
        reg_sp(test_cpu) = 0xFFFC;
        test_mem[0xFFFC] = 0xAD;
        test_mem[0xFFFD] = 0xDE;
        reg_sp(post_cpu) = 0xFFFC;
        post_mem[0xFFFC] = 0xAD;
        post_mem[0xFFFD]= 0xDE;
        reg_pc(post_cpu) = 0x02;
    );
    // RET C
    DO_TEST(tstate, 0xD8, 0x00, 0x00, 1, 5,
        test_cpu->flags.c = SET;
        post_cpu->flags.c = SET;
        reg_sp(test_cpu) = 0xFFFC;
        test_mem[0xFFFC] = 0xAD;
        test_mem[0xFFFD] = 0xDE;
        reg_sp(post_cpu) = 0xFFFE;
        post_mem[0xFFFC] = 0xAD;
        post_mem[0xFFFD]= 0xDE;
        reg_pc(post_cpu) = 0xDEAE;
    );
    DO_TEST(tstate, 0xD8, 0x00, 0x00, 1, 2,
        test_cpu->flags.c = CLEAR;
        post_cpu->flags.c = CLEAR;
        reg_sp(test_cpu) = 0xFFFC;
        test_mem[0xFFFC] = 0xAD;
        test_mem[0xFFFD] = 0xDE;
        reg_sp(post_cpu) = 0xFFFC;
        post_mem[0xFFFC] = 0xAD;
        post_mem[0xFFFD]= 0xDE;
        reg_pc(post_cpu) = 0x02;
    );

    //RET
    DO_TEST(tstate, 0xC9, 0x00, 0x00, 1, 4,
        reg_sp(test_cpu) = 0xFFFC;
        test_mem[0xFFFC] = 0xAD;
        test_mem[0xFFFD] = 0xDE;
        reg_sp(post_cpu) = 0xFFFE;
        post_mem[0xFFFC] = 0xAD;
        post_mem[0xFFFD]= 0xDE;
        reg_pc(post_cpu) = 0xDEAE;
    );

    //RETI
    DO_TEST(tstate, 0xD9, 0x00, 0x00, 1, 4,
        reg_sp(test_cpu) = 0xFFFC;
        test_mem[0xFFFC] = 0xAD;
        test_mem[0xFFFD] = 0xDE;
        reg_sp(post_cpu) = 0xFFFE;
        post_mem[0xFFFC] = 0xAD;
        post_mem[0xFFFD]= 0xDE;
        reg_pc(post_cpu) = 0xDEAE;
        post_cpu->flags.ime = SET;
    );
    
}

void run_call(CPUTestState *tstate) {
    CPUState *test_cpu = tstate->test_cpu;
    CPUState *post_cpu = tstate->post_cpu;
    BYTE *test_mem = tstate->test_mem;
    BYTE *post_mem = tstate->post_mem;
    // CALL NZ, a16
    DO_TEST(tstate, 0xC4, 0xAD, 0xDE, 3, 6,
        test_cpu->flags.z = CLEAR;
        post_cpu->flags.z = CLEAR;
        reg_sp(post_cpu) = 0xFFFC;
        post_mem[0xFFFC] = 0x03;
        post_mem[0xFFFD] = 0x00;
        reg_pc(post_cpu) = 0xDEAE;
    );
    DO_TEST(tstate, 0xC4, 0xAD, 0xDE, 3, 3,
        test_cpu->flags.z = SET;
        post_cpu->flags.z = SET;
        reg_sp(post_cpu) = 0xFFFE;
        reg_pc(post_cpu) = 0x04;
    );
    // CALL NC, a16
    DO_TEST(tstate, 0xD4, 0xAD, 0xDE, 3, 6,
        test_cpu->flags.c = CLEAR;
        post_cpu->flags.c = CLEAR;
        reg_sp(post_cpu) = 0xFFFC;
        post_mem[0xFFFC] = 0x03;
        post_mem[0xFFFD] = 0x00;
        reg_pc(post_cpu) = 0xDEAE;
    );
    DO_TEST(tstate, 0xD4, 0xAD, 0xDE, 3, 3,
        test_cpu->flags.c = SET;
        post_cpu->flags.c = SET;
        reg_sp(post_cpu) = 0xFFFE;
        reg_pc(post_cpu) = 0x04;
    );
    // CALL Z, a16
    DO_TEST(tstate, 0xCC, 0xAD, 0xDE, 3, 6,
        test_cpu->flags.z = SET;
        post_cpu->flags.z = SET;
        reg_sp(post_cpu) = 0xFFFC;
        post_mem[0xFFFC] = 0x03;
        post_mem[0xFFFD] = 0x00;
        reg_pc(post_cpu) = 0xDEAE;
    );
    DO_TEST(tstate, 0xCC, 0xAD, 0xDE, 3, 3,
        test_cpu->flags.z = CLEAR;
        post_cpu->flags.z = CLEAR;
        reg_sp(post_cpu) = 0xFFFE;
        reg_pc(post_cpu) = 0x04;
    );
    // CALL C, a16
    DO_TEST(tstate, 0xDC, 0xAD, 0xDE, 3, 6,
        test_cpu->flags.c = SET;
        post_cpu->flags.c = SET;
        reg_sp(post_cpu) = 0xFFFC;
        post_mem[0xFFFC] = 0x03;
        post_mem[0xFFFD] = 0x00;
        reg_pc(post_cpu) = 0xDEAE;
    );
    DO_TEST(tstate, 0xDC, 0xAD, 0xDE, 3, 3,
        test_cpu->flags.c = CLEAR;
        post_cpu->flags.c = CLEAR;
        reg_sp(post_cpu) = 0xFFFE;
        reg_pc(post_cpu) = 0x04;
    );
    // CALL a16
    DO_TEST(tstate, 0xCD, 0xAD, 0xDE, 3, 6,
        reg_sp(post_cpu) = 0xFFFC;
        post_mem[0xFFFC] = 0x03;
        post_mem[0xFFFD] = 0x00;
        reg_pc(post_cpu) = 0xDEAE;
    );

    // RST 00
    DO_TEST(tstate, 0xC7, 0x00, 0x00, 1, 4,
        reg_sp(post_cpu) = 0xFFFC;
        post_mem[0xFFFC] = 0x01;
        post_mem[0xFFFD] = 0x00;
        reg_pc(post_cpu) = 0x01;
    );
    // RST 10
    DO_TEST(tstate, 0xD7, 0x00, 0x00, 1, 4,
        reg_sp(post_cpu) = 0xFFFC;
        post_mem[0xFFFC] = 0x01;
        post_mem[0xFFFD] = 0x00;
        reg_pc(post_cpu) = 0x11;
    );
    // RST 20
    DO_TEST(tstate, 0xE7, 0x00, 0x00, 1, 4,
        reg_sp(post_cpu) = 0xFFFC;
        post_mem[0xFFFC] = 0x01;
        post_mem[0xFFFD] = 0x00;
        reg_pc(post_cpu) = 0x21;
    );
    // RST 30
    DO_TEST(tstate, 0xF7, 0x00, 0x00, 1, 4,
        reg_sp(post_cpu) = 0xFFFC;
        post_mem[0xFFFC] = 0x01;
        post_mem[0xFFFD] = 0x00;
        reg_pc(post_cpu) = 0x31;
    );
    // RST 08
    DO_TEST(tstate, 0xCF, 0x00, 0x00, 1, 4,
        reg_sp(post_cpu) = 0xFFFC;
        post_mem[0xFFFC] = 0x01;
        post_mem[0xFFFD] = 0x00;
        reg_pc(post_cpu) = 0x09;
    );
    // RST 18
    DO_TEST(tstate, 0xDF, 0x00, 0x00, 1, 4,
        reg_sp(post_cpu) = 0xFFFC;
        post_mem[0xFFFC] = 0x01;
        post_mem[0xFFFD] = 0x00;
        reg_pc(post_cpu) = 0x19;
    );
    // RST 28
    DO_TEST(tstate, 0xEF, 0x00, 0x00, 1, 4,
        reg_sp(post_cpu) = 0xFFFC;
        post_mem[0xFFFC] = 0x01;
        post_mem[0xFFFD] = 0x00;
        reg_pc(post_cpu) = 0x29;
    );
    // RST 38
    DO_TEST(tstate, 0xFF, 0x00, 0x00, 1, 4,
        reg_sp(post_cpu) = 0xFFFC;
        post_mem[0xFFFC] = 0x01;
        post_mem[0xFFFD] = 0x00;
        reg_pc(post_cpu) = 0x39;
    );

}

void run_ioreg_ops(CPUTestState *tstate) {
    CPUState *test_cpu = tstate->test_cpu;
    CPUState *post_cpu = tstate->post_cpu;
    BYTE *test_mem = tstate->test_mem;
    BYTE *post_mem = tstate->post_mem; 

    // LDH (a8), A
    DO_TEST(tstate, 0xE0, 0x10, 0x00, 2, 3,
        reg_a(test_cpu) = 0xDE;
        reg_a(post_cpu) = 0xDE;
        post_mem[0xFF10] = 0xDE;
    );
    // LD (C), A
    DO_TEST(tstate, 0xE2, 0x00, 0x00, 1, 2,
        reg_a(test_cpu) = 0xDE;
        reg_a(post_cpu) = 0xDE;
        reg_c(test_cpu) = 0x12;
        reg_c(post_cpu) = 0x12;
        post_mem[0xFF12] = 0xDE;
    );
    // LDH A, (a8)
    DO_TEST(tstate, 0xF0, 0x10, 0x00, 2, 3,
        reg_a(test_cpu) = 0xBE;
        test_mem[0xFF10] = 0xDE;
        post_mem[0xFF10] = 0xDE;
        reg_a(post_cpu) = 0xDE;
    );
    // LD A, (C)
    DO_TEST(tstate, 0xF2, 0x00, 0x00, 1, 2,
        reg_a(test_cpu) = 0xBE;
        reg_c(test_cpu) = 0x12;
        test_mem[0xFF12] = 0xDE;
        post_mem[0xFF12] = 0xDE;
        reg_c(post_cpu) = 0x12;
        reg_a(post_cpu) = 0xDE;
    );
}

void run_misc(CPUTestState *tstate) {
    CPUState *test_cpu = tstate->test_cpu;
    CPUState *post_cpu = tstate->post_cpu;
    BYTE *test_mem = tstate->test_mem;
    BYTE *post_mem = tstate->post_mem; 

    /* NOP */
    DO_TEST(tstate, 0x00, 0x00, 0x00, 1, 1, );
    /* LD (a16), SP */
    DO_TEST(tstate, 0x08, 0x10, 0x80, 3, 5,
        reg_sp(test_cpu) = 0xDEAD;
        reg_sp(post_cpu) = 0xDEAD;
        post_mem[0x8010] = 0xAD;
        post_mem[0x8011] = 0xDE;
    );
    /* DI and EI */
    DO_TEST(tstate, 0xF3, 0x00, 0x00, 1, 1,
        test_cpu->flags.ime = SET;
        post_cpu->flags.ime = CLEAR;
    );
    DO_TEST(tstate, 0xFB, 0x00, 0x00, 1, 1,
        test_cpu->flags.ime = CLEAR;
        post_cpu->flags.wants_ime = 1;
    );
    
    /* ADD SP, r8 */
    DO_TEST(tstate, 0xE8, 0x11, 0x00, 2, 4,
        reg_sp(test_cpu) = 0xFFF0;
        reg_sp(post_cpu) = 0x01;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, SET);
    );
    DO_TEST(tstate, 0xE8, -32, 0x00, 2, 4,
        reg_sp(test_cpu) = 0xFFF0;
        reg_sp(post_cpu) = 0xFFD0;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    );
    /* LD HL, SP + r8 */
    DO_TEST(tstate, 0xF8, 0x11, 0x00, 2, 3,
        reg_sp(test_cpu) = 0xFFF0;
        reg_hl(test_cpu) = 0xDEAD;
        reg_sp(post_cpu) = 0xFFF0;
        reg_hl(post_cpu) = 0x0001;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, SET);
    );
    DO_TEST(tstate, 0xF8, -32, 0x00, 2, 3,
        reg_sp(test_cpu) = 0xFFF0;
        reg_hl(test_cpu) = 0xDEAD;
        reg_sp(post_cpu) = 0xFFF0;
        reg_hl(post_cpu) = 0xFFD0;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    );
    /* LD SP, HL */
    DO_TEST(tstate, 0xF9, 0x00, 0x00, 1, 2,
        reg_hl(test_cpu) = 0xDEAD;
        reg_hl(post_cpu) = 0xDEAD;
        reg_sp(post_cpu) = 0xDEAD;
    );

    /* LD (a16), A */
    DO_TEST(tstate, 0xEA, 0x10, 0x80, 3, 4,
        reg_a(test_cpu) = 0xDE;
        reg_a(post_cpu) = 0xDE;
        post_mem[0x8010] = 0xDE;
    );
    /* LD A, (a16) */
    DO_TEST(tstate, 0xFA, 0x10, 0x80, 3, 4,
        reg_a(test_cpu) = 0xBE;
        test_mem[0x8010] = 0xDE;
        post_mem[0x8010] = 0xDE;
        reg_a(post_cpu) = 0xDE;
    );

    /* SCF */
    DO_TEST(tstate, 0x37, 0x00, 0x00, 1, 1,
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, SET);
    );

    /* CPL */
    DO_TEST(tstate, 0x2F, 0x00, 0x00, 1, 1,
        reg_a(test_cpu) = 0b01101001;
        SET_ZNHC(post_cpu, CLEAR, SET, SET, CLEAR);
        reg_a(post_cpu) = 0b10010110;
    )

    /* CCF */
    DO_TEST(tstate, 0x3F, 0x00, 0x00, 1, 1,
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, SET);
    );
    DO_TEST(tstate, 0x3F, 0x00, 0x00, 1, 1,
        SET_ZNHC(test_cpu, CLEAR, SET, SET, CLEAR);
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, SET);
    );

    /* RLCA */
    DO_TEST(tstate, 0x07, 0x00, 0x00, 1, 1,
        reg_a(test_cpu) = 0xBC;
        reg_a(post_cpu) = 0x79;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, SET);
    );
    DO_TEST(tstate, 0x07, 0x00, 0x00, 1, 1,
        reg_a(test_cpu) = 0x7C;
        reg_a(post_cpu) = 0xF8;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    );

    /* RLA */
    DO_TEST(tstate, 0x17, 0x00, 0x00, 1, 1,
        reg_a(test_cpu) = 0xBC;
        reg_a(post_cpu) = 0x78;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, SET);
    );
    DO_TEST(tstate, 0x17, 0x00, 0x00, 1, 1,
        test_cpu->flags.c = SET;
        reg_a(test_cpu) = 0xBC;
        reg_a(post_cpu) = 0x79;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, SET);
    );
    DO_TEST(tstate, 0x17, 0x00, 0x00, 1, 1,
        test_cpu->flags.c = SET;
        reg_a(test_cpu) = 0x7C;
        reg_a(post_cpu) = 0xF9;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    ); 

    /* RRCA */
    DO_TEST(tstate, 0x0F, 0x00, 0x00, 1, 1,
        reg_a(test_cpu) = 0x74;
        reg_a(post_cpu) = 0x3A;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR)
    );
    DO_TEST(tstate, 0x0F, 0x00, 0x00, 1, 1,
        reg_a(test_cpu) = 0x75;
        reg_a(post_cpu) = 0xBA;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, SET);
    );

    /* RRA */
    DO_TEST(tstate, 0x1F, 0x00, 0x00, 1, 1,
        reg_a(test_cpu) = 0x74;
        reg_a(post_cpu) = 0x3A;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR)
    );
    DO_TEST(tstate, 0x1F, 0x00, 0x00, 1, 1,
        reg_a(test_cpu) = 0x75;
        reg_a(post_cpu) = 0x3A;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, SET);
    ); 
    DO_TEST(tstate, 0x1F, 0x00, 0x00, 1, 1,
        test_cpu->flags.c = SET;
        reg_a(test_cpu) = 0x74;
        reg_a(post_cpu) = 0xBA;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    ); 
}

void run_tests(CPUTestState *tstate) {
    
    /* LD <R16>, d16 */
    run_ld_16_imm(tstate);
    /* LD (<R16>), A */
    run_wr_r16_a(tstate);
    /* INC/DEC <r16> */
    run_inc_dec_r16(tstate);
    /* INC/DEC <r8> */
    run_inc_dec_r8(tstate);
    /* LD <r8/HL>, d8 */
    run_ld_imm8(tstate);

    /* ADD HL, <r16> */
    run_add_hl_r16(tstate);
    /* LD A, (<r16>) */
    run_ld_a_r16_mem(tstate);
    /* Relative jumps */
    run_rel_jmp(tstate);
    /* Absolute jumps */
    run_abs_jmp(tstate);
    
    /* 8 Bit Regiter loads */
    run_ld_reg_8(tstate);
    /* 8 Bit non-prefix bitwise/math operations */
    run_math_reg_8(tstate);
    /* 8 Bit immediate value bitwise/math operations */
    run_math_imm_8(tstate);

    /* Push 16 bit reg instructions */
    run_push_reg_16(tstate);
    /* Pop 16 bit reg instructions */
    run_pop_reg_16(tstate);
    /* Call instructions */
    run_call(tstate);
    /* Return instructions */
    run_ret(tstate);
    /* LDH operations, act on IO registers at 0xFF00-0xFF7F */
    run_ioreg_ops(tstate);

    /* Miscellaneous instructions */
    run_misc(tstate);

}
void run_prefix_row0(CPUTestState *tstate) {
    CPUState *test_cpu = tstate->test_cpu;
    CPUState *post_cpu = tstate->post_cpu;
    BYTE *test_mem = tstate->test_mem;
    BYTE *post_mem = tstate->post_mem; 

    DO_TEST(tstate, 0xCB, 0x00, 0x00, 2, 2,
        reg_b(test_cpu) = 0xBC;
        reg_b(post_cpu) = 0x79;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, SET);
    );
    DO_TEST(tstate, 0xCB, 0x00, 0x00, 2, 2,
        reg_b(test_cpu) = 0x7C;
        reg_b(post_cpu) = 0xF8;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0xCB, 0x01, 0x00, 2, 2,
        reg_c(test_cpu) = 0xBC;
        reg_c(post_cpu) = 0x79;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, SET);
    );
    DO_TEST(tstate, 0xCB, 0x02, 0x00, 2, 2,
        reg_d(test_cpu) = 0xBC;
        reg_d(post_cpu) = 0x79;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, SET);
    );
    DO_TEST(tstate, 0xCB, 0x03, 0x00, 2, 2,
        reg_e(test_cpu) = 0xBC;
        reg_e(post_cpu) = 0x79;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, SET);
    );
    DO_TEST(tstate, 0xCB, 0x04, 0x00, 2, 2,
        reg_h(test_cpu) = 0xBC;
        reg_h(post_cpu) = 0x79;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, SET);
    );
    DO_TEST(tstate, 0xCB, 0x05, 0x00, 2, 2,
        reg_l(test_cpu) = 0xBC;
        reg_l(post_cpu) = 0x79;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, SET);
    );
    DO_TEST(tstate, 0xCB, 0x06, 0x00, 2, 4,
        reg_hl(test_cpu) = 0x8010;
        test_mem[0x8010] = 0xBC;
        reg_hl(post_cpu) = 0x8010;
        post_mem[0x8010] = 0x79;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, SET);
    );
    DO_TEST(tstate, 0xCB, 0x07, 0x00, 2, 2,
        reg_a(test_cpu) = 0xBC;
        reg_a(post_cpu) = 0x79;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, SET);
    );
    DO_TEST(tstate, 0xCB, 0x08, 0x00, 2, 2,
        reg_b(test_cpu) = 0x74;
        reg_b(post_cpu) = 0x3A;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR)
    );
    DO_TEST(tstate, 0xCB, 0x08, 0x00, 2, 2,
        reg_b(test_cpu) = 0x75;
        reg_b(post_cpu) = 0xBA;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, SET);
    );
    DO_TEST(tstate, 0xCB, 0x09, 0x00, 2, 2,
        reg_c(test_cpu) = 0x75;
        reg_c(post_cpu) = 0xBA;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, SET);
    );
    DO_TEST(tstate, 0xCB, 0x0A, 0x00, 2, 2,
        reg_d(test_cpu) = 0x75;
        reg_d(post_cpu) = 0xBA;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, SET);
    );
    DO_TEST(tstate, 0xCB, 0x0B, 0x00, 2, 2,
        reg_e(test_cpu) = 0x75;
        reg_e(post_cpu) = 0xBA;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, SET);
    );
    DO_TEST(tstate, 0xCB, 0x0C, 0x00, 2, 2,
        reg_h(test_cpu) = 0x75;
        reg_h(post_cpu) = 0xBA;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, SET);
    );
    DO_TEST(tstate, 0xCB, 0x0D, 0x00, 2, 2,
        reg_l(test_cpu) = 0x75;
        reg_l(post_cpu) = 0xBA;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, SET);
    );
    DO_TEST(tstate, 0xCB, 0x0E, 0x00, 2, 4,
        reg_hl(test_cpu) = 0x8010;
        test_mem[0x8010] = 0x75;
        reg_hl(post_cpu) = 0x8010;
        post_mem[0x8010] = 0xBA;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, SET);
    );
    DO_TEST(tstate, 0xCB, 0x0F, 0x00, 2, 2,
        reg_a(test_cpu) = 0x75;
        reg_a(post_cpu) = 0xBA;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, SET);
    );
}

void run_prefix_row1(CPUTestState *tstate) {
    CPUState *test_cpu = tstate->test_cpu;
    CPUState *post_cpu = tstate->post_cpu;
    BYTE *test_mem = tstate->test_mem;
    BYTE *post_mem = tstate->post_mem;

    DO_TEST(tstate, 0xCB, 0x10, 0x00, 2, 2,
        reg_b(test_cpu) = 0xBC;
        reg_b(post_cpu) = 0x78;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, SET);
    );
     DO_TEST(tstate, 0xCB, 0x10, 0x00, 2, 2,
        test_cpu->flags.c = SET;
        reg_b(test_cpu) = 0xBC;
        reg_b(post_cpu) = 0x79;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, SET);
    );
    DO_TEST(tstate, 0xCB, 0x10, 0x00, 2, 2,
        test_cpu->flags.c = SET;
        reg_b(test_cpu) = 0x3C;
        reg_b(post_cpu) = 0x79;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0xCB, 0x11, 0x00, 2, 2,
        test_cpu->flags.c = SET;
        reg_c(test_cpu) = 0x3C;
        reg_c(post_cpu) = 0x79;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0xCB, 0x12, 0x00, 2, 2,
        test_cpu->flags.c = SET;
        reg_d(test_cpu) = 0x3C;
        reg_d(post_cpu) = 0x79;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0xCB, 0x13, 0x00, 2, 2,
        test_cpu->flags.c = SET;
        reg_e(test_cpu) = 0x3C;
        reg_e(post_cpu) = 0x79;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0xCB, 0x14, 0x00, 2, 2,
        test_cpu->flags.c = SET;
        reg_h(test_cpu) = 0x3C;
        reg_h(post_cpu) = 0x79;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0xCB, 0x15, 0x00, 2, 2,
        test_cpu->flags.c = SET;
        reg_l(test_cpu) = 0x3C;
        reg_l(post_cpu) = 0x79;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0xCB, 0x16, 0x00, 2, 4,
        test_cpu->flags.c = SET;
        reg_hl(test_cpu) = 0x8010;
        test_mem[0x8010] = 0x3C;
        reg_hl(post_cpu) = 0x8010;
        post_mem[0x8010] = 0x79;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0xCB, 0x17, 0x00, 2, 2,
        test_cpu->flags.c = SET;
        reg_a(test_cpu) = 0x3C;
        reg_a(post_cpu) = 0x79;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0xCB, 0x18, 0x00, 2, 2,
        reg_b(test_cpu) = 0x74;
        reg_b(post_cpu) = 0x3A;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR)
    );
    DO_TEST(tstate, 0xCB, 0x18, 0x00, 2, 2,
        reg_b(test_cpu) = 0x75;
        reg_b(post_cpu) = 0x3A;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, SET);
    ); 
    DO_TEST(tstate, 0xCB, 0x18, 0x00, 2, 2,
        test_cpu->flags.c = SET;
        reg_b(test_cpu) = 0x74;
        reg_b(post_cpu) = 0xBA;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    ); 
    DO_TEST(tstate, 0xCB, 0x19, 0x00, 2, 2,
        test_cpu->flags.c = SET;
        reg_c(test_cpu) = 0x74;
        reg_c(post_cpu) = 0xBA;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    ); 
    DO_TEST(tstate, 0xCB, 0x1A, 0x00, 2, 2,
        test_cpu->flags.c = SET;
        reg_d(test_cpu) = 0x74;
        reg_d(post_cpu) = 0xBA;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    ); 
    DO_TEST(tstate, 0xCB, 0x1B, 0x00, 2, 2,
        test_cpu->flags.c = SET;
        reg_e(test_cpu) = 0x74;
        reg_e(post_cpu) = 0xBA;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    ); 
    DO_TEST(tstate, 0xCB, 0x1C, 0x00, 2, 2,
        test_cpu->flags.c = SET;
        reg_h(test_cpu) = 0x74;
        reg_h(post_cpu) = 0xBA;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    ); 
    DO_TEST(tstate, 0xCB, 0x1D, 0x00, 2, 2,
        test_cpu->flags.c = SET;
        reg_l(test_cpu) = 0x74;
        reg_l(post_cpu) = 0xBA;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    ); 
    DO_TEST(tstate, 0xCB, 0x1E, 0x00, 2, 4,
        test_cpu->flags.c = SET;
        reg_hl(test_cpu) = 0x8010;
        test_mem[0x8010] = 0x74;
        reg_hl(post_cpu) = 0x8010;
        post_mem[0x8010] = 0xBA;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    ); 
    DO_TEST(tstate, 0xCB, 0x1F, 0x00, 2, 2,
        test_cpu->flags.c = SET;
        reg_a(test_cpu) = 0x74;
        reg_a(post_cpu) = 0xBA;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    ); 
}

void run_prefix_row2(CPUTestState *tstate) {
    CPUState *test_cpu = tstate->test_cpu;
    CPUState *post_cpu = tstate->post_cpu;
    BYTE *test_mem = tstate->test_mem;
    BYTE *post_mem = tstate->post_mem;

    DO_TEST(tstate, 0xCB, 0x20, 0x00, 2, 2,
        reg_b(test_cpu) = 0xBC;
        reg_b(post_cpu) = 0x78;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, SET);
    );
     DO_TEST(tstate, 0xCB, 0x20, 0x00, 2, 2,
        test_cpu->flags.c = SET;
        reg_b(test_cpu) = 0xBC;
        reg_b(post_cpu) = 0x78;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, SET);
    );
    DO_TEST(tstate, 0xCB, 0x20, 0x00, 2, 2,
        reg_b(test_cpu) = 0x3C;
        reg_b(post_cpu) = 0x78;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0xCB, 0x21, 0x00, 2, 2,
        reg_c(test_cpu) = 0x3C;
        reg_c(post_cpu) = 0x78;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0xCB, 0x22, 0x00, 2, 2,
        reg_d(test_cpu) = 0x3C;
        reg_d(post_cpu) = 0x78;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0xCB, 0x23, 0x00, 2, 2,
        reg_e(test_cpu) = 0x3C;
        reg_e(post_cpu) = 0x78;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0xCB, 0x24, 0x00, 2, 2,
        reg_h(test_cpu) = 0x3C;
        reg_h(post_cpu) = 0x78;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0xCB, 0x25, 0x00, 2, 2,
        reg_l(test_cpu) = 0x3C;
        reg_l(post_cpu) = 0x78;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0xCB, 0x26, 0x00, 2, 4,
        reg_hl(test_cpu) = 0x8010;
        test_mem[0x8010] = 0x3C;
        reg_hl(post_cpu) = 0x8010;
        post_mem[0x8010] = 0x78;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0xCB, 0x27, 0x00, 2, 2,
        reg_a(test_cpu) = 0x3C;
        reg_a(post_cpu) = 0x78;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0xCB, 0x28, 0x00, 2, 2,
        reg_b(test_cpu) = 0x74;
        reg_b(post_cpu) = 0x3A;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR)
    );
    DO_TEST(tstate, 0xCB, 0x28, 0x00, 2, 2,
        reg_b(test_cpu) = 0x75;
        reg_b(post_cpu) = 0x3A;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, SET);
    ); 
    DO_TEST(tstate, 0xCB, 0x28, 0x00, 2, 2,
        reg_b(test_cpu) = 0xA4;
        reg_b(post_cpu) = 0xD2;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    ); 
    DO_TEST(tstate, 0xCB, 0x29, 0x00, 2, 2,
        reg_c(test_cpu) = 0xA4;
        reg_c(post_cpu) = 0xD2;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    ); 
    DO_TEST(tstate, 0xCB, 0x2A, 0x00, 2, 2,
        reg_d(test_cpu) = 0xA4;
        reg_d(post_cpu) = 0xD2;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    ); 
    DO_TEST(tstate, 0xCB, 0x2B, 0x00, 2, 2,
        reg_e(test_cpu) = 0xA4;
        reg_e(post_cpu) = 0xD2;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    ); 
    DO_TEST(tstate, 0xCB, 0x2C, 0x00, 2, 2,
        reg_h(test_cpu) = 0xA4;
        reg_h(post_cpu) = 0xD2;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    ); 
    DO_TEST(tstate, 0xCB, 0x2D, 0x00, 2, 2,
        reg_l(test_cpu) = 0xA4;
        reg_l(post_cpu) = 0xD2;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    ); 
    DO_TEST(tstate, 0xCB, 0x2E, 0x00, 2, 4,
        reg_hl(test_cpu) = 0x8010;
        test_mem[0x8010] = 0xA4;
        reg_hl(post_cpu) = 0x8010;
        post_mem[0x8010] = 0xD2;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    ); 
    DO_TEST(tstate, 0xCB, 0x2F, 0x00, 2, 2,
        reg_a(test_cpu) = 0xA4;
        reg_a(post_cpu) = 0xD2;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    ); 
}

void run_prefix_row3(CPUTestState *tstate) {
    CPUState *test_cpu = tstate->test_cpu;
    CPUState *post_cpu = tstate->post_cpu;
    BYTE *test_mem = tstate->test_mem;
    BYTE *post_mem = tstate->post_mem;

    DO_TEST(tstate, 0xCB, 0x30, 0x00, 2, 2,
        reg_b(test_cpu) = 0xBC;
        reg_b(post_cpu) = 0xCB;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0xCB, 0x30, 0x00, 2, 2,
        reg_b(test_cpu) = 0x3C;
        reg_b(post_cpu) = 0xC3;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0xCB, 0x31, 0x00, 2, 2,
        reg_c(test_cpu) = 0x3C;
        reg_c(post_cpu) = 0xC3;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0xCB, 0x32, 0x00, 2, 2,
        reg_d(test_cpu) = 0x3C;
        reg_d(post_cpu) = 0xC3;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0xCB, 0x33, 0x00, 2, 2,
        reg_e(test_cpu) = 0x3C;
        reg_e(post_cpu) = 0xC3;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0xCB, 0x34, 0x00, 2, 2,
        reg_h(test_cpu) = 0x3C;
        reg_h(post_cpu) = 0xC3;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0xCB, 0x35, 0x00, 2, 2,
        reg_l(test_cpu) = 0x3C;
        reg_l(post_cpu) = 0xC3;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0xCB, 0x36, 0x00, 2, 4,
        reg_hl(test_cpu) = 0x8010;
        test_mem[0x8010] = 0x3C;
        reg_hl(post_cpu) = 0x8010;
        post_mem[0x8010] = 0xC3;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0xCB, 0x37, 0x00, 2, 2,
        reg_a(test_cpu) = 0x3C;
        reg_a(post_cpu) = 0xC3;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    );
    DO_TEST(tstate, 0xCB, 0x38, 0x00, 2, 2,
        reg_b(test_cpu) = 0x74;
        reg_b(post_cpu) = 0x3A;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR)
    );
    DO_TEST(tstate, 0xCB, 0x38, 0x00, 2, 2,
        reg_b(test_cpu) = 0x75;
        reg_b(post_cpu) = 0x3A;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, SET);
    ); 
    DO_TEST(tstate, 0xCB, 0x38, 0x00, 2, 2,
        reg_b(test_cpu) = 0xA4;
        reg_b(post_cpu) = 0x52;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    ); 
    DO_TEST(tstate, 0xCB, 0x39, 0x00, 2, 2,
        reg_c(test_cpu) = 0xA4;
        reg_c(post_cpu) = 0x52;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    ); 
    DO_TEST(tstate, 0xCB, 0x3A, 0x00, 2, 2,
        reg_d(test_cpu) = 0xA4;
        reg_d(post_cpu) = 0x52;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    ); 
    DO_TEST(tstate, 0xCB, 0x3B, 0x00, 2, 2,
        reg_e(test_cpu) = 0xA4;
        reg_e(post_cpu) = 0x52;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    ); 
    DO_TEST(tstate, 0xCB, 0x3C, 0x00, 2, 2,
        reg_h(test_cpu) = 0xA4;
        reg_h(post_cpu) = 0x52;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    ); 
    DO_TEST(tstate, 0xCB, 0x3D, 0x00, 2, 2,
        reg_l(test_cpu) = 0xA4;
        reg_l(post_cpu) = 0x52;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    ); 
    DO_TEST(tstate, 0xCB, 0x3E, 0x00, 2, 4,
        reg_hl(test_cpu) = 0x8010;
        test_mem[0x8010] = 0xA4;
        reg_hl(post_cpu) = 0x8010;
        post_mem[0x8010] = 0x52;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    ); 
    DO_TEST(tstate, 0xCB, 0x3F, 0x00, 2, 2,
        reg_a(test_cpu) = 0xA4;
        reg_a(post_cpu) = 0x52;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, CLEAR, CLEAR);
    ); 
}

void run_prefix_bit_tests(CPUTestState *tstate, BYTE op_base) {
    CPUState *test_cpu = tstate->test_cpu;
    CPUState *post_cpu = tstate->post_cpu;
    BYTE *test_mem = tstate->test_mem;
    BYTE *post_mem = tstate->post_mem;
    
    op_base &= 0xF8;
    BYTE offset = ((op_base & 0x30) >> 3) + ((bit_3(op_base)>>3)==1);

    BYTE test_bit_zero, test_bit_one;
    test_bit_zero = 0xFF - (1 << offset);
    test_bit_one = 1 << offset;

    DO_TEST(tstate, 0xCB, op_base, 0x00, 2, 2,
        reg_b(test_cpu) = test_bit_zero;
        reg_b(post_cpu) = test_bit_zero;
        SET_ZNHC(post_cpu, SET, CLEAR, SET, CLEAR);
    );
    DO_TEST(tstate, 0xCB, op_base, 0x00, 2, 2,
        reg_b(test_cpu) = test_bit_one;
        reg_b(post_cpu) = test_bit_one;
        SET_ZNHC(post_cpu, CLEAR, CLEAR, SET, CLEAR);
    );
    DO_TEST(tstate, 0xCB, op_base+1, 0x00, 2, 2,
        reg_c(test_cpu) = test_bit_zero;
        reg_c(post_cpu) = test_bit_zero;
        SET_ZNHC(post_cpu, SET, CLEAR, SET, CLEAR);
    );
    DO_TEST(tstate, 0xCB, op_base+2, 0x00, 2, 2,
        reg_d(test_cpu) = test_bit_zero;
        reg_d(post_cpu) = test_bit_zero;
        SET_ZNHC(post_cpu, SET, CLEAR, SET, CLEAR);
    );
    DO_TEST(tstate, 0xCB, op_base+3, 0x00, 2, 2,
        reg_e(test_cpu) = test_bit_zero;
        reg_e(post_cpu) = test_bit_zero;
        SET_ZNHC(post_cpu, SET, CLEAR, SET, CLEAR);
    );
    DO_TEST(tstate, 0xCB, op_base+4, 0x00, 2, 2,
        reg_h(test_cpu) = test_bit_zero;
        reg_h(post_cpu) = test_bit_zero;
        SET_ZNHC(post_cpu, SET, CLEAR, SET, CLEAR);
    );
    DO_TEST(tstate, 0xCB, op_base+5, 0x00, 2, 2,
        reg_l(test_cpu) = test_bit_zero;
        reg_l(post_cpu) = test_bit_zero;
        SET_ZNHC(post_cpu, SET, CLEAR, SET, CLEAR);
    );
    DO_TEST(tstate, 0xCB, op_base+6, 0x00, 2, 3,
        reg_hl(test_cpu) = 0x8010;
        test_mem[0x8010] = test_bit_zero;
        reg_hl(post_cpu) = 0x8010;
        post_mem[0x8010] = test_bit_zero;
        SET_ZNHC(post_cpu, SET, CLEAR, SET, CLEAR);
    );
    DO_TEST(tstate, 0xCB, op_base+7, 0x00, 2, 2,
        reg_a(test_cpu) = test_bit_zero;
        reg_a(post_cpu) = test_bit_zero;
        SET_ZNHC(post_cpu, SET, CLEAR, SET, CLEAR);
    );
}

void run_prefix_bit_resets(CPUTestState *tstate, BYTE op_base) {
    CPUState *test_cpu = tstate->test_cpu;
    CPUState *post_cpu = tstate->post_cpu;
    BYTE *test_mem = tstate->test_mem;
    BYTE *post_mem = tstate->post_mem;

    // zero last three bits, round to nearest 8
    op_base &= 0xF8;

    BYTE offset =  ((op_base & 0x30)>>3) + ((bit_3(op_base)>>3)==1);

    BYTE res_bit_zero, res_bit_one;
    res_bit_zero = 0xFF - (1 << offset);
    res_bit_one = 1 << offset;

    DO_TEST(tstate, 0xCB, op_base, 0x00, 2, 2,
        reg_b(test_cpu) = 0xFF;
        reg_b(post_cpu) = res_bit_zero;
    );
    DO_TEST(tstate, 0xCB, op_base, 0x00, 2, 2,
        reg_b(test_cpu) = res_bit_zero;
        reg_b(post_cpu) = res_bit_zero;
    );
    DO_TEST(tstate, 0xCB, op_base, 0x00, 2, 2,
        reg_b(test_cpu) = res_bit_one;
        reg_b(post_cpu) = 0;
    );
    DO_TEST(tstate, 0xCB, op_base+1, 0x00, 2, 2,
        reg_c(test_cpu) = 0xFF;
        reg_c(post_cpu) = res_bit_zero;
    );
    DO_TEST(tstate, 0xCB, op_base+2, 0x00, 2, 2,
        reg_d(test_cpu) = 0xFF;
        reg_d(post_cpu) = res_bit_zero;
    );
    DO_TEST(tstate, 0xCB, op_base+3, 0x00, 2, 2,
        reg_e(test_cpu) = 0xFF;
        reg_e(post_cpu) = res_bit_zero;
    );
    DO_TEST(tstate, 0xCB, op_base+4, 0x00, 2, 2,
        reg_h(test_cpu) = 0xFF;
        reg_h(post_cpu) = res_bit_zero;
    );
    DO_TEST(tstate, 0xCB, op_base+5, 0x00, 2, 2,
        reg_l(test_cpu) = 0xFF;
        reg_l(post_cpu) = res_bit_zero;
    );
    DO_TEST(tstate, 0xCB, op_base+6, 0x00, 2, 4,
        reg_hl(test_cpu) = 0x8010;
        test_mem[0x8010] = 0xFF;
        reg_hl(post_cpu) = 0x8010;
        post_mem[0x8010] = res_bit_zero;
    );
    DO_TEST(tstate, 0xCB, op_base+7, 0x00, 2, 2,
        reg_a(test_cpu) = 0xFF;
        reg_a(post_cpu) = res_bit_zero;
    );
}

void run_prefix_bit_sets(CPUTestState *tstate, BYTE op_base) {
    CPUState *test_cpu = tstate->test_cpu;
    CPUState *post_cpu = tstate->post_cpu;
    BYTE *test_mem = tstate->test_mem;
    BYTE *post_mem = tstate->post_mem;

    // zero last three bits, round to nearest 8
    op_base &= 0xF8;

    BYTE offset = ((op_base & 0x30)>>3) + ((bit_3(op_base)>>3)==1);

    BYTE set_bit_zero, set_bit_one;
    set_bit_zero = 0xFF - (1 << offset);
    set_bit_one = 1 << offset;

    DO_TEST(tstate, 0xCB, op_base, 0x00, 2, 2,
        reg_b(test_cpu) = 0xFF;
        reg_b(post_cpu) = 0xFF;
    );
    DO_TEST(tstate, 0xCB, op_base, 0x00, 2, 2,
        reg_b(test_cpu) = set_bit_zero;
        reg_b(post_cpu) = 0xFF;
    );
    DO_TEST(tstate, 0xCB, op_base, 0x00, 2, 2,
        reg_b(test_cpu) = set_bit_one;
        reg_b(post_cpu) = set_bit_one;
    );
    DO_TEST(tstate, 0xCB, op_base+1, 0x00, 2, 2,
        reg_c(test_cpu) = set_bit_zero;
        reg_c(post_cpu) = 0xFF;
    );
    DO_TEST(tstate, 0xCB, op_base+2, 0x00, 2, 2,
        reg_d(test_cpu) = set_bit_zero;
        reg_d(post_cpu) = 0xFF;
    );
    DO_TEST(tstate, 0xCB, op_base+3, 0x00, 2, 2,
        reg_e(test_cpu) = set_bit_zero;
        reg_e(post_cpu) = 0xFF;
    );
    DO_TEST(tstate, 0xCB, op_base+4, 0x00, 2, 2,
        reg_h(test_cpu) = set_bit_zero;
        reg_h(post_cpu) = 0xFF;
    );
    DO_TEST(tstate, 0xCB, op_base+5, 0x00, 2, 2,
        reg_l(test_cpu) = set_bit_zero;
        reg_l(post_cpu) = 0xFF;
    );
    DO_TEST(tstate, 0xCB, op_base+6, 0x00, 2, 4,
        reg_hl(test_cpu) = 0x8010;
        test_mem[0x8010] = set_bit_zero;
        reg_hl(post_cpu) = 0x8010;
        post_mem[0x8010] = 0xFF;
    );
    DO_TEST(tstate, 0xCB, op_base+7, 0x00, 2, 2,
        reg_a(test_cpu) = set_bit_zero;
        reg_a(post_cpu) = 0xFF;
    );
}

void run_prefix_tests(CPUTestState *tstate) {
    run_prefix_row0(tstate);
    run_prefix_row1(tstate);
    run_prefix_row2(tstate);
    run_prefix_row3(tstate);

    run_prefix_bit_tests(tstate, 0x40);
    run_prefix_bit_tests(tstate, 0x48);
    run_prefix_bit_tests(tstate, 0x50);
    run_prefix_bit_tests(tstate, 0x58);
    run_prefix_bit_tests(tstate, 0x60);
    run_prefix_bit_tests(tstate, 0x68);
    run_prefix_bit_tests(tstate, 0x70);
    run_prefix_bit_tests(tstate, 0x78);
    
    run_prefix_bit_resets(tstate, 0x80);
    run_prefix_bit_resets(tstate, 0x88);
    run_prefix_bit_resets(tstate, 0x90);
    run_prefix_bit_resets(tstate, 0x98);
    run_prefix_bit_resets(tstate, 0xA0);
    run_prefix_bit_resets(tstate, 0xA8);
    run_prefix_bit_resets(tstate, 0xB0);
    run_prefix_bit_resets(tstate, 0xB8);

    run_prefix_bit_sets(tstate, 0xC0);
    run_prefix_bit_sets(tstate, 0xC8);
    run_prefix_bit_sets(tstate, 0xD0);
    run_prefix_bit_sets(tstate, 0xD8);
    run_prefix_bit_sets(tstate, 0xE0);
    run_prefix_bit_sets(tstate, 0xE8);
    run_prefix_bit_sets(tstate, 0xF0);
    run_prefix_bit_sets(tstate, 0xF8);
}

void print_test_status(CPUTestState *tstate) {
    
    printf("\n----------------------------\n\n");
    printf("Ran %d tests (%d regular, %d prefix), %d failed (%d regular, %d prefix).\n", 
        tstate->n_tests + tstate->n_prefix_tests,
        tstate->n_tests, 
        tstate->n_prefix_tests,
        tstate->n_failures + tstate->n_prefix_failures,
        tstate->n_failures,
        tstate->n_prefix_failures
    );

    if (tstate->n_failures > 0) {
        for (int i = 0; i < tstate->n_failures; i++) {
            printf("Failure %d: opcode %02x\n", i+1, tstate->failed_opcodes[i]);
        }
    }
    if (tstate->n_prefix_failures > 0) {
        for (int i = 0; i < tstate->n_prefix_failures; i++) {
            printf("Prefix failure %d: opcode %02x\n", i+1, tstate->failed_prefix_opcodes[i]);
        }
    }

    printf("\n-------------------\nCOVERED OPCODES\n");
    for (int i = 0; i < tstate->n_tests; i++) {
        printf("%02x ", tstate->covered_opcodes[i]);
    }
    printf("\n-------------------\nCOVERED PREFIX OPCODES\n");
    for (int i = 0; i < tstate->n_prefix_tests; i++) {
        printf("%02x ", tstate->covered_prefix_opcodes[i]);
    }
}

int main(void) {

    GBState *test_state = initialize_gb(NULL);
    GBState *post_state = initialize_gb(NULL);
    CPUTestState *tstate = initialize_cpu_tests(test_state, post_state);

    run_tests(tstate);
    run_prefix_tests(tstate);
    print_test_status(tstate);

    teardown_cpu_tests(tstate);
    teardown_gb(test_state);
    teardown_gb(post_state);
    
    return 0;
}
