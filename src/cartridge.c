//
// Created by Romain on 19/07/2023.
//

////////////////////////  Includes  ///////////////////////////

#include "cartridge.h"
#include "units/mmu.h"
#include <stdio.h>


//////////////////////  Declarations  /////////////////////////

CartridgeHeader cartridgeHeader;
CartridgeInfo cartridgeInfo;
u8 GBC;
static FILE * file;

const char * const OCODE[] = {"None", "Nintendo", "-", "-", "-", "-", "-", "-", "Capcom", "Hot-B", "Jaleco", "Coconuts Japan", "Elite Systems", "-", "-", "-", "-", "-", "-", "EA (Electronic Arts)", "-", "-", "-", "-", "Hudsonsoft", "ITC Entertainment", "Yanoman", "-", "-", "Japan Clary", "-", "Virgin Interactive", "-", "-", "-", "-", "PCM Complete", "San-X", "-", "-", "Kotobuki Systems", "Seta", "-", "-", "-", "-", "-", "-", "Infogrames", "Nintendo", "Bandai", "See New Code", "Konami", "HectorSoft", "-", "-", "Capcom", "Banpresto", "-", "-", ".Entertainment i", "-", "Gremlin", "-", "-", "Ubisoft", "Atlus", "-", "Malibu", "-", "Angel", "Spectrum Holoby", "-", "Irem", "Virgin Interactive", "-", "-", "Malibu", "-", "U.S. Gold", "Absolute", "Acclaim", "Activision", "American Sammy", "GameTek", "Park Place", "LJN", "Matchbox", "-", "Milton Bradley", "Mindscape", "Romstar", "Naxat Soft", "Tradewest", "-", "-", "Titus", "Virgin Interactive", "-", "-", "-", "-", "-", "Ocean Interactive", "-", "EA (Electronic Arts)", "-", "-", "-", "-", "Elite Systems", "Electro Brain", "Infogrames", "Interplay", "Broderbund", "Sculptered Soft", "-", "The Sales Curve", "-", "-", "t.hq", "Accolade", "Triffix Entertainment", "-", "Microprose", "-", "-", "Kemco", "Misawa Entertainment", "-", "-", "Lozc", "-", "-", "Tokuma Shoten Intermedia", "-", "-", "-", "-", "Bullet-Proof Software", "Vic Tokai", "-", "Ape", "I’Max", "-", "Chunsoft Co.", "Video System", "Tsubaraya Productions Co.", "-", "Varie Corporation", "Yonezawa/S’Pal", "Kaneko", "-", "Arc", "Nihon Bussan", "Tecmo", "Imagineer", "Banpresto", "-", "Nova", "-", "Hori Electric", "Bandai", "-", "Konami", "-", "Kawada", "Takara", "-", "Technos Japan", "Broderbund", "-", "Toei Animation", "Toho", "-", "Namco", "acclaim", "ASCII or Nexsoft", "Bandai", "-", "Square Enix", "-", "HAL Laboratory", "SNK", "-", "Pony Canyon", "Culture Brain", "Sunsoft", "-", "Sony Imagesoft", "-", "Sammy", "Taito", "-", "Kemco", "Squaresoft", "Tokuma Shoten Intermedia", "Data East", "Tonkinhouse", "-", "Koei", "UFL", "Ultra", "Vap", "Use Corporation", "Meldac", ".Pony Canyon or", "Angel", "Taito", "Sofel", "Quest", "Sigma Enterprises", "ASK Kodansha Co.", "-", "Naxat Soft", "Copya System", "-", "Banpresto", "Tomy", "LJN", "-", "NCS", "Human", "Altron", "Jaleco", "Towa Chiki", "Yutaka", "Varie", "-", "Epcoh", "-", "Athena", "Asmik ACE Entertainment", "Natsume", "King Records", "Atlus", "Epic/Sony Records", "-", "IGS", "-", "A Wave", "-", "-", "Extreme Entertainment", "-", "-", "-", "-", "-", "-", "-", "-", "-", "-", "-", "LJN"};
const char * const NCODE[] = {"None", "Nintendo R&D1", "-", "-", "-", "-", "-", "-", "Capcom", "-", "-", "-", "-", "Electronic Arts", "-", "-", "-", "-", "Hudson Soft", "b-ai", "kss", "-", "pow", "-", "PCM Complete", "san-x", "-", "-", "Kemco Japan", "seta", "Viacom", "Nintendo", "Bandai", "Ocean/Acclaim", "Konami", "Hector", "-", "Taito", "Hudson", "Banpresto", "-", "Ubi Soft", "Atlus", "-", "Malibu", "-", "angel", "Bullet-Proof", "-", "irem", "Absolute", "Acclaim", "Activision", "American sammy", "Konami", "Hi tech entertainment", "LJN", "Matchbox", "Mattel", "Milton Bradley", "Titus", "Virgin", "-", "-", "LucasArts", "-", "-", "Ocean", "-", "Electronic Arts", "Infogrames", "Interplay", "Broderbund", "sculptured", "-", "sci", "-", "-", "THQ", "Accolade", "misawa", "-", "-", "lozc", "-", "-", "Tokuma Shoten Intermedia", "Tsukuda Original", "-", "-", "-", "Chunsoft", "Video system", "Ocean/Acclaim", "-", "Varie", "Yonezawa/s’pal", "Kaneko", "-", "Pack in soft"};

const CartridgeCapability CartridgeCapabilities[] = {
		{0, 0, 0, 0, 0, 0}, // 00
		{1, 0, 0, 0, 0, 0}, // 01
		{1, 1, 0, 0, 0, 0}, // 02
		{1, 1, 1, 0, 0, 0}, // 03
		{0, 0, 0, 0, 0, 0}, // 04 --
		{2, 0, 0, 0, 0, 0}, // 05
		{2, 0, 1, 0, 0, 0}, // 06
		{0, 0, 0, 0, 0, 0}, // 07 --
		{0, 0, 0, 0, 0, 0}, // 08 - unknown
		{0, 0, 0, 0, 0, 0}, // 09 - unknown
		{0, 0, 0, 0, 0, 0}, // 0a --
		{4, 0, 0, 0, 0, 0}, // 0b
		{4, 1, 0, 0, 0, 0}, // 0c
		{4, 1, 1, 0, 0, 0}, // 0d
		{0, 0, 0, 0, 0, 0}, // 0e --
		{3, 0, 1, 1, 0, 0}, // 0f
		{3, 1, 1, 1, 0, 0}, // 10
		{3, 0, 0, 0, 0, 0}, // 11
		{3, 1, 0, 0, 0, 0}, // 12
		{3, 1, 1, 0, 0, 0}, // 13
		{0, 0, 0, 0, 0, 0}, // 14 --
		{0, 0, 0, 0, 0, 0}, // 15 --
		{0, 0, 0, 0, 0, 0}, // 16 --
		{0, 0, 0, 0, 0, 0}, // 17 --
		{0, 0, 0, 0, 0, 0}, // 18 --
		{5, 0, 0, 0, 0, 0}, // 19
		{5, 1, 0, 0, 0, 0}, // 1a
		{5, 1, 1, 0, 0, 0}, // 1b
		{5, 0, 0, 0, 1, 0}, // 1c
		{5, 1, 0, 0, 1, 0}, // 1d
		{5, 1, 1, 0, 1, 0}, // 1e
		{0, 0, 0, 0, 0, 0}, // 1f --
		{6, 0, 0, 0, 0, 0}, // 20
		{0, 0, 0, 0, 0, 0}, // 21 --
		{7, 1, 1, 0, 1, 1}, // 22
};


////////////////////////   Methods   //////////////////////////

u8 hchecksum(){
	u8 c = 0;
	for (u8 off = 0x34; off <= 0x4c; off++) c -= cartridgeHeader.raw[off] + 1;
	return c;
}

u16 gchecksum(){
	u16 c = - Memory[0x14e] - Memory[0x14f];
	for (u32 i = 0; i < cartridgeInfo.rom_size; i++) c += Memory[i];
	
	#ifdef LITTLE_ENDIAN
	return c >> 8 | c << 8;
	#else
	return c;
	#endif
}

void load_info(){
	cartridgeInfo.type = CartridgeCapabilities[cartridgeHeader.type];
	GBC = cartridgeHeader.gbc_flag >> 7;
	cartridgeInfo.rom_size = 0x8000 << cartridgeHeader.rom_size;
	cartridgeInfo.rom_bank = 2 << cartridgeHeader.rom_size;
	
	switch (cartridgeHeader.ram_size) {
		case 2: cartridgeInfo.ram_bank = 1; break;
		case 3: cartridgeInfo.ram_bank = 4; break;
		case 4: cartridgeInfo.ram_bank = 16; break;
		case 5: cartridgeInfo.ram_bank = 8; break;
		default: cartridgeInfo.ram_bank = 0; break;
	}
	cartridgeInfo.ram_size = cartridgeInfo.ram_bank << 13;
}

u8 open_cartridge(char * fn) {
	file = fopen(fn, "rb");
	
	if (file) INFO("Cartridge file found", "file: '%s'\n", fn);
	else CRITICAL("Cartridge file not found", "file: '%s'\n", fn);
	
	return file != NULL;
}

u8 load_header() {
	
	fseek(file, 0x100, SEEK_SET);
	
	if (fread(&cartridgeHeader, sizeof(CartridgeHeader), 1, file) == 1) {
		load_info();
		
		INFO("Header loaded",
		    "\n\tEntry point: %04X\n\tTitle: %.11s\n\tManufacturer code: %.4s\n\tGBC flag: %02X"
		    "\n\tNew Code: %c%c : %s\n\tSGB Flag: %02X\n\tType: %02X\n\tRom size: %02X\n\tRam size: %02X"
		    "\n\tRegion: %02X\n\tOld Code: %02X : %s\n\tVersion: %02X\n\tHeader checksum: %02X\n\tGlobal Checksum: %04X\n"
		    "\n\tEmulator Color Mode: %d\n\tRom size: %uo (%d banks)\n\tRam size: %uo (%d banks)\n\tMapper : MBC%d\n\tCapability: ram=%d battery=%d timer=%d rumble=%d sensor=%d\n",
		    *(int *) cartridgeHeader.entry_point,
		    cartridgeHeader.title,
		    cartridgeHeader.mcode,
		    cartridgeHeader.gbc_flag,
		    cartridgeHeader.ncode[0], cartridgeHeader.ncode[1],
		    NCODE[10 * ((u8)(cartridgeHeader.ncode[0] - '0') % 13) + (u8)(cartridgeHeader.ncode[1] - '0')],
		    cartridgeHeader.sgb_flag,
		    cartridgeHeader.type,
		    cartridgeHeader.rom_size,
		    cartridgeHeader.ram_size,
		    cartridgeHeader.region,
		    cartridgeHeader.ocode, OCODE[cartridgeHeader.ocode],
		    cartridgeHeader.version,
		    cartridgeHeader.hchecksum,
		    cartridgeHeader.checksum,
		    GBC,
		    cartridgeInfo.rom_size,
		    cartridgeInfo.rom_bank,
		    cartridgeInfo.ram_size,
		    cartridgeInfo.ram_bank,
		    cartridgeInfo.type.mapper,
		    cartridgeInfo.type.has_ram,
		    cartridgeInfo.type.has_battery,
		    cartridgeInfo.type.has_timer,
		    cartridgeInfo.type.has_rumble,
		    cartridgeInfo.type.has_sensor
		);
		
		if (hchecksum() != cartridgeHeader.hchecksum)
			ERROR("Invalid header checksum", "cartridge: %02X / verified: %02X\n", cartridgeHeader.hchecksum,
			    hchecksum());
		else if ((cartridgeInfo.ram_size > 0) ^ cartridgeInfo.type.has_ram)
			ERROR("Incoherence between RAM specs", "has ram: %d / ram size: %u\n", cartridgeInfo.type.has_ram,
			    cartridgeInfo.ram_size);
		else {
			return 1;
		}
	} else
		CRITICAL("Cannot read header", "\n");
	return 0;
}


u8 load_cartridge() {
	
	fseek(file, 0x0, SEEK_SET);
	u8 ok = fread(Memory, 1, cartridgeInfo.rom_size, file) == cartridgeInfo.rom_size;
	
	fclose(file);
	file = NULL;
	
	if (ok) {
		INFO("Cartridge loaded", "\n");
		
		u16 c = gchecksum();
		if (c != cartridgeHeader.checksum) ERROR( "Invalid global checksum", "header: %04X / computed: %04X\n", cartridgeHeader.checksum, c);

	} else
		CRITICAL("Cannot (fully) read the cartridge file", "\n");

	return ok;
}
