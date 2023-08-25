//
// Created by Romain on 18/07/2023.
//

#ifndef GOBOUEMU_TYPES_H
#define GOBOUEMU_TYPES_H

#include <stdint.h>

////////////////////////   Macros   ///////////////////////////

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define IS_LITTLE_ENDIAN 1
// TODO: check SP/PC/HL/Structs..
#else
#define IS_BIG_ENDIAN 1
#endif


////////////////////////    Types   ///////////////////////////

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;


typedef uint_fast8_t  u8f;
typedef uint_fast16_t u16f;
typedef uint_fast32_t u32f;
typedef uint_fast64_t u64f;

typedef int_fast8_t  s8f;
typedef int_fast16_t s16f;
typedef int_fast32_t s32f;
typedef int_fast64_t s64f;


typedef u8 reader(u16 addr);
typedef void writer(u16 addr, u8 value);

#define Struct typedef struct
#define Union typedef union

#endif //GOBOUEMU_TYPES_H
