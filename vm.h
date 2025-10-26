#ifndef GRBLANG_VIRTUAL_MACHINE_H
#define GRBLANG_VIRTUAL_MACHINE_H
#include <stdint.h>

#include "bytecode_emit.h"
#include "stack.h"

typedef struct {
    Stack stack;
    int* constants;
    uint8_t* code;
    int code_size;
    int pc;
} VM;

void vm_init(VM* vm, BytecodeEmitter* b);

void vm_run(VM* vm);

#endif //GRBLANG_VIRTUAL_MACHINE_H