//
// Created by Romain on 19/07/2023.
//

#ifndef GOBOUEMU_CARTRIDGE_H
#define GOBOUEMU_CARTRIDGE_H

////////////////////////  Includes  ///////////////////////////

#include "utils.h"
#include "types.h"


////////////////////////    Types   ///////////////////////////

/**
 * @brief raw header from GB(C) cartridge
 * */
Union {
	u8 raw[0x50];
	
	struct {
		u8 entry_point[4];
		u8 logo[48];
		
		union {
			u8 title_all[16];
			struct {
				u8 title[11];
				u8 mcode[4];
				u8 gbc_flag;
			};
		};
		
		u8 ncode[2];
		u8 sgb_flag;
		u8 type;
		u8 rom_size;
		u8 ram_size;
		u8 region;
		u8 ocode;
		u8 version;
		u8 hchecksum;
		u16 checksum;
	};
} CartridgeHeader;

/**
 * @brief cartridge capability
 * */
Struct {
	u8 mapper;
	u8 has_ram: 1; // implies mapper != 0
	u8 has_battery: 1;
	u8 has_timer: 1;
	u8 has_rumble: 1;
	u8 has_sensor: 1;
} CartridgeCapability;

/**
 * @brief decoded useful info from the header
 * */
Struct {
	CartridgeCapability type;
	u8 rom_bank;
	u8 ram_bank;
	u32 rom_size;
	u32 ram_size;
} CartridgeInfo;


//////////////////////  Declarations  /////////////////////////

extern CartridgeHeader cartridgeHeader;
extern CartridgeInfo cartridgeInfo;
extern u8 GBC;

extern const char * const OCODE[];
extern const char * const NCODE[];
extern const CartridgeCapability CartridgeCapabilities[];


////////////////////////   Methods   //////////////////////////

u8 open_cartridge(const char * const fn);
u8 load_header();
u8 load_cartridge();


/////////////////////  Registrations  /////////////////////////

Reset(cartridge) {
	if (hard) {
		for (u16 i = 0; i < (u16)sizeof(cartridgeHeader); i++) ((u8*)&cartridgeHeader)[i] = 0x00;
		for (u16 i = 0; i < (u16)sizeof(cartridgeInfo); i++) ((u8*)&cartridgeInfo)[i] = 0x00;
		load_header();
	} // else nothing
}

SaveSize(cartridge, sizeof (cartridgeHeader) + sizeof (cartridgeInfo) )

Save(cartridge) {
	save_obj(cartridgeHeader);
	save_obj(cartridgeInfo);
}

Load(cartridge) {
	load_obj(cartridgeHeader);
	load_obj(cartridgeInfo);
}

#endif //GOBOUEMU_CARTRIDGE_H
