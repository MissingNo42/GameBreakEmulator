//
// Created by Romain on 14/07/2023.
//

////////////////////////  Includes  ///////////////////////////

#include "mmu.h"
#include "../io_ports.h"
#include <malloc.h>


//////////////////////  Declarations  /////////////////////////

u8 * Memory = NULL;
MemoryMap memoryMap;
static const u8 BootRom_DMG[0x100] = {	0x31,0xFE,0xFF,0xAF,0x21,0xFF,0x9F,0x32,0xCB,0x7C,0x20,0xFB,0x21,0x26,0xFF,0x0E,
										0x11,0x3E,0x80,0x32,0xE2,0x0C,0x3E,0xF3,0xE2,0x32,0x3E,0x77,0x77,0x3E,0xFC,0xE0,
										0x47,0x11,0x04,0x01,0x21,0x10,0x80,0x1A,0xCD,0x95,0x00,0xCD,0x96,0x00,0x13,0x7B,
										0xFE,0x34,0x20,0xF3,0x11,0xD8,0x00,0x06,0x08,0x1A,0x13,0x22,0x23,0x05,0x20,0xF9,
										0x3E,0x19,0xEA,0x10,0x99,0x21,0x2F,0x99,0x0E,0x0C,0x3D,0x28,0x08,0x32,0x0D,0x20,
										0xF9,0x2E,0x0F,0x18,0xF3,0x67,0x3E,0x64,0x57,0xE0,0x42,0x3E,0x91,0xE0,0x40,0x04,
										0x1E,0x02,0x0E,0x0C,0xF0,0x44,0xFE,0x90,0x20,0xFA,0x0D,0x20,0xF7,0x1D,0x20,0xF2,
										0x0E,0x13,0x24,0x7C,0x1E,0x83,0xFE,0x62,0x28,0x06,0x1E,0xC1,0xFE,0x64,0x20,0x06,
										0x7B,0xE2,0x0C,0x3E,0x87,0xE2,0xF0,0x42,0x90,0xE0,0x42,0x15,0x20,0xD2,0x05,0x20,
										0x4F,0x16,0x20,0x18,0xCB,0x4F,0x06,0x04,0xC5,0xCB,0x11,0x17,0xC1,0xCB,0x11,0x17,
										0x05,0x20,0xF5,0x22,0x23,0x22,0x23,0xC9,0xCE,0xED,0x66,0x66,0xCC,0x0D,0x00,0x0B,
										0x03,0x73,0x00,0x83,0x00,0x0C,0x00,0x0D,0x00,0x08,0x11,0x1F,0x88,0x89,0x00,0x0E,
										0xDC,0xCC,0x6E,0xE6,0xDD,0xDD,0xD9,0x99,0xBB,0xBB,0x67,0x63,0x6E,0x0E,0xEC,0xCC,
										0xDD,0xDC,0x99,0x9F,0xBB,0xB9,0x33,0x3E,0x3C,0x42,0xB9,0xA5,0xB9,0xA5,0x42,0x3C,
										0x21,0x04,0x01,0x11,0xA8,0x00,0x1A,0x13,0xBE,
										0x20,0xFE, // Logo Locker
										0x23,0x7D,0xFE,0x34,0x20,
										0xF5,0x06,0x19,0x78,0x86,0x23,0x05,0x20,0xFB,0x86,0x20,0xFE,0x3E,0x01,0xE0,0x50};


////////////////////////   Methods   //////////////////////////

void map_bootrom() {
	INFO("Mount the DMG Boot Rom", "\n");
	for(u16 i = 0; i < 0x100; i++) Memory[i] = BootRom_DMG[i];
	
	INFO("Mount the cartridge header", "\n");
	for (u8 i = 0; i < 0x50; i++) Memory[0x100 | i] = cartridgeHeader.raw[i];
}


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
	if (addr & 0x8000) {
		if (addr & 0x4000) {
			if (addr & 0x2000) {
				if ((addr & OAM) == OAM) {
					if (addr & 0xFF00) return (addr & 0x80) ? read_hram(addr): read_io(addr);
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
		} else return (addr & 0x2000) ? read_xram(addr): read_vram(addr); // XRAM / VRAM
	} else return (addr & 0x4000) ? read_rom1(addr): read_rom0(addr); // ROM
}


inline u8 direct_read(u16 addr){
	if (addr & 0x8000) {
		if (addr & 0x4000) {
			if (addr & 0x2000) {
				if ((addr & OAM) == OAM) {
					if (addr & 0xFF00) return (addr & 0x80) ? direct_read_hram(addr): direct_read_io(addr);
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
		} else return (addr & 0x2000) ? direct_read_xram(addr): direct_read_vram(addr); // XRAM / VRAM
	} else return (addr & 0x4000) ? direct_read_rom1(addr): direct_read_rom0(addr); // ROM
}

inline void write(u16 addr, u8 value){
	if (addr & 0x8000) {
		if (addr & 0x4000) {
			if (addr & 0x2000) {
				if ((addr & OAM) == OAM) {
					if (addr & 0xFF00) {
						if (addr == 0xFFFF) { write_ie(value); }
						if (addr & 0x80) { write_hram(addr, value); }
						else write_io(addr, value);
					} else if ((addr & 0x80) && (addr & 0x60))  {
						ERROR("Write in Reserved RAM", "$%04X (%hhu | 0x%02X)\n", addr, value, value);
					} else { write_oam(addr, value); }
				} else {
					DEBUG("Write in Echo RAM", "$%04X (%hhu | 0x%02X)\n", addr, value, value);
					addr &= 0xDFFF;
					goto WR;
				}
			} else {
				WR:
				if (addr & 0x1000) { write_wram1(addr, value); }
				else { write_wram0(addr, value); } // WRAM
			}
		} else {
			if (addr & 0x2000) { write_xram(addr, value); }
			else { write_vram(addr, value); } // XRAM / VRAM
		}
	} else if (addr & 0x4000) { write_rom1(addr, value); }
	else { write_rom0(addr, value); } // ROM
}

inline void direct_write(u16 addr, u8 value){
	if (addr & 0x8000) {
		if (addr & 0x4000) {
			if (addr & 0x2000) {
				if ((addr & OAM) == OAM) {
					if (addr & 0xFF00) {
						if (addr == 0xFFFF) { direct_write_ie(value); }
						else if (addr & 0x80) { direct_write_hram(addr, value); }
						else direct_write_io(addr, value);
					} else if ((addr & 0x80) && (addr & 0x60))  {
						ERROR("Write in Reserved RAM", "$%04X (%hhu | 0x%02X)\n", addr, value, value);
					} else { direct_write_oam(addr, value); }
				} else {
					DEBUG("Write in Echo RAM", "$%04X (%hhu | 0x%02X)\n", addr, value, value);
					addr &= 0xDFFF;
					goto WR;
				}
			} else {
				WR:
				if (addr & 0x1000) { direct_write_wram1(addr, value); }
				else { direct_write_wram0(addr, value); } // WRAM
			}
		} else {
			if (addr & 0x2000) { direct_write_xram(addr, value); }
			else { direct_write_vram(addr, value); } // XRAM / VRAM
		}
	} else if (addr & 0x4000) { direct_write_rom1(addr, value); }
	else { direct_write_rom0(addr, value); } // ROM
}
