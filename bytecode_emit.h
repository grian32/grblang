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
    OP_IADD,
    OP_ISUB,
    OP_IMUL,
    OP_IDIV,
    OP_INEG,
    OP_IGT,
    OP_ILT,
    OP_IEQ,
    OP_IMOD,
    OP_BEQ,
    OP_INEQ,
    OP_BNEQ,
    OP_ISTORE, // int
    OP_ILOAD,
    OP_BSTORE, // bool
    OP_BLOAD,
    OP_NOT,
    OP_JMPN,
    OP_JMP
} BytecodeOp;

void bytecode_init(BytecodeEmitter* b);

void bytecode_resize_code(BytecodeEmitter* b);
void bytecode_resize_const(BytecodeEmitter* b);

void bytecode_gen(ASTNode* node, BytecodeEmitter* b);

uint16_t add_const(BytecodeEmitter* b, StackValue val);

void emit_push_int(BytecodeEmitter* b, int val);
void emit_push_bool(BytecodeEmitter* b, bool val);
void emit_byte(BytecodeEmitter* b, uint8_t val);

void emit_store(BytecodeEmitter* b, VarType type, int slot);
void emit_load(BytecodeEmitter* b, VarType type, int slot);

void emit_iadd(BytecodeEmitter* b);
void emit_isub(BytecodeEmitter* b);
void emit_imul(BytecodeEmitter* b);
void emit_idiv(BytecodeEmitter* b);

void emit_igt(BytecodeEmitter* b);
void emit_ilt(BytecodeEmitter* b);
void emit_ieq(BytecodeEmitter* b);
void emit_ineq(BytecodeEmitter* b);

void emit_beq(BytecodeEmitter* b);
void emit_bneq(BytecodeEmitter* b);

void emit_neg(BytecodeEmitter* b);
void emit_not(BytecodeEmitter* b);

int emit_jmpn(BytecodeEmitter* b, int steps);
int emit_jmp(BytecodeEmitter* b, int steps);

void emit_imod(BytecodeEmitter* b);

void patch_int(BytecodeEmitter* b, int new_value, int starts_at);

#endif //GRBLANG_BYTECODE_EMIT_H
