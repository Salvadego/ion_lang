#include "allocator.h"
#ifndef FRONT_PARSER_H

#        include "lexer.h"
#        include "string/string_view.h"

typedef enum {
        AST_String = 0,
        AST_EOF,
} ASTKind;

typedef union {
        StringView sv;
} ASTValue;

typedef struct AST {
        ASTKind  kind;
        ASTValue value;
} AST;

AST* NewAST(Allocator* alloc, const ASTKind kind, const ASTValue value);

AST ParseASTFromTokens(Allocator* alloc, Token* tokens);

#        define NewASTValue(type) NewASTValue_##type

#        ifdef BSTD_IMPL
#                define PARSER_IMPL
#        endif
#        ifdef PARSER_IMPL
#                include "parser.c"
#        endif

#endif  // !FRONT_PARSER_H
