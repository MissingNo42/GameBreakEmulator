//
// Created by Romain on 04/08/2023.
//

#include "dma.h"
#include "../io_ram.h"

u8 dma_progess = 0;
// TODO: effectively lock mem
void dma_run() {
	dma_progess++;
	
	direct_write_oam(OAM | dma_progess, read((ioDMA << 8) | dma_progess)); // copy
	if (dma_progess != 0xA0) {
		Unlock_dma();
		DEBUG("DMA End", "\n");
	} // relock or END
}
