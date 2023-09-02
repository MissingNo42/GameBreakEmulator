//
// Created by Romain on 14/07/2023.
//

////////////////////////  Includes  ///////////////////////////

#include "mmu.h"
#include "../io_ports.h"
#include "cpu.h"
#include <malloc.h>


//////////////////////  Declarations  /////////////////////////

u8 * Memory = NULL;
MemoryMap memoryMap;
u16 last_addr;
u8 last_value;


////////////////////////   Methods   //////////////////////////


u32 get_size() {
	memoryMap.map_size =
			cartridgeInfo.rom_size    // ROM
			+ cartridgeInfo.ram_size  // External RAM
			+ (0x2000u << GBC)        // VRAM (x2 in GBC)
			+ (0x2000u << (GBC << 1)) // WRAM (x4 in GBC)
			+ 0xa0                    // OAM
			+ 0x80;                   // HRAM + IE
			//+ 0x80                  // IO Registers
			
	return memoryMap.map_size;
}

u8 * alloc_memory() {
	if (!memoryMap.map_size) {
		get_size();
		if (Memory) {
			DEBUG("Unknown-size Virtual Memory freeing", "size: ?? | addr = %p\n", Memory);
			free(Memory); // map_size = current mem allocated -> 0 = NULL -> free (/!\ only valid ptr)
		}
		goto alloc;
		
	} else if (!Memory) {
		alloc: Memory = malloc(memoryMap.map_size);
		if (Memory) DEBUG("Virtual Memory allocated", "size: %u | addr = %p\n", memoryMap.map_size, Memory);
		else CRITICAL("Virtual Memory allocation failed", "requested size: %u\n", memoryMap.map_size);
		
	} else {
		DEBUG("Virtual Memory already allocated", "size: %u | addr = %p\n", memoryMap.map_size, Memory);
	}
	
	return Memory;
}


void free_memory(){
	if (Memory) {
		free(Memory);
		Memory = NULL;
	}
}

/// All the memory is alloced in 1 contiguous space (Memory) and is splited in memoryMap
void map_memory(){
	// Const Sector (Mem layout)
	memoryMap.xram_sector = Memory + cartridgeInfo.rom_size;
	memoryMap.vram_sector = memoryMap.xram_sector + cartridgeInfo.ram_size;
	memoryMap.wram_sector = memoryMap.vram_sector + (0x2000 << GBC);
	
	// Bankable mem mapping
	set_rom0(memoryMap.rom0_bank);
	set_rom1(memoryMap.rom1_bank);
	set_vram(memoryMap.vram_bank);
	
	set_xram(memoryMap.xram_bank);
	set_wram(memoryMap.wram_bank);
	
	// PPU Direct access
	memoryMap.vram0 = memoryMap.vram_sector - 0x8000;
	memoryMap.vram1 = memoryMap.vram0 + 0x2000;
	
	// Static mem mapping
	memoryMap.wram0 = memoryMap.wram_sector - WRAM0;
	
	memoryMap.oam = memoryMap.wram_sector + (0x2000 << (GBC << 1)) - OAM;
	memoryMap.io = ((u8 *) &ioPorts) - IO; // see io_ports.c/h  //memoryMap.oam  + 0xa0 + OAM - IO;
	memoryMap.hram = memoryMap.oam + 0xa0 + OAM - HRAM; //memoryMap.io;//   + 0x80;
}


u8 * init_memory(){
	
	if (alloc_memory()) {
		map_memory();
	}
	
	return Memory;
}

/// RW

inline u8 memory_read(u16 addr){
	last_addr = addr;
	if (addr & 0x8000) {
		if (addr & 0x4000) {
			if (addr & 0x2000) {
				if ((addr & OAM) == OAM) {
					if (addr & 0x0100) return (addr & 0x80) ? read_hram(addr): read_io(addr);
					else if ((addr & 0x80) && (addr & 0x60))  {
						ERROR("Read in Reserved RAM", "$%04X\n", addr);
						return (memoryMap.oam_lock) ? 0xFF: (addr & 0xF0) | (addr & 0xF0 >> 8); // GBC-E behavior
					} else return read_oam(addr);
				} else {
					DEBUG("Read in Echo RAM", "$%04X\n", addr);
					addr &= 0xDFFF;
					goto WR;
				}
			} else WR: return (addr & 0x1000) ? read_wram1(addr): read_wram0(addr); // WRAM
		} else         return (addr & 0x2000) ? read_xram(addr):  read_vram(addr);  // XRAM / VRAM
	} else             return (addr & 0x4000) ? read_rom1(addr):  read_rom0(addr);  // ROM
}


inline u8 direct_read(u16 addr){
	last_addr = addr;
	if (addr & 0x8000) {
		if (addr & 0x4000) {
			if (addr & 0x2000) {
				if ((addr & OAM) == OAM) {
					if (addr & 0x0100) return (addr & 0x80) ? direct_read_hram(addr): direct_read_io(addr);
					else if ((addr & 0x80) && (addr & 0x60))  {
						ERROR("Read in Reserved RAM", "$%04X\n", addr);
						return (memoryMap.oam_lock) ? 0xFF: (addr & 0xF0) | (addr & 0xF0 >> 8); // GBC-E behavior
					} else return direct_read_oam(addr);
				} else {
					DEBUG("Read in Echo RAM", "$%04X\n", addr);
					addr &= 0xDFFF;
					goto WR;
				}
			} else WR: return (addr & 0x1000) ? direct_read_wram1(addr): direct_read_wram0(addr); // WRAM
		} else return         (addr & 0x2000) ? direct_read_xram(addr):  direct_read_vram(addr);  // XRAM / VRAM
	} else return             (addr & 0x4000) ? direct_read_rom1(addr):  direct_read_rom0(addr);  // ROM
}

inline void write(u16 addr, u8 value){
	last_addr = addr;
	last_value = value;
	if (addr & 0x8000) {
		if (addr & 0x4000) {
			if (addr & 0x2000) {
				if ((addr & OAM) == OAM) {
					if (addr & 0x0100) { // FFXX
						if (addr == 0xFFFF) write_ie(value); // bc 0x1F filter
						if (addr & 0x80) write_hram(addr, value);
						else write_io(addr, value);
					} else if ((addr & 0x80) && (addr & 0x60)) ERROR("Write in Reserved RAM", "$%04X (%hhu | 0x%02X)\n", addr, value, value);
					else write_oam(addr, value);
				} else {
					DEBUG("Write in Echo RAM", "$%04X (%hhu | 0x%02X)\n", addr, value, value);
					addr &= 0xDFFF;
					goto WR;
				}
			} else {
				WR:
				if (addr & 0x1000) write_wram1(addr, value);
				else write_wram0(addr, value); // WRAM
			}
		} else {
			if (addr & 0x2000) write_xram(addr, value);
			else write_vram(addr, value); // XRAM / VRAM
		}
	} else if (addr & 0x4000) write_rom1(addr, value);
	else write_rom0(addr, value); // ROM
}

inline void direct_write(u16 addr, u8 value){
	last_addr = addr;
	last_value = value;
	if (addr & 0x8000) {
		if (addr & 0x4000) {
			if (addr & 0x2000) {
				if ((addr & OAM) == OAM) {
					if (addr & 0x0100) {
						if (addr == 0xFFFF) direct_write_ie(value);
						else if (addr & 0x80) direct_write_hram(addr, value);
						else direct_write_io(addr, value);
					} else if ((addr & 0x80) && (addr & 0x60)) ERROR("Write in Reserved RAM", "$%04X (%hhu | 0x%02X)\n", addr, value, value);
					else direct_write_oam(addr, value);
				} else {
					DEBUG("Write in Echo RAM", "$%04X (%hhu | 0x%02X)\n", addr, value, value);
					addr &= 0xDFFF;
					goto WR;
				}
			} else {
				WR:
				if (addr & 0x1000) direct_write_wram1(addr, value);
				else direct_write_wram0(addr, value); // WRAM
			}
		} else {
			if (addr & 0x2000) direct_write_xram(addr, value);
			else direct_write_vram(addr, value); // XRAM / VRAM
		}
	} else if (addr & 0x4000) direct_write_rom1(addr, value);
	else direct_write_rom0(addr, value); // ROM
}
