#include <stdio.h>
#include <stdlib.h>

#define BSTD_IMPL
#include "heap/vm_arena.h"
#include "allocator.h"
#include "core/utils.h"
#include "error/error.h"
#include "heap/arena.h"
#include "io/io.h"
#include "lexer.h"
#include "string/string_view.h"

static void print_token(const Token t) {
        printf("%s", TokenTypeToString(t.type));

        switch (t.type) {
                case TOKENTYPE_IDENTIFIER:
                case TOKENTYPE_NUMBER:
                case TOKENTYPE_STRING:
                        printf(":" SV_Fmt, SV_Args(t.val.symbol));
                        break;
                case TOKENTYPE_CHAR:
                        printf(":%c", (t.val.ch));
                        break;
                default:
                        break;
        }

        printf("\n");
}

int main(int argc, char** argv) {
        if (argc != 2) {
                fprintf(stderr, "usage: test-lexer <file>\n");
                return 1;
        }

        const char* path = argv[1];

        Allocator    arena = {0};
        VMArenaState state = {0};
        With(arena = NewVMArena(&state, Gigabytes(1)),
             Allocator_Destroy(&arena)) {
                StringView file_data = {0};
                Error      err       = IO_ReadFile(&arena, path, &file_data);
                if (isError(err)) {
                        Fatal(err);
                }

                Lexer lexer = NewLexer(&arena, path, file_data);
                Lex(&lexer);

                for (usize i = 0; i < arr_len(lexer.tokens); i++) {
                        Token t = lexer.tokens[i];
                        print_token(t);
                        if (t.type == TOKENTYPE_EOF) break;
                }
        }

        return 0;
}
