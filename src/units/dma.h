//
// Created by Romain on 04/08/2023.
//

#ifndef GBEMU_DMA_H
#define GBEMU_DMA_H

////////////////////////  Includes  ///////////////////////////

#include "../types.h"
#include "mmu.h"


////////////////////////   Macros   ///////////////////////////

#define dma_sync(cycles) if (memoryMap.dma_lock) dma_run(cycles)
#define hdma_running() !(ioHDMA5 & 0x80)


////////////////////////    Types   ///////////////////////////

Struct {
	union {
		Lstruct ( u16 lo: 4, progress: 7, _u0: 2, hblank: 1, general: 1, current_hblank: 1; )
		Lstruct ( u16 offset: 11, _u1: 5; )
		Bstruct ( u16 current_hblank: 1, general: 1, hblank: 1, _: 2, progress: 7, lo: 4; )
		Bstruct ( u16 _u1: 5, offset: 11; )
		u16 hdma;
	};
	union {
		Lstruct(u8 src_low,  src_high;)
		Bstruct(u8 src_high, src_low;)
		u16 src;
	};
	union {
		Lstruct(u8 dst_low,  dst_high;)
		Bstruct(u8 dst_high, dst_low;)
		u16 dst;
	};
} HDMA;


//////////////////////// Declarations ///////////////////////////

extern u8 dma_progess;
extern HDMA hdma;


//////////////////////// Registrations ///////////////////////////

Reset(dma) { dma_progess = 0, hdma.hdma = hdma.src = hdma.dst = 0; }
SaveSize(dma, sizeof(dma_progess) + sizeof(hdma))

Save(dma) {
	save_obj(dma_progess);
	save_obj(hdma);
}

Load(dma) {
	load_obj(dma_progess);
	load_obj(hdma);
}


////////////////////////   Methods   ///////////////////////////

void dma_start();

void dma_run(u8 cycles);
void hdma_start(u8 value);
void hdma_run(u8 cycles);

#endif //GBEMU_DMA_H
