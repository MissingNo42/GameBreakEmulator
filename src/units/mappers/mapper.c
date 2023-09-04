//
// Created by Romain on 20/07/2023.
//

////////////////////////  Includes  ///////////////////////////

#include "mapper.h"
#include "../mmu.h"
#include "../../cartridge.h"


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

#include "mbc1.h"
#include "mbc2.h"
#include "mbc3.h"
#include "mbc5.h"

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
const MapperIO supported_mapper[] = {{read_ram_mbc0, write_ram_mbc0, dummy_write,     dummy_write,     init_mbc0},
							         {read_ram_mbc1, write_ram_mbc1, write_rom0_mbc1, write_rom1_mbc1, init_mbc1},
									 {read_ram_mbc2, write_ram_mbc2, write_rom0_mbc2, write_rom1_mbc2, init_mbc2},
									 {read_ram_mbc3, write_ram_mbc3, write_rom0_mbc3, write_rom1_mbc3, init_mbc3},
									 {read_ram_mbc0, write_ram_mbc0, dummy_write,     dummy_write,     init_mbc0},
									 {read_ram_mbc5, write_ram_mbc5, write_rom0_mbc5, write_rom1_mbc5, init_mbc5},
};

