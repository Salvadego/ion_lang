#ifndef _STD_HEAP_VM_ARENA_H
#define _STD_HEAP_VM_ARENA_H

#include "mem/allocator.h"
#include "platform/vm.h"
#include "types.h"

typedef struct {
        bstd_vm_region_t region;
        usize            committed;
        usize            used;
        usize            page_size;
} VMArenaState;

typedef struct {
        const AllocatorVTable* vtable;
        VMArenaState*          state;
} VMArenaAllocator;

void VMArenaAllocator_init(Allocator* a, usize reserve_size);
void VMArenaAllocator_destroy(Allocator* a);

#endif
