#include "vm.h"
#include "bytecode_emit.h"
#include "lexer.h"
#include "parser.h"
#include "stack.h"

#include <alloca.h>
#include <locale.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

void vm_init(VM *vm, BytecodeEmitter *b, int num_locals) {
    stack_init(&vm->stack);
    vm->constants = b->constants;
    vm->constants_size = b->const_count;
    vm->code = b->code;
    vm->code_size = b->code_size;

    vm->locals_size = num_locals;
    vm->locals = malloc(num_locals * sizeof(StackValue));

    vm->pc = 0;
}

void vm_run(VM* vm) {
    VarType int_type = {.base_type = VALUE_INT, .nested = -1};
    VarType bool_type = {.base_type = VALUE_BOOL, .nested = -1};
    VarType string_type = {.base_type = VALUE_STRING, .nested = -1};
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
            case OP_PUSH_STRING: {
                uint16_t idx = (vm->code[vm->pc] << 8) | vm->code[vm->pc + 1];
                vm->pc += 2;
                StackValue value = vm->constants[idx];
                stack_push(&vm->stack, value);
                break;
            }
            case OP_PUSH_TRUE: {
                StackValue sv = {.type = bool_type, .bool_val = true};
                stack_push(&vm->stack, sv);
                break;
            }
            case OP_PUSH_FALSE: {
                StackValue sv = {.type = bool_type, .bool_val = false};
                stack_push(&vm->stack, sv);
                break;
            }
            case OP_PUSH_ARRAY: {
                int len = (vm->code[vm->pc] << 8) | vm->code[vm->pc + 1];
                vm->pc += 2;

                ArrayValue* arrv = malloc(sizeof(ArrayValue));
                arrv->len = len;
                arrv->ref_count = 0;
                arrv->arr_val = malloc(sizeof(StackValue) * len);

                for (int i = len - 1; i >= 0; i--) {
                    StackValue val = stack_pop(&vm->stack);
                    arrv->arr_val[i] = val;
                }

                VarType arr_type = {.base_type = VALUE_UNKNOWN, .nested = -1};
                if (arrv->len > 0) {
                    arr_type = arrv->arr_val[0].type;
                    arr_type.nested++;
                }

                StackValue sv = {.type = arr_type, .array_val = arrv};

                stack_push(&vm->stack, sv);
                break;
            }
            case OP_IADD: {
                StackValue b = stack_pop(&vm->stack);
                StackValue a = stack_pop(&vm->stack);
                StackValue sv = {.type = int_type, .int_val = a.int_val + b.int_val};
                stack_push(&vm->stack, sv);
                break;
            }
            case OP_ISUB: {
                StackValue b = stack_pop(&vm->stack);
                StackValue a = stack_pop(&vm->stack);
                StackValue sv = {.type = int_type, .int_val = a.int_val - b.int_val};
                stack_push(&vm->stack, sv);
                break;
            }
            case OP_IMUL: {
                StackValue b = stack_pop(&vm->stack);
                StackValue a = stack_pop(&vm->stack);
                StackValue sv = {.type = int_type, .int_val = a.int_val * b.int_val};
                stack_push(&vm->stack, sv);
                break;
            }
            case OP_IDIV: {
                StackValue b = stack_pop(&vm->stack);
                StackValue a = stack_pop(&vm->stack);
                if (b.int_val == 0) {
                    fprintf(stderr, "runtime error: division by 0 not allowed\n");
                    exit(1);
                }
                StackValue sv = {.type = int_type, .int_val = a.int_val / b.int_val};
                stack_push(&vm->stack, sv);
                break;
            }
            case OP_IADDSTORE: {
                StackValue value = stack_pop(&vm->stack);
                int slot = (vm->code[vm->pc] << 8) | vm->code[vm->pc + 1];
                vm->pc += 2;

                if (slot >= vm->locals_size) {
                    fprintf(stderr, "invalid slot `%d` for locals\n", slot);
                    exit(1);
                }

                vm->locals[slot].int_val += value.int_val;
                break;
            }
            case OP_ISUBSTORE: {
                StackValue value = stack_pop(&vm->stack);
                int slot = (vm->code[vm->pc] << 8) | vm->code[vm->pc + 1];
                vm->pc += 2;

                if (slot >= vm->locals_size) {
                    fprintf(stderr, "invalid slot `%d` for locals\n", slot);
                    exit(1);
                }

                vm->locals[slot].int_val -= value.int_val;
                break;
            }
            case OP_IDIVSTORE: {
                StackValue value = stack_pop(&vm->stack);
                int slot = (vm->code[vm->pc] << 8) | vm->code[vm->pc + 1];
                vm->pc += 2;

                if (slot >= vm->locals_size) {
                    fprintf(stderr, "invalid slot `%d` for locals\n", slot);
                    exit(1);
                }

                vm->locals[slot].int_val /= value.int_val;
                break;
            }
            case OP_IMULSTORE: {
                StackValue value = stack_pop(&vm->stack);
                int slot = (vm->code[vm->pc] << 8) | vm->code[vm->pc + 1];
                vm->pc += 2;

                if (slot >= vm->locals_size) {
                    fprintf(stderr, "invalid slot `%d` for locals\n", slot);
                    exit(1);
                }

                vm->locals[slot].int_val *= value.int_val;
                break;
            }
            case OP_IGT: {
                StackValue b = stack_pop(&vm->stack);
                StackValue a = stack_pop(&vm->stack);
                StackValue sv = {.type = bool_type, .bool_val = a.int_val > b.int_val};
                stack_push(&vm->stack, sv);
                break;
            }
            case OP_IGTE: {
                StackValue b = stack_pop(&vm->stack);
                StackValue a = stack_pop(&vm->stack);
                StackValue sv = {.type = bool_type, .bool_val = a.int_val >= b.int_val};
                stack_push(&vm->stack, sv);
                break;
            }
            case OP_ILT: {
                StackValue b = stack_pop(&vm->stack);
                StackValue a = stack_pop(&vm->stack);
                StackValue sv = {.type = bool_type, .bool_val = a.int_val < b.int_val};
                stack_push(&vm->stack, sv);
                break;
            }
            case OP_ILTE: {
                StackValue b = stack_pop(&vm->stack);
                StackValue a = stack_pop(&vm->stack);
                StackValue sv = {.type = bool_type, .bool_val = a.int_val <= b.int_val};
                stack_push(&vm->stack, sv);
                break;
            }
            case OP_IMOD: {
                StackValue b = stack_pop(&vm->stack);
                StackValue a = stack_pop(&vm->stack);
                if (b.int_val == 0) {
                    fprintf(stderr, "runtime error: division by 0 not allowed\n");
                    exit(1);
                }
                StackValue sv = {.type = int_type, .int_val = a.int_val % b.int_val};
                stack_push(&vm->stack, sv);
                break;
            }
            case OP_IEQ: {
                StackValue b = stack_pop(&vm->stack);
                StackValue a = stack_pop(&vm->stack);
                StackValue sv = {.type = bool_type, .bool_val = a.int_val == b.int_val};
                stack_push(&vm->stack, sv);
                break;
            }
            case OP_BEQ: {
                StackValue b = stack_pop(&vm->stack);
                StackValue a = stack_pop(&vm->stack);
                StackValue sv = {.type = bool_type, .bool_val = a.bool_val == b.bool_val};
                stack_push(&vm->stack, sv);
                break;
            }
            case OP_INEQ: {
                StackValue b = stack_pop(&vm->stack);
                StackValue a = stack_pop(&vm->stack);
                StackValue sv = {.type = bool_type, .bool_val = a.int_val != b.int_val};
                stack_push(&vm->stack, sv);
                break;
            }
            case OP_BNEQ: {
                StackValue b = stack_pop(&vm->stack);
                StackValue a = stack_pop(&vm->stack);
                StackValue sv = {.type = bool_type, .bool_val = a.bool_val != b.bool_val};
                stack_push(&vm->stack, sv);
                break;
            }
            case OP_INEG: {
                StackValue a = stack_pop(&vm->stack);
                StackValue sv = {.type = int_type, .int_val = -a.int_val};
                stack_push(&vm->stack, sv);
                break;
            }
            case OP_NOT:
                StackValue a = stack_pop(&vm->stack);
                StackValue sv = {.type = bool_type, .bool_val = !a.bool_val};
                stack_push(&vm->stack, sv);
                break;
            case OP_BLOAD:
            case OP_SLOAD:
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
            case OP_SSTORE:
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
                int steps = (int)(int16_t)(vm->code[vm->pc] << 8 | vm->code[vm->pc + 1]);
                vm->pc += steps + 2; // steps and then the 2 for the actual step amount
                break;
            }
            case OP_JMPN: {
                int steps = (int)(int16_t)(vm->code[vm->pc] << 8 | vm->code[vm->pc + 1]);
                vm->pc += 2;
                StackValue sv = stack_pop(&vm->stack);
                if (!sv.bool_val) {
                    vm->pc += steps;
                }
                break;
            }
            case OP_JMPT: {
                int steps = (int)(int16_t)(vm->code[vm->pc] << 8 | vm->code[vm->pc + 1]);
                vm->pc += 2;
                StackValue sv = stack_pop(&vm->stack);
                if (sv.bool_val) {
                    vm->pc += steps;
                }
                break;
            }
            case OP_SCONCAT: {
                StackValue b = stack_pop(&vm->stack);
                StackValue a = stack_pop(&vm->stack);

                size_t len_a = a.string_val->len;
                size_t len_b = b.string_val->len;

                int len_result = len_a + len_b;

                char* result = malloc((len_result + 1) * sizeof(char));
                if (!result) {
                    fprintf(stderr, "runtime error: failed to allocate memory during string concat\n");
                    exit(1);
                }

                memcpy(result, a.string_val->string_val, len_a);
                memcpy(result + len_a, b.string_val->string_val, len_b);
                result[len_a + len_b] = '\0';

                decrement_ref(a.string_val);
                decrement_ref(b.string_val);

                StringValue* strv = malloc(sizeof(StringValue));
                strv->string_val = result;
                strv->len = len_result;
                strv->ref_count = 1;

                StackValue sv = {.type=string_type, .string_val = strv};

                stack_push(&vm->stack, sv);

                break;
            }
            default:
                fprintf(stderr, "unknown opcode: %d\n", instruction);
                exit(1);
        }
    }
}

void vm_free(VM* vm) {
    for (int i = 0; i <= vm->stack.top; i++) {
        if (vm->stack.data[i].type.base_type == VALUE_STRING && vm->stack.data[i].type.nested == -1) {
            free(vm->stack.data[i].string_val->string_val);
            free(vm->stack.data[i].string_val);
        }
    }

    free(vm->constants);
    free(vm->locals);
    free(vm->code);
    free(vm->stack.data);
}
