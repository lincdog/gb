#include "base.h"
#include "mem.h"
#include <stdlib.h>

#define _read_sb _read_unimplemented
#define _write_sb _write_unimplemented
#define _read_sc _read_unimplemented
#define _write_sc _write_unimplemented

#define _read_nr10 _read_unimplemented
#define _write_nr10 _write_unimplemented
#define _read_nr11 _read_unimplemented
#define _write_nr11 _write_unimplemented
#define _read_nr12 _read_unimplemented
#define _write_nr12 _write_unimplemented
#define _read_nr13 _read_unimplemented
#define _write_nr13 _write_unimplemented
#define _read_nr14 _read_unimplemented
#define _write_nr14 _write_unimplemented
#define _read_nr16 _read_unimplemented
#define _write_nr16 _write_unimplemented
#define _read_nr22 _read_unimplemented
#define _write_nr22 _write_unimplemented
#define _read_nr23 _read_unimplemented
#define _write_nr23 _write_unimplemented
#define _read_nr24 _read_unimplemented
#define _write_nr24 _write_unimplemented
#define _read_nr30 _read_unimplemented
#define _write_nr30 _write_unimplemented
#define _read_nr31 _read_unimplemented
#define _write_nr31 _write_unimplemented
#define _read_nr32 _read_unimplemented
#define _write_nr32 _write_unimplemented
#define _read_nr33 _read_unimplemented
#define _write_nr33 _write_unimplemented
#define _read_nr34 _read_unimplemented
#define _write_nr34 _write_unimplemented
#define _read_nr41 _read_unimplemented
#define _write_nr41 _write_unimplemented
#define _read_nr42 _read_unimplemented
#define _write_nr42 _write_unimplemented
#define _read_nr43 _read_unimplemented
#define _write_nr43 _write_unimplemented
#define _read_nr44 _read_unimplemented
#define _write_nr44 _write_unimplemented
#define _read_nr50 _read_unimplemented
#define _write_nr50 _write_unimplemented
#define _read_nr51 _read_unimplemented
#define _write_nr51 _write_unimplemented
#define _read_nr52 _read_unimplemented
#define _write_nr52 _write_unimplemented

#define _read_wav_regs _read_unimplemented
#define _write_wav_regs _write_unimplemented

/* GBC registers */
#define _read_ocps _read_unimplemented
#define _write_ocps _write_unimplemented
#define _read_ocpd _read_unimplemented
#define _write_ocpd _write_unimplemented
#define _read_bcps _read_unimplemented
#define _write_bcps _write_unimplemented
#define _read_bcpd _read_unimplemented
#define _write_bcpd _write_unimplemented
#define _read_hdma _read_unimplemented
#define _write_hdma _write_unimplemented
#define _read_vbk _read_unimplemented
#define _write_vbk _write_unimplemented
#define _read_key1 _read_unimplemented
#define _write_key1 _write_unimplemented
#define _read_rp _read_unimplemented
#define _write_rp _write_unimplemented
#define _read_svbk _read_unimplemented
#define _write_svbk _write_unimplemented
#define _read_pcm12 _read_unimplemented
#define _write_pcm12 _write_unimplemented
#define _read_pcm34 _read_unimplemented
#define _write_pcm34 _write_unimplemented
/* End GBC registers */

static const IOReg_t ioreg_table[] = {
    {
        .name="p1\0\0\0\0",
        .addr=0xFF00,
        .read=&_read_p1,
        .write=&_write_p1
    },
    {
        .name="sb\0\0\0\0",
        .addr=0xFF01,
        .read=&_read_sb,
        .write=&_write_sb
    },
    {
        .name="sb\0\0\0\0",
        .addr=0xFF02,
        .read=&_read_sc,
        .write=&_write_sc
    },
    unused_ioreg(0xFF03),
    {
        .name="div\0\0\0",
        .addr=0xFF04,
        .read=&_read_div,
        .write=&_write_div
    },
    {
        .name="tima\0\0",
        .addr=0xFF05,
        .read=&_read_tima,
        .write=&_write_tima
    },
    {
        .name="tma\0\0\0",
        .addr=0xFF06,
        .read=&_read_tma,
        .write=&_write_tma
    },
    {
        .name="tac\0\0\0",
        .addr=0xFF07,
        .read=&_read_tac,
        .write=&_write_tac
    },
    unused_ioreg(0xFF08),
    unused_ioreg(0xFF09),
    unused_ioreg(0xFF0A),
    unused_ioreg(0xFF0B),
    unused_ioreg(0xFF0C),
    unused_ioreg(0xFF0D),
    unused_ioreg(0xFF0E),
    {
        .name="if\0\0\0\0",
        .addr=0xFF0F,
        .read=&_read_if,
        .write=&_write_if
    },
    {
        .name="nr10\0\0",
        .addr=0xFF10,
        .read=&_read_nr10,
        .write=&_write_nr10
    },
    {
        .name="nr11\0\0",
        .addr=0xFF11,
        .read=&_read_nr11,
        .write=&_write_nr11
    },
    {
        .name="nr12\0\0",
        .addr=0xFF12,
        .read=&_read_nr12,
        .write=&_write_nr12
    },
    {
        .name="nr13\0\0",
        .addr=0xFF13,
        .read=&_read_nr13,
        .write=&_write_nr13
    },
    {
        .name="nr14\0\0",
        .addr=0xFF14,
        .read=&_read_nr14,
        .write=&_write_nr14
    },
    unused_ioreg(0xFF15),
    {
        .name="nr21\0\0",
        .addr=0xFF16,
        .read=&_read_nr16,
        .write=&_write_nr16
    },
    {
        .name="nr22\0\0",
        .addr=0xFF17,
        .read=&_read_nr22,
        .write=&_write_nr22
    },
    {
        .name="nr23\0\0",
        .addr=0xFF18,
        .read=&_read_nr23,
        .write=&_write_nr23
    },
    {
        .name="nr24\0\0",
        .addr=0xFF19,
        .read=&_read_nr24,
        .write=&_write_nr24
    },
    {
        .name="nr30\0\0",
        .addr=0xFF1A,
        .read=&_read_nr30,
        .write=&_write_nr30
    },
    {
        .name="nr31\0\0",
        .addr=0xFF1B,
        .read=&_read_nr31,
        .write=&_write_nr31
    },
    {
        .name="nr32\0\0",
        .addr=0xFF1C,
        .read=&_read_nr32,
        .write=&_write_nr32
    },
    {
        .name="nr33\0\0",
        .addr=0xFF1D,
        .read=&_read_nr33,
        .write=&_write_nr33
    },
    {
        .name="nr34\0\0",
        .addr=0xFF1E,
        .read=&_read_nr34,
        .write=&_write_nr34
    },
    unused_ioreg(0xFF1F),
    {
        .name="nr41\0\0",
        .addr=0xFF20,
        .read=&_read_nr41,
        .write=&_write_nr41
    },
    {
        .name="nr42\0\0",
        .addr=0xFF21,
        .read=&_read_nr42,
        .write=&_write_nr42
    },
    {
        .name="nr43\0\0",
        .addr=0xFF22,
        .read=&_read_nr43,
        .write=&_write_nr43
    },
    {
        .name="nr44\0\0",
        .addr=0xFF23,
        .read=&_read_nr44,
        .write=&_write_nr44
    },
    {
        .name="nr50\0\0",
        .addr=0xFF24,
        .read=&_read_nr50,
        .write=&_write_nr50
    },
    {
        .name="nr51\0\0",
        .addr=0xFF25,
        .read=&_read_nr51,
        .write=&_write_nr51
    },
    {
        .name="nr52\0\0",
        .addr=0xFF26,
        .read=&_read_nr52,
        .write=&_write_nr52
    },
    unused_ioreg(0xFF27),
    unused_ioreg(0xFF28),
    unused_ioreg(0xFF29),
    unused_ioreg(0xFF2A),
    unused_ioreg(0xFF2B),
    unused_ioreg(0xFF2C),
    unused_ioreg(0xFF2D),
    unused_ioreg(0xFF2E),
    unused_ioreg(0xFF2F),
    {
        .name="wav00\0",
        .addr=0xFF30,
        .read=&_read_wav_regs,
        .write=&_write_wav_regs
    },
    {
        .name="wav01\0",
        .addr=0xFF31,
        .read=&_read_wav_regs,
        .write=&_write_wav_regs
    },
    {
        .name="wav02\0",
        .addr=0xFF32,
        .read=&_read_wav_regs,
        .write=&_write_wav_regs
    },
    {
        .name="wav03\0",
        .addr=0xFF33,
        .read=&_read_wav_regs,
        .write=&_write_wav_regs
    },
    {
        .name="wav04\0",
        .addr=0xFF34,
        .read=&_read_wav_regs,
        .write=&_write_wav_regs
    },
    {
        .name="wav05\0",
        .addr=0xFF35,
        .read=&_read_wav_regs,
        .write=&_write_wav_regs
    },
    {
        .name="wav06\0",
        .addr=0xFF36,
        .read=&_read_wav_regs,
        .write=&_write_wav_regs
    },
    {
        .name="wav07\0",
        .addr=0xFF37,
        .read=&_read_wav_regs,
        .write=&_write_wav_regs
    },
    {
        .name="wav08\0",
        .addr=0xFF38,
        .read=&_read_wav_regs,
        .write=&_write_wav_regs
    },
    {
        .name="wav09\0",
        .addr=0xFF39,
        .read=&_read_wav_regs,
        .write=&_write_wav_regs
    },
    {
        .name="wav10\0",
        .addr=0xFF3A,
        .read=&_read_wav_regs,
        .write=&_write_wav_regs
    },
    {
        .name="wav11\0",
        .addr=0xFF3B,
        .read=&_read_wav_regs,
        .write=&_write_wav_regs
    },
    {
        .name="wav12\0",
        .addr=0xFF3C,
        .read=&_read_wav_regs,
        .write=&_write_wav_regs
    },
    {
        .name="wav13\0",
        .addr=0xFF3D,
        .read=&_read_wav_regs,
        .write=&_write_wav_regs
    },
    {
        .name="wav14\0",
        .addr=0xFF3E,
        .read=&_read_wav_regs,
        .write=&_write_wav_regs
    },
    {
        .name="wav15\0",
        .addr=0xFF3F,
        .read=&_read_wav_regs,
        .write=&_write_wav_regs
    },
    {
        .name="lcdc\0\0",
        .addr=0xFF40,
        .read=&_read_lcdc,
        .write=&_write_lcdc
    },
    {
        .name="stat\0\0",
        .addr=0xFF41,
        .read=&_read_stat,
        .write=&_write_stat
    },
    {
        .name="scy\0\0\0",
        .addr=0xFF42,
        .read=&_read_scy,
        .write=&_write_scy
    },
    {
        .name="scx\0",
        .addr=0xFF43,
        .read=&_read_scx,
        .write=&_write_scx
    },
    {
        .name="ly\0\0\0\0",
        .addr=0xFF44,
        .read=&_read_ly,
        .write=&_write_unimplemented
    },
    {
        .name="lyc\0\0\0",
        .addr=0xFF45,
        .read=&_read_lyc,
        .write=&_write_lyc
    },
    {
        .name="dma\0\0\0",
        .addr=0xFF46,
        .read=&_read_dma,
        .write=&_write_dma
    },
    {
        .name="bgp\0\0\0",
        .addr=0xFF47,
        .read=&_read_bgp,
        .write=&_write_bgp
    },
    {
        .name="obp0\0\0",
        .addr=0xFF48,
        .read=&_read_obp0,
        .write=&_write_obp0
    },
    {
        .name="obp1\0",
        .addr=0xFF49,
        .read=&_read_obp1,
        .write=&_write_obp1
    },
    {
        .name="wy\0\0\0\0",
        .addr=0xFF4A,
        .read=&_read_wy,
        .write=&_write_wy
    },
    {
        .name="wx\0\0\0\0",
        .addr=0xFF4B,
        .read=&_read_wx,
        .write=&_write_wx
    },
    unused_ioreg(0xFF4C),
    {
        .name="key1\0\0",
        .addr=0xFF4D,
        .read=&_read_key1,
        .write=&_write_key1
    },
    unused_ioreg(0xFF4E),
    {
        .name="vbk\0\0\0",
        .addr=0xFF4F,
        .read=&_read_vbk,
        .write=&_write_vbk
    },
    {
        .name="boot\0\0",
        .addr=0xFF50,
        .read=&_read_boot,
        .write=&_write_boot
    },
    {
        .name="hdma1\0",
        .addr=0xFF51,
        .read=&_read_hdma,
        .write=&_write_hdma
    },
    {
        .name="hdma2\0",
        .addr=0xFF52,
        .read=&_read_hdma,
        .write=&_write_hdma
    },
    {
        .name="hdma3\0",
        .addr=0xFF53,
        .read=&_read_hdma,
        .write=&_write_hdma
    },
    {
        .name="hdma4\0",
        .addr=0xFF54,
        .read=&_read_hdma,
        .write=&_write_hdma
    },
    {
        .name="hdma5\0",
        .addr=0xFF55,
        .read=&_read_hdma,
        .write=&_write_hdma
    },
    {
        .name="rp\0\0\0\0",
        .addr=0xFF56,
        .read=&_read_rp,
        .write=&_write_rp
    },
    unused_ioreg(0xFF57),
    unused_ioreg(0xFF58),
    unused_ioreg(0xFF59),
    unused_ioreg(0xFF5A),
    unused_ioreg(0xFF5B),
    unused_ioreg(0xFF5C),
    unused_ioreg(0xFF5D),
    unused_ioreg(0xFF5E),
    unused_ioreg(0xFF5F),
    unused_ioreg(0xFF60),
    unused_ioreg(0xFF61),
    unused_ioreg(0xFF62),
    unused_ioreg(0xFF63),
    unused_ioreg(0xFF64),
    unused_ioreg(0xFF65),
    unused_ioreg(0xFF66),
    unused_ioreg(0xFF67),
    {
        .name="bcps\0\0",
        .addr=0xFF68,
        .read=&_read_bcps,
        .write=&_write_bcps
    },
    {
        .name="bcpd\0\0",
        .addr=0xFF69,
        .read=&_read_bcpd,
        .write=&_write_bcpd
    },
    {
        .name="ocps\0\0",
        .addr=0xFF6A,
        .read=&_read_ocps,
        .write=&_write_ocps
    },
    {
        .name="ocpd\0\0",
        .addr=0xFF6B,
        .read=&_read_ocpd,
        .write=&_write_ocpd
    },
    unused_ioreg(0xFF6C),
    unused_ioreg(0xFF6D),
    unused_ioreg(0xFF6E),
    unused_ioreg(0xFF6F),
    {
        .name="svbk\0\0",
        .addr=0xFF70,
        .read=&_read_svbk,
        .write=&_write_svbk
    },
    unused_ioreg(0xFF71),
    unused_ioreg(0xFF72),
    unused_ioreg(0xFF73),
    unused_ioreg(0xFF74),
    unused_ioreg(0xFF75),
    {
        .name="pcm12\0",
        .addr=0xFF76,
        .read=&_read_pcm12,
        .write=&_write_pcm12
    },
    {
        .name="pcm34\0",
        .addr=0xFF77,
        .read=&_read_pcm34,
        .write=&_write_pcm34
    },
    unused_ioreg(0xFF78),
    unused_ioreg(0xFF79),
    unused_ioreg(0xFF7A),
    unused_ioreg(0xFF7B),
    unused_ioreg(0xFF7C),
    unused_ioreg(0xFF7D),
    unused_ioreg(0xFF7E),
    unused_ioreg(0xFF7F),
    // ... space ...
    {
        .name="ie\0\0\0\0",
        .addr=0xFFFF,
        .read=&_read_ie,
        .write=&_write_ie
    }
};

/*
MemoryRegion mbc1_mem_map[] = {
    { // 0001 1111 1111 1111
        .base=0x0000,
        .end=0x1FFF,
        .len=0x2000,
        .read=&_mbc1_read_rom_base,
        .write=&_mbc1_ram_enable
    },
    { // 0011 1111 1111 1111
        .base=0x2000,
        .end=0x3FFF,
        .len=0x2000,
        .read=&_mbc1_read_rom_base,
        .write=&_mbc1_rom_bank_num
    },
    { // 0101 1111 1111 1111
        .base=0x4000,
        .end=0x5FFF,
        .len=0x2000,
        .read=&_mbc1_read_rom_1,
        .write=&_mbc1_ram_or_upperrom
    },
    { // 0111 1111 1111 1111
        .base=0x6000,
        .end=0x7FFF,
        .len=0x2000,
        .read=&_mbc1_read_rom_1,
        .write=&_mbc1_bank_mode_select
    },
    { // 1011 1111 1111 1111
        .base=0xA000,
        .end=0xBFFF,
        .len=0x2000,
        .read=&_mbc1_read_ram_bank,
        .write=&_mbc1_write_ram_bank
    }
};

MemoryRegion mbc3_mem_map[] = {
    {
        .base=0x0000,
        .end=0x1FFF,
        .len=0x2000,
        .read=&_mbc3_read_rom_base,
        .write=&_mbc3_ram_timer_enable
    },
    {
        .base=0x2000,
        .end=0x3FFF,
        .len=0x2000,
        .read=&_mbc3_read_rom_base,
        .write=&_mbc3_rom_bank_num
    },
    {
        .base=0x4000,
        .end=0x5FFF,
        .len=0x2000,
        .read=&_mbc3_read_rom_1,
        .write=&_mbc3_ram_or_rtc_sel
    },
    {
        .base=0x6000,
        .end=0x7FFF,
        .len=0x2000,
        .read=&_mbc3_read_rom_1,
        .write=&_mbc3_latch_clock
    },
    {
        .base=0xA000,
        .end=0xBFFF,
        .len=0x2000,
        .read=&_mbc3_read_ram_or_rtc,
        .write=&_mbc3_write_ram_or_rtc
    }
};

MemoryRegion system_mem_map[] = {
    { // 0000 0000 1111 1111
        .base=0x0000,
        .end=0xFF,
        .len=0x100,
        .read=&_sys_read_boot_rom,
        .write=&_write_unimplemented
    },
    { // 1001 1111 1111 1111
        .base=0x8000,
        .end=0x9FFF,
        .len=0x2000,
        .read=&_sys_read_vram,
        .write=&_sys_write_vram
    },
    { // 1101 1111 1111 1111
        .base=0xC000,
        .end=0xDFFF,
        .len=0x2000,
        .read=&_sys_read_wram,
        .write=&_sys_write_wram
    },
    { // 1111 1101 1111 1111
        .base=0xE000,
        .end=0xFDFF,
        .len=0x1E00,
        .read=&_read_unimplemented,
        .write=&_write_unimplemented
    },
    { // 1111 1110 1001 1111
        .base=0xFE00,
        .end=0xFE9F,
        .len=0xA0,
        .read=&_sys_read_oam_table,
        .write=&_sys_write_oam_table
    },
    { // 1111 1110 1111 1111
        .base=0xFEA0,
        .end=0xFEFF,
        .len=0x60,
        .read=&_read_unimplemented,
        .write=&_write_unimplemented
    },
    { // 1111 1111 0111 1111
        .base=0xFF00,
        .end=0xFF7F,
        .len=0x80,
        .read=&_sys_read_ioreg,
        .write=&_sys_write_ioreg,
    },
    { // 1111 1111 1111 1110
        .base=0xFF80,
        .end=0xFFFE,
        .len=0x7F,
        .read=&_sys_read_hiram,
        .write=&_sys_write_hiram
    },
    { // 1111 1111 1111 1111
        .base=0xFFFF,
        .end=0xFFFF,
        .len=0x1,
        .read=&_sys_read_ioreg,
        .write=&_sys_write_ioreg
    }
};
*/
MemoryRegion basic_mem_map[] = {
    {
        .base=0x0000,
        .end=0x7FFF,
        .len=0x8000,
        .read=&_basic_read_rom,
        .write=&_basic_write_rom
    }
};

READ_FUNC(_sys_read_bootrom) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;

    return sys_mem->bootrom[rel_addr];
}

READ_FUNC(_sys_read_vram) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;

    return sys_mem->vram[rel_addr];
}

WRITE_FUNC(_sys_write_vram) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;

    sys_mem->vram[rel_addr] = data;

    return 1;
}

READ_FUNC(_sys_read_wram) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;

    return sys_mem->wram[rel_addr];
}

WRITE_FUNC(_sys_write_wram) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;

    sys_mem->wram[rel_addr] = data;

    return 1;
}

READ_FUNC(_sys_read_oam_table) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;

    return sys_mem->oam_table[rel_addr];
}

WRITE_FUNC(_sys_write_oam_table) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;

    sys_mem->oam_table[rel_addr] = data;

    return 1;
}

READ_FUNC(_sys_read_hiram) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;

    return sys_mem->hram[rel_addr];
}

WRITE_FUNC(_sys_write_hiram) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;

    sys_mem->hram[rel_addr] = data;

    return 1;
}

READ_FUNC(_basic_read_rom) {
    BasicCartState *cart_mem = (BasicCartState *)state->mem->cartridge->state;

    return cart_mem->rom[rel_addr];
}

WRITE_FUNC(_basic_write_rom) {
    BasicCartState *cart_mem = (BasicCartState *)state->mem->cartridge->state;

    cart_mem->rom[rel_addr] = data;

    return 1;
}

READ_FUNC(_sys_read_ioreg) {
    
    if (rel_addr < 0x80) 
        return ioreg_table[rel_addr].read(__READ_ARGS);
    else if (rel_addr == 0xFF)
        return ioreg_table[0x80].read(__READ_ARGS);
    else
        return UNINIT;
}

WRITE_FUNC(_sys_write_ioreg) {
    
    if (rel_addr < 0x80) 
        return ioreg_table[rel_addr].write(__WRITE_ARGS);
    else if (rel_addr == 0xFF)
        return ioreg_table[0x80].write(__WRITE_ARGS);
    else
        return UNINIT; 
}

/* BEGIN ioreg read/write functions */
READ_FUNC(_read_p1) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    BYTE cur_value = sys_mem->ioregs->p1;
    if (cur_value ^ 0xDF) {
        // bit 5 is 0, action buttons selected
        // FIXME add state representing keyboard events that
        // map to buttons
        return 0xD2;
    } else if (cur_value ^ 0xEF) {
        // bit 4 is 0, direction buttons selected
        // FIXME add state that map to buttons
        return 0xE4;
    } else {
        return 0xFF;
    }
}
WRITE_FUNC(_write_p1) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    // Only writable bits are 4 and 5
    data &= 0x30;
    sys_mem->ioregs->p1 &= (0xC0 | data | 0xF);
    return 1;
}
READ_FUNC(_read_div) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    return sys_mem->ioregs->div;
}
WRITE_FUNC(_write_div) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    sys_mem->ioregs->div = 0;
    return 1;    
}
READ_FUNC(_read_tima) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    return sys_mem->ioregs->tima;
}
WRITE_FUNC(_write_tima) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    sys_mem->ioregs->tima = data;
}
READ_FUNC(_read_tma) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    return sys_mem->ioregs->tma;
}
WRITE_FUNC(_write_tma) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    sys_mem->ioregs->tma = data;
}
READ_FUNC(_read_tac) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    // FIXME bit 2 is timer enable, bit 0-1 is input clock speed
    return sys_mem->ioregs->tac;
}
WRITE_FUNC(_write_tac) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    data &= 0x07;
    // FIXME bit 2 enables timer, bit 0-1 selects timer frequency
    sys_mem->ioregs->tac = data;
}
READ_FUNC(_read_if) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    //FIXME: bits are set when corresponding interrupts are triggered,
    // and cleared upon successful interrupt servicing
    return sys_mem->ioregs->if_;
}
WRITE_FUNC(_write_if) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    sys_mem->ioregs->if_ = data;
    return 1;    
}
READ_FUNC(_read_ie) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    return sys_mem->ioregs->ie_;
}
WRITE_FUNC(_write_ie) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    sys_mem->ioregs->ie_ = data;
    return 1;    
}

READ_FUNC(_read_dma) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    //FIXME: bits are set when corresponding interrupts are triggered,
    // and cleared upon successful interrupt servicing
    return sys_mem->ioregs->dma;
}
WRITE_FUNC(_write_dma) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    // FIXME: queues DMA for source_start-source_end to FE00-FE9F
    // The CPU can apparently only read in HRAM (FF80-FFFE) during this time
    WORD source_start = ((WORD)data) << 8;
    WORD source_end = source_start | 0x9F;
    sys_mem->ioregs->dma = data;
    return 1;    
}
READ_FUNC(_read_lcdc) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    return sys_mem->ioregs->lcdc;
}
WRITE_FUNC(_write_lcdc) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    // FIXME: all bits of lcdc control aspects of the PPU
    sys_mem->ioregs->lcdc = data;
    return 1;    
}
READ_FUNC(_read_stat) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    // FIXME: returns status of PPU-based interrupts, and current
    // state of PPU (bit 0-1)
    return sys_mem->ioregs->stat;
}
WRITE_FUNC(_write_stat) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    // Only bits 3-6 are writable
    data &= 0x74;
    sys_mem->ioregs->stat &= (data | 0x7);
    return 1;    
}
READ_FUNC(_read_scy) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    // FIXME: top left coords of visible area within 256x256 bg map
    return sys_mem->ioregs->scy;
}
WRITE_FUNC(_write_scy) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    sys_mem->ioregs->scy = data;
    return 1;
}
READ_FUNC(_read_scx) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    return sys_mem->ioregs->scx;
}
WRITE_FUNC(_write_scx) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    sys_mem->ioregs->scx = data;
    return 1;
}
READ_FUNC(_read_ly) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    // FIXME: current horizontal line (0-153, 144-154==VBlank)
    return sys_mem->ioregs->ly;
}
READ_FUNC(_read_lyc) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    // FIXME: triggers LYC=LY interrupt if it is set in STAT,
    // when this equals LY
    return sys_mem->ioregs->lyc;
}
WRITE_FUNC(_write_lyc) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    sys_mem->ioregs->lyc = data;
    return 1;
}
READ_FUNC(_read_wy) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    return sys_mem->ioregs->wy;
}
WRITE_FUNC(_write_wy) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    // FIXME: Sets window position
    sys_mem->ioregs->wy = data;
    return 1;
}
READ_FUNC(_read_wx) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    return sys_mem->ioregs->wx;
}
WRITE_FUNC(_write_wx) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    // FIXME: Sets window position
    sys_mem->ioregs->wx = data;
    return 1;
}
READ_FUNC(_read_bgp) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    // FIXME: returns color palette to BG/window
    return sys_mem->ioregs->bgp;
}
WRITE_FUNC(_write_bgp) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    // FIXME: Sets color palette for bg/window
    sys_mem->ioregs->bgp = data;
    return 1;
}
READ_FUNC(_read_obp0) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    // FIXME: returns color palette to obj palette 0
    return sys_mem->ioregs->obp0;
}
WRITE_FUNC(_write_obp0) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    // FIXME: sets color palette for objs 0
    data &= 0xFC;
    sys_mem->ioregs->obp0 = data;
    return 1;
}
READ_FUNC(_read_obp1) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    // FIXME: returns color palette to obj palette 1
    return sys_mem->ioregs->obp1;
}
WRITE_FUNC(_write_obp1) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    // FIXME: sets color palette for objs 0
    data &= 0xFC;
    sys_mem->ioregs->obp1 = data;
    return 1;
} 
READ_FUNC(_read_boot) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    if (sys_mem->bootrom_mapped)
        return 0xFE;
    else
        return 0xFF;
}
WRITE_FUNC(_write_boot) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    sys_mem->bootrom_mapped = 0;
    sys_mem->ioregs->boot = data | 1;

    return 1;
}

READ_FUNC(_read_unimplemented) {
    return UNINIT;
}
WRITE_FUNC(_write_unimplemented) {
    return -1;
}

/* END ioreg functions */

BYTE read_mem(GBState *state, WORD addr, BYTE flags) {
    BYTE result = UNINIT;
    Memmap_t *sys_map = state->mem->system;
    Memmap_t *cart_map = state->mem->cartridge;
    MemoryRegion *sys_regions = sys_map->regions;
    MemoryRegion *cart_regions = cart_map->regions;
    WORD rel_addr;

    MemoryRegion source;

    for (int i = 0; i < sys_map->n_regions; i++) {
        if (sys_regions[i].flags & MEM_UNMAPPED)
            continue;
        
        if (addr >= sys_regions[i].base && addr <= sys_regions[i].end) {
            source = sys_regions[i];
            rel_addr = addr - source.base;

            goto read_mem_do_read;
        }
    }

    for (int i = 0; i < cart_map->n_regions; i++) {
        if (cart_regions[i].flags & MEM_UNMAPPED)
            continue;

        if (addr >= cart_regions[i].base && addr <= cart_regions[i].end) {
            source = cart_regions[i];
            rel_addr = addr - source.base;

            goto read_mem_do_read;
        } 

    }
    
    return UNINIT;

    read_mem_do_read:
    return source.read(state, rel_addr, flags);
}

int write_mem(GBState *state, WORD addr, BYTE data, BYTE flags) {
    int status;
    BYTE result = UNINIT;
    Memmap_t *sys_map = state->mem->system;
    Memmap_t *cart_map = state->mem->cartridge;
    MemoryRegion *sys_regions = &sys_map->regions;
    MemoryRegion *cart_regions = &cart_map->regions;
    WORD rel_addr;

    MemoryRegion source;

    for (int i = 0; i < sys_map->n_regions; i++) {
        if (sys_regions[i].flags & MEM_UNMAPPED)
            continue;
        
        if (addr >= sys_regions[i].base && addr <= sys_regions[i].end) {
            source = sys_regions[i];
            rel_addr = addr - source.base;
            
            goto write_mem_do_write;
        }
    }

    for (int i = 0; i < cart_map->n_regions; i++) {
        if (cart_regions[i].flags & MEM_UNMAPPED)
            continue;

        if (addr >= cart_regions[i].base && addr <= cart_regions[i].end) {
            source = cart_regions[i];
            rel_addr = addr - source.base;
            
            goto write_mem_do_write;
        } 

    }

    write_mem_do_write:
    return source.write(state, rel_addr, data, flags);
    
}

IORegs *initialize_ioregs(void) {
    IORegs *ioregs = malloc(sizeof(IORegs));
    if (ioregs == NULL) {
        printf("Unable to allocate IORegs\n");
        return NULL;
    }
    ioregs->p1 = 0xCF;
    ioregs->sb = 0x00;
    ioregs->sc = 0x7E;
    ioregs->div = 0xAB;
    ioregs->tima = 0x00;
    ioregs->tma = 0x00;
    ioregs->tac = 0xF8;
    ioregs->if_ = 0xE1;
    ioregs->nr10 = 0x80;
    ioregs->nr11 = 0xBF;
    ioregs->nr12 = 0xF3;
    ioregs->nr13 = 0xFF;
    ioregs->nr14 = 0xBF;
    ioregs->nr21 = 0x3F;
    ioregs->nr22 = 0x00;
    ioregs->nr23 = 0xFF;
    ioregs->nr24 = 0xBF;
    ioregs->nr30 = 0x7F;
    ioregs->nr31 = 0xFF;
    ioregs->nr32 = 0x9F;
    ioregs->nr33 = 0xFF;
    ioregs->nr34 = 0xBF;
    ioregs->nr41 = 0xFF;
    ioregs->nr42 = 0x00;
    ioregs->nr43 = 0x00;
    ioregs->nr44 = 0xBF;
    ioregs->nr50 = 0x77;
    ioregs->nr51 = 0xF3;
    ioregs->nr52 = 0xF1;
    ioregs->lcdc = 0x91;
    ioregs->stat = 0x85;
    ioregs->scy = 0x00;
    ioregs->scx = 0x00;
    ioregs->ly = 0x00;
    ioregs->lyc = 0;
    ioregs->dma = 0xFF;
    ioregs->bgp = 0xFC;
    ioregs->obp0 = UNINIT;
    ioregs->obp1 = UNINIT;
    ioregs->wy = 0x00;
    ioregs->wx = 0x00;
    ioregs->key1 = 0xFF;
    ioregs->vbk = 0xFF;
    ioregs->hdma1 = 0xFF;
    ioregs->hdma2 = 0xFF;
    ioregs->hdma3 = 0xFF;
    ioregs->hdma4 = 0xFF;
    ioregs->hdma5 = 0xFF;
    ioregs->rp = 0xFF;
    ioregs->bcps = 0xFF;
    ioregs->bcpd = 0xFF;
    ioregs->ocps = 0xFF;
    ioregs->ocpd = 0xFF;
    ioregs->svbk = 0xFF;
    ioregs->ie_ = 0x00;

    return ioregs;
}

void teardown_ioregs(IORegs *ioregs) {
    free(ioregs);
}

BYTE *allocate_region(size_t size, char *name) {
    BYTE *result = malloc(size);
    if (result == NULL) {
        printf("Error allocating %s\n", name);
        exit(1);
    }
    memset(result, UNINIT, size);

    return result;
}

SysMemState *initialize_sys_memory(void) {
    SysMemState *sys_mem = malloc(sizeof(SysMemState));
    if (sys_mem == NULL) {
        printf("Error allocating system memory\n");
        exit(1);
    }
    sys_mem->bootrom_mapped = 1;
    sys_mem->bootrom = &DMG_boot_rom;
    sys_mem->vram = allocate_region(0x2000, "vram\0");
    sys_mem->wram = allocate_region(0x2000, "wram\0");
    sys_mem->oam_table = allocate_region(0xA0, "oam\0");
    sys_mem->ioregs = initialize_ioregs();
    sys_mem->hram = allocate_region(0x79, "hram\0");
    
}

void teardown_sys_memory(SysMemState *sys_mem) {
    free(sys_mem->vram);
    free(sys_mem->wram);
    free(sys_mem->oam_table);
    teardown_ioregs(sys_mem->ioregs);
    free(sys_mem->hram);
}

BasicCartState *initialize_basic_memory(void) {
    BasicCartState *cart_mem = malloc(sizeof(BasicCartState));
    if (cart_mem == NULL) {
        printf("Error allocating cartridge memory\n");
        exit(1);
    }

    memset(cart_mem->rom, UNINIT, sizeof(cart_mem->rom));

    return cart_mem;
}

void teardown_basic_memory(BasicCartState *cart_mem) {
    free(cart_mem);
}