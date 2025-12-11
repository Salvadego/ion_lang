#ifndef _STD_ERROR_ERROR_H
#define _STD_ERROR_ERROR_H

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core/utils.h"
#include "types.h"

// None error code = -1
// success codes are < 100
typedef enum { ErrorNone = -1 } ErrorCode;

// Allocator error codes are >= 100 and < 200
typedef enum {
        AllocatorCodeAllocFailed      = 100,
        AllocatorCodeAlreadyDestroyed = 101,
} AllocatorErrorCode;

// IO error codes are >= 200 and < 300
typedef enum {
        IOErrorOpenFailed  = 200,
        IOErrorReadFailed  = 201,
        IOErrorWriteFailed = 202
} IOErrorCode;

// Vector error codes are >= 300 and < 400
typedef enum {
        VectorCodeInvalidArgument = 300,
        VectorCodeOutOfBounds     = 301,
} VectorErrorCode;

typedef struct {
        i32           code;
        ConstC_String message;
} Error;

static inline Error NewError(i32 code, ConstC_String message) {
        return (Error){.code = code, .message = message};
}

static inline Error NewGenericError(ConstC_String message) {
        return NewError(0, message);
}

static inline Error NewErrorFromErrNo(void) {
        return NewError(errno, strerror(errno));
}

static inline Error NewNoneError(void) {
        return NewError(ErrorNone, null);
}

static inline Error AllocErrorAllocFailed(void) {
        return NewError(AllocatorCodeAllocFailed, "allocation failed");
}

static inline Error AllocErrorAlreadyDestroyed(void) {
        return NewError(AllocatorCodeAlreadyDestroyed,
                        "allocator already destroyed");
}

static inline Error VectorErrorInvalidArgument(void) {
        return NewError(VectorCodeInvalidArgument,
                        "invalid argument to vector operation");
}

static inline Error VectorErrorOutOfBounds(void) {
        return NewError(VectorCodeOutOfBounds,
                        "vector operation out of bounds");
}

static inline bool isError(Error error) {
        return error.code != ErrorNone;
}

#define Try(error) Statement(if (error.code != ErrorNone) { return error; })
#define OnError(error, ErrBlock) \
        Statement(if (error.code != ErrorNone) ErrBlock)
#define Fatal(error)                                            \
        Statement(if (error.code != ErrorNone) {                \
                fprintf(stderr, "[fatal] %s\n", error.message); \
                exit(EXIT_FAILURE);                             \
        })

#endif  // _STD_ERROR_ERROR_H
