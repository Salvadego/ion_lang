#include "lexer.h"

#include <stdio.h>

#include "core/utils.h"
#include "string/string_builder.h"

enum CharClass {
        CLASS_OTHER = 0,
        CLASS_DIGIT,
        CLASS_HEX,
        CLASS_DOT,
        CLASS_E,
        CLASS_SIGN,
        CLASS_UNDERSCORE,
        CLASS_S_HEX,
        CLASS_S_OCT,
        CLASS_LAST
};

static inline enum CharClass classify(char c) {
        if (c == 'x' || c == 'X') return CLASS_S_HEX;
        if (c == 'o' || c == 'O') return CLASS_S_OCT;
        if (c == '_') return CLASS_UNDERSCORE;
        if (c == '.') return CLASS_DOT;
        if (c == 'e' || c == 'E') return CLASS_E;
        if (c == '+' || c == '-') return CLASS_SIGN;
        if (c >= '0' && c <= '9') return CLASS_DIGIT;
        if ((c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')) return CLASS_HEX;
        return CLASS_OTHER;
}

typedef enum {
        SERR = 0,
        S_START,
        S_DOT,
        S_EXPSIGN,
        S_EXP,
        S_INT,
        S_EXPDIG,
        S_FRAC,
        S_ZERO,
        S_HEX,
        S_OCT,
        S_LAST
} TOKEN_STATE_NUMBER;

// transitions[state][charclass]
static const TOKEN_STATE_NUMBER NUMBER_DFA[S_LAST][CLASS_LAST] = {
    /*                O         D      H      .      E          S         _
       S_H S_O*/
    [SERR]    = {SERR,     SERR,  SERR,  SERR,  SERR,      SERR,     SERR,  SERR,  SERR},
    [S_START] = {SERR,    S_INT,  SERR,  SERR,  SERR,      SERR,     SERR,  SERR,  SERR},
    [S_INT]   = {SERR,    S_INT,  SERR, S_DOT, S_EXP,      SERR,    S_INT,  SERR,  SERR},
    [S_ZERO]  = {SERR,    S_INT,  SERR, S_DOT, S_EXP,      SERR,    S_INT, S_HEX, S_OCT},
    [S_DOT]   = {SERR,   S_FRAC,  SERR,  SERR,  SERR,      SERR,     SERR,  SERR,  SERR},
    [S_FRAC]  = {SERR,   S_FRAC,  SERR,  SERR, S_EXP,      SERR,   S_FRAC,  SERR,  SERR},
    [S_EXP]   = {SERR, S_EXPDIG,  SERR,  SERR,  SERR, S_EXPSIGN,     SERR,  SERR,  SERR},
    [S_EXPSIGN] = {SERR, S_EXPDIG,  SERR,  SERR,  SERR,      SERR,     SERR,  SERR,  SERR},
    [S_EXPDIG] = {SERR, S_EXPDIG,  SERR,  SERR,  SERR,      SERR, S_EXPDIG,  SERR,  SERR},
    [S_HEX]    = {SERR,    S_HEX, S_HEX,  SERR, S_HEX,      SERR,    S_HEX,  SERR,  SERR},
    [S_OCT]    = {SERR,    S_OCT,  SERR,  SERR,  SERR,      SERR,    S_OCT,  SERR,  SERR},
};

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
        Token*    tokens = arr_new(alloc, Token);
        ResultPtr ptr    = Allocator_AllocType(alloc, StringBuilder);
        if (ptr.is_error) Fatal(ptr.error);
        StringBuilder* sb = ptr.value;
        SB_Init(sb, alloc, Kilobytes(1));

        return (Lexer){
            .location  = NewLocation(path, 1, 1),
            .content   = content,
            .allocator = alloc,
            .tokens    = tokens,
            .temp      = sb,
        };
}

void advance_location(Lexer* lexer, StringView sv) {
        for (usize i = 0; i < sv.len; i++) {
                lexer->location.col += 1;
                if (sv.data[i] == '\n') {
                        lexer->location.row += 1;
                        lexer->location.col = 1;
                }
        }
}

StringView chop_prefix(Lexer* lexer, StringView prefix) {
        StringView sv = SV_ChopPrefix(&lexer->content, prefix);
        advance_location(lexer, sv);
        return sv;
}

StringView chop_while(Lexer* lexer, StringViewPredicate predicate) {
        StringView sv = SV_ChopWhile(&lexer->content, predicate);
        advance_location(lexer, sv);
        return sv;
}

bool SV_IsSymbolic(const char c) {
        return c == '_' || (bool)isalnum(c);
}

bool SV_IsSimpleQuote(const char c) {
        return c == '\'';
}

bool SV_IsNotSimpleQuote(const char c) {
        return c != '\'';
}

bool SV_IsDoubleQuote(const char c) {
        return c == '"';
}

bool SV_IsNotDoubleQuote(const char c) {
        return c != '"';
}

bool SV_IsNotNewLine(const char c) {
        return c != '\n';
}

bool match_IDENTIFIER(Lexer* lx, Token* out) {
        char c = SV_Front(lx->content);
        if (!SV_IsAlphaPredicate(c) && c != '_') return false;

        Location   loc  = lx->location;
        StringView sv   = chop_while(lx, SV_IsSymbolic);
        out->type       = TOKENTYPE_IDENTIFIER;
        out->val.symbol = sv;
        out->location   = loc;
        return true;
}

bool match_NUMBER(Lexer* lx, Token* out) {
        const char* p   = lx->content.data;
        const char* end = p + lx->content.len;

        if (!isdigit(*p)) return false;

        Location    loc   = lx->location;
        const char* start = p;

        TOKEN_STATE_NUMBER state = S_START;

        if (*p == '0') {
                state = S_ZERO;
                p++;
        };

        while (p < end) {
                enum CharClass     cc   = classify(*p);
                TOKEN_STATE_NUMBER next = NUMBER_DFA[state][cc];

                if (next == SERR) break;

                state = next;
                p++;
        }

        if (!(state >= S_INT)) {
                return false;
        }

        StringView sv = NewStringView(start, (u64)(p - start));
        chop_prefix(lx, sv);
        out->type       = TOKENTYPE_NUMBER;
        out->val.symbol = sv;
        out->location   = loc;
        return true;
}

bool match_STRING(Lexer* lx, Token* out) {
        if (SV_Front(lx->content) != '"') return false;
        Location loc = lx->location;
        chop_prefix(lx, SV("\""));
        SB_Clear(lx->temp);

        while (lx->content.len > 0) {
                char c = SV_Front(lx->content);
                chop_prefix(lx, NewStringView(&c, 1));
                if (c == '"') break;

                if (c == '\\') {  // escape sequence
                        char esc = SV_Front(lx->content);
                        chop_prefix(lx, NewStringView(&esc, 1));
                        switch (esc) {
                                case 'n':
                                        SB_WriteChar(lx->temp, '\n');
                                        break;
                                case 't':
                                        SB_WriteChar(lx->temp, '\t');
                                        break;
                                case '\\':
                                        SB_WriteChar(lx->temp, '\\');
                                        break;
                                case '"':
                                        SB_WriteChar(lx->temp, '"');
                                        break;
                                default:
                                        SB_WriteChar(lx->temp, esc);
                                        break;
                        }
                } else {
                        SB_WriteChar(lx->temp, c);
                }
        }

        StringView sv  = {0};
        Error      err = SB_Clone(lx->temp, &sv);
        if (isError(err)) {
                Fatal(err);
        }
        out->type       = TOKENTYPE_STRING;
        out->val.symbol = sv;
        out->location   = loc;
        return true;
}

bool match_CHAR(Lexer* lx, Token* out) {
        if (SV_Front(lx->content) != '\'') return false;

        Location loc = lx->location;

        chop_prefix(lx, SV("\'"));
        StringView body = chop_while(lx, SV_IsNotSimpleQuote);
        chop_prefix(lx, SV("\'"));

        out->type       = TOKENTYPE_CHAR;
        out->val.symbol = body;
        out->location   = loc;
        return true;
}

bool match_COMMENT(Lexer* lx, Token* out) {
        if (SV_Front(lx->content) != '/' || lx->content.len < 2) return false;
        Location loc = lx->location;

        char next = lx->content.data[1];
        if (next == '/') {  // line comment
                chop_prefix(lx, NewStringView(lx->content.data, 2));
                StringView sv   = chop_while(lx, SV_IsNotNewLine);
                out->type       = TOKENTYPE_COMMENT;
                out->val.symbol = sv;
                out->location   = loc;
                return true;
        } else if (next == '*') {  // block comment
                chop_prefix(lx, NewStringView(lx->content.data, 2));
                const char* start = lx->content.data;
                while (lx->content.len > 1) {
                        if (lx->content.data[0] == '*' &&
                            lx->content.data[1] == '/')
                                break;
                        chop_prefix(lx, NewStringView(lx->content.data, 1));
                }
                chop_prefix(lx, NewStringView("*/", 2));
                StringView sv = NewStringView(
                    start, (u64)lx->content.data - (u64)start - 2);
                out->type       = TOKENTYPE_COMMENT;
                out->val.symbol = sv;
                out->location   = loc;
                return true;
        }

        return false;
}

Token lexer_chop(Lexer* lexer) {
        Token result =
            NewToken(TOKENTYPE_EOF, (TokenValue){0}, lexer->location);
        (void)chop_while(lexer, SV_IsSpacePredicate);

        if (lexer->content.len == 0) {
                return result;
        }

        Location loc = lexer->location;

        char c = SV_Front(lexer->content);

        for (usize i = 0; i < MultiLiteralCount; i++) {
                StringView lit = NewStringView(MultiLiteralTable[i].lit,
                                               MultiLiteralTable[i].len);
                if (SV_HasPrefix(lexer->content, lit)) {
                        StringView sv = chop_prefix(lexer, lit);
                        TokenValue v  = {sv};
                        return NewToken(MultiLiteralTable[i].type, v, loc);
                }
        }

        if (isdigit(c)) {
                Token tok;
                if (match_NUMBER(lexer, &tok)) return tok;
        }

        if (isalpha(c) || c == '_') {
                Token tok;
                if (match_IDENTIFIER(lexer, &tok)) return tok;
        }

        if (c == '"') {
                Token tok;
                if (match_STRING(lexer, &tok)) return tok;
        }

        if (c == '\'') {
                Token tok;
                if (match_CHAR(lexer, &tok)) return tok;
        }

        if (c == '/') {
                Token tok;
                if (match_COMMENT(lexer, &tok)) return tok;
        }

        TokenType t = SingleCharTable[(u8)c];
        if (t != 0) {
                StringView sv = chop_prefix(lexer, NewStringView(&c, 1));
                TokenValue v  = {sv};
                return NewToken(t, v, loc);
        }

        printf("%c\n", SV_Front(lexer->content));
        unreachable;
        return NewToken(
            TOKENTYPE_EOF, (TokenValue){.symbol = lexer->content}, loc);
}

bool Lex(Lexer* lexer) {
        Token tok = lexer_chop(lexer);
        if (tok.type != TOKENTYPE_COMMENT) {
                arr_append(lexer->tokens, tok);
        }

        while (tok.type != TOKENTYPE_EOF) {
                tok = lexer_chop(lexer);
                if (tok.type == TOKENTYPE_COMMENT) {
                        continue;
                }
                arr_append(lexer->tokens, tok);
        }

        return true;
}
