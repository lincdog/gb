#include "base.h"
#include "mem.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

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

/* DMG BootRom */
const BYTE DMG_boot_rom[] = {
    0x31, 0xFE, 0xFF, 0xAF, 0x21, 0xFF, 0x9F, 0x32,
    0xCB, 0x7C, 0x20, 0xFB, 0x21, 0x26, 0xFF, 0x0E,
    0x11, 0x3E, 0x80, 0x32, 0xE2, 0x0C, 0x3E, 0xF3,
    0xE2, 0x32, 0x3E, 0x77, 0x77, 0x3E, 0xFC, 0xE0,
    0x47, 0x11, 0x04, 0x01, 0x21, 0x10, 0x80, 0x1A,
    0xCD, 0x95, 0x00, 0xCD, 0x96, 0x00, 0x13, 0x7B,
    0xFE, 0x34, 0x20, 0xF3, 0x11, 0xD8, 0x00, 0x06,
    0x08, 0x1A, 0x13, 0x22, 0x23, 0x05, 0x20, 0xF9,
    0x3E, 0x19, 0xEA, 0x10, 0x99, 0x21, 0x2F, 0x99,
    0x0E, 0x0C, 0x3D, 0x28, 0x08, 0x32, 0x0D, 0x20,
    0xF9, 0x2E, 0x0F, 0x18, 0xF3, 0x67, 0x3E, 0x64,
    0x57, 0xE0, 0x42, 0x3E, 0x91, 0xE0, 0x40, 0x04,
    0x1E, 0x02, 0x0E, 0x0C, 0xF0, 0x44, 0xFE, 0x90,
    0x20, 0xFA, 0x0D, 0x20, 0xF7, 0x1D, 0x20, 0xF2,
    0x0E, 0x13, 0x24, 0x7C, 0x1E, 0x83, 0xFE, 0x62,
    0x28, 0x06, 0x1E, 0xC1, 0xFE, 0x64, 0x20, 0x06,
    0x7B, 0xE2, 0x0C, 0x3E, 0x87, 0xE2, 0xF0, 0x42,
    0x90, 0xE0, 0x42, 0x15, 0x20, 0xD2, 0x05, 0x20,
    0x4F, 0x16, 0x20, 0x18, 0xCB, 0x4F, 0x06, 0x04,
    0xC5, 0xCB, 0x11, 0x17, 0xC1, 0xCB, 0x11, 0x17,
    0x05, 0x20, 0xF5, 0x22, 0x23, 0x22, 0x23, 0xC9,
    0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B,
    0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
    0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E,
    0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
    0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC,
    0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E,
    0x3C, 0x42, 0xB9, 0xA5, 0xB9, 0xA5, 0x42, 0x3C,
    0x21, 0x04, 0x01, 0x11, 0xA8, 0x00, 0x1A, 0x13,
    0xBE, 0x20, 0xFE, 0x23, 0x7D, 0xFE, 0x34, 0x20,
    0xF5, 0x06, 0x19, 0x78, 0x86, 0x23, 0x05, 0x20,
    0xFB, 0x86, 0x20, 0xFE, 0x3E, 0x01, 0xE0, 0x50
};

const BYTE GAMEBOY_LOGO[] = {
    0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 
    0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
    0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E, 
    0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
    0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 
    0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E
};

CHECK_ACCESS_FUNC(_check_always_yes) {
    return 1;
}
CHECK_ACCESS_FUNC(_check_always_no) {
    return 0;
}
CHECK_ACCESS_FUNC(_check_dma) {
    BYTE source = get_mem_source(flags);
    int result;
    LCDControl lcdc = state->ppu->lcdc;

    if (lcdc.lcd_enable == OFF)
        return 1;
        
    if (source == MEM_SOURCE_DMA)
        result = 1;
    else {
        if (state->dma->status == DMA_ON)
            result = 0;
        else
            result = 1;
    }

    return result;
}
CHECK_ACCESS_FUNC(_sys_check_vram) {
    BYTE source = get_mem_source(flags);
    int result;
    LCDStatus stat = state->ppu->stat;
    LCDControl lcdc = state->ppu->lcdc;

    if (lcdc.lcd_enable == OFF)
        return 1;

    switch (source) {
        case MEM_SOURCE_DMA:
            result = 1;
            break;
        case MEM_SOURCE_PPU:
            if (state->dma->status == DMA_ON)
                result = 0;
            else
                result = 1;
            break;
        default:
            if (state->dma->status == DMA_ON)
                result = 0;
            else 
                result = (stat.mode != DRAW);
            break;
    }

    return result;
}

CHECK_ACCESS_FUNC(_sys_check_oam_table) {
    BYTE source = get_mem_source(flags);
    int result;
    LCDStatus stat = state->ppu->stat;
    LCDControl lcdc = state->ppu->lcdc;

    if (lcdc.lcd_enable == OFF)
        return 1;
    
    switch (source) {
        case MEM_SOURCE_DMA:
            result = 1;
            break;
        case MEM_SOURCE_PPU:
            if (state->dma->status == DMA_ON)
                result = 0;
            else
                result = 1;
            break;
        default:
            if (state->dma->status == DMA_ON)
                result = 0;
            else 
                result = (stat.mode != DRAW) && (stat.mode != OAMSCAN);
            break;
    }

    return result; 
}
CHECK_ACCESS_FUNC(_check_obp) {
    BYTE source = get_mem_source(flags);
    int result;
    return 1;

    if (source == MEM_SOURCE_PPU) {
        result = 1;
    } else {
        if (state->ppu->stat.mode == DRAW) {
            result = 0;
        } else {
            result = 1;
        }
    }

    return result;
}

READ_FUNC(_sys_read_boot_rom) {
    return mem_sys(state, SysMemState)->bootrom[rel_addr];
}

READ_FUNC(_sys_read_vram) {
    return mem_sys(state, SysMemState)->vram[rel_addr];
}

WRITE_FUNC(_sys_write_vram) {
    mem_sys(state, SysMemState)->vram[rel_addr] = data;

    return 1;
}

READ_FUNC(_sys_read_wram) {
    return mem_sys(state, SysMemState)->wram[rel_addr];
}
WRITE_FUNC(_sys_write_wram) {
    mem_sys(state, SysMemState)->wram[rel_addr] = data;

    return 1;
}
READ_FUNC(_sys_read_oam_table) {
    return mem_sys(state, SysMemState)->oam_table[rel_addr];
}

WRITE_FUNC(_sys_write_oam_table) {
    mem_sys(state, SysMemState)->oam_table[rel_addr] = data;

    return 1;
}

READ_FUNC(_sys_read_hiram) {
    return mem_sys(state, SysMemState)->hram[rel_addr];
}

WRITE_FUNC(_sys_write_hiram) {
    mem_sys(state, SysMemState)->hram[rel_addr] = data;

    return 1;
}

READ_FUNC(_basic_read_rom) {
    return mem_cart(state, BasicCartState)->rom[rel_addr];
}

WRITE_FUNC(_basic_write_rom) {
    mem_cart(state, BasicCartState)->rom[rel_addr] = data;
    return 1;
}

READ_FUNC(_debug_read_mem) {
    return mem_sys(state, DebugMemState)->mem[rel_addr];
}

WRITE_FUNC(_debug_write_mem) {
    mem_sys(state, DebugMemState)->mem[rel_addr] = data;
    return 1;
}

/* BEGIN ioreg read/write functions */
READ_FUNC(_read_p1) {
    BYTE action_buttons = state->sdl->action_buttons;
    BYTE direction_buttons = state->sdl->direction_buttons;
    BYTE button_select = state->sdl->button_select;

    BYTE result = 0xFF;

    if (joypad_action_selected(button_select))
        result &= action_buttons;
    
    if (joypad_direction_selected(button_select))
        result &= direction_buttons;

    return result;
}
WRITE_FUNC(_write_p1) {
    // Only writable bits are 4 and 5
    data = (data & 0x30) | 0xCF;
    state->sdl->button_select = data;
    return 1;
}
READ_FUNC(_read_div) {
    return state->timer->reg_div;
}
WRITE_FUNC(_write_div) {
    state->timer->reg_div = 0;
    return 1;    
}
READ_FUNC(_read_tima) {
    return state->timer->reg_tima;
}
WRITE_FUNC(_write_tima) {
    state->timer->reg_tima = data;
    return 1;
}
READ_FUNC(_read_tma) {
    return state->timer->reg_tma;
}
WRITE_FUNC(_write_tma) {
    state->timer->reg_tma = data;
    return 1;
}
READ_FUNC(_read_tac) {
    return state->timer->reg_tac;
}
WRITE_FUNC(_write_tac) {
    data &= 0x07;
    static unsigned int tima_period_masks[] = {0x3FF, 0xF, 0x3F, 0xFF};

    if (bit_2(data)) 
        state->timer->timer_enabled = ON;
    else
        state->timer->timer_enabled = OFF;

    state->timer->reg_tac = data;
    state->timer->tima_period_mask = tima_period_masks[data & 0x3];
    
    return 1;
}
READ_FUNC(_read_if) {
    // Upper 3 bits always read as 1
    return 0xE0 | state->cpu->int_flag;
}
WRITE_FUNC(_write_if) {
    state->cpu->int_flag = data & 0x1F;
    return 1;    
}
READ_FUNC(_read_ie) {
    return state->cpu->int_enable;
}
WRITE_FUNC(_write_ie) {
    state->cpu->int_enable = data;
    return 1;    
}

READ_FUNC(_read_dma) {
    return state->dma->addr >> 8;
}
WRITE_FUNC(_write_dma) {
    state->dma->addr = ((WORD)data) << 8;
    state->dma->status = DMA_INIT;
    return 1;    
}
READ_FUNC(_read_lcdc) {
    LCDControl lcdc = state->ppu->lcdc;
    BYTE result = 0;
    result |= ((lcdc.lcd_enable == ON) ? 0x80 : 0);
    result |= ((lcdc.win_map_area == MAP_AREA1) ? 0x40 : 0);
    result |= ((lcdc.window_enable == ON) ? 0x20 : 0);
    result |= ((lcdc.bg_win_data_area == DATA_AREA1) ? 0x10 : 0);
    result |= ((lcdc.bg_map_area == MAP_AREA1) ? 0x8 : 0);
    result |= ((lcdc.obj_size == OBJ_8x16) ? 0x4 : 0);
    result |= ((lcdc.obj_enable == ON) ? 0x2 : 0);
    result |= ((lcdc.bg_window_enable == ON) ? 1 : 0);
    return result;
}
WRITE_FUNC(_write_lcdc) {
    LCDControl *lcdc = &state->ppu->lcdc;
    lcdc->lcd_enable = bit_7(data) ? ON : OFF;
    lcdc->win_map_area = bit_6(data) ? MAP_AREA1 : MAP_AREA0;
    lcdc->window_enable = bit_5(data) ? ON : OFF;
    lcdc->bg_win_data_area = bit_4(data) ? DATA_AREA1 : DATA_AREA0;
    lcdc->bg_map_area = bit_3(data) ? MAP_AREA1 : MAP_AREA0;
    lcdc->obj_size = bit_2(data) ? OBJ_8x16 : OBJ_8x8;
    lcdc->obj_enable = bit_1(data) ? ON : OFF;
    lcdc->bg_window_enable = bit_0(data) ? ON : OFF;

    return 1;    
}
READ_FUNC(_read_stat) {
    PPUState *ppu = state->ppu;
    BYTE result = 0;
    result |= ((ppu->stat.lyc_ly_interrupt == ON) ? 0x40 : 0x00);
    result |= ((ppu->stat.mode_2_interrupt == ON) ? 0x20 : 0x00);
    result |= ((ppu->stat.mode_1_interrupt == ON) ? 0x10 : 0x00);
    result |= ((ppu->stat.mode_0_interrupt == ON) ? 0x08 : 0x00);
    ppu->stat.lyc_ly_equal = (ppu->misc.ly == ppu->misc.lyc) ? ON : OFF;
    result |= ((ppu->stat.lyc_ly_equal == ON) ? 0x04 : 0x00);
    result |= ((BYTE)ppu->stat.mode & 0x3);
    return result;
}
WRITE_FUNC(_write_stat) {
    PPUState *ppu = state->ppu;
    // Only bits 3-6 are writable
    data &= 0x78;
    ppu->stat.lyc_ly_interrupt = bit_6(data) ? ON : OFF;
    ppu->stat.mode_2_interrupt = bit_5(data) ? ON : OFF;
    ppu->stat.mode_1_interrupt = bit_4(data) ? ON : OFF;
    ppu->stat.mode_0_interrupt = bit_3(data) ? ON : OFF;
    return 1;    
}
READ_FUNC(_read_scy) {
    return state->ppu->misc.scy;
}
WRITE_FUNC(_write_scy) {
    state->ppu->misc.scy = data;
    return 1;
}
READ_FUNC(_read_scx) {
    return state->ppu->misc.scx;
}
WRITE_FUNC(_write_scx) {
    state->ppu->misc.scx = data;
    return 1;
}
READ_FUNC(_read_ly) {
    return state->ppu->misc.ly;
}
READ_FUNC(_read_lyc) {
    return state->ppu->misc.lyc;
}
WRITE_FUNC(_write_lyc) {
    state->ppu->misc.lyc = data;
    return 1;
}
READ_FUNC(_read_wy) {
    return state->ppu->misc.wy;
}
WRITE_FUNC(_write_wy) {
    state->ppu->misc.wy = data;
    return 1;
}
READ_FUNC(_read_wx) {
    return state->ppu->misc.wx;
}
WRITE_FUNC(_write_wx) {
    state->ppu->misc.wx = data;
    return 1;
}
READ_FUNC(_read_bgp) {
    return state->ppu->misc.bgp;
}
WRITE_FUNC(_write_bgp) {
    state->ppu->misc.bgp = data;
    return 1;
}
READ_FUNC(_read_obp0) {
    return state->ppu->misc.obp0;
}
WRITE_FUNC(_write_obp0) {
    state->ppu->misc.obp0 = data;
    return 1;
}
READ_FUNC(_read_obp1) {
    return state->ppu->misc.obp1;
}
WRITE_FUNC(_write_obp1) {
    state->ppu->misc.obp1 = data;
    return 1;
} 
READ_FUNC(_read_boot) {
    if (mem_sys(state, SysMemState)->bootrom_mapped)
        return 0xFE;
    else
        return 0xFF;
}
WRITE_FUNC(_write_boot) {
    SysMemState *sys_mem = mem_sys(state, SysMemState);
    if (sys_mem->bootrom_mapped && (data & 0x1)) {
        sys_mem->bootrom_mapped = 0;
        replace_mem_region(&state->mem->cartridge->regions[0], state->mem->table);
    }

    return 1;
}

READ_FUNC(_read_unimplemented) {
    return UNINIT;
}
WRITE_FUNC(_write_unimplemented) {
    return -1;
}

/* END ioreg functions */

static const IOReg_t ioreg_table[] = {
    {
        .name="p1\0\0\0\0",
        .addr=0xFF00,
        .check_access=&_check_always_yes,
        .read=&_read_p1,
        .write=&_write_p1
    },
    {
        .name="sb\0\0\0\0",
        .addr=0xFF01,
        .check_access=&_check_always_yes,
        .read=&_read_sb,
        .write=&_write_sb
    },
    {
        .name="sb\0\0\0\0",
        .addr=0xFF02,
        .check_access=&_check_always_yes,
        .read=&_read_sc,
        .write=&_write_sc
    },
    unused_ioreg(0xFF03),
    {
        .name="div\0\0\0",
        .addr=0xFF04,
        .check_access=&_check_always_yes,
        .read=&_read_div,
        .write=&_write_div
    },
    {
        .name="tima\0\0",
        .addr=0xFF05,
        .check_access=&_check_always_yes,
        .read=&_read_tima,
        .write=&_write_tima
    },
    {
        .name="tma\0\0\0",
        .addr=0xFF06,
        .check_access=&_check_always_yes,
        .read=&_read_tma,
        .write=&_write_tma
    },
    {
        .name="tac\0\0\0",
        .addr=0xFF07,
        .check_access=&_check_always_yes,
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
        .check_access=&_check_always_yes,
        .read=&_read_if,
        .write=&_write_if
    },
    {
        .name="nr10\0\0",
        .addr=0xFF10,
        .check_access=&_check_always_yes,
        .read=&_read_nr10,
        .write=&_write_nr10
    },
    {
        .name="nr11\0\0",
        .addr=0xFF11,
        .check_access=&_check_always_yes,
        .read=&_read_nr11,
        .write=&_write_nr11
    },
    {
        .name="nr12\0\0",
        .addr=0xFF12,
        .check_access=&_check_always_yes,
        .read=&_read_nr12,
        .write=&_write_nr12
    },
    {
        .name="nr13\0\0",
        .addr=0xFF13,
        .check_access=&_check_always_yes,
        .read=&_read_nr13,
        .write=&_write_nr13
    },
    {
        .name="nr14\0\0",
        .addr=0xFF14,
        .check_access=&_check_always_yes,
        .read=&_read_nr14,
        .write=&_write_nr14
    },
    unused_ioreg(0xFF15),
    {
        .name="nr21\0\0",
        .addr=0xFF16,
        .check_access=&_check_always_yes,
        .read=&_read_nr16,
        .write=&_write_nr16
    },
    {
        .name="nr22\0\0",
        .addr=0xFF17,
        .check_access=&_check_always_yes,
        .read=&_read_nr22,
        .write=&_write_nr22
    },
    {
        .name="nr23\0\0",
        .addr=0xFF18,
        .check_access=&_check_always_yes,
        .read=&_read_nr23,
        .write=&_write_nr23
    },
    {
        .name="nr24\0\0",
        .addr=0xFF19,
        .check_access=&_check_always_yes,
        .read=&_read_nr24,
        .write=&_write_nr24
    },
    {
        .name="nr30\0\0",
        .addr=0xFF1A,
        .check_access=&_check_always_yes,
        .read=&_read_nr30,
        .write=&_write_nr30
    },
    {
        .name="nr31\0\0",
        .addr=0xFF1B,
        .check_access=&_check_always_yes,
        .read=&_read_nr31,
        .write=&_write_nr31
    },
    {
        .name="nr32\0\0",
        .addr=0xFF1C,
        .check_access=&_check_always_yes,
        .read=&_read_nr32,
        .write=&_write_nr32
    },
    {
        .name="nr33\0\0",
        .addr=0xFF1D,
        .check_access=&_check_always_yes,
        .read=&_read_nr33,
        .write=&_write_nr33
    },
    {
        .name="nr34\0\0",
        .addr=0xFF1E,
        .check_access=&_check_always_yes,
        .read=&_read_nr34,
        .write=&_write_nr34
    },
    unused_ioreg(0xFF1F),
    {
        .name="nr41\0\0",
        .addr=0xFF20,
        .check_access=&_check_always_yes,
        .read=&_read_nr41,
        .write=&_write_nr41
    },
    {
        .name="nr42\0\0",
        .addr=0xFF21,
        .check_access=&_check_always_yes,
        .read=&_read_nr42,
        .write=&_write_nr42
    },
    {
        .name="nr43\0\0",
        .addr=0xFF22,
        .check_access=&_check_always_yes,
        .read=&_read_nr43,
        .write=&_write_nr43
    },
    {
        .name="nr44\0\0",
        .addr=0xFF23,
        .check_access=&_check_always_yes,
        .read=&_read_nr44,
        .write=&_write_nr44
    },
    {
        .name="nr50\0\0",
        .addr=0xFF24,
        .check_access=&_check_always_yes,
        .read=&_read_nr50,
        .write=&_write_nr50
    },
    {
        .name="nr51\0\0",
        .addr=0xFF25,
        .check_access=&_check_always_yes,
        .read=&_read_nr51,
        .write=&_write_nr51
    },
    {
        .name="nr52\0\0",
        .addr=0xFF26,
        .check_access=&_check_always_yes,
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
        .check_access=&_check_always_yes,
        .read=&_read_wav_regs,
        .write=&_write_wav_regs
    },
    {
        .name="wav01\0",
        .addr=0xFF31,
        .check_access=&_check_always_yes,
        .read=&_read_wav_regs,
        .write=&_write_wav_regs
    },
    {
        .name="wav02\0",
        .addr=0xFF32,
        .check_access=&_check_always_yes,
        .read=&_read_wav_regs,
        .write=&_write_wav_regs
    },
    {
        .name="wav03\0",
        .addr=0xFF33,
        .check_access=&_check_always_yes,
        .read=&_read_wav_regs,
        .write=&_write_wav_regs
    },
    {
        .name="wav04\0",
        .addr=0xFF34,
        .check_access=&_check_always_yes,
        .read=&_read_wav_regs,
        .write=&_write_wav_regs
    },
    {
        .name="wav05\0",
        .addr=0xFF35,
        .check_access=&_check_always_yes,
        .read=&_read_wav_regs,
        .write=&_write_wav_regs
    },
    {
        .name="wav06\0",
        .addr=0xFF36,
        .check_access=&_check_always_yes,
        .read=&_read_wav_regs,
        .write=&_write_wav_regs
    },
    {
        .name="wav07\0",
        .addr=0xFF37,
        .check_access=&_check_always_yes,
        .read=&_read_wav_regs,
        .write=&_write_wav_regs
    },
    {
        .name="wav08\0",
        .addr=0xFF38,
        .check_access=&_check_always_yes,
        .read=&_read_wav_regs,
        .write=&_write_wav_regs
    },
    {
        .name="wav09\0",
        .addr=0xFF39,
        .check_access=&_check_always_yes,
        .read=&_read_wav_regs,
        .write=&_write_wav_regs
    },
    {
        .name="wav10\0",
        .addr=0xFF3A,
        .check_access=&_check_always_yes,
        .read=&_read_wav_regs,
        .write=&_write_wav_regs
    },
    {
        .name="wav11\0",
        .addr=0xFF3B,
        .check_access=&_check_always_yes,
        .read=&_read_wav_regs,
        .write=&_write_wav_regs
    },
    {
        .name="wav12\0",
        .addr=0xFF3C,
        .check_access=&_check_always_yes,
        .read=&_read_wav_regs,
        .write=&_write_wav_regs
    },
    {
        .name="wav13\0",
        .addr=0xFF3D,
        .check_access=&_check_always_yes,
        .read=&_read_wav_regs,
        .write=&_write_wav_regs
    },
    {
        .name="wav14\0",
        .addr=0xFF3E,
        .check_access=&_check_always_yes,
        .read=&_read_wav_regs,
        .write=&_write_wav_regs
    },
    {
        .name="wav15\0",
        .addr=0xFF3F,
        .check_access=&_check_always_yes,
        .read=&_read_wav_regs,
        .write=&_write_wav_regs
    },
    {
        .name="lcdc\0\0",
        .addr=0xFF40,
        .check_access=&_check_always_yes,
        .read=&_read_lcdc,
        .write=&_write_lcdc
    },
    {
        .name="stat\0\0",
        .addr=0xFF41,
        .check_access=&_check_always_yes,
        .read=&_read_stat,
        .write=&_write_stat
    },
    {
        .name="scy\0\0\0",
        .addr=0xFF42,
        .check_access=&_check_always_yes,
        .read=&_read_scy,
        .write=&_write_scy
    },
    {
        .name="scx\0",
        .addr=0xFF43,
        .check_access=&_check_always_yes,
        .read=&_read_scx,
        .write=&_write_scx
    },
    {
        .name="ly\0\0\0\0",
        .addr=0xFF44,
        .check_access=&_check_always_yes,
        .read=&_read_ly,
        .write=&_write_unimplemented
    },
    {
        .name="lyc\0\0\0",
        .addr=0xFF45,
        .check_access=&_check_always_yes,
        .read=&_read_lyc,
        .write=&_write_lyc
    },
    {
        .name="dma\0\0\0",
        .addr=0xFF46,
        .check_access=&_check_always_yes,
        .read=&_read_dma,
        .write=&_write_dma
    },
    {
        .name="bgp\0\0\0",
        .addr=0xFF47,
        .check_access=&_check_obp,
        .read=&_read_bgp,
        .write=&_write_bgp
    },
    {
        .name="obp0\0\0",
        .addr=0xFF48,
        .check_access=&_check_obp,
        .read=&_read_obp0,
        .write=&_write_obp0
    },
    {
        .name="obp1\0",
        .addr=0xFF49,
        .check_access=&_check_obp,
        .read=&_read_obp1,
        .write=&_write_obp1
    },
    {
        .name="wy\0\0\0\0",
        .addr=0xFF4A,
        .check_access=&_check_always_yes,
        .read=&_read_wy,
        .write=&_write_wy
    },
    {
        .name="wx\0\0\0\0",
        .addr=0xFF4B,
        .check_access=&_check_always_yes,
        .read=&_read_wx,
        .write=&_write_wx
    },
    unused_ioreg(0xFF4C),
    {
        .name="key1\0\0",
        .addr=0xFF4D,
        .check_access=&_check_always_yes,
        .read=&_read_key1,
        .write=&_write_key1
    },
    unused_ioreg(0xFF4E),
    {
        .name="vbk\0\0\0",
        .addr=0xFF4F,
        .check_access=&_check_always_yes,
        .read=&_read_vbk,
        .write=&_write_vbk
    },
    {
        .name="boot\0\0",
        .addr=0xFF50,
        .check_access=&_check_always_yes,
        .read=&_read_boot,
        .write=&_write_boot
    },
    {
        .name="hdma1\0",
        .addr=0xFF51,
        .check_access=&_check_always_yes,
        .read=&_read_hdma,
        .write=&_write_hdma
    },
    {
        .name="hdma2\0",
        .addr=0xFF52,
        .check_access=&_check_always_yes,
        .read=&_read_hdma,
        .write=&_write_hdma
    },
    {
        .name="hdma3\0",
        .addr=0xFF53,
        .check_access=&_check_always_yes,
        .read=&_read_hdma,
        .write=&_write_hdma
    },
    {
        .name="hdma4\0",
        .addr=0xFF54,
        .check_access=&_check_always_yes,
        .read=&_read_hdma,
        .write=&_write_hdma
    },
    {
        .name="hdma5\0",
        .addr=0xFF55,
        .check_access=&_check_always_yes,
        .read=&_read_hdma,
        .write=&_write_hdma
    },
    {
        .name="rp\0\0\0\0",
        .addr=0xFF56,
        .check_access=&_check_always_yes,
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
        .check_access=&_check_always_yes,
        .read=&_read_bcps,
        .write=&_write_bcps
    },
    {
        .name="bcpd\0\0",
        .addr=0xFF69,
        .check_access=&_check_always_yes,
        .read=&_read_bcpd,
        .write=&_write_bcpd
    },
    {
        .name="ocps\0\0",
        .addr=0xFF6A,
        .check_access=&_check_always_yes,
        .read=&_read_ocps,
        .write=&_write_ocps
    },
    {
        .name="ocpd\0\0",
        .addr=0xFF6B,
        .check_access=&_check_always_yes,
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
        .check_access=&_check_always_yes,
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
        .check_access=&_check_always_yes,
        .read=&_read_pcm12,
        .write=&_write_pcm12
    },
    {
        .name="pcm34\0",
        .addr=0xFF77,
        .check_access=&_check_always_yes,
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
        .check_access=&_check_always_yes,
        .read=&_read_ie,
        .write=&_write_ie
    }
};
READ_FUNC(_sys_read_ioreg) {
    BYTE result;
    int accessible;
    
    if (rel_addr > 0x7F)
        return UNINIT;

    accessible = ioreg_table[rel_addr].check_access(__CHECK_ACCESS_ARGS);
    if (accessible)
        result = ioreg_table[rel_addr].read(__READ_ARGS);
    else
        result = UNINIT;
    
    return result;
}

WRITE_FUNC(_sys_write_ioreg) {
    int result, accessible;
    
    if (rel_addr > 0x7F)
        return -1;
    
    accessible = ioreg_table[rel_addr].check_access(__CHECK_ACCESS_ARGS);
    if (accessible)
        result = ioreg_table[rel_addr].write(__WRITE_ARGS);
    else
        result = -1;
    
    return result; 
}

GET_PTR_FUNC(_ptr_unimplemented) {
    return NULL;
}
GET_PTR_FUNC(_sys_ptr_boot_rom) {
    return &mem_sys(state, SysMemState)->bootrom[rel_addr];
}
GET_PTR_FUNC(_sys_ptr_vram) {
    return &mem_sys(state, SysMemState)->vram[rel_addr];
}
GET_PTR_FUNC(_sys_ptr_wram) {
    return &mem_sys(state, SysMemState)->wram[rel_addr];
}
GET_PTR_FUNC(_sys_ptr_hiram) {
    return &mem_sys(state, SysMemState)->hram[rel_addr];
}
GET_PTR_FUNC(_sys_ptr_oam_table) {
    return &mem_sys(state, SysMemState)->oam_table[rel_addr];
}
GET_PTR_FUNC(_basic_ptr) {
    return &mem_cart(state, BasicCartState)->rom[rel_addr];
}
GET_PTR_FUNC(_debug_ptr) {
    return &mem_sys(state, DebugMemState)->mem[rel_addr];
}
READ_FUNC(_mbc1_read_rom_base) {
    /* addr/rel_addr = 0 - 0x3FFF */
    MBC1CartState *mbc1 = mem_cart(state, MBC1CartState);
    int eff_addr, active_rom_bank;

    active_rom_bank = 0;
    if (mbc1->bank_mode == MODE_ADVANCED
    && mbc1->cart_type == LARGE_ROM)
        active_rom_bank = (mbc1->reg_2_2bits << 5) & mbc1->rom_bank_mask;

    eff_addr = active_rom_bank * ROM_BANK_SIZE + rel_addr;
    return mbc1->rom_banks[eff_addr];
}
WRITE_FUNC(_mbc1_write_0_3fff) {
    /* addr/rel_addr = 0 - 0x3FFF */
    MBC1CartState *mbc1 = mem_cart(state, MBC1CartState);
    if (rel_addr < 0x2000) {
        /* 0x0000 - 0x1FFF */
        if (ls_nib(data) == 0xA)
            mbc1->ram_enabled = 1;
        else
            mbc1->ram_enabled = 0;
    } else {
        /* 0x2000 - 0x3FFF */
        int active_rom_bank;

        data &= 0x1F;
        if (data == 0) data = 1;
        data &= mbc1->rom_bank_mask;
        mbc1->reg_1_5bits = data;
    }

    return 1;
}
GET_PTR_FUNC(_mbc1_ptr_rom_base) {

}
READ_FUNC(_mbc1_read_rom_1) {
    /*  addr = 0x4000 - 0x7FFF
    rel_addr = 0 - 0x3FFF */
    MBC1CartState *mbc1 = mem_cart(state, MBC1CartState);
    int eff_addr, active_rom_bank;

    if (mbc1->cart_type == LARGE_ROM)
        active_rom_bank = ((mbc1->reg_2_2bits & 0x3) << 5) | mbc1->reg_1_5bits;
    else
        active_rom_bank = mbc1->reg_1_5bits;
    
    active_rom_bank &= mbc1->rom_bank_mask;

    mbc1->active_rom_bank = active_rom_bank;

    eff_addr = mbc1->active_rom_bank * ROM_BANK_SIZE + rel_addr;

    return mbc1->rom_banks[eff_addr];
}
WRITE_FUNC(_mbc1_write_4000_7fff) {
    /*  addr = 0x4000 - 0x7FFF
    rel_addr = 0 - 0x3FFF */
    MBC1CartState *mbc1 = mem_cart(state, MBC1CartState);

    if (rel_addr < 0x2000) { 
        /* 0x4000 - 0x5FFF */
        data &= 0x3;
        mbc1->reg_2_2bits = data;
    } else {
        /* 0x6000 - 0x7FFF */
        mbc1->bank_mode = (data & 0x1) ? MODE_ADVANCED : MODE_SIMPLE;
    }
    return 1;
}
GET_PTR_FUNC(_mbc1_ptr_rom_1) {
    
}
READ_FUNC(_mbc1_read_ram_bank) {
    /* addr  = 0xA000 - 0xBFFF 
    rel_addr = 0 - 0x1FFF */

    MBC1CartState *mbc1 = mem_cart(state, MBC1CartState);
    int eff_addr, active_ram_bank;

    if (! mbc1->ram_enabled)
        return UNINIT;
    
    active_ram_bank = 0;
    if (mbc1->bank_mode == MODE_ADVANCED
    && mbc1->cart_type == LARGE_RAM)
        active_ram_bank = mbc1->reg_2_2bits;
    
    mbc1->active_ram_bank = active_ram_bank;
    assert(mbc1->active_ram_bank < mbc1->n_ram_banks);
    
    eff_addr = mbc1->active_ram_bank * RAM_BANK_SIZE + rel_addr;

    return mbc1->ram_banks[eff_addr];
}
WRITE_FUNC(_mbc1_write_ram_bank) {
    /* addr  = 0xA000 - 0xBFFF 
    rel_addr = 0 - 0x1FFF */
    MBC1CartState *mbc1 = mem_cart(state, MBC1CartState);
    int eff_addr, active_ram_bank;

    if (! mbc1->ram_enabled)
        return -1;

    active_ram_bank = 0;
    if (mbc1->bank_mode == MODE_ADVANCED
    && mbc1->cart_type == LARGE_RAM)
        active_ram_bank = mbc1->reg_2_2bits;
    
    mbc1->active_ram_bank = active_ram_bank;
    assert(mbc1->active_ram_bank < mbc1->n_ram_banks);

    eff_addr = mbc1->active_ram_bank * RAM_BANK_SIZE + rel_addr;

    mbc1->ram_banks[eff_addr] = data;

    return 1;
}
GET_PTR_FUNC(_mbc1_ptr_ram_bank) {
    
}

READ_FUNC(_mbc3_read_rom_base) {
    /* addr/rel_addr = 0 - 0x3FFF */
    MBC3CartState *mbc3 = mem_cart(state, MBC3CartState);
    return mbc3->rom_banks[rel_addr];
}
WRITE_FUNC(_mbc3_write_0_3fff) {
    /* addr/rel_addr = 0 - 0x3FFF */
    MBC3CartState *mbc3 = mem_cart(state, MBC3CartState);
    if (rel_addr < 0x2000) {
        /* 0x0000 - 0x1FFF */
        if (ls_nib(data) == 0xA)
            mbc3->ram_rtc_enabled = 1;
        else
            mbc3->ram_rtc_enabled = 0;
    } else {
        /* 0x2000 - 0x3FFF */
        data &= 0x7F;
        if (data == 0) data = 1;
        data &= mbc3->rom_bank_mask;
        mbc3->reg_1_7bits = data;
        mbc3->active_rom_bank = data;
    }

    return 1;
}
READ_FUNC(_mbc3_read_rom_1) {
    /*  addr = 0x4000 - 0x7FFF
    rel_addr = 0 - 0x3FFF */
    MBC3CartState *mbc3 = mem_cart(state, MBC3CartState);
    int eff_addr;
    eff_addr = mbc3->active_rom_bank * ROM_BANK_SIZE + rel_addr;
    
    return mbc3->rom_banks[eff_addr];
}

void _update_mbc3_rtc(MBC3CartState *mbc3) {
    time_t time_epoch;
    struct tm *time_struct;

    time_epoch = time(NULL);
    time_struct = localtime(&time_epoch);

    mbc3->rtc.time_epoch = time_epoch;
    mbc3->rtc.time_struct = time_struct; 
}

WRITE_FUNC(_mbc3_write_4000_7fff) {
    /*  addr = 0x4000 - 0x7FFF
    rel_addr = 0 - 0x3FFF */
    MBC3CartState *mbc3 = mem_cart(state, MBC3CartState);

    if (rel_addr < 0x2000) { 
        /* 0x4000 - 0x5FFF */
        data &= 0xF;
        mbc3->reg_2_4bits = data;
        if (data < 4) {
            mbc3->active_ram_bank = (data < mbc3->n_ram_banks) ? data : 0;
            mbc3->bank_mode = MODE_RAM;
        } else if (data > 0x7 && data < 0xD) {
            mbc3->active_rtc_bank = data;
            mbc3->bank_mode = MODE_RTC;
        }

    } else {
        /* 0x6000 - 0x7FFF */
        switch (mbc3->rtc.latch) {
            case LATCH_0:
                if (data == 0)
                    mbc3->rtc.latch = LATCH_1;
                break;
            case LATCH_1:
                if (data == 1) {
                    mbc3->rtc.latch = LATCHED;
                    _update_mbc3_rtc(mbc3);
                }
                break;
            case LATCHED:
                if (data == 0)
                    mbc3->rtc.latch = LATCH_1;
                break;
        }
    }
    return 1;
}

READ_FUNC(_mbc3_read_ram_or_rtc) {
    /* 0xA000 - 0xBFFF */
    MBC3CartState *mbc3 = mem_cart(state, MBC3CartState); 
    int eff_addr;
    BYTE result;
    time_t time_epoch;
    struct tm *time_struct;

    if (! mbc3->ram_rtc_enabled)
            return UNINIT;

    if (mbc3->bank_mode == MODE_RAM) {
        assert(mbc3->active_ram_bank < mbc3->n_ram_banks);

        eff_addr = mbc3->active_ram_bank * RAM_BANK_SIZE + rel_addr;
        result = mbc3->ram_banks[eff_addr];
    } else if (mbc3->bank_mode == MODE_RTC) {

        if (mbc3->rtc.latch != LATCHED)
            _update_mbc3_rtc(mbc3);

        switch (mbc3->active_rtc_bank) {
            case 0x8:
                result = mbc3->rtc.time_struct->tm_sec;
                break;
            case 0x9:
                result = mbc3->rtc.time_struct->tm_min;
                break;
            case 0xA:
                result = mbc3->rtc.time_struct->tm_hour;
                break;
            case 0xB:
                result = 0xFF & mbc3->rtc.time_struct->tm_yday;
                break;
            case 0xC:
                result = (0x100 & mbc3->rtc.time_struct->tm_yday) >> 8;
                result |= ((mbc3->rtc.halt) ? 0x40 : 0);
                result |= ((mbc3->rtc.day_carry) ? 0x80 : 0);
                break;
            default:
                result = UNINIT;
                break;
        }
    }

    return result;
}

WRITE_FUNC(_mbc3_write_ram_or_rtc) {
    /* 0xA000 - 0xBFFF */
    MBC3CartState *mbc3 = mem_cart(state, MBC3CartState); 
    int eff_addr; 

    if (! mbc3->ram_rtc_enabled)
        return -1;
    
    if (mbc3->bank_mode == MODE_RAM) {
        eff_addr = mbc3->active_ram_bank * RAM_BANK_SIZE + rel_addr;
        mbc3->ram_banks[eff_addr] = data;

    } else if (mbc3->bank_mode == MODE_RTC) {

    }

    return 1;
}

/* Memory maps */
MemoryRegion mbc1_mem_map[] = {
    { // 0001 1111 1111 1111
        .base=0x0000,
        .end=0x3FFF,
        .len=0x4000,
        .flags=0,
        .check_access=&_check_always_yes,
        .read=&_mbc1_read_rom_base,
        .write=&_mbc1_write_0_3fff,
        .get_ptr=&_mbc1_ptr_rom_base
    },
    { // 0101 1111 1111 1111
        .base=0x4000,
        .end=0x7FFF,
        .len=0x4000,
        .flags=0,
        .check_access=&_check_always_yes,
        .read=&_mbc1_read_rom_1,
        .write=&_mbc1_write_4000_7fff,
        .get_ptr=&_mbc1_ptr_rom_1
    },
    { // 1011 1111 1111 1111
        .base=0xA000,
        .end=0xBFFF,
        .len=0x2000,
        .flags=0,
        .check_access=&_check_always_yes,
        .read=&_mbc1_read_ram_bank,
        .write=&_mbc1_write_ram_bank,
        .get_ptr=&_mbc1_ptr_ram_bank
    }
};

MemoryRegion mbc3_mem_map[] = {
    {
        .base=0x0000,
        .end=0x3FFF,
        .len=0x4000,
        .flags=0,
        .check_access=&_check_always_yes,
        .read=&_mbc3_read_rom_base,
        .write=&_mbc3_write_0_3fff,
        .get_ptr=&_mbc1_ptr_rom_base
    },
    {
        .base=0x4000,
        .end=0x7FFF,
        .len=0x2000,
        .flags=0,
        .check_access=&_check_always_yes,
        .read=&_mbc3_read_rom_1,
        .write=&_mbc3_write_4000_7fff,
        .get_ptr=&_mbc1_ptr_rom_1
    },
    {
        .base=0xA000,
        .end=0xBFFF,
        .len=0x2000,
        .flags=0,
        .check_access=&_check_always_yes,
        .read=&_mbc3_read_ram_or_rtc,
        .write=&_mbc3_write_ram_or_rtc,
        .get_ptr=&_mbc1_ptr_ram_bank
    }
};


MemoryRegion system_mem_map[] = {
    { // 0000 0000 1111 1111
        .base=0x0000,
        .end=0xFF,
        .len=0x100,
        .flags=0,
        .check_access=&_check_dma,
        .read=&_sys_read_boot_rom,
        .write=&_write_unimplemented,
        .get_ptr=&_sys_ptr_boot_rom
    },
    { // 1001 1111 1111 1111
        .base=0x8000,
        .end=0x9FFF,
        .len=0x2000,
        .flags=0,
        .check_access=&_sys_check_vram,
        .read=&_sys_read_vram,
        .write=&_sys_write_vram,
        .get_ptr=&_sys_ptr_vram
    },
    { // 1101 1111 1111 1111
        .base=0xC000,
        .end=0xDFFF,
        .len=0x2000,
        .flags=0,
        .check_access=&_check_dma,
        .read=&_sys_read_wram,
        .write=&_sys_write_wram,
        .get_ptr=&_sys_ptr_wram
    },
    { // 1111 1101 1111 1111
        .base=0xE000,
        .end=0xFDFF,
        .len=0x1E00,
        .flags=0,
        .check_access=&_check_always_no,
        .read=&_read_unimplemented,
        .write=&_write_unimplemented,
        .get_ptr=&_ptr_unimplemented
    },
    { // 1111 1110 1001 1111
        .base=0xFE00,
        .end=0xFE9F,
        .len=0xA0,
        .flags=0,
        .check_access=&_sys_check_oam_table,
        .read=&_sys_read_oam_table,
        .write=&_sys_write_oam_table,
        .get_ptr=&_sys_ptr_oam_table
    },
    { // 1111 1110 1111 1111
        .base=0xFEA0,
        .end=0xFEFF,
        .len=0x60,
        .flags=0,
        .check_access=&_check_always_no,
        .read=&_read_unimplemented,
        .write=&_write_unimplemented,
        .get_ptr=&_ptr_unimplemented
    },
    { // 1111 1111 0111 1111
        .base=0xFF00,
        .end=0xFF7F,
        .len=0x80,
        .flags=0,
        .check_access=&_check_dma,
        .read=&_sys_read_ioreg,
        .write=&_sys_write_ioreg,
        .get_ptr=&_ptr_unimplemented
    },
    { // 1111 1111 1111 1110
        .base=0xFF80,
        .end=0xFFFE,
        .len=0x7F,
        .flags=0,
        .check_access=&_check_always_yes,
        .read=&_sys_read_hiram,
        .write=&_sys_write_hiram,
        .get_ptr=&_sys_ptr_hiram
    },
    { // 1111 1111 1111 1111
        .base=0xFFFF,
        .end=0xFFFF,
        .len=0x1,
        .flags=0,
        .check_access=&_check_always_yes,
        .read=&_read_ie,
        .write=&_write_ie,
        .get_ptr=&_ptr_unimplemented
    }
};

MemoryRegion basic_mem_map[] = {
    {
        .base=0x0000,
        .end=0x7FFF,
        .len=0x8000,
        .flags=0,
        .check_access=&_check_dma,
        .read=&_basic_read_rom,
        .write=&_basic_write_rom,
        .get_ptr=&_basic_ptr
    }
};

MemoryRegion debug_mem_map[] = {
    {
        .base=0x0000,
        .end=0xFFFF,
        .len=0x10000,
        .flags=0,
        .check_access=&_check_always_yes,
        .read=&_debug_read_mem,
        .write=&_debug_write_mem,
        .get_ptr=&_debug_ptr
    }
};

MemoryRegion *find_mem_region(GBState *state, WORD addr, BYTE flags) {
    return state->mem->table[addr];
}

int check_access(MemoryRegion *region, BYTE source_flags) {
    int accessible = 1;

    BYTE reg_flags = region->flags;
    if (reg_flags & MEM_LOCKED)
        accessible = get_mem_source(reg_flags) == get_mem_source(source_flags);
   
    accessible = accessible || (source_flags & MEM_DEBUG);

    return accessible;
}

BYTE read_mem(GBState *state, WORD addr, BYTE flags) {
    BYTE result = UNINIT;
    WORD rel_addr;
    int accessible;
    MemoryRegion *source;

    source = find_mem_region(state, addr, flags);
    
    if (source != NULL) {
        rel_addr = addr - source->base;

        accessible = (flags & MEM_DEBUG) || (*source->check_access)(state, rel_addr, flags);

        if (accessible) {
            result = (*source->read)(state, rel_addr, flags);
        } else {
            result = UNINIT;
        }
    }

    return result;
}

int write_mem(GBState *state, WORD addr, BYTE data, BYTE flags) {
    int status = -1;
    WORD rel_addr;
    int accessible;
    MemoryRegion *source;

    source = find_mem_region(state, addr, flags);

    if (source != NULL) {
        rel_addr = addr - source->base;

        accessible = (flags & MEM_DEBUG) || (*source->check_access)(state, rel_addr, flags);

        if (accessible) {
            status = (*source->write)(state, rel_addr, data, flags);
        } else {
            status = -1;
        }
    }

    return status;
}

BYTE *get_mem_pointer(GBState *state, WORD addr, BYTE flags) {
    WORD rel_addr;
    BYTE *pointer = NULL;
    int accessible;
    MemoryRegion *source;
    source = find_mem_region(state, addr, flags);

    if (source != NULL) {
        rel_addr = addr - source->base;

        accessible = (flags & MEM_DEBUG) || (*source->check_access)(state, rel_addr, flags);

        if (accessible) {
            pointer = (*source->get_ptr)(state, rel_addr, flags);
        } else {
            pointer = NULL;
        }
    }

    return pointer;
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

SysMemState *initialize_sys_memory(CartridgeHeader *header) {
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
    sys_mem->hram = allocate_region(0x7E, "hram\0");
    
    return sys_mem;
}

void teardown_sys_memory(SysMemState *sys_mem) {
    free(sys_mem->vram);
    free(sys_mem->wram);
    free(sys_mem->oam_table);
    free(sys_mem->hram);
}

BasicCartState *initialize_basic_memory(CartridgeHeader *header) {
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

DebugMemState *initialize_debug_memory(CartridgeHeader *header) {
    DebugMemState *debug = malloc(sizeof(DebugMemState));
    if (debug == NULL) {
        printf("Error allocating cartridge memory\n");
        exit(1);
    }
    memset(debug->mem, UNINIT, sizeof(debug->mem));
    return debug; 
}

void teardown_debug_memory(DebugMemState *debug) {
    free(debug);
}
int read_basic_rom_into_mem(GBState *state, FILE *fp) {
    int status = 0;
    int n_read;
    n_read = fread(mem_cart(state, BasicCartState)->rom, 1, 0x8000, fp);
    if (n_read != 0x8000) {
        printf("Error: read %04x rather than 0x8000\n", n_read);
        status = 1;
    }

    return status;
}

MBC1CartState *initialize_mbc1_memory(CartridgeHeader *header) {
    MBC1CartState *mbc1 = malloc(sizeof(MBC1CartState));
    if (mbc1 == NULL) {
        printf("Error allocating MBC1 memory\n");
        exit(1);
    }
    mbc1->ram_enabled = 0;
    mbc1->reg_1_5bits = 1;
    mbc1->reg_2_2bits = 0;
    mbc1->active_ram_bank = 0;
    mbc1->active_rom_bank = 1;
    mbc1->bank_mode = MODE_SIMPLE;

    if (header->rom_size < 7) {
        mbc1->n_rom_banks = 2 << header->rom_size;
        mbc1->rom_bank_mask = mbc1->n_rom_banks - 1;
    } else {
        printf("ILLEGAL ROM SIZE %d\n", header->rom_size);
    }
    
    switch(header->ram_size) {
        case 0:
            mbc1->n_ram_banks = 0;
            break;
        case 1:
        case 2:
            mbc1->n_ram_banks = 1;
            break;
        case 3:
            mbc1->n_ram_banks = 4;
            break;
        default:
            printf("ILLEGAL RAM SIZE %d\n", header->ram_size);
            mbc1->n_ram_banks = 0;
            break;
    }
    if (mbc1->n_ram_banks > 1) // > 8 KiB ram
        mbc1->cart_type = LARGE_RAM;
    else if (mbc1->n_rom_banks >= 64) // >= 1 MiB rom
        mbc1->cart_type = LARGE_ROM;
    else
        mbc1->cart_type = DEFAULT;

    int rom_total_size = mbc1->n_rom_banks * ROM_BANK_SIZE;
    int ram_total_size = mbc1->n_ram_banks * RAM_BANK_SIZE;
    mbc1->rom_banks = allocate_region(rom_total_size, "rom banks");
    mbc1->ram_banks = allocate_region(ram_total_size, "ram banks");

    return mbc1;
}

void teardown_mbc1_memory(MBC1CartState *mbc1) {
    if (mbc1->n_rom_banks > 0)
        free(mbc1->rom_banks);
    if (mbc1->n_ram_banks > 0)
        free(mbc1->ram_banks);
    
    free(mbc1);
}

int read_mbc1_rom_into_mem(GBState *state, FILE *fp) {
    MBC1CartState *mbc1 = mem_cart(state, MBC1CartState);

    int n_banks = mbc1->n_rom_banks;
    int total_rom_size = n_banks * ROM_BANK_SIZE;
    int n_read;

    fseek(fp, 0, SEEK_SET);

    n_read = fread(mbc1->rom_banks, ROM_BANK_SIZE, n_banks, fp);

    assert(n_read == n_banks);

    return 0;
}

MBC3CartState *initialize_mbc3_memory(CartridgeHeader *header) {
    MBC3CartState *mbc3 = malloc(sizeof(MBC1CartState));
    if (mbc3 == NULL) {
        printf("Error allocating MBC1 memory\n");
        exit(1);
    }
    mbc3->ram_rtc_enabled = 0;
    mbc3->rtc.time_epoch = time(NULL);
    mbc3->rtc.time_struct = localtime(&mbc3->rtc.time_epoch);
    mbc3->rtc.halt = 0;
    mbc3->rtc.day_carry = 0;

    mbc3->reg_1_7bits = 1;
    mbc3->reg_2_4bits = 0;
    mbc3->active_ram_bank = 0;
    mbc3->active_rtc_bank = 0;
    mbc3->active_rom_bank = 1;
    mbc3->bank_mode = MODE_SIMPLE;

    if (header->rom_size < 7) {
        mbc3->n_rom_banks = 2 << header->rom_size;
        mbc3->rom_bank_mask = mbc3->n_rom_banks - 1;
    } else {
        printf("ILLEGAL ROM SIZE %d\n", header->rom_size);
    }
    
    switch(header->ram_size) {
        case 0:
            mbc3->n_ram_banks = 0;
            break;
        case 1:
        case 2:
            mbc3->n_ram_banks = 1;
            break;
        case 3:
            mbc3->n_ram_banks = 4;
            break;
        default:
            printf("ILLEGAL RAM SIZE %d\n", header->ram_size);
            mbc3->n_ram_banks = 0;
            break;
    }

    int rom_total_size = mbc3->n_rom_banks * ROM_BANK_SIZE;
    int ram_total_size = mbc3->n_ram_banks * RAM_BANK_SIZE;
    mbc3->rom_banks = allocate_region(rom_total_size, "rom banks");
    mbc3->ram_banks = allocate_region(ram_total_size, "ram banks");

    return mbc3;
}

void teardown_mbc3_memory(MBC3CartState *mbc3) {
    if (mbc3->n_rom_banks > 0)
        free(mbc3->rom_banks);
    if (mbc3->n_ram_banks > 0)
        free(mbc3->ram_banks);
    
    free(mbc3);
}

int read_mbc3_rom_into_mem(GBState *state, FILE *fp) {
    MBC3CartState *mbc1 = mem_cart(state, MBC3CartState);

    int n_banks = mbc1->n_rom_banks;
    int total_rom_size = n_banks * ROM_BANK_SIZE;
    int n_read;

    fseek(fp, 0, SEEK_SET);

    n_read = fread(mbc1->rom_banks, ROM_BANK_SIZE, n_banks, fp);

    assert(n_read == n_banks);

    return 0;
}

void *initialize_null(CartridgeHeader *header) {
    return NULL;
}
void teardown_null(void *ptr) {

}

void mem_map_to_lookup(const MemoryRegion *regions, int n, MemoryRegion **table) {
    MemoryRegion *current;
    int cur, stop;

    for (int i = 0; i < n; i++) {
        current = &regions[i];
        cur = current->base;
        stop = current->end;
        do { 
            table[cur++] = current;
        } while (cur <= stop);
    }

}

void replace_mem_region(const MemoryRegion *new, MemoryRegion **table) {
    int cur, stop;
    cur = new->base;
    stop = new->end;
    do {
        table[cur++] = new;
    } while (cur <= stop);
}

MemoryState *initialize_memory(CartridgeHeader *header) {
    MemoryState *mem = malloc(sizeof(MemoryState));
    if (mem == NULL) {
        printf("Error allocating memory state\n");
        return NULL;
    }
    mem->header = header;

    mem->cartridge = malloc(sizeof(Memmap_t));
    mem->system = malloc(sizeof(Memmap_t));
    if (mem->cartridge == NULL || mem->system == NULL) {
        printf("Error allocating memmap_t\n");
        free(mem);
        return NULL;
    }

    for (int i = 0; i < TOTAL_MEM_SIZE; i++)
        mem->table[i] = NULL;

    if (header == NULL) {
        /* Debug mode - just setup plain 0x10000 byte array */
        mem->cartridge->n_regions = 0;
        mem->cartridge->regions = NULL;
        mem->cartridge->initialize = &initialize_null;
        mem->cartridge->teardown = &teardown_null;
        
        mem->system->n_regions = 1;
        mem->system->regions = debug_mem_map;
        mem->system->initialize = &initialize_debug_memory;
        mem->system->teardown = &teardown_debug_memory;

        goto init_done;
    } else {
        /* If not in debug mode, set up the system memory */
        mem->system->n_regions = 9;
        mem->system->regions = system_mem_map;
        mem->system->initialize = &initialize_sys_memory;
        mem->system->teardown = &teardown_sys_memory;
    }

    if (ms_nib(header->title_or_mfc[15]) == 0xC) {
        printf("Header suggests CGB-only, not supported\n");
        free(mem->cartridge);
        free(mem->system);
        free(mem);
        return NULL;
    }

    /* Set up cartridge memory based on header info */
    switch (header->cartridge_type) {
        case CART_ROM:
            mem->read_rom = &read_basic_rom_into_mem;
            mem->cartridge->n_regions = 1;
            mem->cartridge->regions = basic_mem_map;
            mem->cartridge->initialize = &initialize_basic_memory;
            mem->cartridge->teardown = &teardown_basic_memory;
            
            break;
        
        case CART_MBC1:
        case CART_MBC1_RAM:
        case CART_MBC1_RAM_BAT:
            mem->read_rom = &read_mbc1_rom_into_mem;
            mem->cartridge->n_regions = 3;
            mem->cartridge->regions = mbc1_mem_map;
            mem->cartridge->initialize = &initialize_mbc1_memory;
            mem->cartridge->teardown = &teardown_mbc1_memory;
            break;
        case CART_MBC3:
        case CART_MBC3_RAM:
        case CART_MBC3_RAM_BAT:
        case CART_MBC3_TIMER_BAT:
        case CART_MBC3_TIMER_RAM_BAT:
            mem->read_rom = &read_mbc3_rom_into_mem;
            mem->cartridge->n_regions = 3;
            mem->cartridge->regions = mbc3_mem_map;
            mem->cartridge->initialize = &initialize_mbc3_memory;
            mem->cartridge->teardown = &teardown_mbc3_memory;
            break;
        case CART_MBC2:
        case CART_MBC2_BAT:
            //break;
        case CART_ROM_RAM:
        case CART_ROM_RAM_BAT:
            //break;
        case CART_MMM01:
        case CART_MMM01_RAM:
        case CART_MMM01_RAM_BAT:
            //break;
        case CART_MBC5:
        case CART_MBC5_RAM:
        case CART_MBC5_RAM_BAT:
        case CART_MBC5_RUM:
        case CART_MBC5_RUM_RAM:
        case CART_MBC5_RUM_RAM_BAT:
            //break;
        case CART_MBC6:
            //break;
        case CART_MBC7_SENS_RUM_RAM_BAT:
            //break;
        case CART_CAMERA:
            //break;
        case CART_TAMA5:
            //break;
        case CART_HUC3:
            //break;
        case CART_HUC1_RAM_BAT:
            //break;
        default:
            printf("Unsupported cartridge type 0x%02x\n", header->cartridge_type);
            return NULL;
    }

    init_done:
    mem->cartridge->state = (mem->cartridge->initialize)(header);
    mem->system->state = (mem->system->initialize)(header);
    mem_map_to_lookup(mem->cartridge->regions, mem->cartridge->n_regions, mem->table);
    mem_map_to_lookup(mem->system->regions, mem->system->n_regions, mem->table);

    return mem;
}

void teardown_memory(MemoryState *mem) {
    free(mem->header);

    mem->cartridge->teardown(mem->cartridge->state);
    mem->system->teardown(mem->system->state);
    free(mem->cartridge);
    free(mem->system);
    free(mem);
}

TimerState *initialize_timer(void) {
    TimerState *timer = malloc(sizeof(TimerState));
    if (timer == NULL) {
        printf("Error allocating timer\n");
        exit(1);
    }
    timer->reg_div = 0xAB;
    timer->reg_tac = 0x00;
    timer->reg_tima = 0x00;
    timer->reg_tma = 0x00;
    timer->timer_enabled = OFF;
    timer->tima_period_mask = 0x3FF;

    return timer;
}

void teardown_timer(TimerState *timer) {
    free(timer);
}

void task_div_timer(GBState *state) {
    state->timer->reg_div++;
}

void task_tima_timer(GBState *state) {
    TimerState *timer = state->timer;
    if (timer->timer_enabled) {
        if (! (state->counter & timer->tima_period_mask)) {
            timer->reg_tima++;

            if (timer->reg_tima == 0) {
                timer->reg_tima = timer->reg_tma;
                printf("timer expired\n");
                REQUEST_INTERRUPT(state, INT_TIMER);
            }
        }
    }
}

DMAState *initialize_dma(void) {
    DMAState *dma = malloc(sizeof(DMAState));
    dma->addr = 0;
    dma->status = DMA_OFF;

    return dma;
}

void teardown_dma(DMAState *dma) {
    free(dma);
}

void task_dma_cycle(GBState *state) {
    DMAState *dma = state->dma;
    if (dma->status == DMA_OFF)
        goto dma_cycle_end;
    else if (dma->status == DMA_INIT) {
        dma->status = DMA_ON;
        goto dma_cycle_end;
    }
    
    BYTE data;
    WORD source_addr = dma->addr;
    WORD dest_addr = 0xFE00 | (source_addr & 0xFF);
    
    data = read_mem(state, source_addr, MEM_SOURCE_DMA);
    write_mem(state, dest_addr, data, MEM_SOURCE_DMA);
    
    dma->addr++;
    if ((dma->addr & 0xFF) == 0xA0)
        dma->status = DMA_OFF;

    dma_cycle_end:
    1;
}