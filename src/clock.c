//
// Created by Romain on 30/07/2023.
//

#include "clock.h"
#include "units/cpu.h"

//TODO all

Divider clk_divider = {.wdiv = 0, .snd_fed = 0, .tma_fed = 0};

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

void clock_cycle() {
	clk_divider.wdiv++;
	ioDIV = clk_divider.div;
	
	fed_set(clk_divider.snd_fed, (clk_divider.div >> ((double_speed) ? 6: 5)) & 1);
	if (fed_detect(clk_divider.snd_fed)) // TODO clock sound
		(void)0;
	
	if (clk_divider.rst_tma) {
		regIE |= INT_TIMER;
		ioTIMA = ioTMA;
	}
	
	static u8 bit[] = {9, 3, 5, 7};
	fed_set(clk_divider.tma_fed, ioTAC & 4 && (clk_divider.wdiv >> bit[ioTAC & 3]) & 1);
	if (fed_detect(clk_divider.tma_fed)) {
		ioTIMA++;
		clk_divider.rst_tma = !ioTIMA;
	}
}
