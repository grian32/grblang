#include "lexer.h"
#include "util.h"

int main(void) {
    long src_len;
    char* src = read_file("../test.grb", &src_len);
    printf("from: %s\n", src);
    Lexer l;
    lexer_init(&l, src);
    Token *tokens = (Token *) malloc(100 * sizeof(Token));

    for (int i = 0; i <= src_len; i++) {
        tokens[i] = lex_next(&l);
    }

    for (int i = 0; i <= src_len; i++) {
        printf("%d: %s ", i, token_type_string(tokens[i].type));
    }

    free(tokens);
    return 0;
}