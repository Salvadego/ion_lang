#ifndef STD_IO_IO_H
#define STD_IO_IO_H

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include "error/error.h"
#include "string/string_view.h"
#include "types.h"

// TODO: Add a File type
// typedef struct File {
//         // file_t is the actual file type on the platform
//         // file_t f;
// } File;

Error IO_ReadFile(Allocator* alloc, ConstC_String path, StringView* sv);

#ifdef BSTD_IMPL
#        define IO_IMPL
#endif
#ifdef IO_IMPL
Error IO_ReadFile(Allocator* alloc, ConstC_String path, StringView* sv) {
        FILE* f = fopen(path, "rb");
        if (f == NULL) {
                return NewErrorFromErrNo();
        }

        if (fseek(f, 0, SEEK_END) != 0) {
                fclose(f);
                return NewErrorFromErrNo();
        }

        i64 count = ftell(f);
        if (count < 0) {
                fclose(f);
                return NewErrorFromErrNo();
        }

        if (fseek(f, 0, SEEK_SET) != 0) {
                fclose(f);
                return NewErrorFromErrNo();
        }

        ResultPtr r = Allocator_Alloc(alloc, (usize)count * sizeof(char));
        if (r.is_error) {
                fclose(f);
                return r.error;
        }

        C_String data = r.value;
        if (fread(data, (usize)count, 1, f) != 1) {
                fclose(f);
                Allocator_Free(alloc, data);
                return NewErrorFromErrNo();
        }

        if (ferror(f)) {
                fclose(f);
                Allocator_Free(alloc, data);
                return NewErrorFromErrNo();
        }

        if (fclose(f) != 0) {
                Allocator_Free(alloc, data);
                return NewErrorFromErrNo();
        }

        *sv = NewStringView(data, (usize)count);
        return NewNoneError();
}
#endif

#endif
