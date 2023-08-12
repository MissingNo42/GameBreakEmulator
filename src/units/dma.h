//
// Created by Romain on 04/08/2023.
//

#ifndef GOBOUEMU_DMA_H
#define GOBOUEMU_DMA_H

#include "../types.h"
#include "mmu.h"

#define dma_sync(cycle) if (memoryMap.dma_lock) for (u8 i = 0; i < (cycle); i += 4) dma_run()

extern u8 dma_progess;

void inline dma_start() {
	dma_progess = 0;
	Lock_dma();
}

void dma_run();

#endif //GOBOUEMU_DMA_H
