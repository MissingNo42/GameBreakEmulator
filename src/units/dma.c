//
// Created by Romain on 04/08/2023.
//

////////////////////////  Includes  ///////////////////////////

#include "dma.h"
#include "../io_ports.h"


////////////////////////   Macros   ///////////////////////////

#define HDMA_SRC ((ioHDMA1 << 8) | ioHDMA2)
#define HDMA_DST ((ioHDMA3 << 8) | ioHDMA4)


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
		dma_progess++;
		
		direct_write_oam(OAM | dma_progess, read((ioDMA << 8) | dma_progess)); // copy
		if (dma_progess != 0xA0) {
			Unlock_dma();
			DEBUG("DMA End", "\n");
			break;
		} // relock or END
	} while (--cycles);
}


void hdma_start(u8 value) {
	if (hdma.hblank && ((value ^ 0x80) & 0x80)) { // cancel
		ioHDMA5 |= 0x80;
		hdma.hdma = 0;
		DEBUG("HDMA Cancel", "HDMA5 = %02X\n", ioHDMA5);
		
	} else { // (re)start
		DEBUG("HDMA Start", "src = $%04X | dst = $%04X | HDMA5 = %02X\n", HDMA_SRC, HDMA_DST, value);
		if (HDMA_SRC & 0x8000 && (((HDMA_SRC & 0x4000) >> 1) == (HDMA_SRC & 0x2000))) ERROR("HDMA Out of Bound", "src = $%04X | dst = $%04X | HDMA5 = %02X\n", HDMA_SRC, HDMA_DST, value);
		
		hdma.hdma = 0; // (hdma.current_hblank = 0) -> prevent tranfer start mid HBlank
		hdma.hblank = value >> 7;
		hdma.general = !hdma.hblank;
		ioHDMA5 = value & 0x7F; // .7 = 0 -> HDMA Active
	}
}


void hdma_run(u8 cycles) {
	cycles >>= 1;
	u16 src = HDMA_SRC + hdma.offset, dst = HDMA_DST + hdma.offset;
	
	do {
		direct_write_vram(dst++, read(src++));
		hdma.offset++;
	} while (--cycles && hdma.lo);
	
	if (!hdma.lo) {
		ioHDMA5--; // if 0 then overflow to FF = stop
		hdma.current_hblank = 0;
		
		if (ioHDMA5 == 0xFF) {
			end: hdma.hdma = 0;
			ioHDMA1 = ioHDMA2 = ioHDMA3 = ioHDMA4 = 0xFF; // hdma reset?
			DEBUG("HDMA End", "HDMA5 = %02X\n", ioHDMA5);
		}
		else if (dst & 0x2000) { // DST Overflow
			ioHDMA5 |= 0x80;     // Switch to Inactive, leave the left blocks num
			goto end;
		}
	}
}
