#ifndef GRBLANG_LEXER_H
#define GRBLANG_LEXER_H

#include <stddef.h>

typedef struct {
    const char* src;
    size_t pos;
    char current;
} Lexer;

typedef enum {
    TOK_UNKNOWN,
    TOK_ERROR,
    TOK_PLUS, // +
    TOK_MINUS, // -
    TOK_MULT, // *
    TOK_DIV, // /
    TOK_LPAREN, // (
    TOK_RPAREN, // )
    TOK_INT,
    TOK_EOF
} TokenType;

typedef struct {
    TokenType type;
    const char* start_literal;
    int length;
    union {
        int int_val;
        // TODO: more as needed
    } value;
} Token;

void token_string(Token t, char buffer[50]);

void lex_advance(Lexer* l);
void lexer_init(Lexer* l, const char* src);
void lex_skip_whitespace(Lexer* l);
int lex_parse_int(Lexer* l, const char** start_out, int* len_out);
Token lex_next(Lexer* l);

#endif //GRBLANG_LEXER_H
