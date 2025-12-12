#include "lexer.h"

static inline const char* TokenTypeToString(TokenType t) {
        return (t >= 0 && t < TokensCount) ? TokenTypeName[t] : "Unknown";
}

Token NewToken(const TokenType  type,
               const TokenValue value,
               const Location   location) {
        return (Token){
            .type     = type,
            .val      = value,
            .location = location,
        };
}

Location NewLocation(const char* path, const usize col, const usize row) {
        return (Location){
            .file_path = path,
            .col       = col,
            .row       = row,
        };
}

Lexer NewLexer(Allocator* alloc, const char* path, const StringView content) {
        Token* tokens = arr_new(alloc, Token);
        return (Lexer){
            .location  = NewLocation(path, 1, 1),
            .content   = content,
            .allocator = alloc,
            .tokens    = tokens,
        };
}

void lexer_advance_location(Lexer* lexer, StringView sv) {
        for (usize i = 0; i < sv.len; i++) {
                lexer->location.col += 1;
                if (sv.data[i] == '\n') {
                        lexer->location.row += 1;
                        lexer->location.col = 1;
                }
        }
}

StringView lexer_chop_prefix(Lexer* lexer, StringView prefix) {
        StringView sv = SV_ChopPrefix(&lexer->content, prefix);
        lexer_advance_location(lexer, sv);
        return sv;
}

StringView lexer_chop_while(Lexer* lexer, StringViewPredicate predicate) {
        StringView sv = SV_ChopWhile(&lexer->content, predicate);
        lexer_advance_location(lexer, sv);
        return sv;
}

bool SV_IsSymbolic(const char c) {
        return c == '_' || (bool)isalnum(c);
}

bool SV_IsQuotePredicate(const char c) {
        return c == '"';
}

bool SV_IsNotQuotePredicate(const char c) {
        return c != '"';
}

Token lexer_chop(Lexer* lexer) {
        Token result =
            NewToken(TOKENTYPE_EOF, (TokenValue){0}, lexer->location);
        (void)lexer_chop_while(lexer, SV_IsSpacePredicate);

        if (lexer->content.len == 0) {
                return result;
        }

        Location loc = lexer->location;

        for (usize i = 0; i < TokensCount; i++) {
                StringView lit = NewStringView(TokenMatchTable[i].lit_data,
                                               TokenMatchTable[i].lit_len);
                if (lit.len > 0 && SV_HasPrefix(lexer->content, lit)) {
                        StringView chopped = lexer_chop_prefix(lexer, lit);
                        TokenValue val     = {.symbol = chopped};
                        return NewToken(TokenMatchTable[i].type, val, loc);
                }
        }

        char c0 = SV_Front(lexer->content);
        if (SV_IsAlphaPredicate(c0)) {
                StringView id = lexer_chop_while(lexer, SV_IsSymbolic);
                TokenValue v  = {.symbol = id};
                return NewToken(TOKENTYPE_IDENTIFIER, v, loc);
        }

        if (SV_IsDigitPredicate(c0)) {
                StringView id = lexer_chop_while(lexer, SV_IsDigitPredicate);
                TokenValue v  = {.symbol = id};
                return NewToken(TOKENTYPE_NUMBER, v, loc);
        }

        if (SV_IsQuotePredicate(c0)) {
                (void)lexer_chop_prefix(lexer, SV("\""));  // consume quote
                StringView id = lexer_chop_while(lexer, SV_IsNotQuotePredicate);
                (void)lexer_chop_prefix(lexer, SV("\""));  // consume quote
                TokenValue v = {.symbol = id};
                return NewToken(TOKENTYPE_STRING, v, loc);
        }

        printf("%c", SV_Front(lexer->content));
        unreachable;
        return NewToken(
            TOKENTYPE_EOF, (TokenValue){.symbol = lexer->content}, loc);
}

bool Lex(Lexer* lexer) {
        Token tok = lexer_chop(lexer);
        arr_append(lexer->tokens, tok);

        while (tok.type != TOKENTYPE_EOF) {
                tok = lexer_chop(lexer);
                arr_append(lexer->tokens, tok);
        }

        return true;
}
