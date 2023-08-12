//
// Created by Romain on 30/07/2023.
//

#ifndef GOBOUEMU_CLOCK_H
#define GOBOUEMU_CLOCK_H

#include "types.h"
#include "io_ram.h"

#define ClkPerSec 4194304 // 1 << 22

Struct {
	union {
		struct {
#ifdef LITTLE_ENDIAN
			u8 _, div;
#else
			u8 div, _;
#endif
		};
		u16 wdiv;
	};
	u8 snd_fed: 2, tma_fed: 2; // FED: falling edge detector
	u8 rst_tma: 1;
} Divider;

extern Divider clk_divider;

//TODO tma write after tma->tima

#define fed_detect(fed) (fed == 0x02)
#define fed_set(fed, value) fed = (fed << 1) | (value)

void clock_cycle();

#endif //GOBOUEMU_CLOCK_H
