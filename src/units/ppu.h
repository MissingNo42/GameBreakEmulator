//
// Created by Romain on 21/07/2023.
//

#ifndef GOBOUEMU_PPU_H
#define GOBOUEMU_PPU_H

////////////////////////  Includes  ///////////////////////////

#include "../types.h"
#include "../io_ports.h"


////////////////////////   Macros   ///////////////////////////

#define inWindow(x) ((x) == (x) && ioLY) //TODO
#define PPU_MODE (ioSTAT & 3)


////////////////////////    Types   ///////////////////////////

Struct {
	u8 X, Y, tile, attr;
} ObjAttribute;

Struct {
	u8 color: 4, palette: 3, bg_priority: 1, priority: 6;
} Pixel;

Struct {
	u8 selected_obj[10];           // selected obj at the end of mode 2
	u8 current_oam: 6;             // oam iterator (mode 2)
	u8 selected_num: 4;            // number of selected obj (max 10)
	u8 stat_line: 2;               // interrupt occurs only on front edge
	u8 X,                          // current X on the scanline
	
	ctile_index,
	ctile_low[8], ctile_high[8];
	
	s16 dots;
	u8 shifting: 3;
	
} PPU_Mem;


//////////////////////  Declarations  /////////////////////////

extern PPU_Mem ppu_mem;


////////////////////////   Methods   //////////////////////////

void STAT_changed();

void ppu_step(u8 cycles);
void ppu_reset();

/////////////////////  Registrations  /////////////////////////

Reset(ppu) {
	ppu_reset();
}

SaveSize(ppu, sizeof (ppu_mem))

Save(ppu) {
	save_obj(ppu_mem);
}

Load(ppu) {
	load_obj(ppu_mem);
}

#endif //GOBOUEMU_PPU_H
