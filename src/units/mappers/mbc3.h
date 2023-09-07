//
// Created by Romain on 03/09/2023.
//

#ifndef GAMEBREAKEMULATOR_MBC3_H
#define GAMEBREAKEMULATOR_MBC3_H


#include "../mmu.h"


static inline void init_mbc3(){
	memoryMap.xram_enable = 0;
	mapper.data.mbc3.halt_date = mapper.data.mbc3.date = time(NULL);
	mapper.data.mbc3.S = mapper.data.mbc3.M = mapper.data.mbc3.Hr = mapper.data.mbc3.DL = mapper.data.mbc3.latch = 0;
	mapper.data.mbc3.DH = 0xFF;
}

static inline void write_rom0_mbc3(u16 addr, u8 value){
	if (addr & 0x2000) { // 0x2000-0x3fff
		value &= cartridgeInfo.rom_bank - 1; // mask
		if (!value) value = 1;
		set_rom1(value);
	} else {
		if (cartridgeInfo.type.has_ram) {
			memoryMap.xram_enable = (value & 0xF) == 0xA; // 0x000-0x1fff
		}
	}
}

static inline void upd_timer_mbc3() { // latch clock data
	time_t nd = mapper.data.mbc3.halt ? mapper.data.mbc3.halt_date: time(NULL);
	time_t diff = nd - mapper.data.mbc3.date;
	
	if (diff > 0) { // never < 0 unless savestate and/or system time change
		diff += mapper.data.mbc3.S +
		        mapper.data.mbc3.M * 60 +
		        mapper.data.mbc3.Hr * 3600 +
		        mapper.data.mbc3.DL * 86400 +
		        ((mapper.data.mbc3.day) << 8) * 86400;
		
		mapper.data.mbc3.S = diff % 60;
		mapper.data.mbc3.M = (diff / 60) % 60;
		mapper.data.mbc3.Hr = (diff / 3600) % 24;
		u32 day = (diff / 86400) % 24;
		mapper.data.mbc3.DL = day;
		mapper.data.mbc3.day = day >> 8;
		mapper.data.mbc3.of |= day >= 512;
	}
	mapper.data.mbc3.date = nd;
}

static inline void write_rom1_mbc3(u16 addr, u8 value){
	if (addr & 0x2000) {
		if (!value) mapper.data.mbc3.latch = 1;
		else {
			if (value == 1 && mapper.data.mbc3.latch == 1) upd_timer_mbc3();
			mapper.data.mbc3.latch = 0;
		}
	} // 0x6000-0x7fff
	else {                                            // 0x4000-0x5fff
		if (cartridgeInfo.type.has_timer && (value & 8) && value <= 0xc) {
			memoryMap.xram_bank = value;
		} else if (cartridgeInfo.type.has_ram) {
			value &= cartridgeInfo.ram_bank - 1; // mask
			set_xram(value);
		}
	}
}

static inline u8 read_ram_mbc3(u16 addr){
	if (memoryMap.xram_enable) {
		if (memoryMap.xram_bank & 8) {
			switch (memoryMap.xram_bank) {
				case 0x8: return mapper.data.mbc3.S;
				case 0x9: return mapper.data.mbc3.M;
				case 0xa: return mapper.data.mbc3.Hr;
				case 0xb: return mapper.data.mbc3.DL;
				case 0xc: return mapper.data.mbc3.DH;
				default:
					CRITICAL("Read to undefined RTC register", "n°%01X at $%04X\n", memoryMap.xram_bank, addr);
					return 0xff;
			}
		} else return memoryMap.xram[addr];
	} else {
		CRITICAL("Read to locked XRAM", "at $%04X\n", addr);
		return 0xff;
	}
}


static inline void write_ram_mbc3(u16 addr, u8 value){
	if (memoryMap.xram_enable) {
		if (memoryMap.xram_bank & 8) {
			switch (memoryMap.xram_bank) {
				case 0x8: if (value < 60) mapper.data.mbc3.S = value; break;
				case 0x9: if (value < 60) mapper.data.mbc3.M = value; break;
				case 0xa: if (value < 24) mapper.data.mbc3.Hr = value; break;
				case 0xb: mapper.data.mbc3.DL = value; break;
				case 0xc: {
					
					u8 old_h = mapper.data.mbc3.halt;
					mapper.data.mbc3.DH = value;
					
					if (!old_h && value & 0x40) mapper.data.mbc3.halt_date = time(NULL); // become halted: keep halt time
					else if (old_h && !(value & 0x40)) mapper.data.mbc3.date += time(NULL) - mapper.data.mbc3.halt_date; // become unhalted: add halted time to the date
					break;
				}
				default: CRITICAL("Write to undefined RTC register", "n°%01X at $%04X (new = %02X)\n", memoryMap.xram_bank, addr, value);
			}
		} else memoryMap.xram[addr] = value;
	} else CRITICAL("Write to locked XRAM", "at $%04X = %02X (new = %02X)\n", addr, memoryMap.xram[addr], value);
}

#endif //GAMEBREAKEMULATOR_MBC1_H
