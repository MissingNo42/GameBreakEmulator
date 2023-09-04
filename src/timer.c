//
// Created by Romain on 30/07/2023.
//

#include "timer.h"
#include "units/cpu.h"

const u16 divmask[] = {0x200, 0x8, 0x20, 0x80};

Timer timer;

void upd(u8 value, u8 tma) {
	if (tma) ioTMA = value;
	u8 itima = ioTIMA++;
	u8 carry = !itima;
	u8 old_carry;
	
	u8 nt;
	if (tma) {
		nt = value;
	} else nt = itima;
	
	u8 flag = old_carry && !tma;
	if (flag) ioIF |= INT_TIMER;
	if (flag || tma) {
		nt = ioTMA;
	}
	
}

void clock_run2(u8 cycles) {
	if (cycles != 4 && cycles != 8) CRITICAL("", "g\n");
	do {
		timer.wdiv++;
		ioDIV = timer.div;
		
		fed_set(timer.snd_fed, (timer.div >> ((double_speed) ? 6 : 5)) & 1);
		if (fed_detect(timer.snd_fed)) // TODO timer sound
			(void)0;
		
		if (timer.reset_tima_done){
			if (timer.reset_tima_delay--) {
				timer.reset_tima_done = 0;
			}
		}
		
		
		if (timer.reset_tima_rq){ // overflow reset delayed
			if (timer.reset_tima_delay--) {
				timer.reset_tima_rq = 0;
				timer.reset_tima_done = 1;
				ioTIMA = ioTMA;
				add_interrupt(INT_TIMER);
			}
		}
		
		fed_set(timer.tma_fed, (ioTAC & 4) &&      // TAC.Enable
		       (timer.wdiv & divmask[ioTAC & 3])); // TAC.Freq
		if (fed_detect(timer.tma_fed)) {
			ioTIMA++;
			timer.reset_tima_rq = !ioTIMA;
		}
	} while(--cycles);
}


void clock_run(u8 cycles) {
	if (cycles != 4 && cycles != 8) CRITICAL("", "g\n");
	cycles >>= 2;
	do {
		timer.wdiv += 4;
		ioDIV = timer.div;
		
		fed_set(timer.snd_fed, (timer.div >> ((double_speed) ? 6 : 5)) & 1); // rework for m-Cycle
		if (fed_detect(timer.snd_fed)) // TODO timer sound
			(void)0;
		
		if (timer.reset_tima_done) timer.reset_tima_done = 0;
		
		if (timer.reset_tima_rq){ // overflow reset delayed
			timer.reset_tima_rq = 0;
			timer.reset_tima_done = 1;
			ioTIMA = ioTMA;
			add_interrupt(INT_TIMER);
		}
		
		fed_set(timer.tma_fed, (ioTAC & 4) &&      // TAC.Enable
		       (timer.wdiv & divmask[ioTAC & 3])); // TAC.Freq
		if (fed_detect(timer.tma_fed)) {
			ioTIMA++;
			timer.reset_tima_rq = !ioTIMA; // Overflow
		}
	} while(--cycles);
}
