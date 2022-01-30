#include "base.h"
#include "mem.h"
#include <stdlib.h>

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
        .write=&_write_ly
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
};

MemoryRegion mbc1_mem_map[] = {
    {
        .base=0x0000,
        .end=0x1FFF,
        .len=0x2000,
        .read=&_read_rom_base,
        .write=&_mbc1_ram_enable
    },
    {
        .base=0x2000,
        .end=0x3FFF,
        .len=0x2000,
        .read=&_read_rom_base,
        .write=&_mbc1_rom_bank_num
    },
    {
        .base=0x4000,
        .end=0x5FFF,
        .len=0x2000,
        .read=&_read_rom_1,
        .write=&_mbc1_ram_or_upperrom
    },
    {
        .base=0x6000,
        .end=0x7FFF,
        .len=0x2000,
        .read=&_read_rom_1,
        .write=&_mbc_bank_mode_select
    },
    {
        .base=0xA000,
        .end=0xBFFF,
        .len=0x2000,
        .read=&_read_ram_bank,
        .write=&_write_ram_bank
    }
};

MemoryRegion system_mem_map[] = {
    {
        .base=0x0000,
        .end=0xFF,
        .len=0x100,
        .read=&_read_boot_rom,
        .write=&_write_unimplemented
    },
    {
        .base=0x8000,
        .end=0x9FFF,
        .len=0x2000,
        .read=&_read_vram,
        .write=&_write_vram
    },
    {
        .base=0xC000,
        .end=0xDFFF,
        .len=0x2000,
        .read=&_read_wram,
        .write=&_write_wram
    },
    {
        .base=0xE000,
        .end=0xFDFF,
        .len=0x1E00,
        .read=&_read_unimplemented,
        .write=&_write_unimplemented
    },
    {
        .base=0xFE00,
        .end=0xFE9F,
        .len=0xA0,
        .read=&_read_oam_table,
        .write=&_write_oam_table
    },
    {
        .base=0xFEA0,
        .end=0xFEFF,
        .len=0x60,
        .read=&_read_unimplemented,
        .write=&_write_unimplemented
    },
    {
        .base=0xFF00,
        .end=0xFF7F,
        .len=0x80,
        .read=&_read_ioreg,
        .write=&_write_ioreg,
    },
    {
        .base=0xFF80,
        .end=0xFFFE,
        .len=0x7F
        .read=&_read_hiram,
        .write=&_write_hiram
    },
    {
        .base=0xFFFF,
        .end=0xFFFF,
        .len=0x1,
        .read=&_read_ie,
        .write=&_write_ie
    }
};

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

READ_FUNC(_read_p1) {

}

WRITE_FUNC(_write_p1) {

}

READ_FUNC(_read_unimplemented) {
    return UNINIT;
}

WRITE_FUNC(_write_unimplemented) {
    return -1;
}

BYTE read_mem(GBState *state, WORD addr, BYTE flags) {
    BYTE result = UNINIT;
    MemoryState *mem = state->mem;
    

    if ((addr & 0xFF00)==0xFF00) {
        if (is_stack(addr))
            result =state->code[addr];
        else if (addr == 0xFFFF)
            result = _read_ie(state, addr, flags);
        else
            result = (*ioreg_table[addr & 0xFF].read)(state, addr, flags);
    } else {
        // TODO: Check PPU status for locks
        result = state->code[addr];
    }
    return result;
}

int write_mem(GBState *state, WORD addr, BYTE data, BYTE flags) {
    int status;

    if (is_rom(addr)) {
        status = 0;
    } else if (is_rom_selector(addr)) {
        // do rom select
        status = 2;
    } else {
        if ((addr & 0xFF00)==0xFF00) {
            if (is_stack(addr)) {
                state->code[addr] = data;
                status = 1;
            } else if (addr == 0xFFFF) {
                status = _write_ie(state, addr, data, flags);
            } else {
                status = (*ioreg_table[addr & 0xFF].write)(state, addr, data, flags);
            }
        } else {
            // TODO: Check PPU etc for locks, other roms
            state->code[addr] = data;
            status = 1;
        }
    }

    return status;
}