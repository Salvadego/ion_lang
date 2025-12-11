#ifndef __STD_TYPES_OPTION_H__
#define __STD_TYPES_OPTION_H__

#include "error/error.h"
#include "types.h"

/* ---------- Option (value-style) ---------- */
/* Use when success holds a value (not pointer) */
#define DefineOption(type)                                                \
        typedef struct {                                                  \
                bool is_some;                                             \
                type value;                                               \
        } Option_##type;                                                  \
                                                                          \
        static inline Option_##type Some_##type(type v) {                 \
                return (Option_##type){.is_some = 1, .value = v};         \
        }                                                                 \
                                                                          \
        static inline Option_##type None_##type(void) {                   \
                return (Option_##type){.is_some = 0};                     \
        }                                                                 \
                                                                          \
        static inline type UnwrapOr_##type(Option_##type o, type def) {   \
                return o.is_some ? o.value : def;                         \
        }                                                                 \
                                                                          \
        static inline type UnwrapOrDefault_##type(Option_##type o) {      \
                return o.is_some ? o.value : (type){0};                   \
        }                                                                 \
                                                                          \
        static inline type UnwrapFatalOption_##type(Option_##type o) {    \
                if (!o.is_some) {                                         \
                        fprintf(stderr,                                   \
                                "[fatal] Option unwrap failed (none)\n"); \
                        exit(EXIT_FAILURE);                               \
                }                                                         \
                return o.value;                                           \
        }

#define Option(type)      Option_##type
#define Some(type, value) Some_##type(value)
#define None(type)        None_##type()

#define UnwrapOptionOr(type, option, def)   UnwrapOr_##type((option), (def))
#define UnwrapOptionOrDefault(type, option) UnwrapOrDefault_##type((option))
#define ExpectOption(type, option)          UnwrapFatalOption_##type((option))

/*
 * Base Options types
 * */

DefineOption(u8)
DefineOption(u16)
DefineOption(u32)
DefineOption(u64)

DefineOption(i8)
DefineOption(i16)
DefineOption(i32)
DefineOption(i64)

DefineOption(f32)
DefineOption(f64)

DefineOption(bool)

DefineOption(C_String)
DefineOption(ConstC_String)

#endif  // __STD_TYPES_OPTION_H__
