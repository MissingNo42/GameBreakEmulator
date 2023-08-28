//
// Created by Romain on 18/07/2023.
//

#ifndef GBEMU_UTILS_H
#define GBEMU_UTILS_H


////////////////////////   Macros   ///////////////////////////

#define Reset(name) inline static void Reset_##name(u8 hard)
#define SaveSize(name, size) inline u16 SaveSize_##name() { return size; }
#define Save(name) inline void Save_##name()
#define Load(name) inline void Load_##name()
#define save_obj(obj) // sizeof(obj) | &obj
#define load_obj(obj)

#define SEGFAULT() *(char*)0 = 1;


////////////////////////    Types   ///////////////////////////

typedef enum {
	Debug    = 32,
	Info     = 112,
	Error    = 208,
	Critical = 196
} ELOG;


////////////////////////   Methods   //////////////////////////

void Log(ELOG type, char lock, const char * title, const char * str, ...);
void LogInst();
void Lock();

#define DEBUG(...) Log(Debug, 0, __VA_ARGS__)
#define INFO(...) Log(Info, 0, __VA_ARGS__)
#define ERROR(...) Log(Error, 0, __VA_ARGS__)
#define CRITICAL(...) Log(Critical, 1, __VA_ARGS__)

#endif //GBEMU_UTILS_H
