//
// Created by Romain on 20/07/2023.
//

#ifndef GBEMU_MAPPER_H
#define GBEMU_MAPPER_H

////////////////////////  Includes  ///////////////////////////

#include "../../types.h"
#include "../../utils.h"
#include <time.h>


////////////////////////    Types   ///////////////////////////

Struct {
	union {
		struct {
			u8 rom_bank: 5;
			u8 ram_bank: 2;
			u8 mode: 1;
		};
		struct {
			u8 lrom_bank: 7;
			u8 _: 1;
		};
		u8 raw;
	};
	u8 rom_mask: 7;
	u8 mapped_ram: 1;
} MBC1;

Struct {
	time_t date;      // Emu side date
	time_t halt_date; // Emu side date
	u8 S, M, Hr, DL;   // GB side date
	union {
		Lstruct(u8 day: 1, _: 5, halt: 1, of: 1;)
		Bstruct(u8 of: 1, halt: 1, _: 5, day: 1;)
		u8 DH;
	};
	u8 latch;
} MBC3;


Union {
	struct {
		u16 lrom_bank: 8;
		u16 hrom_bank: 1;
		u16 ram_bank: 4;
		u16 rumble: 1;
		u16 mapped_ram: 1;
		u16 _0: 1;
	};
	struct {
		u16 rom_bank: 9;
		u16 _1: 7;
	};
	u16 raw;
} MBC5;


Struct {
	reader * read_ram;   // MBC2/3...
	writer * write_ram;
	writer * write_rom0;
	writer * write_rom1; // reader override exists? -> maybe no
	void (* init)();
} MapperIO;


Union {
	MBC1 mbc1;
	MBC3 mbc3;
	MBC5 mbc5;
} MapperData;


Struct {
	MapperIO io;
	MapperData data;
} Mapper;


//////////////////////  Declarations  /////////////////////////

extern Mapper mapper;
extern const MapperIO supported_mapper[6]; // TODO improve (union: switched Reset)


////////////////////////   Methods   //////////////////////////

void mount_mapper();
void init_mapper();


/////////////////////  Registrations  /////////////////////////

Reset(mapper){
	if (hard) init_mapper();
	else if (mapper.io.init) mapper.io.init();
}

SaveSize(mapper, sizeof (mapper.data))

Save(mapper) {
	save_obj(mapper.data);
}

Load(mapper) {
	mount_mapper();
	load_obj(mapper.data);
}


#endif //GBEMU_MAPPER_H
