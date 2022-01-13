#include "base.h"
#include "macros.h"
#include "gb.c"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define TEST_EQUALS(__r) printf("test->" # __r ": %04x\t post->" # __r ": %04x\n", \
    test_state->__r, post_state->__r); \
    if (test_state->__r != post_state->__r) { \
        failure = 1; \
        printf("FAILURE: " # __r "\n"); \
    }

int test_instruction(
    GBState *test_state,
    GBState *post_state, 
    BYTE *opcode,
    int opcode_length,
    WORD post_pc
) {
    int new_pc = 0;
    int failure = 0;

    printf("Testing opcode %02x", opcode[0]);
    if (opcode_length == 2) {
        printf(" [%02x]", opcode[1]);
    } else if (opcode_length == 3) {
        printf(" [%02x %02x]", opcode[1], opcode[2]);
    }
    printf("\n----------------\n");

    new_pc = execute_instruction(test_state, opcode);

    printf("next PC: %x, specified PC: %x\n", new_pc, post_pc);
    if (new_pc != post_pc) {
        printf("FAILURE: PC\n");
    }

    TEST_EQUALS(a);
    //TEST_EQUALS(pc);
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

#define STAGE_TEST(op0, op1, op2) reset_registers(test_state); \
                                reset_registers(post_state); \
                                opcode[0] = op0; opcode[1] = op1; opcode[2] = op2;

#define RUN_TEST_ABSJMP(len, addr) test_instruction(test_state, post_state, \
                        opcode, len, addr)

#define RUN_TEST_RELJMP(len, offset) test_instruction(test_state, post_state, \
                        opcode, len, post_state->pc + len + offset)

#define RUN_TEST_NONJMP(len) RUN_TEST_RELJMP(len, 0)

#define TEST_LD_8(inst, dest, src) STAGE_TEST(inst, 0, 0); \
                                test_state->src = 0xEA; \
                                post_state->src = 0xEA; post_state->dest = 0xEA; \
                                RUN_TEST_NONJMP(1);

#define STAGE_PREFIX(op) STAGE_TEST(0xCB, op, 0)

#define SET_ZNHC(__z, __n, __h, __c) post_state->flags.z = __z; \
                            post_state->flags.n = __n; \
                            post_state->flags.h = __h; \
                            post_state->flags.c = __c;

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

#define TEST_OP_8REGS(start, __TEST) __TEST(start, bc.w.h); \
                                    __TEST(start+1, bc.w.l); \
                                    __TEST(start+2, de.w.h); \
                                    __TEST(start+3, de.w.l); \
                                    __TEST(start+4, hl.w.h); \
                                    __TEST(start+5, hl.w.l); \
                                    __TEST(start+7, a);

#define TEST_LD_8REGS(start, dest, __TEST) __TEST(start, dest, bc.w.h); \
                                            __TEST(start+1, dest, bc.w.l); \
                                            __TEST(start+2, dest, de.w.h); \
                                            __TEST(start+3, dest, de.w.l); \
                                            __TEST(start+4, dest, hl.w.h); \
                                            __TEST(start+5, dest, hl.w.l); \
                                            __TEST(start+7, dest, a);

#define TEST_LD_IMM_16(op, dest, ...) STAGE_TEST(op, 0xEF, 0xBE); \
                                post_state->dest = (0xBE << 8) + 0xEF; \
                                RUN_TEST_NONJMP(3);

#define TEST_PM_16(op, dest, offset, ...) STAGE_TEST(op, 0, 0); \
                                    test_state->dest = 0x100; \
                                    post_state->dest = 0x100 + offset; \
                                    RUN_TEST_NONJMP(1);

#define TEST_PM_8(op, dest, offset, ...) STAGE_TEST(op, 0, 0); \
                                        test_state->dest = 0x10; \
                                        post_state->dest = 0x10 + offset; \
                                        SET_ZNHC(0, (offset<0), check_half(post_state->dest, 0), 0); \
                                        RUN_TEST_NONJMP(1);                                          

#define TEST_ADD_16(op, src, dest, ...) STAGE_TEST(op, 0, 0); \
                                        test_state->dest = 0x0108;\
                                        test_state->src = 0x0204; \
                                        post_state->dest = 0x108; \
                                        post_state->src = 0x0204; \
                                        post_state->dest += post_state->src; \
                                        SET_ZNHC(0, 0, check_half(post_state->dest, 0), 0); \
                                        RUN_TEST_NONJMP(1);

#define TEST_16REGS(start, __TEST, ...) __TEST(start, bc.dw, __VA_ARGS__); \
                                        __TEST(start+0x10, de.dw, __VA_ARGS__); \
                                        __TEST(start+0x20, hl.dw, __VA_ARGS__); \
                                        __TEST(start+0x30, sp, __VA_ARGS__);

int main(void) {

    BYTE opcode[3] = {0, 0, 0};

    GBState *test_state, *post_state;
    test_state = initialize_state();
    post_state = initialize_state();
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

    return 0;
}
