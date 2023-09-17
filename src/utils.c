//
// Created by Romain on 18/07/2023.
//

////////////////////////  Includes  ///////////////////////////

#include "utils.h"
#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include "units/cpu.h"
#include "units/mappers/mapper.h"
#include "io_ports.h"
#include "units/ppu.h"
#include "units/dma.h"
#include "units/ctrl.h"
#include "timer.h"



////////////////////////   Methods   //////////////////////////


void ResetEmulator(char hard) {
	INFO("Reset Emulator", "hard reset = %hhu\n", hard);
	Reset_io_ports(hard);
	Reset_cpu(hard);
	Reset_ppu(hard);
	Reset_dma(hard);
	Reset_ctrl(hard);
	Reset_timer(hard);
	
	Reset_cartridge(hard);
	Reset_bios(hard);
	Reset_mmu(hard);
	Reset_mapper(hard);
}


void SaveState(){
	FILE * fh = fopen("savestate.bin", "w");
	INFO("Save State", "to %s\n", "savestate.bin");
	
	if (fh) {
		Save_io_ports(fh);
		Save_cpu(fh);
		Save_ppu(fh);
		Save_dma(fh);
		Save_ctrl(fh);
		Save_timer(fh);
		Save_cartridge(fh);
		Save_bios(fh);
		Save_mmu(fh);
		Save_mapper(fh);
		fclose(fh);
	} else
		ERROR("Save State", "cannot create the save file\n");
}


void LoadState(){
	FILE * fh = fopen("savestate.bin", "r");
	INFO("Load State", "from %s\n", "savestate.bin");
	
	if (fh) {
		Load_io_ports(fh);
		Load_cpu(fh);
		Load_ppu(fh);
		Load_dma(fh);
		Load_ctrl(fh);
		Load_timer(fh);
		Load_cartridge(fh);
		Load_bios(fh);
		Load_mmu(fh);
		Load_mapper(fh);
		fclose(fh);
	} else
		ERROR("Load State", "cannot open the save file\n");
}


void save_state(void * fh, void * buf, u32 size) {
	fwrite(buf, size, 1, fh);
}


void load_state(void * fh, void * buf, u32 size) {
	if (!fread(buf, size, 1, fh)) CRITICAL("Load State - Incomplete Data", "dst = %p of size = %u\n", buf, size);
}


void Log(ELOG type, char lock, const char * title, const char * str, ...){
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	printf("\x1b[38;5;%dm%d-%02d-%02d %02d:%02d:%02d ($%04X %s [%02X] %04X): %s\x1b[0m\t",
		   type, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,
		   PCX,
		   //OPCODE == 0xCB ? OPName[0x100 | O1]: OPName[OPCODE], OPCODE, OPERAND,
		   "UNKN0", 0, 0,
		   title);
	va_list l;
	va_start(l, str);
	if (str) vprintf(str, l);
	fflush(stdout);
	va_end(l);
	
	if (lock) Lock();
}
FILE * f = NULL;
void LogInst() { // Mesen2 [PC,h] [A,2h] [B,2h][C,2h] [D,2h][E,2h] [PS,4] [H,2h][L,2h] [SP,4h]
	static u32 V = 0;
	if (!f) f = fopen("gbn.txt", "w");
	
	//fprintf(f, "%02X %02X %04X %04X %c%c%c%c %04X %04X (%04X %s [%02X %02X] %d %d <%d %d: %02X(%d) %02X>)\n",
	//		PC, A, BC, DE, (z) ? 'Z' : 'z', (n) ? 'N' : 'n', (h) ? 'H' : 'h', (c) ? 'C' : 'c', HL, SP, PCX, OPCODE == 0xCB ? OPName[0x100 | O1]: OPName[OPCODE],
	//		OPCODE, OPERAND, mapper.data.mbc5.rom_bank, mapper.data.mbc5.ram_bank, ioLY, ppu_mem.dots, ioSTAT, PPU_MODE, ioLCDC);
	//V++;
	//if (V == 20000) {
	//	V = 0;
	//	fflush(f);
	//}
}

void Lock(){
	static u32 lock = 0;
	
	if (lock) lock--;
	else {
		int x = getchar();
		switch (x) {
			case '\n': break;
			case 'w': {
				fclose(f);
				f = fopen("gbn.txt", "a");
				break;
			}
			case 'u': {
				ppu_mem.frame_ready = 1;
				break;
			}
			case 'm': {
				INFO("rom0:", " %p\n", memoryMap.rom0);
				INFO("rom1:", " %p\n", memoryMap.rom1);
				INFO("xram:", " %p\n", memoryMap.xram);
				INFO("vram:", " %p\n", memoryMap.vram);
				INFO("wram0:", " %p\n", memoryMap.wram0);
				INFO("wram1:", " %p\n", memoryMap.wram1);
				INFO("oam:", " %p\n", memoryMap.oam);
				INFO("io:", " %p\n", memoryMap.io);
				INFO("hram:", " %p\nenter:", memoryMap.hram);
				break;
			}
			default: scanf("%u%*c", &lock);
		}
	}
}
