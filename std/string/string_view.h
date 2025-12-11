#ifndef _STD_STRING_STRING_VIEW_H
#define _STD_STRING_STRING_VIEW_H

#include <ctype.h>

#include "core/types/option.h"
#include "core/types/result.h"
#include "core/types/slices.h"
#include "core/utils.h"
#include "extras/slices.h"
#include "mem/allocator.h"
#include "types.h"

/* Immutable string slice */
typedef struct {
        ConstC_String data;
        u64           len;
} StringView;

/* Constructors */
#define NewStringView(ptr, length) \
        ((StringView){.data = (ptr), .len = (length)})

#define SV_LIT(s) ((StringView){.data = (s), .len = sizeof(s) - 1})
#define SV(cstr)  NewStringView((cstr), SV_CStrLen((cstr)))

#define SV_Fmt          "%.*s"
#define SV_Args(__sv__) (int)(__sv__).len, (__sv__).data

/* Result<StringView> */
DefineResult(StringView)

typedef bool (*StringViewPredicate)(const char);

/* Basic operations */
// clang-format off
char       SV_At(StringView sv, size_t index);                                          // Get a character at index
char       SV_Front(StringView sv);                                                     // Get the first character
char       SV_Back(StringView sv);                                                      // Get the last character
bool       SV_Equal(StringView a, StringView b);                                        // Compares two strings
StringView SV_SubSV(StringView sv, u64 start, u64 end);                                 // Get a substring
bool       SV_HasPrefix(StringView sv, StringView pre);                                 // Checks if sv starts with pre
bool       SV_HasSuffix(StringView sv, StringView suf);                                 // Checks if sv ends with suf
StringView SV_ChopStartingBytes(StringView* sv, u64 n);                                 // pointer math, remove first n bytes and return the removed into sv
StringView SV_ChopEndingBytes(StringView* sv, u64 n);                                   // pointer math, remove last n bytes and return the removed into sv
StringView SV_ChopPrefix(StringView* sv, StringView pre);                               // pointer math, remove prefix and return the prefix if success
StringView SV_ChopSuffix(StringView* sv, StringView suf);                               // pointer math, remove suffix and return the prefix if success
StringView SV_ChopWhile(StringView* sv, StringViewPredicate predicate);                 // pointer math, remove starting bytes while predicate is true, returning the removed into sv
StringView SV_ChopByDelim(StringView *sv, char delim);                                  // pointer math, remove starting bytes while predicate is true, returning the removed into sv
void       SV_TrimLeft(StringView* sv);                                                 // trim whitespaces from the start. inline void
void       SV_TrimRight(StringView* sv);                                                // trim whitespaces from the end.
void       SV_Trim(StringView* sv);                                                     // Trim whitespaces from both start and end.
u64        StringCodepointLen(ConstC_String s, u64 len);                                // count codepoints
bool       SV_IsAlphaPredicate(const char);
bool       SV_IsAlnumPredicate(const char);
bool       SV_IsDigitPredicate(const char);
bool       SV_IsSpacePredicate(const char);

Option(f64) SV_ToF64(StringView sv);
Option(f32) SV_ToF32(StringView sv);
Option(u8)  SV_ToU8(StringView sv);
Option(u16) SV_ToU16(StringView sv);
Option(u32) SV_ToU32(StringView sv);
Option(u64) SV_ToU64(StringView sv);
Option(i8)  SV_ToI8(StringView sv);
Option(i16) SV_ToI16(StringView sv);
Option(i32) SV_ToI32(StringView sv);
Option(i64) SV_ToI64(StringView sv);
// clang-format on

/* Helpers */
static inline u64 SV_CStrLen(ConstC_String s) {
        u64 n = 0;
        while (s[n]) n++;
        return n;
}

static inline u64 SV_Utf8Len(StringView sv) {
        return StringCodepointLen(sv.data, sv.len);
}

static inline bool SV_IsEmpty(StringView sv) {
        return sv.len == 0;
}

static inline ConstC_String SV_Data(StringView sv) {
        return sv.data;
}

/* Options */
Option(u64) SV_IndexByte(StringView sv, byte b);

/* Freeing */
Error SV_FreeOwned(StringView* sv, Allocator* alloc);

#ifdef BSTD_IMPL
#        define STRING_VIEW_IMPL
#endif
#ifdef STRING_VIEW_IMPL
#        include "string/string_view.c"
#endif

#endif
