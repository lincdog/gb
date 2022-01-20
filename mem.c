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

BYTE read_mem(GBState *state, WORD addr) {
    if ((addr & 0xFF00)==0xFF00) {
        return ((BYTE *)state->io_regs)[addr & 0xFF];
    } else {
        // TODO: Check PPU status for locks
        return state->code[addr];
    }
    return 0;
}

int write_mem(GBState *state, WORD addr, BYTE data) {
    int status;

    if (is_rom(addr)) {
        status = 0;
    } else if (is_rom_selector(addr)) {
        // do rom select
        status = 2;
    } else {
        // TODO: Check PPU etc for locks, other roms
        state->code[addr] = data;
        status = 1;
    }

    return status;
}