#ifndef FRONT_LEXER_H

#define FRONT_LEXER_H

#include "core/utils.h"
#include "string/string_view.h"
#include "types.h"

typedef enum {
#define TOKEN_DEF(name, text) name,
#include "./def/token.def"
#undef TOKEN_DEF
} TokenType;

static const char* TokenTypeName[] = {
#define TOKEN_DEF(name, text) [name] = #name,
#include "./def/token.def"
#undef TOKEN_DEF
};

static const usize TokensCount = ArrayCount(TokenTypeName);

typedef struct {
        const char* lit_data;
        usize       lit_len;
        TokenType   type;
} TokenMatch;

static const TokenMatch TokenMatchTable[] = {
#define TOKEN_DEF(name, text) {text, sizeof(text) - 1, name},
#include "./def/token.def"
#undef TOKEN_DEF
};

typedef struct {
        const char* file_path;
        usize       col;
        usize       row;
} Location;

Location NewLocation(const char* path, const usize col, const usize row);

typedef union {
        StringView symbol;
} TokenValue;

typedef struct {
        TokenType  type;
        TokenValue val;
        Location   location;
} Token;

#define Token_Fmt "(%s:%zu:%zu) %s " SV_Fmt
#define Token_Args(__token__)                                             \
        token.location.file_path, token.location.row, token.location.col, \
            TokenTypeToString(token.type), SV_Args(token.val.symbol)

Token NewToken(const TokenType  type,
               const TokenValue value,
               const Location   location);

typedef struct {
        StringView content;
        Location   location;
        Token*     tokens;
} Lexer;

Lexer NewLexer(const char* path, const StringView content);
Token lexer_chop(Lexer* lexer);
bool  Lex(Lexer* lexer);

static const char* TokenTypeToString(TokenType t);

#ifdef BSTD_IMPL
#        define FRONT_LEXER_IMPL
#endif
#ifdef FRONT_LEXER_IMPL
#        include "lexer.c"
#endif

#endif  // !FRONT_LEXER_H
