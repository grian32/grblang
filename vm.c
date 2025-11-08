#include "vm.h"
#include "bytecode_emit.h"
#include "stack.h"

#include <stdio.h>
#include <stdlib.h>

inline void vm_init(VM *vm, BytecodeEmitter *b, int num_locals) {
    stack_init(&vm->stack);
    vm->constants = b->constants;
    vm->code = b->code;
    vm->code_size = b->code_size;

    vm->locals_size = num_locals;
    vm->locals = malloc(num_locals * sizeof(StackValue));

    vm->pc = 0;
}

void vm_run(VM* vm) {
    while (vm->pc < vm->code_size) {
        uint8_t instruction = vm->code[vm->pc++];

        switch (instruction) {
            case OP_PUSH: {
                uint16_t idx = (vm->code[vm->pc] << 8) | vm->code[vm->pc + 1];
                vm->pc += 2;
                StackValue value = vm->constants[idx];
                stack_push(&vm->stack, value);
                break;
            }
            case OP_PUSH_TRUE: {
                StackValue sv = {.type = VALUE_BOOL, .bool_val = true};
                stack_push(&vm->stack, sv);
                break;
            }
            case OP_PUSH_FALSE: {
                StackValue sv = {.type = VALUE_BOOL, .bool_val = false};
                stack_push(&vm->stack, sv);
                break;
            }
            case OP_IADD: {
                StackValue b = stack_pop(&vm->stack);
                StackValue a = stack_pop(&vm->stack);
                StackValue sv = {.type = VALUE_INT, .int_val = a.int_val + b.int_val};
                stack_push(&vm->stack, sv);
                break;
            }
            case OP_ISUB: {
                StackValue b = stack_pop(&vm->stack);
                StackValue a = stack_pop(&vm->stack);
                StackValue sv = {.type = VALUE_INT, .int_val = a.int_val - b.int_val};
                stack_push(&vm->stack, sv);
                break;
            }
            case OP_IMUL: {
                StackValue b = stack_pop(&vm->stack);
                StackValue a = stack_pop(&vm->stack);
                StackValue sv = {.type = VALUE_INT, .int_val = a.int_val * b.int_val};
                stack_push(&vm->stack, sv);
                break;
            }
            case OP_IDIV: {
                StackValue b = stack_pop(&vm->stack);
                StackValue a = stack_pop(&vm->stack);
                if (b.int_val == 0) {
                    fprintf(stderr, "error: division by 0 not allowed\n");
                    exit(1);
                }
                StackValue sv = {.type = VALUE_INT, .int_val = a.int_val / b.int_val};
                stack_push(&vm->stack, sv);
                break;
            }
            case OP_IGT: {
                StackValue b = stack_pop(&vm->stack);
                StackValue a = stack_pop(&vm->stack);
                StackValue sv = {.type = VALUE_BOOL, .bool_val = a.int_val > b.int_val};
                stack_push(&vm->stack, sv);
                break;
            }
            case OP_ILT: {
                StackValue b = stack_pop(&vm->stack);
                StackValue a = stack_pop(&vm->stack);
                StackValue sv = {.type = VALUE_BOOL, .bool_val = a.int_val < b.int_val};
                stack_push(&vm->stack, sv);
                break;
            }
            case OP_IEQ: {
                StackValue b = stack_pop(&vm->stack);
                StackValue a = stack_pop(&vm->stack);
                StackValue sv = {.type = VALUE_BOOL, .bool_val = a.int_val == b.int_val};
                stack_push(&vm->stack, sv);
                break;
            }
            case OP_BEQ: {
                StackValue b = stack_pop(&vm->stack);
                StackValue a = stack_pop(&vm->stack);
                StackValue sv = {.type = VALUE_BOOL, .bool_val = a.bool_val == b.bool_val};
                stack_push(&vm->stack, sv);
                break;
            }
            case OP_INEQ: {
                StackValue b = stack_pop(&vm->stack);
                StackValue a = stack_pop(&vm->stack);
                StackValue sv = {.type = VALUE_BOOL, .bool_val = a.int_val != b.int_val};
                stack_push(&vm->stack, sv);
                break;
            }
            case OP_BNEQ: {
                StackValue b = stack_pop(&vm->stack);
                StackValue a = stack_pop(&vm->stack);
                StackValue sv = {.type = VALUE_BOOL, .bool_val = a.bool_val != b.bool_val};
                stack_push(&vm->stack, sv);
                break;
            }
            case OP_INEG: {
                StackValue a = stack_pop(&vm->stack);
                StackValue sv = {.type = VALUE_INT, .int_val = -a.int_val};
                stack_push(&vm->stack, sv);
                break;
            }
            case OP_NOT:
                StackValue a = stack_pop(&vm->stack);
                StackValue sv = {.type = VALUE_BOOL, .bool_val = !a.bool_val};
                stack_push(&vm->stack, sv);
                break;
            case OP_BLOAD:
            case OP_ILOAD: {
                int slot = (vm->code[vm->pc] << 8) | vm->code[vm->pc + 1];
                vm->pc += 2;
                if (slot >= vm->locals_size) {
                    fprintf(stderr, "invalid slot `%d` for locals\n", slot);
                    exit(1);
                }
                stack_push(&vm->stack, vm->locals[slot]);
                break;
            }
            case OP_BSTORE:
            case OP_ISTORE: {
                int slot = (vm->code[vm->pc] << 8) | vm->code[vm->pc + 1];
                vm->pc += 2;
                if (slot >= vm->locals_size) {
                    fprintf(stderr, "invalid slot `%d` for locals\n", slot);
                    exit(1);
                }
                vm->locals[slot] = stack_pop(&vm->stack);
                break;
            }
            case OP_JMP: {
                int steps = (vm->code[vm->pc] << 8 | vm->code[vm->pc + 1]);
                vm->pc += steps + 2; // steps and then the 2 for the actual step amount
                break;
            }
            case OP_JMPN: {
                int steps = (vm->code[vm->pc] << 8 | vm->code[vm->pc + 1]);
                vm->pc += 2;
                StackValue sv = stack_pop(&vm->stack);
                if (!sv.bool_val) {
                    vm->pc += steps;
                }
                break;
            }
            default:
                fprintf(stderr, "unknown opcode: %d\n", instruction);
                exit(1);
        }
    }
}
