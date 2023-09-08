//
// Created by Romain on 04/08/2023.
//

////////////////////////  Includes  ///////////////////////////

#include <stdio.h>
#include "dma.h"
#include "../io_ports.h"


//////////////////////  Declarations  /////////////////////////

u8 dma_progess;
HDMA hdma;


////////////////////////   Methods   //////////////////////////

void dma_start() {
	dma_progess = 0;
	Lock_dma();
}

void dma_run(u8 cycles) {
	do {
		direct_write_oam(OAM | dma_progess, direct_read((ioDMA << 8) | dma_progess)); // copy
		dma_progess++;
		
		if (dma_progess == 0xA0) {
			Unlock_dma();
			//DEBUG("DMA End", "\n");
			break;
		} // relock or END
	} while (--cycles);
}


void hdma_start(u8 value) {
	if (hdma.hblank && ((value ^ 0x80) & 0x80)) { // cancel
		ioHDMA5 = 0x80 | value;
		hdma.hdma = 0;
		DEBUG("HDMA Cancel", "HDMA5 = %02X\n", ioHDMA5);
		
	} else { // (re)start
		DEBUG("HDMA Start", "src = $%04X | dst = $%04X | HDMA5 = %02X\n", hdma.src, hdma.dst, value);
		if (hdma.src & 0x8000 && (((hdma.src & 0x4000) >> 1) == (hdma.src & 0x2000))) CRITICAL("HDMA Out of Bound", "src = $%04X | dst = $%04X | HDMA5 = %02X\n", hdma.src, hdma.dst, value);
		
		hdma.hdma = 0; // (hdma.current_hblank = 0) -> prevent tranfer start mid HBlank
		hdma.hblank = value >> 7;
		hdma.general = !hdma.hblank;
		ioHDMA5 = value & 0x7F; // .7 = 0 -> HDMA Active
	}
}


void hdma_run(u8 cycles) {
	cycles >>= 1;
	
	do {
		//if(hdma.hblank)CRITICAL("COPY", "%04X to %04X (%02X)\n", hdma.src, hdma.dst, hdma.offset);
		direct_write_vram(hdma.dst++, mmu_read(hdma.src++));
		hdma.offset++;
	} while (--cycles && hdma.lo);
	
	if (!hdma.lo) {
		ioHDMA5--; // if 0 then overflow to FF = stop
		hdma.current_hblank = 0;
		
		if (ioHDMA5 == 0xFF) {
			end: hdma.hdma = 0;
			DEBUG("HDMA End", "HDMA5 = %02X\n", ioHDMA5);
		}
		else if (hdma.dst & 0x2000) { // DST Overflow
			ioHDMA5 |= 0x80;     // Switch to Inactive, leave the left blocks num
			goto end;
		}
	}
}
