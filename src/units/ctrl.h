//
// Created by Romain on 21/08/2023.
//

#ifndef GOBOUEMU_CTRL_H
#define GOBOUEMU_CTRL_H


#include "cpu.h"


#define BTN_DOWN   0x80
#define BTN_UP     0x40
#define BTN_LEFT   0x20
#define BTN_RIGHT  0x10
#define BTN_START  0x08
#define BTN_SELECT 0x04
#define BTN_A      0x02
#define BTN_B      0x01

#define ctrl_press(key, opp) emu_ctrl_btn &= ~(key); if (pcb_ctrl_btn & (opp)) pcb_ctrl_btn &= ~(key);
#define ctrl_release(key, opp) emu_ctrl_btn |= (key), pcb_ctrl_btn |= (key); if (!(emu_ctrl_btn & (opp))) pcb_ctrl_btn &= ~(opp);


extern u8 emu_ctrl_btn; // hold dir key press (emu side)
extern u8 pcb_ctrl_btn; // hold all key press (gb's pcb side = no opposite direction hold)


// Press

static inline void ControllerPress(u8 key) {
	pcb_ctrl_btn &= ~key;
}

static inline void ControllerPressUp() {
	ctrl_press(BTN_UP, BTN_DOWN);
}

static inline void ControllerPressDown() {
	ctrl_press(BTN_DOWN, BTN_UP);
}

static inline void ControllerPressRight() {
	ctrl_press(BTN_RIGHT, BTN_LEFT);
}

static inline void ControllerPressLeft() {
	ctrl_press(BTN_LEFT, BTN_RIGHT);
}

#define ControllerPressA() ControllerPress(BTN_A)
#define ControllerPressB() ControllerPress(BTN_B)
#define ControllerPressStart() ControllerPress(BTN_START)
#define ControllerPressSelect() ControllerPress(BTN_SELECT)


// Release

static inline void ControllerRelease(u8 key) {
	pcb_ctrl_btn |= key;
}

static inline void ControllerReleaseUp() {
	ctrl_release(BTN_UP, BTN_DOWN);
}

static inline void ControllerReleaseDown() {
	ctrl_release(BTN_DOWN, BTN_UP);
}

static inline void ControllerReleaseRight() {
	ctrl_release(BTN_RIGHT, BTN_LEFT);
}

static inline void ControllerReleaseLeft() {
	ctrl_release(BTN_LEFT, BTN_RIGHT);
}

#define ControllerReleaseA() ControllerRelease(BTN_A)
#define ControllerReleaseB() ControllerRelease(BTN_B)
#define ControllerReleaseStart() ControllerRelease(BTN_START)
#define ControllerReleaseSelect() ControllerRelease(BTN_SELECT)


void ControllerSync();

#undef ctrl_press
#undef ctrl_release


Reset(ctrl){
	if (hard) emu_ctrl_btn = pcb_ctrl_btn = 0xFF;
	else ControllerSync();
}

SaveSize(ctrl, 0)

Save(ctrl) {
}

Load(ctrl) {
	ControllerSync();
}

#endif //GOBOUEMU_CTRL_H
