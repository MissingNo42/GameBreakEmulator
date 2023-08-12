//
// Created by Romain on 20/07/2023.
//

#ifndef GOBOUEMU_MAPPER_H
#define GOBOUEMU_MAPPER_H

#include "types.h"

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

Struct {
	MapperIO io;
	union {
		MBC1 mbc1;
	};
} Mapper;

extern Mapper mapper;
extern MapperIO supported_mapper[2];

void init_mapper();

#endif //GOBOUEMU_MAPPER_H
