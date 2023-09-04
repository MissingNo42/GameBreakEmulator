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
	
	INFO("rom:", " %p\n", memoryMap.rom0+ROM0);
	INFO("vram:", " %p\n", memoryMap.vram+VRAM);
	INFO("xram:", " %p\n", memoryMap.xram+XRAM);
	INFO("wram0:", " %p\n", memoryMap.wram0+WRAM0);
	INFO("wram1:", " %p\n", memoryMap.wram1+WRAM1);
	INFO("oam:", " %p\n", memoryMap.oam+OAM);
	INFO("io:", " %p\n", memoryMap.io+IO);
	INFO("hram:", " %p\nenter:", memoryMap.hram+HRAM);
	//getchar();
	INFO("GameBreak Start !", "\n");
	
	//thrd_create(&emu_th, emulator_loop, NULL);
}
u16 SPX=0;
int emulator_loop(void * uns) {
	(void)uns;
	static u32 I = 0;
	u32 i = 0;
	ControllerSync();
	u8 m = 0;
	static u8 k = 0;
	
	
	
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
	
	
	while(!isFrameReady()){// || !memoryMap.bootrom_unmapped ) {
		if (memoryMap.bootrom_unmapped && !k) {
			k++;
			//for (u8 j = 0; j < 0x80; j++) INFO("IO", "FF%02X = %02X\n", j, direct_read_io(IO|j));
			//CRITICAL("", "A %02X | BC %04X | DE %04X | HL %04X | SP %04X | PC %04X [%c %c %c %c] %02X - %04X %02X\n",
			//      A, BC, DE, HL, SP, PC,
			//      (z) ? 'Z' : 'z', (n) ? 'N' : 'n', (h) ? 'H' : 'h', (c) ? 'C' : 'c', read_ie(), timer.wdiv, ioDIV);
		}
		//if (PC > 0x4250 && PC < 0x4270) CRITICAL("PC4", "\n");
		//if (PC > 0xC250 && PC < 0xC270) CRITICAL("PCC", "\n");
		//if (OPCODE == 0xCD) m++;
		//else if (m < 2) m = 0;
		if (memoryMap.bootrom_unmapped) {
			if (SPX != SP) {
				//CRITICAL("SP", "%04X to %04X\n", SPX, SP);
				SPX = SP;
			}
			//if (!k) {
			//	ioLY = 144;
			//	ppu_mem.dots = 456 - 191;
			//	k=1;
			//}
			//LogInst();
			//INFO("\t",
			//      "%u : %u \t A %02X | BC %04X | DE %04X | HL %04X | SP %04X | PC %04X [%c %c %c %c]  ( LY %hhu 0x%02X) <%02X %02X> (%02X %02X %02X %02X)\n",
			//      I++, i++, A, BC, DE, HL, SP, PC,
			//      (z) ? 'Z' : 'z', (n) ? 'N' : 'n', (h) ? 'H' : 'h', (c) ? 'C' : 'c',
			//      ioLY, ioLY, ioIF, read_ie(), ioDIV, ioTIMA, ioTMA, ioLCDC);
			//if (HL == 0xA100 || HL == 0xA101 || OPERAND == 0xA100 || OPERAND == 0xA101)
			//	CRITICAL("BUG", "%04X %04X, (%04X %02X), %02X, %02X\n", HL, OPERAND, last_addr, last_value, direct_read(0xA100), direct_read(0xA101));
			//if (!(i % 20)) CRITICAL("Wait...", "\n");
			//DEBUG("PPU", "%02X %02X %02X %02X %02X\n", ioLCDC, ioSCX, ioSCY, ioWX, ioWY);
		} else I = 0;
		
		cpu_run();
	}
	//INFO("PPU", "%02X %02X %02X %02X %02X\n", ioLCDC, ioSCX, ioSCY, ioWX, ioWY);
	//if (memoryMap.bootrom_unmapped) CRITICAL("FRAME", "\n");
	
	ppu_mem.frame_ready = 0;
	return 0;
}
