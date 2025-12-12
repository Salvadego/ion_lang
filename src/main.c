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

typedef struct {
        const char* input_file;
        bool        is_pretty;
        bool        help;
} Args;

const char* shift(int* argc, char*** argv) {
        if (argc == 0) unreachable;
        --(*argc);
        return *(*argv)++;
}

static bool parse_args_sv(int*        argc,
                          char***     argv,
                          Args*       out,
                          const char* program) {
        *out           = (Args){0};
        bool stop_opts = false;

        while (*(argc)) {
                const char* current_arg = shift(argc, argv);
                StringView  a           = SV(current_arg);

                if (!stop_opts && SV_Equal(a, SV("--"))) {
                        stop_opts = true;
                        continue;
                }

                if (!stop_opts && SV_HasPrefix(a, SV("-"))) {
                        if (SV_Equal(a, SV("-h")) ||
                            SV_Equal(a, SV("--help"))) {
                                out->help = true;
                                continue;
                        }

                        if (SV_Equal(a, SV("-p")) ||
                            SV_Equal(a, SV("--pretty"))) {
                                out->is_pretty = true;
                                continue;
                        }

                        /* combined flags: -hp, -ph, etc. */
                        if (SV_Front(a) == '-' && a.len >= 2 &&
                            a.data[1] != '-') {
                                StringView flags = SV_SubSV(a, 1, a.len);
                                for (u64 j = 0; j < flags.len; j++) {
                                        char c = flags.data[j];
                                        if (c == 'h') {
                                                out->help = true;
                                        } else if (c == 'p') {
                                                out->is_pretty = true;
                                        } else {
                                                fprintf(
                                                    stderr,
                                                    "%s: unknown option -%c\n",
                                                    program,
                                                    c);
                                                return false;
                                        }
                                }
                                continue;
                        }

                        fprintf(stderr,
                                "%s: unknown option " SV_Fmt "\n",
                                program,
                                SV_Args(a));
                        return false;
                }

                if (out->input_file == NULL) {
                        out->input_file = a.data;
                        continue;
                }

                fprintf(stderr,
                        "%s: unexpected positional argument: " SV_Fmt "\n",
                        program,
                        SV_Args(a));
                return false;
        }

        return true;
}

void usage(FILE* stream, const char* const program) {
        fprintf(stream, "USAGE:\n");
        fprintf(stream, "        %s <input_file> [OPTIONS]\n", program);
        fprintf(stream, "OPTIONS: \n");
        fprintf(stream, "        -h, --help      Print this to stdout\n");
        fprintf(stream, "        -p, --pretty    Print tokens with indent\n");
}

void print_tokens(Token* tokens) {
        for (usize i = 0; i < arr_len(tokens); i++) {
                Token token = tokens[i];
                printf("%s\n", TokenTypeToString(token.type));
        }
}

void print_tokens_formatted(Token* tokens) {
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

void Lexerize(Allocator* arena, Args args) {
        StringView content = {0};
        Error      e       = IO_ReadFile(arena, args.input_file, &content);
        Fatal(e);

        Lexer lexer = NewLexer(arena, args.input_file, content);
        if (!Lex(&lexer)) {
                unreachable;
                exit(1);
        }

        if (args.is_pretty) {
                print_tokens_formatted(lexer.tokens);
                return;
        }
        print_tokens(lexer.tokens);
}

int main(int argc, char** argv) {
        const char* program = shift(&argc, &argv);
        if (argc == 0) {
                usage(stderr, program);
                exit(1);
        }

        Args args = {0};
        if (!parse_args_sv(&argc, &argv, &args, program)) {
                usage(stderr, program);
                return 1;
        }

        if (args.help) {
                usage(stdout, program);
                exit(0);
        }

        Allocator arena = {0};
        With(arena = NewVMArena(Gigabytes(1)), Allocator_Destroy(&arena)) {
                Lexerize(&arena, args);
        }

        return 0;
}
