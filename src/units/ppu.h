//
// Created by Romain on 21/07/2023.
//

#ifndef GBEMU_PPU_H
#define GBEMU_PPU_H

////////////////////////  Includes  ///////////////////////////

#include "../types.h"
#include "../io_ports.h"


////////////////////////   Macros   ///////////////////////////

#define inWindow(x) ((x) == ioWX - 7)
#define PPU_MODE (ioSTAT & 3)
#define isFrameReady() ppu_mem.frame_ready


////////////////////////    Types   ///////////////////////////

#ifdef IS_LITTLE_ENDIAN
Struct {
	u8 Y, X, tile;
	union {
		struct {u8 gbc_pal: 3, gbc_vram: 1, dmg_pal: 1, h_flip: 1, v_flip: 1, bg_priority: 1;};
		u8 attr;
	};
} ObjAttribute;
#else
Struct {
	u8 X, Y, tile;
	union {
		struct {u8 bg_priority: 1, v_flip: 1, h_flip: 1, dmg_pal: 1, gbc_vram: 1, gbc_pal: 2;};
		u8 attr;
	};
} ObjAttribute;
#endif

Union {
	struct { u16 color: 2, palette: 3, bg_priority: 1, priority: 10; };
	u16 raw;
} Pixel;

Struct {
	union {
		struct { u8 r, g, b, a; };
		u32 color;
	};
} RGBPixel;

typedef Pixel Scanline[160];
typedef RGBPixel RGBScanline[160];
typedef RGBScanline * Screen;

Struct {
	Scanline bg, obj;
	u8 selected_obj_x[10];         // selected obj screen X coord -> timing purpose 11 - min(5, (x + SCX) % 8) | 255 - WX
	u8 selected_obj[10];           // selected obj at the end of mode 2
	u8 current_oam: 6;             // oam iterator (mode 2)
	u8 selected_num: 4;            // number of selected obj (max 10)
	u8 stat_line: 2;               // interrupt occurs only on front edge
	u8 X,                          // current X on the scanline
	WLY,                           // LY for the Window
	ctile_index,
	ctile_low[8], ctile_high[8];
	
	s16 dots;
	u8 shifting: 3;
	u8 frame_ready: 1;
	
	u8 loaded: 1;
	u16 sleep;
	
	u8 wy_condition: 1,
	window_enable: 1,
	window_visible: 1;
} PPU_Mem;


//////////////////////  Declarations  /////////////////////////

extern PPU_Mem ppu_mem;
extern Screen screen;


////////////////////////   Methods   //////////////////////////

void STAT_changed();

void ppu_run(u8 cycles);
void ppu_reset();
inline void ppu_set_screen(Screen s) { screen = s; }

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

#endif //GBEMU_PPU_H
