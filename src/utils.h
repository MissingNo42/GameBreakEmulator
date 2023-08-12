//
// Created by Romain on 18/07/2023.
//

#ifndef GOBOUEMU_UTILS_H
#define GOBOUEMU_UTILS_H

typedef enum {
	Debug    = 32,
	Info     = 112,
	Error    = 208,
	Critical = 196
} ELOG;

void Log(ELOG type, char * title, char * str, ...);

#define DEBUG(...) Log(Debug, __VA_ARGS__)
#define INFO(...) Log(Info, __VA_ARGS__)
#define ERROR(...) Log(Error, __VA_ARGS__)
#define CRITICAL(...) Log(Critical, __VA_ARGS__)

#endif //GOBOUEMU_UTILS_H
