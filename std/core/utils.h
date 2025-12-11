#ifndef MACROS_UTILS_H
#define MACROS_UTILS_H

#include "types.h"

#define null ((opaque)0)

#define Statement(S) \
        do {         \
                S    \
        } while (0)

#if defined(BASE_COMPILER_CLANG)
#        define FILE_NAME __FILE_NAME__
#else
#        define FILE_NAME __FILE__
#endif

#define flush fflush(stdout)
#define trace Statement(printf("%s:%d: Trace\n", FILE_NAME, __LINE__); flush;)
#define unreachable                                                 \
        Statement(printf("How did we get here? In %s on line %d\n", \
                         FILE_NAME,                                 \
                         __LINE__);                                 \
                  flush;)

#if !defined(AssertBreak)
#        define AssertBreak() (*(int*)0 = 0)
#endif

#if ENABLE_ASSERT
#        define AssertFailed(expr, file, line)                    \
                Statement(fprintf(stderr,                         \
                                  "Assert failed: %s at %s:%d\n", \
                                  expr,                           \
                                  file,                           \
                                  line);                          \
                          flush;                                  \
                          AssertBreak();)

#        define Assert(cond)                                      \
                Statement(if (!(cond)) {                          \
                        AssertFailed(#cond, FILE_NAME, __LINE__); \
                })
#else
#        define Assert(cond) ((void)0)
#endif

#define Stringify(x) #x

#define Glue2(a, b)          a##b
#define Glue3(a, b, c)       a##b##c
#define Glue4(a, b, c, d)    a##b##c##d
#define Glue5(a, b, c, d, e) a##b##c##d##e

#define GET_GLUE(_1, _2, _3, _4, NAME, ...) NAME
#define Glue(...)                           GET_GLUE(__VA_ARGS__, Glue4, Glue3, Glue2)(__VA_ARGS__)

#define ArrayCount(a) (sizeof(a) / sizeof(*(a)))

#define IntFromPtr(p) ((uintptr_t)(p))
#define PtrFromInt(i) ((opaque)(uintptr_t)(i))

#define OffsetOfMember(T, m) offsetof(T, m)
#define SizeOfType(T)        sizeof(T)
#define AlignOfType(T)      \
        offsetof(           \
            struct {        \
                    char c; \
                    T    t; \
            },              \
            t)

#define Kilobytes(count) (u64)((count) * (u64)1024)
#define Megabytes(count) (u64)((count)*Kilobytes(1024))
#define Gigabytes(count) (u64)((count)*Megabytes(1024))
#define Terabytes(count) (u64)((count)*Gigabytes(1024))

#define Min(a, b)             (((a) < (b)) ? (a) : (b))
#define Max(a, b)             (((a) > (b)) ? (a) : (b))
#define Clamp(a, x, b)        (((x) < (a)) ? (a) : ((b) < (x)) ? (b) : (x))
#define ClampTop(a, b)        Min(a, b)
#define ClampBot(a, b)        Max(a, b)
#define ReverseClamp(a, x, b) (((x) < (a)) ? (b) : ((b) < (x)) ? (a) : (x))
#define Wrap(a, x, b)         ReverseClamp(a, x, b)

#include <string.h>
#define MemoryZero(d, z)      memset((d), 0, (z))
#define MemoryZeroStruct(d)   MemoryZero((d), sizeof(*(d)))
#define MemoryZeroArray(d)    MemoryZero((d), sizeof(d))
#define MemoryZeroTyped(d, t) MemoryZero((d), sizeof(*(d)) * (t))

#define MemoryMatch(a, b, z) (memcmp((a), (b), (z)) == 0)

#define MemoryCopy(d, s, z) memmove((d), (s), (z))
#define MemoryCopyStruct(d, s) \
        MemoryCopy((d), (s), Min(sizeof(*(d)), sizeof(*(s))))
#define MemoryCopyArray(d, s) MemoryCopy((d), (s), Min(sizeof(s), sizeof(d)))
#define MemoryCopyTyped(d, s, t) \
        MemoryCopy((d), (s), Min(sizeof(*(d)), sizeof(*(s))) * (t))

#define With(init, cleanup) \
        for (i32 _once = ((init), 0); !_once; _once = 1, (cleanup))

#endif /* MACROS_UTILS_H */
