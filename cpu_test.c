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
    printf("\n----------------\n");

    CPUState *test_cpu = tstate->test_cpu;
    CPUState *post_cpu = tstate->post_cpu;

    /* Note we add 1 to n_m_cycles because the first cycle is only fetching
    the first opcode. */
    for (int i = 0; i < tstate->n_m_cycles + 1; i++) {
        cpu_m_cycle(tstate->test_state);
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
        if (tstate->test_state->code[i] != 
            tstate->post_state->code[i]) {
            failure++;
            printf("MEM FAILURE: code[0x%04x]: (test) %02x (post) %02x\n",
                i, tstate->test_state->code[i], tstate->post_state->code[i]);
        }
    }

    for (WORD i = STACK_CHECK_START; i < STACK_CHECK_STOP; i++) {
        if (tstate->test_state->code[i] != 
            tstate->post_state->code[i]) {
            failure++;
            printf("STACK FAILURE: code[0x%04x]: (test) %02x (post) %02x\n",
                i, tstate->test_state->code[i], tstate->post_state->code[i]);
        }
    }

    for (WORD i = IO_CHECK_START; i < IO_CHECK_STOP; i++) {
        if (tstate->test_state->code[i] != 
            tstate->post_state->code[i]) {
            failure++;
            printf("IOREG FAILURE: code[0x%04x]: (test) %02x (post) %02x\n",
                i, tstate->test_state->code[i], tstate->post_state->code[i]);
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
    if (op0 == 0x36)
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

    memset(&tstate->test_state->code[MEM_CHECK_START], 0, MEM_CHECK_SIZE);
    memset(&tstate->post_state->code[MEM_CHECK_START], 0, MEM_CHECK_SIZE);

    for (int i = 0; i < tstate->opcode_length; i++) {
        tstate->test_state->code[i] = tstate->opcode[i];
        tstate->post_state->code[i] = tstate->opcode[i];
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

#define TEST_RLC(op, dest) STAGE_PREFIX(op); \
                            test_state->dest = 0b10001001; \
                            post_state->dest = 0b00010011; \
                            SET_ZNHC(0, 0, 0, 1); \
                            RUN_TEST_NONJMP(2); \
                            STAGE_PREFIX(op); \
                            test_state->dest = 0b01000000; \
                            post_state->dest = 0b10000000; \
                            SET_ZNHC(0, 0, 0, 0); \
                            RUN_TEST_NONJMP(2);

#define TEST_RL(op, dest) STAGE_PREFIX(op); \
                            test_state->dest = 0b10001001; \
                            test_state->flags.c = 1; \
                            post_state->dest = 0b00010011; \
                            SET_ZNHC(0, 0, 0, 1); \
                            RUN_TEST_NONJMP(2); \
                            STAGE_PREFIX(op); \
                            test_state->dest = 0b10001001; \
                            test_state->flags.c = 0; \
                            post_state->dest = 0b00010010; \
                            SET_ZNHC(0, 0, 0, 1); \
                            RUN_TEST_NONJMP(2); \
                            STAGE_PREFIX(op); \
                            test_state->dest = 0b00001001; \
                            test_state->flags.c = 1; \
                            post_state->dest = 0b00010011; \
                            SET_ZNHC(0, 0, 0, 0); \
                            RUN_TEST_NONJMP(2);

#define TEST_RRC(op, dest) STAGE_PREFIX(op); \
                            test_state->dest = 0b10001001; \
                            post_state->dest = 0b11000100; \
                            SET_ZNHC(0, 0, 0, 1); \
                            RUN_TEST_NONJMP(2); \
                            STAGE_PREFIX(op); \
                            test_state->dest = 0b10001000; \
                            post_state->dest = 0b01000100; \
                            SET_ZNHC(0, 0, 0, 0); \
                            RUN_TEST_NONJMP(2); 

#define TEST_RR(op, dest) STAGE_PREFIX(op); \
                            test_state->dest = 0b10001000; \
                            test_state->flags.c = 1; \
                            post_state->dest = 0b11000100; \
                            SET_ZNHC(0, 0, 0, 0); \
                            RUN_TEST_NONJMP(2); \
                            STAGE_PREFIX(op); \
                            test_state->dest = 0b10001001; \
                            test_state->flags.c = 1; \
                            post_state->dest = 0b11000100; \
                            SET_ZNHC(0, 0, 0, 1); \
                            RUN_TEST_NONJMP(2); 

#define TEST_SLA(op, dest) STAGE_PREFIX(op); \
                            test_state->dest = 0b10001001; \
                            post_state->dest = 0b00010010; \
                            SET_ZNHC(0, 0, 0, 1); \
                            RUN_TEST_NONJMP(2); \
                            STAGE_PREFIX(op); \
                            test_state->dest = 0b01001100; \
                            post_state->dest = 0b10011000; \
                            SET_ZNHC(0, 0, 0, 0); \
                            RUN_TEST_NONJMP(2);

#define TEST_SRA(op, dest) STAGE_PREFIX(op); \
                            test_state->dest = 0b10001011; \
                            post_state->dest = 0b11000101; \
                            SET_ZNHC(0, 0, 0, 1); \
                            RUN_TEST_NONJMP(2); \
                            STAGE_PREFIX(op); \
                            test_state->dest = 0b00011000; \
                            post_state->dest = 0b00001100; \
                            SET_ZNHC(0, 0, 0, 0); \
                            RUN_TEST_NONJMP(2);

#define TEST_SRL(op, dest) STAGE_PREFIX(op); \
                            test_state->dest = 0b10001011; \
                            post_state->dest = 0b01000101; \
                            SET_ZNHC(0, 0, 0, 1); \
                            RUN_TEST_NONJMP(2); \
                            STAGE_PREFIX(op); \
                            test_state->dest = 0b00011000; \
                            post_state->dest = 0b00001100; \
                            SET_ZNHC(0, 0, 0, 0); \
                            RUN_TEST_NONJMP(2);

#define TEST_SWAP(op, dest) STAGE_PREFIX(op); \
                            test_state->dest = 0b10011111; \
                            post_state->dest = 0b11111001; \
                            SET_ZNHC(0, 0, 0, 0); \
                            RUN_TEST_NONJMP(2);

#define TEST_BIT_N(op, dest, __n) STAGE_PREFIX(op); \
                                    test_state->dest = (1 << __n); \
                                    post_state->dest = (1 << __n); \
                                    SET_ZNHC(0, 0, 1, 0); \
                                    RUN_TEST_NONJMP(2); \
                                    STAGE_PREFIX(op); \
                                    test_state->dest = (0xFF - (1 << __n)); \
                                    post_state->dest = (0xFF - (1 << __n)); \
                                    SET_ZNHC(1, 0, 1, 0); \
                                    RUN_TEST_NONJMP(2);

#define TEST_BIT_0(op, dest) TEST_BIT_N(op, dest, 0)
#define TEST_BIT_1(op, dest) TEST_BIT_N(op, dest, 1)
#define TEST_BIT_2(op, dest) TEST_BIT_N(op, dest, 2)
#define TEST_BIT_3(op, dest) TEST_BIT_N(op, dest, 3)
#define TEST_BIT_4(op, dest) TEST_BIT_N(op, dest, 4)
#define TEST_BIT_5(op, dest) TEST_BIT_N(op, dest, 5)
#define TEST_BIT_6(op, dest) TEST_BIT_N(op, dest, 6)
#define TEST_BIT_7(op, dest) TEST_BIT_N(op, dest, 7)

#define TEST_RES_N(op, dest, __n) STAGE_PREFIX(op); \
                                    test_state->dest = 0b11111111; \
                                    post_state->dest = 0b11111111 - (1 << __n); \
                                    RUN_TEST_NONJMP(2); \
                                    STAGE_PREFIX(op); \
                                    test_state->dest = 0; \
                                    post_state->dest = 0; \
                                    RUN_TEST_NONJMP(2);

#define TEST_RES_0(op, dest) TEST_RES_N(op, dest, 0)
#define TEST_RES_1(op, dest) TEST_RES_N(op, dest, 1)
#define TEST_RES_2(op, dest) TEST_RES_N(op, dest, 2)
#define TEST_RES_3(op, dest) TEST_RES_N(op, dest, 3)
#define TEST_RES_4(op, dest) TEST_RES_N(op, dest, 4)
#define TEST_RES_5(op, dest) TEST_RES_N(op, dest, 5)
#define TEST_RES_6(op, dest) TEST_RES_N(op, dest, 6)
#define TEST_RES_7(op, dest) TEST_RES_N(op, dest, 7)

#define TEST_SET_N(op, dest, __n) STAGE_PREFIX(op); \
                                    test_state->dest = 0; \
                                    post_state->dest = (1 << __n); \
                                    RUN_TEST_NONJMP(2); \
                                    STAGE_PREFIX(op); \
                                    test_state->dest = 0b11111111; \
                                    post_state->dest = 0b11111111; \
                                    RUN_TEST_NONJMP(2);

#define TEST_SET_0(op, dest) TEST_SET_N(op, dest, 0)
#define TEST_SET_1(op, dest) TEST_SET_N(op, dest, 1)
#define TEST_SET_2(op, dest) TEST_SET_N(op, dest, 2)
#define TEST_SET_3(op, dest) TEST_SET_N(op, dest, 3)
#define TEST_SET_4(op, dest) TEST_SET_N(op, dest, 4)
#define TEST_SET_5(op, dest) TEST_SET_N(op, dest, 5)
#define TEST_SET_6(op, dest) TEST_SET_N(op, dest, 6)
#define TEST_SET_7(op, dest) TEST_SET_N(op, dest, 7)
                             

#define TEST_ADD_16(op, src, dest, ...) STAGE_TEST(op, 0, 0); \
                                        test_state->dest = 0x0108;\
                                        test_state->src = 0x0204; \
                                        post_state->dest = 0x108; \
                                        post_state->src = 0x0204; \
                                        post_state->dest += post_state->src; \
                                        SET_ZNHC(0, 0, check_half(post_state->dest, 0), 0); \
                                        RUN_TEST_NONJMP(1);

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
    BYTE *test_mem = tstate->test_state->code;
    BYTE *post_mem = tstate->post_state->code; 
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
    BYTE *test_mem = tstate->test_state->code;
    BYTE *post_mem = tstate->post_state->code; 

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
    BYTE *test_mem = tstate->test_state->code;
    BYTE *post_mem = tstate->post_state->code;

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
    BYTE *test_mem = tstate->test_state->code;
    BYTE *post_mem = tstate->post_state->code;

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
    BYTE *test_mem = tstate->test_state->code;
    BYTE *post_mem = tstate->post_state->code;

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
    BYTE *test_mem = tstate->test_state->code;
    BYTE *post_mem = tstate->post_state->code;

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
    BYTE *test_mem = tstate->test_state->code;
    BYTE *post_mem = tstate->post_state->code; 

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
    BYTE *test_mem = tstate->test_state->code;
    BYTE *post_mem = tstate->post_state->code;
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
    BYTE *test_mem = tstate->test_state->code;
    BYTE *post_mem = tstate->post_state->code; 
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
    BYTE *test_mem = tstate->test_state->code;
    BYTE *post_mem = tstate->post_state->code; 

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
    BYTE *test_mem = tstate->test_state->code;
    BYTE *post_mem = tstate->post_state->code;

}

void run_push_reg_16(CPUTestState *tstate) {
    CPUState *test_cpu = tstate->test_cpu;
    CPUState *post_cpu = tstate->post_cpu;
    BYTE *test_mem = tstate->test_state->code;
    BYTE *post_mem = tstate->post_state->code;

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
    BYTE *test_mem = tstate->test_state->code;
    BYTE *post_mem = tstate->post_state->code;

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
    BYTE *test_mem = tstate->test_state->code;
    BYTE *post_mem = tstate->post_state->code;

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
    BYTE *test_mem = tstate->test_state->code;
    BYTE *post_mem = tstate->post_state->code;
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



void run_tests(CPUTestState *tstate) {
    CPUState *test_cpu = tstate->test_cpu;
    CPUState *post_cpu = tstate->post_cpu;
    BYTE *test_mem = tstate->test_state->code;
    BYTE *post_mem = tstate->post_state->code;

    /* NOP */
    DO_TEST(tstate, 0x00, 0x00, 0x00, 1, 1, );

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

    /* LD (a16), SP */
    DO_TEST(tstate, 0x08, 0x10, 0x80, 3, 5,
        reg_sp(test_cpu) = 0xDEAD;
        reg_sp(post_cpu) = 0xDEAD;
        post_mem[0x8010] = 0xAD;
        post_mem[0x8011] = 0xDE;
    );

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

    /* Push 16 bit reg instructions */
    run_push_reg_16(tstate);
    /* Pop 16 bit reg instructions */
    run_pop_reg_16(tstate);
    /* Call instructions */
    run_call(tstate);
    /* Return instructions */
    run_ret(tstate);

    /* DI and EI */
    DO_TEST(tstate, 0xF3, 0x00, 0x00, 1, 1,
        test_cpu->flags.ime = SET;
        post_cpu->flags.ime = CLEAR;
    );
    DO_TEST(tstate, 0xFB, 0x00, 0x00, 1, 1,
        test_cpu->flags.ime = CLEAR;
        post_cpu->flags.wants_ime = 1;
    );
    

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
    if (tstate->n_prefix_failures < 0) {
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

    GBState *test_state = initialize_gb();
    GBState *post_state = initialize_gb();
    CPUTestState *tstate = initialize_cpu_tests(test_state, post_state);

    run_tests(tstate);
    print_test_status(tstate);

    teardown_cpu_tests(tstate);
    teardown_gb(test_state);
    teardown_gb(post_state);
    
    /*
    RUN_TEST_NONJMP(1);

    TEST_16REGS(0x01, TEST_LD_IMM_16);
    TEST_16REGS(0x03, TEST_PM_16, 1);
    TEST_PM_8(0x04, bc.w.h, 1);
    TEST_PM_8(0x05, bc.w.h, -1);
    TEST_PM_8(0x0C, bc.w.l, 1);
    TEST_PM_8(0x0D, bc.w.l, -1);

    TEST_16REGS(0x0B, TEST_PM_16, -1);
    TEST_16REGS(0x09, TEST_ADD_16, hl.dw);

    TEST_PM_8(0x14, de.w.h, 1);
    TEST_PM_8(0x15, de.w.h, -1);
    TEST_PM_8(0x1C, de.w.l, 1);
    TEST_PM_8(0x1D, de.w.l, -1);

    // 8 bit loads row 1
    TEST_LD_8REGS(0x40, bc.w.h, TEST_LD_8);
    TEST_LD_8REGS(0x48, bc.w.l, TEST_LD_8);
    
    // 8 bit loads row 2 
    TEST_LD_8REGS(0x50, de.w.h, TEST_LD_8);
    TEST_LD_8REGS(0x58, de.w.l, TEST_LD_8);

    // 8 bit loads row 3
    TEST_LD_8REGS(0x60, hl.w.h, TEST_LD_8);
    TEST_LD_8REGS(0x68, hl.w.l, TEST_LD_8);
    
    // 8 bit loads row 4
    TEST_LD_8REGS(0x78, a, TEST_LD_8);
    
    // Prefix instructions
    TEST_OP_8REGS(0x00, TEST_RLC);
    TEST_OP_8REGS(0x08, TEST_RRC);

    TEST_OP_8REGS(0x10, TEST_RL);
    TEST_OP_8REGS(0x18, TEST_RR);

    TEST_OP_8REGS(0x20, TEST_SLA);
    TEST_OP_8REGS(0x28, TEST_SRA);

    TEST_OP_8REGS(0x30, TEST_SWAP);
    TEST_OP_8REGS(0x38, TEST_SRL);

    TEST_OP_8REGS(0x40, TEST_BIT_0);
    TEST_OP_8REGS(0x48, TEST_BIT_1);
    TEST_OP_8REGS(0x50, TEST_BIT_2);
    TEST_OP_8REGS(0x58, TEST_BIT_3);
    TEST_OP_8REGS(0x60, TEST_BIT_4);
    TEST_OP_8REGS(0x68, TEST_BIT_5);
    TEST_OP_8REGS(0x70, TEST_BIT_6);
    TEST_OP_8REGS(0x78, TEST_BIT_7);

    TEST_OP_8REGS(0x80, TEST_RES_0);
    TEST_OP_8REGS(0x88, TEST_RES_1);
    TEST_OP_8REGS(0x90, TEST_RES_2);
    TEST_OP_8REGS(0x98, TEST_RES_3);
    TEST_OP_8REGS(0xA0, TEST_RES_4);
    TEST_OP_8REGS(0xA8, TEST_RES_5);
    TEST_OP_8REGS(0xB0, TEST_RES_6);
    TEST_OP_8REGS(0xB8, TEST_RES_7);

    TEST_OP_8REGS(0xC0, TEST_SET_0);
    TEST_OP_8REGS(0xC8, TEST_SET_1);
    TEST_OP_8REGS(0xD0, TEST_SET_2);
    TEST_OP_8REGS(0xD8, TEST_SET_3);
    TEST_OP_8REGS(0xE0, TEST_SET_4);
    TEST_OP_8REGS(0xE8, TEST_SET_5);
    TEST_OP_8REGS(0xF0, TEST_SET_6);
    TEST_OP_8REGS(0xF8, TEST_SET_7);
    // End prefix instructions
    */
    return 0;
}
