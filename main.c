#include <stdbool.h>

#include "lexer.h"
#include "util.h"

int main(void) {
    long src_len;
    char* src = read_file("../test.grb", &src_len);
    printf("from: %s\n", src);

    Lexer l;
    lexer_init(&l, src);

    Token *tokens = malloc(src_len * sizeof(Token));

    Token tok;
    int i = 0;
    while ((tok = lex_next(&l)).type != TOK_EOF) {
        tokens[i++] = tok;
    }

    tokens[i].type = TOK_EOF;

    i = 0;
    while (tokens[i].type != TOK_EOF) {
        char buffer[50] = {0};
        token_string(tokens[i], buffer);
        printf("%s ", buffer);
        i++;
    }
    // print eof
    char buffer[50] = {0};
    token_string(tokens[i], buffer);
    printf("%s", buffer);

    free(tokens);
    return 0;
}