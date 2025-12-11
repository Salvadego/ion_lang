#include <stdio.h>

#define BSTD_IMPL

#include "core/utils.h"
#include "error/error.h"
#include "heap/arena.h"
#include "io/io.h"
#include "lexer.h"
#include "string/string_view.h"

int main(void) {
        Allocator         arena = {0};
        const char* const path  = "demo";

        With(arena = NewVMArena(Gigabytes(1)), Allocator_Destroy(&arena)) {
                StringView content = {0};
                Error      e       = IO_ReadFile(&arena, path, &content);
                Fatal(e);

                Lexer lexer = NewLexer(path, content);
                Token token = lexer_chop(&lexer);
                printf(Token_Fmt "\n", Token_Args(token));
                token = lexer_chop(&lexer);
                printf(Token_Fmt "\n", Token_Args(token));
                token = lexer_chop(&lexer);
                printf(Token_Fmt "\n", Token_Args(token));
        }

        return 0;
}
