#ifndef _STD_STRING_STRING_BUILDER_H
#define _STD_STRING_STRING_BUILDER_H

#include "collections/vector.h"
#include "core/types/option.h"
#include "core/types/result.h"
#include "core/utils.h"
#include "error/error.h"
#include "extras/slices.h"
#include "mem/allocator.h"
#include "string/string_view.h"

DefineVector(u8)

typedef struct {
        Vector(u8) buf;
} StringBuilder;

DefineResult(StringBuilder)
DefineOption(StringBuilder)

// clang-format off
Error SB_Init(StringBuilder* sb, Allocator* alloc, u64 capacity);                        // Create
Error SB_Free(StringBuilder* sb);                                                        // Destroy
Error SB_Write( StringBuilder* sb, const u8* data, usize len);                           // Append raw bytes
Error SB_WriteSV(StringBuilder* sb, StringView sv);                                      // Append SV
Error SB_WriteC(StringBuilder* sb, ConstC_String cstr);                                  // Append C-string
Error SB_EnsureNull(StringBuilder* sb);                                                  // Ensure trailing NUL (writes one but does not count it)
Slice(u8) SB_Slice(StringBuilder* sb);                                                   // Obtain as non-owning slice
void SB_ToString(StringBuilder* sb, StringView* sv);
// clang-format on

#ifdef BSTD_IMPL
#        define STRING_BUILDER_IMPL
#endif

#ifdef STRING_BUILDER_IMPL
#        include "string/string_builder.c"
#endif

#endif
