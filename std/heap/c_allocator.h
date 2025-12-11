#ifndef _STD_HEAP_C_ALLOCATOR_H
#define _STD_HEAP_C_ALLOCATOR_H

#include "mem/allocator.h"
#include "types.h"

void                    CAllocator_init(Allocator* allocator);
static inline Allocator NewCAllocator(void) {
        Allocator a;
        CAllocator_init(&a);
        return a;
}

#ifdef BSTD_IMPL
#        define HEAP_IMPL_C_ALLOCATOR
#endif
#ifdef HEAP_IMPL_C_ALLOCATOR
#        include "heap/c_allocator.c"
#endif

#endif  // _STD_HEAP_C_ALLOCATOR_H
