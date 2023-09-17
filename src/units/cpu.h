//
// Created by Romain on 21/07/2023.
//

#ifndef GBEMU_CPU_H
#define GBEMU_CPU_H

////////////////////////  Includes  ///////////////////////////

#include "../types.h"
#include "../utils.h"


//////////////////////  Declarations  /////////////////////////
extern u32 double_speed;
extern const char * const OPName[];
extern u16 PCX; // debug opc's PC

////////////////////////   Macros   ///////////////////////////


////////////////////////   Methods   //////////////////////////

void cpu_init();
void cpu_run();
void cpu_reset();
u32 cpu_savesize();
void cpu_save(void * fh);
void cpu_load(void * fh);


/////////////////////  Registrations  /////////////////////////

Reset(cpu) { cpu_reset(); }

SaveSize(cpu, cpu_savesize())

Save(cpu) {	cpu_save(fh); }

Load(cpu) {	cpu_load(fh); }

#endif //GBEMU_CPU_H
