#ifndef GRBLANG_BYTECODE_EMIT_H
#define GRBLANG_BYTECODE_EMIT_H
#include <stdint.h>

#include "lexer.h"
#include "parser.h"
#include "resolver.h"
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
    OP_PUSH, // 0
    OP_PUSH_TRUE, // true // 1
    OP_PUSH_FALSE, // false // 2
    OP_PUSH_ARRAY, // 3
    OP_IADD, // 4
    OP_IADDSTORE, // 5
    OP_ISUB, // 6
    OP_ISUBSTORE, // 7
    OP_IMUL, // 8
    OP_IMULSTORE, // 9
    OP_IDIV, // 10
    OP_IDIVSTORE, // 11
    OP_INEG, // 12
    OP_IGT, // 13
    OP_IGTE, // 14
    OP_ILT, // 15
    OP_ILTE, // 16
    OP_IEQ, // 17
    OP_IMOD, // 18
    OP_BEQ, // 19
    OP_INEQ, // 20
    OP_BNEQ, // 21
    OP_ISTORE, // int // 22
    OP_ILOAD, // 23
    OP_BSTORE, // bool // 24
    OP_BLOAD, // 25
    OP_ARRSTORE, // arrays // 26
    OP_ARRLOAD, // 27
    OP_NOT, // 28
    OP_JMPN, // 29
    OP_JMPT, // 30
    OP_JMP, // 31
    OP_PUSH_STRING, // 32
    OP_SCONCAT, // 33
    OP_SSTORE, // 34
    OP_SLOAD, // 35
    OP_ARRLOADIDX, // 36
    OP_ARRSTOREIDX, // 37
    OP_ARRAPPEND, // 38
} BytecodeOp;

void bytecode_init(BytecodeEmitter* b);

void bytecode_resize_code(BytecodeEmitter* b);
void bytecode_resize_const(BytecodeEmitter* b);

void bytecode_gen(ASTNode* node, BytecodeEmitter* b, Resolver* r);

uint16_t add_const(BytecodeEmitter* b, StackValue val);

void emit_push_int(BytecodeEmitter* b, int val);
void emit_push_bool(BytecodeEmitter* b, bool val);
void emit_byte(BytecodeEmitter* b, uint8_t val);

void emit_store(BytecodeEmitter* b, VarType type, int slot);
void emit_load(BytecodeEmitter* b, VarType type, int slot);

void emit_icompound_assignment(BytecodeEmitter* b, TokenType op, int slot);

int emit_jmpn(BytecodeEmitter* b, int steps);
int emit_jmpt(BytecodeEmitter* b, int steps);
int emit_jmp(BytecodeEmitter* b, int steps);


void patch_int(BytecodeEmitter* b, int new_value, int starts_at);

void emit_push_string(BytecodeEmitter* b, char* str);

#endif //GRBLANG_BYTECODE_EMIT_H
