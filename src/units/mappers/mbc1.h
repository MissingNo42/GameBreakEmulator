//
// Created by Romain on 03/09/2023.
//

#ifndef GAMEBREAKEMULATOR_MBC1_H
#define GAMEBREAKEMULATOR_MBC1_H


#include "../mmu.h"


static inline void init_mbc1(){
	memoryMap.xram_enable = 0;
	mapper.data.mbc1.raw = 0;
	mapper.data.mbc1.rom_mask = 0x1F;
	mapper.data.mbc1.mapped_ram = cartridgeInfo.ram_bank > 1;
	if (cartridgeInfo.rom_bank > 32) mapper.data.mbc1.rom_mask = 0x7f;
	else while ((1 << mapper.data.mbc1.rom_mask) > cartridgeInfo.rom_bank) mapper.data.mbc1.rom_mask >>= 1;
}


static inline void update_mbc1_bank(){
	if (mapper.data.mbc1.mode) {
		set_rom0(mapper.data.mbc1.lrom_bank & mapper.data.mbc1.rom_mask & 0x60); // (ram+rom) - ram? - rom
		if (mapper.data.mbc1.mapped_ram) set_xram(mapper.data.mbc1.ram_bank);
	} else {
		set_rom0(0);
		if (mapper.data.mbc1.mapped_ram) set_xram(0);
	}
	
	if (mapper.data.mbc1.rom_bank) {
		if (mapper.data.mbc1.mapped_ram) set_rom1(mapper.data.mbc1.rom_bank & mapper.data.mbc1.rom_mask);
		else set_rom1(mapper.data.mbc1.lrom_bank & mapper.data.mbc1.rom_mask);
	} else {
		if (mapper.data.mbc1.mapped_ram) set_rom1(1);
		else set_rom1((mapper.data.mbc1.lrom_bank & mapper.data.mbc1.rom_mask & 0x60) | 1);
	}
}

static inline void write_rom0_mbc1(u16 addr, u8 value){
	if (addr & 0x2000) { // 0x2000-0x3fff
		mapper.data.mbc1.rom_bank = value;
		update_mbc1_bank();
		
	} else {
		if (cartridgeInfo.type.has_ram) {
			memoryMap.xram_enable = (value & 0xF) == 0xA; // 0x000-0x1fff
		}
	}
}

static inline void write_rom1_mbc1(u16 addr, u8 value){
	if (addr & 0x2000) mapper.data.mbc1.mode = value; // 0x6000-0x7fff
	else mapper.data.mbc1.ram_bank = value; // 0x4000-0x5fff
	update_mbc1_bank();
}

#define read_ram_mbc1 read_ram_mbc0
#define write_ram_mbc1 write_ram_mbc0

#endif //GAMEBREAKEMULATOR_MBC1_H
