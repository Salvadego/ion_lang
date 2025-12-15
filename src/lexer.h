#ifndef FRONT_LEXER_H

#define FRONT_LEXER_H

#include "collections/arrays.h"
#include "string/string_builder.h"
#include "string/string_view.h"

typedef enum {
#define SINGLE_TOKEN_DEF(name, ch, printed)    name,
#define MULTI_LITERAL_DEF(name, text, printed) name,
#define COMPLEX_TOKEN_DEF(name, printed)       name,
#include "./def/tokens.def"
#undef SINGLE_TOKEN_DEF
#undef MULTI_LITERAL_DEF
#undef COMPLEX_TOKEN_DEF
} TokenType;

static const char* TokenTypeName[] = {
#define SINGLE_TOKEN_DEF(name, ch, printed)    [name] = #printed,
#define MULTI_LITERAL_DEF(name, text, printed) [name] = #printed,
#define COMPLEX_TOKEN_DEF(name, printed)       [name] = #printed,
#include "./def/tokens.def"
#undef SINGLE_TOKEN_DEF
#undef MULTI_LITERAL_DEF
#undef COMPLEX_TOKEN_DEF
};

static const usize TokensCount = ArrayCount(TokenTypeName);

typedef struct {
        const char* file_path;
        usize       col;
        usize       row;
} Location;

Location NewLocation(const char* path, const usize col, const usize row);

typedef union {
        StringView symbol;
        char       ch;
} TokenValue;

typedef struct {
        TokenType  type;
        TokenValue val;
        Location   location;
} Token;

typedef struct {
        const char* lit;
        usize       len;
        TokenType   type;
} MultiLiteral;

typedef struct Lexer {
        StringView     content;
        StringBuilder* temp;
        Location       location;
        Token*         tokens;
        Allocator*     allocator;
} Lexer;

typedef bool (*TokenMatcher)(Lexer*, Token* out);

// Single character dispatch (256 table)
static const TokenType SingleCharTable[256] = {
#define MULTI_LITERAL_DEF(...)
#define COMPLEX_TOKEN_DEF(...)
#define SINGLE_TOKEN_DEF(name, ch, printed) [(unsigned char) ch] = name,
#include "./def/tokens.def"
#undef MULTI_LITERAL_DEF
#undef COMPLEX_TOKEN_DEF
#undef SINGLE_TOKEN_DEF
};

// Multi literal tokens

static const MultiLiteral MultiLiteralTable[] = {
#define MULTI_LITERAL_DEF(name, text, printed) {text, sizeof(text) - 1, name},
#define SINGLE_TOKEN_DEF(...)
#define COMPLEX_TOKEN_DEF(...)
#include "./def/tokens.def"
#undef MULTI_LITERAL_DEF
#undef COMPLEX_TOKEN_DEF
#undef SINGLE_TOKEN_DEF
};

const usize MultiLiteralCount = ArrayCount(MultiLiteralTable);

Token NewToken(TokenType type, TokenValue value, Location loc);
Lexer NewLexer(Allocator* allocator, const char* path, StringView content);
Token lexer_chop(Lexer* lexer);
bool  Lex(Lexer* lexer);

static inline const char* TokenTypeToString(TokenType t) {
        return (t < TokensCount) ? TokenTypeName[t] : "Unknown";
}

#ifdef BSTD_IMPL
#        define FRONT_LEXER_IMPL
#endif
#ifdef FRONT_LEXER_IMPL
#        include "lexer.c"
#endif

#endif
