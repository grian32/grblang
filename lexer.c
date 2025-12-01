#include "lexer.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IS_DIGIT(c) ((c) >= '0' && (c) <= '9')
#define IS_ALPHA(c) (((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z') || ((c) == '_'))

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
        case TOK_EXCLAM:
            sprintf(buffer, "EXCLAM(@%d)", t.length);
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
        case TOK_STRING:
            sprintf(buffer, "STRING(=%s, @%d)", t.value.string_val, t.length);
            break;
        case TOK_LPAREN:
            sprintf(buffer, "LPAREN(@%d)", t.length);
            break;
        case TOK_ASSIGN:
            sprintf(buffer, "ASSIGN(@%d)", t.length);
            break;
        case TOK_VAR:
            sprintf(buffer, "VAR(@%d)", t.length);
            break;
        case TOK_TRUE:
            sprintf(buffer, "TRUE(@%d)", t.length);
            break;
        case TOK_FALSE:
            sprintf(buffer, "FALSE(@%d)", t.length);
            break;
        case TOK_IDENT:
            sprintf(buffer, "IDENT(=%s, @%d)", t.value.ident_val, t.length);
            break;
        case TOK_RPAREN:
            sprintf(buffer, "RPAREN(@%d)", t.length);
            break;
        case TOK_SEMICOLON:
            sprintf(buffer, "SEMICOLON(@%d)", t.length);
            break;
        case TOK_LESS:
            sprintf(buffer, "LESS(@%d)", t.length);
            break;
        case TOK_GREATER:
            sprintf(buffer, "GREATER(@%d)", t.length);
            break;
        case TOK_EQUALS:
            sprintf(buffer, "EQUALS(@%d)", t.length);
            break;
        case TOK_LBRACE:
            sprintf(buffer, "LBRACE(@%d)", t.length);
            break;
        case TOK_RBRACE:
            sprintf(buffer, "RBRACE(@%d)", t.length);
            break;
        case TOK_COLON:
            sprintf(buffer, "COLON(@%d)", t.length);
            break;
        case TOK_TYPE:
            sprintf(buffer, "TYPE(&%d@%d)", t.value.type_val, t.length);
            break;
        case TOK_IF:
            sprintf(buffer, "IF(@%d)", t.length);
            break;
        case TOK_ELSE:
            sprintf(buffer, "ELSE(@%d)", t.length);
            break;
        default:
            sprintf(buffer, "UNKNOWN");
    }
}

void lex_advance(Lexer* lex) {
    if (lex->pos < strlen(lex->src)) {
        lex->pos++;
        lex->current = lex->src[lex->pos];
    } else {
        lex->current = '\0';
    }
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

void lex_parse_string(Lexer* l, char** str_out, const char** start_out, int* len_out) {
    lex_advance(l);
    const char* start = &l->current;
    int curr_capacity = 256;
    char* str = malloc(sizeof(char) * curr_capacity);
    int i = 0;

    while (l->current != '"') {
        if (i >= curr_capacity) {
            curr_capacity *= 2;
            char* new_str = realloc(str, sizeof(char) * curr_capacity);
            if (!new_str) {
                fprintf(stderr, "failed to realloc str while parsing str\n");
                exit(1);
            }
            str = new_str;
        }
        if (l->current == '\\') {
            lex_advance(l);
            char esc = l->current;
            char out;
            switch (esc) {
                case 'n': out = '\n'; break;
                case 't': out = '\t'; break;
                case 'r': out = '\r'; break;
                case '\\': out = '\\'; break;
                case '"': out = '"'; break;
                case '0': out = '\0'; break;
                default: out = esc; break;
            }
            str[i++] = out;
            lex_advance(l);
        } else {
            str[i++] = l->current;
            lex_advance(l);
        }
    }
    str[i] = '\0';

    if (l->current != '"') {
        fprintf(stderr, "expected \" after string\n");
        exit(1);
    }

    lex_advance(l);
    *str_out = str;
    *start_out = start;
    *len_out = i;
}

TokenType lex_parse_ident(Lexer* l, char** ident_out, const char** start_out, int* len_out, DataType* type_out) {
    const char* start = &l->current;
    char ident_str[256];
    int i = 0;

    while (IS_ALPHA(l->current)) {
        if (i >= 255) { // previous condition was i < 256 in while; this preserves the same behaviour
            fprintf(stderr, "error: identifiers not allowed to be longer than 256");
            exit(1);
        }
        ident_str[i++] = l->current;
        lex_advance(l);
    }
    ident_str[i] = '\0';

    if (start_out) *start_out = start;
    if (len_out) *len_out = i;

    switch (strlen(ident_str)) {
        case 2:
            if (strcmp(ident_str, "if") == 0) {
                return TOK_IF;
            }
            if (strcmp(ident_str, "fn") == 0) {
                return TOK_FN;
            }
        case 3:
            if (strcmp(ident_str, "var") == 0) {
                return TOK_VAR;
            }

            if (strcmp(ident_str, "int") == 0) {
                *type_out = DATA_INT;
                return TOK_TYPE;
            }
            break;
        case 4:
            if (strcmp(ident_str, "true") == 0) {
                return TOK_TRUE;
            }

            if (strcmp(ident_str, "bool") == 0) {
                *type_out = DATA_BOOL;
                return TOK_TYPE;
            }

            if (strcmp(ident_str, "else") == 0) {
                return TOK_ELSE;
            }
            break;
        case 5:
            if (strcmp(ident_str, "false") == 0) {
                return TOK_FALSE;
            }
            if (strcmp(ident_str, "while") == 0) {
                return TOK_WHILE;
            }
            break;
        case 6:
            if (strcmp(ident_str, "return") == 0) {
                return TOK_RETURN;
            }
            if (strcmp(ident_str, "string") == 0) {
                *type_out = DATA_STRING;
                return TOK_TYPE;
            }
            break;

        default: break;
    }

    if (ident_out) *ident_out = strdup(ident_str);
    return TOK_IDENT;
}

Token lex_next(Lexer* l) {
    Token t;
    lex_skip_whitespace(l);

    t.start_literal = &l->current;
    t.length = 1;

    switch (l->current) {
        // TODO: abstract out x token plus equals = other token into a function
        case '+':
            t.type = TOK_PLUS;
            lex_advance(l);

            if (l->current == '=') {
                t.type = TOK_PLUS_EQUALS;
                t.length++;
                lex_advance(l);
            }

            return t;
        case '-':
            t.type = TOK_MINUS;
            lex_advance(l);

            if (l->current == '=') {
                t.type = TOK_MINUS_EQUALS;
                t.length++;
                lex_advance(l);
            }

            return t;
        case '*':
            t.type = TOK_MULT;
            lex_advance(l);

            if (l->current == '=') {
                t.type = TOK_MULT_EQUALS;
                t.length++;
                lex_advance(l);
            }

            return t;
        case '/':
            t.type = TOK_DIV;
            lex_advance(l);

            if (l->current == '=') {
                t.type = TOK_DIV_EQUALS;
                t.length++;
                lex_advance(l);
            }

            return t;
        case '(':
            t.type = TOK_LPAREN;
            break;
        case ')':
            t.type = TOK_RPAREN;
            break;
        case '=':
            t.type = TOK_ASSIGN;
            lex_advance(l);

            if (l->current == '=') {
                t.type = TOK_EQUALS;
                t.length++;
                lex_advance(l);
            }

            return t;
        case ';':
            t.type = TOK_SEMICOLON;
            break;
        case '!':
            t.type = TOK_EXCLAM;
            lex_advance(l);

            if (l->current == '=') {
                t.type = TOK_NOT_EQUALS;
                t.length++;
                lex_advance(l);
            }

            return t;
        case '<':
            t.type = TOK_LESS;
            lex_advance(l);

            if (l->current == '=') {
                t.type = TOK_LESS_EQUALS;
                t.length++;
                lex_advance(l);
            }

            return t;
        case '>':
            t.type = TOK_GREATER;
            lex_advance(l);

            if (l->current == '=') {
                t.type = TOK_GREATER_EQUALS;
                t.length++;
                lex_advance(l);
            }

            return t;
        case ':':
            t.type = TOK_COLON;
            break;
        case '{':
            t.type = TOK_LBRACE;
            break;
        case '}':
            t.type = TOK_RBRACE;
            break;
        case '[':
            t.type = TOK_LBRACKET;
            break;
        case ']':
            t.type = TOK_RBRACKET;
            break;
        case ',':
            t.type = TOK_COMMA;
            break;
        case '%':
            t.type = TOK_MODULO;
            break;
        case '&':
            lex_advance(l);
            if (l->current != '&') {
                fprintf(stderr, "error: expected second & after &");
                exit(1);
            }
            t.type = TOK_AND;
            t.length++;
            lex_advance(l);

            return t;
        case '|':
            lex_advance(l);
            if (l->current != '|') {
                fprintf(stderr, "error: expected second | after |");
                exit(1);
            }
            t.type = TOK_OR;
            t.length++;
            lex_advance(l);

            return t;
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
            if (IS_ALPHA(l->current)) {
                t.type = lex_parse_ident(l, &t.value.ident_val, &t.start_literal, &t.length, &t.value.type_val);
                return t;
            }
            if (l->current == '"') {
                t.type = TOK_STRING;
                lex_parse_string(l, &t.value.string_val, &t.start_literal, &t.length);
                return t;
            }

            t.type = TOK_UNKNOWN;
            break;
    }

    lex_advance(l);
    return t;
}
