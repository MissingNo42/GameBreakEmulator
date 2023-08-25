//
// Created by Romain on 21/08/2023.
//

#ifndef GOBOUEMU_CORE_H
#define GOBOUEMU_CORE_H

#include "cartridge.h"
#include "io_ports.h"
#include "units/cpu.h"
#include "units/ppu.h"
#include "units/dma.h"
#include "units/mmu.h"
#include "units/ctrl.h"
#include "mapper.h"
#include "utils.h"

//#include <pthread.h>
//
//extern thrd_t emu_th;

void emulator_start(const char * const fn);
int emulator_loop(void * uns);

#endif //GOBOUEMU_CORE_H
