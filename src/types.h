//
// Created by Romain on 18/07/2023.
//

#ifndef GBEMU_TYPES_H
#define GBEMU_TYPES_H

#include <stdint.h>


////////////////////////   Macros   ///////////////////////////

/**
 * @brief Define macros for Endian care struct
 * */
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define IS_LITTLE_ENDIAN 1
#define Lstruct(...) struct {__VA_ARGS__;};
#define Bstruct(...)
// TODO: check SP/PC/HL/Structs..
#else
#define IS_BIG_ENDIAN 1
#define Lstruct(...)
#define Bstruct(...) struct {__VA_ARGS__;};
#endif


/// Endianned Struct (N fields): takes the little endian layout
//#define Estruct(type, ...) Lstruct(type __VA_ARGS__) Bstruct(type __VA_ARGS__)
#define Estruct2(type, A, B) Lstruct(type A, B) Bstruct(type B, A)
#define Estruct3(type, A, B, C) Lstruct(type A, B, C) Bstruct(type C, B, A)
#define Estruct4(type, A, B, C, D) Lstruct(type A, B, C, D) Bstruct(type D, C, B, A)
#define Estruct5(type, A, B, C, D, E) Lstruct(type A, B, C, D, E) Bstruct(type E, D, C, B, A)
#define Estruct6(type, A, B, C, D, E, F) Lstruct(type A, B, C, D, E, F) Bstruct(type F, E, D, C, B, A)


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

#endif //GBEMU_TYPES_H
