//
// Created by Romain on 29/08/2023.
//

#include <stdio.h>
#include <dirent.h>
#include "bios.h"
#include "mmu.h"

u8 GBC;      // run on CGB or not (DMG or CGB BIOS)
u8 DMG_MODE; // run on DMG or DMG compatibility mode

Configuration configuration = {.dmg_game = 1, .gbc_game = 1, .both_game = 1};


void select_bios() {
	if (cartridgeHeader.gbc_flag & 0x80) {
		if (cartridgeHeader.gbc_flag & 0x0C) CRITICAL("PGB Mode not implemented", "may lead to some issues\n");
		GBC = (cartridgeHeader.gbc_flag & 0x40) ? configuration.gbc_game : configuration.both_game;
	} else GBC = configuration.dmg_game;
	
	DMG_MODE = !GBC; // Fallback to DMG Comp. Mode done later
}

void set_compatibility_mode() {
	if (GBC && !(cartridgeHeader.gbc_flag & 0x80)) DMG_MODE = 1;
}

s32 load_bootrom() {
	char bootrom[20];
	char * bootrom_dir = "../bootrom";
	DIR* dir;
	
	do {
		dir = opendir(bootrom_dir);
		
		if (dir != NULL) {
			closedir(dir);
			goto sbreak;
		}
		
		bootrom_dir++; // switch from parent to current dir
	} while (bootrom_dir[0] == '.');
	
	CRITICAL("Folder bootrom/ not found", "require bootrom/ including bootroms inside: dmg.bin and cgb.bin\n");
	return 0;
	
	sbreak:;
	
	snprintf(bootrom, 20, "%s/%s.bin", bootrom_dir, (GBC) ? "cgb" : "dmg");
	
	FILE * f = fopen(bootrom, "r");
	
	if (f) {
		INFO("Mounting the boot rom", "at \"%s\" ...\n", bootrom);
		if (!fread(Memory, (GBC) ? 0x900: 0x100, 1, f)) {
			CRITICAL("Failed to load the boot rom", "empty or incomplete bootrom file\n");
			fclose(f);
			return 0;
		}
		fclose(f);
	} else {
		CRITICAL("Failed to load the boot rom", "file \"%s\" not found\n", bootrom);
		return 0;
	}
	
	INFO("Mounting the cartridge header", "\n");
	for (u8 i = 0; i < 0x50; i++) Memory[0x100 | i] = cartridgeHeader.raw[i];
	return 1;
}
