//
// Created by Romain on 30/07/2023.
//

#include "timer.h"
#include "units/cpu.h"

const u16 divmask[] = {0x200, 0x8, 0x20, 0x80};

Timer timer;


void clock_run(u8 cycles) {
	//if (cycles != 4 && cycles != 8) CRITICAL("", "g\n");
	
	cycles >>= 2;
	s32 snd_mask = double_speed ? 0x40 : 0x20;
	s32 tma_mask = (ioTAC & 4) ? divmask[ioTAC & 3]: 0;
	
	do {
		timer.wdiv += 4;
		ioDIV = timer.div;
		
		s32 clock_fed = timer.div & snd_mask;
		
		if (timer.snd_fed && !clock_fed) {
			timer.snd_fed = 0;
			 // TODO timer sound
		} else timer.snd_fed = clock_fed;
		
		if (timer.reset_tima_done) timer.reset_tima_done = 0;
		
		if (timer.reset_tima_rq){ // overflow reset delay (instead of !ioTIMA because 0x00 write dont reset TIMA)
			timer.reset_tima_rq = 0;
			timer.reset_tima_done = 1;
			ioTIMA = ioTMA;
			add_interrupt(INT_TIMER);
		}
		
		clock_fed = timer.wdiv & tma_mask;
		if (timer.tma_fed && !clock_fed) {
			timer.tma_fed = 0;
			ioTIMA++;
			if (!ioTIMA) timer.reset_tima_rq = 1; // Overflow
		} else timer.tma_fed = clock_fed;
		
	} while(--cycles);
}
