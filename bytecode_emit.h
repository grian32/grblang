#ifndef GRBLANG_BYTECODE_EMIT_H
#define GRBLANG_BYTECODE_EMIT_H
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "parser.h"

typedef struct {
    uint8_t* code;
    int code_size;
    int code_capacity;

    int* constants;
    int const_count;
    int const_capacity;
} BytecodeEmitter;

typedef enum {
    OP_PUSH,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_NEG,
} BytecodeOp;

void bytecode_init(BytecodeEmitter* b);

void bytecode_resize_code(BytecodeEmitter* b);
void bytecode_resize_const(BytecodeEmitter* b);

void bytecode_gen(ASTNode* node, BytecodeEmitter* b);

uint16_t add_const(BytecodeEmitter* b, int val);

void emit_push_int(BytecodeEmitter* b, int val);
void emit_byte(BytecodeEmitter* b, uint8_t val);

void emit_add(BytecodeEmitter* b);
void emit_sub(BytecodeEmitter* b);
void emit_mul(BytecodeEmitter* b);
void emit_div(BytecodeEmitter* b);

void emit_neg(BytecodeEmitter* b);

#endif //GRBLANG_BYTECODE_EMIT_H