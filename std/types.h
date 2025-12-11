#ifndef __TYPES_H__
#define __TYPES_H__

#include <stddef.h>
#include <stdint.h>

// String types.
typedef char*       C_String;
typedef const char* ConstC_String;

// Unsigned int types.
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef size_t   usize;

// Regular int types.
typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

// Float types.
typedef float  f32;
typedef double f64;

// Boolean.
typedef i32 bool;
#define true 1
#define false 0

// Byte, word, dword, qword.
typedef u8  byte;
typedef u16 word;
typedef u32 dword;
typedef u64 qword;

// Void type.
typedef void* opaque;
typedef void  VoidFunc(void);

#endif
