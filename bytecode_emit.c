#include "bytecode_emit.h"

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
    uint8_t* new_code = realloc(b->code, new_capacity);
    if (!new_code) {
        fprintf(stderr, "failed to realloc bytecode arr\n");
        exit(1);
    }

    b->code = new_code;
    b->code_capacity = new_capacity;
}

void bytecode_resize_const(BytecodeEmitter* b) {
    int new_capacity = b->const_capacity * 2;
    int* new_constants = realloc(b->constants, new_capacity);
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
                default: break;
            }
    }
}

uint16_t add_const(BytecodeEmitter* b, int val) {
    if (b->const_count >= b->const_capacity) {
        bytecode_resize_const(b);
    }

    b->constants[b->const_count] = val;
    return b->const_count++;
}

void emit_push_int(BytecodeEmitter* b, int val) {
    uint16_t idx = add_const(b, val);
    emit_byte(b, OP_PUSH);
    emit_byte(b, (idx >> 8) & 0xFF);
    emit_byte(b, idx & 0xFF);
}

void emit_byte(BytecodeEmitter* b, uint8_t val) {
    if (b->code_size >= b->code_capacity) {
        bytecode_resize_code(b);
    }

    b->code[b->code_size++] = val;
}

void emit_add(BytecodeEmitter* b) {
    emit_byte(b, OP_ADD);
}

void emit_sub(BytecodeEmitter* b) {
    emit_byte(b, OP_SUB);
}

void emit_mul(BytecodeEmitter* b) {
    emit_byte(b, OP_MUL);
}

void emit_div(BytecodeEmitter* b) {
    emit_byte(b, OP_DIV);
}

void emit_neg(BytecodeEmitter* b) {
    emit_byte(b, OP_NEG);
}
