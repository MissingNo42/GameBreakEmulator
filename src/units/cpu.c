//
// Created by Romain on 21/07/2023.
//

////////////////////////  Includes  ///////////////////////////

#include "cpu.h"
#include "ppu.h"
#include "mmu.h"
#include "dma.h"
#include "../timer.h"


////////////////////////    Types   ///////////////////////////

Union {
	Estruct2(u8, Low, High ;)
	u16 Reg;
} Register16;

Union {
	Estruct6(u8, _:4, _rc:1, _rh:1, _rn:1, _rz:1, __;)
	Estruct2(u8, Low, High ;)
	u16 Reg;
} Register16F;

Struct {
	Register16F reg_AF;
	Register16 reg_BC, reg_DE, reg_HL, reg_PC, reg_SP, reg_OPERAND;
	u8 reg_OPCODE;
	u8 rIME: 1, rIME_DELAY: 2, rhalted: 1, rhalt_bug: 1, rstopped: 1;
} CPURegisters;


//////////////////////  Declarations  /////////////////////////

u32 double_speed;
CPURegisters Registers;
u16 PCX = 0;


////////////////////////   Macros   ///////////////////////////

#define cpu_write(addr, value) mmu_write(addr, value); sync(4)

/// Macro for 8bits registers access (from Registers)
#define reg_A reg_AF.High
#define reg_F reg_AF.Low
#define reg_B reg_BC.High
#define reg_C reg_BC.Low
#define reg_D reg_DE.High
#define reg_E reg_DE.Low
#define reg_H reg_HL.High
#define reg_L reg_HL.Low
#define reg_O2 reg_OPERAND.High
#define reg_O1 reg_OPERAND.Low

/// Macro for registers access (from Register16 internal host register: cpu_run)
#define A  Reg_AF.High
#define Fx Reg_AF._
#define B  Reg_BC.High
#define C  Reg_BC.Low
#define D  Reg_DE.High
#define E  Reg_DE.Low
#define H  Reg_HL.High
#define L  Reg_HL.Low
#define z  Reg_AF._rz
#define n  Reg_AF._rn
#define h  Reg_AF._rh
#define c  Reg_AF._rc
#define AF Reg_AF.Reg
#define BC Reg_BC.Reg
#define DE Reg_DE.Reg
#define HL Reg_HL.Reg
#define SP Reg_SP.Reg
#define PC Reg_PC.Reg
#define OPCODE  Reg_OPCODE
#define OPERAND Reg_OPERAND.Reg
#define O1      Reg_OPERAND.Low
#define O2      Reg_OPERAND.High

#define IME          Registers.rIME
#define IME_DELAY    Registers.rIME_DELAY
#define halted       Registers.rhalted
#define stopped      Registers.rstopped
#define halt_bug     Registers.rhalt_bug


/////////////////  Registrations Methods  /////////////////////

void cpu_reset() {
	for (u16 i = 0; i < (u16)sizeof(Registers); i++) ((u8*)&Registers)[i] = 0x00;
	double_speed = 0;
}

u32 cpu_savesize() { return sizeof (double_speed) + sizeof (Registers); }

void cpu_save(void * fh) {
	save_obj(Registers);
	save_obj(double_speed);
}

void cpu_load(void * fh) {
	load_obj(Registers);
	load_obj(double_speed);
}


////////////////////////   Methods   //////////////////////////

static inline void hdma_sync() {
	do {
		u8 cycles = 4; // 4 because most requires at least 2 (4 >> 1 == 2)
		dma_sync(cycles);
		
		clock_run(cycles);
		if (double_speed) cycles >>= 1;
		hdma_run(cycles);
		
		ppu_run(cycles);
		
	} while (hdma.general);
}

static inline void sync(u8 cycles) {
	if (hdma.general && !stopped) hdma_sync();
	else {
		//if (memoryMap.bootrom_unmapped && ioTAC == 5)
		//	INFO("TM", "%02X %02X %02X %02X\n", ioDIV, ioTIMA, ioTMA, ioTAC);
		clock_run(cycles);
		dma_sync(cycles);
		
		if (double_speed) cycles >>= 1;
		
		//TODO run every units here
		ppu_run(cycles);
	}
}

static inline u8 cpu_read(u16 addr) {
	u8 r = mmu_read(addr);
	sync(4);
	return r;
}

#define UnknownOpcode() CRITICAL("Unknown Opcode", "%02X\n", OPCODE)

#define INST(code, name) i0x0##code:
#define INST_H(code, name) i0x1##code:

#define ADD(r) R = A + (r), Cx = A ^ (r) ^ R; A = R; c = Cx >> 8; z = !A; h = Cx >> 4; n = 0
#define ADC(r) R = A + (r) + c, Cx = A ^ (r) ^ R; A = R; c = Cx >> 8; z = !A; h = Cx >> 4; n = 0

//TODO REQ s32 not u32 -> error??
#define SUB(r) R = A - (r), Cx = A ^ (r) ^ R; A = R; c = Cx >> 8; z = !A; h = Cx >> 4; n = 1; sync(4)
#define SBC(r) R = A - (r) - c, Cx = A ^ (r) ^ R; A = R; c = Cx >> 8; z = !A; h = Cx >> 4; n = 1; sync(4)

#define ADD_SP(r) Cx = SP ^ (r) ^ (SP + (r)); SP = SP + (s8)(r); c = Cx >> 8; h = Cx >> 4; z = n = 0; sync(8)
#define ADD_SP_d(r, dst) Cx = SP ^ (r) ^ (SP + (r)); (dst) = SP + (s8)(r); c = Cx >> 8; h = Cx >> 4; z = n = 0; sync(4)
#define ADD_HL(r) R = HL + (r), Cx = HL ^ (r) ^ R; HL = R; c = Cx >> 16; h = Cx >> 12; n = 0; sync(4)

#define INC(r) h = ((r) & 0x0F) == 0x0F; (r)++; z = !(r); n = 0
#define DEC(r) h = !((r) & 0x0F); (r)--; z = !(r); n = 1

#define AND(r) A &= (r); c = n = 0, h = 1, z = !A
#define XOR(r) A ^= (r); c = n = h = 0, z = !A
#define OR(r) A |= (r); c = n = h = 0, z = !A
#define CP(r) z = A == (r), n = 1, h = (A & 0x0F) < ((r) & 0x0F), c = A < r

#define RL(r) u8 A7 = (r) >> 7; (r) = (r) << 1 | c, c = A7, z = !(r), n = h = 0
#define RLA() u8 A7 = A >> 7; A = A << 1 | c, c = A7, z = n = h = 0
#define RLC(r) c = (r) >> 7; (r) = (r) << 1 | c, z = !(r), n = h = 0
#define RLCA() c = A >> 7; A = A << 1 | c, z = n = h = 0

#define RR(r) u8 A0 = (r) & 1; (r) = (r) >> 1 | c << 7, c = A0, z = !(r), n = h = 0
#define RRA() u8 A0 = A & 1; A = A >> 1 | c << 7, c = A0, z = n = h = 0
#define RRC(r) c = (r) & 1; (r) = (r) >> 1 | c << 7, z = !(r), n = h = 0
#define RRCA() c = A & 1; A = A >> 1 | c << 7, z = n = h = 0

#define SLA(r) c = (r) >> 7; (r) <<= 1; z = !(r), n = h = 0
#define SRA(r) c = (r) & 1; (r) = ((r) >> 1) | ((r) & 0x80); z = !(r), n = h = 0
#define SRL(r) c = (r) & 1; (r) >>= 1; z = !(r), n = h = 0

#define SWAP(r) (r) = ((r) >> 4) | ((r) << 4); z = !(r), h = c = n = 0
#define BIT(k, r) z = !(((r) >> (k)) & 1), n = 0, h = 1
#define RES(k, r) (r) &= ~(1 << (k))
#define SET(k, r) (r) |= 1 << k

#define SCF() c = 1, n = h = 0
#define CCF() c = ~c, n = h = 0
#define CPL() A = ~A, n = h = 1

#define PUSH8(r) cpu_write(--SP, (u8)(r))
#define PUSH16(r) PUSH8((r) >> 8); PUSH8(r)
#define POP8() cpu_read(SP++)
//#define POP16() POP8() | (POP8() << 8)
#define exp_POP16(dst) dst = POP8(); dst |= (POP8() << 8)

/*
static inline void PUSH16(u16 r){
	PUSH8(r >> 8);
	PUSH8(r);
}*/


static inline u16 POP16(){
	//s32 r = POP8();
	//return (u16)(r | (POP8() << 8));
}


///////////////// INSTRUCTION /////////////////

#pragma region AUTOGENERATED


static u8 OPSize[] = {
	0, 2, 0, 0, 0, 0, 1, 0, 2, 0, 0, 0, 0, 0, 1, 0, // 00
	1, 2, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, // 10
	1, 2, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, // 20
	1, 2, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, // 30
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 40
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 50
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 60
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 70
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 80
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 90
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // A0
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // B0
	0, 0, 2, 2, 2, 0, 1, 0, 0, 0, 2, 1, 2, 2, 1, 0, // C0
	0, 0, 2, 0, 2, 0, 1, 0, 0, 0, 2, 0, 2, 0, 1, 0, // D0
	1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 2, 0, 0, 0, 1, 0, // E0
	1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 2, 0, 0, 0, 1, 0, // F0
};

const char * const OPName[] = {
	"NOP        ", "LD BC,d16  ", "LD (BC),A  ", "INC BC     ", "INC B      ", "DEC B      ", "LD B,d8    ", "RLCA       ", "LD (a16),SP", "ADD HL,BC  ", "LD A,(BC)  ", "DEC BC     ", "INC C      ", "DEC C      ", "LD C,d8    ", "RRCA       ", // 00
	"STOP 0     ", "LD DE,d16  ", "LD (DE),A  ", "INC DE     ", "INC D      ", "DEC D      ", "LD D,d8    ", "RLA        ", "JR r8      ", "ADD HL,DE  ", "LD A,(DE)  ", "DEC DE     ", "INC E      ", "DEC E      ", "LD E,d8    ", "RRA        ", // 10
	"JR NZ,r8   ", "LD HL,d16  ", "LD (HL+),A ", "INC HL     ", "INC H      ", "DEC H      ", "LD H,d8    ", "DAA        ", "JR Z,r8    ", "ADD HL,HL  ", "LD A,(HL+) ", "DEC HL     ", "INC L      ", "DEC L      ", "LD L,d8    ", "CPL        ", // 20
	"JR NC,r8   ", "LD SP,d16  ", "LD (HL-),A ", "INC SP     ", "INC (HL)   ", "DEC (HL)   ", "LD (HL),d8 ", "SCF        ", "JR C,r8    ", "ADD HL,SP  ", "LD A,(HL-) ", "DEC SP     ", "INC A      ", "DEC A      ", "LD A,d8    ", "CCF        ", // 30
	"LD B,B     ", "LD B,C     ", "LD B,D     ", "LD B,E     ", "LD B,H     ", "LD B,L     ", "LD B,(HL)  ", "LD B,A     ", "LD C,B     ", "LD C,C     ", "LD C,D     ", "LD C,E     ", "LD C,H     ", "LD C,L     ", "LD C,(HL)  ", "LD C,A     ", // 40
	"LD D,B     ", "LD D,C     ", "LD D,D     ", "LD D,E     ", "LD D,H     ", "LD D,L     ", "LD D,(HL)  ", "LD D,A     ", "LD E,B     ", "LD E,C     ", "LD E,D     ", "LD E,E     ", "LD E,H     ", "LD E,L     ", "LD E,(HL)  ", "LD E,A     ", // 50
	"LD H,B     ", "LD H,C     ", "LD H,D     ", "LD H,E     ", "LD H,H     ", "LD H,L     ", "LD H,(HL)  ", "LD H,A     ", "LD L,B     ", "LD L,C     ", "LD L,D     ", "LD L,E     ", "LD L,H     ", "LD L,L     ", "LD L,(HL)  ", "LD L,A     ", // 60
	"LD (HL),B  ", "LD (HL),C  ", "LD (HL),D  ", "LD (HL),E  ", "LD (HL),H  ", "LD (HL),L  ", "HALT       ", "LD (HL),A  ", "LD A,B     ", "LD A,C     ", "LD A,D     ", "LD A,E     ", "LD A,H     ", "LD A,L     ", "LD A,(HL)  ", "LD A,A     ", // 70
	"ADD A,B    ", "ADD A,C    ", "ADD A,D    ", "ADD A,E    ", "ADD A,H    ", "ADD A,L    ", "ADD A,(HL) ", "ADD A,A    ", "ADC A,B    ", "ADC A,C    ", "ADC A,D    ", "ADC A,E    ", "ADC A,H    ", "ADC A,L    ", "ADC A,(HL) ", "ADC A,A    ", // 80
	"SUB B      ", "SUB C      ", "SUB D      ", "SUB E      ", "SUB H      ", "SUB L      ", "SUB (HL)   ", "SUB A      ", "SBC A,B    ", "SBC A,C    ", "SBC A,D    ", "SBC A,E    ", "SBC A,H    ", "SBC A,L    ", "SBC A,(HL) ", "SBC A,A    ", // 90
	"AND B      ", "AND C      ", "AND D      ", "AND E      ", "AND H      ", "AND L      ", "AND (HL)   ", "AND A      ", "XOR B      ", "XOR C      ", "XOR D      ", "XOR E      ", "XOR H      ", "XOR L      ", "XOR (HL)   ", "XOR A      ", // A0
	"OR B       ", "OR C       ", "OR D       ", "OR E       ", "OR H       ", "OR L       ", "OR (HL)    ", "OR A       ", "CP B       ", "CP C       ", "CP D       ", "CP E       ", "CP H       ", "CP L       ", "CP (HL)    ", "CP A       ", // B0
	"RET NZ     ", "POP BC     ", "JP NZ,a16  ", "JP a16     ", "CALL NZ,a16", "PUSH BC    ", "ADD A,d8   ", "RST 00H    ", "RET Z      ", "RET        ", "JP Z,a16   ", "PREFIX CB  ", "CALL Z,a16 ", "CALL a16   ", "ADC A,d8   ", "RST 08H    ", // C0
	"RET NC     ", "POP DE     ", "JP NC,a16  ", "UNKN       ", "CALL NC,a16", "PUSH DE    ", "SUB d8     ", "RST 10H    ", "RET C      ", "RETI       ", "JP C,a16   ", "UNKN       ", "CALL C,a16 ", "UNKN       ", "SBC A,d8   ", "RST 18H    ", // D0
	"LDH (a8),A ", "POP HL     ", "LD (C),A   ", "UNKN       ", "UNKN       ", "PUSH HL    ", "AND d8     ", "RST 20H    ", "ADD SP,r8  ", "JP (HL)    ", "LD (a16),A ", "UNKN       ", "UNKN       ", "UNKN       ", "XOR d8     ", "RST 28H    ", // E0
	"LDH A,(a8) ", "POP AF     ", "LD A,(C)   ", "DI         ", "UNKN       ", "PUSH AF    ", "OR d8      ", "RST 30H    ", "LD HL,SP+r8", "LD SP,HL   ", "LD A,(a16) ", "EI         ", "UNKN       ", "UNKN       ", "CP d8      ", "RST 38H    ", // F0
	"RLC B      ", "RLC C      ", "RLC D      ", "RLC E      ", "RLC H      ", "RLC L      ", "RLC (HL)   ", "RLC A      ", "RRC B      ", "RRC C      ", "RRC D      ", "RRC E      ", "RRC H      ", "RRC L      ", "RRC (HL)   ", "RRC A      ", // 00
	"RL B       ", "RL C       ", "RL D       ", "RL E       ", "RL H       ", "RL L       ", "RL (HL)    ", "RL A       ", "RR B       ", "RR C       ", "RR D       ", "RR E       ", "RR H       ", "RR L       ", "RR (HL)    ", "RR A       ", // 10
	"SLA B      ", "SLA C      ", "SLA D      ", "SLA E      ", "SLA H      ", "SLA L      ", "SLA (HL)   ", "SLA A      ", "SRA B      ", "SRA C      ", "SRA D      ", "SRA E      ", "SRA H      ", "SRA L      ", "SRA (HL)   ", "SRA A      ", // 20
	"SWAP B     ", "SWAP C     ", "SWAP D     ", "SWAP E     ", "SWAP H     ", "SWAP L     ", "SWAP (HL)  ", "SWAP A     ", "SRL B      ", "SRL C      ", "SRL D      ", "SRL E      ", "SRL H      ", "SRL L      ", "SRL (HL)   ", "SRL A      ", // 30
	"BIT 0,B    ", "BIT 0,C    ", "BIT 0,D    ", "BIT 0,E    ", "BIT 0,H    ", "BIT 0,L    ", "BIT 0,(HL) ", "BIT 0,A    ", "BIT 1,B    ", "BIT 1,C    ", "BIT 1,D    ", "BIT 1,E    ", "BIT 1,H    ", "BIT 1,L    ", "BIT 1,(HL) ", "BIT 1,A    ", // 40
	"BIT 2,B    ", "BIT 2,C    ", "BIT 2,D    ", "BIT 2,E    ", "BIT 2,H    ", "BIT 2,L    ", "BIT 2,(HL) ", "BIT 2,A    ", "BIT 3,B    ", "BIT 3,C    ", "BIT 3,D    ", "BIT 3,E    ", "BIT 3,H    ", "BIT 3,L    ", "BIT 3,(HL) ", "BIT 3,A    ", // 50
	"BIT 4,B    ", "BIT 4,C    ", "BIT 4,D    ", "BIT 4,E    ", "BIT 4,H    ", "BIT 4,L    ", "BIT 4,(HL) ", "BIT 4,A    ", "BIT 5,B    ", "BIT 5,C    ", "BIT 5,D    ", "BIT 5,E    ", "BIT 5,H    ", "BIT 5,L    ", "BIT 5,(HL) ", "BIT 5,A    ", // 60
	"BIT 6,B    ", "BIT 6,C    ", "BIT 6,D    ", "BIT 6,E    ", "BIT 6,H    ", "BIT 6,L    ", "BIT 6,(HL) ", "BIT 6,A    ", "BIT 7,B    ", "BIT 7,C    ", "BIT 7,D    ", "BIT 7,E    ", "BIT 7,H    ", "BIT 7,L    ", "BIT 7,(HL) ", "BIT 7,A    ", // 70
	"RES 0,B    ", "RES 0,C    ", "RES 0,D    ", "RES 0,E    ", "RES 0,H    ", "RES 0,L    ", "RES 0,(HL) ", "RES 0,A    ", "RES 1,B    ", "RES 1,C    ", "RES 1,D    ", "RES 1,E    ", "RES 1,H    ", "RES 1,L    ", "RES 1,(HL) ", "RES 1,A    ", // 80
	"RES 2,B    ", "RES 2,C    ", "RES 2,D    ", "RES 2,E    ", "RES 2,H    ", "RES 2,L    ", "RES 2,(HL) ", "RES 2,A    ", "RES 3,B    ", "RES 3,C    ", "RES 3,D    ", "RES 3,E    ", "RES 3,H    ", "RES 3,L    ", "RES 3,(HL) ", "RES 3,A    ", // 90
	"RES 4,B    ", "RES 4,C    ", "RES 4,D    ", "RES 4,E    ", "RES 4,H    ", "RES 4,L    ", "RES 4,(HL) ", "RES 4,A    ", "RES 5,B    ", "RES 5,C    ", "RES 5,D    ", "RES 5,E    ", "RES 5,H    ", "RES 5,L    ", "RES 5,(HL) ", "RES 5,A    ", // A0
	"RES 6,B    ", "RES 6,C    ", "RES 6,D    ", "RES 6,E    ", "RES 6,H    ", "RES 6,L    ", "RES 6,(HL) ", "RES 6,A    ", "RES 7,B    ", "RES 7,C    ", "RES 7,D    ", "RES 7,E    ", "RES 7,H    ", "RES 7,L    ", "RES 7,(HL) ", "RES 7,A    ", // B0
	"SET 0,B    ", "SET 0,C    ", "SET 0,D    ", "SET 0,E    ", "SET 0,H    ", "SET 0,L    ", "SET 0,(HL) ", "SET 0,A    ", "SET 1,B    ", "SET 1,C    ", "SET 1,D    ", "SET 1,E    ", "SET 1,H    ", "SET 1,L    ", "SET 1,(HL) ", "SET 1,A    ", // C0
	"SET 2,B    ", "SET 2,C    ", "SET 2,D    ", "SET 2,E    ", "SET 2,H    ", "SET 2,L    ", "SET 2,(HL) ", "SET 2,A    ", "SET 3,B    ", "SET 3,C    ", "SET 3,D    ", "SET 3,E    ", "SET 3,H    ", "SET 3,L    ", "SET 3,(HL) ", "SET 3,A    ", // D0
	"SET 4,B    ", "SET 4,C    ", "SET 4,D    ", "SET 4,E    ", "SET 4,H    ", "SET 4,L    ", "SET 4,(HL) ", "SET 4,A    ", "SET 5,B    ", "SET 5,C    ", "SET 5,D    ", "SET 5,E    ", "SET 5,H    ", "SET 5,L    ", "SET 5,(HL) ", "SET 5,A    ", // E0
	"SET 6,B    ", "SET 6,C    ", "SET 6,D    ", "SET 6,E    ", "SET 6,H    ", "SET 6,L    ", "SET 6,(HL) ", "SET 6,A    ", "SET 7,B    ", "SET 7,C    ", "SET 7,D    ", "SET 7,E    ", "SET 7,H    ", "SET 7,L    ", "SET 7,(HL) ", "SET 7,A    ", // F0
};


#pragma endregion

#pragma endregion

///////////////// CPU /////////////////

void cpu_init() {
	//A = GBC ? 0x11: 0x01;
	//F = 0xB0, BC = 0x13, DE = 0xD8, HL = 0x014D, SP = 0xFFFE, PC = 0x100;
	IME = halted = halt_bug = IME_DELAY = double_speed = 0;
}

static inline void cpu_debug(Register16F Reg_AF, Register16 Reg_BC, Register16 Reg_DE, Register16 Reg_HL, Register16 Reg_PC, Register16 Reg_SP, Register16 Reg_OPERAND, u16 Reg_OPCODE) {
	static u32 i = 0;
	if (memoryMap.bootrom_unmapped) {
		//LogInst();
		CRITICAL("\t",
		      "[(%02X): %s | %04X] %u \t A %02X | BC %04X | DE %04X | HL %04X | SP %04X | PC %04X [%c %c %c %c]  ( LY %hhu 0x%02X) <%02X %02X> (%02X %02X %02X %02X) %hhu\n",
			  OPCODE, OPCODE == 0xCB ? OPName[0x100 | O1]: OPName[OPCODE], OPERAND,
		      i++, A, BC, DE, HL, SP, PC,
		      (z) ? 'Z' : 'z', (n) ? 'N' : 'n', (h) ? 'H' : 'h', (c) ? 'C' : 'c',
		      ioLY, ioLY, ioIF, read_ie(), ioDIV, ioTIMA, ioTMA, ioLCDC, PPU_MODE);
	}
}


void cpu_run() { // run 1 frame
	
	/// Copy GBZ80 Registers to the Emu Host CPU Registers
	
	register Register16F Reg_AF = Registers.reg_AF;
	register Register16 Reg_BC  = Registers.reg_BC;
	register Register16 Reg_DE  = Registers.reg_DE;
	register Register16 Reg_HL  = Registers.reg_HL;
	register Register16 Reg_PC  = Registers.reg_PC;
	register Register16 Reg_SP  = Registers.reg_SP;
	register Register16 Reg_OPERAND = Registers.reg_OPERAND;
	register u16 Reg_OPCODE         = Registers.reg_OPCODE;
	register u8 r;
	register u32 R;
	register u32 Cx;
	
	
	/// Implement the GBZ80 as a jumptable (faster than optimized switch or calltable)
	
	const static void * Instructions[] = {
			&&i0x000, &&i0x001, &&i0x002, &&i0x003, &&i0x004, &&i0x005, &&i0x006, &&i0x007, &&i0x008, &&i0x009, &&i0x00A, &&i0x00B, &&i0x00C, &&i0x00D, &&i0x00E, &&i0x00F,
			&&i0x010, &&i0x011, &&i0x012, &&i0x013, &&i0x014, &&i0x015, &&i0x016, &&i0x017, &&i0x018, &&i0x019, &&i0x01A, &&i0x01B, &&i0x01C, &&i0x01D, &&i0x01E, &&i0x01F,
			&&i0x020, &&i0x021, &&i0x022, &&i0x023, &&i0x024, &&i0x025, &&i0x026, &&i0x027, &&i0x028, &&i0x029, &&i0x02A, &&i0x02B, &&i0x02C, &&i0x02D, &&i0x02E, &&i0x02F,
			&&i0x030, &&i0x031, &&i0x032, &&i0x033, &&i0x034, &&i0x035, &&i0x036, &&i0x037, &&i0x038, &&i0x039, &&i0x03A, &&i0x03B, &&i0x03C, &&i0x03D, &&i0x03E, &&i0x03F,
			&&i0x040, &&i0x041, &&i0x042, &&i0x043, &&i0x044, &&i0x045, &&i0x046, &&i0x047, &&i0x048, &&i0x049, &&i0x04A, &&i0x04B, &&i0x04C, &&i0x04D, &&i0x04E, &&i0x04F,
			&&i0x050, &&i0x051, &&i0x052, &&i0x053, &&i0x054, &&i0x055, &&i0x056, &&i0x057, &&i0x058, &&i0x059, &&i0x05A, &&i0x05B, &&i0x05C, &&i0x05D, &&i0x05E, &&i0x05F,
			&&i0x060, &&i0x061, &&i0x062, &&i0x063, &&i0x064, &&i0x065, &&i0x066, &&i0x067, &&i0x068, &&i0x069, &&i0x06A, &&i0x06B, &&i0x06C, &&i0x06D, &&i0x06E, &&i0x06F,
			&&i0x070, &&i0x071, &&i0x072, &&i0x073, &&i0x074, &&i0x075, &&i0x076, &&i0x077, &&i0x078, &&i0x079, &&i0x07A, &&i0x07B, &&i0x07C, &&i0x07D, &&i0x07E, &&i0x07F,
			&&i0x080, &&i0x081, &&i0x082, &&i0x083, &&i0x084, &&i0x085, &&i0x086, &&i0x087, &&i0x088, &&i0x089, &&i0x08A, &&i0x08B, &&i0x08C, &&i0x08D, &&i0x08E, &&i0x08F,
			&&i0x090, &&i0x091, &&i0x092, &&i0x093, &&i0x094, &&i0x095, &&i0x096, &&i0x097, &&i0x098, &&i0x099, &&i0x09A, &&i0x09B, &&i0x09C, &&i0x09D, &&i0x09E, &&i0x09F,
			&&i0x0A0, &&i0x0A1, &&i0x0A2, &&i0x0A3, &&i0x0A4, &&i0x0A5, &&i0x0A6, &&i0x0A7, &&i0x0A8, &&i0x0A9, &&i0x0AA, &&i0x0AB, &&i0x0AC, &&i0x0AD, &&i0x0AE, &&i0x0AF,
			&&i0x0B0, &&i0x0B1, &&i0x0B2, &&i0x0B3, &&i0x0B4, &&i0x0B5, &&i0x0B6, &&i0x0B7, &&i0x0B8, &&i0x0B9, &&i0x0BA, &&i0x0BB, &&i0x0BC, &&i0x0BD, &&i0x0BE, &&i0x0BF,
			&&i0x0C0, &&i0x0C1, &&i0x0C2, &&i0x0C3, &&i0x0C4, &&i0x0C5, &&i0x0C6, &&i0x0C7, &&i0x0C8, &&i0x0C9, &&i0x0CA, &&i0x0CB, &&i0x0CC, &&i0x0CD, &&i0x0CE, &&i0x0CF,
			&&i0x0D0, &&i0x0D1, &&i0x0D2, &&i0x0D3, &&i0x0D4, &&i0x0D5, &&i0x0D6, &&i0x0D7, &&i0x0D8, &&i0x0D9, &&i0x0DA, &&i0x0DB, &&i0x0DC, &&i0x0DD, &&i0x0DE, &&i0x0DF,
			&&i0x0E0, &&i0x0E1, &&i0x0E2, &&i0x0E3, &&i0x0E4, &&i0x0E5, &&i0x0E6, &&i0x0E7, &&i0x0E8, &&i0x0E9, &&i0x0EA, &&i0x0EB, &&i0x0EC, &&i0x0ED, &&i0x0EE, &&i0x0EF,
			&&i0x0F0, &&i0x0F1, &&i0x0F2, &&i0x0F3, &&i0x0F4, &&i0x0F5, &&i0x0F6, &&i0x0F7, &&i0x0F8, &&i0x0F9, &&i0x0FA, &&i0x0FB, &&i0x0FC, &&i0x0FD, &&i0x0FE, &&i0x0FF,
			&&i0x100, &&i0x101, &&i0x102, &&i0x103, &&i0x104, &&i0x105, &&i0x106, &&i0x107, &&i0x108, &&i0x109, &&i0x10A, &&i0x10B, &&i0x10C, &&i0x10D, &&i0x10E, &&i0x10F,
			&&i0x110, &&i0x111, &&i0x112, &&i0x113, &&i0x114, &&i0x115, &&i0x116, &&i0x117, &&i0x118, &&i0x119, &&i0x11A, &&i0x11B, &&i0x11C, &&i0x11D, &&i0x11E, &&i0x11F,
			&&i0x120, &&i0x121, &&i0x122, &&i0x123, &&i0x124, &&i0x125, &&i0x126, &&i0x127, &&i0x128, &&i0x129, &&i0x12A, &&i0x12B, &&i0x12C, &&i0x12D, &&i0x12E, &&i0x12F,
			&&i0x130, &&i0x131, &&i0x132, &&i0x133, &&i0x134, &&i0x135, &&i0x136, &&i0x137, &&i0x138, &&i0x139, &&i0x13A, &&i0x13B, &&i0x13C, &&i0x13D, &&i0x13E, &&i0x13F,
			&&i0x140, &&i0x141, &&i0x142, &&i0x143, &&i0x144, &&i0x145, &&i0x146, &&i0x147, &&i0x148, &&i0x149, &&i0x14A, &&i0x14B, &&i0x14C, &&i0x14D, &&i0x14E, &&i0x14F,
			&&i0x150, &&i0x151, &&i0x152, &&i0x153, &&i0x154, &&i0x155, &&i0x156, &&i0x157, &&i0x158, &&i0x159, &&i0x15A, &&i0x15B, &&i0x15C, &&i0x15D, &&i0x15E, &&i0x15F,
			&&i0x160, &&i0x161, &&i0x162, &&i0x163, &&i0x164, &&i0x165, &&i0x166, &&i0x167, &&i0x168, &&i0x169, &&i0x16A, &&i0x16B, &&i0x16C, &&i0x16D, &&i0x16E, &&i0x16F,
			&&i0x170, &&i0x171, &&i0x172, &&i0x173, &&i0x174, &&i0x175, &&i0x176, &&i0x177, &&i0x178, &&i0x179, &&i0x17A, &&i0x17B, &&i0x17C, &&i0x17D, &&i0x17E, &&i0x17F,
			&&i0x180, &&i0x181, &&i0x182, &&i0x183, &&i0x184, &&i0x185, &&i0x186, &&i0x187, &&i0x188, &&i0x189, &&i0x18A, &&i0x18B, &&i0x18C, &&i0x18D, &&i0x18E, &&i0x18F,
			&&i0x190, &&i0x191, &&i0x192, &&i0x193, &&i0x194, &&i0x195, &&i0x196, &&i0x197, &&i0x198, &&i0x199, &&i0x19A, &&i0x19B, &&i0x19C, &&i0x19D, &&i0x19E, &&i0x19F,
			&&i0x1A0, &&i0x1A1, &&i0x1A2, &&i0x1A3, &&i0x1A4, &&i0x1A5, &&i0x1A6, &&i0x1A7, &&i0x1A8, &&i0x1A9, &&i0x1AA, &&i0x1AB, &&i0x1AC, &&i0x1AD, &&i0x1AE, &&i0x1AF,
			&&i0x1B0, &&i0x1B1, &&i0x1B2, &&i0x1B3, &&i0x1B4, &&i0x1B5, &&i0x1B6, &&i0x1B7, &&i0x1B8, &&i0x1B9, &&i0x1BA, &&i0x1BB, &&i0x1BC, &&i0x1BD, &&i0x1BE, &&i0x1BF,
			&&i0x1C0, &&i0x1C1, &&i0x1C2, &&i0x1C3, &&i0x1C4, &&i0x1C5, &&i0x1C6, &&i0x1C7, &&i0x1C8, &&i0x1C9, &&i0x1CA, &&i0x1CB, &&i0x1CC, &&i0x1CD, &&i0x1CE, &&i0x1CF,
			&&i0x1D0, &&i0x1D1, &&i0x1D2, &&i0x1D3, &&i0x1D4, &&i0x1D5, &&i0x1D6, &&i0x1D7, &&i0x1D8, &&i0x1D9, &&i0x1DA, &&i0x1DB, &&i0x1DC, &&i0x1DD, &&i0x1DE, &&i0x1DF,
			&&i0x1E0, &&i0x1E1, &&i0x1E2, &&i0x1E3, &&i0x1E4, &&i0x1E5, &&i0x1E6, &&i0x1E7, &&i0x1E8, &&i0x1E9, &&i0x1EA, &&i0x1EB, &&i0x1EC, &&i0x1ED, &&i0x1EE, &&i0x1EF,
			&&i0x1F0, &&i0x1F1, &&i0x1F2, &&i0x1F3, &&i0x1F4, &&i0x1F5, &&i0x1F6, &&i0x1F7, &&i0x1F8, &&i0x1F9, &&i0x1FA, &&i0x1FB, &&i0x1FC, &&i0x1FD, &&i0x1FE, &&i0x1FF,
	};
	
	do {
		// Debug
		//cpu_debug(Reg_AF, Reg_BC, Reg_DE, Reg_HL, Reg_PC, Reg_SP, Reg_OPERAND, Reg_OPCODE);
		
		
		// Update IME State
		
		if (IME_DELAY & 1)
			IME = 1; // assumes that EI, EI, ..., EI will take effect at the end of the 2nd, and not the last
		IME_DELAY >>= 1;
		
		
		/// ISR: Interrupt Service Routine
		
		u8 flag = ioIF & read_ie() & 0x1f;
		if (flag) {
			if (halted) {
				halted = 0; // Disable HALT
				PC++;       // Forward after HALT
			}
			if (IME) {
				sync(8); // TODO: depend on speed? (= always 8 cycles ppu?)
				IME = 0;
				
				u8 log = 0;
				while (!(flag & (1 << log))) log++;
				
				ioIF = (ioIF & (~(1 << log))) | 0xe0;
				
				if (halt_bug) halt_bug = 0, PC--; // Push the HALT addr, not the following (EI -> HALT Sequence 'halt bug')
				PUSH16(PC);
				PCX = PC;
				PC = 0x40 | (log << 3); // INT Vec jump
				//CRITICAL("INT TO", "%02X from %04X\n", PC, PCX);
				sync(4);
			}
		}
		
		
		/// Fetch Instruction's Opcode
		
		PCX = PC;
		OPCODE = cpu_read(PC);
		if (halt_bug) halt_bug = 0; // dont increment PC once
		else PC++;
		
		
		/// Fetch Instruction's Operand
		
		u8 sz;
		if ((sz = OPSize[OPCODE])) {
			O1 = cpu_read(PC++);
			O2 = (sz == 2) ? cpu_read(PC++): 0;
		}
		
		
		/// Execute GBZ80 Instruction
		
		inst: goto *Instructions[OPCODE];
		
		#pragma region Low Instructions
			
			INST(88, ADC_A_B) {
				ADC(B);
				continue;
			}  // 88 = ADC A,B	 | Fg: Z 0 H C | Sz: 1 | Cc: 4
			
			INST(89, ADC_A_C) {
				ADC(C);
				continue;
			}  // 89 = ADC A,C	 | Fg: Z 0 H C | Sz: 1 | Cc: 4
			
			INST(8A, ADC_A_D) {
				ADC(D);
				continue;
			}  // 8A = ADC A,D	 | Fg: Z 0 H C | Sz: 1 | Cc: 4
			
			INST(8B, ADC_A_E) {
				ADC(E);
				continue;
			}  // 8B = ADC A,E	 | Fg: Z 0 H C | Sz: 1 | Cc: 4
			
			INST(8C, ADC_A_H) {
				ADC(H);
				continue;
			}  // 8C = ADC A,H	 | Fg: Z 0 H C | Sz: 1 | Cc: 4
			
			INST(8D, ADC_A_L) {
				ADC(L);
				continue;
			}  // 8D = ADC A,L	 | Fg: Z 0 H C | Sz: 1 | Cc: 4
			
			INST(8E, ADC_A_(HL)) {
				r = cpu_read(HL);
				ADC(r);
				continue;
			}  // 8E = ADC A,(HL)	 | Fg: Z 0 H C | Sz: 1 | Cc: 8
			
			INST(8F, ADC_A_A) {
				ADC(A);
				continue;
			}  // 8F = ADC A,A	 | Fg: Z 0 H C | Sz: 1 | Cc: 4
			
			INST(CE, ADC_A_D8) {
				ADC(O1);
				continue;
			}  // CE = ADC A,d8	 | Fg: Z 0 H C | Sz: 2 | Cc: 8
			
			INST(09, ADD_HL_BC) {
				ADD_HL(BC);
				continue;
			}  // 09 = ADD HL,BC	 | Fg: - 0 H C | Sz: 1 | Cc: 8
			
			INST(19, ADD_HL_DE) {
				ADD_HL(DE);
				continue;
			}  // 19 = ADD HL,DE	 | Fg: - 0 H C | Sz: 1 | Cc: 8
			
			INST(29, ADD_HL_HL) {
				ADD_HL(HL);
				continue;
			}  // 29 = ADD HL,HL	 | Fg: - 0 H C | Sz: 1 | Cc: 8
			
			INST(39, ADD_HL_SP) {
				ADD_HL(SP);
				continue;
			}  // 39 = ADD HL,SP	 | Fg: - 0 H C | Sz: 1 | Cc: 8
			
			INST(80, ADD_A_B) {
				ADD(B);
				continue;
			}  // 80 = ADD A,B	 | Fg: Z 0 H C | Sz: 1 | Cc: 4
			
			INST(81, ADD_A_C) {
				ADD(C);
				continue;
			}  // 81 = ADD A,C	 | Fg: Z 0 H C | Sz: 1 | Cc: 4
			
			INST(82, ADD_A_D) {
				ADD(D);
				continue;
			}  // 82 = ADD A,D	 | Fg: Z 0 H C | Sz: 1 | Cc: 4
			
			INST(83, ADD_A_E) {
				ADD(E);
				continue;
			}  // 83 = ADD A,E	 | Fg: Z 0 H C | Sz: 1 | Cc: 4
			
			INST(84, ADD_A_H) {
				ADD(H);
				continue;
			}  // 84 = ADD A,H	 | Fg: Z 0 H C | Sz: 1 | Cc: 4
			
			INST(85, ADD_A_L) {
				ADD(L);
				continue;
			}  // 85 = ADD A,L	 | Fg: Z 0 H C | Sz: 1 | Cc: 4
			
			INST(86, ADD_A_(HL)) {
				r = cpu_read(HL);
				ADD(r);
				continue;
			}  // 86 = ADD A,(HL)	 | Fg: Z 0 H C | Sz: 1 | Cc: 8
			
			INST(87, ADD_A_A) {
				ADD(A);
				continue;
			}  // 87 = ADD A,A	 | Fg: Z 0 H C | Sz: 1 | Cc: 4
			
			INST(C6, ADD_A_D8) {
				ADD(O1);
				continue;
			}  // C6 = ADD A,d8	 | Fg: Z 0 H C | Sz: 2 | Cc: 8
			
			INST(E8, ADD_SP_R8) {
				ADD_SP(O1);
				continue;
			}  // E8 = ADD SP,r8	 | Fg: 0 0 H C | Sz: 2 | Cc: 16
			
			INST(A0, AND_B) {
				AND(B);
				continue;
			}  // A0 = AND B	 | Fg: Z 0 1 0 | Sz: 1 | Cc: 4
			
			INST(A1, AND_C) {
				AND(C);
				continue;
			}  // A1 = AND C	 | Fg: Z 0 1 0 | Sz: 1 | Cc: 4
			
			INST(A2, AND_D) {
				AND(D);
				continue;
			}  // A2 = AND D	 | Fg: Z 0 1 0 | Sz: 1 | Cc: 4
			
			INST(A3, AND_E) {
				AND(E);
				continue;
			}  // A3 = AND E	 | Fg: Z 0 1 0 | Sz: 1 | Cc: 4
			
			INST(A4, AND_H) {
				AND(H);
				continue;
			}  // A4 = AND H	 | Fg: Z 0 1 0 | Sz: 1 | Cc: 4
			
			INST(A5, AND_L) {
				AND(L);
				continue;
			}  // A5 = AND L	 | Fg: Z 0 1 0 | Sz: 1 | Cc: 4
			
			INST(A6, AND_(HL)) {
				AND(cpu_read(HL));
				continue;
			}  // A6 = AND (HL)	 | Fg: Z 0 1 0 | Sz: 1 | Cc: 8
			
			INST(A7, AND_A) {
				AND(A);
				continue;
			}  // A7 = AND A	 | Fg: Z 0 1 0 | Sz: 1 | Cc: 4
			
			INST(E6, AND_D8) {
				AND(O1);
				continue;
			}  // E6 = AND d8	 | Fg: Z 0 1 0 | Sz: 2 | Cc: 8
			
			INST(C4, CALL_NZ_A16) {
				if (!z) {
					PUSH16(PC);
					PC = OPERAND;
					sync(4);
				}
				continue;
			}  // C4 = CALL NZ,a16	 | Fg: - - - - | Sz: 3 | Cc: 24/12
			
			INST(CC, CALL_Z_A16) {
				if (z) {
					PUSH16(PC);
					PC = OPERAND;
					sync(4);
				}
				continue;
			}  // CC = CALL Z,a16	 | Fg: - - - - | Sz: 3 | Cc: 24/12
			
			INST(CD, CALL_A16) {
				PUSH16(PC);
				PC = OPERAND;
				sync(4);
				continue;
			}  // CD = CALL a16	 | Fg: - - - - | Sz: 3 | Cc: 24
			
			INST(D4, CALL_NC_A16) {
				if (!c) {
					PUSH16(PC);
					PC = OPERAND;
					sync(4);
				}
				continue;
			}  // D4 = CALL NC,a16	 | Fg: - - - - | Sz: 3 | Cc: 24/12
			
			INST(DC, CALL_C_A16) {
				if (c) {
					PUSH16(PC);
					PC = OPERAND;
					sync(4);
				}
				continue;
			}  // DC = CALL C,a16	 | Fg: - - - - | Sz: 3 | Cc: 24/12
			
			INST(3F, CCF) {
				CCF();
				continue;
			}  // 3F = CCF	 | Fg: - 0 0 C | Sz: 1 | Cc: 4
			
			INST(B8, CP_B) {
				CP(B);
				continue;
			}  // B8 = CP B	 | Fg: Z 1 H C | Sz: 1 | Cc: 4
			
			INST(B9, CP_C) {
				CP(C);
				continue;
			}  // B9 = CP C	 | Fg: Z 1 H C | Sz: 1 | Cc: 4
			
			INST(BA, CP_D) {
				CP(D);
				continue;
			}  // BA = CP D	 | Fg: Z 1 H C | Sz: 1 | Cc: 4
			
			INST(BB, CP_E) {
				CP(E);
				continue;
			}  // BB = CP E	 | Fg: Z 1 H C | Sz: 1 | Cc: 4
			
			INST(BC, CP_H) {
				CP(H);
				continue;
			}  // BC = CP H	 | Fg: Z 1 H C | Sz: 1 | Cc: 4
			
			INST(BD, CP_L) {
				CP(L);
				continue;
			}  // BD = CP L	 | Fg: Z 1 H C | Sz: 1 | Cc: 4
			
			INST(BE, CP_(HL)) {
				r = cpu_read(HL);
				CP(r);
				continue;
			}  // BE = CP (HL)	 | Fg: Z 1 H C | Sz: 1 | Cc: 8
			
			INST(BF, CP_A) {
				CP(A);
				continue;
			}  // BF = CP A	 | Fg: Z 1 H C | Sz: 1 | Cc: 4
			
			INST(FE, CP_D8) {
				CP(O1);
				continue;
			}  // FE = CP d8	 | Fg: Z 1 H C | Sz: 2 | Cc: 8
			
			INST(2F, CPL) {
				CPL();
				continue;
			}  // 2F = CPL	 | Fg: - 1 1 - | Sz: 1 | Cc: 4
			
			INST(27, DAA) {
				s32 daa = A;
				if (n) {
					if (h) daa = (daa - 0x06) & 0xff;
					if (c) daa -= 0x60;
				} else {
					if (h || (daa & 0xf) > 9) daa += 0x06;
					if (c || daa > 0x9f) daa += 0x60;
				}
				
				A = daa, h = 0, z = !A;
				if ((daa & 0x100) == 0x100) c = 1;
				continue;
			}  // 27 = DAA	 | Fg: Z - 0 C | Sz: 1 | Cc: 4
			
			INST(05, DEC_B) {
				DEC(B);
				continue;
			}  // 05 = DEC B	 | Fg: Z 1 H - | Sz: 1 | Cc: 4
			
			INST(0B, DEC_BC) {
				BC--;
				sync(4);
				continue;
			}  // 0B = DEC BC	 | Fg: - - - - | Sz: 1 | Cc: 8
			
			INST(0D, DEC_C) {
				DEC(C);
				continue;
			}  // 0D = DEC C	 | Fg: Z 1 H - | Sz: 1 | Cc: 4
			
			INST(15, DEC_D) {
				DEC(D);
				continue;
			}  // 15 = DEC D	 | Fg: Z 1 H - | Sz: 1 | Cc: 4
			
			INST(1B, DEC_DE) {
				DE--;
				sync(4);
				continue;
			}  // 1B = DEC DE	 | Fg: - - - - | Sz: 1 | Cc: 8
			
			INST(1D, DEC_E) {
				DEC(E);
				continue;
			}  // 1D = DEC E	 | Fg: Z 1 H - | Sz: 1 | Cc: 4
			
			INST(25, DEC_H) {
				DEC(H);
				continue;
			}  // 25 = DEC H	 | Fg: Z 1 H - | Sz: 1 | Cc: 4
			
			INST(2B, DEC_HL) {
				HL--;
				sync(4);
				continue;
			}  // 2B = DEC HL	 | Fg: - - - - | Sz: 1 | Cc: 8
			
			INST(2D, DEC_L) {
				DEC(L);
				continue;
			}  // 2D = DEC L	 | Fg: Z 1 H - | Sz: 1 | Cc: 4
			
			INST(35, DEC_(HL)) {
				r = cpu_read(HL);
				DEC(r);
				cpu_write(HL, r);
				continue;
			}  // 35 = DEC (HL)	 | Fg: Z 1 H - | Sz: 1 | Cc: 12
			
			INST(3B, DEC_SP) {
				SP--;
				sync(4);
				continue;
			}  // 3B = DEC SP	 | Fg: - - - - | Sz: 1 | Cc: 8
			
			INST(3D, DEC_A) {
				DEC(A);
				continue;
			}  // 3D = DEC A	 | Fg: Z 1 H - | Sz: 1 | Cc: 4
			
			INST(F3, DI) {
				IME = 0;
				continue;
			}  // F3 = DI	 | Fg: - - - - | Sz: 1 | Cc: 4
			
			INST(FB, EI) {
				IME_DELAY |= 2;
				continue;
			}  // FB = EI	 | Fg: - - - - | Sz: 1 | Cc: 4
			
			INST(76, HALT) {
				if (halted) goto dec;
				
				if (!IME && ioIF & read_ie() & 0x1f) halt_bug = 1; // All GB are affected including GBC in GBC Mode (blargg test / sameboy)
				else {
					halted = 1;
					dec: PC--; // emulate halt by reexecuting it
				}
				continue;
			}  // 76 = HALT	 | Fg: - - - - | Sz: 1 | Cc: 4
			
			INST(03, INC_BC) {
				BC++;
				sync(4);
				continue;
			}  // 03 = INC BC	 | Fg: - - - - | Sz: 1 | Cc: 8
			
			INST(04, INC_B) {
				INC(B);
				continue;
			}  // 04 = INC B	 | Fg: Z 0 H - | Sz: 1 | Cc: 4
			
			INST(0C, INC_C) {
				INC(C);
				continue;
			}  // 0C = INC C	 | Fg: Z 0 H - | Sz: 1 | Cc: 4
			
			INST(13, INC_DE) {
				DE++;
				sync(4);
				continue;
			}  // 13 = INC DE	 | Fg: - - - - | Sz: 1 | Cc: 8
			
			INST(14, INC_D) {
				INC(D);
				continue;
			}  // 14 = INC D	 | Fg: Z 0 H - | Sz: 1 | Cc: 4
			
			INST(1C, INC_E) {
				INC(E);
				continue;
			}  // 1C = INC E	 | Fg: Z 0 H - | Sz: 1 | Cc: 4
			
			INST(23, INC_HL) {
				HL++;
				sync(4);
				continue;
			}  // 23 = INC HL	 | Fg: - - - - | Sz: 1 | Cc: 8
			
			INST(24, INC_H) {
				INC(H);
				continue;
			}  // 24 = INC H	 | Fg: Z 0 H - | Sz: 1 | Cc: 4
			
			INST(2C, INC_L) {
				INC(L);
				continue;
			}  // 2C = INC L	 | Fg: Z 0 H - | Sz: 1 | Cc: 4
			
			INST(33, INC_SP) {
				SP++;
				sync(4);
				continue;
			}  // 33 = INC SP	 | Fg: - - - - | Sz: 1 | Cc: 8
			
			INST(34, INC_(HL)) {
				r = cpu_read(HL);
				INC(r);
				cpu_write(HL, r);
				continue;
			}  // 34 = INC (HL)	 | Fg: Z 0 H - | Sz: 1 | Cc: 12
			
			INST(3C, INC_A) {
				INC(A);
				continue;
			}  // 3C = INC A	 | Fg: Z 0 H - | Sz: 1 | Cc: 4
			
			INST(C2, JP_NZ_A16) {
				if (!z) {
					PC = OPERAND;
					sync(4);
				}
				continue;
			}  // C2 = JP NZ,a16	 | Fg: - - - - | Sz: 3 | Cc: 16/12
			
			INST(C3, JP_A16) {
				PC = OPERAND;
				sync(4);
				continue;
			}  // C3 = JP a16	 | Fg: - - - - | Sz: 3 | Cc: 16
			
			INST(CA, JP_Z_A16) {
				if (z) {
					PC = OPERAND;
					sync(4);
				}
				continue;
			}  // CA = JP Z,a16	 | Fg: - - - - | Sz: 3 | Cc: 16/12
			
			INST(D2, JP_NC_A16) {
				if (!c) {
					PC = OPERAND;
					sync(4);
				}
				continue;
			}  // D2 = JP NC,a16	 | Fg: - - - - | Sz: 3 | Cc: 16/12
			
			INST(DA, JP_C_A16) {
				if (c) {
					PC = OPERAND;
					sync(4);
				}
				continue;
			}  // DA = JP C,a16	 | Fg: - - - - | Sz: 3 | Cc: 16/12
			
			INST(E9, JP_(HL)) {
				PC = HL;
				continue;
			}  // E9 = JP (HL)	 | Fg: - - - - | Sz: 1 | Cc: 4
			
			INST(18, JR_R8) {
				PC += (s8) O1;
				sync(4);
				continue;
			}  // 18 = JR r8	 | Fg: - - - - | Sz: 2 | Cc: 12
			
			INST(20, JR_NZ_R8) {
				if (!z) {
					PC += (s8) O1;
					sync(4);
				}
				continue;
			}  // 20 = JR NZ,r8	 | Fg: - - - - | Sz: 2 | Cc: 12/8
			
			INST(28, JR_Z_R8) {
				if (z) {
					PC += (s8) O1;
					sync(4);
				}
				continue;
			}  // 28 = JR Z,r8	 | Fg: - - - - | Sz: 2 | Cc: 12/8
			
			INST(30, JR_NC_R8) {
				if (!c) {
					PC += (s8) O1;
					sync(4);
				}
				continue;
			}  // 30 = JR NC,r8	 | Fg: - - - - | Sz: 2 | Cc: 12/8
			
			INST(38, JR_C_R8) {
				if (c) {
					PC += (s8) O1;
					sync(4);
				}
				continue;
			}  // 38 = JR C,r8	 | Fg: - - - - | Sz: 2 | Cc: 12/8
			
			INST(01, LD_BC_D16) {
				BC = OPERAND;
				continue;
			}  // 01 = LD BC,d16	 | Fg: - - - - | Sz: 3 | Cc: 12
			
			INST(02, LD_(BC)_A) {
				cpu_write(BC, A);
				continue;
			}  // 02 = LD (BC),A	 | Fg: - - - - | Sz: 1 | Cc: 8
			
			INST(06, LD_B_D8) {
				B = O1;
				continue;
			}  // 06 = LD B,d8	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST(08, LD_(A16)_SP) {
				cpu_write(OPERAND, SP);
				cpu_write(OPERAND + 1, SP >> 8);
				continue;
			}  // 08 = LD (a16),SP	 | Fg: - - - - | Sz: 3 | Cc: 20
			
			INST(0A, LD_A_(BC)) {
				A = cpu_read(BC);
				continue;
			}  // 0A = LD A,(BC)	 | Fg: - - - - | Sz: 1 | Cc: 8
			
			INST(0E, LD_C_D8) {
				C = O1;
				continue;
			}  // 0E = LD C,d8	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST(11, LD_DE_D16) {
				DE = OPERAND;
				continue;
			}  // 11 = LD DE,d16	 | Fg: - - - - | Sz: 3 | Cc: 12
			
			INST(12, LD_(DE)_A) {
				cpu_write(DE, A);
				continue;
			}  // 12 = LD (DE),A	 | Fg: - - - - | Sz: 1 | Cc: 8
			
			INST(16, LD_D_D8) {
				D = O1;
				continue;
			}  // 16 = LD D,d8	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST(1A, LD_A_(DE)) {
				A = cpu_read(DE);
				continue;
			}  // 1A = LD A,(DE)	 | Fg: - - - - | Sz: 1 | Cc: 8
			
			INST(1E, LD_E_D8) {
				E = O1;
				continue;
			}  // 1E = LD E,d8	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST(21, LD_HL_D16) {
				HL = OPERAND;
				continue;
			}  // 21 = LD HL,d16	 | Fg: - - - - | Sz: 3 | Cc: 12
			
			INST(22, LD_(HL_)_A) {
				cpu_write(HL++, A);
				continue;
			}  // 22 = LD (HL+),A	 | Fg: - - - - | Sz: 1 | Cc: 8
			
			INST(26, LD_H_D8) {
				H = O1;
				continue;
			}  // 26 = LD H,d8	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST(2A, LD_A_(HL_)) {
				A = cpu_read(HL++);
				continue;
			}  // 2A = LD A,(HL+)	 | Fg: - - - - | Sz: 1 | Cc: 8
			
			INST(2E, LD_L_D8) {
				L = O1;
				continue;
			}  // 2E = LD L,d8	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST(31, LD_SP_D16) {
				SP = OPERAND;
				continue;
			}  // 31 = LD SP,d16	 | Fg: - - - - | Sz: 3 | Cc: 12
			
			INST(32, LD_(HL - )_A) {
				cpu_write(HL--, A);
				continue;
			}  // 32 = LD (HL-),A	 | Fg: - - - - | Sz: 1 | Cc: 8
			
			INST(36, LD_(HL)_D8) {
				cpu_write(HL, O1);
				continue;
			}  // 36 = LD (HL),d8	 | Fg: - - - - | Sz: 2 | Cc: 12
			
			INST(3A, LD_A_(HL - )) {
				A = cpu_read(HL--);
				continue;
			}  // 3A = LD A,(HL-)	 | Fg: - - - - | Sz: 1 | Cc: 8
			
			INST(3E, LD_A_D8) {
				A = O1;
				continue;
			}  // 3E = LD A,d8	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST(40, LD_B_B) {
				CRITICAL("TEST STOP", "ld b,b\n");
				B = B;
				continue;
			}  // 40 = LD B,B	 | Fg: - - - - | Sz: 1 | Cc: 4
			
			INST(41, LD_B_C) {
				B = C;
				continue;
			}  // 41 = LD B,C	 | Fg: - - - - | Sz: 1 | Cc: 4
			
			INST(42, LD_B_D) {
				B = D;
				continue;
			}  // 42 = LD B,D	 | Fg: - - - - | Sz: 1 | Cc: 4
			
			INST(43, LD_B_E) {
				B = E;
				continue;
			}  // 43 = LD B,E	 | Fg: - - - - | Sz: 1 | Cc: 4
			
			INST(44, LD_B_H) {
				B = H;
				continue;
			}  // 44 = LD B,H	 | Fg: - - - - | Sz: 1 | Cc: 4
			
			INST(45, LD_B_L) {
				B = L;
				continue;
			}  // 45 = LD B,L	 | Fg: - - - - | Sz: 1 | Cc: 4
			
			INST(46, LD_B_(HL)) {
				B = cpu_read(HL);
				continue;
			}  // 46 = LD B,(HL)	 | Fg: - - - - | Sz: 1 | Cc: 8
			
			INST(47, LD_B_A) {
				B = A;
				continue;
			}  // 47 = LD B,A	 | Fg: - - - - | Sz: 1 | Cc: 4
			
			INST(48, LD_C_B) {
				C = B;
				continue;
			}  // 48 = LD C,B	 | Fg: - - - - | Sz: 1 | Cc: 4
			
			INST(49, LD_C_C) {
				C = C;
				continue;
			}  // 49 = LD C,C	 | Fg: - - - - | Sz: 1 | Cc: 4
			
			INST(4A, LD_C_D) {
				C = D;
				continue;
			}  // 4A = LD C,D	 | Fg: - - - - | Sz: 1 | Cc: 4
			
			INST(4B, LD_C_E) {
				C = E;
				continue;
			}  // 4B = LD C,E	 | Fg: - - - - | Sz: 1 | Cc: 4
			
			INST(4C, LD_C_H) {
				C = H;
				continue;
			}  // 4C = LD C,H	 | Fg: - - - - | Sz: 1 | Cc: 4
			
			INST(4D, LD_C_L) {
				C = L;
				continue;
			}  // 4D = LD C,L	 | Fg: - - - - | Sz: 1 | Cc: 4
			
			INST(4E, LD_C_(HL)) {
				C = cpu_read(HL);
				continue;
			}  // 4E = LD C,(HL)	 | Fg: - - - - | Sz: 1 | Cc: 8
			
			INST(4F, LD_C_A) {
				C = A;
				continue;
			}  // 4F = LD C,A	 | Fg: - - - - | Sz: 1 | Cc: 4
			
			INST(50, LD_D_B) {
				D = B;
				continue;
			}  // 50 = LD D,B	 | Fg: - - - - | Sz: 1 | Cc: 4
			
			INST(51, LD_D_C) {
				D = C;
				continue;
			}  // 51 = LD D,C	 | Fg: - - - - | Sz: 1 | Cc: 4
			
			INST(52, LD_D_D) {
				D = D;
				continue;
			}  // 52 = LD D,D	 | Fg: - - - - | Sz: 1 | Cc: 4
			
			INST(53, LD_D_E) {
				D = E;
				continue;
			}  // 53 = LD D,E	 | Fg: - - - - | Sz: 1 | Cc: 4
			
			INST(54, LD_D_H) {
				D = H;
				continue;
			}  // 54 = LD D,H	 | Fg: - - - - | Sz: 1 | Cc: 4
			
			INST(55, LD_D_L) {
				D = L;
				continue;
			}  // 55 = LD D,L	 | Fg: - - - - | Sz: 1 | Cc: 4
			
			INST(56, LD_D_(HL)) {
				D = cpu_read(HL);
				continue;
			}  // 56 = LD D,(HL)	 | Fg: - - - - | Sz: 1 | Cc: 8
			
			INST(57, LD_D_A) {
				D = A;
				continue;
			}  // 57 = LD D,A	 | Fg: - - - - | Sz: 1 | Cc: 4
			
			INST(58, LD_E_B) {
				E = B;
				continue;
			}  // 58 = LD E,B	 | Fg: - - - - | Sz: 1 | Cc: 4
			
			INST(59, LD_E_C) {
				E = C;
				continue;
			}  // 59 = LD E,C	 | Fg: - - - - | Sz: 1 | Cc: 4
			
			INST(5A, LD_E_D) {
				E = D;
				continue;
			}  // 5A = LD E,D	 | Fg: - - - - | Sz: 1 | Cc: 4
			
			INST(5B, LD_E_E) {
				E = E;
				continue;
			}  // 5B = LD E,E	 | Fg: - - - - | Sz: 1 | Cc: 4
			
			INST(5C, LD_E_H) {
				E = H;
				continue;
			}  // 5C = LD E,H	 | Fg: - - - - | Sz: 1 | Cc: 4
			
			INST(5D, LD_E_L) {
				E = L;
				continue;
			}  // 5D = LD E,L	 | Fg: - - - - | Sz: 1 | Cc: 4
			
			INST(5E, LD_E_(HL)) {
				E = cpu_read(HL);
				continue;
			}  // 5E = LD E,(HL)	 | Fg: - - - - | Sz: 1 | Cc: 8
			
			INST(5F, LD_E_A) {
				E = A;
				continue;
			}  // 5F = LD E,A	 | Fg: - - - - | Sz: 1 | Cc: 4
			
			INST(60, LD_H_B) {
				H = B;
				continue;
			}  // 60 = LD H,B	 | Fg: - - - - | Sz: 1 | Cc: 4
			
			INST(61, LD_H_C) {
				H = C;
				continue;
			}  // 61 = LD H,C	 | Fg: - - - - | Sz: 1 | Cc: 4
			
			INST(62, LD_H_D) {
				H = D;
				continue;
			}  // 62 = LD H,D	 | Fg: - - - - | Sz: 1 | Cc: 4
			
			INST(63, LD_H_E) {
				H = E;
				continue;
			}  // 63 = LD H,E	 | Fg: - - - - | Sz: 1 | Cc: 4
			
			INST(64, LD_H_H) {
				H = H;
				continue;
			}  // 64 = LD H,H	 | Fg: - - - - | Sz: 1 | Cc: 4
			
			INST(65, LD_H_L) {
				H = L;
				continue;
			}  // 65 = LD H,L	 | Fg: - - - - | Sz: 1 | Cc: 4
			
			INST(66, LD_H_(HL)) {
				H = cpu_read(HL);
				continue;
			}  // 66 = LD H,(HL)	 | Fg: - - - - | Sz: 1 | Cc: 8
			
			INST(67, LD_H_A) {
				H = A;
				continue;
			}  // 67 = LD H,A	 | Fg: - - - - | Sz: 1 | Cc: 4
			
			INST(68, LD_L_B) {
				L = B;
				continue;
			}  // 68 = LD L,B	 | Fg: - - - - | Sz: 1 | Cc: 4
			
			INST(69, LD_L_C) {
				L = C;
				continue;
			}  // 69 = LD L,C	 | Fg: - - - - | Sz: 1 | Cc: 4
			
			INST(6A, LD_L_D) {
				L = D;
				continue;
			}  // 6A = LD L,D	 | Fg: - - - - | Sz: 1 | Cc: 4
			
			INST(6B, LD_L_E) {
				L = E;
				continue;
			}  // 6B = LD L,E	 | Fg: - - - - | Sz: 1 | Cc: 4
			
			INST(6C, LD_L_H) {
				L = H;
				continue;
			}  // 6C = LD L,H	 | Fg: - - - - | Sz: 1 | Cc: 4
			
			INST(6D, LD_L_L) {
				L = L;
				continue;
			}  // 6D = LD L,L	 | Fg: - - - - | Sz: 1 | Cc: 4
			
			INST(6E, LD_L_(HL)) {
				L = cpu_read(HL);
				continue;
			}  // 6E = LD L,(HL)	 | Fg: - - - - | Sz: 1 | Cc: 8
			
			INST(6F, LD_L_A) {
				L = A;
				continue;
			}  // 6F = LD L,A	 | Fg: - - - - | Sz: 1 | Cc: 4
			
			INST(70, LD_(HL)_B) {
				cpu_write(HL, B);
				continue;
			}  // 70 = LD (HL),B	 | Fg: - - - - | Sz: 1 | Cc: 8
			
			INST(71, LD_(HL)_C) {
				cpu_write(HL, C);
				continue;
			}  // 71 = LD (HL),C	 | Fg: - - - - | Sz: 1 | Cc: 8
			
			INST(72, LD_(HL)_D) {
				cpu_write(HL, D);
				continue;
			}  // 72 = LD (HL),D	 | Fg: - - - - | Sz: 1 | Cc: 8
			
			INST(73, LD_(HL)_E) {
				cpu_write(HL, E);
				continue;
			}  // 73 = LD (HL),E	 | Fg: - - - - | Sz: 1 | Cc: 8
			
			INST(74, LD_(HL)_H) {
				cpu_write(HL, H);
				continue;
			}  // 74 = LD (HL),H	 | Fg: - - - - | Sz: 1 | Cc: 8
			
			INST(75, LD_(HL)_L) {
				cpu_write(HL, L);
				continue;
			}  // 75 = LD (HL),L	 | Fg: - - - - | Sz: 1 | Cc: 8
			
			INST(77, LD_(HL)_A) {
				cpu_write(HL, A);
				continue;
			}  // 77 = LD (HL),A	 | Fg: - - - - | Sz: 1 | Cc: 8
			
			INST(78, LD_A_B) {
				A = B;
				continue;
			}  // 78 = LD A,B	 | Fg: - - - - | Sz: 1 | Cc: 4
			
			INST(79, LD_A_C) {
				A = C;
				continue;
			}  // 79 = LD A,C	 | Fg: - - - - | Sz: 1 | Cc: 4
			
			INST(7A, LD_A_D) {
				A = D;
				continue;
			}  // 7A = LD A,D	 | Fg: - - - - | Sz: 1 | Cc: 4
			
			INST(7B, LD_A_E) {
				A = E;
				continue;
			}  // 7B = LD A,E	 | Fg: - - - - | Sz: 1 | Cc: 4
			
			INST(7C, LD_A_H) {
				A = H;
				continue;
			}  // 7C = LD A,H	 | Fg: - - - - | Sz: 1 | Cc: 4
			
			INST(7D, LD_A_L) {
				A = L;
				continue;
			}  // 7D = LD A,L	 | Fg: - - - - | Sz: 1 | Cc: 4
			
			INST(7E, LD_A_(HL)) {
				A = cpu_read(HL);
				continue;
			}  // 7E = LD A,(HL)	 | Fg: - - - - | Sz: 1 | Cc: 8
			
			INST(7F, LD_A_A) {
				A = A;
				continue;
			}  // 7F = LD A,A	 | Fg: - - - - | Sz: 1 | Cc: 4
			
			INST(E2, LD_(C)_A) {
				cpu_write(0xFF00 | C, A);
				continue;
			}  // E2 = LD (C),A	 | Fg: - - - - | Sz: 1 | Cc: 8
			
			INST(EA, LD_(A16)_A) {
				cpu_write(OPERAND, A);
				continue;
			}  // EA = LD (a16),A	 | Fg: - - - - | Sz: 3 | Cc: 16
			
			INST(F2, LD_A_(C)) {
				A = cpu_read(0xFF00 | C);
				continue;
			}  // F2 = LD A,(C)	 | Fg: - - - - | Sz: 1 | Cc: 8
			
			INST(F8, LD_HL_SP_R8) {
				ADD_SP_d(O1, HL);
				continue;
			}  // F8 = LD HL,SP+r8	 | Fg: 0 0 H C | Sz: 2 | Cc: 12
			
			INST(F9, LD_SP_HL) {
				SP = HL;
				sync(4);
				continue;
			}  // F9 = LD SP,HL	 | Fg: - - - - | Sz: 1 | Cc: 8
			
			INST(FA, LD_A_(A16)) {
				A = cpu_read(OPERAND);
				continue;
			}  // FA = LD A,(a16)	 | Fg: - - - - | Sz: 3 | Cc: 16
			
			INST(E0, LDH_(A8)_A) {
				cpu_write(0xFF00 | O1, A);
				continue;
			}  // E0 = LDH (a8),A	 | Fg: - - - - | Sz: 2 | Cc: 12
			
			INST(F0, LDH_A_(A8)) {
				A = cpu_read(0xFF00 | O1);
				continue;
			}  // F0 = LDH A,(a8)	 | Fg: - - - - | Sz: 2 | Cc: 12
			
			INST(00, NOP) {
				continue;
			}  // 00 = NOP	 | Fg: - - - - | Sz: 1 | Cc: 4
			
			INST(B0, OR_B) {
				OR(B);
				continue;
			}  // B0 = OR B	 | Fg: Z 0 0 0 | Sz: 1 | Cc: 4
			
			INST(B1, OR_C) {
				OR(C);
				continue;
			}  // B1 = OR C	 | Fg: Z 0 0 0 | Sz: 1 | Cc: 4
			
			INST(B2, OR_D) {
				OR(D);
				continue;
			}  // B2 = OR D	 | Fg: Z 0 0 0 | Sz: 1 | Cc: 4
			
			INST(B3, OR_E) {
				OR(E);
				continue;
			}  // B3 = OR E	 | Fg: Z 0 0 0 | Sz: 1 | Cc: 4
			
			INST(B4, OR_H) {
				OR(H);
				continue;
			}  // B4 = OR H	 | Fg: Z 0 0 0 | Sz: 1 | Cc: 4
			
			INST(B5, OR_L) {
				OR(L);
				continue;
			}  // B5 = OR L	 | Fg: Z 0 0 0 | Sz: 1 | Cc: 4
			
			INST(B6, OR_(HL)) {
				OR(cpu_read(HL));
				continue;
			}  // B6 = OR (HL)	 | Fg: Z 0 0 0 | Sz: 1 | Cc: 8
			
			INST(B7, OR_A) {
				OR(A);
				continue;
			}  // B7 = OR A	 | Fg: Z 0 0 0 | Sz: 1 | Cc: 4
			
			INST(F6, OR_D8) {
				OR(O1);
				continue;
			}  // F6 = OR d8	 | Fg: Z 0 0 0 | Sz: 2 | Cc: 8
			
			INST(C1, POP_BC) {
				exp_POP16(BC);
				//BC = POP16();
				continue;
			}  // C1 = POP BC	 | Fg: - - - - | Sz: 1 | Cc: 12
			
			INST(D1, POP_DE) {
				
				exp_POP16(DE);
				//DE = POP16();
				continue;
			}  // D1 = POP DE	 | Fg: - - - - | Sz: 1 | Cc: 12
			
			INST(E1, POP_HL) {
				
				exp_POP16(HL);
				//HL = POP16();
				continue;
			}  // E1 = POP HL	 | Fg: - - - - | Sz: 1 | Cc: 12
			
			INST(F1, POP_AF) {
				
				exp_POP16(AF);
				//AF = POP16();
				Fx = 0;
				continue;
			}  // F1 = POP AF	 | Fg: Z N H C | Sz: 1 | Cc: 12
			
			INST(CB, PREFIX_CB) {
				OPCODE = 0x100 | O1;
				goto inst;
			} // CB = PREFIX CB	 | Fg: - - - - | Sz: 2 | Cc: 4
			
			INST(C5, PUSH_BC) {
				PUSH16(BC);
				sync(4);
				continue;
			}  // C5 = PUSH BC	 | Fg: - - - - | Sz: 1 | Cc: 16
			
			INST(D5, PUSH_DE) {
				PUSH16(DE);
				sync(4);
				continue;
			}  // D5 = PUSH DE	 | Fg: - - - - | Sz: 1 | Cc: 16
			
			INST(E5, PUSH_HL) {
				PUSH16(HL);
				sync(4);
				continue;
			}  // E5 = PUSH HL	 | Fg: - - - - | Sz: 1 | Cc: 16
			
			INST(F5, PUSH_AF) {
				PUSH16(AF);
				sync(4);
				continue;
			}  // F5 = PUSH AF	 | Fg: - - - - | Sz: 1 | Cc: 16
			
			INST(C0, RET_NZ) {
				sync(4);
				if (!z) {
				
				exp_POP16(PC);
				//PC = POP16();
					sync(4);
				}
				continue;
			}  // C0 = RET NZ	 | Fg: - - - - | Sz: 1 | Cc: 20/8
			
			INST(C8, RET_Z) {
				sync(4);
				if (z) {
				
				exp_POP16(PC);
				//PC = POP16();
					sync(4);
				}
				continue;
			}  // C8 = RET Z	 | Fg: - - - - | Sz: 1 | Cc: 20/8
			
			INST(C9, RET) {
				
				exp_POP16(PC);
				//PC = POP16();
				sync(4);
				continue;
			}  // C9 = RET	 | Fg: - - - - | Sz: 1 | Cc: 16
			
			INST(D0, RET_NC) {
				sync(4);
				if (!c) {
				
				exp_POP16(PC);
				//PC = POP16();
					sync(4);
				}
				continue;
			}  // D0 = RET NC	 | Fg: - - - - | Sz: 1 | Cc: 20/8
			
			INST(D8, RET_C) {
				sync(4);
				if (c) {
				
				exp_POP16(PC);
				//PC = POP16();
					sync(4);
				}
				continue;
			}  // D8 = RET C	 | Fg: - - - - | Sz: 1 | Cc: 20/8
			
			INST(D9, RETI) {
				
				exp_POP16(PC);
				//PC = POP16();
				sync(4);
				IME = 1;
				continue;
			}  // D9 = RETI	 | Fg: - - - - | Sz: 1 | Cc: 16
			
			INST(17, RLA) {
				RLA();
				continue;
			}  // 17 = RLA	 | Fg: 0 0 0 C | Sz: 1 | Cc: 4
			
			INST(07, RLCA) {
				RLCA();
				continue;
			}  // 07 = RLCA	 | Fg: 0 0 0 C | Sz: 1 | Cc: 4
			
			INST(1F, RRA) {
				RRA();
				continue;
			}  // 1F = RRA	 | Fg: 0 0 0 C | Sz: 1 | Cc: 4
			
			INST(0F, RRCA) {
				RRCA();
				continue;
			}  // 0F = RRCA	 | Fg: 0 0 0 C | Sz: 1 | Cc: 4
			
			INST(C7, RST_00H) {
				PUSH16(PC);
				PC = 0x00;
				sync(4);
				continue;
			}  // C7 = RST 00H	 | Fg: - - - - | Sz: 1 | Cc: 16
			
			INST(CF, RST_08H) {
				PUSH16(PC);
				PC = 0x08;
				sync(4);
				continue;
			}  // CF = RST 08H	 | Fg: - - - - | Sz: 1 | Cc: 16
			
			INST(D7, RST_10H) {
				PUSH16(PC);
				PC = 0x10;
				sync(4);
				continue;
			}  // D7 = RST 10H	 | Fg: - - - - | Sz: 1 | Cc: 16
			
			INST(DF, RST_18H) {
				PUSH16(PC);
				PC = 0x18;
				sync(4);
				continue;
			}  // DF = RST 18H	 | Fg: - - - - | Sz: 1 | Cc: 16
			
			INST(E7, RST_20H) {
				PUSH16(PC);
				PC = 0x20;
				sync(4);
				continue;
			}  // E7 = RST 20H	 | Fg: - - - - | Sz: 1 | Cc: 16
			
			INST(EF, RST_28H) {
				PUSH16(PC);
				PC = 0x28;
				sync(4);
				continue;
			}  // EF = RST 28H	 | Fg: - - - - | Sz: 1 | Cc: 16
			
			INST(F7, RST_30H) {
				PUSH16(PC);
				PC = 0x30;
				sync(4);
				continue;
			}  // F7 = RST 30H	 | Fg: - - - - | Sz: 1 | Cc: 16
			
			INST(FF, RST_38H) {
				PUSH16(PC);
				PC = 0x38;
				sync(4);
				continue;
			}  // FF = RST 38H	 | Fg: - - - - | Sz: 1 | Cc: 16
			
			INST(98, SBC_A_B) {
				SBC(B);
				continue;
			}  // 98 = SBC A,B	 | Fg: Z 1 H C | Sz: 1 | Cc: 4
			
			INST(99, SBC_A_C) {
				SBC(C);
				continue;
			}  // 99 = SBC A,C	 | Fg: Z 1 H C | Sz: 1 | Cc: 4
			
			INST(9A, SBC_A_D) {
				SBC(D);
				continue;
			}  // 9A = SBC A,D	 | Fg: Z 1 H C | Sz: 1 | Cc: 4
			
			INST(9B, SBC_A_E) {
				SBC(E);
				continue;
			}  // 9B = SBC A,E	 | Fg: Z 1 H C | Sz: 1 | Cc: 4
			
			INST(9C, SBC_A_H) {
				SBC(H);
				continue;
			}  // 9C = SBC A,H	 | Fg: Z 1 H C | Sz: 1 | Cc: 4
			
			INST(9D, SBC_A_L) {
				SBC(L);
				continue;
			}  // 9D = SBC A,L	 | Fg: Z 1 H C | Sz: 1 | Cc: 4
			
			INST(9E, SBC_A_(HL)) {
				r = cpu_read(HL);
				SBC(r);
				continue;
			}  // 9E = SBC A,(HL)	 | Fg: Z 1 H C | Sz: 1 | Cc: 8
			
			INST(9F, SBC_A_A) {
				SBC(A);
				continue;
			}  // 9F = SBC A,A	 | Fg: Z 1 H C | Sz: 1 | Cc: 4
			
			INST(DE, SBC_A_D8) {
				SBC(O1);
				continue;
			}  // DE = SBC A,d8	 | Fg: Z 1 H C | Sz: 2 | Cc: 8
			
			INST(37, SCF) {
				SCF();
				continue;
			}  // 37 = SCF	 | Fg: - 0 0 1 | Sz: 1 | Cc: 4
			
			INST(10, STOP_0) {
				
				
				stopped = 1;
				if (O1) {
					direct_write_io(LCDC, ioLCDC | 0x80);
					CRITICAL("Corrupted Stop", "LCDC Switch On : %02X\n", ioLCDC);
				}
				
				if ((~ioJOYP) & 0x0F) { // Btn selected
					if (ioIF & read_ie() & 0x1f) {
					
					} else {
						//PC++;
						ERROR("STOP Instruction - HALT Mode entrance", "unimplemented\n");
						halted = 1;
					}
				} else {
					if ((ioKEY1 & 1) && GBC) {
						if (ioIF & read_ie() & 0x1f) {
							if (IME) {
								ERROR("STOP Instruction error", "non-deterministic crash\n");
								// glitch
							} else {
								ch_speed:
								for (u16 u = 0; u < 32768; u++) sync(4);
								double_speed = !double_speed;
								direct_write_io(KEY1, 0);
								direct_write_io(DIV, 0);
								INFO("Speed Switch", "Double speed mode = %hhu\n", double_speed);
							}
						} else {
							//PC++;
							ERROR("STOP Instruction - Special HALT Mode entrance", "unimplemented\n");
							// halted = 1; ???
							goto ch_speed;
						}
					} else {
						//if (!(ioIF & read_ie())) PC++;
						ERROR("STOP Instruction - STOP Mode entrance", "unimplemented\n");
						// STOP Mode
						direct_write_io(DIV, 0);
					}
				}
				stopped = 0;
				
				continue;
			}  // 10 = STOP 0	 | Fg: - - - - | Sz: 1 | Cc: 4
			
			INST(90, SUB_B) {
				SUB(B);
				continue;
			}  // 90 = SUB B	 | Fg: Z 1 H C | Sz: 1 | Cc: 4
			
			INST(91, SUB_C) {
				SUB(C);
				continue;
			}  // 91 = SUB C	 | Fg: Z 1 H C | Sz: 1 | Cc: 4
			
			INST(92, SUB_D) {
				SUB(D);
				continue;
			}  // 92 = SUB D	 | Fg: Z 1 H C | Sz: 1 | Cc: 4
			
			INST(93, SUB_E) {
				SUB(E);
				continue;
			}  // 93 = SUB E	 | Fg: Z 1 H C | Sz: 1 | Cc: 4
			
			INST(94, SUB_H) {
				SUB(H);
				continue;
			}  // 94 = SUB H	 | Fg: Z 1 H C | Sz: 1 | Cc: 4
			
			INST(95, SUB_L) {
				SUB(L);
				continue;
			}  // 95 = SUB L	 | Fg: Z 1 H C | Sz: 1 | Cc: 4
			
			INST(96, SUB_(HL)) {
				r = cpu_read(HL);
				SUB(r);
				continue;
			}  // 96 = SUB (HL)	 | Fg: Z 1 H C | Sz: 1 | Cc: 8
			
			INST(97, SUB_A) {
				SUB(A);
				continue;
			}  // 97 = SUB A	 | Fg: Z 1 H C | Sz: 1 | Cc: 4
			
			INST(D6, SUB_D8) {
				SUB(O1);
				continue;
			}  // D6 = SUB d8	 | Fg: Z 1 H C | Sz: 2 | Cc: 8
			INST(D3, UNKN)
			INST(DB, UNKN)
			INST(DD, UNKN)
			INST(E3, UNKN)
			INST(E4, UNKN)
			INST(EB, UNKN)
			INST(EC, UNKN)
			INST(ED, UNKN)
			INST(F4, UNKN)
			INST(FC, UNKN)
			INST(FD, UNKN) {
				UnknownOpcode();
				continue;
			}
			
			INST(A8, XOR_B) {
				XOR(B);
				continue;
			}  // A8 = XOR B	 | Fg: Z 0 0 0 | Sz: 1 | Cc: 4
			
			INST(A9, XOR_C) {
				XOR(C);
				continue;
			}  // A9 = XOR C	 | Fg: Z 0 0 0 | Sz: 1 | Cc: 4
			
			INST(AA, XOR_D) {
				XOR(D);
				continue;
			}  // AA = XOR D	 | Fg: Z 0 0 0 | Sz: 1 | Cc: 4
			
			INST(AB, XOR_E) {
				XOR(E);
				continue;
			}  // AB = XOR E	 | Fg: Z 0 0 0 | Sz: 1 | Cc: 4
			
			INST(AC, XOR_H) {
				XOR(H);
				continue;
			}  // AC = XOR H	 | Fg: Z 0 0 0 | Sz: 1 | Cc: 4
			
			INST(AD, XOR_L) {
				XOR(L);
				continue;
			}  // AD = XOR L	 | Fg: Z 0 0 0 | Sz: 1 | Cc: 4
			
			INST(AE, XOR_(HL)) {
				XOR(cpu_read(HL));
				continue;
			}  // AE = XOR (HL)	 | Fg: Z 0 0 0 | Sz: 1 | Cc: 8
			
			INST(AF, XOR_A) {
				XOR(A);
				continue;
			}  // AF = XOR A	 | Fg: Z 0 0 0 | Sz: 1 | Cc: 4
			
			INST(EE, XOR_D8) {
				XOR(O1);
				continue;
			}  // EE = XOR d8	 | Fg: Z 0 0 0 | Sz: 2 | Cc: 8
			
			#pragma endregion

		#pragma region High Instructions
			
			INST_H(40, BIT_0_B) {
				BIT(0, B);
				continue;
			}  // 40 = BIT 0,B	 | Fg: Z 0 1 - | Sz: 2 | Cc: 8
			
			INST_H(41, BIT_0_C) {
				BIT(0, C);
				continue;
			}  // 41 = BIT 0,C	 | Fg: Z 0 1 - | Sz: 2 | Cc: 8
			
			INST_H(42, BIT_0_D) {
				BIT(0, D);
				continue;
			}  // 42 = BIT 0,D	 | Fg: Z 0 1 - | Sz: 2 | Cc: 8
			
			INST_H(43, BIT_0_E) {
				BIT(0, E);
				continue;
			}  // 43 = BIT 0,E	 | Fg: Z 0 1 - | Sz: 2 | Cc: 8
			
			INST_H(44, BIT_0_H) {
				BIT(0, H);
				continue;
			}  // 44 = BIT 0,H	 | Fg: Z 0 1 - | Sz: 2 | Cc: 8
			
			INST_H(45, BIT_0_L) {
				BIT(0, L);
				continue;
			}  // 45 = BIT 0,L	 | Fg: Z 0 1 - | Sz: 2 | Cc: 8
			
			INST_H(46, BIT_0_(HL)) {
				r = cpu_read(HL);
				BIT(0, r);
				continue;
			}  // 46 = BIT 0,(HL)	 | Fg: Z 0 1 - | Sz: 2 | Cc: 12
			
			INST_H(47, BIT_0_A) {
				BIT(0, A);
				continue;
			}  // 47 = BIT 0,A	 | Fg: Z 0 1 - | Sz: 2 | Cc: 8
			
			INST_H(48, BIT_1_B) {
				BIT(1, B);
				continue;
			}  // 48 = BIT 1,B	 | Fg: Z 0 1 - | Sz: 2 | Cc: 8
			
			INST_H(49, BIT_1_C) {
				BIT(1, C);
				continue;
			}  // 49 = BIT 1,C	 | Fg: Z 0 1 - | Sz: 2 | Cc: 8
			
			INST_H(4A, BIT_1_D) {
				BIT(1, D);
				continue;
			}  // 4A = BIT 1,D	 | Fg: Z 0 1 - | Sz: 2 | Cc: 8
			
			INST_H(4B, BIT_1_E) {
				BIT(1, E);
				continue;
			}  // 4B = BIT 1,E	 | Fg: Z 0 1 - | Sz: 2 | Cc: 8
			
			INST_H(4C, BIT_1_H) {
				BIT(1, H);
				continue;
			}  // 4C = BIT 1,H	 | Fg: Z 0 1 - | Sz: 2 | Cc: 8
			
			INST_H(4D, BIT_1_L) {
				BIT(1, L);
				continue;
			}  // 4D = BIT 1,L	 | Fg: Z 0 1 - | Sz: 2 | Cc: 8
			
			INST_H(4E, BIT_1_(HL)) {
				r = cpu_read(HL);
				BIT(1, r);
				continue;
			}  // 4E = BIT 1,(HL)	 | Fg: Z 0 1 - | Sz: 2 | Cc: 12
			
			INST_H(4F, BIT_1_A) {
				BIT(1, A);
				continue;
			}  // 4F = BIT 1,A	 | Fg: Z 0 1 - | Sz: 2 | Cc: 8
			
			INST_H(50, BIT_2_B) {
				BIT(2, B);
				continue;
			}  // 50 = BIT 2,B	 | Fg: Z 0 1 - | Sz: 2 | Cc: 8
			
			INST_H(51, BIT_2_C) {
				BIT(2, C);
				continue;
			}  // 51 = BIT 2,C	 | Fg: Z 0 1 - | Sz: 2 | Cc: 8
			
			INST_H(52, BIT_2_D) {
				BIT(2, D);
				continue;
			}  // 52 = BIT 2,D	 | Fg: Z 0 1 - | Sz: 2 | Cc: 8
			
			INST_H(53, BIT_2_E) {
				BIT(2, E);
				continue;
			}  // 53 = BIT 2,E	 | Fg: Z 0 1 - | Sz: 2 | Cc: 8
			
			INST_H(54, BIT_2_H) {
				BIT(2, H);
				continue;
			}  // 54 = BIT 2,H	 | Fg: Z 0 1 - | Sz: 2 | Cc: 8
			
			INST_H(55, BIT_2_L) {
				BIT(2, L);
				continue;
			}  // 55 = BIT 2,L	 | Fg: Z 0 1 - | Sz: 2 | Cc: 8
			
			INST_H(56, BIT_2_(HL)) {
				r = cpu_read(HL);
				BIT(2, r);
				continue;
			}  // 56 = BIT 2,(HL)	 | Fg: Z 0 1 - | Sz: 2 | Cc: 12
			
			INST_H(57, BIT_2_A) {
				BIT(2, A);
				continue;
			}  // 57 = BIT 2,A	 | Fg: Z 0 1 - | Sz: 2 | Cc: 8
			
			INST_H(58, BIT_3_B) {
				BIT(3, B);
				continue;
			}  // 58 = BIT 3,B	 | Fg: Z 0 1 - | Sz: 2 | Cc: 8
			
			INST_H(59, BIT_3_C) {
				BIT(3, C);
				continue;
			}  // 59 = BIT 3,C	 | Fg: Z 0 1 - | Sz: 2 | Cc: 8
			
			INST_H(5A, BIT_3_D) {
				BIT(3, D);
				continue;
			}  // 5A = BIT 3,D	 | Fg: Z 0 1 - | Sz: 2 | Cc: 8
			
			INST_H(5B, BIT_3_E) {
				BIT(3, E);
				continue;
			}  // 5B = BIT 3,E	 | Fg: Z 0 1 - | Sz: 2 | Cc: 8
			
			INST_H(5C, BIT_3_H) {
				BIT(3, H);
				continue;
			}  // 5C = BIT 3,H	 | Fg: Z 0 1 - | Sz: 2 | Cc: 8
			
			INST_H(5D, BIT_3_L) {
				BIT(3, L);
				continue;
			}  // 5D = BIT 3,L	 | Fg: Z 0 1 - | Sz: 2 | Cc: 8
			
			INST_H(5E, BIT_3_(HL)) {
				r = cpu_read(HL);
				BIT(3, r);
				continue;
			}  // 5E = BIT 3,(HL)	 | Fg: Z 0 1 - | Sz: 2 | Cc: 12
			
			INST_H(5F, BIT_3_A) {
				BIT(3, A);
				continue;
			}  // 5F = BIT 3,A	 | Fg: Z 0 1 - | Sz: 2 | Cc: 8
			
			INST_H(60, BIT_4_B) {
				BIT(4, B);
				continue;
			}  // 60 = BIT 4,B	 | Fg: Z 0 1 - | Sz: 2 | Cc: 8
			
			INST_H(61, BIT_4_C) {
				BIT(4, C);
				continue;
			}  // 61 = BIT 4,C	 | Fg: Z 0 1 - | Sz: 2 | Cc: 8
			
			INST_H(62, BIT_4_D) {
				BIT(4, D);
				continue;
			}  // 62 = BIT 4,D	 | Fg: Z 0 1 - | Sz: 2 | Cc: 8
			
			INST_H(63, BIT_4_E) {
				BIT(4, E);
				continue;
			}  // 63 = BIT 4,E	 | Fg: Z 0 1 - | Sz: 2 | Cc: 8
			
			INST_H(64, BIT_4_H) {
				BIT(4, H);
				continue;
			}  // 64 = BIT 4,H	 | Fg: Z 0 1 - | Sz: 2 | Cc: 8
			
			INST_H(65, BIT_4_L) {
				BIT(4, L);
				continue;
			}  // 65 = BIT 4,L	 | Fg: Z 0 1 - | Sz: 2 | Cc: 8
			
			INST_H(66, BIT_4_(HL)) {
				r = cpu_read(HL);
				BIT(4, r);
				continue;
			}  // 66 = BIT 4,(HL)	 | Fg: Z 0 1 - | Sz: 2 | Cc: 12
			
			INST_H(67, BIT_4_A) {
				BIT(4, A);
				continue;
			}  // 67 = BIT 4,A	 | Fg: Z 0 1 - | Sz: 2 | Cc: 8
			
			INST_H(68, BIT_5_B) {
				BIT(5, B);
				continue;
			}  // 68 = BIT 5,B	 | Fg: Z 0 1 - | Sz: 2 | Cc: 8
			
			INST_H(69, BIT_5_C) {
				BIT(5, C);
				continue;
			}  // 69 = BIT 5,C	 | Fg: Z 0 1 - | Sz: 2 | Cc: 8
			
			INST_H(6A, BIT_5_D) {
				BIT(5, D);
				continue;
			}  // 6A = BIT 5,D	 | Fg: Z 0 1 - | Sz: 2 | Cc: 8
			
			INST_H(6B, BIT_5_E) {
				BIT(5, E);
				continue;
			}  // 6B = BIT 5,E	 | Fg: Z 0 1 - | Sz: 2 | Cc: 8
			
			INST_H(6C, BIT_5_H) {
				BIT(5, H);
				continue;
			}  // 6C = BIT 5,H	 | Fg: Z 0 1 - | Sz: 2 | Cc: 8
			
			INST_H(6D, BIT_5_L) {
				BIT(5, L);
				continue;
			}  // 6D = BIT 5,L	 | Fg: Z 0 1 - | Sz: 2 | Cc: 8
			
			INST_H(6E, BIT_5_(HL)) {
				r = cpu_read(HL);
				BIT(5, r);
				continue;
			}  // 6E = BIT 5,(HL)	 | Fg: Z 0 1 - | Sz: 2 | Cc: 12
			
			INST_H(6F, BIT_5_A) {
				BIT(5, A);
				continue;
			}  // 6F = BIT 5,A	 | Fg: Z 0 1 - | Sz: 2 | Cc: 8
			
			INST_H(70, BIT_6_B) {
				BIT(6, B);
				continue;
			}  // 70 = BIT 6,B	 | Fg: Z 0 1 - | Sz: 2 | Cc: 8
			
			INST_H(71, BIT_6_C) {
				BIT(6, C);
				continue;
			}  // 71 = BIT 6,C	 | Fg: Z 0 1 - | Sz: 2 | Cc: 8
			
			INST_H(72, BIT_6_D) {
				BIT(6, D);
				continue;
			}  // 72 = BIT 6,D	 | Fg: Z 0 1 - | Sz: 2 | Cc: 8
			
			INST_H(73, BIT_6_E) {
				BIT(6, E);
				continue;
			}  // 73 = BIT 6,E	 | Fg: Z 0 1 - | Sz: 2 | Cc: 8
			
			INST_H(74, BIT_6_H) {
				BIT(6, H);
				continue;
			}  // 74 = BIT 6,H	 | Fg: Z 0 1 - | Sz: 2 | Cc: 8
			
			INST_H(75, BIT_6_L) {
				BIT(6, L);
				continue;
			}  // 75 = BIT 6,L	 | Fg: Z 0 1 - | Sz: 2 | Cc: 8
			
			INST_H(76, BIT_6_(HL)) {
				r = cpu_read(HL);
				BIT(6, r);
				continue;
			}  // 76 = BIT 6,(HL)	 | Fg: Z 0 1 - | Sz: 2 | Cc: 12
			
			INST_H(77, BIT_6_A) {
				BIT(6, A);
				continue;
			}  // 77 = BIT 6,A	 | Fg: Z 0 1 - | Sz: 2 | Cc: 8
			
			INST_H(78, BIT_7_B) {
				BIT(7, B);
				continue;
			}  // 78 = BIT 7,B	 | Fg: Z 0 1 - | Sz: 2 | Cc: 8
			
			INST_H(79, BIT_7_C) {
				BIT(7, C);
				continue;
			}  // 79 = BIT 7,C	 | Fg: Z 0 1 - | Sz: 2 | Cc: 8
			
			INST_H(7A, BIT_7_D) {
				BIT(7, D);
				continue;
			}  // 7A = BIT 7,D	 | Fg: Z 0 1 - | Sz: 2 | Cc: 8
			
			INST_H(7B, BIT_7_E) {
				BIT(7, E);
				continue;
			}  // 7B = BIT 7,E	 | Fg: Z 0 1 - | Sz: 2 | Cc: 8
			
			INST_H(7C, BIT_7_H) {
				BIT(7, H);
				continue;
			}  // 7C = BIT 7,H	 | Fg: Z 0 1 - | Sz: 2 | Cc: 8
			
			INST_H(7D, BIT_7_L) {
				BIT(7, L);
				continue;
			}  // 7D = BIT 7,L	 | Fg: Z 0 1 - | Sz: 2 | Cc: 8
			
			INST_H(7E, BIT_7_(HL)) {
				r = cpu_read(HL);
				BIT(7, r);
				continue;
			}  // 7E = BIT 7,(HL)	 | Fg: Z 0 1 - | Sz: 2 | Cc: 12
			
			INST_H(7F, BIT_7_A) {
				BIT(7, A);
				continue;
			}  // 7F = BIT 7,A	 | Fg: Z 0 1 - | Sz: 2 | Cc: 8
			
			INST_H(80, RES_0_B) {
				RES(0, B);
				continue;
			}  // 80 = RES 0,B	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(81, RES_0_C) {
				RES(0, C);
				continue;
			}  // 81 = RES 0,C	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(82, RES_0_D) {
				RES(0, D);
				continue;
			}  // 82 = RES 0,D	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(83, RES_0_E) {
				RES(0, E);
				continue;
			}  // 83 = RES 0,E	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(84, RES_0_H) {
				RES(0, H);
				continue;
			}  // 84 = RES 0,H	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(85, RES_0_L) {
				RES(0, L);
				continue;
			}  // 85 = RES 0,L	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(86, RES_0_(HL)) {
				r = cpu_read(HL);
				RES(0, r);
				cpu_write(HL, r);
				continue;
			}  // 86 = RES 0,(HL)	 | Fg: - - - - | Sz: 2 | Cc: 16
			
			INST_H(87, RES_0_A) {
				RES(0, A);
				continue;
			}  // 87 = RES 0,A	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(88, RES_1_B) {
				RES(1, B);
				continue;
			}  // 88 = RES 1,B	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(89, RES_1_C) {
				RES(1, C);
				continue;
			}  // 89 = RES 1,C	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(8A, RES_1_D) {
				RES(1, D);
				continue;
			}  // 8A = RES 1,D	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(8B, RES_1_E) {
				RES(1, E);
				continue;
			}  // 8B = RES 1,E	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(8C, RES_1_H) {
				RES(1, H);
				continue;
			}  // 8C = RES 1,H	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(8D, RES_1_L) {
				RES(1, L);
				continue;
			}  // 8D = RES 1,L	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(8E, RES_1_(HL)) {
				r = cpu_read(HL);
				RES(1, r);
				cpu_write(HL, r);
				continue;
			}  // 8E = RES 1,(HL)	 | Fg: - - - - | Sz: 2 | Cc: 16
			
			INST_H(8F, RES_1_A) {
				RES(1, A);
				continue;
			}  // 8F = RES 1,A	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(90, RES_2_B) {
				RES(2, B);
				continue;
			}  // 90 = RES 2,B	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(91, RES_2_C) {
				RES(2, C);
				continue;
			}  // 91 = RES 2,C	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(92, RES_2_D) {
				RES(2, D);
				continue;
			}  // 92 = RES 2,D	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(93, RES_2_E) {
				RES(2, E);
				continue;
			}  // 93 = RES 2,E	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(94, RES_2_H) {
				RES(2, H);
				continue;
			}  // 94 = RES 2,H	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(95, RES_2_L) {
				RES(2, L);
				continue;
			}  // 95 = RES 2,L	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(96, RES_2_(HL)) {
				r = cpu_read(HL);
				RES(2, r);
				cpu_write(HL, r);
				continue;
			}  // 96 = RES 2,(HL)	 | Fg: - - - - | Sz: 2 | Cc: 16
			
			INST_H(97, RES_2_A) {
				RES(2, A);
				continue;
			}  // 97 = RES 2,A	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(98, RES_3_B) {
				RES(3, B);
				continue;
			}  // 98 = RES 3,B	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(99, RES_3_C) {
				RES(3, C);
				continue;
			}  // 99 = RES 3,C	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(9A, RES_3_D) {
				RES(3, D);
				continue;
			}  // 9A = RES 3,D	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(9B, RES_3_E) {
				RES(3, E);
				continue;
			}  // 9B = RES 3,E	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(9C, RES_3_H) {
				RES(3, H);
				continue;
			}  // 9C = RES 3,H	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(9D, RES_3_L) {
				RES(3, L);
				continue;
			}  // 9D = RES 3,L	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(9E, RES_3_(HL)) {
				r = cpu_read(HL);
				RES(3, r);
				cpu_write(HL, r);
				continue;
			}  // 9E = RES 3,(HL)	 | Fg: - - - - | Sz: 2 | Cc: 16
			
			INST_H(9F, RES_3_A) {
				RES(3, A);
				continue;
			}  // 9F = RES 3,A	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(A0, RES_4_B) {
				RES(4, B);
				continue;
			}  // A0 = RES 4,B	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(A1, RES_4_C) {
				RES(4, C);
				continue;
			}  // A1 = RES 4,C	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(A2, RES_4_D) {
				RES(4, D);
				continue;
			}  // A2 = RES 4,D	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(A3, RES_4_E) {
				RES(4, E);
				continue;
			}  // A3 = RES 4,E	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(A4, RES_4_H) {
				RES(4, H);
				continue;
			}  // A4 = RES 4,H	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(A5, RES_4_L) {
				RES(4, L);
				continue;
			}  // A5 = RES 4,L	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(A6, RES_4_(HL)) {
				r = cpu_read(HL);
				RES(4, r);
				cpu_write(HL, r);
				continue;
			}  // A6 = RES 4,(HL)	 | Fg: - - - - | Sz: 2 | Cc: 16
			
			INST_H(A7, RES_4_A) {
				RES(4, A);
				continue;
			}  // A7 = RES 4,A	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(A8, RES_5_B) {
				RES(5, B);
				continue;
			}  // A8 = RES 5,B	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(A9, RES_5_C) {
				RES(5, C);
				continue;
			}  // A9 = RES 5,C	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(AA, RES_5_D) {
				RES(5, D);
				continue;
			}  // AA = RES 5,D	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(AB, RES_5_E) {
				RES(5, E);
				continue;
			}  // AB = RES 5,E	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(AC, RES_5_H) {
				RES(5, H);
				continue;
			}  // AC = RES 5,H	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(AD, RES_5_L) {
				RES(5, L);
				continue;
			}  // AD = RES 5,L	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(AE, RES_5_(HL)) {
				r = cpu_read(HL);
				RES(5, r);
				cpu_write(HL, r);
				continue;
			}  // AE = RES 5,(HL)	 | Fg: - - - - | Sz: 2 | Cc: 16
			
			INST_H(AF, RES_5_A) {
				RES(5, A);
				continue;
			}  // AF = RES 5,A	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(B0, RES_6_B) {
				RES(6, B);
				continue;
			}  // B0 = RES 6,B	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(B1, RES_6_C) {
				RES(6, C);
				continue;
			}  // B1 = RES 6,C	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(B2, RES_6_D) {
				RES(6, D);
				continue;
			}  // B2 = RES 6,D	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(B3, RES_6_E) {
				RES(6, E);
				continue;
			}  // B3 = RES 6,E	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(B4, RES_6_H) {
				RES(6, H);
				continue;
			}  // B4 = RES 6,H	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(B5, RES_6_L) {
				RES(6, L);
				continue;
			}  // B5 = RES 6,L	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(B6, RES_6_(HL)) {
				r = cpu_read(HL);
				RES(6, r);
				cpu_write(HL, r);
				continue;
			}  // B6 = RES 6,(HL)	 | Fg: - - - - | Sz: 2 | Cc: 16
			
			INST_H(B7, RES_6_A) {
				RES(6, A);
				continue;
			}  // B7 = RES 6,A	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(B8, RES_7_B) {
				RES(7, B);
				continue;
			}  // B8 = RES 7,B	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(B9, RES_7_C) {
				RES(7, C);
				continue;
			}  // B9 = RES 7,C	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(BA, RES_7_D) {
				RES(7, D);
				continue;
			}  // BA = RES 7,D	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(BB, RES_7_E) {
				RES(7, E);
				continue;
			}  // BB = RES 7,E	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(BC, RES_7_H) {
				RES(7, H);
				continue;
			}  // BC = RES 7,H	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(BD, RES_7_L) {
				RES(7, L);
				continue;
			}  // BD = RES 7,L	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(BE, RES_7_(HL)) {
				r = cpu_read(HL);
				RES(7, r);
				cpu_write(HL, r);
				continue;
			}  // BE = RES 7,(HL)	 | Fg: - - - - | Sz: 2 | Cc: 16
			
			INST_H(BF, RES_7_A) {
				RES(7, A);
				continue;
			}  // BF = RES 7,A	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(10, RL_B) {
				RL(B);
				continue;
			}  // 10 = RL B	 | Fg: Z 0 0 C | Sz: 2 | Cc: 8
			
			INST_H(11, RL_C) {
				RL(C);
				continue;
			}  // 11 = RL C	 | Fg: Z 0 0 C | Sz: 2 | Cc: 8
			
			INST_H(12, RL_D) {
				RL(D);
				continue;
			}  // 12 = RL D	 | Fg: Z 0 0 C | Sz: 2 | Cc: 8
			
			INST_H(13, RL_E) {
				RL(E);
				continue;
			}  // 13 = RL E	 | Fg: Z 0 0 C | Sz: 2 | Cc: 8
			
			INST_H(14, RL_H) {
				RL(H);
				continue;
			}  // 14 = RL H	 | Fg: Z 0 0 C | Sz: 2 | Cc: 8
			
			INST_H(15, RL_L) {
				RL(L);
				continue;
			}  // 15 = RL L	 | Fg: Z 0 0 C | Sz: 2 | Cc: 8
			
			INST_H(16, RL_(HL)) {
				r = cpu_read(HL);
				RL(r);
				cpu_write(HL, r);
				continue;
			}  // 16 = RL (HL)	 | Fg: Z 0 0 C | Sz: 2 | Cc: 16
			
			INST_H(17, RL_A) {
				RL(A);
				continue;
			}  // 17 = RL A	 | Fg: Z 0 0 C | Sz: 2 | Cc: 8
			
			INST_H(00, RLC_B) {
				RLC(B);
				continue;
			}  // 00 = RLC B	 | Fg: Z 0 0 C | Sz: 2 | Cc: 8
			
			INST_H(01, RLC_C) {
				RLC(C);
				continue;
			}  // 01 = RLC C	 | Fg: Z 0 0 C | Sz: 2 | Cc: 8
			
			INST_H(02, RLC_D) {
				RLC(D);
				continue;
			}  // 02 = RLC D	 | Fg: Z 0 0 C | Sz: 2 | Cc: 8
			
			INST_H(03, RLC_E) {
				RLC(E);
				continue;
			}  // 03 = RLC E	 | Fg: Z 0 0 C | Sz: 2 | Cc: 8
			
			INST_H(04, RLC_H) {
				RLC(H);
				continue;
			}  // 04 = RLC H	 | Fg: Z 0 0 C | Sz: 2 | Cc: 8
			
			INST_H(05, RLC_L) {
				RLC(L);
				continue;
			}  // 05 = RLC L	 | Fg: Z 0 0 C | Sz: 2 | Cc: 8
			
			INST_H(06, RLC_(HL)) {
				r = cpu_read(HL);
				RLC(r);
				cpu_write(HL, r);
				continue;
			}  // 06 = RLC (HL)	 | Fg: Z 0 0 C | Sz: 2 | Cc: 16
			
			INST_H(07, RLC_A) {
				RLC(A);
				continue;
			}  // 07 = RLC A	 | Fg: Z 0 0 C | Sz: 2 | Cc: 8
			
			INST_H(18, RR_B) {
				RR(B);
				continue;
			}  // 18 = RR B	 | Fg: Z 0 0 C | Sz: 2 | Cc: 8
			
			INST_H(19, RR_C) {
				RR(C);
				continue;
			}  // 19 = RR C	 | Fg: Z 0 0 C | Sz: 2 | Cc: 8
			
			INST_H(1A, RR_D) {
				RR(D);
				continue;
			}  // 1A = RR D	 | Fg: Z 0 0 C | Sz: 2 | Cc: 8
			
			INST_H(1B, RR_E) {
				RR(E);
				continue;
			}  // 1B = RR E	 | Fg: Z 0 0 C | Sz: 2 | Cc: 8
			
			INST_H(1C, RR_H) {
				RR(H);
				continue;
			}  // 1C = RR H	 | Fg: Z 0 0 C | Sz: 2 | Cc: 8
			
			INST_H(1D, RR_L) {
				RR(L);
				continue;
			}  // 1D = RR L	 | Fg: Z 0 0 C | Sz: 2 | Cc: 8
			
			INST_H(1E, RR_(HL)) {
				r = cpu_read(HL);
				RR(r);
				cpu_write(HL, r);
				continue;
			}  // 1E = RR (HL)	 | Fg: Z 0 0 C | Sz: 2 | Cc: 16
			
			INST_H(1F, RR_A) {
				RR(A);
				continue;
			}  // 1F = RR A	 | Fg: Z 0 0 C | Sz: 2 | Cc: 8
			
			INST_H(08, RRC_B) {
				RRC(B);
				continue;
			}  // 08 = RRC B	 | Fg: Z 0 0 C | Sz: 2 | Cc: 8
			
			INST_H(09, RRC_C) {
				RRC(C);
				continue;
			}  // 09 = RRC C	 | Fg: Z 0 0 C | Sz: 2 | Cc: 8
			
			INST_H(0A, RRC_D) {
				RRC(D);
				continue;
			}  // 0A = RRC D	 | Fg: Z 0 0 C | Sz: 2 | Cc: 8
			
			INST_H(0B, RRC_E) {
				RRC(E);
				continue;
			}  // 0B = RRC E	 | Fg: Z 0 0 C | Sz: 2 | Cc: 8
			
			INST_H(0C, RRC_H) {
				RRC(H);
				continue;
			}  // 0C = RRC H	 | Fg: Z 0 0 C | Sz: 2 | Cc: 8
			
			INST_H(0D, RRC_L) {
				RRC(L);
				continue;
			}  // 0D = RRC L	 | Fg: Z 0 0 C | Sz: 2 | Cc: 8
			
			INST_H(0E, RRC_(HL)) {
				r = cpu_read(HL);
				RRC(r);
				cpu_write(HL, r);
				continue;
			}  // 0E = RRC (HL)	 | Fg: Z 0 0 C | Sz: 2 | Cc: 16
			
			INST_H(0F, RRC_A) {
				RRC(A);
				continue;
			}  // 0F = RRC A	 | Fg: Z 0 0 C | Sz: 2 | Cc: 8
			
			INST_H(C0, SET_0_B) {
				SET(0, B);
				continue;
			}  // C0 = SET 0,B	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(C1, SET_0_C) {
				SET(0, C);
				continue;
			}  // C1 = SET 0,C	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(C2, SET_0_D) {
				SET(0, D);
				continue;
			}  // C2 = SET 0,D	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(C3, SET_0_E) {
				SET(0, E);
				continue;
			}  // C3 = SET 0,E	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(C4, SET_0_H) {
				SET(0, H);
				continue;
			}  // C4 = SET 0,H	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(C5, SET_0_L) {
				SET(0, L);
				continue;
			}  // C5 = SET 0,L	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(C6, SET_0_(HL)) {
				r = cpu_read(HL);
				SET(0, r);
				cpu_write(HL, r);
				continue;
			}  // C6 = SET 0,(HL)	 | Fg: - - - - | Sz: 2 | Cc: 16
			
			INST_H(C7, SET_0_A) {
				SET(0, A);
				continue;
			}  // C7 = SET 0,A	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(C8, SET_1_B) {
				SET(1, B);
				continue;
			}  // C8 = SET 1,B	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(C9, SET_1_C) {
				SET(1, C);
				continue;
			}  // C9 = SET 1,C	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(CA, SET_1_D) {
				SET(1, D);
				continue;
			}  // CA = SET 1,D	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(CB, SET_1_E) {
				SET(1, E);
				continue;
			}  // CB = SET 1,E	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(CC, SET_1_H) {
				SET(1, H);
				continue;
			}  // CC = SET 1,H	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(CD, SET_1_L) {
				SET(1, L);
				continue;
			}  // CD = SET 1,L	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(CE, SET_1_(HL)) {
				r = cpu_read(HL);
				SET(1, r);
				cpu_write(HL, r);
				continue;
			}  // CE = SET 1,(HL)	 | Fg: - - - - | Sz: 2 | Cc: 16
			
			INST_H(CF, SET_1_A) {
				SET(1, A);
				continue;
			}  // CF = SET 1,A	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(D0, SET_2_B) {
				SET(2, B);
				continue;
			}  // D0 = SET 2,B	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(D1, SET_2_C) {
				SET(2, C);
				continue;
			}  // D1 = SET 2,C	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(D2, SET_2_D) {
				SET(2, D);
				continue;
			}  // D2 = SET 2,D	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(D3, SET_2_E) {
				SET(2, E);
				continue;
			}  // D3 = SET 2,E	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(D4, SET_2_H) {
				SET(2, H);
				continue;
			}  // D4 = SET 2,H	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(D5, SET_2_L) {
				SET(2, L);
				continue;
			}  // D5 = SET 2,L	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(D6, SET_2_(HL)) {
				r = cpu_read(HL);
				SET(2, r);
				cpu_write(HL, r);
				continue;
			}  // D6 = SET 2,(HL)	 | Fg: - - - - | Sz: 2 | Cc: 16
			
			INST_H(D7, SET_2_A) {
				SET(2, A);
				continue;
			}  // D7 = SET 2,A	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(D8, SET_3_B) {
				SET(3, B);
				continue;
			}  // D8 = SET 3,B	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(D9, SET_3_C) {
				SET(3, C);
				continue;
			}  // D9 = SET 3,C	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(DA, SET_3_D) {
				SET(3, D);
				continue;
			}  // DA = SET 3,D	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(DB, SET_3_E) {
				SET(3, E);
				continue;
			}  // DB = SET 3,E	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(DC, SET_3_H) {
				SET(3, H);
				continue;
			}  // DC = SET 3,H	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(DD, SET_3_L) {
				SET(3, L);
				continue;
			}  // DD = SET 3,L	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(DE, SET_3_(HL)) {
				r = cpu_read(HL);
				SET(3, r);
				cpu_write(HL, r);
				continue;
			}  // DE = SET 3,(HL)	 | Fg: - - - - | Sz: 2 | Cc: 16
			
			INST_H(DF, SET_3_A) {
				SET(3, A);
				continue;
			}  // DF = SET 3,A	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(E0, SET_4_B) {
				SET(4, B);
				continue;
			}  // E0 = SET 4,B	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(E1, SET_4_C) {
				SET(4, C);
				continue;
			}  // E1 = SET 4,C	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(E2, SET_4_D) {
				SET(4, D);
				continue;
			}  // E2 = SET 4,D	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(E3, SET_4_E) {
				SET(4, E);
				continue;
			}  // E3 = SET 4,E	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(E4, SET_4_H) {
				SET(4, H);
				continue;
			}  // E4 = SET 4,H	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(E5, SET_4_L) {
				SET(4, L);
				continue;
			}  // E5 = SET 4,L	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(E6, SET_4_(HL)) {
				r = cpu_read(HL);
				SET(4, r);
				cpu_write(HL, r);
				continue;
			}  // E6 = SET 4,(HL)	 | Fg: - - - - | Sz: 2 | Cc: 16
			
			INST_H(E7, SET_4_A) {
				SET(4, A);
				continue;
			}  // E7 = SET 4,A	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(E8, SET_5_B) {
				SET(5, B);
				continue;
			}  // E8 = SET 5,B	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(E9, SET_5_C) {
				SET(5, C);
				continue;
			}  // E9 = SET 5,C	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(EA, SET_5_D) {
				SET(5, D);
				continue;
			}  // EA = SET 5,D	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(EB, SET_5_E) {
				SET(5, E);
				continue;
			}  // EB = SET 5,E	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(EC, SET_5_H) {
				SET(5, H);
				continue;
			}  // EC = SET 5,H	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(ED, SET_5_L) {
				SET(5, L);
				continue;
			}  // ED = SET 5,L	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(EE, SET_5_(HL)) {
				r = cpu_read(HL);
				SET(5, r);
				cpu_write(HL, r);
				continue;
			}  // EE = SET 5,(HL)	 | Fg: - - - - | Sz: 2 | Cc: 16
			
			INST_H(EF, SET_5_A) {
				SET(5, A);
				continue;
			}  // EF = SET 5,A	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(F0, SET_6_B) {
				SET(6, B);
				continue;
			}  // F0 = SET 6,B	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(F1, SET_6_C) {
				SET(6, C);
				continue;
			}  // F1 = SET 6,C	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(F2, SET_6_D) {
				SET(6, D);
				continue;
			}  // F2 = SET 6,D	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(F3, SET_6_E) {
				SET(6, E);
				continue;
			}  // F3 = SET 6,E	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(F4, SET_6_H) {
				SET(6, H);
				continue;
			}  // F4 = SET 6,H	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(F5, SET_6_L) {
				SET(6, L);
				continue;
			}  // F5 = SET 6,L	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(F6, SET_6_(HL)) {
				r = cpu_read(HL);
				SET(6, r);
				cpu_write(HL, r);
				continue;
			}  // F6 = SET 6,(HL)	 | Fg: - - - - | Sz: 2 | Cc: 16
			
			INST_H(F7, SET_6_A) {
				SET(6, A);
				continue;
			}  // F7 = SET 6,A	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(F8, SET_7_B) {
				SET(7, B);
				continue;
			}  // F8 = SET 7,B	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(F9, SET_7_C) {
				SET(7, C);
				continue;
			}  // F9 = SET 7,C	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(FA, SET_7_D) {
				SET(7, D);
				continue;
			}  // FA = SET 7,D	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(FB, SET_7_E) {
				SET(7, E);
				continue;
			}  // FB = SET 7,E	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(FC, SET_7_H) {
				SET(7, H);
				continue;
			}  // FC = SET 7,H	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(FD, SET_7_L) {
				SET(7, L);
				continue;
			}  // FD = SET 7,L	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(FE, SET_7_(HL)) {
				r = cpu_read(HL);
				SET(7, r);
				cpu_write(HL, r);
				continue;
			}  // FE = SET 7,(HL)	 | Fg: - - - - | Sz: 2 | Cc: 16
			
			INST_H(FF, SET_7_A) {
				SET(7, A);
				continue;
			}  // FF = SET 7,A	 | Fg: - - - - | Sz: 2 | Cc: 8
			
			INST_H(20, SLA_B) {
				SLA(B);
				continue;
			}  // 20 = SLA B	 | Fg: Z 0 0 C | Sz: 2 | Cc: 8
			
			INST_H(21, SLA_C) {
				SLA(C);
				continue;
			}  // 21 = SLA C	 | Fg: Z 0 0 C | Sz: 2 | Cc: 8
			
			INST_H(22, SLA_D) {
				SLA(D);
				continue;
			}  // 22 = SLA D	 | Fg: Z 0 0 C | Sz: 2 | Cc: 8
			
			INST_H(23, SLA_E) {
				SLA(E);
				continue;
			}  // 23 = SLA E	 | Fg: Z 0 0 C | Sz: 2 | Cc: 8
			
			INST_H(24, SLA_H) {
				SLA(H);
				continue;
			}  // 24 = SLA H	 | Fg: Z 0 0 C | Sz: 2 | Cc: 8
			
			INST_H(25, SLA_L) {
				SLA(L);
				continue;
			}  // 25 = SLA L	 | Fg: Z 0 0 C | Sz: 2 | Cc: 8
			
			INST_H(26, SLA_(HL)) {
				r = cpu_read(HL);
				SLA(r);
				cpu_write(HL, r);
				continue;
			}  // 26 = SLA (HL)	 | Fg: Z 0 0 C | Sz: 2 | Cc: 16
			
			INST_H(27, SLA_A) {
				SLA(A);
				continue;
			}  // 27 = SLA A	 | Fg: Z 0 0 C | Sz: 2 | Cc: 8
			
			INST_H(28, SRA_B) {
				SRA(B);
				continue;
			}  // 28 = SRA B	 | Fg: Z 0 0 0 | Sz: 2 | Cc: 8
			
			INST_H(29, SRA_C) {
				SRA(C);
				continue;
			}  // 29 = SRA C	 | Fg: Z 0 0 0 | Sz: 2 | Cc: 8
			
			INST_H(2A, SRA_D) {
				SRA(D);
				continue;
			}  // 2A = SRA D	 | Fg: Z 0 0 0 | Sz: 2 | Cc: 8
			
			INST_H(2B, SRA_E) {
				SRA(E);
				continue;
			}  // 2B = SRA E	 | Fg: Z 0 0 0 | Sz: 2 | Cc: 8
			
			INST_H(2C, SRA_H) {
				SRA(H);
				continue;
			}  // 2C = SRA H	 | Fg: Z 0 0 0 | Sz: 2 | Cc: 8
			
			INST_H(2D, SRA_L) {
				SRA(L);
				continue;
			}  // 2D = SRA L	 | Fg: Z 0 0 0 | Sz: 2 | Cc: 8
			
			INST_H(2E, SRA_(HL)) {
				r = cpu_read(HL);
				SRA(r);
				cpu_write(HL, r);
				continue;
			}  // 2E = SRA (HL)	 | Fg: Z 0 0 0 | Sz: 2 | Cc: 16
			
			INST_H(2F, SRA_A) {
				SRA(A);
				continue;
			}  // 2F = SRA A	 | Fg: Z 0 0 0 | Sz: 2 | Cc: 8
			
			INST_H(38, SRL_B) {
				SRL(B);
				continue;
			}  // 38 = SRL B	 | Fg: Z 0 0 C | Sz: 2 | Cc: 8
			
			INST_H(39, SRL_C) {
				SRL(C);
				continue;
			}  // 39 = SRL C	 | Fg: Z 0 0 C | Sz: 2 | Cc: 8
			
			INST_H(3A, SRL_D) {
				SRL(D);
				continue;
			}  // 3A = SRL D	 | Fg: Z 0 0 C | Sz: 2 | Cc: 8
			
			INST_H(3B, SRL_E) {
				SRL(E);
				continue;
			}  // 3B = SRL E	 | Fg: Z 0 0 C | Sz: 2 | Cc: 8
			
			INST_H(3C, SRL_H) {
				SRL(H);
				continue;
			}  // 3C = SRL H	 | Fg: Z 0 0 C | Sz: 2 | Cc: 8
			
			INST_H(3D, SRL_L) {
				SRL(L);
				continue;
			}  // 3D = SRL L	 | Fg: Z 0 0 C | Sz: 2 | Cc: 8
			
			INST_H(3E, SRL_(HL)) {
				r = cpu_read(HL);
				SRL(r);
				cpu_write(HL, r);
				continue;
			}  // 3E = SRL (HL)	 | Fg: Z 0 0 C | Sz: 2 | Cc: 16
			
			INST_H(3F, SRL_A) {
				SRL(A);
				continue;
			}  // 3F = SRL A	 | Fg: Z 0 0 C | Sz: 2 | Cc: 8
			
			INST_H(30, SWAP_B) {
				SWAP(B);
				continue;
			}  // 30 = SWAP B	 | Fg: Z 0 0 0 | Sz: 2 | Cc: 8
			
			INST_H(31, SWAP_C) {
				SWAP(C);
				continue;
			}  // 31 = SWAP C	 | Fg: Z 0 0 0 | Sz: 2 | Cc: 8
			
			INST_H(32, SWAP_D) {
				SWAP(D);
				continue;
			}  // 32 = SWAP D	 | Fg: Z 0 0 0 | Sz: 2 | Cc: 8
			
			INST_H(33, SWAP_E) {
				SWAP(E);
				continue;
			}  // 33 = SWAP E	 | Fg: Z 0 0 0 | Sz: 2 | Cc: 8
			
			INST_H(34, SWAP_H) {
				SWAP(H);
				continue;
			}  // 34 = SWAP H	 | Fg: Z 0 0 0 | Sz: 2 | Cc: 8
			
			INST_H(35, SWAP_L) {
				SWAP(L);
				continue;
			}  // 35 = SWAP L	 | Fg: Z 0 0 0 | Sz: 2 | Cc: 8
			
			INST_H(36, SWAP_(HL)) {
				r = cpu_read(HL);
				SWAP(r);
				cpu_write(HL, r);
				continue;
			}  // 36 = SWAP (HL)	 | Fg: Z 0 0 0 | Sz: 2 | Cc: 16
			
			INST_H(37, SWAP_A) {
				SWAP(A);
				continue;
			}  // 37 = SWAP A	 | Fg: Z 0 0 0 | Sz: 2 | Cc: 8

#pragma endregion
	
	} while (!isFrameReady());
	
	//ppu_mem.frame_ready = 0;
	
	
	Registers.reg_AF = Reg_AF;
	Registers.reg_BC = Reg_BC;
	Registers.reg_DE = Reg_DE;
	Registers.reg_HL = Reg_HL;
	Registers.reg_PC = Reg_PC;
	Registers.reg_SP = Reg_SP;
	Registers.reg_OPERAND = Reg_OPERAND;
	Registers.reg_OPCODE = Reg_OPCODE;
}
