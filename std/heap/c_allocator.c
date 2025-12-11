#include "c_allocator.h"

#include <stdlib.h>

static ResultPtr c_alloc(const Allocator* allocator, usize size) {
        (void)allocator;
        opaque ptr = malloc(size);
        if (!ptr)
                return ErrPtr(
                    NewError(AllocatorCodeAllocFailed, "C alloc failed"));
        return OkPtr(ptr);
}

static void c_free(const Allocator* allocator, opaque ptr) {
        (void)allocator;
        free(ptr);
}

static Error c_destroy(Allocator* allocator) {
        if (!allocator->state)
                return NewError(AllocatorCodeAlreadyDestroyed,
                                "Allocator already destroyed");
        allocator->state  = null;
        allocator->vtable = null;
        return NewError(ErrorNone, null);
}

void CAllocator_init(Allocator* allocator) {
        static const AllocatorVTable vtable = {
            .alloc   = c_alloc,
            .free    = c_free,
            .destroy = c_destroy,
        };
        allocator->vtable = &vtable;
        allocator->state  = allocator;
}
