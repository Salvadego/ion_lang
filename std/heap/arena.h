#ifndef _STD_HEAP_ARENA_H
#define _STD_HEAP_ARENA_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "allocator.h"
#include "types.h"
#include "vm_arena.h"

typedef struct Arena Arena;
typedef enum {
        ArenaTypeStatic,
        ArenaTypeResizable,
} ArenaType;

// Arena vtable with function pointers for operations
typedef struct {
        void (*init)(Arena* arena, u64 capacity);
        opaque (*alloc)(Arena* arena, u64 size);
        void (*reset)(Arena* arena);
        void (*free)(Arena* arena);
} ArenaVTable;

// Generic Arena struct, delegates operations to vtable
struct Arena {
        const ArenaVTable* vtable;
        AllocatorState     state;  // pointer to arena-specific data
};

// ---- Static Arena Implementation ----
typedef struct {
        u8* buffer;
        u64 capacity;
        u64 offset;
} StaticArenaState;

// ---- Resizable Arena Implementation (chunked, pointer-stable) ----
typedef struct ArenaChunk ArenaChunk;
struct ArenaChunk {
        u8*         buffer;
        u64         capacity;
        u64         offset;
        ArenaChunk* next;
};

typedef struct {
        ArenaChunk* head;
        ArenaChunk* tail;
} ResizeableArenaState;

void   Arena_init(Arena* arena, u64 capacity, ArenaType type);
opaque Arena_alloc(Arena* arena, u64 size);
void   Arena_reset(Arena* arena);
void   Arena_free(Arena* arena);
void   ArenaAllocator_init(Allocator* allocator, u64 capacity, ArenaType type);
void   ArenaAllocator_destroy(Allocator* allocator);

static inline Allocator NewArenaStatic(u64 capacity) {
        Allocator a;
        ArenaAllocator_init(&a, capacity, ArenaTypeStatic);
        return a;
}

static inline Allocator NewArenaResizable(u64 capacity) {
        Allocator a;
        ArenaAllocator_init(&a, capacity, ArenaTypeResizable);
        return a;
}

static inline Allocator NewVMArena(u64 capacity) {
        Allocator a;
        VMArenaAllocator_init(&a, capacity);
        return a;
}

#ifdef BSTD_IMPL
#        define HEAP_IMPL_ARENA
#endif

#ifdef HEAP_IMPL_ARENA
#        include "heap/arena.c"
#endif

#endif  // _STD_MEM_ARENA_H
