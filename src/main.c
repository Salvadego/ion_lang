#include <stdio.h>
#include <stdlib.h>

#include "allocator.h"

#define BSTD_IMPL

#include "collections/arrays.h"
#include "core/utils.h"
#include "error/error.h"
#include "heap/arena.h"
#include "io/io.h"
#include "lexer.h"
#include "string/string_builder.h"
#include "string/string_view.h"

void usage(FILE* stream, const char* const program) {
        fprintf(stream, "%s <input_file>\n", program);
        exit(1);
}

const char* shift(int* argc, char*** argv) {
        if (argc == 0) unreachable;
        --(*argc);
        return *(*argv)++;
}

void print_tokens(Token* tokens) {
        const char* tab       = "    ";
        usize       last_line = 1;
        i32         indent    = 0;

        for (usize i = 0; i < arr_len(tokens); i++) {
                Token token = tokens[i];
                if (token.type == TOKENTYPE_EOF) break;

                bool is_lbrace = token.type == TOKENTYPE_LBRACE;
                bool is_rbrace = token.type == TOKENTYPE_RBRACE;

                // Move to next line
                if (token.location.row > last_line) {
                        last_line = token.location.row;
                        puts("");

                        // RBRACE reduces indentation BEFORE printing on new
                        // line
                        i32 line_indent = indent - (is_rbrace ? 1 : 0);
                        if (line_indent < 0) line_indent = 0;

                        for (i32 t = 0; t < line_indent; t++)
                                fputs(tab, stdout);
                }

                // Print token
                printf("%s ", TokenTypeToString(token.type));

                // Update indentation AFTER printing
                if (is_lbrace) indent++;
                if (is_rbrace) indent--;
        }

        puts("");
}

void Lexerize(Allocator* arena, const char* path) {
        StringView content = {0};
        Error      e       = IO_ReadFile(arena, path, &content);
        Fatal(e);

        Lexer lexer = NewLexer(arena, path, content);
        if (!Lex(&lexer)) {
                unreachable;
                exit(1);
        }

        print_tokens(lexer.tokens);
}

int main(int argc, char** argv) {
        const char* program = shift(&argc, &argv);
        if (argc == 0) {
                usage(stderr, program);
        }

        Allocator   arena = {0};
        const char* path  = shift(&argc, &argv);

        With(arena = NewVMArena(Gigabytes(1)), Allocator_Destroy(&arena)) {
                Lexerize(&arena, path);
        }

        return 0;
}
