//
// Created by Romain on 21/08/2023.
//

#include "ctrl.h"
#include "cpu.h"
#include "../io_ports.h"

u8 emu_ctrl_btn;
u8 pcb_ctrl_btn;

void ControllerSync(){
	
	u8 ActionBtn = ioP1 & 0x20 ? 0xF: pcb_ctrl_btn & 0xF;
	u8 DirectionBtn = ioP1 & 0x10 ? 0xF: pcb_ctrl_btn >> 4;
	
	u8 old = ioP1;
	ioP1 = (ioP1 & 0xF0) | (ActionBtn & DirectionBtn);
	u8 G = old;
	old ^= ioP1;
	
	//INFO("C", "%01X %01X | %02X -> %02X : %d\n", ActionBtn, DirectionBtn, G, ioP1, (ioP1 & (old & 0xF)) != (old & 0xF));
	if ((ioP1 & (old & 0xF)) != (old & 0xF)) add_interrupt(INT_JOYPAD); // Interrupt on change (high to low), even with selector change?
	
}
