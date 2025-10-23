#ifndef GRBLANG_LEXER_H
#define GRBLANG_LEXER_H

#include <stddef.h>

typedef struct {
    const char* src;
    size_t pos;
    char current;
} Lexer;

typedef enum {
    TOK_PLUS, // +
    TOK_MINUS, // -
    TOK_MULT, // *
    TOK_DIV, // /
    TOK_INT,
    TOK_EOF // end of file
} TokenType;

typedef struct {
    TokenType type;
    union {
        int int_val;
        // TODO: more as needed
    } value;
} Token;

void lex_advance(Lexer* lex);
void lexer_init(Lexer* lexer, const char* src);
#endif //GRBLANG_LEXER_H
