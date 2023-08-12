//
// Created by Romain on 14/07/2023.
//

#ifndef GOBOUEMU_MMU_H
#define GOBOUEMU_MMU_H

#include "../types.h"
#include "../cartridge.h"

#define ROM0  0x0000
#define ROM1  0x4000
#define VRAM  0x8000
#define XRAM  0xA000
#define WRAM0 0xC000
#define WRAM1 0xD000
#define ERAM  0xE000
#define OAM   0xFE00
#define RSV   0xFEA0
#define IO    0xFF00
#define HRAM  0xFF80
#define IE    0xFFFF

#define regIE memoryMap.ie

/**
 * @brief padded offsets in Memory
 * */
Struct {
	u8 * xram_sector, * vram_sector, * wram_sector; // const
	u8 * rom0,  // 0000 - 3fff
	   * rom1,  // 4000 - 7fff
	   * vram,  // 8000 - 9fff
	   * xram,  // a000 - bfff
	   * wram0, // c000 - cfff
	   * wram1, // d000 - dfff
	   * oam,   // fe00 - fe9f
	   * io,    // ff00 - ff7f
	   * hram,  // ff80 - fffe
	   ie;      // ffff - ffff (not a ptr bc not needed)
    u8 cram[0x80]; // GBC
    u8 vram_bank: 1, wram_bank: 3; // GBC
	u8 oam_lock: 1, vram_lock: 1;
	u8 ppu_oam_lock: 1; // oam
	u8 ppu_ram_lock: 1; // vram / cram
	u8 dma_lock: 1; // lock vram, oam, io
	u8 mem_lock: 1; // lock rom, xram, wram (DMG only)
	u8 xram_enable: 1; // 1 = enable
} MemoryMap;

extern u8 * Memory;
extern MemoryMap memoryMap;

// Mem Lock
#define Lock_ppu_oam() memoryMap.oam_lock = memoryMap.ppu_oam_lock = 1 // Lock OAM
#define Unlock_ppu_oam() memoryMap.ppu_oam_lock = 0; if (!memoryMap.dma_lock) memoryMap.oam_lock = 0

#define Lock_ppu_ram() memoryMap.ram_lock = memoryMap.ppu_ram_lock = 1 // Lock VRAM + CRAM + OAM
#define Unlock_ppu_ram() memoryMap.ppu_ram_lock = 0; if (!memoryMap.dma_lock) memoryMap.ram_lock = 0

#define dma_locking(n) memoryMap.dma_lock = n; if (!cartridgeInfo.GBC_MODE) memoryMap.mem_lock = n
#define Lock_dma() memoryMap.oam_lock = memoryMap.vram_lock = dma_locking(1)
#define Unlock_dma() memoryMap.mem_lock = dma_locking(0); if(!memoryMap.ppu_oam_lock)  memoryMap.oam_lock = 0; if(!memoryMap.ppu_ram_lock) memoryMap.vram_lock = 0

// Mem Bank
#define set_rom0(bank) memoryMap.rom0 = (Memory + ((bank) << 14) - ROM0)
#define set_rom1(bank) memoryMap.rom1 = (Memory + ((bank) << 14) - ROM1)

#define set_vram(bank) memoryMap.vram_bank = (bank), memoryMap.vram = (memoryMap.vram_sector + ((bank) << 13) - VRAM)
#define set_wram(bank) memoryMap.wram_bank = (bank), memoryMap.wram1 = (memoryMap.wram_sector + ((bank) << 12) - WRAM1)
#define set_xram(bank) memoryMap.xram = (memoryMap.xram_sector + ((bank) << 13) - XRAM)

// Mem Access
#define direct_reader(R, addr) memoryMap.R[addr]
#define indirect_reader(R, addr, cond) (cond) ? 0xFF : direct_reader(R, addr)

#define direct_writer(R, addr, value) memoryMap.R[addr] = value
#define indirect_writer(R, addr, value, cond) if (!(cond)) direct_writer(R, addr, value)


#define  direct_read_ie() memoryMap.ie
#define direct_write_ie(value) memoryMap.ie = ((value) & 0x1f)
#define  read_ie() direct_read_ie()
#define write_ie(value) direct_write_ie(value)


#define  direct_read_rom_0(addr) direct_reader(rom0, addr)
#define direct_write_rom_0(addr, value) mapper.io.write_rom0(addr, value)
#define  read_rom_0(addr) indirect_reader(rom0, addr, memoryMap.mem_lock)
#define write_rom_0(addr, value) indirect_writer(rom0, addr, value, memoryMap.mem_lock)


#define  direct_read_rom_1(addr) direct_reader(rom1, addr)
#define direct_write_rom_1(addr, value) mapper.io.write_rom1(addr, value)
#define  read_rom_1(addr) indirect_reader(rom1, addr, memoryMap.mem_lock)
#define write_rom_1(addr, value) indirect_writer(rom1, addr, value, memoryMap.mem_lock)


#define  direct_read_vram(addr) direct_reader(vram, addr)
#define direct_write_vram(addr, value) direct_writer(vram, addr, value)
#define  read_vram(addr) indirect_reader(vram, addr, memoryMap.vram_lock)
#define write_vram(addr, value) indirect_writer(vram, addr, value, memoryMap.vram_lock)


#define  direct_read_xram(addr) mapper.io.read_ram(addr)
#define direct_write_xram(addr, value) mapper.io.write_ram(addr, value)
#define  read_xram(addr) indirect_reader(xram, addr, memoryMap.mem_lock)
#define write_xram(addr, value) indirect_writer(xram, addr, value, memoryMap.mem_lock)


#define  direct_read_wram0(addr) direct_reader(wram0, addr)
#define direct_write_wram0(addr, value) direct_writer(wram0, addr, value)
#define  read_wram0(addr) indirect_reader(wram0, addr, memoryMap.mem_lock)
#define write_wram0(addr, value) indirect_writer(wram0, addr, value, memoryMap.mem_lock)


#define  direct_read_wram1(addr) direct_reader(wram1, addr)
#define direct_write_wram1(addr, value) direct_writer(wram1, addr, value)
#define  read_wram1(addr) indirect_reader(wram1, addr, memoryMap.mem_lock)
#define write_wram1(addr, value) indirect_writer(wram1, addr, value, memoryMap.mem_lock)


#define  direct_read_oam(addr) direct_reader(oam, addr)
#define direct_write_oam(addr, value) direct_writer(oam, addr, value)
#define  read_oam(addr) indirect_reader(oam, addr, memoryMap.oam_lock)
#define write_oam(addr, value) indirect_writer(oam, addr, value, memoryMap.oam_lock)


#define  direct_read_hram(addr) direct_reader(hram, addr)
#define direct_write_hram(addr, value) direct_writer(hram, addr, value)
#define  read_hram(addr) direct_read_hram(addr)
#define write_hram(addr, value) direct_write_hram(addr, value)


// Interrupt Enum (io: IE)
#define INT_VBLANK 1
#define INT_LCD_STAT 2
#define INT_TIMER 4
#define INT_SERIAL 8
#define INT_JOYPAD 16

u8 * init_memory();

u8 read(u16 addr);
void write(u16 addr, u8 value);

u8 direct_read(u16 addr);
void direct_write(u16 addr, u8 value);

static inline void dummy_write(u16 addr, u8 value){}

#endif //GOBOUEMU_MMU_H
