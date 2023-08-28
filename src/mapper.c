//
// Created by Romain on 20/07/2023.
//

////////////////////////  Includes  ///////////////////////////

#include "mapper.h"
#include "units/mmu.h"
#include "cartridge.h"


////////////////////////   Methods   //////////////////////////

/// No MBC (MBC0)

void init_mbc0(){
	memoryMap.xram_enable = cartridgeInfo.type.has_ram;
}

u8 read_ram_mbc0(u16 addr){
	if (memoryMap.xram_enable) return memoryMap.xram[addr];
	else {
		CRITICAL("Read to locked XRAM", "at $%04X = %02X\n", addr, memoryMap.xram[addr]);
		return 0xff;
	}
}


void write_ram_mbc0(u16 addr, u8 value){
	if (memoryMap.xram_enable) memoryMap.xram[addr] = value;
	else CRITICAL("Write to locked XRAM", "at $%04X = %02X (new = %02X)\n", addr, memoryMap.xram[addr], value);
}


/// MBC1

void init_mbc1(){
	memoryMap.xram_enable = 0;
	mapper.data.mbc1.raw = 0;
	mapper.data.mbc1.rom_mask = 0x1F;
	mapper.data.mbc1.mapped_ram = cartridgeInfo.ram_bank > 1;
	if (cartridgeInfo.rom_bank > 32) mapper.data.mbc1.rom_mask = 0x7f;
	else while ((1 << mapper.data.mbc1.rom_mask) > cartridgeInfo.rom_bank) mapper.data.mbc1.rom_mask >>= 1;
}


void update_mbc1_bank(){
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


#define read_ram_mbc1 read_ram_mbc0
#define write_ram_mbc1 write_ram_mbc0


void write_rom0_mbc1(u16 addr, u8 value){
	if (addr & 0x2000) { // 0x2000-0x3fff
		mapper.data.mbc1.rom_bank = value;
		update_mbc1_bank();
		
	} else {
		if (cartridgeInfo.type.has_ram) {
			memoryMap.xram_enable = (value & 0xF) == 0xA; // 0x000-0x1fff
		}
	}
}


void write_rom1_mbc1(u16 addr, u8 value){
	if (addr & 0x2000) mapper.data.mbc1.mode = value; // 0x6000-0x7fff
	else mapper.data.mbc1.ram_bank = value; // 0x4000-0x5fff
	update_mbc1_bank();
}


/// MBC5


void init_mbc5(){
	//CRITICAL("MBC5 Init", "\n");
	memoryMap.xram_enable = 0;
	mapper.data.mbc5.raw = 0;
	mapper.data.mbc5.mapped_ram = cartridgeInfo.ram_bank > 1;
}


#define read_ram_mbc5 read_ram_mbc1
#define write_ram_mbc5 write_ram_mbc1


void write_rom0_mbc5(u16 addr, u8 value){
	//CRITICAL("MBC5 rom0", "%04X = %02X\n", addr, value);
	if (addr & 0x2000) { // 0x2000-0x3fff
		if (addr & 0x1000) mapper.data.mbc5.hrom_bank = value & 1;
		else mapper.data.mbc5.lrom_bank = value;
		set_rom1(mapper.data.mbc5.rom_bank);
	} else if (cartridgeInfo.type.has_ram) memoryMap.xram_enable = (value & 0xF) == 0xA; // 0x000-0x1fff
}


void write_rom1_mbc5(u16 addr, u8 value){
	//CRITICAL("MBC5 rom0", "%04X = %02X\n", addr, value);
	if (addr & 0x2000); // 0x6000-0x7fff
	else if (cartridgeInfo.type.has_rumble) { // 0x4000-0x5fff
		mapper.data.mbc5.rumble = value >> 3;
		mapper.data.mbc5.ram_bank = value & 0x7;
	}
	else mapper.data.mbc5.ram_bank = value;
	if (mapper.data.mbc5.mapped_ram) set_xram(mapper.data.mbc5.ram_bank);
}

/// Global mapper

void mount_mapper(){
	
	if (cartridgeInfo.type.mapper < sizeof(supported_mapper) / sizeof(MapperIO)) {
		INFO("Load mapper", "MBC%d\n", cartridgeInfo.type.mapper);
		mapper.io = supported_mapper[cartridgeInfo.type.mapper];
		
	} else {
		CRITICAL("Unsupported mapper", "MBC%d\n", cartridgeInfo.type.mapper);
		mapper.io = supported_mapper[0]; // mbc0 fallback or error??
	}
}

void init_mapper(){
	mount_mapper();
	if (mapper.io.init) mapper.io.init();
}


//////////////////////  Declarations  /////////////////////////

Mapper mapper;
const MapperIO supported_mapper[] = {{read_ram_mbc0, write_ram_mbc0, dummy_write, dummy_write, init_mbc0},
							         {read_ram_mbc1, write_ram_mbc1, write_rom0_mbc1, write_rom1_mbc1, init_mbc1},
									 {read_ram_mbc0, write_ram_mbc0, dummy_write, dummy_write, init_mbc0},
									 {read_ram_mbc0, write_ram_mbc0, dummy_write, dummy_write, init_mbc0},
									 {read_ram_mbc0, write_ram_mbc0, dummy_write, dummy_write, init_mbc0},
									 {read_ram_mbc5, write_ram_mbc5, write_rom0_mbc5, write_rom1_mbc5, init_mbc5},
};

