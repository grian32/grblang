#include "lexer.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#define IS_DIGIT(c) ((c) >= '0' && (c) <= '9')

void token_string(Token t, char buffer[50]) {
    switch (t.type) {
        case TOK_PLUS:
            sprintf(buffer, "PLUS(@%d)", t.length);
            break;
        case TOK_MINUS:
            sprintf(buffer, "MINUS(@%d)", t.length);
            break;
        case TOK_MULT:
            sprintf(buffer, "MULT(@%d)", t.length);
            break;
        case TOK_DIV:
            sprintf(buffer, "DIV(@%d)", t.length);
            break;
        case TOK_EOF:
            sprintf(buffer, "EOF");
            break;
        case TOK_INT:
            sprintf(buffer, "INT(=%d, @%d)", t.value.int_val, t.length);
            break;
        default:
            sprintf(buffer, "UNKNOWN");
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

int lex_parse_int(Lexer* l, const char **start_out, int* length_out) {
    const char* start = &l->current;
    char int_str[11];
    int i = 0;

    while (IS_DIGIT(l->current) && i < 10) {
        int_str[i++] = l->current;
        lex_advance(l);
    }
    int_str[i] = '\0';

    if (start_out) *start_out = start;
    if (length_out) *length_out = i;

    if (i == 0) {
        return -1;
    }

    errno = 0;
    char *end;
    long conv_int = strtol(int_str, &end, 10);

    if (conv_int > INT_MAX || errno == ERANGE) {
        return -1;
    }

    return (int) conv_int;
}

Token lex_next(Lexer* l) {
    Token t;
    lex_skip_whitespace(l);

    t.start_literal = &l->current;
    t.length = 1;

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
            t.length = 0;
            break;
        default:
            if (IS_DIGIT(l->current)) {
                t.value.int_val = lex_parse_int(l, &t.start_literal, &t.length);
                if (t.value.int_val == -1) {
                    t.type = TOK_ERROR;
                } else {
                    t.type = TOK_INT;
                }
                return t;
            }

            t.type = TOK_UNKNOWN;
            break;
    }

    lex_advance(l);
    return t;
}