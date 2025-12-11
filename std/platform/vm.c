#include "platform/vm.h"

#include "memory.h"
#include "platform.h"
#include "types.h"

#if defined(BASE_OS_WINDOWS)
#        include <windows.h>
#elif defined(BASE_OS_LINUX) || defined(BASE_OS_FREEBSD) || \
    defined(BASE_OS_OPENBSD) || defined(BASE_OS_NETBSD) ||  \
    defined(BASE_OS_ILLUMOS) || defined(BASE_OS_UNIX) ||    \
    defined(BASE_OS_MACOS)
#        define _GNU_SOURCE
#        define _DEFAULT_SOURCE
#        define __USE_MISC
#        include <sys/mman.h>
#        include <unistd.h>

#        ifndef MAP_ANON
#                define MAP_ANON MAP_ANONYMOUS
#        endif

#else
#        error "Unsupported OS for virtual memory implementation"
#endif

usize vm_page_size(void) {
#if defined(BASE_OS_WINDOWS)
        SYSTEM_INFO info;
        GetSystemInfo(&info);
        return (usize)info.dwPageSize;
#else
        return (usize)sysconf(_SC_PAGESIZE);
#endif
}

bstd_vm_region_t bstd_vm_reserve(usize size) {
        bstd_vm_region_t r    = {0};
        usize            page = vm_page_size();
        size                  = align_up(size, page);

#if defined(BASE_OS_WINDOWS)
        opaque ptr = VirtualAlloc(NULL, size, MEM_RESERVE, PAGE_NOACCESS);
        if (!ptr) return r;
        r.start = ptr;
        r.size  = size;
        return r;

#else
        opaque ptr = mmap(NULL, size, PROT_NONE, MAP_PRIVATE | MAP_ANON, -1, 0);
        if (ptr == MAP_FAILED) return r;
        r.start = ptr;
        r.size  = size;
        return r;
#endif
}

bool bstd_vm_commit(opaque start, usize size) {
        usize page = vm_page_size();
        size       = align_up(size, page);

#if defined(BASE_OS_WINDOWS)
        opaque ptr = VirtualAlloc(start, size, MEM_COMMIT, PAGE_READWRITE);
        return ptr != NULL;

#else
        int rc = mprotect(start, size, PROT_READ | PROT_WRITE);
        return rc == 0;
#endif
}

bool bstd_vm_decommit(opaque start, usize size) {
        usize page = vm_page_size();
        size       = align_up(size, page);

#if defined(BASE_OS_WINDOWS)

        return VirtualFree(start, size, MEM_DECOMMIT) != 0;

#else
        int rc1 = mprotect(start, size, PROT_NONE);
        int rc2 = 0;

#        if defined(_POSIX_ADVISORY_INFO) || defined(__linux__) || \
            defined(__APPLE__) || defined(__FreeBSD__) ||          \
            defined(__NetBSD__) || defined(__OpenBSD__) || defined(__sun)

#                include <sys/mman.h>

#                if defined(MADV_DONTNEED)
        rc2     = madvise(start, size, MADV_DONTNEED);
#                elif defined(POSIX_MADV_DONTNEED)
        rc2 = posix_madvise(start, size, POSIX_MADV_DONTNEED);
#                else
        rc2 = 0;
#                endif

#        else
        rc2 = 0;
#        endif

        return rc1 == 0 && rc2 == 0;
#endif
}

bool bstd_vm_release(opaque start, usize size) {
        usize page = vm_page_size();
        size       = align_up(size, page);

#if defined(BASE_OS_WINDOWS)
        /* size must be 0 when using MEM_RELEASE */
        return VirtualFree(start, 0, MEM_RELEASE) != 0;

#else
        return munmap(start, size) == 0;
#endif
}
