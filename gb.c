#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define WORD uint16_t
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

#define get_flag_zero (reg_af & 0x40)
#define get_flag_n (reg_af & 0x20)
#define get_flag_h (reg_af & 0x10)
#define get_flag_c (reg_af & 0x08)

#define set_reg_a(x) (reg_af = ((x)<<8) | get_reg_f)
#define set_reg_b(x) (reg_bc = ((x)<<8) | get_reg_c)
#define set_reg_c(x) (reg_bc = (reg_bc & 0xFF00) | (x))
#define set_reg_d(x) (reg_de = ((x)<<8) | get_reg_e)
#define set_reg_e(x) (reg_de = (reg_de & 0xFF00) | (x))
#define set_reg_h(x) (reg_hl = ((x)<<8) | get_reg_l)
#define set_reg_l(x) (reg_hl = (reg_hl & 0xFF00) | (x))

typedef union {
    struct {
        unsigned char l;
        unsigned char h;
    } w;
    unsigned short dw;
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

uint8_t reg_a;
uint8_t reg_f;
reg reg_bc;
reg reg_de;
reg reg_hl;
uint16_t reg_sp;
uint16_t reg_pc;

enum {READY, IM_8, IM_16_LSB, IM_16_MSB, CB} instr_state = READY;

#define bit_7(x) ((x) & 0x80)
#define bit_6(x) ((x) & 0x40)
#define bit_5(x) ((x) & 0x20)
#define bit_4(x) ((x) & 0x10)
#define bit_3(x) ((x) & 0x08)
#define bit_2(x) ((x) & 0x04)
#define bit_1(x) ((x) & 0x02)
#define bit_0(x) ((x) & 0x01)

#define REGION_CHECK(x, lb, ub) (((uint16_t)(x)>=lb)&&((uint16_t)(x)<=ub)))

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



int main(void) {

    reg_bc.dw = 0x2345;
    reg_de.dw = 0x3456;
    reg_hl.dw = 0x4567;

    printf("bc.h: %lx bc.l: %lx\n", a(reg_bc.w.h), a(reg_bc.w.l));
    printf("de.h: %lx de.l: %lx\n", a(reg_de.w.h), a(reg_de.w.l));

    printf("bc.dw: %x, bc.h: %x, bc.l: %x\n", 
        reg_bc.dw, reg_bc.w.h, reg_bc.w.l);
   

    return 0;
}