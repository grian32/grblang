#ifndef GRBLANG_BYTECODE_EMIT_H
#define GRBLANG_BYTECODE_EMIT_H
#include <stdint.h>

#include "parser.h"
#include "stack.h"

typedef struct {
    uint8_t* code;
    int code_size;
    int code_capacity;

    StackValue* constants;
    int const_count;
    int const_capacity;
} BytecodeEmitter;

typedef enum {
    OP_PUSH,
    OP_PUSH_TRUE, // true
    OP_PUSH_FALSE, // false
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_NEG,
    OP_ISTORE, // int
    OP_ILOAD,
    OP_BSTORE, // bool
    OP_BLOAD,
    OP_NOT,
} BytecodeOp;

void bytecode_init(BytecodeEmitter* b);

void bytecode_resize_code(BytecodeEmitter* b);
void bytecode_resize_const(BytecodeEmitter* b);

void bytecode_gen(ASTNode* node, BytecodeEmitter* b);

uint16_t add_const(BytecodeEmitter* b, StackValue val);

void emit_push_int(BytecodeEmitter* b, int val);
void emit_push_bool(BytecodeEmitter* b, bool val);
void emit_byte(BytecodeEmitter* b, uint8_t val);

void emit_store(BytecodeEmitter* b, ASTNodeType type, int slot);
void emit_load(BytecodeEmitter* b, ASTNodeType type, int slot);

void emit_add(BytecodeEmitter* b);
void emit_sub(BytecodeEmitter* b);
void emit_mul(BytecodeEmitter* b);
void emit_div(BytecodeEmitter* b);

void emit_neg(BytecodeEmitter* b);
void emit_not(BytecodeEmitter* b);

#endif //GRBLANG_BYTECODE_EMIT_H