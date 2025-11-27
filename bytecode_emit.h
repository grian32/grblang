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
    OP_PUSH,
    OP_PUSH_TRUE, // true
    OP_PUSH_FALSE, // false
    OP_PUSH_ARRAY,
    OP_IADD,
    OP_IADDSTORE,
    OP_ISUB,
    OP_ISUBSTORE,
    OP_IMUL,
    OP_IMULSTORE,
    OP_IDIV,
    OP_IDIVSTORE,
    OP_INEG,
    OP_IGT,
    OP_IGTE,
    OP_ILT,
    OP_ILTE,
    OP_IEQ,
    OP_IMOD,
    OP_BEQ,
    OP_INEQ,
    OP_BNEQ,
    OP_ISTORE, // int
    OP_ILOAD,
    OP_BSTORE, // bool
    OP_BLOAD,
    OP_ARRSTORE, // arrays
    OP_ARRLOAD,
    OP_NOT,
    OP_JMPN,
    OP_JMPT,
    OP_JMP,
    OP_PUSH_STRING,
    OP_SCONCAT,
    OP_SSTORE,
    OP_SLOAD,
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

void emit_iadd(BytecodeEmitter* b);
void emit_isub(BytecodeEmitter* b);
void emit_imul(BytecodeEmitter* b);
void emit_idiv(BytecodeEmitter* b);

void emit_igt(BytecodeEmitter* b);
void emit_igte(BytecodeEmitter* b);
void emit_ilt(BytecodeEmitter* b);
void emit_ilte(BytecodeEmitter* b);
void emit_ieq(BytecodeEmitter* b);
void emit_ineq(BytecodeEmitter* b);
void emit_icompound_assignment(BytecodeEmitter* b, TokenType op, int slot);

void emit_beq(BytecodeEmitter* b);
void emit_bneq(BytecodeEmitter* b);

void emit_neg(BytecodeEmitter* b);
void emit_not(BytecodeEmitter* b);

int emit_jmpn(BytecodeEmitter* b, int steps);
int emit_jmpt(BytecodeEmitter* b, int steps);
int emit_jmp(BytecodeEmitter* b, int steps);

void emit_imod(BytecodeEmitter* b);

void patch_int(BytecodeEmitter* b, int new_value, int starts_at);

void emit_push_string(BytecodeEmitter* b, char* str);
void emit_sconcat(BytecodeEmitter* b);

#endif //GRBLANG_BYTECODE_EMIT_H
