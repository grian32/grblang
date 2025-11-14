#include <stdbool.h>
#include <stdio.h>
#include <string.h>

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

int main(int argc, char* argv[]) {
    long src_len;
    if (argc != 2 && argc != 3) {
        fprintf(stderr, "error: invalid number of arguments\n");
        exit(1);
    }
    bool print_debug = false;
    if (argc == 3 && strcmp(argv[2], "-p") == 0) {
        print_debug = true;
    }
    char* src = read_file(argv[1], &src_len);
    if (print_debug) {
        printf("from: %s\n", src);
    }

    Lexer l;
    lexer_init(&l, src);

    Parser p;
    parser_init(&p, &l);

    ASTNode* node = parse_program(&p);
    Resolver r;
    resolver_init(&r);
    resolve(node, &r);

    if (print_debug) {
        print_ast(node, 0, true);
    }

    type_check(node, &r);

    BytecodeEmitter b;
    bytecode_init(&b);
    bytecode_gen(node, &b, &r);

    VM vm;
    vm_init(&vm, &b, r.count);
    vm_run(&vm);

    char buffer[50];
    stack_value_string(vm.stack.data[vm.stack.top], true, buffer);
    printf("%s\n", buffer);

    return 0;
}
