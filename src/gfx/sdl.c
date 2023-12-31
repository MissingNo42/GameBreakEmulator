//
// Created by Romain on 21/08/2023.
//

#include "sdl.h"

SDL_Window * window = NULL;
SDL_Renderer * renderer = NULL;
SDL_Texture * LCD = NULL, * DebugMemory = NULL, * DebugTileMaps = NULL,
* DebugAttrMaps = NULL, *DebugColor = NULL, * DebugTileData = NULL;



s32 GfxSetup() {
	if (SDL_Init(SDL_INIT_VIDEO)) {
		CRITICAL("SDL Init failed", "%s\n", SDL_GetError());
		return 0;
		
	}else {
		if (SDL_CreateWindowAndRenderer(S_WIDTH, S_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE, &window, &renderer)) {
			CRITICAL("SDL Init window failed", "%s\n", SDL_GetError());
			GfxQuit();
			return 0;
		
		} else {
			SDL_SetWindowTitle(window, "GBC - Emulator");
			LCD           = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, 160, 144);
			DebugTileMaps = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, 256, 512);
			DebugAttrMaps = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, 256, 512);
			DebugTileData = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, 256, 192);
			DebugColor    = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, 64, 16);
			DebugMemory   = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, 256, 226);
			
			if (!LCD || !DebugMemory || !DebugTileMaps || !DebugAttrMaps || !DebugColor || !DebugTileData) {
				CRITICAL("SDL Init texture failed", "%s\n", SDL_GetError());
				GfxQuit();
				return 0;
				
			} else {
				INFO("Load supported game-controller", "%d\n", SDL_GameControllerAddMappingsFromFile("../assets/SDL_CtrlDB.txt"));
			}
		}
	}
	return 1;
}

void GfxQuit() {
	if (LCD) SDL_DestroyTexture(LCD);
	if (DebugTileMaps) SDL_DestroyTexture(DebugTileMaps);
	if (DebugAttrMaps) SDL_DestroyTexture(DebugAttrMaps);
	if (DebugTileData) SDL_DestroyTexture(DebugTileData);
	if (DebugColor) SDL_DestroyTexture(DebugColor);
	if (DebugMemory) SDL_DestroyTexture(DebugMemory);
	if (window) SDL_DestroyWindow(window);
	SDL_Quit();
}

void GfxRender_Memory() {
	s32 i = 0;
	
	u8 * px;
	int pitch;
	SDL_LockTexture(DebugMemory, NULL, (void **)&px, &pitch);
	
	for (; i < 0x4000; i++, px += 4) {
		u8 cc = direct_read_rom0(i);
		px[0] = cc;
		px[1] = px[2] = 0;
		
	}
	
	for (; i < 0x8000; i++, px += 4) {
		u8 cc = direct_read_rom1(i);
		px[1] = px[0] = cc;
		px[2] = 0;
	}
	
	for (; i < 0xa000; i++, px += 4) {
		u8 cc = direct_read_vram(i);
		px[1] = cc;
		px[0] = px[2] = 0;
	}
	
	for (; i < 0xc000; i++, px += 4) {
		u8 cc = memoryMap.xram[i];
		px[2] = px[1] = cc;
		px[0] = 0;
	}
	
	for (; i < 0xd000; i++, px += 4) {
		u8 cc = direct_read_wram0(i);
		px[2] = cc;
		px[1] = px[0] = 0;
	}
	
	for (; i < 0xe000; i++, px += 4) {
		u8 cc = direct_read_wram1(i);
		px[2] = cc;
		px[1] = px[0] = 0;
	}
	
	for (i = 0xfe00; i < 0xfea0; i++, px += 4) {
		u8 cc = direct_read_oam(i);
		px[1] = px[2] = cc;
		px[0] = 0;
	}
	
	for (; i < 0xff00; i++, px += 4) {
		px[1] = px[2] = px[0] = 0;
	}
	
	for (; i < 0xff80; i++, px += 4) {
		u8 cc = direct_read_io(i);
		px[1] = px[2] = cc;
		px[0] = 0;
	}
	
	for (; i < 0x10000; i++, px += 4) {
		u8 cc = direct_read_hram(i);
		px[1] = px[2] = cc;
		px[0] = 0;
	}
	
	SDL_UnlockTexture(DebugMemory);
	SDL_Rect  rc = {256, 0, 256, 226};
	SDL_RenderCopy(renderer, DebugMemory, NULL, &rc);
}

static inline u32 min(u32 a, u32 b){ return (a < b) ? a: b; }

void GfxRender_VRAM() {
	s32 i, x = 0, y = 0;
	SDL_Color gcl[4] = {{0x10, 0x10, 0x10, 0},
						{0x40, 0x40, 0x40, 0},
						{0x90, 0x90, 0x90, 0},
						{0xff, 0xff, 0xff, 0}};
	SDL_Color * px;
	int pitch;
	
	/////////////////////////////////
	//////////// TILE ///////////////
	/////////////////////////////////
	
	SDL_LockTexture(DebugTileMaps, NULL, (void **)&px, &pitch);
	
	for (i = 0x9800; i < 0x9FFF; i++, x += 8, y = x >> 8 << 3) {
		u16 tile = direct_read_vram0(i);
		if (!ioLCDC4) tile = 0x80 + (tile ^ 0x80);
		tile <<= 4;
		
		for (s32 yy = y; yy < y + 8; yy++) {
			u8 cA = direct_read_vram0(0x8000 + tile + ((yy - y) << 1));
			u8 cB = direct_read_vram0(0x8001 + tile + ((yy - y) << 1));
			for (s32 xx = x & 0xff; xx < (x & 0xff) + 8; xx++) {
				px[(yy << 8) + xx] = gcl[((cB >> (7 - xx + x) & 1) << 1) | (cA >> (7 - xx + x) & 1)];
			}
		}
		
	}
	
	SDL_UnlockTexture(DebugTileMaps);
	
	SDL_Rect  rc = {512, 0, 256, 512};
	SDL_RenderCopy(renderer, DebugTileMaps, NULL, &rc);
	
	/////////////////////////////////
	//////////// ATTR ///////////////
	/////////////////////////////////
	
	if (GBC) {
		x = 0, y = 0;
		SDL_LockTexture(DebugAttrMaps, NULL, (void **) &px, &pitch);
		
		for (i = 0x9800; i < 0x9FFF; i++, x += 8, y = x >> 8 << 3) {
			u16 tile = direct_read_vram1(i);
			TileAttribute attr;
			attr.attr = tile;
			if (!ioLCDC4) tile = 0x80 + (tile ^ 0x80);
			tile <<= 4;
			for (s32 yy = y; yy < y + 8; yy++) {
				u8 cA = 15;
				u8 cB = (yy < y + 4) ? 0: 255;
				for (s32 xx = x & 0xff; xx < (x & 0xff) + 8; xx++) {
					px[(yy << 8) + xx] = *(SDL_Color *)&GBC_Color[memoryMap.bg_color[attr.gbc_pal][((cB >> (7 - xx + x) & 1) << 1) | (cA >> (7 - xx + x) & 1)]];
				}
			}
			
		}
		
		SDL_UnlockTexture(DebugAttrMaps);
		
		SDL_Rect ra = {768, 0, 256, 512};
		SDL_RenderCopy(renderer, DebugAttrMaps, NULL, &ra);
	}
	
	SDL_SetRenderDrawColor(renderer, 0, 0xb2, 0x7f, 0);
	
	SDL_RenderDrawLine(renderer, 768, 0, 768, 512);
	
	////////////////////////////////////////
	////////////////////////////////////////
	
	SDL_LockTexture(DebugTileData, NULL, (void **)&px, &pitch);
	x = 0, y = 0;
	for (i = 0x0; i < 384; i++, x += 8, y += x >> 8 << 3) {
		u16 tile = i;
		x &= 0xff;
		
		for (s32 yy = y; yy < y + 8; yy++) {
			u8 cA = direct_read_vram0(0x8000 + (tile<<4) + ((yy - y) << 1));
			u8 cB = direct_read_vram0(0x8001 + (tile<<4) + ((yy - y) << 1));
			for (s32 xx = x; xx < x + 8; xx++) {
				px[(yy << 8) + xx] = gcl[((cB >> (7 - xx + x) & 1) << 1) | (cA >> (7 - xx + x) & 1)];
			}
		}
	}
	
	if (GBC) {
		x = 0, y = 0;
		for (i = 0x0; i < 384; i++, x += 8, y += x >> 8 << 3) {
			u16 tile = i;
			x &= 0xff;
	
			for (s32 yy = y; yy < y + 8; yy++) {
				u8 cA = direct_read_vram1(0x8000 + (tile << 4) + ((yy - y) << 1));
				u8 cB = direct_read_vram1(0x8001 + (tile << 4) + ((yy - y) << 1));
				for (s32 xx = x; xx < x + 8; xx++) {
					px[((yy + 96) << 8) + xx] = gcl[((cB >> (7 - xx + x) & 1) << 1) | (cA >> (7 - xx + x) & 1)];
				}
			}
		}
	}
	
	
	SDL_UnlockTexture(DebugTileData);
	
	SDL_Rect  rd = {256, 256, 256, 192};
	SDL_RenderCopy(renderer, DebugTileData, NULL, &rd);
	SDL_SetRenderDrawColor(renderer, 0, 0xb2, 0x7f, 0);
	
	SDL_RenderDrawLine(renderer, 256, 256+96, 512, 256+96);
	
	////////////////////////////////////////
	////////////////////////////////////////
	
	u8 wrapX = ioSCX + 160 >= 256;
	u8 wrapY = ioSCY + 144 >= 256;
	
	SDL_Rect  bg0 = {ioSCX, ioSCY, wrapX ? 256 - ioSCX : 160, wrapY ? 256 - ioSCY : 144};
	SDL_Rect  bg1 = {0, ioSCY, 160 - bg0.w, wrapY ? 256 - ioSCY : 144};         // X
	SDL_Rect  bg2 = {ioSCX, 0, wrapX ? 256 - ioSCX : 160, 144 - (256 - ioSCY)}; // Y
	SDL_Rect  bg3 = {0, 0, 160 - bg0.w, 144 - (256 - ioSCY)};                   // XY
	
	if (ioLCDC3) {
		bg0.y += 256;
		bg1.y += 256;
		bg2.y += 256;
		bg3.y += 256;
	}
	bg0.x += 512;
	bg1.x += 512;
	bg2.x += 512;
	bg3.x += 512;
	
	SDL_SetRenderDrawColor(renderer, 0xff, 0, 0, 0);
	
	SDL_RenderDrawRect(renderer, &bg0);
	if (wrapX) SDL_RenderDrawRect(renderer, &bg1);
	if (wrapY) SDL_RenderDrawRect(renderer, &bg2);
	if (wrapX && wrapY) SDL_RenderDrawRect(renderer, &bg3);
	
	SDL_RenderDrawLine(renderer, 512, 256, 512 + 256, 256);
	
	SDL_Rect  wd = {512, ioLCDC6 ? 256: 0, 160 - ioWX + 7, 144 - ioWY};
	SDL_SetRenderDrawColor(renderer, 0x80, 0x80, 0, 0);
	if (ioLCDC5) SDL_RenderDrawRect(renderer, &wd);
	
	if (ioLCDC1) for (u8 o = 0; o < 40; o++){
		ObjAttribute oa = direct_read_oam_block(o);
		int d = ioSCX + oa.X - 8;
		wrapX = d >= 256 || d < 0;
		d = ioSCY + oa.Y - 16;
		wrapY = d >= 256 || d < 0;
		SDL_Rect ref = wrapX ? (wrapY ? bg3:bg1): (wrapY? bg2:bg0);
		SDL_Rect ob = {ref.x + oa.X - 8, ref.y + oa.Y - 16, 8, ioLCDC2 ? 16: 8};
		if (oa.attr.bg_priority) SDL_SetRenderDrawColor(renderer, 0, 0x80, 0x80, 0);
		else SDL_SetRenderDrawColor(renderer, 0x80, 0, 0x80, 0);
		SDL_RenderDrawRect(renderer, &ob);
		
		/*
		u16 tile = oa.tile;
		u8 k = 0;
		tile <<= 4;
		
		draw:
		for (s32 yy = ob.y; yy < ob.y + 8; yy++) {
			u8 cA = direct_read_vram(0x8000 + tile + ((yy - ob.y) << 1));
			u8 cB = direct_read_vram(0x8001 + tile + ((yy - ob.y) << 1));
			for (s32 xx = ob.x; xx < ob.x + 8; xx++) {
				px[(yy << 8) + xx] = gcl[((cB >> (7 - xx + ob.x) & 1) << 1) | (cA >> (7 - xx + ob.x) & 1)];
			}
		}
		
		if (ioLCDC2 && !k) {
			tile += 16;
			ob.y += 8;
			k = 1;
			goto draw;
		}*/
	}
	
	static u8 fr = 10;
	SDL_Rect lcd = {100, 256, fr, 10};
	fr++;
	if (fr > 10)fr=1;
	SDL_SetRenderDrawColor(renderer, ioLCDC7 ? 0: 0xff, ioLCDC7 ? 0xff: 0, 0, 0);
	SDL_RenderDrawRect(renderer, &lcd); // red = ppu off | green = ppu on
	
	
	
	SDL_LockTexture(DebugColor, NULL, (void **)&px, &pitch);
	x = 0, y = 0;
	for (i = 0x0; i < 16; i++, x += 8, y += x >> 6 << 3) {
		x &= 0x3f;
		
		for (s32 yy = y; yy < y + 8; yy++) {
			u8 cA = 0x0F;
			u8 cB = (yy < y + 4) ? 0: 255;
			for (s32 xx = x; xx < x + 8; xx++) {
				px[(yy << 6) + xx] = *(SDL_Color *)&GBC_Color[memoryMap.color[i][((cB >> (7 - xx + x) & 1) << 1) | (cA >> (7 - xx + x) & 1)]];
			}
		}
	}
	
	SDL_UnlockTexture(DebugColor);
	
	SDL_Rect  rv = {256, 256+192, 64, 16};
	SDL_RenderCopy(renderer, DebugColor, NULL, &rv);
}
