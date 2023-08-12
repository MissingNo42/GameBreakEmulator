//
// Created by Romain on 21/07/2023.
//

#ifndef GOBOUEMU_PPU_H
#define GOBOUEMU_PPU_H

#include "../types.h"

//TODO
#define inWindow(x) ((x) == (x) && ioLY)

Struct {
	u8 X, Y, tile, attr;
} ObjAttribute;

#endif //GOBOUEMU_PPU_H
