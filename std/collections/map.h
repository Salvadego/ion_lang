#ifndef STD_COLLECTIONS_MAP_H
#define STD_COLLECTIONS_MAP_H

#include "allocator.h"
#include "collections/vector.h"
#include "core/contract.h"
#include "core/types/option.h"
#include "core/types/result.h"
#include "error/error.h"
#include "types.h"

typedef bool(eq_fn)(opaque, opaque);
typedef u64(hash_fn)(opaque);

Contract(Map) {
        VFuncConst0(usize, len);
        VFuncConst0(bool, is_empty);
        VFuncConst(bool, contains, opaque key);
        VFuncConst(bool, put, opaque key, opaque value);
        VFuncConst(opaque, get, opaque key);
        VFuncConst(void, delete, opaque key);
        VFuncConst0(void, clear);
};

// TODO: Change to include define impl

typedef struct HashMapHeader {
        hash_fn* hash;
        eq_fn*   eq;
        usize    type_size;
} HashMapHeader;

typedef struct {
        opaque key;
        opaque value;
} Entry;

typedef struct HashMap {
        HashMapHeader header;
        usize         len;
        usize         cap;
        Entry*        data;
        Allocator     allocator;
} HashMap;

bool HashMap_new(Allocator     allocator,
                 HashMap*      m,
                 usize         cap,
                 HashMapHeader header) {
        ResultPtr r = Allocator_Alloc(&allocator, sizeof(Entry) * cap);
        if (r.is_error) return false;

        Entry* data = r.value;
        MemoryZero(data, sizeof(Entry) * cap);

        *m = (HashMap){
            .len       = 0,
            .cap       = cap,
            .data      = data,
            .allocator = allocator,
            .header    = header,
        };

        return true;
}

usize HashMap_len(const opaque vself) {
        CastSelf(HashMap);
        return self->len;
}

bool HashMap_is_empty(const opaque vself) {
        CastSelf(HashMap);
        return self->len == 0;
}

bool HashMap_contains(const opaque vself, opaque key) {
        CastSelf(HashMap);
        u64   key_hash = self->header.hash(key);
        usize index    = key_hash % self->cap;
        return self->data[index].value != null &&
               self->header.eq(self->data[index].key, key);
}

bool HashMap_put(const opaque vself, opaque key, opaque value) {
        CastSelf(HashMap);
        u64   key_hash = self->header.hash(key);
        usize index    = key_hash % self->cap;

        if (self->data[index].value != null) {
                return false;
        }

        ResultPtr result =
            Allocator_Alloc(&self->allocator, self->header.type_size);
        if (result.is_error) {
                return false;
        }

        memcpy(result.value, (opaque)value, self->header.type_size);

        self->data[index].key   = key;
        self->data[index].value = result.value;
        self->len += 1;

        return true;
}

void HashMap_delete(const opaque vself, opaque key) {
        CastSelf(HashMap);
        u64   key_hash = self->header.hash(key);
        usize index    = key_hash % self->cap;

        if (self->data[index].value == null) {
                return;
        }

        self->data[index].value = null;
        self->data[index].key   = null;

        self->len -= 1;
}

void HashMap_clear(const opaque vself) {
        CastSelf(HashMap);
        for (usize i = 0; i < self->cap; i++) {
                if (self->data[i].value != null) {
                        self->header.eq(self->data[i].value, null);
                        self->data[i].value = null;
                        self->data[i].key   = null;
                }
        }
        self->len = 0;
}

opaque HashMap_get(const opaque vself, opaque key) {
        CastSelf(HashMap);
        u64   key_hash = self->header.hash(key);
        usize index    = key_hash % self->cap;

        if (self->data[index].key == null) {
                return null;
        }
        if (self->data[index].value == null) {
                return null;
        }

        return self->data[index].value;
}

Sign(Map, HashMap) = {
    .len      = HashMap_len,
    .is_empty = HashMap_is_empty,
    .contains = HashMap_contains,
    .put      = HashMap_put,
    .delete   = HashMap_delete,
    .clear    = HashMap_clear,
};

DefineOption(Map)
static inline Option(Map) NewHashMap(Allocator allocator,
                                     usize     capacity,
                                     usize     type_size,
                                     hash_fn   hash,
                                     eq_fn     eq) {
        HashMapHeader header = {.hash = hash, .eq = eq, .type_size = type_size};

        ResultPtr r = Allocator_Alloc(&allocator, sizeof(HashMap));
        if (r.is_error) {
                return None(Map);
        }

        HashMap* m = (HashMap*)r.value;

        if (!HashMap_new(allocator, m, capacity, header)) {
                return None(Map);
        }

        Map map = HashMap_AsMap(m);
        return Some(Map, map);
}

static inline u64 djb2_hash_cstr(const opaque x) {
        if (x == null) return 0;
        const unsigned char* s    = (const unsigned char*)x;
        u64                  hash = 5381;

        while (*s) {
                hash = ((hash << 5) + hash) + *s;
                s++;
        }
        return hash;
}
static inline bool djb2_eq_cstr(const opaque x, const opaque y) {
        if (x == null || y == null) return x == y;
        return strcmp((const char*)x, (const char*)y) == 0;
}

static inline u64 djb2_hash(const opaque x) {
        if (x == null) {
                return 0;
        }
        u64 hash = 5381;
        for (usize i = 0; i < sizeof(u64); i++) {
                hash = ((hash << 5) + hash) + ((u8*)x)[i];
        }
        return hash;
}

static inline bool djb2_eq(const opaque x, const opaque y) {
        return djb2_hash(x) == djb2_hash(y);
}

static inline u64 HashU64(const opaque x) {
        return *(const u64*)x;
}

static inline bool EqU64(const opaque x, const opaque y) {
        return *(const u64*)x == *(const u64*)y;
}
#endif
