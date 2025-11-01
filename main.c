#include <stdbool.h>

#include "bytecode_emit.h"
#include "lexer.h"
#include "parser.h"
#include "resolver.h"
#include "stack.h"
#include "util.h"
#include "vm.h"

int main(void) {
    long src_len;
    char* src = read_file("../test.grb", &src_len);
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

    BytecodeEmitter b;
    bytecode_init(&b);
    bytecode_gen(node, &b);

    VM vm;
    vm_init(&vm, &b, r.count);
    vm_run(&vm);

    char buffer[50];
    stack_value_string(vm.stack.data[vm.stack.top], buffer);
    printf("vm result: %s", buffer);

    return 0;
}