#include <stdio.h>
#include <malloc.h>
#include "src/cartridge.h"
#include "src/mapper.h"
#include "src/io_ports.h"
#include "src/units/mmu.h"
#include "src/units/cpu.h"
#include "src/units/dma.h"
#include "src/units/ppu.h"

#define tr "../tloz.gb"
//#define tr "../cpu_instrs.gb"

int main() {
	INFO("GobouEmulator starting", "cartridge file = %s\n", tr);
	
	open_cartridge(tr);
	
	Reset_io_ports(1);
	Reset_cpu(1);
	Reset_ppu(1);
	Reset_dma(1);
	
	Reset_cartridge(1);
	Reset_mmu(1);
	Reset_mapper(1);
	
	/*
	if (open_cartridge(tr)
	&& load_header()
	&& init_memory()) {
		INFO("Memory loaded", "@ %p\n", Memory);
		init_mapper();
		
		INFO("Load cartridge...", "\n");
		load_cartridge();
	}*/
	
	/* approx mapper1 test
	u8 t[ROM1];
	u8 r=1;
	for (u16 i = 0; i <= ROM1; i++) {
		t[i]=read(i+ROM1);
	}
	write(0x2000, 255);
	for (u16 i = 0; i <= ROM1; i++) {
		r&=t[i]==read(i+ROM1);
	}
	
	DEBUG(">> r", " = %d\n", r);
	
	for (u16 i = XRAM; i < XRAM+0x2000; i++) {
		if (0xff!=read(i)) {
			ERROR("Ram fault", "%04X %02X\n", i, read(i));
			break;
		}
	}
	
	write(0x0000, 0xAA);
	
	for (u16 i = XRAM; i <= XRAM+0x2000; i++) {
		write(i, 42);
		if (42!=read(i)) ERROR("Ram fault", "%04X %hhu\n", i, read(i));
	}*/
	
	INFO("rom:", " %p\nenter:", memoryMap.rom0+ROM0);
	getchar();
	
	INFO("Gobou Start !", "\n");
	//cpu_init();
	//for (u32 i = 0; i < (4194304<<8); i++) {
	
	for (u32 i = 0; i < (4000000 * 100); i++) {
		//if (i > 47000 && ioLY > 0x88 && ioLY < 0x92)
		//	DEBUG("\t", "%d \t A %02X (z %hhu) | C %02X | E %02X | HL %04X ( LY %hhu 0x%02X)\n", i, A, z, C, E, HL,ioLY, ioLY);
		if (!(i & 0xffff))
			DEBUG("\t", "%d \t A %02X (z %hhu) | C %02X | E %02X | HL %04X ( LY %hhu 0x%02X)\n", i, A, z, C, E, HL,ioLY, ioLY);
		cpu_run();
		if (PC > 0x100) break;
	}
	
	char xx=0;
	for (u32 i = 0; i < 20000000; i++) {
		//if (!(i & 0xffff))
		if (i > 100) xx=1;
		//if (xx)DEBUG("\t", "%d \t A %02X (z %hhu) | C %02X | E %02X | HL %04X ( LY %hhu 0x%02X)\n", i, A, z, C, E, HL,ioLY, ioLY);
		cpu_run();
	}
	INFO("Gobou Exit !", "\n");
	
	free(Memory);
	return 0;
}
