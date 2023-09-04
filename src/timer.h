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
} Timer;

extern Timer timer;

#define fed_detect(fed) ((fed) == 0x02)
#define fed_set(fed, value) fed = ((fed) << 1) | (value)

void clock_run(u8 cycles);

Reset(timer) {
	timer.wdiv = 0;
	timer.snd_fed = 0;
	timer.tma_fed = 0;
	timer.reset_tima_rq = 0;
	timer.reset_tima_done = 0;
	timer.reset_tima_delay = 3;
}

SaveSize(timer, sizeof(Timer))

Save(timer) {
	save_obj(timer);
}

Load(timer) {
	load_obj(timer);
}

#endif //GBEMU_CLOCK_H
