//
// Created by Romain on 21/08/2023.
//

#include <stdio.h>
#include "core.h"

//#include <pthread.h>
//
//th

void emulator_start(const char * const fn) {
	INFO("GameBreakEmulator starting", "cartridge file = %s\n", fn);
	
	open_cartridge(fn);
	
	ResetEmulator(1);
	
	INFO("rom0:", " %p\n", memoryMap.rom0+ROM0);
	INFO("rom1:", " %p\n", memoryMap.rom1+ROM0);
	INFO("xram:", " %p\n", memoryMap.xram+XRAM);
	INFO("vram:", " %p\n", memoryMap.vram+VRAM);
	INFO("wram0:", " %p\n", memoryMap.wram0+WRAM0);
	INFO("wram1:", " %p\n", memoryMap.wram1+WRAM1);
	INFO("oam:", " %p\n", memoryMap.oam+OAM);
	INFO("io:", " %p\n", memoryMap.io+IO);
	INFO("hram:", " %p\nenter:", memoryMap.hram+HRAM);
	//getchar();
	INFO("GameBreak Start !", "\n");
	
	//thrd_create(&emu_th, emulator_loop, NULL);
}


int emulator_loop(void * uns) {
	(void)uns;
	ControllerSync();
	
	
	//static u8 kr[0x80];
	//int shw = 0;
	//for (int w = 0; w < 0x80; w++) {
	//	if (memoryMap.cram[w] != kr[w]) {
	//		shw = 1;
	//	}
	//	kr[w] = memoryMap.cram[w];
	//}
	//if (shw) {
	//	ERROR("PAL BG ", " ");
	//	for (int w = 0; w < 0x40; w++) printf("%02X ", memoryMap.cram[w]);
	//	printf("\n");
	//	ERROR("PAL OBJ", " ");
	//	for (int w = 0x40; w < 0x80; w++) printf("%02X ", memoryMap.cram[w]);
	//	CRITICAL("", "\n\n");
	//}
	//while (!isFrameReady()) {
		cpu_run();
	//}
	ppu_mem.frame_ready = 0;
	//INFO("PPU", "%02X %02X %02X %02X %02X\n", ioLCDC, ioSCX, ioSCY, ioWX, ioWY);
	//if (memoryMap.bootrom_unmapped) CRITICAL("FRAME", "\n");
	
	return 0;
}
