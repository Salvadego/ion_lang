#ifndef __STD_TYPES_RESULT_H__
#define __STD_TYPES_RESULT_H__

#include "error/error.h"
#include "types.h"

typedef opaque Ptr;
typedef struct {
        bool  is_error;
        Ptr   value;
        Error error;
} ResultPtr;

static inline ResultPtr OkPtr(Ptr v) {
        return (ResultPtr){.is_error = 0, .value = v};
}

static inline ResultPtr ErrPtr(Error e) {
        return (ResultPtr){.is_error = 1, .error = e};
}

/* Use when the success case contains a pointer to __type__ */
#define DefineResult(type)                                                     \
        typedef struct {                                                       \
                bool  is_error;                                                \
                type* value;                                                   \
                Error error;                                                   \
        } Result_##type;                                                       \
                                                                               \
        static inline Result_##type Ok_##type(type* v) {                       \
                return (Result_##type){.is_error = 0, .value = v};             \
        }                                                                      \
                                                                               \
        static inline Result_##type Err_##type(Error e) {                      \
                return (Result_##type){.is_error = 1, .error = e};             \
        }                                                                      \
                                                                               \
        static inline type* UnwrapFatal_##type(Result_##type r) {              \
                if (r.is_error) {                                              \
                        fprintf(stderr,                                        \
                                "[fatal] Unwrap failed: %s\n",                 \
                                r.error.message);                              \
                        exit(EXIT_FAILURE);                                    \
                }                                                              \
                return r.value;                                                \
        }                                                                      \
                                                                               \
        static inline Error Unwrap_##type(Result_##type r, type** out) {       \
                if (r.is_error) return r.error;                                \
                *out = r.value;                                                \
                return NewNoneError();                                         \
        }                                                                      \
                                                                               \
        static inline type* UnwrapOrPtr_##type(Result_##type r, type* def) {   \
                return r.is_error ? def : r.value;                             \
        }                                                                      \
                                                                               \
        static inline Result_##type Expect_##type(Result_##type r,             \
                                                  const char*   msg) {           \
                if (r.is_error) {                                              \
                        fprintf(                                               \
                            stderr, "[fatal] %s: %s\n", msg, r.error.message); \
                        exit(EXIT_FAILURE);                                    \
                }                                                              \
                return r;                                                      \
        }

#define Result(type) Result_##type

#define Ok(type, val)  Ok_##type(val)
#define Err(type, err) Err_##type(err)

/* Generic TryBind for Result<T> (pointer-style) */
#define TryBind(type, name, optional)                               \
        type* name;                                                 \
        Statement(Result_##type _tmp = (optional);                  \
                  if (_tmp.is_error) return Err_##type(_tmp.error); \
                  name = _tmp.value;)

#define TryBindPtr(type, name, optional)                            \
        type* name;                                                 \
        Statement(ResultPtr _tmp = (optional);                      \
                  if (_tmp.is_error) return Err_##type(_tmp.error); \
                  name = _tmp.value;)

/* Generic Try for Result<T> that produces Error on failure and returns from
 * current fn */
#define TryResult(type, optional) \
        Statement(Error e = Unwrap_##type((optional), (type**)0); Try(e))

#define UnwrapFatal(type, optional) UnwrapFatal_##type(optional)
#define Unwrap(type, optional, out) Unwrap_##type(optional, out)

/* UnwrapOr for Result<T> (pointer-style): returns provided pointer default on
 * error */
#define UnwrapOrPtr(type, result, def) UnwrapOrPtr_##type((result), (def))

/* UnwrapOrDefault for Result<T>: on error return a null pointer (for
 * pointer-style) */
#define UnwrapOrNull(type, result) \
        ((result).is_error ? (type*)null : (result).value)

/* Expect helper for Result<T> that aborts with message (non-returning) */
#define ExpectResult(type, result, msg)          \
        Statement(if ((result).is_error) {       \
                fprintf(stderr,                  \
                        "[fatal] %s: %s\n",      \
                        (msg),                   \
                        (result).error.message); \
                exit(EXIT_FAILURE);              \
        })

/* Match-like macros for Result<T> (pointer-style). Blocks must reference
 * variable 'r'. */
#define MatchResult(result, OkBlock, ErrBlock) \
        Statement(if (!(result).is_error) { OkBlock } else {ErrBlock})

#define OnErrorResult(result, ErrBlock) \
        Statement(if ((result).is_error) ErrBlock)

DefineResult(u8)
DefineResult(u16)
DefineResult(u32)
DefineResult(u64)

DefineResult(i8)
DefineResult(i16)
DefineResult(i32)
DefineResult(i64)

DefineResult(f32)
DefineResult(f64)

DefineResult(bool)

DefineResult(C_String)
DefineResult(ConstC_String)

#endif  // __STD_TYPES_RESULT_H__
