#include "vm.h"

inline void vm_init(VM *vm, BytecodeEmitter *b) {
    stack_init(&vm->stack);
    vm->constants = b->constants;
    vm->code = b->code;
    vm->code_size = b->code_size;
    vm->pc = 0;
}

void vm_run(VM* vm) {
    while (vm->pc < vm->code_size) {
        uint8_t instruction = vm->code[vm->pc++];

        switch (instruction) {
            case OP_PUSH: {
                uint16_t idx = (vm->code[vm->pc] << 8) | vm->code[vm->pc + 1];
                vm->pc += 2;
                int value = vm->constants[idx];
                stack_push(&vm->stack, value);
                break;
            }
            case OP_ADD: {
                int b = stack_pop(&vm->stack);
                int a = stack_pop(&vm->stack);
                stack_push(&vm->stack, a + b);
                break;
            }
            case OP_SUB: {
                int b = stack_pop(&vm->stack);
                int a = stack_pop(&vm->stack);
                stack_push(&vm->stack, a - b);
                break;
            }
            case OP_MUL: {
                int b = stack_pop(&vm->stack);
                int a = stack_pop(&vm->stack);
                stack_push(&vm->stack, a * b);
                break;
            }
            case OP_DIV: {
                int b = stack_pop(&vm->stack);
                int a = stack_pop(&vm->stack);
                stack_push(&vm->stack, a / b);
                break;
            }
            case OP_NEG: {
                int a = stack_pop(&vm->stack);
                stack_push(&vm->stack, -a);
                break;
            }

            default:
                fprintf(stderr, "unknown opcode: %d\n", instruction);
                exit(1);
        }
    }
}