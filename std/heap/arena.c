#include "arena.h"

#include <stdlib.h>

#include "error/error.h"
#include "platform/vm.h"
#include "vm_arena.h"

void static_arena_init(Arena* arena, u64 capacity) {
        StaticArenaState* state =
            (StaticArenaState*)malloc(sizeof(StaticArenaState));
        assert(state != null);
        state->buffer = (u8*)malloc(capacity);
        assert(state->buffer != null);
        state->capacity = capacity;
        state->offset   = 0;
        arena->state    = state;
}

void static_arena_free(Arena* arena) {
        StaticArenaState* state = (StaticArenaState*)arena->state;
        free(state->buffer);
        free(state);
        arena->state = null;
}

opaque static_arena_alloc(Arena* arena, usize size) {
        StaticArenaState* state          = (StaticArenaState*)arena->state;
        u64               alignment      = sizeof(opaque);
        usize             aligned_offset = align_up(state->offset, alignment);
        if (aligned_offset + size > state->capacity) {
                return null;  // no resize for static arena
        }
        opaque ptr    = state->buffer + aligned_offset;
        state->offset = aligned_offset + size;
        return ptr;
}

void static_arena_reset(Arena* arena) {
        StaticArenaState* state = (StaticArenaState*)arena->state;
        state->offset           = 0;
}

static ArenaChunk* chunk_new(usize capacity) {
        ArenaChunk* c = (ArenaChunk*)malloc(sizeof(ArenaChunk));
        if (!c) return null;
        c->buffer = (u8*)malloc(capacity);
        if (!c->buffer) {
                free(c);
                return null;
        }
        c->capacity = capacity;
        c->offset   = 0;
        c->next     = null;
        return c;
}

void resizeable_arena_init(Arena* arena, usize capacity) {
        ResizeableArenaState* state =
            (ResizeableArenaState*)malloc(sizeof(ResizeableArenaState));
        ArenaChunk* c = chunk_new(capacity);
        assert(c != null);
        state->head  = c;
        state->tail  = c;
        arena->state = state;
}

void resizeable_arena_free(Arena* arena) {
        ResizeableArenaState* state = (ResizeableArenaState*)arena->state;
        ArenaChunk*           c     = state->head;
        while (c) {
                ArenaChunk* next = c->next;
                free(c->buffer);
                free(c);
                c = next;
        }
        free(state);
        arena->state = null;
}

opaque resizeable_arena_alloc(Arena* arena, usize size) {
        ResizeableArenaState* state     = (ResizeableArenaState*)arena->state;
        usize                 alignment = sizeof(opaque);
        ArenaChunk*           tail      = state->tail;
        usize aligned_offset            = align_up(tail->offset, alignment);

        if (aligned_offset + size <= tail->capacity) {
                opaque ptr   = tail->buffer + aligned_offset;
                tail->offset = aligned_offset + size;
                return ptr;
        }

        u64 new_capacity = tail->capacity * 2;
        if (new_capacity < size) new_capacity = size;
        ArenaChunk* c = chunk_new(new_capacity);
        if (!c) return null;

        tail->next  = c;
        state->tail = c;

        usize  off = align_up(c->offset, alignment); /* usually 0 */
        opaque ptr = c->buffer + off;
        c->offset  = off + size;
        return ptr;
}

void resizeable_arena_reset(Arena* arena) {
        ResizeableArenaState* state = (ResizeableArenaState*)arena->state;
        ArenaChunk*           c     = state->head->next;
        while (c) {
                ArenaChunk* next = c->next;
                free(c->buffer);
                free(c);
                c = next;
        }
        state->head->next   = null;
        state->tail         = state->head;
        state->head->offset = 0;
}

const ArenaVTable static_arena_vtable = {
    .init  = static_arena_init,
    .alloc = static_arena_alloc,
    .reset = static_arena_reset,
    .free  = static_arena_free,
};

const ArenaVTable resizeable_arena_vtable = {
    .init  = resizeable_arena_init,
    .alloc = resizeable_arena_alloc,
    .reset = resizeable_arena_reset,
    .free  = resizeable_arena_free,
};

inline void Arena_init(Arena* arena, usize capacity, ArenaType type) {
        if (type == ArenaTypeStatic) {
                arena->vtable = &static_arena_vtable;
                arena->vtable->init(arena, capacity);
                return;
        }

        arena->vtable = &resizeable_arena_vtable;
        arena->vtable->init(arena, capacity);
}

inline opaque Arena_alloc(Arena* arena, usize size) {
        return arena->vtable->alloc(arena, size);
}

inline void Arena_reset(Arena* arena) {
        arena->vtable->reset(arena);
}

inline void Arena_free(Arena* arena) {
        arena->vtable->free(arena);
}

typedef struct {
        Arena arena;
} ArenaAllocatorState;

static ResultPtr arena_alloc(const Allocator* allocator, usize size) {
        ArenaAllocatorState* state = allocator->state;
        opaque               p     = Arena_alloc(&state->arena, size);
        if (!p) return ErrPtr(AllocErrorAllocFailed());
        return OkPtr(p);
}

static void arena_free(const Allocator* allocator, opaque ptr) {
        (void)allocator;
        (void)ptr;
}

static Error arena_destroy(Allocator* allocator) {
        ArenaAllocatorState* state = allocator->state;
        if (!state) return AllocErrorAlreadyDestroyed();
        Arena_free(&state->arena);
        free(state);
        allocator->state  = null;
        allocator->vtable = null;
        return NewError(ErrorNone, null);
}

void ArenaAllocator_init(Allocator* allocator, usize capacity, ArenaType type) {
        ArenaAllocatorState* state =
            (ArenaAllocatorState*)malloc(sizeof(ArenaAllocatorState));
        assert(state != null);
        Arena_init(&state->arena, capacity, type);

        static AllocatorVTable vtable = {
            .alloc   = arena_alloc,
            .free    = arena_free,
            .destroy = arena_destroy,
        };

        allocator->vtable = &vtable;
        allocator->state  = state;
}

static ResultPtr vm_arena_alloc(const Allocator* alloc, usize size) {
        if (!alloc || !alloc->state) return ErrPtr(AllocErrorAllocFailed());
        VMArenaState* s = (VMArenaState*)alloc->state;

        const usize alignment = sizeof(opaque);
        usize       cur       = align_up(s->used, alignment);
        usize       new_used  = cur + size;

        if (new_used > s->region.size) return ErrPtr(AllocErrorAllocFailed());

        if (new_used > s->committed) {
                usize need        = new_used - s->committed;
                usize commit_size = align_up(need, s->page_size);

                if (s->committed + commit_size > s->region.size) {
                        commit_size = s->region.size - s->committed;
                }

                opaque commit_addr = (u8*)s->region.start + s->committed;
                if (!bstd_vm_commit(commit_addr, commit_size)) {
                        return ErrPtr(AllocErrorAllocFailed());
                }
                s->committed += commit_size;
        }

        opaque out = (u8*)s->region.start + cur;
        s->used    = new_used;
        return OkPtr(out);
}

static void vm_arena_free(const Allocator* a, opaque p) {
        (void)a;
        (void)p;
}

static Error vm_arena_destroy(Allocator* a) {
        if (!a || !a->state) return AllocErrorAlreadyDestroyed();
        VMArenaState* s = (VMArenaState*)a->state;

        if (s->committed > 0) {
                bstd_vm_decommit(s->region.start, s->committed);
        }
        bstd_vm_release(s->region.start, s->region.size);

        a->state  = NULL;
        a->vtable = NULL;
        return NewNoneError();
}

void VMArenaAllocator_init(Allocator* a, VMArenaState* s, usize reserve_size) {
        if (!a) return;

        // VMArenaState* s = (VMArenaState*)malloc(sizeof(VMArenaState));
        if (!s) {
                a->state  = NULL;
                a->vtable = NULL;
                return;
        }

        usize p = vm_page_size();
        if (p <= 0) p = 4096;

        s->page_size = (usize)p;
        reserve_size = align_up(reserve_size, s->page_size);

        s->region = bstd_vm_reserve(reserve_size);
        if (!s->region.start || s->region.size == 0) {
                a->state  = NULL;
                a->vtable = NULL;
                return;
        }

        s->committed = 0;
        s->used      = 0;

        static const AllocatorVTable vt = {
            .alloc   = vm_arena_alloc,
            .free    = vm_arena_free,
            .destroy = vm_arena_destroy,
        };

        a->vtable = &vt;
        a->state  = s;
}
