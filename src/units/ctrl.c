//
// Created by Romain on 21/08/2023.
//

#include "ctrl.h"
#include "cpu.h"
#include "../io_ports.h"

u8 emu_ctrl_btn;
u8 pcb_ctrl_btn;

void ControllerSync(){
	if ((pcbBtn ^ pcb_ctrl_btn) & ~pcb_ctrl_btn) ioIF |= INT_JOYPAD; // Interrupt on change (high to low)
	pcbBtn = pcb_ctrl_btn;
}
