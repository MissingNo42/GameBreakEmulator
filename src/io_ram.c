//
// Created by Romain on 27/07/2023.
//

#include "io_ram.h"
#include "units/cpu.h"
#include "units/dma.h"
#include "units/ppu.h"

IO_RAM ioRam;

/**
 * Interpret given value and write only the readable data to the register
 * */
 
void w00(u16 addr, u8 value) {
	btn_selector = value >> 4;
	ioP1 = value & 0x30 // readable?
	    | (ActionBtn >> (value & 0x20))
		| (DirectionBtn >> (value & 0x10));
};

void w01(u16 addr, u8 value) {
	direct_raw_write_io(addr, value);
}

void w02(u16 addr, u8 value) {
	direct_raw_write_io(addr, value);
}

#define w03 dummy_write

void w04(u16 addr, u8 value) {
	direct_raw_write_io(addr, 0x00);
}

void w05(u16 addr, u8 value) {
	direct_raw_write_io(addr, value);
}

void w06(u16 addr, u8 value) {
	direct_raw_write_io(addr, value);
}

void w07(u16 addr, u8 value) {
	direct_raw_write_io(addr, value);
}

#define w08 dummy_write
#define w09 dummy_write
#define w0A dummy_write
#define w0B dummy_write
#define w0C dummy_write
#define w0D dummy_write
#define w0E dummy_write

void w0F(u16 addr, u8 value) {
	direct_raw_write_io(addr, value);
}

void w10(u16 addr, u8 value) {
	direct_raw_write_io(addr, value);
}

void w11(u16 addr, u8 value) {
	direct_raw_write_io(addr, value);
}

void w12(u16 addr, u8 value) {
	direct_raw_write_io(addr, value);
}

void w13(u16 addr, u8 value) {
	direct_raw_write_io(addr, value);
}

void w14(u16 addr, u8 value) {
	direct_raw_write_io(addr, value);
}

#define w15 dummy_write
void w16(u16 addr, u8 value) {
	direct_raw_write_io(addr, value);
}

void w17(u16 addr, u8 value) {
	direct_raw_write_io(addr, value);
}

void w18(u16 addr, u8 value) {
	direct_raw_write_io(addr, value);
}

void w19(u16 addr, u8 value) {
	direct_raw_write_io(addr, value);
}

void w1A(u16 addr, u8 value) {
	direct_raw_write_io(addr, value);
}

void w1B(u16 addr, u8 value) {
	direct_raw_write_io(addr, value);
}

void w1C(u16 addr, u8 value) {
	direct_raw_write_io(addr, value);
}

void w1D(u16 addr, u8 value) {
	direct_raw_write_io(addr, value);
}

void w1E(u16 addr, u8 value) {
	direct_raw_write_io(addr, value);
}

#define w1F dummy_write

void w20(u16 addr, u8 value) {
	direct_raw_write_io(addr, value);
}

void w21(u16 addr, u8 value) {
	direct_raw_write_io(addr, value);
}

void w22(u16 addr, u8 value) {
	direct_raw_write_io(addr, value);
}

void w23(u16 addr, u8 value) {
	direct_raw_write_io(addr, value);
}

void w24(u16 addr, u8 value) {
	direct_raw_write_io(addr, value);
}

void w25(u16 addr, u8 value) {
	direct_raw_write_io(addr, value);
}

void w26(u16 addr, u8 value) {
	direct_raw_write_io(addr, value);
}

#define w27 dummy_write
#define w28 dummy_write
#define w29 dummy_write
#define w2A dummy_write
#define w2B dummy_write
#define w2C dummy_write
#define w2D dummy_write
#define w2E dummy_write
#define w2F dummy_write

void w30(u16 addr, u8 value) { // TODO
	direct_raw_write_io(addr, value);
}

#define w31 dummy_write
#define w32 dummy_write
#define w33 dummy_write
#define w34 dummy_write
#define w35 dummy_write
#define w36 dummy_write
#define w37 dummy_write
#define w38 dummy_write
#define w39 dummy_write
#define w3A dummy_write
#define w3B dummy_write
#define w3C dummy_write
#define w3D dummy_write
#define w3E dummy_write
#define w3F dummy_write

void w40(u16 addr, u8 value) { // LCDC
	u8 changed = ioLCDC ^ value;
	// if (changed & 0x1) { // LCDC.0
	//
	// }
	// if (changed & 0x2) { // LCDC.1
	//
	// }
	// if (changed & 0x4) { // LCDC.2
	//
	// }
	// if (changed & 0x8) { // LCDC.3
	//
	// }
	// if (changed & 0x10) { // LCDC.4
	//
	// }
	// if (changed & 0x20) { // LCDC.5
	//
	// }
	// if (changed & 0x40) { // LCDC.6
	//
	// }
	if (changed & 0x80) { // LCDC.7
		if (value & 0x80) ;
		else {
			DEBUG("LCDC.7 Switch off", "\n");
		}
	}
	ioLCDC = value;
}

void w41(u16 addr, u8 value) {
	ioSTAT = value & 0x78;
	STAT_changed();
}

void w42(u16 addr, u8 value) {
	direct_raw_write_io(addr, value);
}

void w43(u16 addr, u8 value) {
	direct_raw_write_io(addr, value);
}

#define w44 dummy_write // LY: Read only

void w45(u16 addr, u8 value) { // LYC
	ioLYC = value;
	if (ioLYC == ioLY) ioSTAT |= 0x4; // Upd STAT.2
	else ioSTAT &= ~0x4;
	STAT_changed();
}

void w46(u16 addr, u8 value) { // OAM DMA
	ioDMA = value;
	if ((0xE0 & value) != 0xE0) { // <= 0xDF
		DEBUG("DMA Start", "from %02X00\n", ioDMA);
		dma_start();
	}
}

void w47(u16 addr, u8 value) {
	direct_raw_write_io(addr, value);
}

void w48(u16 addr, u8 value) {
	direct_raw_write_io(addr, value);
}

void w49(u16 addr, u8 value) {
	direct_raw_write_io(addr, value);
}

void w4A(u16 addr, u8 value) {
	direct_raw_write_io(addr, value);
}

void w4B(u16 addr, u8 value) {
	direct_raw_write_io(addr, value);
}

#define w4C dummy_write

void w4D(u16 addr, u8 value) {
	direct_raw_write_io(addr, (double_speed << 7) | (value & 1));
}

#define w4E dummy_write
void w4F(u16 addr, u8 value) {
	direct_raw_write_io(addr, value);
}

#define w50 dummy_write
void w51(u16 addr, u8 value) {
	direct_raw_write_io(addr, value);
}

void w52(u16 addr, u8 value) {
	direct_raw_write_io(addr, value);
}

void w53(u16 addr, u8 value) {
	direct_raw_write_io(addr, value);
}

void w54(u16 addr, u8 value) {
	direct_raw_write_io(addr, value);
}

void w55(u16 addr, u8 value) {
	direct_raw_write_io(addr, value);
}

void w56(u16 addr, u8 value) {
	direct_raw_write_io(addr, value);
}

#define w57 dummy_write
#define w58 dummy_write
#define w59 dummy_write
#define w5A dummy_write
#define w5B dummy_write
#define w5C dummy_write
#define w5D dummy_write
#define w5E dummy_write
#define w5F dummy_write
#define w60 dummy_write
#define w61 dummy_write
#define w62 dummy_write
#define w63 dummy_write
#define w64 dummy_write
#define w65 dummy_write
#define w66 dummy_write
#define w67 dummy_write

void w68(u16 addr, u8 value) {
	ioBCPS = value & 0xbf;
	ioBCPD = memoryMap.cram[value & 0x3f];
}

void w69(u16 addr, u8 value) {
	// TODO: if cram locked
	ioBCPD = memoryMap.cram[ioBCPS & 0x3f] = value;
	
	if (ioBCPS & 0x80) w68(BCPS, ioBCPS + 1);
}

void w6A(u16 addr, u8 value) {
	ioOCPS = value & 0xbf;
	ioOCPD = memoryMap.cram[value & 0x3f | 0x40];
}

void w6B(u16 addr, u8 value) {
	// TODO: if cram locked
	ioOCPD = memoryMap.cram[ioOCPS & 0x3f | 0x40] = value;
	
	if (ioOCPS & 0x80) w6A(OCPS, ioOCPS + 1);
}

void w6C(u16 addr, u8 value) {
	direct_raw_write_io(addr, value);
}

#define w6D dummy_write
#define w6E dummy_write
#define w6F dummy_write

void w70(u16 addr, u8 value) {
	direct_raw_write_io(addr, value);
}

#define w71 dummy_write
#define w72 dummy_write
#define w73 dummy_write
#define w74 dummy_write
#define w75 dummy_write

void w76(u16 addr, u8 value) {
	direct_raw_write_io(addr, value);
}

void w77(u16 addr, u8 value) {
	direct_raw_write_io(addr, value);
}

#define w78 dummy_write
#define w79 dummy_write
#define w7A dummy_write
#define w7B dummy_write
#define w7C dummy_write
#define w7D dummy_write
#define w7E dummy_write
#define w7F dummy_write

writer * IO_wHANDLER[] = {
	w00, w01, w02, w03, w04, w05, w06, w07, w08, w09, w0A, w0B, w0C, w0D, w0E, w0F, // 00
	w10, w11, w12, w13, w14, w15, w16, w17, w18, w19, w1A, w1B, w1C, w1D, w1E, w1F, // 10
	w20, w21, w22, w23, w24, w25, w26, w27, w28, w29, w2A, w2B, w2C, w2D, w2E, w2F, // 20
	w30, w31, w32, w33, w34, w35, w36, w37, w38, w39, w3A, w3B, w3C, w3D, w3E, w3F, // 30
	w40, w41, w42, w43, w44, w45, w46, w47, w48, w49, w4A, w4B, w4C, w4D, w4E, w4F, // 40
	w50, w51, w52, w53, w54, w55, w56, w57, w58, w59, w5A, w5B, w5C, w5D, w5E, w5F, // 50
	w60, w61, w62, w63, w64, w65, w66, w67, w68, w69, w6A, w6B, w6C, w6D, w6E, w6F, // 60
	w70, w71, w72, w73, w74, w75, w76, w77, w78, w79, w7A, w7B, w7C, w7D, w7E, w7F, // 70
};

writer ** io_write_handlers = IO_wHANDLER - IO;
