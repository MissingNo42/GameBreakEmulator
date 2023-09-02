//
// Created by Romain on 29/08/2023.
//

#ifndef GBEMU_BIOS_H
#define GBEMU_BIOS_H

#include "../types.h"
#include "../utils.h"

Struct { u8       // select the console for (1 = GBC | 0 = DMG):
	dmg_game: 1,  // DMG Only games
	gbc_game: 1,  // GBC Only games
	both_game: 1; // Games supporting both consoles
} Configuration;


extern u8 GBC;      // run on CGB or not (DMG or CGB BIOS)
extern u8 DMG_MODE; // run on DMG or DMG compatibility mode

extern Configuration configuration;

void select_bios();
void set_compatibility_mode();
s32 load_bootrom();

Reset(bios) {
	if (hard) {
		// Uninitialized
	} else select_bios();
}

SaveSize(bios, sizeof (GBC) + sizeof (DMG_MODE) )

Save(bios) {
	save_obj(GBC);
	save_obj(DMG_MODE);
}

Load(bios) {
	load_obj(GBC);
	load_obj(DMG_MODE);
}

#endif //GBEMU_BIOS_H
