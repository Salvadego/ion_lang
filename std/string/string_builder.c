#include "string/string_builder.h"

#include "core/utils.h"
#include "error/error.h"
#include "mem/allocator.h"
#include "string/string_view.h"

Error SB_Init(StringBuilder* sb, Allocator* alloc, u64 capacity) {
        if (!sb) return NewGenericError("StringBuilder pointer is null");
        if (!alloc) return AllocErrorAllocFailed();
        return NewVectorC(u8, &sb->buf, alloc, capacity);
}

Error SB_Free(StringBuilder* sb) {
        if (!sb) return NewNoneError();
        Error e = VectorFree(u8, &sb->buf);

        sb->buf.data  = null;
        sb->buf.len   = 0;
        sb->buf.cap   = 0;
        sb->buf.alloc = null;
        return e;
}

Error SB_Write(StringBuilder* sb, const u8* data, usize len) {
        if (!sb) return NewGenericError("StringBuilder is null");
        if (!sb->buf.alloc) return AllocErrorAllocFailed();
        if (!data && len > 0) return NewGenericError("data is null");

        for (usize i = 0; i < len; i++) {
                Error e = VectorPush(u8, &sb->buf, data[i]);
                if (e.code != ErrorNone) return e;
        }
        return NewNoneError();
}

Error SB_WriteChar(StringBuilder* sb, const char c) {
        return SB_Write(sb, (const u8*)&c, 1);
}

Error SB_WriteSV(StringBuilder* sb, StringView sv) {
        if (!sb) return NewGenericError("StringBuilder is null");
        return SB_Write(sb, (const u8*)sv.data, sv.len);
}

Error SB_WriteC(StringBuilder* sb, ConstC_String cstr) {
        if (!sb) return NewGenericError("StringBuilder is null");
        if (!cstr) return NewGenericError("cstr is null");
        u64 n = SV_CStrLen(cstr);
        return SB_Write(sb, (const u8*)cstr, n);
}

Error SB_EnsureNull(StringBuilder* sb) {
        if (!sb) return NewGenericError("StringBuilder is null");
        if (!sb->buf.alloc) return AllocErrorAllocFailed();

        if (sb->buf.len < sb->buf.cap) {
                sb->buf.data[sb->buf.len] = 0;
                return NewNoneError();
        }

        Error e = VectorPush(u8, &sb->buf, 0);
        if (e.code != ErrorNone) return e;

        if (sb->buf.len > 0) sb->buf.len -= 1;
        return NewNoneError();
}

Slice(u8) SB_Slice(StringBuilder* sb) {
        if (!sb) return NewSliceFromPtr(u8, null, 0);
        return NewSliceFromPtr(u8, sb->buf.data, sb->buf.len);
}

StringView SB_String(StringBuilder* sb) {
        Slice(u8) slice = SB_Slice(sb);
        StringView s    = NewStringView((const char*)slice.data, slice.len);
        return s;
}

inline void SB_Clear(StringBuilder* sb) {
        if (sb && sb->buf.data) {
                sb->buf.len = 0;
        }
}

Error SB_Clone(StringBuilder* sb, StringView* sv) {
        ResultPtr data_copy =
            Allocator_Alloc(sb->buf.alloc, sb->buf.len * sb->buf.data[0]);
        if (data_copy.is_error) return data_copy.error;
        MemoryCopy(data_copy.value, sb->buf.data, sb->buf.len);
        *sv = NewStringView((const char*)data_copy.value, sb->buf.len);
        return NewNoneError();
}
