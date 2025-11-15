#include "bytecode_emit.h"
#include "lexer.h"
#include "parser.h"
#include "resolver.h"
#include "stack.h"
#include "type_checker.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

void bytecode_init(BytecodeEmitter* b) {
    int initial_capacity = 128;
    b->code = malloc(initial_capacity * sizeof(uint8_t));
    b->code_size = 0;
    b->code_capacity = initial_capacity;

    b->constants = malloc(initial_capacity * sizeof(int));
    b->const_count = 0;
    b->const_capacity = initial_capacity;
}

void bytecode_resize_code(BytecodeEmitter* b) {
    int new_capacity = b->code_capacity * 2;
    uint8_t* new_code = realloc(b->code, new_capacity * sizeof(uint8_t));
    if (!new_code) {
        fprintf(stderr, "failed to realloc bytecode arr\n");
        exit(1);
    }

    b->code = new_code;
    b->code_capacity = new_capacity;
}

void bytecode_resize_const(BytecodeEmitter* b) {
    int new_capacity = b->const_capacity * 2;
    StackValue* new_constants = realloc(b->constants, new_capacity * sizeof(StackValue));
    if (!new_constants) {
        fprintf(stderr,"failed to realloc constants arr\n");
        exit(1);
    }

    b->constants = new_constants;
    b->const_capacity = new_capacity;
}

void bytecode_gen(ASTNode* node, BytecodeEmitter* b, Resolver* r) {
    if (!node) return;

    switch (node->type) {
        case AST_INT:
            emit_push_int(b, node->int_val);
            break;
        case AST_BOOL:
            emit_push_bool(b, node->bool_val);
            break;
        case AST_STRING:
            emit_push_string(b, node->string.string_val);
            break;
        case AST_BINARY_OP:
            // need to output left/right differently from all other ops for and and or
            if (node->binary_op.op == TOK_AND) {
                bytecode_gen(node->binary_op.left, b, r);
                int jmpn_start = emit_jmpn(b, 0);
                bytecode_gen(node->binary_op.right, b, r);
                int jmpn_step_count = b->code_size - (jmpn_start + 2);
                patch_int(b, jmpn_step_count, jmpn_start);
                break;
            } else if (node->binary_op.op == TOK_OR) {
                bytecode_gen(node->binary_op.left, b, r);
                int jmpt_start = emit_jmpt(b, 0);
                bytecode_gen(node->binary_op.right, b, r);
                int jmpt_step_count = b->code_size - (jmpt_start + 2);
                patch_int(b, jmpt_step_count, jmpt_start);
                break;
            }

            bytecode_gen(node->binary_op.left, b, r);
            bytecode_gen(node->binary_op.right, b, r);

            switch (node->binary_op.op) {
                case TOK_PLUS: {
                    VarType leftType = get_expr_type(node->binary_op.left, r);
                    if (leftType == VALUE_INT) {
                        emit_iadd(b);
                    } else {
                        emit_sconcat(b);
                    }
                    break;
                }
                case TOK_MINUS: emit_isub(b); break;
                case TOK_MULT: emit_imul(b); break;
                case TOK_DIV: emit_idiv(b); break;
                case TOK_GREATER: emit_igt(b); break;
                case TOK_LESS: emit_ilt(b); break;
                case TOK_GREATER_EQUALS: emit_igte(b); break;
                case TOK_LESS_EQUALS: emit_ilte(b); break;
                case TOK_MODULO: emit_imod(b); break;
                case TOK_EQUALS: {
                    // can just check the left node as type checker should catch cases where it's not the same type on both sides
                    VarType leftType = get_expr_type(node->binary_op.left, r);
                    if (leftType == VALUE_INT) {
                        emit_ieq(b);
                    } else {
                        emit_beq(b);
                    }
                    break;
                }
                case TOK_NOT_EQUALS: {
                    VarType leftType = get_expr_type(node->binary_op.left, r);
                    if (leftType == VALUE_INT) {
                        emit_ineq(b);
                    } else {
                        emit_bneq(b);
                    }
                    break;
                }
                default: break;
            }
            break;
        case AST_UNARY_OP:
            bytecode_gen(node->unary_op.right, b, r);

            switch (node->unary_op.op) {
                case TOK_MINUS: emit_neg(b); break;
                case TOK_EXCLAM: emit_not(b); break;
                default: break;
            }
            break;
        case AST_VAR_DECL:
            bytecode_gen(node->var_decl.value, b, r);
            emit_store(b, node->var_type, node->var_decl.slot);
            break;
        case AST_VAR_ASSIGN:
            bytecode_gen(node->var_assign.value, b, r);
            emit_store(b, node->var_type, node->var_assign.slot);
            break;
        case AST_COMPOUND_ASSIGNMENT:
            bytecode_gen(node->compound_assignment.value, b, r);
            emit_icompound_assignment(b, node->compound_assignment.op, node->compound_assignment.slot);
            break;
        case AST_VAR_REF:
            emit_load(b, node->var_type, node->var_ref.slot);
            break;
        case AST_PROGRAM:
            for (int i = 0; i < node->program.count; i++) {
                bytecode_gen(node->program.statements[i], b, r);
            }
            break;
        case AST_IF: {
            bytecode_gen(node->if_stmt.condition, b, r);

            int jmpn_step_start = emit_jmpn(b, 0);
            int curr_instruction_count = b->code_size;
            for (int i = 0; i < node->if_stmt.success_count; i++) {
                bytecode_gen(node->if_stmt.success_statements[i], b, r);
            }
            int jmpn_instruction_count = b->code_size - curr_instruction_count;
            if (node->if_stmt.fail_statements) {
                // required to skip the JMP generated by the else block
                jmpn_instruction_count += 3;
            }
            patch_int(b, jmpn_instruction_count, jmpn_step_start);

            int jmp_step_start = -1;

            if (node->if_stmt.fail_statements) {
                int jmp_step_start = emit_jmp(b, 0);
                int curr_instruction_count = b->code_size;
                for (int i = 0; i < node->if_stmt.fail_count; i++) {
                    bytecode_gen(node->if_stmt.fail_statements[i], b, r);
                }
                int jmp_instruction_count = b->code_size - curr_instruction_count;
                patch_int(b, jmp_instruction_count, jmp_step_start);
            }
            break;
        }
        case AST_WHILE: {
            int jmp_start = b->code_size;
            bytecode_gen(node->while_stmt.condition, b, r);
            int jmpn_idx = emit_jmpn(b, 0);

            for (int i = 0; i < node->while_stmt.statements_count; i++) {
                bytecode_gen(node->while_stmt.statements[i], b, r);
            }

            emit_jmp(b, jmp_start - b->code_size - 3);
            patch_int(b, b->code_size - (jmpn_idx + 2), jmpn_idx);
            break;
        }
    }
}

uint16_t add_const(BytecodeEmitter* b, StackValue val) {
    if (b->const_count >= b->const_capacity) {
        bytecode_resize_const(b);
    }

    b->constants[b->const_count] = val;
    return b->const_count++;
}

void emit_push_int(BytecodeEmitter* b, int val) {
    StackValue sv = {.type = VALUE_INT, .int_val = val};
    uint16_t idx = add_const(b, sv);
    emit_byte(b, OP_PUSH);
    emit_byte(b, (idx >> 8) & 0xFF);
    emit_byte(b, idx & 0xFF);
}

void emit_push_bool(BytecodeEmitter* b, bool val) {
    if (val) {
        emit_byte(b, OP_PUSH_TRUE);
    } else {
        emit_byte(b, OP_PUSH_FALSE);
    }
}

void emit_byte(BytecodeEmitter* b, uint8_t val) {
    if (b->code_size >= b->code_capacity) {
        bytecode_resize_code(b);
    }

    b->code[b->code_size++] = val;
}

void emit_store(BytecodeEmitter* b, VarType type, int slot) {
    switch (type) {
    case VALUE_INT:
        emit_byte(b, OP_ISTORE);
        break;
    case VALUE_BOOL:
        emit_byte(b, OP_BSTORE);
        break;
    case VALUE_STRING:
        emit_byte(b, OP_SSTORE);
        break;
    case VALUE_UNKNOWN:
        fprintf(stderr, "unknown value type for store");
        break;
    }
    emit_byte(b, (slot>> 8) & 0xFF);
    emit_byte(b, slot & 0xFF);
}

void emit_load(BytecodeEmitter* b, VarType type, int slot) {
    if (type == VALUE_INT) {
        emit_byte(b, OP_ILOAD);
    } else {
        emit_byte(b, OP_BLOAD);
    }
    emit_byte(b, (slot>> 8) & 0xFF);
    emit_byte(b, slot & 0xFF);
}

void emit_iadd(BytecodeEmitter* b) {
    emit_byte(b, OP_IADD);
}

void emit_isub(BytecodeEmitter* b) {
    emit_byte(b, OP_ISUB);
}

void emit_imul(BytecodeEmitter* b) {
    emit_byte(b, OP_IMUL);
}

void emit_idiv(BytecodeEmitter* b) {
    emit_byte(b, OP_IDIV);
}

void emit_igt(BytecodeEmitter* b) {
    emit_byte(b, OP_IGT);
}

void emit_igte(BytecodeEmitter* b) {
    emit_byte(b, OP_IGTE);
}

void emit_ilt(BytecodeEmitter* b) {
    emit_byte(b, OP_ILT);
}

void emit_ilte(BytecodeEmitter* b) {
    emit_byte(b, OP_ILTE);
}

void emit_ieq(BytecodeEmitter* b) {
    emit_byte(b, OP_IEQ);
}

void emit_icompound_assignment(BytecodeEmitter* b, TokenType op, int slot) {
    switch (op) {
        case TOK_PLUS_EQUALS:
            emit_byte(b, OP_IADDSTORE);
            break;
        case TOK_MINUS_EQUALS:
            emit_byte(b, OP_ISUBSTORE);
            break;
        case TOK_MULT_EQUALS:
            emit_byte(b, OP_IMULSTORE);
            break;
        case TOK_DIV_EQUALS:
            emit_byte(b, OP_IDIVSTORE);
            break;
        default:
            fprintf(stderr, "invalid operator token while trying to emit compound assignment operation: %d\n", op);
            exit(1);
            break;
    }
    emit_byte(b, (slot>> 8) & 0xFF);
    emit_byte(b, slot & 0xFF);
}

void emit_beq(BytecodeEmitter* b) {
    emit_byte(b, OP_BEQ);
}

void emit_ineq(BytecodeEmitter* b) {
    emit_byte(b, OP_INEQ);
}

void emit_bneq(BytecodeEmitter* b) {
    emit_byte(b, OP_BNEQ);
}

void emit_neg(BytecodeEmitter* b) {
    emit_byte(b, OP_INEG);
}

void emit_not(BytecodeEmitter* b) {
    emit_byte(b, OP_NOT);
}

int emit_jmpn(BytecodeEmitter* b, int steps) {
    emit_byte(b, OP_JMPN);

    int idx = b->code_size;
    emit_byte(b, (steps >> 8) & 0xFF);
    emit_byte(b, steps & 0xFF);
    return idx;
}

int emit_jmpt(BytecodeEmitter* b, int steps) {
    emit_byte(b, OP_JMPT);

    int idx = b->code_size;
    emit_byte(b, (steps >> 8) & 0xFF);
    emit_byte(b, steps & 0xFF);
    return idx;
}

int emit_jmp(BytecodeEmitter* b, int steps) {
    emit_byte(b, OP_JMP);

    int idx = b->code_size;
    emit_byte(b, (steps >> 8) & 0xFF);
    emit_byte(b, steps & 0xFF);
    return idx;
}

void emit_imod(BytecodeEmitter* b) {
    emit_byte(b, OP_IMOD);
}

void patch_int(BytecodeEmitter* b, int new_val, int starts_at) {
    b->code[starts_at] = (new_val >> 8) & 0xFF;
    b->code[starts_at + 1] = new_val & 0xFF;
}

void emit_push_string(BytecodeEmitter* b, char* str) {
    StringValue* strv = malloc(sizeof(StringValue));
    strv->string_val = strdup(str);
    strv->len = strlen(str);
    strv->ref_count = 1;
    StackValue sv = {.type = VALUE_STRING, .string_val = strv};

    uint16_t idx = add_const(b, sv);

    emit_byte(b, OP_PUSH_STRING);
    emit_byte(b, (idx >> 8) & 0xFF);
    emit_byte(b, idx & 0xFF);
}

void emit_sconcat(BytecodeEmitter* b) {
    emit_byte(b, OP_SCONCAT);
}
