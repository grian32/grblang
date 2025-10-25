#include <stdbool.h>

#include "lexer.h"
#include "parser.h"
#include "util.h"

int main(void) {
    long src_len;
    char* src = read_file("../test.grb", &src_len);
    printf("from: %s\n", src);

    Lexer l;
    lexer_init(&l, src);
    Parser p;
    parser_init(&p, &l);

    ASTNode* node = parse_expr(&p);

    print_ast(node, 0);

    return 0;
}