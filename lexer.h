#ifndef GRBLANG_LEXER_H
#define GRBLANG_LEXER_H

#include <stddef.h>

typedef struct {
    const char* src;
    size_t pos;
    char current;
} Lexer;

typedef enum {
    TOK_UNKNOWN, // 0
    TOK_ERROR, // 1
    TOK_PLUS, // + // 2
    TOK_PLUS_EQUALS, // += // 3
    TOK_MINUS, // - // 4
    TOK_MINUS_EQUALS, // -= // 5
    TOK_MULT, // * // 6
    TOK_MULT_EQUALS, // *= // 7
    TOK_DIV, // / // 8
    TOK_DIV_EQUALS, // /= // 9
    TOK_LPAREN, // ( // 10
    TOK_RPAREN, // ) // 11
    TOK_ASSIGN, // = // 12
    TOK_SEMICOLON, // ; // 13
    TOK_EXCLAM, // ! // 14
    TOK_LESS, // < // 15
    TOK_GREATER, // > // 16
    TOK_COLON, // : // 17
    TOK_LBRACE, // { // 18
    TOK_RBRACE, // } // 19
    TOK_MODULO, // % // 20
    TOK_AND, // && // 21
    TOK_OR, // || // 22
    TOK_EQUALS, // == // 23
    TOK_NOT_EQUALS, // != // 24
    TOK_LESS_EQUALS, // <= // 25
    TOK_GREATER_EQUALS, // >= // 26
    TOK_VAR, // var // 27
    TOK_TRUE, // true // 28
    TOK_FALSE, // false // 29
    TOK_WHILE, // while // 30
    TOK_IF, // if // 31
    TOK_ELSE, // else // 32
    TOK_IDENT, // 33
    TOK_STRING, // 34
    TOK_INT, // 35
    TOK_TYPE, // 36
    TOK_EOF // 37
} TokenType;

typedef enum {
    DATA_INT,
    DATA_BOOL,
    DATA_STRING,
} DataType;

typedef struct {
    TokenType type;
    const char* start_literal;
    int length;
    union {
        int int_val;
        char* ident_val;
        char* string_val;
        DataType type_val;
        // TODO: more as needed
    } value;
} Token;

void token_string(Token t, char buffer[50]);

void lex_advance(Lexer* l);
void lexer_init(Lexer* l, const char* src);
void lex_skip_whitespace(Lexer* l);
int lex_parse_int(Lexer* l, const char** start_out, int* len_out);
// returns token type, if TOK_IDENT is returned, then ident_out has been set to the identifier, otherwise it has found a keyword and has not set ident_out. start_out and len_out are set in both cases
TokenType lex_parse_ident(Lexer* l, char** ident_out, const char** start_out, int* len_out, DataType* type_out);
void lex_parse_string(Lexer* l, char** str_out, const char** start_out, int* len_out);

Token lex_next(Lexer* l);

#endif //GRBLANG_LEXER_H
