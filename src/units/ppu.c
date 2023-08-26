//
// Created by Romain on 21/07/2023.
//

#include "../types.h"
#include "ppu.h"
#include "mmu.h"
#include "dma.h"

PPU_Mem ppu_mem;
Screen screen;

static inline void mode_0_start();
static inline void mode_1_start();
static inline void mode_2_start();
static inline void mode_3_start();

#define set_mode_0() ioSTAT &= 0xFC                  // From mode (3)
#define set_mode_1() ioSTAT = (ioSTAT & 0xFC) | 1    // From mode (0)
#define set_mode_2() ioSTAT = (ioSTAT & 0xFC) | 2    // From mode (0, 1)
#define set_mode_3() ioSTAT |= 0x03                  // From mode (2)

#define check_lyc() ioSTAT = (ioSTAT & 0xFB) | ((ioLY == ioLYC) << 2)

const RGBPixel DMG_Color[] = {{.color = 0xFFFFFFFF}, {.color = 0xFFAAAAAA}, {.color = 0xFF555555}, {.color = 0xFF000000}};


void STAT_changed() {
	// TODO STAT-BUG + stat_line go down between M2 & LYC
	ppu_mem.stat_line <<= 1;
	//u8 S;
	//
	//if (!GBC) {
	//	S = ioSTAT;     // Save
	//	ioSTAT |= 0xF0; // Corrupt
	//}
	
	Check: if (
			((ioSTAT & 0x44) == 0x44) || // LYC=LY
			((ioSTAT & 0x23) == 0x22) || // MODE 2
			((ioSTAT & 0x13) == 0x11) || // MODE 1
			((ioSTAT & 0x0b) == 0x08))   // MODE 0
		ppu_mem.stat_line |= 1;
	if (ppu_mem.stat_line == 1) ioIF |= INT_LCD_STAT;
	
	//if (!GBC) {
	//	ioSTAT = S; // Restore
	//	goto Check;
	//}
}

	static u32 ccc = 0;  ////////////////////////
	
// --------------------------------------------------------------
// -------------------------- MODE 0 ----------------------------
// --------------------------------------------------------------

#pragma region Mode0

static inline void mode_0_start() {
	Unlock_ppu_oam();
	Unlock_ppu_ram();
	set_mode_0();
	STAT_changed();
	if (hdma.hblank) hdma.current_hblank = 1; // avoid starting mid hblank
}

static inline void mode_0_end() {
	ioLY++;
	check_lyc();
	if (ioLY == 144) mode_1_start();
	else mode_2_start();
	STAT_changed();
}

static inline u8 mode_0_step(u8 cycles) {
	if (hdma.current_hblank) hdma_run(cycles);
	ppu_mem.dots -= (s16)cycles;
	
	if (ppu_mem.dots <= 0) {
		cycles = -ppu_mem.dots;
		mode_0_end();
	} else cycles = 0;
	return cycles;
}

#pragma endregion

// --------------------------------------------------------------
// -------------------------- MODE 1 ----------------------------
// --------------------------------------------------------------

#pragma region Mode1


static inline void mode_1_start() {
	set_mode_1();
	ioIF |= INT_VBLANK; // Request VBLANK interrupt
	ppu_mem.dots = 456;
	ppu_mem.frame_ready = 1;
	//TODO Frame ready
}

static inline void mode_1_end() {
	if (ioLY == 153) { // Mode 1 End
		//ERROR("PPU Cycle 70224?: ", "%u %hhu\n", ccc, ioLY);
		ioLY = 0;
		ppu_mem.WLY = 0;
		ccc = 0;
		mode_2_start();
	} else { // Mode 1 Continue
		ioLY++;
		ppu_mem.dots = 456;
	}
	check_lyc();

	STAT_changed();
}

static inline u8 mode_1_step(u8 cycles) {
	ppu_mem.dots -= (s16)cycles;
	
	if (ppu_mem.dots <= 0) {
		cycles = -ppu_mem.dots;
		mode_1_end();
	} else cycles = 0;
	return cycles;
}

#pragma endregion

// --------------------------------------------------------------
// -------------------------- MODE 2 ----------------------------
// --------------------------------------------------------------

#pragma region Mode2

static inline void mode_2_start() {
	ppu_mem.selected_num = 0;
	ppu_mem.current_oam = 0;
	
	ppu_mem.wy_condition = ioWY <= ioLY;
	
	Lock_ppu_oam();
	set_mode_2();
}

static inline void mode_2_end() {
	if (!GBC && ppu_mem.selected_num) { // sort by priority: X, OAM idx -> sort by X & keep OAM order for same X
		for (u8 idx = ppu_mem.selected_num - 1; idx > 0; idx--) {
			u8 maxX = ((ObjAttribute *)(memoryMap.oam + OAM))[ppu_mem.selected_obj[0]].X, max = 0;
			for (u8 m = 1; m <= idx; m++)
				if (((ObjAttribute *)(memoryMap.oam + OAM))[ppu_mem.selected_obj[m]].X >= maxX)
					maxX = ((ObjAttribute *)(memoryMap.oam + OAM))[ppu_mem.selected_obj[m]].X, max = m;
			
			u8 tmp = ppu_mem.selected_obj[max];
			ppu_mem.selected_obj[max] = ppu_mem.selected_obj[idx];
			ppu_mem.selected_obj[idx] = tmp;
		}
	} // else: OAM idx (done)
	mode_3_start();
}

static inline u8 mode_2_step(u8 cycles) {
	//cycles >>= 1; // 1 oam memory_read = 2 cycles
	
	u8 sz = ioLCDC2 ? 16 : 8;
	u8 next = ppu_mem.current_oam + (cycles >> 1);
	u8 cycles_left = 0;
	
	if (next > 40) cycles_left = next - 40, next = 40;
	
	if (memoryMap.dma_lock) ppu_mem.current_oam = next; // dma -> memory_read FF -> nothing on-screen = selected
	
	else for (; ppu_mem.current_oam < next && ppu_mem.selected_num < 10; ppu_mem.current_oam++) {
		ObjAttribute oa = direct_read_oam_block(ppu_mem.current_oam);
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

#pragma endregion

// --------------------------------------------------------------
// -------------------------- MODE 3 ----------------------------
// --------------------------------------------------------------

#pragma region Mode3


static inline void get_tile(s32 X){
	u16 addr = ((ioLCDC3 && !ppu_mem.window_visible)
			||  (ioLCDC6 && ppu_mem.window_visible)) ? 0x9C00: 0x9800;
	
	u8 cX = X, cY = ioLY;
	if (ppu_mem.window_visible) {
		cX -= ioWX - 7;
		cY = ppu_mem.WLY;
	} else {
		cX += ioSCX;
		cY = ioLY + ioSCY;
	}
	
	cX >>= 3; // pix coord -> tile coord
	cY >>= 3; // pix coord -> tile coord
	
	ppu_mem.ctile_index = direct_read_vram(addr + cX + (cY << 5)); //(memoryMap.dma_lock) ? 0xFF:  -> LY0 blink due to bad mode3 timing
}

static inline void load_tile_data(u16f addr) {
	u16f addrh = addr | 1;
	
	for (u8f i = 0; i < 8; i++) {
		ppu_mem.ctile_low[i] = direct_read_vram(addr | (i << 1));
		ppu_mem.ctile_high[i] = direct_read_vram(addrh | (i << 1));
	}
}

static inline void get_tile_data(){
	u16f addr;
	if (ioLCDC4) { // 8000
		addr = 0x8000 | (ppu_mem.ctile_index << 4);
	} else { // 8800
		ppu_mem.ctile_index ^= 0x80;
		addr = 0x8800 + (ppu_mem.ctile_index << 4);
	}
	
	load_tile_data(addr);
}

static inline void get_tile_data_obj(u8 tile){
	load_tile_data(0x8000 | (tile << 4));
}


static inline void prerender_bg() {
	s32 X = 0, Y = (ioLY + (ioSCY & 7)) & 7;
	
	s32 bg_limit = (ppu_mem.wy_condition && ppu_mem.window_enable && (ioWX - 7 < 160)) ? ioWX - 7: 160;
	
	s32 offset = (ppu_mem.shifting ^ 7) + 1;
	get_tile(0);
	get_tile_data();
	
	loop: for (; X < bg_limit; X++) { // BG (first loop) | Window (second loop)
		if (!offset) {
			get_tile(X);
			get_tile_data();
			offset = 8;
		}
		
		offset--;
		ppu_mem.bg[X].color = (((ppu_mem.ctile_high[Y] >> offset) & 1) << 1) | ((ppu_mem.ctile_low[Y] >> offset) & 1);
	}
	
	if (X < 160) {
		ppu_mem.window_visible = 1;
		bg_limit = 160;
		if (ioWX & 0xf8) offset = 0;
		else {
			offset = ioWX + 1;
			get_tile(X);
			get_tile_data();
		}
		goto loop;
	}
	
	if (ppu_mem.window_visible) ppu_mem.WLY++;
}

static inline void prerender_obj() {
	
	for (s32 X = 0; X < 160; X++) ppu_mem.obj[X].raw = 0; // Clear
	
	for (s32 obj = ppu_mem.selected_num - 1; obj >= 0; obj--) { // Iter in reverse order to deal with priority more easily
		ObjAttribute oa = read_oam_block_mode3(ppu_mem.selected_obj[obj]);
		s32 Y = ioLY - oa.Y + 16;
		
		if (oa.v_flip) Y ^= 0xF; // switch the to tiles
		if (ioLCDC2) get_tile_data_obj((Y & 8) ? (oa.tile | 1): (oa.tile & 0xFE));
		else get_tile_data_obj(oa.tile); // assume Y is valid (never > 8 / > 16)
		Y &= 7;
		
		s32 X = oa.X - 8;
		for (s32 offset = 0; !(offset & 8) && X < 160; X++, offset++) {
			s32 f_offset = (oa.h_flip) ? offset : offset ^ 7;
			s32 color = (((ppu_mem.ctile_high[Y] >> f_offset) & 1) << 1) | ((ppu_mem.ctile_low[Y] >> f_offset) & 1);
			
			if (color) { // color over priority-lower color OR color over alpha
				ppu_mem.obj[X].bg_priority = oa.bg_priority;
				ppu_mem.obj[X].priority = oa.X;
				ppu_mem.obj[X].palette = oa.dmg_pal;
				
				ppu_mem.obj[X].color = color;
			}
		}
	}
}

static inline void render_blend() {
	
	for (s32 X = 0; X < 160; X++) {
		if (ioLCDC0 && ((ppu_mem.obj[X].bg_priority && !ppu_mem.bg[X].color) || !ppu_mem.obj[X].color)) {
			screen[ioLY][X] = DMG_Color[(ioBGP >> (ppu_mem.bg[X].color << 1)) & 3];
		} else if (ioLCDC1) {
			screen[ioLY][X] = DMG_Color[((ppu_mem.obj[X].palette ? ioOBP1: ioOBP0) >> (ppu_mem.obj[X].color << 1)) & 3];
		}
	}
}

static inline void mode_3_start() {
	ppu_mem.dots = 376;
	ppu_mem.X = 0;
	ppu_mem.sleep = ppu_mem.shifting = ioSCX & 7;
	ppu_mem.loaded = 0;
	ppu_mem.current_oam = 0;
	ppu_mem.window_visible = 0;
	ppu_mem.window_enable = ioLCDC5 >> 5;
	
	prerender_bg();
	prerender_obj();
	
	ppu_mem.sleep += 8 * ppu_mem.selected_num + 174 + 6 * ppu_mem.window_visible;
	
	Lock_ppu_ram();
	set_mode_3();
	STAT_changed();
}


static inline u8 mode_3_step(u8 cycles){ // 289-172
	
	do {
		ppu_mem.dots--;
	} while (--cycles && --ppu_mem.sleep);
	
	if (!ppu_mem.sleep) {
		render_blend();
		mode_0_start();
		//INFO("M3 T: ", "%d\n", 376-ppu_mem.dots);
	}
	
	return cycles;
}

#pragma endregion


// --------------------------------------------------------------
// ---------------------------- PPU -----------------------------
// --------------------------------------------------------------

void ppu_reset() {
	for (u16 i = 0; i < (u16) sizeof(ppu_mem); i++) ((u8*)&ppu_mem)[i] = 0x00; //memset0
	
	if (ioLCDC7) {
		ioLY = 0;
		check_lyc();
		mode_2_start();
		INFO("PPU Start", "Mode 2\n");
	} else { // LCD Off
		ioLY = 0;
		set_mode_1(); // mode_1_start();
		Unlock_ppu_oam();
		Unlock_ppu_ram();
		INFO("PPU Stop", "Mode 1\n");
	}
	STAT_changed(); // needed/required ??   may interrupt on invalid switch off (outside vblank)
}

void ppu_step(u8 cycles) { // mult of 2
	if (!ioLCDC7) return;
	
	ccc += cycles;
	//DEBUG("PPU Mode", "%hhu: %hhu %hhu\n", PPU_MODE, ppu_mem.current_oam, ppu_mem.selected_num);
	do {
		switch (PPU_MODE) {
			case 3: {
				cycles = mode_3_step(cycles);
				break;
			}
			case 2: {
				cycles = mode_2_step(cycles);
				break;
			}
			case 0: {
				cycles = mode_0_step(cycles);
				break;
			}
			case 1: {
				cycles = mode_1_step(cycles);
				break;
			}
		}
	} while (cycles);
}



