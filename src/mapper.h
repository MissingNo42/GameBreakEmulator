//
// Created by Romain on 20/07/2023.
//

#ifndef GOBOUEMU_MAPPER_H
#define GOBOUEMU_MAPPER_H

////////////////////////  Includes  ///////////////////////////

#include "types.h"
#include "utils.h"


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
	reader * read_ram;   // MBC3/5...
	writer * write_ram;
	writer * write_rom0;
	writer * write_rom1; //  reader overriding not exist?
	void (* init)();
} MapperIO;

Union {
	MBC1 mbc1;
} MapperData;

Struct {
	MapperIO io;
	MapperData data;
} Mapper;


//////////////////////  Declarations  /////////////////////////

extern Mapper mapper;
extern const MapperIO supported_mapper[2]; // TODO improve (union: switched Reset)


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


#endif //GOBOUEMU_MAPPER_H
