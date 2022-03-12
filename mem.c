#include "base.h"
#include "mem.h"
#include <stdio.h>
#include <string.h>
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

READ_FUNC(_debug_read_mem) {
    DebugMemState *debug = (DebugMemState *)state->mem->system->state;
    return debug->mem[rel_addr];
}

WRITE_FUNC(_debug_write_mem) {
    DebugMemState *debug = (DebugMemState *)state->mem->system->state;
    debug->mem[rel_addr] = data;
    return 1;
}

/* BEGIN ioreg read/write functions */
READ_FUNC(_read_p1) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    BYTE action_buttons = state->sdl->action_buttons;
    BYTE direction_buttons = state->sdl->direction_buttons;
    BYTE button_select = state->sdl->button_select;

    BYTE result = 0xFF;

    if (joypad_action_selected(button_select))
        result &= action_buttons;
    
    if (joypad_direction_selected(button_select))
        result &= direction_buttons;

    sys_mem->ioregs->p1 = result;

    return result;
}
WRITE_FUNC(_write_p1) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    // Only writable bits are 4 and 5
    data = (data & 0x30) | 0xCF;
    state->sdl->button_select = data;
    sys_mem->ioregs->p1 &= data;
    return 1;
}
READ_FUNC(_read_div) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    sys_mem->ioregs->div = state->timer->reg_div;
    return sys_mem->ioregs->div;
}
WRITE_FUNC(_write_div) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    state->timer->reg_div = 0;
    sys_mem->ioregs->div = 0;
    return 1;    
}
READ_FUNC(_read_tima) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    sys_mem->ioregs->tima = state->timer->reg_tima;
    return sys_mem->ioregs->tima;
}
WRITE_FUNC(_write_tima) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    state->timer->reg_tima = data;
    sys_mem->ioregs->tima = data;
    return 1;
}
READ_FUNC(_read_tma) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    sys_mem->ioregs->tma = state->timer->reg_tma;
    return sys_mem->ioregs->tma;
}
WRITE_FUNC(_write_tma) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    state->timer->reg_tma = data;
    sys_mem->ioregs->tma = data;
    return 1;
}
READ_FUNC(_read_tac) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    return sys_mem->ioregs->tac;
}
WRITE_FUNC(_write_tac) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    data &= 0x07;

    if (data & 0x4) 
        state->timer->timer_enabled = ON;
    else
        state->timer->timer_enabled = OFF;

    state->timer->reg_tac = data;
    switch (data & 0x3) {
        case 0:
            state->timer->tima_period_cycles = _00;
            break;
        case 1:
            state->timer->tima_period_cycles = _01;
            break;
        case 2:
            state->timer->tima_period_cycles = _10;
            break;
        case 3:
            state->timer->tima_period_cycles = _11;
            break;
    }
    
    // FIXME bit 2 enables timer, bit 0-1 selects timer frequency
    sys_mem->ioregs->tac = data;
    return 1;
}
READ_FUNC(_read_if) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    // Upper 3 bits always read as 1
    sys_mem->ioregs->if_ = 0xE0 | state->cpu->int_flag;
    return sys_mem->ioregs->if_;
}
WRITE_FUNC(_write_if) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    state->cpu->int_flag = data & 0x1F;
    sys_mem->ioregs->if_ = data;
    return 1;    
}
READ_FUNC(_read_ie) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    sys_mem->ioregs->ie_ = state->cpu->int_enable;
    return sys_mem->ioregs->ie_;
}
WRITE_FUNC(_write_ie) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    state->cpu->int_enable = data;
    sys_mem->ioregs->ie_ = data;
    return 1;    
}

READ_FUNC(_read_dma) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    return sys_mem->ioregs->dma;
}
WRITE_FUNC(_write_dma) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    state->dma->addr = ((WORD)data) << 8;
    state->dma->status = DMA_ON;
    sys_mem->ioregs->dma = data;
    return 1;    
}
READ_FUNC(_read_lcdc) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    // FIXME 
    return sys_mem->ioregs->lcdc;
}
WRITE_FUNC(_write_lcdc) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    LCDControl *lcdc = &state->ppu->lcdc;
    // FIXME: all bits of lcdc control aspects of the PPU
    lcdc->lcd_enable = bit_7(data) ? ON : OFF;
    lcdc->win_map_area = bit_6(data) ? MAP_AREA1 : MAP_AREA0;
    lcdc->window_enable = bit_5(data) ? ON : OFF;
    lcdc->bg_win_data_area = bit_4(data) ? DATA_AREA1 : DATA_AREA0;
    lcdc->bg_map_area = bit_3(data) ? MAP_AREA1 : MAP_AREA0;
    lcdc->obj_size = bit_2(data) ? OBJ_8x16 : OBJ_8x8;
    lcdc->obj_enable = bit_1(data) ? ON : OFF;
    lcdc->bg_window_enable = bit_0(data) ? ON : OFF;


    sys_mem->ioregs->lcdc = data;
    return 1;    
}
READ_FUNC(_read_stat) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    PPUState *ppu = state->ppu;
    BYTE result = 0;
    result |= ((ppu->stat.lyc_ly_interrupt == ON) ? 0x40 : 0x00);
    result |= ((ppu->stat.mode_2_interrupt == ON) ? 0x20 : 0x00);
    result |= ((ppu->stat.mode_1_interrupt == ON) ? 0x10 : 0x00);
    result |= ((ppu->stat.mode_0_interrupt == ON) ? 0x08 : 0x00);
    ppu->stat.lyc_ly_equal = (ppu->misc.ly == ppu->misc.lyc) ? ON : OFF;
    result |= ((ppu->stat.lyc_ly_equal == ON) ? 0x04 : 0x00);
    result |= ((BYTE)ppu->stat.mode & 0x3);
    sys_mem->ioregs->stat = result;
    return result;
}
WRITE_FUNC(_write_stat) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    PPUState *ppu = state->ppu;
    // Only bits 3-6 are writable
    data &= 0x78;
    ppu->stat.lyc_ly_interrupt = bit_6(data) ? ON : OFF;
    ppu->stat.mode_2_interrupt = bit_5(data) ? ON : OFF;
    ppu->stat.mode_1_interrupt = bit_4(data) ? ON : OFF;
    ppu->stat.mode_0_interrupt = bit_3(data) ? ON : OFF;
    sys_mem->ioregs->stat &= (data | 0x7);
    return 1;    
}
READ_FUNC(_read_scy) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    PPUState *ppu = state->ppu;
    sys_mem->ioregs->scy = ppu->misc.scy;
    return sys_mem->ioregs->scy;
}
WRITE_FUNC(_write_scy) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    PPUState *ppu = state->ppu;
    ppu->misc.scy = data;
    sys_mem->ioregs->scy = data;
    return 1;
}
READ_FUNC(_read_scx) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    PPUState *ppu = state->ppu;
    sys_mem->ioregs->scx = ppu->misc.scx;
    return sys_mem->ioregs->scx;
}
WRITE_FUNC(_write_scx) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    PPUState *ppu = state->ppu;
    ppu->misc.scx = data;
    sys_mem->ioregs->scx = data;
    return 1;
}
READ_FUNC(_read_ly) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    PPUState *ppu = state->ppu;
    // FIXME: current horizontal line (0-153, 144-154==VBlank)
    sys_mem->ioregs->ly = ppu->misc.ly;
    return sys_mem->ioregs->ly;
}
READ_FUNC(_read_lyc) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    PPUState *ppu = state->ppu;
    
    sys_mem->ioregs->lyc = ppu->misc.lyc;
    return sys_mem->ioregs->lyc;
}
WRITE_FUNC(_write_lyc) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    PPUState *ppu = state->ppu;
    ppu->misc.lyc = data;
    sys_mem->ioregs->lyc = data;
    return 1;
}
READ_FUNC(_read_wy) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    PPUState *ppu = state->ppu;
    sys_mem->ioregs->wy = ppu->misc.wy;
    return sys_mem->ioregs->wy;
}
WRITE_FUNC(_write_wy) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    PPUState *ppu = state->ppu;
    // FIXME: Sets window position
    ppu->misc.wy = data;
    sys_mem->ioregs->wy = data;
    return 1;
}
READ_FUNC(_read_wx) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    PPUState *ppu = state->ppu;
    sys_mem->ioregs->wx = ppu->misc.wx;
    return sys_mem->ioregs->wx;
}
WRITE_FUNC(_write_wx) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    PPUState *ppu = state->ppu;
    // FIXME: Sets window position
    ppu->misc.wx = data;
    sys_mem->ioregs->wx = data;
    return 1;
}
READ_FUNC(_read_bgp) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    PPUState *ppu = state->ppu;
    // FIXME: returns color palette to BG/window
    sys_mem->ioregs->bgp = ppu->misc.bgp;
    return sys_mem->ioregs->bgp;
}
WRITE_FUNC(_write_bgp) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    PPUState *ppu = state->ppu;
    // FIXME: Sets color palette for bg/window
    ppu->misc.bgp = data;
    sys_mem->ioregs->bgp = data;
    return 1;
}
READ_FUNC(_read_obp0) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    PPUState *ppu = state->ppu;
    // FIXME: returns color palette to obj palette 0
    sys_mem->ioregs->obp0 = ppu->misc.obp0;
    return sys_mem->ioregs->obp0;
}
WRITE_FUNC(_write_obp0) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    PPUState *ppu = state->ppu;
    // FIXME: sets color palette for objs 0
    data &= 0xFC;
    ppu->misc.obp0 = data;
    sys_mem->ioregs->obp0 = data;
    return 1;
}
READ_FUNC(_read_obp1) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    PPUState *ppu = state->ppu;
    // FIXME: returns color palette to obj palette 1
    sys_mem->ioregs->obp1 = ppu->misc.obp1;
    return sys_mem->ioregs->obp1;
}
WRITE_FUNC(_write_obp1) {
    SysMemState *sys_mem = (SysMemState *)state->mem->system->state;
    PPUState *ppu = state->ppu;
    // FIXME: sets color palette for objs 0
    //data &= 0xFC;
    ppu->misc.obp1 = data;
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
    if (sys_mem->bootrom_mapped && (data & 0x1)) {
        sys_mem->bootrom_mapped = 0;
        sys_mem->ioregs->boot = 0x1;
        state->mem->system->regions[0].flags |= MEM_UNMAPPED; 
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
    SysMemState *sys_mem;
    sys_mem = (SysMemState *)state->mem->system->state;
    return &sys_mem->bootrom[rel_addr];
}
GET_PTR_FUNC(_sys_ptr_vram) {
    SysMemState *sys_mem;
    sys_mem = (SysMemState *)state->mem->system->state;
    return &sys_mem->vram[rel_addr];
}
GET_PTR_FUNC(_sys_ptr_wram) {
    SysMemState *sys_mem;
    sys_mem = (SysMemState *)state->mem->system->state;
    return &sys_mem->wram[rel_addr];
}
GET_PTR_FUNC(_sys_ptr_hiram) {
    SysMemState *sys_mem;
    sys_mem = (SysMemState *)state->mem->system->state;
    return &sys_mem->hram[rel_addr];
}
GET_PTR_FUNC(_sys_ptr_oam_table) {
    SysMemState *sys_mem;
    sys_mem = (SysMemState *)state->mem->system->state;
    return &sys_mem->oam_table[rel_addr];
}
GET_PTR_FUNC(_basic_ptr) {
    BasicCartState *basic_mem;
    basic_mem = (BasicCartState *)state->mem->cartridge->state;
    return &basic_mem->rom[rel_addr];
}
GET_PTR_FUNC(_debug_ptr) {
    DebugMemState *debug_mem;
    debug_mem = (DebugMemState *)state->mem->system->state;
    return &debug_mem->mem[rel_addr];
}
/*
MemoryRegion mbc1_mem_map[] = {
    { // 0001 1111 1111 1111
        .base=0x0000,
        .end=0x1FFF,
        .len=0x2000,
        .flags=0,
        .read=&_mbc1_read_rom_base,
        .write=&_mbc1_ram_enable
    },
    { // 0011 1111 1111 1111
        .base=0x2000,
        .end=0x3FFF,
        .len=0x2000,
        .flags=0,
        .read=&_mbc1_read_rom_base,
        .write=&_mbc1_rom_bank_num
    },
    { // 0101 1111 1111 1111
        .base=0x4000,
        .end=0x5FFF,
        .len=0x2000,
        .flags=0,
        .read=&_mbc1_read_rom_1,
        .write=&_mbc1_ram_or_upperrom
    },
    { // 0111 1111 1111 1111
        .base=0x6000,
        .end=0x7FFF,
        .len=0x2000,
        .flags=0,
        .read=&_mbc1_read_rom_1,
        .write=&_mbc1_bank_mode_select
    },
    { // 1011 1111 1111 1111
        .base=0xA000,
        .end=0xBFFF,
        .len=0x2000,
        .flags=0,
        .read=&_mbc1_read_ram_bank,
        .write=&_mbc1_write_ram_bank
    }
};

MemoryRegion mbc3_mem_map[] = {
    {
        .base=0x0000,
        .end=0x1FFF,
        .len=0x2000,
        .flags=0,
        .read=&_mbc3_read_rom_base,
        .write=&_mbc3_ram_timer_enable
    },
    {
        .base=0x2000,
        .end=0x3FFF,
        .len=0x2000,
        .flags=0,
        .read=&_mbc3_read_rom_base,
        .write=&_mbc3_rom_bank_num
    },
    {
        .base=0x4000,
        .end=0x5FFF,
        .len=0x2000,
        .flags=0,
        .read=&_mbc3_read_rom_1,
        .write=&_mbc3_ram_or_rtc_sel
    },
    {
        .base=0x6000,
        .end=0x7FFF,
        .len=0x2000,
        .flags=0,
        .read=&_mbc3_read_rom_1,
        .write=&_mbc3_latch_clock
    },
    {
        .base=0xA000,
        .end=0xBFFF,
        .len=0x2000,
        .flags=0,
        .read=&_mbc3_read_ram_or_rtc,
        .write=&_mbc3_write_ram_or_rtc
    }
};
*/

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
    MemoryRegion *source = NULL;
    Memmap_t *sys_map = state->mem->system;
    Memmap_t *cart_map = state->mem->cartridge;
    MemoryRegion *sys_regions = sys_map->regions;
    MemoryRegion *cart_regions = cart_map->regions;

    for (int i = 0; i < sys_map->n_regions; i++) {
        if (sys_regions[i].flags & MEM_UNMAPPED)
            continue;
        
        if (addr >= sys_regions[i].base && addr <= sys_regions[i].end) {
            source = &sys_regions[i];
            goto found_mem_region;
        }
    }

    for (int i = 0; i < cart_map->n_regions; i++) {
        if (cart_regions[i].flags & MEM_UNMAPPED)
            continue;

        if (addr >= cart_regions[i].base && addr <= cart_regions[i].end) {
            source = &cart_regions[i];
            goto found_mem_region;
        } 

    }

    found_mem_region:
    return source;

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
    ioregs->lyc = 0x00;
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
    
    return sys_mem;
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

DebugMemState *initialize_debug_memory(void) {
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

MemoryState *initialize_memory(CartridgeHeader *header) {
    MemoryState *mem = malloc(sizeof(MemoryState));
    if (mem == NULL) {
        printf("Error allocating memory state\n");
        exit(1);
    }
    //mem->mode = flag;
    mem->cartridge = malloc(sizeof(Memmap_t));
    mem->system = malloc(sizeof(Memmap_t));
    if (mem->cartridge == NULL || mem->system == NULL) {
        printf("Error allocating memmap_t\n");
        exit(1);
    }

    /* DEBUG condition */
    if (header == NULL) {
        mem->cartridge->n_regions = 0;
        
        mem->system->n_regions = 1;
        mem->system->regions = debug_mem_map;
        mem->system->initialize = &initialize_debug_memory;
        mem->system->teardown = &teardown_debug_memory;
        mem->system->state = initialize_debug_memory();

        goto init_done;
    }

    if (flag == BASIC) {
        mem->cartridge->n_regions = 1;
        mem->cartridge->regions = basic_mem_map;
        mem->cartridge->initialize = &initialize_basic_memory;
        mem->cartridge->teardown = &teardown_basic_memory;
        // FIXME could do this later, as we save the fn pointer
        // Also, may need a loadrom function that handles loading
        // the cartridge
        mem->cartridge->state = initialize_basic_memory();

        mem->system->n_regions = 9;
        mem->system->regions = system_mem_map;
        mem->system->initialize = &initialize_sys_memory;
        mem->system->teardown = &teardown_sys_memory;
        mem->system->state = initialize_sys_memory();
    } else if (flag == DEBUG) {
        mem->cartridge->n_regions = 0;
        
        mem->system->n_regions = 1;
        mem->system->regions = debug_mem_map;
        mem->system->initialize = &initialize_debug_memory;
        mem->system->teardown = &teardown_debug_memory;
        mem->system->state = initialize_debug_memory();
    } else {
        printf("Unsupported MemInitFlag %d\n", flag);
        exit(1);
    }

    init_done:
    return mem;
}

void teardown_memory(MemoryState *mem) {

    if (mem->cartridge->n_regions > 0) {
        mem->cartridge->teardown(mem->cartridge->state);
        free(mem->cartridge);
    }

    mem->system->teardown(mem->system->state);
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
    timer->tima_period_cycles = _00;

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
        if ((state->counter % timer->tima_period_cycles) == 0) {
            timer->reg_tima++;

            if (timer->reg_tima == 0) {
                timer->reg_tima = timer->reg_tma;
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
    
    printf("In DMA, addr = %04x\n", dma->addr);

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

int read_basic_rom_into_mem(GBState *state, FILE *fp) {
    int status = 0;
    int n_read;
    n_read = fread(((BasicCartState *)state->mem->cartridge->state)->rom, 1, 0x8000, fp);
    if (n_read != 0x8000) {
        printf("Error: read %04x rather than 0x8000\n", n_read);
        status = 1;
    }

    return status;
}

int read_rom_into_mem(GBState *state, FILE *fp) {
    int status = 0;
    switch (state->mem->mode) {
        case BASIC:
            status = read_basic_rom_into_mem(state, fp);
            break;
        case DEBUG:
        case MBC1:
        case MBC3:
        default:
            printf("Unsupported cartridge type at this time\n");
            status = 1;
    }

    return status;
}