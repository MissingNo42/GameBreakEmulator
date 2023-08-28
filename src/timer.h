//
// Created by Romain on 30/07/2023.
//

#ifndef GBEMU_CLOCK_H
#define GBEMU_CLOCK_H

#include "types.h"
#include "io_ports.h"

#define ClkPerSec 4194304 // 1 << 22

Struct {
	union {
		struct {
#ifdef IS_LITTLE_ENDIAN
			u8 _, div;
#else
			u8 div, _;
#endif
		};
		u16 wdiv;
	};
	u8 snd_fed: 2, tma_fed: 2; // FED: falling edge detector
	u8 reset_tima_rq: 1;
	u8 reset_tima_done: 1;
	u8 reset_tima_delay: 2;    // = 3 default
} Clock;

extern Clock clock;

#define fed_detect(fed) ((fed) == 0x02)
#define fed_set(fed, value) fed = ((fed) << 1) | (value)

void clock_run(u8 cycles);

Reset(clock) {
	clock.wdiv = 0;
	clock.snd_fed = 0;
	clock.tma_fed = 0;
	clock.reset_tima_rq = 0;
	clock.reset_tima_done = 0;
	clock.reset_tima_delay = 3;
}

SaveSize(clock, sizeof(Clock))

Save(clock) {
	save_obj(clock);
}

Load(clock) {
	load_obj(clock);
}

#endif //GBEMU_CLOCK_H
