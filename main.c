#include <stdio.h>
#include "src/units/mmu.h"
#include "src/cartridge.h"
#include "src/mapper.h"
#include "src/units/cpu.h"

#define tr "../tloz.gb"
//#define tr "../cpu_instrs.gb"
#include <malloc.h>

int main() {
	INFO("GobouEmulator starting", "cartridge = %s\n", tr);
	load_header(tr);
	init_memory();
	INFO("Memory loaded", "@ %p\n", Memory);
	init_mapper();
	
	INFO("Load cartridge...", "\n");
	load_cartridge(tr);
	
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
	
	INFO("Gobou Start !", "\n");
	cpu_init();
	for (u32 i = 0; i < (4194304<<8); i++) {
		//DEBUG("I", "%d\n", i);
		cpu_run();
	}
	INFO("Gobou Exit !", "\n");
	
	free(Memory);
	return 0;
}
