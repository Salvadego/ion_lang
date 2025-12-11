#ifndef STD_COLLECTIONS_ARRAYS_H_
#define STD_COLLECTIONS_ARRAYS_H_

#include "allocator.h"
#include "types.h"

typedef struct {
        usize      size;
        usize      capacity;
        usize      elem_size;
        Allocator* alloc;
} arr_header_t;

#define arr_header(v) \
        ((arr_header_t*)((u8*)(v) - *(usize*)((u8*)(v) - sizeof(usize))))

static inline opaque arr_new_impl(Allocator* a,
                                  usize      elem_size,
                                  usize      elem_align) {
        usize cap = 4;

        // Allocate enough for header + cap*elem + worst-case alignment padding
        usize total_bytes = sizeof(arr_header_t) + cap * elem_size + elem_align;

        ResultPtr res = Allocator_Alloc(a, total_bytes);
        if (res.is_error) return null;

        u8* base = res.value;

        // Header is always at the beginning
        arr_header_t* h = (arr_header_t*)base;

        h->size      = 0;
        h->capacity  = cap;
        h->elem_size = elem_size;
        h->alloc     = a;

        u8*       after_header = base + sizeof(arr_header_t) + sizeof(usize);
        uintptr_t aligned =
            ((uintptr_t)after_header + (elem_align - 1)) & ~(elem_align - 1);

        *(usize*)(aligned - sizeof(usize)) = (usize)(aligned - (uintptr_t)base);

        return (opaque)aligned;
}

static inline opaque arr_grow(opaque arr) {
        arr_header_t* h = arr_header(arr);

        usize new_cap = h->capacity * 2;
        usize total   = sizeof(arr_header_t) + sizeof(usize) + h->elem_size +
                      new_cap * h->elem_size;

        ResultPtr res = Allocator_Alloc(h->alloc, total);
        if (res.is_error) return null;
        u8* base = res.value;

        arr_header_t* nh = (arr_header_t*)base;
        *nh              = *h;
        nh->capacity     = new_cap;

        u8*       after_header = base + sizeof(arr_header_t) + sizeof(usize);
        uintptr_t aligned = ((uintptr_t)after_header + (h->elem_size - 1)) &
                            ~(h->elem_size - 1);

        *(usize*)(aligned - sizeof(usize)) = (usize)(aligned - (uintptr_t)base);

        memcpy((u8*)aligned, arr, h->size * h->elem_size);

        Allocator_Free(h->alloc, arr_header(arr));

        return (opaque)aligned;
}

#define arr_new(alloc, type) \
        (type*)arr_new_impl((alloc), SizeOfType(type), AlignOfType(type))
#define arr(alloc, type) arr_new(alloc, type)
#define arr_len(v)       (arr_header(v)->size)
#define arr_cap(v)       (arr_header(v)->capacity)
#define arr_free(v)      Allocator_Free(arr_header(v)->alloc, arr_header(v))

#define arr_append(v, val)                                                  \
        Statement(arr_header_t* _h = arr_header(v); if (_h->size ==         \
                                                        _h->capacity) {     \
                opaque _new = arr_grow(v);                                  \
                if (_new == null) break;                                    \
                v  = _new;                                                  \
                _h = arr_header(v);                                         \
        } memcpy((u8*)v + _h->size * _h->elem_size, &(val), _h->elem_size); \
                  _h->size++;)

#define arr_print_header(v)                        \
        Statement(arr_header_t* h = arr_header(v); \
                                                   \
                  printf("Size: %lu\nCapacity: %lu\n", h->size, h->capacity);)

#endif  // STD_COLLECTIONS_ARRAYS_H_
