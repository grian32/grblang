#ifndef GRBLANG_LEXER_H
#define GRBLANG_LEXER_H

#include <stddef.h>

typedef struct {
    const char* src;
    size_t pos;
    char current;
} Lexer;

typedef enum {
    TOK_UNKNOWN, // +
    TOK_PLUS, // +
    TOK_MINUS, // -
    TOK_MULT, // *
    TOK_DIV, // /
    TOK_INT,
    TOK_EOF // end of file
} TokenType;


const char* token_type_string(TokenType tt);

typedef struct {
    TokenType type;
    union {
        int int_val;
        // TODO: more as needed
    } value;
} Token;

void lex_advance(Lexer* l);
void lexer_init(Lexer* l, const char* src);
void lex_skip_whitespace(Lexer* l);
int lex_parse_int(Lexer* l);
Token lex_next(Lexer* l);

#endif //GRBLANG_LEXER_H
