#include "bytecode_emit.h"

#include <stdio.h>
#include <stdlib.h>

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

void bytecode_gen(ASTNode* node, BytecodeEmitter* b) {
    if (!node) return;

    switch (node->type) {
        case AST_INT:
            emit_push_int(b, node->int_val);
            break;
        case AST_BOOL:
            emit_push_bool(b, node->bool_val);
            break;
        case AST_BINARY_OP:
            bytecode_gen(node->binary_op.left, b);
            bytecode_gen(node->binary_op.right, b);

            switch (node->binary_op.op) {
                case TOK_PLUS: emit_add(b); break;
                case TOK_MINUS: emit_sub(b); break;
                case TOK_MULT: emit_mul(b); break;
                case TOK_DIV: emit_div(b); break;
                default: break;
            }
            break;
        case AST_UNARY_OP:
            bytecode_gen(node->unary_op.right, b);

            switch (node->unary_op.op) {
                case TOK_MINUS: emit_neg(b); break;
                case TOK_EXCLAM: emit_not(b); break;
                default: break;
            }
            break;
        case AST_VAR_DECL:
            bytecode_gen(node->var_decl.value, b);
            emit_store(b, node->type, node->var_decl.slot);
            break;
        case AST_VAR_ASSIGN:
            bytecode_gen(node->var_assign.value, b);
            emit_store(b, node->type, node->var_assign.slot);
            break;
        case AST_VAR_REF:
            emit_load(b, node->type, node->var_ref.slot);
            break;
        case AST_PROGRAM:
            for (int i = 0; i < node->program.count; i++) {
                bytecode_gen(node->program.statements[i], b);
            }
            break;
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

void emit_store(BytecodeEmitter* b, ASTNodeType type, int slot) {
    if (type == AST_INT) {
        emit_byte(b, OP_ISTORE);
    } else {
        emit_byte(b, OP_BSTORE);
    }
    emit_byte(b, (slot>> 8) & 0xFF);
    emit_byte(b, slot & 0xFF);
}

void emit_load(BytecodeEmitter* b, ASTNodeType type, int slot) {
    if (type == AST_INT) {
        emit_byte(b, OP_ILOAD);
    } else {
        emit_byte(b, OP_BLOAD);
    }
    emit_byte(b, (slot>> 8) & 0xFF);
    emit_byte(b, slot & 0xFF);
}

void emit_add(BytecodeEmitter* b) {
    emit_byte(b, OP_IADD);
}

void emit_sub(BytecodeEmitter* b) {
    emit_byte(b, OP_ISUB);
}

void emit_mul(BytecodeEmitter* b) {
    emit_byte(b, OP_IMUL);
}

void emit_div(BytecodeEmitter* b) {
    emit_byte(b, OP_IDIV);
}

void emit_neg(BytecodeEmitter* b) {
    emit_byte(b, OP_INEG);
}

void emit_not(BytecodeEmitter* b) {
    emit_byte(b, OP_NOT);
}
