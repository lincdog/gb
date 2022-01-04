#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define WORD uint16_t
#define BYTE uint8_t
#define START_ADDR 0x100

static char GAMEBOY_LOGO[] = {
    0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
    0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E, 0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
    0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E
};

typedef union {
    struct {
        BYTE l;
        BYTE h;
    } w;
    WORD dw;
} reg;

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

#define a(x) ((size_t)&x)

#define ms_nib(x) (((x) & 0xF0)>>4)
#define ls_nib(x) (((x) & 0x0F))

#define bit_7(x) ((x) & 0x80)
#define bit_6(x) ((x) & 0x40)
#define bit_5(x) ((x) & 0x20)
#define bit_4(x) ((x) & 0x10)
#define bit_3(x) ((x) & 0x08)
#define bit_2(x) ((x) & 0x04)
#define bit_1(x) ((x) & 0x02)
#define bit_0(x) ((x) & 0x01)


/* Memory mapping */
#define REGION_CHECK(x, lb, ub) (((WORD)(x)>=lb)&&((WORD)(x)<=ub)))

#define is_fixed_rom(x) REGION_CHECK(x, 0x0000, 0x3FFF)
#define is_rom_selector(x) REGION_CHECK(x, 0x2000, 0x3FFF)
#define is_banked_rom(x) REGION_CHECK(x, 0x4000, 0x7FFF)
#define is_rom(x) REGION_CHECK(x, 0x0000, 0x7FFF)
#define is_ram_selector(x) REGION_CHECK(x, 0x4000, 0x5FFF)
#define is_vram(x) REGION_CHECK(x, 0x8000, 0x9FFF)
#define is_banked_ram(x) REGION_CHECK(x, 0xA000, 0xBFFF)
#define is_wram(x) REGION_CHECK(x, 0xC000, 0xDFFF)
#define is_eram(x) REGION_CHECK(x, 0xE000, 0xFDFF)
#define is_ioreg(x) REGION_CHECK(x, 0xFF00, 0xFF7F)
#define is_unusable(x) (is_eram(x) || REGION_CHECK(x, 0xFEA0, 0xFEFF))
#define is_hram(x) REGION_CHECK(x, 0xFF80, 0xFFFE)
#define is_ram(x) (REGION_CHECK(x, 0x8000, 0xDFFF) || is_hram(x) || is_ioreg(x))


enum EXEC_STATE {READY, IM_8, IM_16_LSB, IM_16_MSB, CB};

typedef struct GBFlags {
    BYTE z;
    BYTE n;
    BYTE h;
    BYTE c;
    BYTE ime;
} GBFlags;

typedef struct {
    BYTE a;
    reg bc;
    reg de;
    reg hl;
    WORD sp;
    WORD pc;
    struct GBFlags flags;
    
    BYTE *code;
} GBState;

BYTE read_8(WORD addr, BYTE *code) {
    return code[addr];
}
WORD read_16(WORD addr, BYTE *code) {
    return (WORD)code[addr];
}

int write_8(WORD addr, BYTE *code, BYTE data) {
    code[addr] = data;

    return 1;
}

int write_16(WORD addr, BYTE *code, WORD data) {
    code[addr] = data;

    return 1;
}

#define check_half(a, b) ((((a)+(b)) & 0x1F) > 9)
#define check_carry_16(a, b) (((int)(a) + (int)(b)) > 0xFFFF)
#define check_carry_8(a, b) (((WORD)(a) + (WORD)(b)) > 0xFF)
#define check_sub(a, b) ((b) > (a));

BYTE code[32767];


void execute_program(GBState *state) {

    BYTE *opcode = &state->code[state->pc];
    char offset;
    BYTE scratch;

#define INC_8(r) r += 1; if (r == 0) state->flags.z = 1; \
                if (check_half(r, 0)) state->flags.h = 1; \
                state->flags.n = 0; \
                break;
#define DEC_8(r) r -= 1; if (r == 0) state->flags.z = 1; \
                if (check_half(r, 0)) state->flags.h = 1; \
                state->flags.n = 0; \
                break;
#define ADD_16_REG(dest, src) \
    if (check_carry_16(state->hl.dw, state->bc.dw)) state->flags.c = 1; \
    if (check_half(state->hl.dw, state->bc.dw)) state->flags.h = 1; \
    state->flags.n = 0; state->hl.dw += state->bc.dw; \
    break;

#define LD_REG(dest, src) state->dest = state->src; break;
#define MEM_REG(dest, src) write_8(state->dest, state->code, state->src); break;

#define ADD_8(dest, src) if (check_carry_8(state->dest, src)) state->flags.c = 1; \
                        state->dest += src; \
                        if (state->dest == 0) state->flags.z = 1; \
                        if (check_half(state->dest, 0)) state->flags.h = 1; \
                        state->flags.n = 0; \
                        break;
#define ADD_8_REG(dest, rsrc) ADD_8(dest, state->rsrc)
#define ADC_8(dest, src) ADD_8(dest, src + state->flags.c)
#define ADC_8_REG(dest, rsrc) ADC_8(dest, state->rsrc)

#define SUB_8(dest, src) if (check_carry_8(state->dest, -(src))) state->flags.c = 1; \
                        state->dest -= src; \
                        if (state->dest == 0) state->flags.z = 1; \
                        if (check_half(state->dest, 0)) state->flags.h = 1; \
                        state->flags.n = 1; \
                        break;
#define SUB_8_REG(dest, rsrc) SUB_8(dest, state->rsrc)
#define SBC_8(dest, src) SUB_8(dest, src + state->flags.c)
#define SBC_8_REG(dest, rsrc) SBC_8(dest, state->rsrc)

#define AND_8(dest, src) state->dest &= src; \
                        if (state->dest == 0) state->flags.z = 1; \
                        state->flags.n = 0; \
                        state->flags.h = 1; \
                        state->flags.c = 0; \
                        break;
#define AND_8_REG(dest, rsrc) AND_8(dest, state->rsrc);

#define OR_8(dest, src) state->dest |= src; \
                        if (state->dest == 0) state->flags.z = 1; \
                        state->flags.n = 0; \
                        state->flags.h = 0; \
                        state->flags.c = 0; \
                        break;
#define OR_8_REG(dest, rsrc) OR_8(dest, state->rsrc);

#define XOR_8(dest, src) state->dest ^= src; \
                        if (state->dest == 0) state->flags.z = 1; \
                        state->flags.n = 0; \
                        state->flags.h = 0; \
                        state->flags.c = 0; \
                        break;
#define XOR_8_REG(dest, rsrc) XOR_8(dest, state->rsrc);

#define CP_8(dest, src) offset = state->dest - src; \
                        state->flags.n = 1; \
                        if (offset == 0) state->flags.z = 1; \
                        if (offset < 0) state->flags.c = 1; \
                        if (check_half(offset, 0)) state->flags.h = 1; \
                        break;
#define CP_8_REG(dest, rsrc) CP_8(dest, state->rsrc);
                        
#define PUSH_16(data) state->sp -= 2; \
                      write_16(state->sp, state->code, data); \
                      break;
#define PUSH_8(data) state->sp -= 1; \
                     write_8(state->sp, state->code, data); \
                     break;

#define POP_8(dest) dest = read_8(state->sp, state->code); \
                    state->sp += 1; \
                    break;

#define POP_16(dest) dest = read_16(state->sp, state->code); \
                     state->sp += 2; \
                     break;

#define CALL(dest) PUSH_16(state->pc + 1); state->pc = dest; \
                   break;
#define RET() POP_16(state->pc); break;
#define ILLEGAL(inst) print("Illegal instruction %x\n", inst); exit(1); break;

    // Main loop
    for (;;) {

        switch (*opcode) {
            case 0x00:
                break;
            case 0x01:
                state->bc.w.l = opcode[1];
                state->bc.w.h = opcode[2];
                state->pc += 2;
                break;
            case 0x02:
                write_8(state->bc.dw, state->code, state->a);
                break;
            case 0x03:
                // Increment BC
                state->bc.dw += 1;
                break;
            case 0x04:
                // Increment B
                INC_8(state->bc.w.h);
            case 0x05:
                // Decrement B
                DEC_8(state->bc.w.h);
            case 0x06:
                // Load b with immediate 8bits
                state->bc.w.h = opcode[1];
                state->pc += 1;
                break;
            case 0x07:
                // Rotate A left
                scratch = (state->a & 0x80) != 0;
                state->a = (state->a << 1) + scratch;
                state->flags.c = scratch;
                state->flags.z = 0;
                state->flags.n = 0;
                state->flags.h = 0;
                break;
            case 0x08:
                // Load immediate 16 bit address with SP
                write_16((WORD)opcode[1], state->code, state->sp);
                state->pc += 2;
                break;
            case 0x09:
                ADD_16_REG(state->hl.dw, state->bc.dw);
            case 0x0A:
                state->a = read_8(state->bc.dw, state->code);
                break;
            case 0x0B:
                state->bc.dw -= 1;
                break;
            case 0x0C:
                INC_8(state->bc.w.l);
            case 0x0D:
                DEC_8(state->bc.w.l);
            case 0x0E:
                state->bc.w.l = opcode[1];
                state->pc += 1;
                break;
            case 0x0F:
                // Rotate right A
                scratch = state->a & 0x01;
                state->a = (scratch << 7) | (state->a >> 1);
                state->flags.c = scratch;

                state->flags.z = 0;
                state->flags.n = 0;
                state->flags.h = 0;
                break;
            case 0x10:
                // STOP
                opcode[1];
                break;
            case 0x11:
                state->de.w.l = opcode[1];
                state->de.w.h = opcode[2];
                state->pc += 2;
                break;
            case 0x12:
                write_8(state->de.dw, state->code, state->a);
                break;
            case 0x13:
                state->de.dw += 1;
                break;
            case 0x14:
                INC_8(state->de.w.h);
            case 0x15:
                DEC_8(state->de.w.h);
            case 0x16:
                state->de.w.h = opcode[1];
                state->pc += 1;
                break;
            case 0x17:
                // Rotate left A through carry
                // 0 0 0 C
                scratch = (state->a & 0x80) != 0;
                state->a = (state->a << 1) + state->flags.c;
                state->flags.c = scratch;

                state->flags.z = 0;
                state->flags.n = 0;
                state->flags.h = 0;
                break;
            case 0x18:
                offset = (char)opcode[1];
                state->pc += 1;
                state->pc += offset;
                break;
            case 0x19:
                ADD_16_REG(state->hl.dw, state->de.dw);
            case 0x1A:
                state->a = read_8(state->de.dw, state->code);
                break;
            case 0x1B:
                state->de.dw -= 1;
                break;
            case 0x1C:
                INC_8(state->de.w.l);
            case 0x1D:
                DEC_8(state->de.w.l);
            case 0x1E:
                state->de.w.l = opcode[1];
                state->pc += 1;
                break;
            case 0x1F:
                // rotate A right thru carry
                // Set 0 0 0 C
                // Rotate Right A through carry
                // Sets 0 0 0 C
                scratch = state->flags.c;
                state->flags.c = state->a & 0x01;
                state->a = (scratch << 7) | (state->a >> 1);

                state->flags.z = 0;
                state->flags.n = 0;
                state->flags.h = 0;
                break;

            case 0x20:
                // Relative jump if zero flag not set
                offset = (char)opcode[1];
                state->pc += 1;

                if (state->flags.z == 0) {
                    state->pc += offset;
                }

                break;
            case 0x21:
                state->hl.w.l = opcode[1];
                state->hl.w.h = opcode[2];
                state->pc += 2;

                break;
            case 0x22:
                write_8(state->hl.dw, state->code, state->a);
                state->hl.dw += 1;
                break;
            case 0x23:
                state->hl.dw += 1;
                break;
            case 0x24:
                INC_8(state->hl.w.h);
            case 0x25:
                DEC_8(state->hl.w.h);
            case 0x26:
                state->hl.w.h = opcode[1];
                state->pc += 1;
                break;
            case 0x27:
                // Decimal adjust A
                //SET Z - 0 C
                if (((state->a & 0xF) > 9) || (state->flags.h)) {
                    if (check_carry_8(state->a, 6)) 
                        state->flags.z = 1;  
                    state->a += 6;
                }
                if (((state->a & 0xF0)>>4) > 9) {
                    if (check_carry_8(state->a, 0x60)) 
                        state->flags.z = 1;
                    state->a += 0x60;
                }
                if (state->a == 0) {
                    state->flags.z = 1;
                }
                state->flags.h = 0;
                break;

            case 0x28:
                offset = (char)opcode[1];
                state->pc += 1;
                
                if (state->flags.z == 1) {
                    state->pc += offset;
                }
                break;
            case 0x29:
                ADD_16_REG(state->hl.dw, state->de.dw);
            case 0x2A:
                state->a = read_8(state->hl.dw, state->code);
                state->hl.dw += 1;

                break;
            case 0x2B:
                state->hl.dw -= 1;
                break;
            case 0x2C:
                INC_8(state->hl.w.l);
            case 0x2D:
                DEC_8(state->hl.w.l);
            case 0x2E:
                state->hl.w.l = opcode[1];
                state->pc += 1;
                break;
            case 0x2F:
                state->a = ~state->a;
                state->flags.n = 1;
                state->flags.h = 1;
                break;
            case 0x30:
                offset = (char)opcode[1];
                state->pc++;
                if (state->flags.c == 0) {
                    state->pc += offset;
                }
                break;
            case 0x31:
                state->sp = (WORD)opcode[1];
                state->pc += 2;
                break;
            case 0x32:
                write_8(state->hl.dw, state->code, state->a);
                state->hl.dw -= 1;
                break;
            case 0x33:
                state->sp += 1;
                break;
            case 0x34:
                scratch = read_8(state->hl.dw, state->code);
                if (check_half(scratch, 1)) state->flags.h = 1;
                if ((scratch + 1) == 0) state->flags.z = 1;
                state->flags.n = 0;
                write_8(state->hl.dw, state->code, scratch + 1);
                break;
            case 0x35:
                scratch = read_8(state->hl.dw, state->code);
                if (check_half(scratch, -1)) state->flags.h = 1;
                if ((scratch - 1) == 0) state->flags.z = 1;
                state->flags.n = 1;
                write_8(state->hl.dw, state->code, scratch - 1);
                break;
            case 0x36:
                write_8(state->hl.dw, state->code, opcode[1]);
                state->pc += 1;
                break;
            case 0x37:
                state->flags.n = 0;
                state->flags.h = 0;
                state->flags.c = 1;
                break;
            case 0x38:
                offset = (char)opcode[1];
                state->pc += 1;
                if (state->flags.c == 1) {
                    state->pc += offset;
                }
                break;
            case 0x39:
                ADD_16_REG(state->hl.dw, state->sp);
            case 0x3A:
                state->a = read_8(state->hl.dw, state->code);
                state->hl.dw -= 1;
                break;
            case 0x3B:
                state->sp -= 1;
                break;
            case 0x3C:
                INC_8(state->a);
            case 0x3D:
                DEC_8(state->a);
            case 0x3E:
                state->a = opcode[1];
                break;
            case 0x3F:
                state->flags.n = 0;
                state->flags.h = 0;
                state->flags.c = ~state->flags.c;
                break;
            case 0x40:
                // B, B
                LD_REG(bc.w.h, bc.w.h);
            case 0x41:
                // B, C
                LD_REG(bc.w.h, bc.w.l);
            case 0x42:
                // B, D
                LD_REG(bc.w.h, de.w.h);
            case 0x43:
                // B E
                LD_REG(bc.w.h, de.w.l);
            case 0x44:
                // B H
                LD_REG(bc.w.h, hl.w.h);
            case 0x45:
                // B L
                LD_REG(bc.w.h, hl.w.l);
            case 0x46:
                // B (HL)
                state->bc.w.h = read_8(state->hl.dw, state->code);
                break;
            case 0x47:
                LD_REG(bc.w.h, a);
            case 0x48:
                LD_REG(bc.w.l, bc.w.h);
            case 0x49:
                LD_REG(bc.w.l, bc.w.l);
            case 0x4A:
                LD_REG(bc.w.l, de.w.h);
            case 0x4B:
                LD_REG(bc.w.l, de.w.l);
            case 0x4C:
                LD_REG(bc.w.l, hl.w.h);
            case 0x4D:
                LD_REG(bc.w.l, hl.w.l);
            case 0x4E:
                state->bc.w.l = read_8(state->hl.dw, state->code);
                break;
            case 0x4F:
                LD_REG(bc.w.l, a);
            case 0x50:
                // D, B
                LD_REG(de.w.h, bc.w.h);
            case 0x51:
                // D, C
                LD_REG(de.w.h, bc.w.l);
            case 0x52:
                // D, D
                LD_REG(de.w.h, de.w.h);
            case 0x53:
                // D E
                LD_REG(de.w.h, de.w.l);
            case 0x54:
                // D H
                LD_REG(de.w.h, hl.w.h);
            case 0x55:
                // D L
                LD_REG(de.w.h, hl.w.l);
            case 0x56:
                // D (HL)
                state->de.w.h = read_8(state->hl.dw, state->code);
                break;
            case 0x57:
                LD_REG(bc.w.h, a);
            case 0x58:
                LD_REG(de.w.l, bc.w.h);
            case 0x59:
                LD_REG(de.w.l, bc.w.l);
            case 0x5A:
                LD_REG(de.w.l, de.w.h);
            case 0x5B:
                LD_REG(de.w.l, de.w.l);
            case 0x5C:
                LD_REG(bc.w.l, hl.w.h);
            case 0x5D:
                LD_REG(bc.w.l, hl.w.l);
            case 0x5E:
                state->de.w.l = read_8(state->hl.dw, state->code);
                break;
            case 0x5F:
                LD_REG(de.w.l, a);
                break;
            case 0x60:
                // H, B
                LD_REG(hl.w.h, bc.w.h);
            case 0x61:
                // H, C
                LD_REG(hl.w.h, bc.w.l);
            case 0x62:
                // H, D
                LD_REG(hl.w.h, de.w.h);
            case 0x63:
                // H E
                LD_REG(hl.w.h, de.w.l);
            case 0x64:
                // H H
                LD_REG(hl.w.h, hl.w.h);
            case 0x65:
                // h L
                LD_REG(hl.w.h, hl.w.l);
            case 0x66:
                // H (HL)
                state->hl.w.h = read_8(state->hl.dw, state->code);
                break;
            case 0x67:
                LD_REG(hl.w.h, a);
            case 0x68:
                LD_REG(hl.w.l, bc.w.h);
            case 0x69:
                LD_REG(hl.w.l, bc.w.l);
            case 0x6A:
                LD_REG(hl.w.l, de.w.h);
            case 0x6B:
                LD_REG(hl.w.l, de.w.l);
            case 0x6C:
                LD_REG(hl.w.l, hl.w.h);
            case 0x6D:
                LD_REG(hl.w.l, hl.w.l);
            case 0x6E:
                state->hl.w.l = read_8(state->hl.dw, state->code);
                break;
            case 0x6F:
                LD_REG(hl.w.l, a);
            case 0x70:
                MEM_REG(hl.dw, bc.w.h);
            case 0x71:
                MEM_REG(hl.dw, bc.w.l);
            case 0x72:
                MEM_REG(hl.dw, de.w.h);
            case 0x73:
                MEM_REG(hl.dw, de.w.l);
            case 0x74:
                MEM_REG(hl.dw, hl.w.h);
            case 0x75:
                MEM_REG(hl.dw, hl.w.l);
            case 0x76:
                // HALT
                printf("HALT CALLED\n");
                break;
            case 0x77:
                MEM_REG(hl.dw, a);
            case 0x78:
                LD_REG(a, bc.w.h);
            case 0x79:
                LD_REG(a, bc.w.l);
            case 0x7A:
                LD_REG(a, de.w.h);
            case 0x7B:
                LD_REG(a, de.w.l);
            case 0x7C:
                LD_REG(a, hl.w.h);
            case 0x7D:
                LD_REG(a, hl.w.l);
            case 0x7E:
                state->a = read_8(state->hl.dw, state->code);
                break;
            case 0x7F:
                LD_REG(a, a);
            case 0x80:
                ADD_8_REG(a, bc.w.h);
            case 0x81:
                ADD_8_REG(a, bc.w.l);
            case 0x82:
                ADD_8_REG(a, de.w.h);
            case 0x83: 
                ADD_8_REG(a, de.w.l);
            case 0x84:
                ADD_8_REG(a, hl.w.h);
            case 0x85:
                ADD_8_REG(a, hl.w.l);
            case 0x86:
                ADD_8(a, read_8(state->hl.dw, state->code));
            case 0x87: 
                ADD_8_REG(a, a);
            case 0x88:
                ADC_8_REG(a, bc.w.h);
            case 0x89:
                ADC_8_REG(a, bc.w.l);
            case 0x8A:
                ADC_8_REG(a, de.w.h);
            case 0x8B:
                ADC_8_REG(a, de.w.l);
            case 0x8C:
                ADC_8_REG(a, hl.w.h);
            case 0x8D:
                ADC_8_REG(a, hl.w.l);
            case 0x8E:
                ADC_8(a, read_8(state->hl.dw, state->code));
            case 0x8F:
                ADC_8_REG(a, a);
            case 0x90:
                SUB_8_REG(a, bc.w.h);
            case 0x91:
                SUB_8_REG(a, bc.w.l);
            case 0x92:
                SUB_8_REG(a, de.w.h);
            case 0x93:
                SUB_8_REG(a, de.w.l);
            case 0x94:
                SUB_8_REG(a, hl.w.h);
            case 0x95:
                SUB_8_REG(a, hl.w.l);
            case 0x96:
                SUB_8(a, read_8(state->hl.dw, state->code));
            case 0x97:
                SUB_8_REG(a, a);
            case 0x98:
                SBC_8_REG(a, bc.w.h);
            case 0x99:
                SBC_8_REG(a, bc.w.l);
            case 0x9A:
                SBC_8_REG(a, de.w.h);
            case 0x9B:
                SBC_8_REG(a, de.w.l);
            case 0x9C:
                SBC_8_REG(a, hl.w.h);
            case 0x9D:
                SBC_8_REG(a, hl.w.l);
            case 0x9E:
                SBC_8(a, read_8(state->hl.dw, state->code));
            case 0x9F:
                SBC_8_REG(a, a);
            case 0xA0:
                AND_8_REG(a, bc.w.h);
            case 0xA1:
                AND_8_REG(a, bc.w.l);
            case 0xA2:
                AND_8_REG(a, de.w.h);
            case 0xA3: 
                AND_8_REG(a, de.w.l);
            case 0xA4:
                AND_8_REG(a, hl.w.h);
            case 0xA5:
                AND_8_REG(a, hl.w.l);
            case 0xA6:
                AND_8(a, read_8(state->hl.dw, state->code));
            case 0xA7:
                AND_8_REG(a, a);
            case 0xA8:
                XOR_8_REG(a, bc.w.h);
            case 0xA9:
                XOR_8_REG(a, bc.w.l);
            case 0xAA:
                XOR_8_REG(a, de.w.h);
            case 0xAB:
                XOR_8_REG(a, de.w.l);
            case 0xAC:
                XOR_8_REG(a, hl.w.h);
            case 0xAD:
                XOR_8_REG(a, hl.w.l);
            case 0xAE:
                XOR_8(a, read_8(state->hl.dw, state->code));
            case 0xAF:
                CP_8_REG(a, a);
            case 0xB0:
                OR_8_REG(a, bc.w.h);
            case 0xB1:
                OR_8_REG(a, bc.w.l);
            case 0xB2:
                OR_8_REG(a, de.w.h);
            case 0xB3:
                OR_8_REG(a, de.w.l);
            case 0xB4:
                OR_8_REG(a, hl.w.h);
            case 0xB5:
                OR_8_REG(a, hl.w.l);
            case 0xB6:
                OR_8(a, read_8(state->hl.dw, state->code));
            case 0xB7:
                OR_8_REG(a, a);
            case 0xB8:
                CP_8_REG(a, bc.w.h);
            case 0xB9:
                CP_8_REG(a, bc.w.l);
            case 0xBA:
                CP_8_REG(a, de.w.h);
            case 0xBB:
                CP_8_REG(a, de.w.l);
            case 0xBC:
                CP_8_REG(a, hl.w.h);
            case 0xBD:
                CP_8_REG(a, hl.w.l);
            case 0xBE:
                CP_8(a, read_8(state->hl.dw, state->code));
            case 0xBF:
                CP_8_REG(a, a);
            case 0xC0:
                // RET NZ
                if (state->flags.z == 0) {
                    RET()
                }
                break;
            case 0xC1:
                // POP BC
                POP_16(state->bc.dw);
            case 0xC2:
                // JP NZ a16
                if (state->flags.z == 0) {
                    state->pc = (WORD)opcode[1];
                }
                break;
            case 0xC3:
                // JP a16
                state->pc = (WORD)opcode[1];
                break;
            case 0xC4:
                if (state->flags.z == 0) {
                    CALL((WORD)opcode[1]);
                }
                break;
            case 0xC5:
                PUSH_16(state->bc.dw);
                break;
            case 0xC6:
                ADD_8(a, opcode[1]);
            case 0xC7:
                CALL((WORD)0x0000);
            case 0xC8:
                if (state->flags.z == 1) {
                    RET();
                }
                break;
            case 0xC9:
                RET();
            case 0xCA:
                if (state->flags.z == 1) {
                    state->pc = (WORD)opcode[1];
                }
                break;
            case 0xCB:
                //HANDLE_PREFIX_INSTR
                break;
            case 0xCC:
                if (state->flags.z == 1) {
                    CALL((WORD)opcode[1]);
                }
                break;
            case 0xCD:
                CALL((WORD)opcode[1]);
            case 0xCE:
                ADC_8(a, opcode[1]);
            case 0xCF:
                CALL((WORD)0x0008);
            case 0xD0:
                if (state->flags.c == 0) {
                    RET();
                }
                break;
            case 0xD1:
                POP_16(state->de.dw);
            case 0xD2:
                if (state->flags.c == 0) {
                    state->pc = (WORD)opcode[1];
                }
                break;
            case 0xD3:
                ILLEGAL(0xD3);
            case 0xD4:
                if (state->flags.c == 0) {
                    CALL((WORD)opcode[1]);
                }
                break;
            case 0xD5:
                PUSH_16(state->de.dw);
            case 0xD6:
                SUB_8(a, opcode[1]);
            case 0xD7:
                CALL((WORD)0x0010);
            case 0xD8:
                if (state->flags.c == 1) {
                    RET();
                }
                break;
            case 0xD9:
                state->flags.ime = 1;
                RET();
            case 0xDA:
                if (state->flags.c == 1) {
                    state->pc = (WORD)opcode[1];
                }
                break;
            case 0xDB:
               ILLEGAL(0xDB);
            case 0xDC:
                if (state->flags.c == 1) {
                    CALL((WORD)opcode[1]);
                }
                break;
            case 0xDD:
                ILLEGAL(0xDD);
            case 0xDE:
                SBC_8(a, opcode[1]);
            case 0xDF:
                CALL((WORD)0x0018);
                break;
            case 0xE0:
                write_8((WORD)(0xFF00 + opcode[1]), state->code, state->a);
                break;
            case 0xE1:
                state->hl.dw = read_16(state->sp, state->code);
                state->sp += 2;
                break;
            case 0xE2:
                write_8((WORD)(0xFF00 + state->bc.w.l), state->code, state->a);
                break;
            case 0xE3:
                ILLEGAL(0xE3);
            case 0xE4:
                ILLEGAL(0xE4);
            case 0xE5:
                PUSH_16(state->hl.dw);
            case 0xE6:
                AND_8(a, opcode[1]);
            case 0xE7:
                CALL((WORD)0x0020);
            case 0xE8:
                offset = (char)opcode[1];
                if (check_half(state->sp, offset)) {
                    state->flags.h = 1;
                }
                if (check_carry_16(state->sp, offset)) {
                    state->flags.c = 1;
                }
                state->flags.z = 0;
                state->flags.n = 0;
                state->sp += (WORD)offset;
                break;
            case 0xE9:
                state->pc = state->hl.dw;
                break;
            case 0xEA:
                write_8((WORD)opcode[1], state->code, state->a);
                break;
            case 0xEB:
                ILLEGAL(0xEB);
            case 0xEC:
                ILLEGAL(0xEB);
            case 0xED:
                ILLEGAL(0xED);
            case 0xEE:
                XOR_8(a, opcode[1]);
            case 0xEF:
                CALL((WORD)0x0028);
            case 0xF0:
                state->a = read_8((WORD)(0xFF00+opcode[1]), state->code);
                break;
            case 0xF1:
                POP_8(scratch);
                state->flags.z = (bit_7(scratch)>>7);
                state->flags.n = (bit_6(scratch)>>6);
                state->flags.h = (bit_5(scratch)>>5);
                state->flags.c = (bit_4(scratch)>>4);

                POP_8(state->a);
                break;
            case 0xF2:
                state->a = read_8((WORD)(0xFF00+state->bc.w.l), state->code);
                break;
            case 0xF3:
                state->flags.ime = 0;
                break;
            case 0xF4:
                ILLEGAL(0xF4);
            case 0xF5:
                scratch = ((state->flags.z << 7) 
                         | (state->flags.n << 6)
                         | (state->flags.h << 5)
                         | (state->flags.c << 4));
                PUSH_8(scratch);
                PUSH_8(state->a);
                break;
            case 0xF6:
                OR_8(a, opcode[1]);
            case 0xF7:
                CALL((WORD)0x0030);
            case 0xF8:
                offset = (char)opcode[1];
                if (check_carry_16(state->sp, (WORD)offset)) {
                    state->flags.c = 1;
                }
                if (check_half(state->sp, (WORD)offset)) {
                    state->flags.h = 1;
                }
                state->flags.z = 0;
                state->flags.n = 0;
                state->hl.dw = state->sp + (WORD)offset;
                break;
            case 0xF9:
                state->sp = state->hl.dw;
                break;
            case 0xFA:
                state->a = read_8((WORD)opcode[1], state->code);
                break;
            case 0xFB:
                state->flags.ime = 1;
                break;
            case 0xFC:
                ILLEGAL(0xFC);
            case 0xFD:
                ILLEGAL(0xFD);
            case 0xFE:
                CP_8(a, opcode[1]);
            case 0xFF:
                CALL((WORD)0x0038);
        }
    }

}

int main(void) {
    FILE *fp;
    fp = fopen("gb-bootroms/bin/dmg.bin", "r");

    int n_read = fread(code, 1, 258, fp);
    printf("n read: %d, first three bytes: %x %x %x\n",
        n_read, code[0], code[1], code[2]);

    fclose(fp);
    return 0;
}
