//
// Created by Romain on 21/08/2023.
//

#include "sdl.h"

SDL_Window * window = NULL;
SDL_Renderer * renderer = NULL;
SDL_Texture * LCD = NULL, * DebugMemory = NULL, * DebugTileMaps = NULL, * DebugTileData = NULL;



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
			DebugTileData = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, 256, 384);
			DebugMemory   = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, 256, 226);
			
			if (!LCD || !DebugMemory || !DebugTileMaps || !DebugTileData) {
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
	if (DebugTileData) SDL_DestroyTexture(DebugTileData);
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
		u8 cc = direct_read_xram(i);
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


void GfxRender_VRAM() {
	s32 i, x = 0, y = 0;
	SDL_Color gcl[4] = {{0xff, 0xff, 0xff, 0}, {0x90, 0x90, 0x90, 0}, {0x40, 0x40, 0x40, 0}, {0x10, 0x10, 0x10, 0}};
	SDL_Color * px;
	int pitch;
	SDL_LockTexture(DebugTileMaps, NULL, (void **)&px, &pitch);
	
	for (i = 0x9800; i < 0x9FFF; i++, x += 8, y = x >> 8 << 3) {
		u16 tile = direct_read_vram(i);
		if (!ioLCDC4) tile = 0x80 + (tile ^ 0x80);
		tile <<= 4;
		
		for (s32 yy = y; yy < y + 8; yy++) {
			u8 cA = direct_read_vram(0x8000 + tile + ((yy - y) << 1));
			u8 cB = direct_read_vram(0x8001 + tile + ((yy - y) << 1));
			for (s32 xx = x & 0xff; xx < (x & 0xff) + 8; xx++) {
				px[(yy << 8) + xx] = gcl[((cB >> (7 - xx + x) & 1) << 1) | (cA >> (7 - xx + x) & 1)];
			}
		}
		
	}
	
	
	SDL_UnlockTexture(DebugTileMaps);
	
	SDL_Rect  rc = {512, 0, 256, 512};
	SDL_RenderCopy(renderer, DebugTileMaps, NULL, &rc);
	
	
	////////////////////////////////////////
	////////////////////////////////////////
	
	SDL_LockTexture(DebugTileData, NULL, (void **)&px, &pitch);
	x = 0, y = 0;
	for (i = 0x0; i < 384; i++, x += 8, y += x >> 8 << 3) {
		u16 tile = i;
		x &= 0xff;
		
		for (s32 yy = y; yy < y + 8; yy++) {
			u8 cA = direct_read_vram(0x8000 + (tile<<4) + ((yy - y) << 1));
			u8 cB = direct_read_vram(0x8001 + (tile<<4) + ((yy - y) << 1));
			for (s32 xx = x; xx < x + 8; xx++) {
				px[(yy << 8) + xx] = gcl[((cB >> (7 - xx + x) & 1) << 1) | (cA >> (7 - xx + x) & 1)];
			}
		}
		
	}
	
	
	SDL_UnlockTexture(DebugTileData);
	
	SDL_Rect  rd = {768, 0, 256, 384};
	SDL_RenderCopy(renderer, DebugTileData, NULL, &rd);
	////////////////////////////////////////
	////////////////////////////////////////
	
	SDL_Rect  bg0 = {512 + ioSCX, (ioLCDC3 ? 512: 0) + ioSCY, 160, 144};
	SDL_SetRenderDrawColor(renderer, 0xff, 0, 0, 0);
	SDL_RenderDrawRect(renderer, &bg0);
	if (bg0.x + bg0.w > 512 + 255) {
		SDL_Rect  bg1 = {512, (ioLCDC3 ? 512: 0) + ioSCY, bg0.x + bg0.w - 512 - 255, 144};
		SDL_RenderDrawRect(renderer, &bg1);
	}
	
	SDL_RenderDrawLine(renderer, 512, 512, 512 + 256, 512);
	
	SDL_Rect  wd = {512, ioLCDC6 ? 512: 0, 160 - ioWX + 7, 144 - ioWY};
	SDL_SetRenderDrawColor(renderer, 0x80, 0x80, 0, 0);
	if (ioLCDC5) SDL_RenderDrawRect(renderer, &wd);
	
	SDL_SetRenderDrawColor(renderer, 0, 0x80, 0x80, 0);
	if (ioLCDC1) for (u8 o = 0; o < 40; o++){
		ObjAttribute oa = direct_read_oam_block(o);
		SDL_Rect ob = {bg0.x + oa.X - 8, bg0.y + oa.Y - 16, 8, ioLCDC2 ? 16: 8};
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
}
