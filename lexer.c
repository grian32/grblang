#include "lexer.h"

void lex_advance(Lexer* lex) {
    lex->pos++;
    lex->current = lex->src[lex->pos];
}

void lexer_init(Lexer* lexer, const char* src) {
    lexer->src = src;
    lexer->pos = 0;
    lexer->current = src[0];
}
