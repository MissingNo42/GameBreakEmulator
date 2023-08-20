//
// Created by Romain on 18/07/2023.
//

////////////////////////  Includes  ///////////////////////////

#include "utils.h"
#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include "units/cpu.h"



////////////////////////   Methods   //////////////////////////

void Log(ELOG type, char lock, const char * title, const char * str, ...){
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	printf("\x1b[38;5;%dm%d-%02d-%02d %02d:%02d:%02d ($%04X %s [%02X] %04X): %s\x1b[0m\t",
		   type, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,
		   PCX, OPName[OPCODE], OPCODE, OPERAND, title);
	va_list l;
	va_start(l, str);
	if (str) vprintf(str, l);
	fflush(stdout);
	va_end(l);
	
	if (lock) Lock();
}

void Lock(){
	(void)getchar();
}
