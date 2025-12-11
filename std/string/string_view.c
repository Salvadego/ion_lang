#include "string/string_view.h"

#include <ctype.h>
#include <stdio.h>

static bool sv_internal_parse_float(StringView sv, f64* out_value) {
        SV_Trim(&sv);
        if (sv.len == 0) return false;

        const char* p   = sv.data;
        const char* end = sv.data + sv.len;

        // Sign
        int sign = 1;
        if (*p == '+') {
                p++;
        } else if (*p == '-') {
                sign = -1;
                p++;
        }

        // Integer part
        f64  int_part   = 0.0;
        bool has_digits = false;
        while (p < end && isdigit((u8)*p)) {
                int_part = int_part * 10.0 + (*p - '0');
                p++;
                has_digits = true;
        }

        // Fractional part
        f64 frac_part  = 0.0;
        f64 frac_scale = 1.0;
        if (p < end && *p == '.') {
                p++;
                while (p < end && isdigit((u8)*p)) {
                        frac_part = frac_part * 10.0 + (*p - '0');
                        frac_scale *= 10.0;
                        p++;
                        has_digits = true;
                }
        }

        if (!has_digits) return false;

        f64 value = int_part + frac_part / frac_scale;

        // Exponent part
        if (p < end && (*p == 'e' || *p == 'E')) {
                p++;
                i32 exp_sign = 1;
                if (p < end && *p == '+') {
                        p++;
                } else if (p < end && *p == '-') {
                        exp_sign = -1;
                        p++;
                }

                if (p >= end || !isdigit((u8)*p)) return false;

                i32 exp_value = 0;
                while (p < end && isdigit((u8)*p)) {
                        exp_value = exp_value * 10 + (*p - '0');
                        p++;
                }

                f64 scale = 1.0;
                for (i32 i = 0; i < exp_value; i++) {
                        scale *= 10.0;
                }

                if (exp_sign < 0) {
                        value /= scale;
                } else {
                        value *= scale;
                }
        }

        if (p != end) return false;  // leftover non-numeric characters

        *out_value = sign * value;
        return true;
}

static bool sv_internal_parse_unsigned(StringView sv, u64* out_value) {
        SV_Trim(&sv);
        if (sv.len == 0) return false;

        const char* p   = sv.data;
        const char* end = sv.data + sv.len;

        // Reject sign for unsigned
        if (*p == '-') return false;

        if (!isdigit((u8)*p)) return false;

        u64 value = 0;
        while (p < end && isdigit((u8)*p)) {
                u32 digit = (u32)(*p - '0');

                // u64 overflow check
                if (value > UINT64_MAX / 10) return false;
                if (value == UINT64_MAX / 10 && digit > (UINT64_MAX % 10))
                        return false;

                value = value * 10 + digit;

                p++;
        }

        // Reject trailing junk
        if (p != end) return false;

        *out_value = value;
        return true;
}

static bool sv_internal_parse_signed(StringView sv, i64* out_value) {
        SV_Trim(&sv);
        if (sv.len == 0) return false;

        const char* p   = sv.data;
        const char* end = sv.data + sv.len;

        int sign = 1;
        if (*p == '+') {
                p++;
        } else if (*p == '-') {
                sign = -1;
                p++;
        }

        if (p == end || !isdigit((u8)*p)) return false;

        i64 value = 0;
        while (p < end && isdigit((unsigned char)*p)) {
                u32 digit = (u32)(*p - '0');

                // i64 overflow check
                if (value > (INT64_MAX / 10)) return false;
                if (value == INT64_MAX / 10 && digit > (sign == -1 ? 8 : 7))
                        return false;

                value = value * 10 + digit;
                p++;
        }

        if (p != end) return false;

        if (sign == -1) {
                if ((u64)value > ((u64)INT64_MAX + 1)) return false;
                value = -value;
        } else {
                if (value > INT64_MAX) return false;
        }

        *out_value = value;
        return true;
}

inline char SV_At(StringView sv, size_t index) {
        return index < sv.len ? sv.data[index] : '\0';
}

inline char SV_Front(StringView sv) {
        return SV_At(sv, 0);
}

inline char SV_Back(StringView sv) {
        return SV_At(sv, sv.len - 1);
}

inline bool SV_Equal(StringView a, StringView b) {
        if (a.len != b.len) return false;
        return MemoryMatch(a.data, b.data, a.len);
}

inline StringView SV_SubSV(StringView sv, u64 start, u64 end) {
        if (start > end) start = end;
        if (start > sv.len) start = sv.len;
        if (end > sv.len) end = sv.len;
        return NewStringView(sv.data + start, end - start);
}

u64 StringCodepointLen(ConstC_String s, u64 len) {
        u64 codepoints = 0;
        for (u64 i = 0; i < len;) {
                u8  c    = (u8)s[i];
                u64 step = (c < 0x80) ? 1 : (c < 0xE0) ? 2 : (c < 0xF0) ? 3 : 4;
                i += step;  // amount of bytes in a single codepoint
                codepoints++;
        }
        return codepoints;
}

inline bool SV_HasPrefix(StringView sv, StringView pre) {
        if (pre.len > sv.len) return false;
        return MemoryMatch(sv.data, pre.data, pre.len);
}

inline bool SV_HasSuffix(StringView sv, StringView suf) {
        if (suf.len > sv.len) return false;
        u64 offset = sv.len - suf.len;
        return MemoryMatch(sv.data + offset, suf.data, suf.len);
}

inline StringView SV_ChopStartingBytes(StringView* sv, u64 n) {
        if (!sv || !sv->len) return NewStringView("", 0);
        if (n > sv->len) n = sv->len;
        StringView result = NewStringView(sv->data, n);
        sv->data += n;
        sv->len -= n;
        return result;
}

inline StringView SV_ChopEndingBytes(StringView* sv, u64 n) {
        if (!sv || !sv->len) return NewStringView("", 0);
        if (n > sv->len) n = sv->len;
        StringView result = NewStringView(sv->data + (sv->len - n), n);
        sv->len -= n;
        return result;
}

inline StringView SV_ChopPrefix(StringView* sv, StringView pre) {
        if (!sv || !sv->len) return NewStringView("", 0);
        if (!SV_HasPrefix(*sv, pre)) {
                return NewStringView("", 0);
        }

        return SV_ChopStartingBytes(sv, pre.len);
}

inline StringView SV_ChopSuffix(StringView* sv, StringView suf) {
        if (!sv || !sv->len) return NewStringView("", 0);
        if (!SV_HasSuffix(*sv, suf)) {
                return NewStringView("", 0);
        }

        return SV_ChopEndingBytes(sv, suf.len);
}

inline StringView SV_ChopWhile(StringView* sv, StringViewPredicate predicate) {
        if (!sv || !sv->len) return NewStringView("", 0);
        usize count = 0;

        while (count < sv->len && predicate(sv->data[count])) {
                count += 1;
        }

        return SV_ChopStartingBytes(sv, count);
}

inline StringView SV_ChopByDelim(StringView* sv, char delim) {
        usize cursor_pos = 0;
        while (cursor_pos < sv->len && sv->data[cursor_pos] != delim) {
                cursor_pos += 1;
        }

        if (cursor_pos < sv->len) {
                StringView r = SV_ChopStartingBytes(sv, cursor_pos + 1);
                r.len--;
                return r;
        }

        return SV_ChopStartingBytes(sv, cursor_pos);
}

inline void SV_TrimLeft(StringView* sv) {
        if (!sv || !sv->len) return;
        while (isspace(sv->data[0]) && sv->len > 0) {
                sv->data++;
                sv->len--;
        }
}

inline void SV_TrimRight(StringView* sv) {
        if (!sv || !sv->len) return;
        while (isspace(SV_Back(*sv)) && sv->len > 0) {
                sv->len--;
        }
}

inline void SV_Trim(StringView* sv) {
        if (!sv || !sv->len) return;
        SV_TrimLeft(sv);
        SV_TrimRight(sv);
}

Option(u64) SV_IndexByte(StringView sv, byte b) {
        for (u64 i = 0; i < sv.len; i++) {
                if ((byte)sv.data[i] == b) return Some(u64, i);
        }
        return None(u64);
}

Error SV_FreeOwned(StringView* sv, Allocator* alloc) {
        if (!sv) return NewGenericError("StringView is null");
        if (!alloc) return NewGenericError("Allocator is null");

        if (sv->data) {
                Allocator_Free(alloc, (opaque)sv->data);
        }

        Allocator_Free(alloc, sv);
        return NewNoneError();
}

inline bool SV_IsAlphaPredicate(const char c) {
        return (bool)isalpha((u32)c);
}

inline bool SV_IsAlnumPredicate(const char c) {
        return (bool)isalnum((u32)c);
}

inline bool SV_IsQuotePredicate(const char c) {
        return c == '"';
}

inline bool SV_IsDigitPredicate(const char c) {
        return (bool)isdigit((u32)c);
}

inline bool SV_IsSpacePredicate(const char c) {
        return (bool)isspace((u32)c);
}

Option(f64) SV_ToF64(StringView sv) {
        f64 v = 0.0;
        if (!sv_internal_parse_float(sv, &v)) {
                return None(f64);
        }
        return Some(f64, v);
}

Option(f32) SV_ToF32(StringView sv) {
        f64 v = 0.0;
        if (!sv_internal_parse_float(sv, &v)) {
                return None(f32);
        }
        return Some(f32, (f32)v);
}

Option(u8) SV_ToU8(StringView sv) {
        u64 tmp;
        if (!sv_internal_parse_unsigned(sv, &tmp) || tmp > UINT8_MAX)
                return None(u8);
        return Some(u8, (u8)tmp);
}

Option(u16) SV_ToU16(StringView sv) {
        u64 tmp;
        if (!sv_internal_parse_unsigned(sv, &tmp) || tmp > UINT16_MAX)
                return None(u16);
        return Some(u16, (u16)tmp);
}

Option(u32) SV_ToU32(StringView sv) {
        u64 tmp;
        if (!sv_internal_parse_unsigned(sv, &tmp) || tmp > UINT32_MAX)
                return None(u32);
        return Some(u32, (u32)tmp);
}

Option(u64) SV_ToU64(StringView sv) {
        u64 tmp;
        if (!sv_internal_parse_unsigned(sv, &tmp)) return None(u64);
        return Some(u64, tmp);
}

Option(i8) SV_ToI8(StringView sv) {
        i64 tmp;
        if (!sv_internal_parse_signed(sv, &tmp) || tmp < INT8_MIN ||
            tmp > INT8_MAX)
                return None(i8);
        return Some(i8, (i8)tmp);
}

Option(i16) SV_ToI16(StringView sv) {
        i64 tmp;
        if (!sv_internal_parse_signed(sv, &tmp) || tmp < INT16_MIN ||
            tmp > INT16_MAX)
                return None(i16);
        return Some(i16, (i16)tmp);
}

Option(i32) SV_ToI32(StringView sv) {
        i64 tmp;
        if (!sv_internal_parse_signed(sv, &tmp) || tmp < INT32_MIN ||
            tmp > INT32_MAX)
                return None(i32);
        return Some(i32, (i32)tmp);
}

Option(i64) SV_ToI64(StringView sv) {
        i64 tmp;
        if (!sv_internal_parse_signed(sv, &tmp)) return None(i64);
        return Some(i64, tmp);
}
