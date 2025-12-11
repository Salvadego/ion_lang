#ifndef BARED_STD_PLATFORM_MEMORY_MEMORY_H_
#define BARED_STD_PLATFORM_MEMORY_MEMORY_H_

#ifdef BSTD_IMPL
#        define PLATFORM_MEMORY_IMPL
#endif

#include "core/utils.h"
#include "platform.h"
#include "types.h"

typedef struct bstd_vm_region {
        opaque start;
        usize  size;
} bstd_vm_region_t;

bstd_vm_region_t bstd_vm_reserve(usize size);
bool             bstd_vm_commit(opaque start, usize size);
bool             bstd_vm_decommit(opaque start, usize size);
bool             bstd_vm_release(opaque start, usize size);
usize            vm_page_size(void);

static inline usize align_up(usize x, usize a) {
        return (x + (a - 1)) & ~(a - 1);
}

#ifdef BSTD_IMPL
#        define BARED_PLATFORM_MEMORY_IMPL
#endif

#ifdef BARED_PLATFORM_MEMORY_IMPL
#        include "platform/vm.c"
#endif  // PLATFORM_MEMORY_IMPL

#endif  // BARED_STD_PLATFORM_MEMORY_MEMORY_H_
