#ifndef _STD_MEM_ALLOCATOR_H
#define _STD_MEM_ALLOCATOR_H
#include <stddef.h>

#include "core/types/result.h"
#include "error/error.h"

typedef struct Allocator Allocator;
typedef opaque           AllocatorState;

typedef struct {
        ResultPtr (*alloc)(const Allocator* allocator, usize size);
        void (*free)(const Allocator* allocator, opaque ptr);
        Error (*destroy)(Allocator* allocator);
} AllocatorVTable;

struct Allocator {
        const AllocatorVTable* vtable;
        AllocatorState         state;
};

#define Allocator_AllocType(alloc, Type) \
        (Allocator_Alloc((alloc), sizeof(Type)))

static inline ResultPtr Allocator_Alloc(const Allocator* alloc, usize size) {
        if (!alloc || !alloc->vtable)
                return ErrPtr(AllocErrorAlreadyDestroyed());
        return alloc->vtable->alloc(alloc, size);
}

static inline void Allocator_Free(const Allocator* alloc, opaque ptr) {
        if (!alloc || !alloc->vtable) return;
        alloc->vtable->free(alloc, ptr);
}
static inline Error Allocator_Destroy(Allocator* alloc) {
        if (!alloc || !alloc->vtable) return AllocErrorAlreadyDestroyed();
        return alloc->vtable->destroy(alloc);
}

#endif  // _STD_MEM_ALLOCATOR_H
