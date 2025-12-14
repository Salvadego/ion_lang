#ifndef _STD_TYPES_CONTRACT_H
#define _STD_TYPES_CONTRACT_H

// CastSelf: cast vself to concrete type pointer inside impl functions
#define CastSelf(type) type* self = (type*)vself

// VFunc: declare contract method signatures in the contract definition
#define VFuncConst0(ret, name) ret (*name)(const opaque vself)
#define VFuncConst(ret, name, ...) \
        ret (*name)(const opaque vself, ##__VA_ARGS__)
#define VFunc(ret, name, ...) ret (*name)(opaque vself, ##__VA_ARGS__)
#define VFunc0(ret, name)     ret (*name)(opaque vself)

// Contract macro: defines the vtable struct and the contract struct with vself
// and vptr
#define Contract(name)                            \
        typedef struct name##VTable name##VTable; \
        typedef opaque              name##State;  \
        typedef struct name {                     \
                name##State         vself;        \
                const name##VTable* vptr;         \
        } name;                                   \
        struct name##VTable

// Sign macro: declares static vtable and conversion function to create contract
// from concrete obj
#define Sign(name, type)                                            \
        static const name##VTable type##_##name##_vtable;           \
        static inline name        type##_As##name(type* obj) {      \
                return (name){obj, &type##_##name##_vtable}; \
        }                                                           \
        static const name##VTable type##_##name##_vtable

// ContractCall macro: calls contract method conveniently
#define ContractCall0(obj, method) ((obj).vptr->method((obj).vself))
#define ContractCall(obj, method, ...) \
        ((obj).vptr->method((obj).vself, ##__VA_ARGS__))

// CttType macro: casts obj to contract type
#define CttType(name, type, ...) type##_As##name((&(type)__VA_ARGS__))

// CttTypeP macro: casts obj to contract type
#define CttTypeP(name, type, obj) type##_As##name((obj))

#endif /* _STD_TYPES_CONTRACT_H */
