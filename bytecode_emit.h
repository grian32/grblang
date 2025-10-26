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

void bytecode_init(BytecodeEmitter* b);

void bytecode_resize_code(BytecodeEmitter* b);
void bytecode_resize_const(BytecodeEmitter* b);

void bytecode_gen(ASTNode* node, BytecodeEmitter* b);

#endif //GRBLANG_BYTECODE_EMIT_H