//
// Created by Romain on 19/07/2023.
//

#ifndef GOBOUEMU_CARTRIDGE_H
#define GOBOUEMU_CARTRIDGE_H

#include "utils.h"
#include "types.h"

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
	
} CartridgeType;

/**
 * @brief decoded useful info from the header
 * */
Struct {
	CartridgeType type;
	u8 GBC_MODE: 1;
	u8 rom_bank;
	u8 ram_bank;
	u32 rom_size;
	u32 ram_size;
} CartridgeInfo;


extern CartridgeHeader cartridgeHeader;
extern CartridgeInfo cartridgeInfo;

extern const char * const OCODE[];
extern const char * const NCODE[];
extern const CartridgeType CartridgeTypes[];



u8 load_header(char * fn);
u8 load_cartridge(char * fn);

#endif //GOBOUEMU_CARTRIDGE_H
