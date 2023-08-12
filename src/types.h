//
// Created by Romain on 18/07/2023.
//

#ifndef GOBOUEMU_TYPES_H
#define GOBOUEMU_TYPES_H


#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define LITTLE_ENDIAN 1
// TODO: check SP/PC/HL..
#else
#define BIG_ENDIAN 1
#endif

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

typedef signed char  s8;
typedef signed short s16;
typedef signed int   s32;

typedef u8 reader(u16 addr);
typedef void writer(u16 addr, u8 value);

#define Struct typedef struct
#define Union typedef union

#endif //GOBOUEMU_TYPES_H
