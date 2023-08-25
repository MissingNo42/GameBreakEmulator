//
// Created by Romain on 14/07/2023.
//

#ifndef GOBOUEMU_MMU_H
#define GOBOUEMU_MMU_H

////////////////////////  Includes  ///////////////////////////

#include <stddef.h>
#include "../types.h"
#include "../cartridge.h"
#include "../mapper.h"


////////////////////////    Types   ///////////////////////////

/**
 * @brief padded offsets in Memory
 * */
Struct {
	u8 * xram_sector, * vram_sector, * wram_sector; // const
	u8 * rom0,  // $0000 - $3fff
	   * rom1,  // $4000 - $7fff
	   * vram,  // $8000 - $9fff
	   * xram,  // $a000 - $bfff
	   * wram0, // $c000 - $cfff
	   * wram1, // $d000 - $dfff
	   * oam,   // $fe00 - $fe9f
	   * io,    // $ff00 - $ff7f
	   * hram;  // $ff80 - $fffe + IE at $ffff
	   //ie;    // $ffff - $ffff (not a ptr bc not needed)
    u8 vram_bank: 1, wram_bank: 3; // GBC
	u8 xram_bank: 4; // TODO verify :4
	u8 rom0_bank, rom1_bank;
	u32 map_size;
    u8 cram[0x80]; // GBC
	u8 oam_lock: 1, vram_lock: 1;
	u8 ppu_oam_lock: 1; // oam
	u8 ppu_ram_lock: 1; // vram / cram
	u8 dma_lock: 1; // lock vram, oam, io
	u8 mem_lock: 1; // lock rom, xram, wram (DMG only)
	u8 xram_enable: 1; // 1 = enable
	u8 bootrom_unmapped: 1; // 1 = unmapped
} MemoryMap;


//////////////////////  Declarations  /////////////////////////

extern u8 * Memory;
extern MemoryMap memoryMap;


////////////////////////   Macros   ///////////////////////////

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


// Mem Lock
#define Lock_ppu_oam() memoryMap.oam_lock = memoryMap.ppu_oam_lock = 1 // Lock OAM
#define Unlock_ppu_oam() memoryMap.ppu_oam_lock = 0; if (!memoryMap.dma_lock) memoryMap.oam_lock = 0

#define Lock_ppu_ram() memoryMap.vram_lock = memoryMap.ppu_ram_lock = 1 // Lock VRAM + CRAM + OAM
#define Unlock_ppu_ram() memoryMap.ppu_ram_lock = 0; if (!memoryMap.dma_lock) memoryMap.vram_lock = 0

#define dma_locking(n) memoryMap.dma_lock = n; if (!GBC) memoryMap.mem_lock = n
#define Lock_dma() memoryMap.oam_lock = memoryMap.vram_lock = dma_locking(1)
#define Unlock_dma() memoryMap.mem_lock = dma_locking(0); if(!memoryMap.ppu_oam_lock)  memoryMap.oam_lock = 0; if(!memoryMap.ppu_ram_lock) memoryMap.vram_lock = 0

// Mem Bank
#define set_rom0(bank) memoryMap.rom0_bank = (bank), memoryMap.rom0 = (Memory + ((bank) << 14) - ROM0)
#define set_rom1(bank) memoryMap.rom1_bank = (bank), memoryMap.rom1 = (Memory + ((bank) << 14) - ROM1)

#define set_vram(bank) memoryMap.vram_bank = (bank), memoryMap.vram = (memoryMap.vram_sector + ((bank) << 13) - VRAM)
#define set_wram(bank) memoryMap.wram_bank = (bank), memoryMap.wram1 = (memoryMap.wram_sector + ((bank) << 12) - WRAM1)
#define set_xram(bank) memoryMap.xram_bank = (bank), memoryMap.xram = (memoryMap.xram_sector + ((bank) << 13) - XRAM)

// Mem Access

/// Macro-form
#define direct_reader(R, addr) memoryMap.R[addr]
#define indirect_reader(R, addr, cond) (cond) ? 0xFF : direct_read_##R(addr)

#define direct_writer(R, addr, value) memoryMap.R[addr] = value
#define indirect_writer(R, addr, value, cond) if (!(cond)) direct_write_##R(addr, value)

/// Inlined-form
#define mmu_DRX(R, access) inline u8 direct_read_##R(u16 addr) { return access; }
#define mmu_DWX(R, access) inline void direct_write_##R(u16 addr, u8 value) { access; }

#define mmu_DR(R) inline u8 direct_read_##R(u16 addr) { return memoryMap.R[addr]; }
#define mmu_IR(R, cond) inline u8 read_##R(u16 addr) { return (cond) ? 0xFF : direct_read_##R(addr); }

#define mmu_DW(R) inline void direct_write_##R(u16 addr, u8 value) { memoryMap.R[addr] = value; }
#define mmu_IW(R, cond) inline void write_##R(u16 addr, u8 value) { if (!(cond)) direct_write_##R(addr, value); }


#define mmu_D(R) mmu_DR(R) mmu_DW(R)
#define mmu_DX(R, accessR, accessW) mmu_DRX(R, accessR) mmu_DWX(R, accessW)
#define mmu_I(R, cond) mmu_IR(R, cond) mmu_IW(R, cond)
#define mmu_RW(R, cond) mmu_D(R) mmu_I(R, cond)
#define mmu_RWX(R, accessW, cond) mmu_DR(R) mmu_DWX(R, accessW) mmu_I(R, cond)
#define mmu_RXW(R, accessR, cond) mmu_DRX(R, accessR) mmu_DW(R) mmu_I(R, cond)
#define mmu_RXWX(R, accessR, accessW, cond) mmu_DX(R, accessR, accessW) mmu_I(R, cond)

inline u8 direct_read_ie() { return memoryMap.hram[0xFFFF]; }
inline void direct_write_ie(u8 value) { memoryMap.hram[0xFFFF] = (value & 0x1f); }
#define read_ie() direct_read_ie()
#define write_ie(value) direct_write_ie(value)

#define add_interrupt(int) write_ie(read_ie() | (int))
#define rmv_interrupt(int) write_ie(read_ie() & ~(int))

mmu_RWX(rom0, mapper.io.write_rom0(addr, value), memoryMap.mem_lock)
mmu_RWX(rom1, mapper.io.write_rom1(addr, value), memoryMap.mem_lock)

mmu_RW(vram,  memoryMap.vram_lock)
mmu_RW(cram,  memoryMap.ppu_ram_lock) // addr is offset here
mmu_RW(wram0, memoryMap.mem_lock)
mmu_RW(wram1, memoryMap.mem_lock)
mmu_RW(oam,   memoryMap.oam_lock)
mmu_RW(hram,  0)

mmu_RXWX(xram, mapper.io.read_ram(addr), mapper.io.write_ram(addr, value), memoryMap.mem_lock)


#define direct_read_oam_block(index) ((ObjAttribute *)(memoryMap.oam + OAM))[index]
#define read_oam_block(index) ((memoryMap.dma_lock) ? (ObjAttribute){0xFF, 0xFF, 0xFF, 0xFF} : direct_read_oam_block(index))
#define read_oam_mode3(offset) ((memoryMap.dma_lock) ? direct_reader(oam, OAM | dma_progess) : direct_reader(oam, OAM | (offset)))
#define read_oam_block_mode3(index) ((memoryMap.dma_lock) ?  direct_read_oam_block((dma_progess & 0xFC) >> 2) : direct_read_oam_block(index))


// Interrupt Enum (io: IE)
#define INT_VBLANK 1
#define INT_LCD_STAT 2
#define INT_TIMER 4
#define INT_SERIAL 8
#define INT_JOYPAD 16


////////////////////////   Methods   //////////////////////////

void map_bootrom();

u32 get_size();
u8 * init_memory();
u8 * alloc_memory();
void map_memory();
void free_memory();

u8 memory_read(u16 addr);
void write(u16 addr, u8 value);

u8 direct_read(u16 addr);
void direct_write(u16 addr, u8 value);

static inline void dummy_write(u16 addr, u8 value){}


/////////////////////  Registrations  /////////////////////////

Reset(mmu) {
	// Reset the mapping
	for (u16 i = 0; i < (u16)sizeof(memoryMap); i++) ((u8*)&memoryMap)[i] = 0x00;
	memoryMap.rom1_bank = memoryMap.wram_bank = 1;
	
	if (hard) {
		free_memory();
	} else {
		get_size(); // avoid freeing the existing map
	}
	
	init_memory();
	
	if (hard) map_bootrom();
}

SaveSize(mmu, sizeof (memoryMap) + memoryMap.map_size)

Save(mmu) {
	save_obj(memoryMap);
	save_obj(*(u8 (*)[memoryMap.map_size])Memory);
}

Load(mmu) {
	load_obj(memoryMap);
	load_obj(*(u8 (*)[memoryMap.map_size])Memory); // need correct map_size
	map_memory();
}

#endif //GOBOUEMU_MMU_H
