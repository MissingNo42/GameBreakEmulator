//
// Created by Romain on 21/07/2023.
//

#ifndef GOBOUEMU_CPU_H
#define GOBOUEMU_CPU_H

////////////////////////  Includes  ///////////////////////////

#include "../types.h"
#include "../utils.h"


////////////////////////    Types   ///////////////////////////

#ifdef LITTLE_ENDIAN
Struct {
	union {
		struct {
			union {
				struct {u8 _:4, rc:1, rh:1, rn:1, rz:1;};
				u8 rF;
			};
			u8 rA;
		};
		u16 rAF;
	};
	union {
		struct { u8 rC, rB; };
		u16 rBC;
	};
	union {
		struct { u8 rE, rD; };
		u16 rDE;
	};
	union {
		struct { u8 rL, rH; };
		u16 rHL;
	};
	u16 rSP, rPC;
	union {
		u16 rOPERAND;
		struct { u8 rO1, rO2; };
	};
	u8 rOPCODE;
	u8 rIME: 1, rIME_DELAY: 2, rhalted: 1, rhalt_bug: 1, rbtn_selector: 2, rdouble_speed: 1;
	u8 rActionBtn: 4, rDirectionBtn: 4; // used to keep all btn, then dispatched to JOYP
} CPURegisters;
#else
Struct {
	union {
		struct {
			u8 rA;
			union {
				struct {u8 rz:1, rn:1, rh:1, rc:1, _:4;};
				u8 rF;
			};
		};
		u16 rAF;
	};
	union {
		struct { u8 rB, rC; };
		u16 rBC;
	};
	union {
		struct { u8 rD, rE; };
		u16 rDE;
	};
	union {
		struct { u8 rH, rL; };
		u16 rHL;
	};
	u16 rSP, rPC;
	union {
		u16 rOPERAND;
		struct { u8 rO2, rO1; };
	};
	u8 rOPCODE;
	u8 rIME: 1, rIME_DELAY: 2, rhalted: 1, rhalt_bug: 1;
	u8 rActionBtn: 4, rDirectionBtn: 4; // used to keep all btn, then dispatched to JOYP
} CPURegisters;
#endif


//////////////////////  Declarations  /////////////////////////

extern const char * const OPName[];
extern CPURegisters Registers;
extern u16 PCX; // debug opc's PC

////////////////////////   Macros   ///////////////////////////

#define A Registers.rA // r__ prefix avoid C-macro making trash
#define F Registers.rF
#define B Registers.rB
#define C Registers.rC
#define D Registers.rD
#define E Registers.rE
#define H Registers.rH
#define L Registers.rL
#define z Registers.rz
#define n Registers.rn
#define h Registers.rh
#define c Registers.rc
#define AF Registers.rAF
#define BC Registers.rBC
#define DE Registers.rDE
#define HL Registers.rHL
#define SP Registers.rSP
#define PC Registers.rPC
#define O1 Registers.rO1
#define O2 Registers.rO2
#define Fx Registers._ // excluded part of F b0..3 = 0000
#define OPERAND Registers.rOPERAND
#define OPCODE Registers.rOPCODE
#define IME Registers.rIME
#define IME_DELAY Registers.rIME_DELAY
#define halted Registers.rhalted
#define halt_bug Registers.rhalt_bug
#define ActionBtn Registers.rActionBtn
#define DirectionBtn Registers.rDirectionBtn
#define btn_selector Registers.rbtn_selector
#define double_speed Registers.rdouble_speed


/////////////////////  Registrations  /////////////////////////

Reset(cpu){
	for (u16 i = 0; i < (u16)sizeof(Registers); i++) ((u8*)&Registers)[i] = 0x00;
}

SaveSize(cpu, sizeof (Registers))

Save(cpu) {
	save_obj(Registers);
}

Load(cpu) {
	load_obj(Registers);
	// TODO release BTN
}


////////////////////////   Methods   //////////////////////////

void cpu_init();
void cpu_run();

#endif //GOBOUEMU_CPU_H
