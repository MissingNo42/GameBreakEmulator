//
// Created by Romain on 21/07/2023.
//

#ifndef GOBOUEMU_PPU_H
#define GOBOUEMU_PPU_H

#include "../types.h"
#include "../io_ram.h"


#define inWindow(x) ((x) == (x) && ioLY) //TODO
#define PPU_MODE (ioSTAT & 3)

Struct {
	u8 X, Y, tile, attr;
} ObjAttribute;

Struct {
	u8 selected_obj[10];           // selected obj at the end of mode 2
	u8 current_oam: 6;             // oam iterator (mode 2)
	u8 selected_num: 4;            // number of selected obj (max 10)
	u8 stat_line: 2;               // interrupt occurs only on front edge
	u8 X,                          // current X on the scanline
	
	ctile_index,
	ctile_low[8], ctile_high[8];
	
	s16 dots;
	
} PPU_Mem;

extern PPU_Mem ppu_mem;

void inline STAT_changed() {
	ppu_mem.stat_line <<= 1;
	if (
			((ioSTAT & 0x44) == 0x44) || // LYC=LY
			((ioSTAT & 0x23) == 0x22) || // MODE 2
			((ioSTAT & 0x13) == 0x11) || // MODE 1
			((ioSTAT & 0x0b) == 0x08))   // MODE 0
		ppu_mem.stat_line |= 1;
	if (ppu_mem.stat_line == 1) ioIF |= INT_LCD_STAT;
}

void ppu_step(u8 cycles);

#endif //GOBOUEMU_PPU_H
