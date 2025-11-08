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
    TOK_ASSIGN, // =
    TOK_SEMICOLON, // ;
    TOK_EXCLAM, // !
    TOK_LESS, // <
    TOK_GREATER, // >
    TOK_COLON, // :
    TOK_LBRACE, // {
    TOK_RBRACE, // }
    TOK_IDENT,
    TOK_EQUALS, // ==
    TOK_NOT_EQUALS, // !=
    TOK_VAR, // var
    TOK_TRUE, // true
    TOK_FALSE, // false
    TOK_INT,
    TOK_INT_TYPE,
    TOK_BOOL_TYPE,
    TOK_IF,
    TOK_ELSE,
    TOK_EOF
} TokenType;

typedef struct {
    TokenType type;
    const char* start_literal;
    int length;
    union {
        int int_val;
        char* ident_val;
        // TODO: more as needed
    } value;
} Token;

void token_string(Token t, char buffer[50]);

void lex_advance(Lexer* l);
void lexer_init(Lexer* l, const char* src);
void lex_skip_whitespace(Lexer* l);
int lex_parse_int(Lexer* l, const char** start_out, int* len_out);
// returns token type, if TOK_IDENT is returned, then ident_out has been set to the identifier, otherwise it has found a keyword and has not set ident_out. start_out and len_out are set in both cases
TokenType lex_parse_ident(Lexer* l, char** ident_out, const char** start_out, int* len_out);

Token lex_next(Lexer* l);

#endif //GRBLANG_LEXER_H
