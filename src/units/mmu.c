//
// Created by Romain on 14/07/2023.
//

#include "mmu.h"
#include "../cartridge.h"
#include "../mapper.h"
#include "../io_ram.h"
#include <malloc.h>

u8 * Memory;
MemoryMap memoryMap;

u8 * init_memory(){
	u32 sz =
			cartridgeInfo.rom_size                 // ROM
			+ cartridgeInfo.ram_size                    // External RAM
			+ (0x2000 << cartridgeInfo.GBC_MODE)        // VRAM (x2 in GBC)
			+ (0x2000 << (cartridgeInfo.GBC_MODE << 1)) // WRAM (x4 in GBC)
			+ 0xa0                                      // OAM
			//+ 0x80                                      // IO Registers
			+ 0x7f;                                     // HRAM // (+ IE) //TODO : add IE to HRAM read/write if HRAM never locked (see TODO-A)
	
	Memory = malloc(sz);
	DEBUG("Init Virtual Memory", "size: %u | addr = %p\n", sz, Memory);
	
	// memoryMap.rom_sector = Memory
	memoryMap.xram_sector = Memory + cartridgeInfo.rom_size;
	memoryMap.vram_sector = memoryMap.xram_sector + cartridgeInfo.ram_size;
	memoryMap.wram_sector = memoryMap.vram_sector + (0x2000 << cartridgeInfo.GBC_MODE);

	set_rom0(0);
	set_rom1(1);
	
	set_vram(0);
	
	if ((memoryMap.xram_enable = cartridgeInfo.type.has_ram)) set_xram(0); // must be disabled at mapper init (if any)
	else memoryMap.xram = NULL;
	
	memoryMap.wram0 = memoryMap.wram_sector - WRAM0;
	set_wram(1);
	
	memoryMap.oam  = memoryMap.wram_sector + (0x2000 << (cartridgeInfo.GBC_MODE << 1)) - OAM;
	memoryMap.io   = ((u8*)&ioRam) - IO; //memoryMap.oam  + 0xa0 + OAM - IO;
	memoryMap.hram = memoryMap.oam  + 0xa0 + OAM - HRAM; //memoryMap.io;//   + 0x80;
	//memoryMap.ie   = memoryMap.hram + 0x7f + IE;
	
	return Memory;
}

u8 inline read(u16 addr){
	if (addr & 0x8000) {
		if (addr & 0x4000) {
			if (addr & 0x2000) {
				if ((addr & OAM) == OAM) {
					if (addr & 0xFF00) return (addr == 0xFFFF) ? read_ie(): ((addr & 0x80) ? read_hram(addr): read_io(addr));
					else if ((addr & 0x80) && (addr & 0x60))  {
						Log(Error, "Read in Reserved RAM", "$%04X\n", addr);
						return (memoryMap.oam_lock) ? 0xFF: (addr & 0xF0) | (addr & 0xF0 >> 8); // GBC-E behavior
					} else return read_oam(addr);
				} else {
					Log(Debug, "Read in Echo RAM", "$%04X\n", addr);
					addr &= 0xDFFF;
					goto WR;
				}
			} else WR: return (addr & 0x1000) ? read_wram1(addr): read_wram0(addr); // WRAM
		} else return (addr & 0x2000) ? read_xram(addr): read_vram(addr); // XRAM / VRAM
	} else return (addr & 0x4000) ? read_rom_1(addr): read_rom_0(addr); // ROM
}


u8 inline direct_read(u16 addr){
	if (addr & 0x8000) {
		if (addr & 0x4000) {
			if (addr & 0x2000) {
				if ((addr & OAM) == OAM) {
					if (addr & 0xFF00) return (addr == 0xFFFF) ? direct_read_ie(): ((addr & 0x80) ? direct_read_hram(addr): direct_read_io(addr));
					else if ((addr & 0x80) && (addr & 0x60))  {
						Log(Error, "Read in Reserved RAM", "$%04X\n", addr);
						return (memoryMap.oam_lock) ? 0xFF: (addr & 0xF0) | (addr & 0xF0 >> 8); // GBC-E behavior
					} else return direct_read_oam(addr);
				} else {
					Log(Debug, "Read in Echo RAM", "$%04X\n", addr);
					addr &= 0xDFFF;
					goto WR;
				}
			} else WR: return (addr & 0x1000) ? direct_read_wram1(addr): direct_read_wram0(addr); // WRAM
		} else return (addr & 0x2000) ? direct_read_xram(addr): direct_read_vram(addr); // XRAM / VRAM
	} else return (addr & 0x4000) ? direct_read_rom_1(addr): direct_read_rom_0(addr); // ROM
}

void inline write(u16 addr, u8 value){
	if (addr & 0x8000) {
		if (addr & 0x4000) {
			if (addr & 0x2000) {
				if ((addr & OAM) == OAM) {
					if (addr & 0xFF00) {
						if (addr == 0xFFFF) { write_ie(value); }
						else if (addr & 0x80) { write_hram(addr, value); }
						else write_io(addr, value);
					} else if ((addr & 0x80) && (addr & 0x60))  {
						Log(Error, "Write in Reserved RAM", "$%04X (%hhu / %02X)\n", addr, value, value);
					} else { write_oam(addr, value); }
				} else {
					Log(Debug, "Write in Echo RAM", "$%04X (%hhu / %02X)\n", addr, value, value);
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
	} else if (addr & 0x4000) { write_rom_1(addr, value); }
	else { write_rom_0(addr, value); } // ROM
}

void inline direct_write(u16 addr, u8 value){
	if (addr & 0x8000) {
		if (addr & 0x4000) {
			if (addr & 0x2000) {
				if ((addr & OAM) == OAM) {
					if (addr & 0xFF00) {
						if (addr == 0xFFFF) { direct_write_ie(value); }
						else if (addr & 0x80) { direct_write_hram(addr, value); }
						else direct_write_io(addr, value);
					} else if ((addr & 0x80) && (addr & 0x60))  {
						Log(Error, "Write in Reserved RAM", "$%04X (%hhu / %02X)\n", addr, value, value);
					} else { direct_write_oam(addr, value); }
				} else {
					Log(Debug, "Write in Echo RAM", "$%04X (%hhu / %02X)\n", addr, value, value);
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
	} else if (addr & 0x4000) { direct_write_rom_1(addr, value); }
	else { direct_write_rom_0(addr, value); } // ROM
}
