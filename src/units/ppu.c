//
// Created by Romain on 21/07/2023.
//

#include "../types.h"
#include "ppu.h"
#include "mmu.h"
#include "../io_ram.h"
#include "../cartridge.h"

static ObjAttribute selected_obj[10];
static u8 selected_num;
static u8 X, ctile_index, ctile_low[8], ctile_high[8];

void select_ojb(){
	selected_num = 0;
	u8 sz = ioLCDC2 ? 16: 8;
	
	for(u16 obj = 0; obj < 40 && selected_num < 10; obj++){
		ObjAttribute oa = ((ObjAttribute *)memoryMap.oam)[obj];
		oa.Y -= 16;
		if (oa.Y <= ioLY && ioLY < oa.Y + sz) selected_obj[selected_num++] =  oa;
	}
	
	if (!cartridgeInfo.GBC_MODE) { // sort by priority: X, OAM idx -> sort by X & keep OAM order for same X
		for (u8 idx = selected_num - 1; idx > 0; idx--) {
			u8 maxX = selected_obj[0].X, max = 0;
			for (u8 m = 1; m <= idx; m++) if (selected_obj[m].X >= maxX) maxX = selected_obj[m].X, max = m;
			
			ObjAttribute tmp = selected_obj[max];
			selected_obj[max] = selected_obj[idx];
			selected_obj[idx] = tmp;
		}
	} // else: OAM idx (done)
}

static inline void get_tile(){
//5: w enable
//6: w tm
//4: bg+w data
//3: bg tm
	u8 W = inWindow(X);
	u16 addr = (ioLCDC3 && !W
			||  ioLCDC6 && W) ? 0x9C00: 0x9800;
	u8 cX, cY;//TODO opti =
	if (W) {
		cX = X;
		cY = ioLY;
	}
	else {
		cX = (ioSCX + X);
		cY = (ioLY + ioSCY);
	}
	
	ctile_index = read_vram(addr + (cX >> 3) + ((cY & 0xF8) << 2));
}

static inline void get_tile_low(){
	u16 addr;
	if (ioLCDC4) { // 8000
		addr = 0x8000 | (ctile_index << 4);
	} else { // 8800
		ctile_index ^= 0x80;
		addr = 0x8800 | (ctile_index << 4);
	}
	for (u8 i = 0; i < 8; i++) {
		ctile_low[i] = read_vram(addr | (i << 1));
	}
}

static inline void get_tile_high(){
	u16 addr;
	if (ioLCDC4) { // 8000
		addr = 0x8001 | (ctile_index << 4);
	} else { // 8800
		ctile_index ^= 0x80;
		addr = 0x8801 | (ctile_index << 4);
	}
	for (u8 i = 0; i < 8; i++) {
		ctile_high[i] = read_vram(addr | (i << 1));
	}
}
