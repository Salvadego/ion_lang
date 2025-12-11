#ifndef _STD_COLLECTIONS_VECTOR_H
#define _STD_COLLECTIONS_VECTOR_H

#include "core/types/option.h"
#include "core/types/result.h"
#include "core/utils.h"
#include "error.h"
#include "mem/allocator.h"

#define DefineVector(type)                                                     \
        typedef struct {                                                       \
                type*      data;                                               \
                u64        len;                                                \
                u64        cap;                                                \
                Allocator* alloc;                                              \
        } Vector_##type;                                                       \
                                                                               \
        DefineResult(Vector_##type)                                            \
        DefineOption(Vector_##type)                                            \
                                                                               \
        static inline Error NewVector_##type(                                  \
            Vector_##type* v, Allocator* a, u64 cap) {                         \
                v->data  = null;                                               \
                v->len   = 0;                                                  \
                v->cap   = 0;                                                  \
                v->alloc = a;                                                  \
                                                                               \
                if (cap == 0) return NewNoneError();                           \
                                                                               \
                ResultPtr rp = Allocator_Alloc(a, cap * sizeof(type));         \
                if (rp.is_error) return rp.error;                              \
                v->data = (type*)rp.value;                                     \
                v->cap  = cap;                                                 \
                return NewNoneError();                                         \
        }                                                                      \
                                                                               \
        static inline Error VectorFree_##type(Vector_##type* v) {              \
                if (!v) return NewNoneError();                                 \
                if (v->data) Allocator_Free(v->alloc, v->data);                \
                v->data = null;                                                \
                v->len  = 0;                                                   \
                v->cap  = 0;                                                   \
                return NewNoneError();                                         \
        }                                                                      \
                                                                               \
        static inline Error VectorPush_##type(Vector_##type* v, type value) {  \
                if (v->len == v->cap) {                                        \
                        u64       newcap = (v->cap == 0) ? 4 : v->cap * 2;     \
                        ResultPtr rp =                                         \
                            Allocator_Alloc(v->alloc, newcap * sizeof(type));  \
                        if (rp.is_error) return rp.error;                      \
                        type* p = (type*)rp.value;                             \
                        if (v->data) {                                         \
                                MemoryCopy(p, v->data, v->len * sizeof(type)); \
                                Allocator_Free(v->alloc, v->data);             \
                        }                                                      \
                        v->data = p;                                           \
                        v->cap  = newcap;                                      \
                }                                                              \
                v->data[v->len++] = value;                                     \
                return NewNoneError();                                         \
        }                                                                      \
                                                                               \
        static inline Option(type) VectorPop_##type(Vector_##type* v) {        \
                if (v->len == 0) return None(type);                            \
                v->len--;                                                      \
                return Some(type, v->data[v->len]);                            \
        }                                                                      \
                                                                               \
        static inline Option(type)                                             \
            VectorGet_##type(const Vector_##type* v, u64 i) {                  \
                if (i >= v->len) return None(type);                            \
                return Some(type, v->data[i]);                                 \
        }                                                                      \
                                                                               \
        static inline Result(type)                                             \
            VectorGetPtr_##type(Vector_##type* v, u64 i) {                     \
                if (!v) return Err(type, VectorErrorInvalidArgument());        \
                if (i >= v->len) return Err(type, VectorErrorOutOfBounds());   \
                return Ok(type, &v->data[i]);                                  \
        }                                                                      \
                                                                               \
        static inline u64 VectorLen_##type(const Vector_##type* v) {           \
                return v->len;                                                 \
        }

/* Helpers */
#define Vector(type)                 Vector_##type
#define NewVector(type, vptr, a)     NewVector_##type((vptr), (a), 4)
#define NewVectorC(type, vptr, a, c) NewVector_##type((vptr), (a), (c))
#define VectorFree(type, v)          VectorFree_##type(v)
#define VectorPush(type, v, value)   VectorPush_##type(v, value)
#define VectorPop(type, v)           VectorPop_##type(v)
#define VectorGet(type, v, i)        VectorGet_##type(v, i)
#define VectorGetPtr(type, v, i)     VectorGetPtr_##type(v, i)
#define VectorLen(type, v)           VectorLen_##type(v)

#define OkVector(type, val)   (Ok(Vector_##type, val))
#define ErrVector(type, err)  (Err(Vector_##type, err))
#define SomeVector(type, val) (Some(Vector_##type, val))
#define NoneVector(type)      (None(Vector_##type))

#endif
