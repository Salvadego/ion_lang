#ifndef _STD_TYPES_SLICE_H
#define _STD_TYPES_SLICE_H

#include "core/types/option.h"
#include "core/types/result.h"
#include "core/utils.h"
#include "types.h"

/*
    Slice(T) is a non-owning view of elements:
        data: pointer to T
        len:  number of elements
*/

#define DefineSlice(type)                                                     \
        typedef struct {                                                      \
                type* data;                                                   \
                u64   len;                                                    \
        } Slice_##type;                                                       \
                                                                              \
        static inline Slice_##type NewSlice_##type(type* data, u64 len) {     \
                if (!data) len = 0;                                           \
                return (Slice_##type){.data = data, .len = len};              \
        }                                                                     \
                                                                              \
        static inline bool SliceIsEmpty_##type(const Slice_##type s) {        \
                return s.len == 0;                                            \
        }                                                                     \
                                                                              \
        static inline type* SliceGetPtr_##type(const Slice_##type s, u64 i) { \
                return (s.data && i < s.len) ? &s.data[i] : (type*)null;      \
        }                                                                     \
                                                                              \
        static inline Option(type)                                            \
            SliceGet_##type(const Slice_##type s, u64 i) {                    \
                if (s.data && i < s.len) return Some(type, s.data[i]);        \
                return None(type);                                            \
        }                                                                     \
        static inline bool SliceContains_##type(const Slice_##type s,         \
                                                type               v) {                     \
                for (u64 i = 0; i < s.len; i++) {                             \
                        if (MemoryMatch(&s.data[i], &v, sizeof(type)))        \
                                return true;                                  \
                }                                                             \
                return false;                                                 \
        }                                                                     \
        static inline bool SliceSet_##type(                                   \
            const Slice_##type s, u64 i, type v) {                            \
                if (s.data && i < s.len) {                                    \
                        s.data[i] = v;                                        \
                        return true;                                          \
                }                                                             \
                return false;                                                 \
        }                                                                     \
                                                                              \
        static inline Slice_##type SliceSub_##type(                           \
            const Slice_##type s, u64 start, u64 end) {                       \
                if (!s.data || start >= s.len)                                \
                        return NewSlice_##type(null, 0);                      \
                if (start > end) start = end;                                 \
                if (start > s.len) start = s.len;                             \
                if (end > s.len) end = s.len;                                 \
                return NewSlice_##type(s.data + start, end - start);          \
        }                                                                     \
                                                                              \
        static inline bool SliceEquals_##type(const Slice_##type a,           \
                                              const Slice_##type b) {         \
                if (a.len != b.len) return false;                             \
                if (a.len == 0) return true;                                  \
                return MemoryMatch(a.data, b.data, sizeof(type) * a.len);     \
        }                                                                     \
                                                                              \
        static inline Option(u64)                                             \
            SliceIndexOf_##type(const Slice_##type s, const type v) {         \
                for (u64 i = 0; i < s.len; i++) {                             \
                        if (MemoryMatch(&s.data[i], &v, sizeof(type)))        \
                                return Some(u64, i);                          \
                }                                                             \
                return None(u64);                                             \
        }                                                                     \
        static inline Option(type) SliceFirst_##type(const Slice_##type s) {  \
                return SliceGet_##type(s, 0);                                 \
        }                                                                     \
        static inline Option(type) SliceLast_##type(const Slice_##type s) {   \
                return (s.len == 0) ? None(type)                              \
                                    : Some(type, s.data[s.len - 1]);          \
        }                                                                     \
        DefineResult(Slice_##type)                                            \
        DefineOption(Slice_##type)

#define Slice(type) Slice_##type

#define NewSliceFrom(type, ...)                \
        NewSlice_##type((type[]){__VA_ARGS__}, \
                        ArrayCount(((type[]){__VA_ARGS__})))

#define NewSliceFromArray(type, arr)    NewSlice_##type(arr, ArrayCount(arr))
#define NewSliceFromPtr(type, ptr, len) NewSlice_##type(ptr, len)

/* Helpers */
#define SliceGet(type, s, i)    SliceGet_##type(s, i)
#define SliceGetPtr(type, s, i) SliceGetPtr_##type(s, i)

#define OkSlice(type, val)  (Ok(Slice_##type, val))
#define ErrSlice(type, err) (Err(Slice_##type, err))

/* Option<Slice<T>> */
#define SomeSlice(type, val) (Some(Slice_##type, val))
#define NoneSlice(type)      (None(Slice_##type))

/* Indexing helper for Option<u64> */
#define SliceIndexOf(type, s, v) SliceIndexOf_##type(s, v)

/* First element helper */
#define SliceFirst(type, s) SliceFirst_##type(s)

/* Last element helper */
#define SliceLast(type, s) SliceLast_##type(s)

/* Sub-slice helper */
#define SliceSub(type, s, start, end) SliceSub_##type(s, start, end)

/* Comparison helper */
#define SliceEquals(type, a, b) SliceEquals_##type(a, b)

#define SliceIsEmpty(type, s) SliceIsEmpty_##type(s)

#define SliceContains(type, s, v) SliceContains_##type(s, v)
#define SliceSet(type, s, i, v)   SliceSet_##type(s, i, v)

#endif /* _STD_TYPES_SLICE_H */
