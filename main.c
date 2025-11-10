#include <stdbool.h>
#include <stdio.h>

#include "bytecode_emit.h"
#include "lexer.h"
#include "parser.h"
#include "resolver.h"
#include "stack.h"
#include "type_checker.h"
#include "util.h"
#include "vm.h"

void print_lexer(Lexer* l) {
    Token next;
    while ((next=lex_next(l)).type != TOK_EOF) {
        char buffer[50];
        token_string(next, buffer);
        printf("%s ", buffer);
    }

    char buffer[50];
    token_string(next, buffer);
    printf("%s\n", buffer);
}

int main(void) {
    long src_len;
    char* src = read_file("./test.grb", &src_len);
    printf("from: %s\n", src);

    Lexer l;
    lexer_init(&l, src);
    Parser p;
    parser_init(&p, &l);

    ASTNode* node = parse_program(&p);
    Resolver r;
    resolver_init(&r);
    resolve(node, &r);

    print_ast(node, 0, true);

    type_check(node, &r);

    // BytecodeEmitter b;
    // bytecode_init(&b);
    // bytecode_gen(node, &b);

    // VM vm;
    // vm_init(&vm, &b, r.count);
    // vm_run(&vm);

    // char buffer[50];
    // stack_value_string(vm.stack.data[vm.stack.top], buffer);
    // printf("\nvm result: %s\n", buffer);

    return 0;
}
