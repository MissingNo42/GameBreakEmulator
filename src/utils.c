//
// Created by Romain on 18/07/2023.
//

#include "utils.h"
#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include "units/cpu.h"

void Log(ELOG type, char * title, char * str, ...){
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	printf("\x1b[38;5;%dm%d-%02d-%02d %02d:%02d:%02d ($%04X %s [%02X] %04X): %s\x1b[0m\t",
		   type, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,
		   PC, OPName[OPCODE], OPCODE, OPERAND, title);
	va_list l;
	va_start(l, str);
	vprintf(str, l);
	va_end(l);
}
