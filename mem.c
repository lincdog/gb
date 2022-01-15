#include "base.h"
#include "mem.h"

BYTE read_8(WORD addr, BYTE *code) {
    return code[addr];
}
WORD read_16(WORD addr, BYTE *code) {
    return (WORD)(code[addr] | (code[addr+1] << 8));
}

int write_8(WORD addr, BYTE *code, BYTE data) {
    code[addr] = data;

    return 1;
}

int write_16(WORD addr, BYTE *code, WORD data) {
    code[addr] = data & 0xFF;
    code[addr+1] = data >> 8;

    return 2;
}