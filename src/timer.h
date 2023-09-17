//
// Created by Romain on 30/07/2023.
//

#ifndef GBEMU_CLOCK_H
#define GBEMU_CLOCK_H

#include "types.h"
#include "io_ports.h"

// #define ClkPerSec 4194304 // 1 << 22

Struct {
	union {
		Estruct2(u8, _, div)
		u16 wdiv;
	};
	u8 reset_tima_rq;
	u8 reset_tima_done;
	u8 snd_fed;
	u16 tma_fed; // FED: falling edge detector
} Timer;

extern Timer timer;

void clock_run(u8 cycles);

Reset(timer) {
	timer.wdiv = 0;
	timer.snd_fed = 0;
	timer.tma_fed = 0;
	timer.reset_tima_rq = 0;
	timer.reset_tima_done = 0;
}

SaveSize(timer, sizeof(Timer))

Save(timer) {
	save_obj(timer);
}

Load(timer) {
	load_obj(timer);
}

#endif //GBEMU_CLOCK_H
