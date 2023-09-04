//
// Created by Romain on 03/09/2023.
//

#ifndef GAMEBREAKEMULATOR_MBC2_H
#define GAMEBREAKEMULATOR_MBC2_H


#include "../mmu.h"


static inline void init_mbc2(){
	memoryMap.xram_enable = 0;
}


static inline u8 read_ram_mbc2(u16 addr){
	if (memoryMap.xram_enable) {
		u8 r = memoryMap.xram[addr & 0xff];
		if (addr & 0x100) r >>= 4;
		return r;
	} else {
		CRITICAL("Read to locked XRAM", "at $%04X = %02X\n", addr, memoryMap.xram[addr & 0xff]);
		return 0xff;
	}
}


static inline void write_ram_mbc2(u16 addr, u8 value){
	if (memoryMap.xram_enable) {
		u8 r = memoryMap.xram[addr & 0xff];
		if (addr & 0x100) {
			r &= 0x0f;
			r |= (value & 0x0f) << 4;
		} else {
			r &= 0xf0;
			r |= value & 0x0f;
		}
		memoryMap.xram[addr & 0xff] = r;
	}
	else CRITICAL("Write to locked XRAM", "at $%04X = %02X (new = %02X)\n", addr, memoryMap.xram[addr & 0xff], value & 0xf);
}


static inline void write_rom0_mbc2(u16 addr, u8 value){
	
	if (addr & 0x100) {
		u8f bk = value & 0xf;
		if (!bk) bk = 1;
		set_rom1(bk);
	} else {
		memoryMap.xram_enable = value == 0xA;
	}
}

#define write_rom1_mbc2 dummy_write


#endif //GAMEBREAKEMULATOR_MBC2_H
