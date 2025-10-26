#include <stdbool.h>

#include "bytecode_emit.h"
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
    BytecodeEmitter b;
    bytecode_init(&b);
    bytecode_gen(node, &b);

    for (int i = 0; i < b.code_size; i++) {
        printf("%d ", b.code[i]);
    }

    return 0;
}