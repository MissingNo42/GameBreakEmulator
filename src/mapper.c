//
// Created by Romain on 20/07/2023.
//

#include <stddef.h>
#include "mapper.h"
#include "units/mmu.h"
#include "cartridge.h"

Mapper mapper;


///////////// No MBC (MBC0)

// init_mbc0 = NULL: xram_enable = has_ram

u8 read_ram_mbc0(u16 addr){
	return memoryMap.xram_enable ? memoryMap.xram[addr]: 0xff;
}

void write_ram_mbc0(u16 addr, u8 value){
	if (memoryMap.xram_enable) memoryMap.xram[addr] = value;
}


///////////// MBC1

void init_mbc1(){
	memoryMap.xram_enable = 0;
	mapper.mbc1.raw = 0;
	mapper.mbc1.rom_mask = 0x1F;
	mapper.mbc1.mapped_ram = cartridgeInfo.ram_bank > 1;
	if (cartridgeInfo.rom_bank > 32) mapper.mbc1.rom_mask = 0x7f;
	else while ((1 << mapper.mbc1.rom_mask) > cartridgeInfo.rom_bank) mapper.mbc1.rom_mask >>= 1;
}

void update_mbc1_bank(){
	if (mapper.mbc1.mode) {
		set_rom0(mapper.mbc1.lrom_bank & mapper.mbc1.rom_mask & 0x60); // (ram+rom) - ram? - rom
		if (mapper.mbc1.mapped_ram) set_xram(mapper.mbc1.ram_bank);
	} else {
		set_rom0(0);
		if (mapper.mbc1.mapped_ram) set_xram(0);
	}
	
	if (mapper.mbc1.rom_bank) {
		if (mapper.mbc1.mapped_ram) set_rom1(mapper.mbc1.rom_bank & mapper.mbc1.rom_mask);
		else set_rom1(mapper.mbc1.lrom_bank & mapper.mbc1.rom_mask);
	} else {
		if (mapper.mbc1.mapped_ram) set_rom1(1);
		else set_rom1(mapper.mbc1.lrom_bank & mapper.mbc1.rom_mask & 0x60 | 1);
	}
}

#define read_ram_mbc1 read_ram_mbc0
#define write_ram_mbc1 write_ram_mbc0

void write_rom0_mbc1(u16 addr, u8 value){
	if (addr & 0x2000) { // 0x2000-0x3fff
		mapper.mbc1.rom_bank = value;
		update_mbc1_bank();
		
	} else if (cartridgeInfo.type.has_ram) memoryMap.xram_enable = (value & 0xA) == 0xA; // 0x000-0x1fff
}

void write_rom1_mbc1(u16 addr, u8 value){
	if (addr & 0x2000) mapper.mbc1.mode = value; // 0x6000-0x7fff
	else mapper.mbc1.ram_bank = value; // 0x4000-0x5fff
	update_mbc1_bank();
}

///////////// MBC Setup

MapperIO supported_mapper[] = {{read_ram_mbc0, write_ram_mbc0, dummy_write, dummy_write, NULL},
							   {read_ram_mbc1, write_ram_mbc1, write_rom0_mbc1, write_rom1_mbc1, init_mbc1}};

void init_mapper(){
	if (cartridgeInfo.type.mapper < sizeof(supported_mapper) / sizeof(MapperIO)) {
		INFO("Load mapper", "MBC%d\n", cartridgeInfo.type.mapper);
		mapper.io = supported_mapper[cartridgeInfo.type.mapper];
	} else {
		CRITICAL("Unsupported mapper", "MBC%d\n", cartridgeInfo.type.mapper);
		mapper.io = supported_mapper[0]; // mbc0 fallback or error??
	}
	if(mapper.io.init) mapper.io.init();
}
