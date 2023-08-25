//
// Created by Romain on 21/08/2023.
//

#include <stdio.h>
#include "core.h"

//#include <pthread.h>
//
//th

void emulator_start(const char * const fn) {
	INFO("GobouEmulator starting", "cartridge file = %s\n", fn);
	
	open_cartridge(fn);
	
	Reset_io_ports(1);
	Reset_cpu(1);
	Reset_ppu(1);
	Reset_dma(1);
	Reset_ctrl(1);
	
	Reset_cartridge(1);
	Reset_mmu(1);
	Reset_mapper(1);
	
	INFO("rom:", " %p\n", memoryMap.rom0+ROM0);
	INFO("vram:", " %p\n", memoryMap.vram+VRAM);
	INFO("xram:", " %p\n", memoryMap.xram+XRAM);
	INFO("wram0:", " %p\n", memoryMap.wram0+WRAM0);
	INFO("wram1:", " %p\n", memoryMap.wram1+WRAM1);
	INFO("oam:", " %p\n", memoryMap.oam+OAM);
	INFO("io:", " %p\n", memoryMap.io+IO);
	INFO("hram:", " %p\nenter:", memoryMap.hram+HRAM);
	//getchar();
	INFO("Gobou Start !", "\n");
	
	//thrd_create(&emu_th, emulator_loop, NULL);
}

int emulator_loop(void * uns) {
	(void)uns;
	u32 i = 0;
	ControllerSync();
	while(!isFrameReady()) {
		//DEBUG("\t", "%u \t A %02X | BC %04X | DE %04X | HL %04X  [%c %c %c %c]  ( LY %hhu 0x%02X) <%02X %02X>\n", i++, A, BC, DE, HL,
		//		  (z) ? 'Z': 'z', (n) ? 'N': 'n', (h) ? 'H': 'h', (c) ? 'C': 'c',
		//	      ioLY, ioLY, ioIF, read_ie());
		cpu_run();
	}
	ppu_mem.frame_ready = 0;
	return 0;
}
