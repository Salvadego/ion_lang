#ifndef _STD_EXTRAS_SLICES_H
#define _STD_EXTRAS_SLICES_H

#include "core/types/slices.h"

/*
 * Common Slice Types
 * */
DefineSlice(u8)
DefineSlice(u16)
DefineSlice(u32)
DefineSlice(u64)

DefineSlice(i8)
DefineSlice(i16)
DefineSlice(i32)
DefineSlice(i64)

DefineSlice(f32)
DefineSlice(f64)

DefineSlice(bool)

DefineSlice(C_String)
DefineSlice(ConstC_String)

#endif /* _STD_EXTRAS_SLICES_H */
