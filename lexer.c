#include "lexer.h"

#define IS_DIGIT(c) c >= '0' && c <= '9'

const char* token_type_string(TokenType tt) {
    switch (tt) {
        case TOK_PLUS:
            return "+";
        case TOK_MINUS:
            return "-";
        case TOK_MULT:
            return "*";
        case TOK_DIV:
            return "/";
        case TOK_EOF:
            return "EOF";
        default:
            return "UNKNOWN";
    }
}

void lex_advance(Lexer* lex) {
    lex->pos++;
    lex->current = lex->src[lex->pos];
}

void lexer_init(Lexer* l, const char* src) {
    l->src = src;
    l->pos = 0;
    l->current = src[0];
}

void lex_skip_whitespace(Lexer* l) {
    while (l->current == ' ' || l->current == '\t' || l->current == '\n' || l->current == '\r') {
        lex_advance(l);
    }
}

int lex_parse_int(Lexer* l) {
    char* 
    return -1;
}

Token lex_next(Lexer* l) {
    Token t;
    lex_skip_whitespace(l);

    switch (l->current) {
        case '+':
            t.type = TOK_PLUS;
            break;
        case '-':
            t.type = TOK_MINUS;
            break;
        case '*':
            t.type = TOK_MULT;
            break;
        case '/':
            t.type = TOK_DIV;
            break;
        case '\0':
            t.type = TOK_EOF;
            break;
        default:
            if (IS_DIGIT(l->current)) {
                t.type = TOK_INT;
                t.value.int_val = lex_parse_int(l);
            }
            t.type = TOK_UNKNOWN;
            break;
    }

    lex_advance(l);
    return t;
}