//
// Created by Romain on 03/09/2023.
//

#ifndef GAMEBREAKEMULATOR_MBC5_H
#define GAMEBREAKEMULATOR_MBC5_H


#include "../mmu.h"


static inline void init_mbc5(){
	memoryMap.xram_enable = 0;
	mapper.data.mbc5.raw = 0;
	mapper.data.mbc5.mapped_ram = cartridgeInfo.ram_bank > 1;
}


#define read_ram_mbc5 read_ram_mbc1
#define write_ram_mbc5 write_ram_mbc1


static inline void write_rom0_mbc5(u16 addr, u8 value){
	if (addr & 0x2000) { // 0x2000-0x3fff
		if (addr & 0x1000) mapper.data.mbc5.hrom_bank = value & 1;
		else mapper.data.mbc5.lrom_bank = value;
		//TODO Mask
		set_rom1(mapper.data.mbc5.rom_bank);
	} else if (cartridgeInfo.type.has_ram) memoryMap.xram_enable = (value & 0xF) == 0xA; // 0x000-0x1fff
}


static inline void write_rom1_mbc5(u16 addr, u8 value){
	//CRITICAL("MBC5 rom0", "%04X = %02X\n", addr, value);
	if (addr & 0x2000); // 0x6000-0x7fff
	else if (cartridgeInfo.type.has_rumble) { // 0x4000-0x5fff
		mapper.data.mbc5.rumble = value >> 3;
		mapper.data.mbc5.ram_bank = value & 0x7;
	}
	else mapper.data.mbc5.ram_bank = value;
	if (mapper.data.mbc5.mapped_ram) set_xram(mapper.data.mbc5.ram_bank);
}


#endif //GAMEBREAKEMULATOR_MBC5_H
