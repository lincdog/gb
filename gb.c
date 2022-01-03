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

#define get_reg_a (WORD)((reg_af & 0xFF00)>>8)
#define get_reg_f (WORD)((reg_af & 0x00FF))
#define get_reg_b (WORD)((reg_bc & 0xFF00)>>8)
#define get_reg_c (WORD)((reg_bc & 0x00FF))
#define get_reg_d (WORD)((reg_de & 0xFF00)>>8)
#define get_reg_e (WORD)((reg_de & 0x00FF))
#define get_reg_h (WORD)((reg_hl & 0xFF00)>>8)
#define get_reg_l (WORD)((reg_hl & 0x00FF))

#define set_reg_a(x) (reg_af = ((x)<<8) | get_reg_f)
#define set_reg_b(x) (reg_bc = ((x)<<8) | get_reg_c)
#define set_reg_c(x) (reg_bc = (reg_bc & 0xFF00) | (x))
#define set_reg_d(x) (reg_de = ((x)<<8) | get_reg_e)
#define set_reg_e(x) (reg_de = (reg_de & 0xFF00) | (x))
#define set_reg_h(x) (reg_hl = ((x)<<8) | get_reg_l)
#define set_reg_l(x) (reg_hl = (reg_hl & 0xFF00) | (x))

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
BYTE code[32767];

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


BYTE reg_a;
BYTE reg_f;

#define get_flag_z() (reg_f & 0b10000000)
#define set_flag_z() (reg_f |= 0b10000000)
#define clear_flag_z() (reg_f &= 0b01111111)
#define get_flag_n() (reg_f & 0b01000000)
#define set_flag_n() (reg_f |= 0x01000000)
#define clear_flag_n() (reg_f &= 0b10111111)
#define get_flag_h() (reg_f & 0b00100000)
#define set_flag_h() (reg_f |= 0b00100000)
#define clear_flag_h() (reg_f &= 0b11011111)
#define get_flag_c() (reg_f & 0b00010000)
#define set_flag_c() (reg_f |= 0b00010000)
#define clear_flag_c() (reg_f &= 0b11101111)

reg reg_bc;
reg reg_de;
reg reg_hl;

WORD reg_sp;
WORD reg_pc = 0;

enum STATE {READY, IM_8, IM_16_LSB, IM_16_MSB, CB};

typedef struct GBFlags {
    BYTE z;
    BYTE n;
    BYTE h;
    BYTE c;
} GBFlags;

struct {
    BYTE reg_a;
    reg reg_bc;
    reg reg_de;
    reg reg_hl;
    WORD reg_sp;
    WORD reg_pc;
    struct GBFlags flags;
    
    char *code;
} GBState;

BYTE read_8(WORD addr) {
    return code[addr];
}
WORD read_16(WORD addr) {
    return (WORD)code[addr];
}

int write_8(WORD addr, BYTE data) {
    code[addr] = data;

    return 1;
}

int write_16(WORD addr, WORD data) {
    code[addr] = data;

    return 1;
}

#define IMMED_8() read_8(reg_pc++)
#define IMMED_16() read_16(reg_pc += 2);

#define LOAD_REG_8_IMMED(r) r = IMMED_8(); break;
#define LOAD_REG_16_IMMED(r) r = IMMED_16(); break;
#define WRITE_8(dest, src) write_8(dest, src); break;
#define WRITE_16(dest, src) write_16(dest, src); break;
#define INC_16(r) r++; break;
#define INC_8(r) r++; clear_flag_n(); break;
#define DEC_16(r) r--; break;
#define DEC_8(r) r--; set_flag_n(); break;
#define ADD_16_REG(dest, src) dest += src; clear_flag_n(); break;
#define LOAD_REG_8_ADDR(dest, src) dest = read_8(src); break;
#define LOAD_REG_8_REG(dest, src) dest = src; break;
#define COND_REL_JUMP_IMMED(cond) int8_t offset = IMMED_8(); \
    if (cond) reg_pc += offset; break;
#define COND_ABS_JUMP_IMMED(cond) WORD addr = IMMED_16(); \
    if (cond) reg_pc = addr; break;
#define UNCOND_REL_JUMP_IMMED() reg_pc += IMMED_8(); break;
#define UNCOND_ABS_JUMP_IMMED() reg_pc = IMMED_16(); break;
#define ADD_8(dest, src) dest += src; clear_flag_n(); break;
#define SUB_8(dest, src) dest -= src; set_flag_n(); break;
#define AND_8(dest, src) dest &= src; clear_flag_n(); set_flag_h(); clear_flag_c(); break;



void execute_program(BYTE *code) {
    BYTE operation, next_operation;
    enum STATE instr_state = READY;
    char offset;

    // Main loop
    for (;;) {
        next_operation = read_8(reg_pc);
        reg_pc++;

        switch (next_operation) {
            case 0x00:
                break;
            case 0x01:
                // Load 16 immediate to BC
                break;
            case 0x02:
                // write_8(reg_bc, reg_a);
                break;
            case 0x03:
                // Increment BC
                reg_bc.dw += 1;
                break;
            case 0x04:
                // Increment B
                clear_flag_n();
                // Z 0 H -
                break;
            case 0x05:
                // Decrement B
                // Z 1 H -
                set_flag_n();
                break;
            case 0x06:
                // Load b with immediate 8bits
                reg_bc.w.h = read_8(reg_pc);
                reg_pc++;
                break;
            case 0x07:
                reg_a = (reg_a << 1) + ((reg_a & 0x80) == 1);
                clear_flag_z();
                clear_flag_n();
                clear_flag_h();
                break;
            case 0x08:
                // Load immediate 16 bit address with SP
                // write_16(reg_pc++ | reg_pc++, reg_sp);
                break;
            case 0x09:
                reg_hl.dw += reg_bc.dw;
                // Set - 0 H C
                clear_flag_n();
                break;
            case 0x0A:
                reg_a = read_8(reg_bc.dw);
                break;
            case 0x0B:
                reg_bc.dw -= 1;
                break;
            case 0x0C:
                reg_bc.w.l += 1;
                // Sets Z 0 H -
                clear_flag_n();
                break;
            case 0x0D:
                reg_bc.w.l -= 1;
                // Sets Z 1 H -
                set_flag_n();
                break;
            case 0x0E:
                reg_bc.w.l = read_8(reg_pc);
                reg_pc ++;
                break;
            case 0x0F:
                // Rotate A through carry
                // Sets 0 0 0 C
                clear_flag_z();
                clear_flag_n();
                clear_flag_h();
                break;
            case 0x10:
                // STOP
                // read_8(reg_pc++);
                return;
                break;
            case 0x11:
                reg_de.dw = read_16(reg_pc);
                reg_pc += 2;
                break;
            case 0x12:
                // write_8(reg_de.dw, reg_a);
                break;
            case 0x13:
                reg_de.dw += 1;
                break;
            case 0x14:
                reg_de.w.h += 1;
                // Sets Z 0 H 
                clear_flag_n();
                break;
            case 0x15:
                reg_de.w.h -= 1;
                // Sets Z 1 H
                set_flag_n();
                break;
            case 0x16:
                reg_de.w.h = read_8(reg_pc);
                reg_pc++;
                break;
            case 0x17:
                // Rotate left A through carry
                // 0 0 0 C
                clear_flag_z();
                clear_flag_n();
                clear_flag_h();
                break;
            case 0x18:
                reg_pc += (char)read_8(reg_pc);
                reg_pc++;
                break;
            case 0x19:
                reg_hl.dw += reg_de.dw;
                // Set - 0 H C
                clear_flag_n();
                break;
            case 0x1A:
                reg_a = read_8(reg_de.dw);
                break;
            case 0x1B:
                reg_de.w.h -= 1;
                break;
            case 0x1C:
                reg_de.w.l += 1;
                // Set Z 0 H -
                clear_flag_n();
                break;
            case 0x1D:
                reg_de.w.l -= 1;
                // Set Z 1 H - 
                set_flag_n();
                break;
            case 0x1E:
                reg_de.w.l = read_8(reg_pc);
                reg_pc++;
                break;
            case 0x1F:
                // rotate A right thru carry
                // Set 0 0 0 C
                clear_flag_z();
                clear_flag_n();
                clear_flag_h();
                break;

            case 0x20:
                // Relative jump if nonzero 
                offset = (char)read_8(reg_pc);
                reg_pc++;

                if (! get_flag_z()) {
                    reg_pc += offset;
                }

                break;
            case 0x21:
                reg_hl.dw = read_16(reg_pc);
                reg_pc += 2;

                break;
            case 0x22:
                //write_8(reg_hl.dw++, reg_a);
                break;
            case 0x23:
                reg_hl.dw += 1;
                break;
            case 0x24:
                reg_hl.w.h += 1;
                clear_flag_n();
                // Set Z 0 H -
                break;
            case 0x25:
                reg_hl.w.h -= 1;
                set_flag_n();
                // Set Z 1 H -
                break;
            case 0x26:
                reg_hl.w.h = read_8(reg_pc);
                reg_pc++;
                break;
            case 0x27:
                // Decimal adjust A
                //SET Z - 0 C
                clear_flag_h();
                break;
            case 0x28:
                offset = (char)read_8(reg_pc);
                reg_pc++;
                
                if (get_flag_z()) {
                    reg_pc += offset;
                }
                break;
            case 0x29:
                reg_hl.dw += reg_de.dw;
                // Set - 0 H C
                clear_flag_n();
                break;
            case 0x2A:
                reg_a = read_8(reg_hl.dw);
                reg_hl.dw++;
                break;
            case 0x2B:
                reg_hl.dw -= 1;
                break;
            case 0x2C:
                reg_hl.w.l += 1;
                // Set Z 0 H -
                clear_flag_n();
                break;
            case 0x2D:
                reg_hl.w.l -= 1;
                // Set Z 1 H -
                set_flag_n();
                break;
            case 0x2E:
                reg_hl.w.l = read_8(reg_pc);
                reg_pc++;
                break;
            case 0x2F:
                reg_a = ~reg_a;
                set_flag_n();
                set_flag_h();
                break;
            case 0x30:
                offset = (char)read_8(reg_pc);
                reg_pc++;
                if (! get_flag_c()) {
                    reg_pc += offset;
                }
                break;
            case 0x31:
                reg_sp = read_16(reg_pc);
                reg_pc += 2;
                break;
            case 0x32:
                // write(reg_hl.dw--, reg_a);
                break;
            case 0x33:
                reg_sp += 1;
                break;
            case 0x34:
                
                break;

        }
    }

}

int main(void) {
    FILE *fp;
    fp = fopen("gb-bootroms/bin/dmg.bin", "r");

    reg_bc.dw = 0x2345;
    reg_de.dw = 0x3456;
    reg_hl.dw = 0x4567;

    printf("bc.h: %lx bc.l: %lx\n", a(reg_bc.w.h), a(reg_bc.w.l));
    printf("de.h: %lx de.l: %lx\n", a(reg_de.w.h), a(reg_de.w.l));

    printf("bc.dw: %x, bc.h: %x, bc.l: %x\n", 
        reg_bc.dw, reg_bc.w.h, reg_bc.w.l);
   
    int n_read = fread(code, 1, 258, fp);
    printf("n read: %d, first three bytes: %x %x %x\n",
        n_read, code[0], code[1], code[2]);

    fclose(fp);
    return 0;
}
