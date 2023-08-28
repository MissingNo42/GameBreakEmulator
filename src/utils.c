//
// Created by Romain on 18/07/2023.
//

////////////////////////  Includes  ///////////////////////////

#include "utils.h"
#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include "units/cpu.h"
#include "mapper.h"
#include "io_ports.h"
#include "units/ppu.h"



////////////////////////   Methods   //////////////////////////

void Log(ELOG type, char lock, const char * title, const char * str, ...){
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	printf("\x1b[38;5;%dm%d-%02d-%02d %02d:%02d:%02d ($%04X %s [%02X] %04X): %s\x1b[0m\t",
		   type, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,
		   PCX, OPCODE == 0xCB ? OPName[0x100 | O1]: OPName[OPCODE], OPCODE, OPERAND, title);
	va_list l;
	va_start(l, str);
	if (str) vprintf(str, l);
	fflush(stdout);
	va_end(l);
	
	if (lock) Lock();
}

void LogInst() { // Mesen2 [PC,h] [A,2h] [B,2h][C,2h] [D,2h][E,2h] [PS,4] [H,2h][L,2h] [SP,4h]
	static u8 V = 0;
	static FILE * f = NULL;
	if (!f) f = fopen("gbn.txt", "w");
	
	fprintf(f, "%02X %02X %04X %04X %c%c%c%c %04X %04X (%04X %s [%02X %02X] %d %d <%d %d>)\n",
			PC, A, BC, DE, (z) ? 'Z' : 'z', (n) ? 'N' : 'n', (h) ? 'H' : 'h', (c) ? 'C' : 'c', HL, SP, PCX, OPCODE == 0xCB ? OPName[0x100 | O1]: OPName[OPCODE],
			OPCODE, OPERAND, mapper.data.mbc5.rom_bank, mapper.data.mbc5.ram_bank, ioLY, ppu_mem.dots);
	V++;
	if ((V&0x3f) == 0x20) {
		fflush(f);
	}
}

void Lock(){
	static u32 lock = 0;
	if (lock) lock--;
	else if (getchar() != '\n') scanf("%u%*c", &lock);
}
