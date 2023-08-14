//
// Created by Romain on 21/07/2023.
//

#include "../types.h"
#include "ppu.h"
#include "mmu.h"
#include "../io_ram.h"
#include "../cartridge.h"

PPU_Mem ppu_mem;

static inline void mode_0_start();
static inline void mode_1_start();
static inline void mode_2_start();
static inline void mode_3_start();

// --------------------------------------------------------------
// -------------------------- MODE 2 ----------------------------
// --------------------------------------------------------------

static inline void mode_2_start() {
	ppu_mem.selected_num = 0;
	ppu_mem.current_oam = 0;
}

static inline void mode_2_end() {
	if (!cartridgeInfo.GBC_MODE && ppu_mem.selected_num) { // sort by priority: X, OAM idx -> sort by X & keep OAM order for same X
		for (u8 idx = ppu_mem.selected_num - 1; idx > 0; idx--) {
			u8 maxX = ((ObjAttribute *)memoryMap.oam)[ppu_mem.selected_obj[0]].X, max = 0;
			for (u8 m = 1; m <= idx; m++)
				if (((ObjAttribute *)memoryMap.oam)[ppu_mem.selected_obj[m]].X >= maxX)
					maxX = ((ObjAttribute *)memoryMap.oam)[ppu_mem.selected_obj[m]].X, max = m;
			
			u8 tmp = ppu_mem.selected_obj[max];
			ppu_mem.selected_obj[max] = ppu_mem.selected_obj[idx];
			ppu_mem.selected_obj[idx] = tmp;
		}
	} // else: OAM idx (done)
	
	mode_3_start();
}

static inline u8 mode_2_step(u8 cycles) {
	//cycles >>= 1; // 1 oam read = 2 cycles
	
	u8 sz = ioLCDC2 ? 16 : 8;
	u8 next = ppu_mem.current_oam + (cycles >> 1);
	u8 cycles_left = 0;
	
	if (next > 40) cycles_left = next - 40, next = 40;
	
	for (; ppu_mem.current_oam < next && ppu_mem.selected_num < 10; ppu_mem.current_oam++) {
		ObjAttribute oa = ((ObjAttribute *) memoryMap.oam)[ppu_mem.current_oam];
		oa.Y -= 16; // padding
		if (oa.Y <= ioLY && ioLY < oa.Y + sz) ppu_mem.selected_obj[ppu_mem.selected_num++] = ppu_mem.current_oam;
	}
	
	if (ppu_mem.current_oam < next) { // -> select_num == 10
		cycles_left += next - ppu_mem.current_oam;
		end:
		mode_2_end();
	} else if (next == 40) goto end;
	
	return cycles_left << 1;
}



// --------------------------------------------------------------
// -------------------------- MODE 3 ----------------------------
// --------------------------------------------------------------

static inline void mode_3_start() {
	ioSTAT |= 3; // Set Mode 3
	STAT_changed();
	ppu_mem.dots = 376;
}

static inline void get_tile(){
//5: w enable
//6: w tm
//4: bg+w data
//3: bg tm
	u8 W = inWindow(ppu_mem.X);
	u16 addr = (ioLCDC3 && !W
			||  ioLCDC6 && W) ? 0x9C00: 0x9800;
	u8 cX, cY;//TODO opti =
	if (W) {
		cX = ppu_mem.X;
		cY = ioLY;
	}
	else {
		cX = (ioSCX + ppu_mem.X);
		cY = (ioLY + ioSCY);
	}
	
	ppu_mem.ctile_index = read_vram(addr + (cX >> 3) + ((cY & 0xF8) << 2));
}

static inline void get_tile_low(){
	u16 addr;
	if (ioLCDC4) { // 8000
		addr = 0x8000 | (ppu_mem.ctile_index << 4);
	} else { // 8800
		ppu_mem.ctile_index ^= 0x80;
		addr = 0x8800 | (ppu_mem.ctile_index << 4);
	}
	for (u8 i = 0; i < 8; i++) {
		ppu_mem.ctile_low[i] = read_vram(addr | (i << 1));
	}
}

static inline void get_tile_high(){
	u16 addr;
	if (ioLCDC4) { // 8000
		addr = 0x8001 | (ppu_mem.ctile_index << 4);
	} else { // 8800
		ppu_mem.ctile_index ^= 0x80;
		addr = 0x8801 | (ppu_mem.ctile_index << 4);
	}
	for (u8 i = 0; i < 8; i++) {
		ppu_mem.ctile_high[i] = read_vram(addr | (i << 1));
	}
}

void ppu_step(u8 cycles) { // mult of 2
	do {
		DEBUG("PPU Mode", "%hhu: %hhu %hhu\n", PPU_MODE, ppu_mem.current_oam, ppu_mem.selected_num);
		switch (PPU_MODE) {
			case 3: {
				//cycles = mode_3_step(cycles);
				break;
			}
			case 2: {
				cycles = mode_2_step(cycles);
				break;
			}
			case 0: {
				ppu_mem.dots -= (s16)cycles;
				
				if (ppu_mem.dots < 0) {
					cycles = -ppu_mem.dots;
					goto end0;
				} else {
					cycles = 0;
					if (ppu_mem.dots == 0) {
						end0:
						ioLY++;
						ioSTAT = ioSTAT & 0xFB | ((ioLY == ioLYC) << 2);
						if (ioLY == 144) {
							ioSTAT |= 1; // Set MODE 1 - VBLANK
							ioIF |= INT_VBLANK; // Request VBLANK interrupt
							//TODO Frame ready
							ppu_mem.dots = 456;
						}
						STAT_changed();
					}
				}
				break;
			}
			case 1: {
				ppu_mem.dots -= (s16)cycles;
				
				if (ppu_mem.dots < 0) {
					cycles = -ppu_mem.dots;
					goto end1;
				} else {
					cycles = 0;
					if (ppu_mem.dots == 0) {
						end1:
						if (ioLY == 153) { // Mode 1 End
							ioLY = 0;
							ioSTAT ^= 3; // Set MODE 2 - OAM Scan
							mode_2_start();
						} else {
							ioLY++;
							ppu_mem.dots = 456;
						}
						ioSTAT = ioSTAT & 0xFB | ((ioLY == ioLYC) << 2);

						STAT_changed();
					}
				}
				break;
			}
		}
	} while (cycles);
	
}



