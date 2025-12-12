#include "lexer.h"

enum CharClass {
        CLASS_OTHER = 0,
        CLASS_DIGIT,
        CLASS_HEX,
        CLASS_DOT,
        CLASS_E,
        CLASS_SIGN,
        CLASS_UNDERSCORE,
        CLASS_LAST
};

static inline enum CharClass classify(char c) {
        if (c == '_') return CLASS_UNDERSCORE;
        if (c == '.') return CLASS_DOT;
        if (c == 'e' || c == 'E') return CLASS_E;
        if (c == '+' || c == '-') return CLASS_SIGN;
        if (c >= '0' && c <= '9') return CLASS_DIGIT;
        if ((c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')) return CLASS_HEX;
        return CLASS_OTHER;
}

enum {
        SERR = 0,
        S_START,
        S_INT,
        S_DOT,
        S_FRAC,
        S_EXP,
        S_EXPSIGN,
        S_EXPDIG,
        S_HEX,
        S_LAST
};

// transitions[state][charclass]
static const unsigned char NUMBER_DFA[S_LAST][CLASS_LAST] = {
    /*                O         D      H      .      E          S         _*/
    [SERR]      = {SERR,     SERR,  SERR,  SERR,  SERR,      SERR,     SERR},
    [S_START]   = {SERR,    S_INT,  SERR,  SERR,  SERR,      SERR,     SERR},
    [S_INT]     = {SERR,    S_INT,  SERR, S_DOT, S_EXP,      SERR,    S_INT},
    [S_DOT]     = {SERR,   S_FRAC,  SERR,  SERR,  SERR,      SERR,     SERR},
    [S_FRAC]    = {SERR,   S_FRAC,  SERR,  SERR, S_EXP,      SERR,   S_FRAC},
    [S_EXP]     = {SERR, S_EXPDIG,  SERR,  SERR,  SERR, S_EXPSIGN,     SERR},
    [S_EXPSIGN] = {SERR, S_EXPDIG,  SERR,  SERR,  SERR,      SERR,     SERR},
    [S_EXPDIG]  = {SERR, S_EXPDIG,  SERR,  SERR,  SERR,      SERR, S_EXPDIG},
    [S_HEX]     = {SERR,    S_HEX, S_HEX,  SERR,  SERR,      SERR,    S_HEX},
};

// 12e+
// 12.34e-56

static inline bool try_hex(const char* p, const char* end, const char** out) {
        if (!(p[0] == '0' && (p[1] == 'x' || p[1] == 'X'))) return false;

        const char* q = p + 2;
        if (q >= end) return false;
        if (!isxdigit(*q)) return false;

        q++;
        while (q < end && (isxdigit(*q) || *q == '_')) q++;

        *out = q;
        return true;
}

bool match_IDENTIFIER(Lexer* lx, Token* out);
bool match_NUMBER(Lexer* lx, Token* out);
bool match_STRING(Lexer* lx, Token* out);
bool match_CHAR(Lexer* lx, Token* out);
bool match_EOF(Lexer* lx, Token* out);

// Complex matchers (number, string, ident)
static const TokenMatcher ComplexMatchers[] = {
#define MULTI_LITERAL_DEF(...)
#define SINGLE_TOKEN_DEF(...)
#define COMPLEX_TOKEN_DEF(name, printed) match_##printed,
#include "./def/tokens.def"
#undef SINGLE_TOKEN_DEF
#undef COMPLEX_TOKEN_DEF
#undef MULTI_LITERAL_DEF
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
        Token* tokens = arr_new(alloc, Token);
        return (Lexer){
            .location  = NewLocation(path, 1, 1),
            .content   = content,
            .allocator = alloc,
            .tokens    = tokens,
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

        /* Hex case first */
        const char* hexend;
        if (try_hex(p, end, &hexend)) {
                StringView sv = NewStringView(start, (u64)(hexend - start));
                chop_prefix(lx, sv);
                out->type       = TOKENTYPE_NUMBER;
                out->val.symbol = sv;
                out->location   = loc;
                return true;
        }

        /* DFA for decimal, float, exponent */
        unsigned state = S_START;

        while (p < end) {
                enum CharClass cc   = classify(*p);
                unsigned       next = NUMBER_DFA[state][cc];

                if (next == SERR) break;

                state = next;
                p++;
        }

        /* Valid end states */
        if (!(state == S_INT || state == S_FRAC || state == S_EXPDIG)) {
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
        if (SV_Front(lx->content) != '\"') return false;

        Location loc = lx->location;

        chop_prefix(lx, SV("\""));
        StringView body = chop_while(lx, SV_IsNotDoubleQuote);
        chop_prefix(lx, SV("\""));

        out->type       = TOKENTYPE_STRING;
        out->val.symbol = body;
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

bool match_EOF(Lexer* lx, Token* out) {
        if (lx->content.len != 0) return false;
        out->type       = TOKENTYPE_EOF;
        out->val.symbol = SV("");
        out->location   = lx->location;
        return true;
}

Token lexer_chop(Lexer* lexer) {
        Token result =
            NewToken(TOKENTYPE_EOF, (TokenValue){0}, lexer->location);
        (void)chop_while(lexer, SV_IsSpacePredicate);

        if (lexer->content.len == 0) {
                return result;
        }

        Location loc = lexer->location;

        char      c = SV_Front(lexer->content);
        TokenType t = SingleCharTable[(unsigned char)c];
        if (t != 0) {
                StringView sv = chop_prefix(lexer, NewStringView(&c, 1));
                TokenValue v  = {sv};
                return NewToken(t, v, loc);
        }

        for (usize i = 0; i < MultiLiteralCount; i++) {
                StringView lit = NewStringView(MultiLiteralTable[i].lit,
                                               MultiLiteralTable[i].len);
                if (SV_HasPrefix(lexer->content, lit)) {
                        StringView sv = chop_prefix(lexer, lit);
                        TokenValue v  = {sv};
                        return NewToken(MultiLiteralTable[i].type, v, loc);
                }
        }

        for (usize i = 0; i < ComplexMatcherCount; i++) {
                Token out;
                if (ComplexMatchers[i](lexer, &out)) {
                        return out;
                }
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
