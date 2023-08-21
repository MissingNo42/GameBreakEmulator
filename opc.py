r1 = """
NOP	LD BC,d16	LD (BC),A	INC BC	INC B	DEC B	LD B,d8	RLCA	LD (a16),SP	ADD HL,BC	LD A,(BC)	DEC BC	INC C	DEC C	LD C,d8	RRCA
1  4	3  12	1  8	1  8	1  4	1  4	2  8	1  4	3  20	1  8	1  8	1  8	1  4	1  4	2  8	1  4
- - - -	- - - -	- - - -	- - - -	Z 0 H -	Z 1 H -	- - - -	0 0 0 C	- - - -	- 0 H C	- - - -	- - - -	Z 0 H -	Z 1 H -	- - - -	0 0 0 C
STOP 0	LD DE,d16	LD (DE),A	INC DE	INC D	DEC D	LD D,d8	RLA	JR r8	ADD HL,DE	LD A,(DE)	DEC DE	INC E	DEC E	LD E,d8	RRA
1  4	3  12	1  8	1  8	1  4	1  4	2  8	1  4	2  12	1  8	1  8	1  8	1  4	1  4	2  8	1  4
- - - -	- - - -	- - - -	- - - -	Z 0 H -	Z 1 H -	- - - -	0 0 0 C	- - - -	- 0 H C	- - - -	- - - -	Z 0 H -	Z 1 H -	- - - -	0 0 0 C
JR NZ,r8	LD HL,d16	LD (HL+),A	INC HL	INC H	DEC H	LD H,d8	DAA	JR Z,r8	ADD HL,HL	LD A,(HL+)	DEC HL	INC L	DEC L	LD L,d8	CPL
2  12/8	3  12	1  8	1  8	1  4	1  4	2  8	1  4	2  12/8	1  8	1  8	1  8	1  4	1  4	2  8	1  4
- - - -	- - - -	- - - -	- - - -	Z 0 H -	Z 1 H -	- - - -	Z - 0 C	- - - -	- 0 H C	- - - -	- - - -	Z 0 H -	Z 1 H -	- - - -	- 1 1 -
JR NC,r8	LD SP,d16	LD (HL-),A	INC SP	INC (HL)	DEC (HL)	LD (HL),d8	SCF	JR C,r8	ADD HL,SP	LD A,(HL-)	DEC SP	INC A	DEC A	LD A,d8	CCF
2  12/8	3  12	1  8	1  8	1  12	1  12	2  12	1  4	2  12/8	1  8	1  8	1  8	1  4	1  4	2  8	1  4
- - - -	- - - -	- - - -	- - - -	Z 0 H -	Z 1 H -	- - - -	- 0 0 1	- - - -	- 0 H C	- - - -	- - - -	Z 0 H -	Z 1 H -	- - - -	- 0 0 C
LD B,B	LD B,C	LD B,D	LD B,E	LD B,H	LD B,L	LD B,(HL)	LD B,A	LD C,B	LD C,C	LD C,D	LD C,E	LD C,H	LD C,L	LD C,(HL)	LD C,A
1  4	1  4	1  4	1  4	1  4	1  4	1  8	1  4	1  4	1  4	1  4	1  4	1  4	1  4	1  8	1  4
- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -
LD D,B	LD D,C	LD D,D	LD D,E	LD D,H	LD D,L	LD D,(HL)	LD D,A	LD E,B	LD E,C	LD E,D	LD E,E	LD E,H	LD E,L	LD E,(HL)	LD E,A
1  4	1  4	1  4	1  4	1  4	1  4	1  8	1  4	1  4	1  4	1  4	1  4	1  4	1  4	1  8	1  4
- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -
LD H,B	LD H,C	LD H,D	LD H,E	LD H,H	LD H,L	LD H,(HL)	LD H,A	LD L,B	LD L,C	LD L,D	LD L,E	LD L,H	LD L,L	LD L,(HL)	LD L,A
1  4	1  4	1  4	1  4	1  4	1  4	1  8	1  4	1  4	1  4	1  4	1  4	1  4	1  4	1  8	1  4
- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -
LD (HL),B	LD (HL),C	LD (HL),D	LD (HL),E	LD (HL),H	LD (HL),L	HALT	LD (HL),A	LD A,B	LD A,C	LD A,D	LD A,E	LD A,H	LD A,L	LD A,(HL)	LD A,A
1  8	1  8	1  8	1  8	1  8	1  8	1  4	1  8	1  4	1  4	1  4	1  4	1  4	1  4	1  8	1  4
- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -
ADD A,B	ADD A,C	ADD A,D	ADD A,E	ADD A,H	ADD A,L	ADD A,(HL)	ADD A,A	ADC A,B	ADC A,C	ADC A,D	ADC A,E	ADC A,H	ADC A,L	ADC A,(HL)	ADC A,A
1  4	1  4	1  4	1  4	1  4	1  4	1  8	1  4	1  4	1  4	1  4	1  4	1  4	1  4	1  8	1  4
Z 0 H C	Z 0 H C	Z 0 H C	Z 0 H C	Z 0 H C	Z 0 H C	Z 0 H C	Z 0 H C	Z 0 H C	Z 0 H C	Z 0 H C	Z 0 H C	Z 0 H C	Z 0 H C	Z 0 H C	Z 0 H C
SUB B	SUB C	SUB D	SUB E	SUB H	SUB L	SUB (HL)	SUB A	SBC A,B	SBC A,C	SBC A,D	SBC A,E	SBC A,H	SBC A,L	SBC A,(HL)	SBC A,A
1  4	1  4	1  4	1  4	1  4	1  4	1  8	1  4	1  4	1  4	1  4	1  4	1  4	1  4	1  8	1  4
Z 1 H C	Z 1 H C	Z 1 H C	Z 1 H C	Z 1 H C	Z 1 H C	Z 1 H C	Z 1 H C	Z 1 H C	Z 1 H C	Z 1 H C	Z 1 H C	Z 1 H C	Z 1 H C	Z 1 H C	Z 1 H C
AND B	AND C	AND D	AND E	AND H	AND L	AND (HL)	AND A	XOR B	XOR C	XOR D	XOR E	XOR H	XOR L	XOR (HL)	XOR A
1  4	1  4	1  4	1  4	1  4	1  4	1  8	1  4	1  4	1  4	1  4	1  4	1  4	1  4	1  8	1  4
Z 0 1 0	Z 0 1 0	Z 0 1 0	Z 0 1 0	Z 0 1 0	Z 0 1 0	Z 0 1 0	Z 0 1 0	Z 0 0 0	Z 0 0 0	Z 0 0 0	Z 0 0 0	Z 0 0 0	Z 0 0 0	Z 0 0 0	Z 0 0 0
OR B	OR C	OR D	OR E	OR H	OR L	OR (HL)	OR A	CP B	CP C	CP D	CP E	CP H	CP L	CP (HL)	CP A
1  4	1  4	1  4	1  4	1  4	1  4	1  8	1  4	1  4	1  4	1  4	1  4	1  4	1  4	1  8	1  4
Z 0 0 0	Z 0 0 0	Z 0 0 0	Z 0 0 0	Z 0 0 0	Z 0 0 0	Z 0 0 0	Z 0 0 0	Z 1 H C	Z 1 H C	Z 1 H C	Z 1 H C	Z 1 H C	Z 1 H C	Z 1 H C	Z 1 H C
RET NZ	POP BC	JP NZ,a16	JP a16	CALL NZ,a16	PUSH BC	ADD A,d8	RST 00H	RET Z	RET	JP Z,a16	PREFIX CB	CALL Z,a16	CALL a16	ADC A,d8	RST 08H
1  20/8	1  12	3  16/12	3  16	3  24/12	1  16	2  8	1  16	1  20/8	1  16	3  16/12	2  4	3  24/12	3  24	2  8	1  16
- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	Z 0 H C	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	Z 0 H C	- - - -
RET NC	POP DE	JP NC,a16		CALL NC,a16	PUSH DE	SUB d8	RST 10H	RET C	RETI	JP C,a16		CALL C,a16		SBC A,d8	RST 18H
1  20/8	1  12	3  16/12		3  24/12	1  16	2  8	1  16	1  20/8	1  16	3  16/12		3  24/12		2  8	1  16
- - - -	- - - -	- - - -		- - - -	- - - -	Z 1 H C	- - - -	- - - -	- - - -	- - - -		- - - -		Z 1 H C	- - - -
LDH (a8),A	POP HL	LD (C),A			PUSH HL	AND d8	RST 20H	ADD SP,r8	JP (HL)	LD (a16),A				XOR d8	RST 28H
2  12	1  12	1  8			1  16	2  8	1  16	2  16	1  4	3  16				2  8	1  16
- - - -	- - - -	- - - -			- - - -	Z 0 1 0	- - - -	0 0 H C	- - - -	- - - -				Z 0 0 0	- - - -
LDH A,(a8)	POP AF	LD A,(C)	DI		PUSH AF	OR d8	RST 30H	LD HL,SP+r8	LD SP,HL	LD A,(a16)	EI			CP d8	RST 38H
2  12	1  12	1  8	1  4		1  16	2  8	1  16	2  12	1  8	3  16	1  4			2  8	1  16
- - - -	Z N H C	- - - -	- - - -		- - - -	Z 0 0 0	- - - -	0 0 H C	- - - -	- - - -	- - - -			Z 1 H C	- - - -
"""

r2 = """
RLC B	RLC C	RLC D	RLC E	RLC H	RLC L	RLC (HL)	RLC A	RRC B	RRC C	RRC D	RRC E	RRC H	RRC L	RRC (HL)	RRC A
2  8	2  8	2  8	2  8	2  8	2  8	2  16	2  8	2  8	2  8	2  8	2  8	2  8	2  8	2  16	2  8
Z 0 0 C	Z 0 0 C	Z 0 0 C	Z 0 0 C	Z 0 0 C	Z 0 0 C	Z 0 0 C	Z 0 0 C	Z 0 0 C	Z 0 0 C	Z 0 0 C	Z 0 0 C	Z 0 0 C	Z 0 0 C	Z 0 0 C	Z 0 0 C
RL B	RL C	RL D	RL E	RL H	RL L	RL (HL)	RL A	RR B	RR C	RR D	RR E	RR H	RR L	RR (HL)	RR A
2  8	2  8	2  8	2  8	2  8	2  8	2  16	2  8	2  8	2  8	2  8	2  8	2  8	2  8	2  16	2  8
Z 0 0 C	Z 0 0 C	Z 0 0 C	Z 0 0 C	Z 0 0 C	Z 0 0 C	Z 0 0 C	Z 0 0 C	Z 0 0 C	Z 0 0 C	Z 0 0 C	Z 0 0 C	Z 0 0 C	Z 0 0 C	Z 0 0 C	Z 0 0 C
SLA B	SLA C	SLA D	SLA E	SLA H	SLA L	SLA (HL)	SLA A	SRA B	SRA C	SRA D	SRA E	SRA H	SRA L	SRA (HL)	SRA A
2  8	2  8	2  8	2  8	2  8	2  8	2  16	2  8	2  8	2  8	2  8	2  8	2  8	2  8	2  16	2  8
Z 0 0 C	Z 0 0 C	Z 0 0 C	Z 0 0 C	Z 0 0 C	Z 0 0 C	Z 0 0 C	Z 0 0 C	Z 0 0 0	Z 0 0 0	Z 0 0 0	Z 0 0 0	Z 0 0 0	Z 0 0 0	Z 0 0 0	Z 0 0 0
SWAP B	SWAP C	SWAP D	SWAP E	SWAP H	SWAP L	SWAP (HL)	SWAP A	SRL B	SRL C	SRL D	SRL E	SRL H	SRL L	SRL (HL)	SRL A
2  8	2  8	2  8	2  8	2  8	2  8	2  16	2  8	2  8	2  8	2  8	2  8	2  8	2  8	2  16	2  8
Z 0 0 0	Z 0 0 0	Z 0 0 0	Z 0 0 0	Z 0 0 0	Z 0 0 0	Z 0 0 0	Z 0 0 0	Z 0 0 C	Z 0 0 C	Z 0 0 C	Z 0 0 C	Z 0 0 C	Z 0 0 C	Z 0 0 C	Z 0 0 C
BIT 0,B	BIT 0,C	BIT 0,D	BIT 0,E	BIT 0,H	BIT 0,L	BIT 0,(HL)	BIT 0,A	BIT 1,B	BIT 1,C	BIT 1,D	BIT 1,E	BIT 1,H	BIT 1,L	BIT 1,(HL)	BIT 1,A
2  8	2  8	2  8	2  8	2  8	2  8	2  16	2  8	2  8	2  8	2  8	2  8	2  8	2  8	2  16	2  8
Z 0 1 -	Z 0 1 -	Z 0 1 -	Z 0 1 -	Z 0 1 -	Z 0 1 -	Z 0 1 -	Z 0 1 -	Z 0 1 -	Z 0 1 -	Z 0 1 -	Z 0 1 -	Z 0 1 -	Z 0 1 -	Z 0 1 -	Z 0 1 -
BIT 2,B	BIT 2,C	BIT 2,D	BIT 2,E	BIT 2,H	BIT 2,L	BIT 2,(HL)	BIT 2,A	BIT 3,B	BIT 3,C	BIT 3,D	BIT 3,E	BIT 3,H	BIT 3,L	BIT 3,(HL)	BIT 3,A
2  8	2  8	2  8	2  8	2  8	2  8	2  16	2  8	2  8	2  8	2  8	2  8	2  8	2  8	2  16	2  8
Z 0 1 -	Z 0 1 -	Z 0 1 -	Z 0 1 -	Z 0 1 -	Z 0 1 -	Z 0 1 -	Z 0 1 -	Z 0 1 -	Z 0 1 -	Z 0 1 -	Z 0 1 -	Z 0 1 -	Z 0 1 -	Z 0 1 -	Z 0 1 -
BIT 4,B	BIT 4,C	BIT 4,D	BIT 4,E	BIT 4,H	BIT 4,L	BIT 4,(HL)	BIT 4,A	BIT 5,B	BIT 5,C	BIT 5,D	BIT 5,E	BIT 5,H	BIT 5,L	BIT 5,(HL)	BIT 5,A
2  8	2  8	2  8	2  8	2  8	2  8	2  16	2  8	2  8	2  8	2  8	2  8	2  8	2  8	2  16	2  8
Z 0 1 -	Z 0 1 -	Z 0 1 -	Z 0 1 -	Z 0 1 -	Z 0 1 -	Z 0 1 -	Z 0 1 -	Z 0 1 -	Z 0 1 -	Z 0 1 -	Z 0 1 -	Z 0 1 -	Z 0 1 -	Z 0 1 -	Z 0 1 -
BIT 6,B	BIT 6,C	BIT 6,D	BIT 6,E	BIT 6,H	BIT 6,L	BIT 6,(HL)	BIT 6,A	BIT 7,B	BIT 7,C	BIT 7,D	BIT 7,E	BIT 7,H	BIT 7,L	BIT 7,(HL)	BIT 7,A
2  8	2  8	2  8	2  8	2  8	2  8	2  16	2  8	2  8	2  8	2  8	2  8	2  8	2  8	2  16	2  8
Z 0 1 -	Z 0 1 -	Z 0 1 -	Z 0 1 -	Z 0 1 -	Z 0 1 -	Z 0 1 -	Z 0 1 -	Z 0 1 -	Z 0 1 -	Z 0 1 -	Z 0 1 -	Z 0 1 -	Z 0 1 -	Z 0 1 -	Z 0 1 -
RES 0,B	RES 0,C	RES 0,D	RES 0,E	RES 0,H	RES 0,L	RES 0,(HL)	RES 0,A	RES 1,B	RES 1,C	RES 1,D	RES 1,E	RES 1,H	RES 1,L	RES 1,(HL)	RES 1,A
2  8	2  8	2  8	2  8	2  8	2  8	2  16	2  8	2  8	2  8	2  8	2  8	2  8	2  8	2  16	2  8
- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -
RES 2,B	RES 2,C	RES 2,D	RES 2,E	RES 2,H	RES 2,L	RES 2,(HL)	RES 2,A	RES 3,B	RES 3,C	RES 3,D	RES 3,E	RES 3,H	RES 3,L	RES 3,(HL)	RES 3,A
2  8	2  8	2  8	2  8	2  8	2  8	2  16	2  8	2  8	2  8	2  8	2  8	2  8	2  8	2  16	2  8
- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -
RES 4,B	RES 4,C	RES 4,D	RES 4,E	RES 4,H	RES 4,L	RES 4,(HL)	RES 4,A	RES 5,B	RES 5,C	RES 5,D	RES 5,E	RES 5,H	RES 5,L	RES 5,(HL)	RES 5,A
2  8	2  8	2  8	2  8	2  8	2  8	2  16	2  8	2  8	2  8	2  8	2  8	2  8	2  8	2  16	2  8
- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -
RES 6,B	RES 6,C	RES 6,D	RES 6,E	RES 6,H	RES 6,L	RES 6,(HL)	RES 6,A	RES 7,B	RES 7,C	RES 7,D	RES 7,E	RES 7,H	RES 7,L	RES 7,(HL)	RES 7,A
2  8	2  8	2  8	2  8	2  8	2  8	2  16	2  8	2  8	2  8	2  8	2  8	2  8	2  8	2  16	2  8
- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -
SET 0,B	SET 0,C	SET 0,D	SET 0,E	SET 0,H	SET 0,L	SET 0,(HL)	SET 0,A	SET 1,B	SET 1,C	SET 1,D	SET 1,E	SET 1,H	SET 1,L	SET 1,(HL)	SET 1,A
2  8	2  8	2  8	2  8	2  8	2  8	2  16	2  8	2  8	2  8	2  8	2  8	2  8	2  8	2  16	2  8
- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -
SET 2,B	SET 2,C	SET 2,D	SET 2,E	SET 2,H	SET 2,L	SET 2,(HL)	SET 2,A	SET 3,B	SET 3,C	SET 3,D	SET 3,E	SET 3,H	SET 3,L	SET 3,(HL)	SET 3,A
2  8	2  8	2  8	2  8	2  8	2  8	2  16	2  8	2  8	2  8	2  8	2  8	2  8	2  8	2  16	2  8
- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -
SET 4,B	SET 4,C	SET 4,D	SET 4,E	SET 4,H	SET 4,L	SET 4,(HL)	SET 4,A	SET 5,B	SET 5,C	SET 5,D	SET 5,E	SET 5,H	SET 5,L	SET 5,(HL)	SET 5,A
2  8	2  8	2  8	2  8	2  8	2  8	2  16	2  8	2  8	2  8	2  8	2  8	2  8	2  8	2  16	2  8
- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -
SET 6,B	SET 6,C	SET 6,D	SET 6,E	SET 6,H	SET 6,L	SET 6,(HL)	SET 6,A	SET 7,B	SET 7,C	SET 7,D	SET 7,E	SET 7,H	SET 7,L	SET 7,(HL)	SET 7,A
2  8	2  8	2  8	2  8	2  8	2  8	2  16	2  8	2  8	2  8	2  8	2  8	2  8	2  8	2  16	2  8
- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -	- - - -
"""

n1 = [i.split('\t') for i in r1.splitlines()[1:]]
n2 = [i.split('\t') for i in r2.splitlines()[1:]]
c = []

def parse_opcode(n, opc):
    k = (opc >> 4) * 3
    nm = n[k][opc & 0xf]
    if not nm:
        return "UNKN", 1, 4, ["-"] * 4
    sz, c = n[k + 1][opc & 0xf].split('\xa0\xa0')
    f = n[k + 2][opc & 0xf].split(' ')
    return nm, sz, c, f

class i:
    def __init__(s, n, opc):
        s.n, s.sz, s.c, s.f = parse_opcode(n, opc)
        s.opc = opc
        s.H = n is n2
        s.cd=""
        
    def __repr__(s):
        return f"<{s.opc:02X} = {s.n}\t | Fg: {' '.join(s.f)} | Sz: {s.sz} | Cc: {s.c}>"
    
    def x(s):
        if s.cd: return s.cd
        if s.n.startswith('UNKN'):
            return f"#define i{s.opc:02X} UnknownOpcode"
        
        h=f"\n{'INST_H' if s.H else 'INST'}({s.opc:02X}, {s.n.replace(' ', '_').replace('+', '_').replace(',', '_').upper()})"
        h+=f' {{\n'
        
        if s.n.startswith('LD'):
            a,b=s.n.split(" ",1)[-1].replace('a16', 'OPERAND').replace('d16', 'OPERAND').replace('d8', 'O1').replace('a8', 'O1').replace('+', '++').replace('-', '--').split(',')
            if '(' in a:
                a = a[1:-1]
                if a in ['O1', 'O2'] or len(a) == 1: a = "0xFF00 | " + a 
                h += f"\tcpu_write({a}, {b});\n"
                if b in ("SP",):
                    h += f"\tcpu_write({a} + 1, {b} >> 8);\n"
            elif '(' in b:
                b = b[1:-1]
                if b  in ['O1', 'O2'] or len(b) == 1: b = "0xFF00 | " + b 
                h += f"\t{a} = cpu_read({b});\n"
            elif "r8" in b:
                h+=f"\tADD_SP_d(O1, {a});\n"
            else:
                h+=f"\t{a} = {b};\n"
                if b == "HL":
                    h+=f"\tsync(4);\n"
            
        elif s.n.startswith('INC'):
            a=s.n.split(" ",1)[-1]
            if '(' in a:
                a = a[1:-1]
                h += f"\tu8 r = cpu_read({a});\n"
                h += f"\tINC(r);\n"
                h += f"\tcpu_write({a}, r);\n"
            elif len(a) > 1:
                h += f"\t{a}++;\n"
                h += f"\tsync(4);\n"
            else:
                h += f"\tINC({a});\n"
            
        elif s.n.startswith('DEC'):
            a=s.n.split(" ",1)[-1]
            if '(' in a:
                a = a[1:-1]
                h += f"\tu8 r = cpu_read({a});\n"
                h += f"\tDEC(r);\n"
                h += f"\tcpu_write({a}, r);\n"
            elif len(a) > 1:
                h += f"\t{a}--;\n"
                h += f"\tsync(4);\n"
            else:
                h += f"\tDEC({a});\n"
            
        elif s.n.startswith('AND'):
            a=s.n.split(" ",1)[-1].replace('d8', 'O1')
            if '(' in a:
                a = a[1:-1]
                h += f"\tAND(cpu_read({a}));\n"
            else:
                h += f"\tAND({a});\n"
            
        elif s.n.startswith('OR'):
            a=s.n.split(" ",1)[-1].replace('d8', 'O1')
            if '(' in a:
                a = a[1:-1]
                h += f"\tOR(cpu_read({a}));\n"
            else:
                h += f"\tOR({a});\n"
            
        elif s.n.startswith('XOR'):
            a=s.n.split(" ",1)[-1].replace('d8', 'O1')
            if '(' in a:
                a = a[1:-1]
                h += f"\tXOR(cpu_read({a}));\n"
            else:
                h += f"\tXOR({a});\n"
            
        elif s.n.startswith('CP '):
            a=s.n.split(" ",1)[-1].replace('d8', 'O1')
            if '(' in a:
                a = a[1:-1]
                h += f"\tu8 r = cpu_read({a});\n"
                h += f"\tCP(r);\n"
            else:
                h += f"\tCP({a});\n"
            
        elif s.n.startswith('ADD A'):
            a=s.n.split(" ",1)[-1].split(",",1)[-1].replace('d8', 'O1')
            if '(' in a:
                a = a[1:-1]
                h += f"\tu8 r = cpu_read({a});\n"
                h += f"\tADD(r);\n"
            else:
                h += f"\tADD({a});\n"
            
        elif s.n.startswith('ADC'):
            a=s.n.split(" ",1)[-1].split(",",1)[-1].replace('d8', 'O1')
            if '(' in a:
                a = a[1:-1]
                h += f"\tu8 r = cpu_read({a});\n"
                h += f"\tADC(r);\n"
            else:
                h += f"\tADC({a});\n"
            
        elif s.n.startswith('SUB'):
            a=s.n.split(" ",1)[-1].split(",",1)[-1].replace('d8', 'O1')
            if '(' in a:
                a = a[1:-1]
                h += f"\tu8 r = cpu_read({a});\n"
                h += f"\tSUB(r);\n"
            else:
                h += f"\tSUB({a});\n"
            
        elif s.n.startswith('SBC'):
            a=s.n.split(" ",1)[-1].split(",",1)[-1].replace('d8', 'O1')
            if '(' in a:
                a = a[1:-1]
                h += f"\tu8 r = cpu_read({a});\n"
                h += f"\tSBC(r);\n"
            else:
                h += f"\tSBC({a});\n"
            
        elif s.n.startswith('ADD SP'):
            a=s.n.split(" ",1)[-1].split(",",1)[-1].replace('r8', 'O1')
            if '(' in a:
                a = a[1:-1]
                h += f"\tu8 r = cpu_read({a});\n"
                h += f"\tADD_SP(r);\n"
            else:
                h += f"\tADD_SP({a});\n"
            
        elif s.n.startswith('ADD HL'):
            a=s.n.split(" ",1)[-1].split(",",1)[-1].replace('r8', 'O1')
            if '(' in a:
                a = a[1:-1]
                h += f"\tu8 r = cpu_read({a});\n"
                h += f"\tADD_HL(r);\n"
            else:
                h += f"\tADD_HL({a});\n"
            
        elif s.n.startswith('RST'):
            a=s.n.split(" ",1)[-1].replace('H', '')
            h += f"\tPUSH16(PC);\n"
            h += f"\tPC = 0x{a};\n"
            h += f"\tsync(4);\n"
            
        elif s.n.startswith('POP'):
            a=s.n.split(" ",1)[-1]
            h += f"\t{a} = POP16();\n"
            if a == 'AF':
                h += f"\tFx = 0;\n"
            
        elif s.n.startswith('PUSH'):
            a=s.n.split(" ",1)[-1]
            h += f"\tPUSH16({a});\n"
            h += f"\tsync(4);\n"
            
        elif s.n.startswith('POP'):
            a=s.n.split(" ",1)[-1]
            h += f"\t{a} = POP16();\n"
            if a == 'AF':
                h += f"\tFx = 0;\n"
            
        elif s.n.startswith('RET'):
            a=s.n.split(" ",1)[-1]
            match len(a):
                case 1:
                    h += f"\tsync(4);\n"
                    h += f"\tif ({a.lower()}) {{\n"
                    h += f"\t\tPC = POP16();\n"
                    h += f"\t\tsync(4);\n"
                    h += f"\t\t return 20;\n"
                    h += f"\t}}\n"
                case 2:
                    h += f"\tsync(4);\n"
                    h += f"\tif ({a.lower().replace('n', '!')}) {{\n"
                    h += f"\t\tPC = POP16();\n"
                    h += f"\t\tsync(4);\n"
                    h += f"\t\t return 20;\n"
                    h += f"\t}}\n"
                case 4: #RETI
                    h += f"\tPC = POP16();\n"
                    h += f"\tsync(4);\n"
                    h += f"\tIME = 1;\n"
                case x:
                    h += f"\tPC = POP16();\n"
                    h += f"\tsync(4);\n"
            
        elif s.n.startswith('EI'):
            h += f"\tIME_DELAY |= 2;\n"
            
        elif s.n.startswith('DI'):
            h += f"\tIME = 0;\n"
            
        elif s.n.startswith('JP'):
            a=s.n.split(" ",1)[-1].replace("a16", "OPERAND").replace("(HL)", "HL").split(",",1)
            match len(a):
                case 1:
                    h += f"\tPC = {a[0]};\n"
                    if "OP" in a[0]: h += f"\tsync(4);\n"
                case 2:
                    h += f"\tif ({a[0].lower().replace('n', '!')}) {{\n"
                    h += f"\t\tPC = {a[1]};\n"
                    h += f"\t\tsync(4);\n"
                    h += f"\t\t return 16;\n"
                    h += f"\t}}\n"
            
        elif s.n.startswith('JR'):
            a=s.n.split(" ",1)[-1].replace("r8", "O1").split(",",1)
            match len(a):
                case 1:
                    h += f"\tPC += (s8){a[0]};\n"
                    h += f"\tsync(4);\n"
                case 2:
                    h += f"\tif ({a[0].lower().replace('n', '!')}) {{\n"
                    h += f"\t\tPC += (s8){a[1]};\n"
                    h += f"\t\tsync(4);\n"
                    h += f"\t\treturn 12;\n"
                    h += f"\t}}\n"
            
        elif s.n.startswith('CALL'):
            a=s.n.split(" ",1)[-1].replace("a16", "OPERAND").replace("(HL)", "HL").split(",",1)
            match len(a):
                case 1:
                    h += f"\tPUSH16(PC);\n"
                    h += f"\tPC = {a[0]};\n"
                    h += f"\tsync(4);\n"
                case 2:
                    h += f"\tif ({a[0].lower().replace('n', '!')}) {{\n"
                    h += f"\t\tPUSH16(PC);\n"
                    h += f"\t\tPC = {a[1]};\n"
                    h += f"\tsync(4);\n"
                    h += f"\t\treturn 24;\n"
                    h += f"\t}}\n"

        
        elif s.n.startswith('NOP'):
            pass
        
        elif s.n.startswith('DAA'):
            h += f"\ts16 daa = A;\n"
            h += f"\tif (n) {{\n"
            h += f"\t\tif (h) daa = (s16)((daa - 0x06) & 0xff);\n"
            h += f"\t\tif (c) daa -= 0x60;\n"
            h += f"\t}} else {{\n"
            h += f"\t\tif (h || (daa & 0xf) > 9) daa += 0x06;\n"
            h += f"\t\tif (c || daa > 0x9f) daa += 0x60;\n"
            h += f"\t}}\n\n"
            h += f"\tA = daa, h = 0, z = !A, c = (daa & 0x100) == 0x100;\n"
        
        elif s.n.startswith('RLCA'):
            h += f"\tRLCA();\n"
        
        elif s.n.startswith('RLA'):
            h += f"\tRLA();\n"
        
        elif s.n.startswith('RRCA'):
            h += f"\tRRCA();\n"
        
        elif s.n.startswith('RRA'):
            h += f"\tRRA();\n"
        
        elif s.n.startswith('SCF'):
            h += f"\tSCF();\n"
        
        elif s.n.startswith('CCF'):
            h += f"\tCCF();\n"
        
        elif s.n.startswith('CPL'):
            h += f"\tCPL();\n"

        elif s.n.startswith('HALT'):
            h += "\tif (halted) goto dec;\n\n"
            h += "\tif (!IME && ioIF & read_ie()) {\n"
            h += "\t\tif (GBC) {}\n"
            h += "\t\telse halt_bug = 1;\n"
            h += "\t} else {\n"
            h += "\t\thalted = 1;\n"
            h += "\t\tdec:\n"
            h += "\t\tPC--; // emulate halting by reexecuting halt\n}\n"

        elif s.n.startswith("STOP"):
            h += r"""
	if ((~ioJOYP) & 0x0F){ // Btn selected
		if (ioIF & read_ie()) {
		
		} else {
			PC++;
			ERROR("STOP Instruction - HALT Mode entrance", "unimplemented\n");
			halted = 1;
		}
	} else {
		if ((ioKEY1 & 1) && GBC) {
			if (ioIF & read_ie()) {
				if (IME) {
					ERROR("STOP Instruction error", "non-deterministic crash\n");
					// glitch
				} else {
					ch_speed:
					double_speed = !double_speed;
					write_io(KEY1, 0);
					// TODO: Reset DIV
				}
			} else {
				PC++;
				ERROR("STOP Instruction - Special HALT Mode entrance", "unimplemented\n");
				// halted = 1; ???
				goto ch_speed;
			}
		} else {
			if (!(ioIF & read_ie())) PC++;
			ERROR("STOP Instruction - STOP Mode entrance", "unimplemented\n");
			// STOP Mode
			// TODO: Reset DIV
		}
	}

	"""

        ## H-opcode
        elif s.n.startswith('PREFIX'):
            h += f"\treturn HighInstructions[O1]();\n"
            h += f'}} // {repr(s)[1:-1]}'
            s.cd=h
            return h

        elif s.n.startswith('RLC'):
            a = s.n.split(' ')[-1]
            if '(' in a:
                h += f"\tu8 r = cpu_read(HL);\n"
                h += f"\tRLC(r);\n"
                h += f"\tcpu_write(HL, r);\n"
            else: h += f"\tRLC({a});\n"

        elif s.n.startswith('RRC'):
            a = s.n.split(' ')[-1]
            if '(' in a:
                h += f"\tu8 r = cpu_read(HL);\n"
                h += f"\tRRC(r);\n"
                h += f"\tcpu_write(HL, r);\n"
            else: h += f"\tRRC({a});\n"

        elif s.n.startswith('RL '):
            a = s.n.split(' ')[-1]
            if '(' in a:
                h += f"\tu8 r = cpu_read(HL);\n"
                h += f"\tRL(r);\n"
                h += f"\tcpu_write(HL, r);\n"
            else: h += f"\tRL({a});\n"

        elif s.n.startswith('RR '):
            a = s.n.split(' ')[-1]
            if '(' in a:
                h += f"\tu8 r = cpu_read(HL);\n"
                h += f"\tRR(r);\n"
                h += f"\tcpu_write(HL, r);\n"
            else: h += f"\tRR({a});\n"

        elif s.n.startswith('SLA'):
            a = s.n.split(' ')[-1]
            if '(' in a:
                h += f"\tu8 r = cpu_read(HL);\n"
                h += f"\tSLA(r);\n"
                h += f"\tcpu_write(HL, r);\n"
            else: h += f"\tSLA({a});\n"

        elif s.n.startswith('SRA'):
            a = s.n.split(' ')[-1]
            if '(' in a:
                h += f"\tu8 r = cpu_read(HL);\n"
                h += f"\tSRA(r);\n"
                h += f"\tcpu_write(HL, r);\n"
            else: h += f"\tSRA({a});\n"

        elif s.n.startswith('SWAP'):
            a = s.n.split(' ')[-1]
            if '(' in a:
                h += f"\tu8 r = cpu_read(HL);\n"
                h += f"\tSWAP(r);\n"
                h += f"\tcpu_write(HL, r);\n"
            else: h += f"\tSWAP({a});\n"

        elif s.n.startswith('SRL'):
            a = s.n.split(' ')[-1]
            if '(' in a:
                h += f"\tu8 r = cpu_read(HL);\n"
                h += f"\tSRL(r);\n"
                h += f"\tcpu_write(HL, r);\n"
            else: h += f"\tSRL({a});\n"

        elif s.n.startswith('BIT'):
            b, a = s.n.split(' ')[-1].split(',')
            if '(' in a:
                h += f"\tu8 r = cpu_read(HL);\n"
                h += f"\tBIT({b}, r);\n"
                h += f"\tcpu_write(HL, r);\n"
            else: h += f"\tBIT({b}, {a});\n"

        elif s.n.startswith('RES'):
            b, a = s.n.split(' ')[-1].split(',')
            if '(' in a:
                h += f"\tu8 r = cpu_read(HL);\n"
                h += f"\tRES({b}, r);\n"
                h += f"\tcpu_write(HL, r);\n"
            else: h += f"\tRES({b}, {a});\n"

        elif s.n.startswith('SET'):
            b, a = s.n.split(' ')[-1].split(',')
            if '(' in a:
                h += f"\tu8 r = cpu_read(HL);\n"
                h += f"\tSET({b}, r);\n"
                h += f"\tcpu_write(HL, r);\n"
            else: h += f"\tSET({b}, {a});\n"

        ## Undef
        
        else:
            h += "\t// TODO: Inst\n"
            ff=""
            pf=""
            z, n, h_, c = s.f
            for l, v in [("z",z),("n",n),("h",h_),("c",c)]:
                if v.lower()==l and not pf: pf="//TODO: "
                if v!='-':ff+=f"{l} = {v.lower()}, "
            if ff:ff=ff[:-2]+";\n"
            h+=pf+ff
        
        #if '/' in s.c: h+="\t//TODO: "
        h += f"\treturn {s.c.split('/')[-1]};\n"
        
        h += f'}}  // {repr(s)[1:-1]}'
        s.cd=h
        return h
    def cy(s):
        r=[s.c]
        a=list(map(int, s.c.split('/')))
        a=list(map(lambda x: x - int(s.sz) * 4, a))
        r.append('/'.join(map(str, a)))
        a=list(map(lambda x: x - (s.cd.count("cpu_write(") + s.cd.count("cpu_read(") + s.cd.count("sync("))*4, a))
        r.append('/'.join(map(str, a)))
        #a=list(map(lambda x: x - (s.cd.count("ADD(") + s.cd.count("ADC(") + s.cd.count("SUB(") + s.cd.count("SBC("))*4, a))
        a=list(map(lambda x: x - (s.cd.count("PUSH8(") + s.cd.count("POP8("))*4, a))
        a=list(map(lambda x: x - (s.cd.count("POP16(") + s.cd.count("PUSH16(") + s.cd.count("ADD_SP("))*8, a))
        a=list(map(lambda x: x - (s.cd.count("ADD_HL(") + s.cd.count("ADD_SP_d("))*4, a))
        r.append('/'.join(map(str, a)))
        return r[-1][0]=='0', r

    
for u in range(256):
    c.append(i(n1, u))
    
for u in range(256):
    c.append(i(n2, u))

print("#pragma region Low Instructions")

for u in sorted(c[:256], key=lambda x: x.n.split(' ')[0]):
    print(u.x())

print("\n#pragma endregion\n\n#pragma region High Instructions")

for u in sorted(c[256:], key=lambda x: x.n.split(' ')[0]):
    print(u.x())

print("\n#pragma endregion\n\n#pragma region GBZ80 Instruction Set")

print("static Instruction * Instructions[] = {\n\t", end='')
for u in range(256):
    if u & 15 == 0 and u: print(f"// {(u-1)&0xf0:02X}\n\t", end='')
    print(f"i{u:02X}, ", end='')
print('// F0\n};\n')


print("static Instruction * HighInstructions[] = {\n\t", end='')
for u in range(256):
    if u & 15 == 0 and u: print(f"// {(u-1)&0xf0:02X}\n\t", end='')
    print(f"iCB{u:02X}, ", end='')
print('// F0\n};\n')


print("static u8 OPSize[] = {\n\t", end='')
for u in range(256):
    if u & 15 == 0 and u: print(f"// {(u-1)&0xf0:02X}\n\t", end='')
    print(f"{int(c[u].sz) - 1}, ", end='')
print('// F0\n};\n')



print("const char * const OPName[] = {\n\t", end='')
for u in range(512):
    if u & 15 == 0 and u: print(f"// {(u-1)&0xf0:02X}\n\t", end='')
    print(f"\"{c[u].n:{max(len(i.n) for i in c)}}\", ", end='')
print('// F0\n};\n')

print("\n#pragma endregion\n")

for u in range(512):
    if c[u].n!="UNKN":
        v,r=c[u].cy()
        if not v:print(f"{v}\t{c[u].opc:02X} : {c[u].n}\t: {int(c[u].c.split('/')[-1])-int(c[u].sz)*4}:", *r)





















