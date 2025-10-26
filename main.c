#include <stdbool.h>

#include "bytecode_emit.h"
#include "lexer.h"
#include "parser.h"
#include "stack.h"
#include "util.h"
#include "vm.h"


void test_stack() {
    printf("test stack\n");

    Stack stack;
    stack_init(&stack);
    printf("pushing 44\n");
    stack_push(&stack, 44);
    printf("pushing 42\n");
    stack_push(&stack, 42);
    printf("pushing 32\n");
    stack_push(&stack, 32);
    printf("pushing 12\n");
    stack_push(&stack, 12);

    printf("out:\n");
    printf("%d\n", stack_pop(&stack));
    printf("%d\n", stack_pop(&stack));
    printf("%d\n", stack_pop(&stack));
    printf("%d\n", stack_pop(&stack));
}

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

    VM vm;
    Stack s;
    vm_init(&vm, &b);
    vm_run(&vm);

    printf("vm result: %d", vm.stack.data[vm.stack.top]);

    return 0;
}