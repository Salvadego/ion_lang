#include "parser.h"

#include "allocator.h"
#include "lexer.h"

AST* NewAST(Allocator* alloc, const ASTKind kind, const ASTValue value) {
        ResultPtr res = Allocator_AllocType(alloc, AST);
        if (res.is_error) Fatal(res.error);

        AST* ast = res.value;

        ast->value = value;
        ast->kind  = kind;

        return ast;
}

bool Parser_parseExpression(Allocator* alloc, Token* tokens, AST** expression) {
        if (arr_len(tokens) <= 0) return false;

        if (tokens[0].type == TOKENTYPE_STRING) {
                Token token_string = TokensShift(tokens);
                *expression        = NewAST(alloc,
                                     AST_String,
                                     (ASTValue){.sv = token_string.val.symbol});
                return true;
        }

        return false;
}

AST ParseASTFromTokens(Allocator* alloc, Token* tokens) {
        AST result = {0};
        return result;
}
