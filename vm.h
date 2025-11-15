#ifndef GRBLANG_VIRTUAL_MACHINE_H
#define GRBLANG_VIRTUAL_MACHINE_H
#include <stdint.h>

#include "bytecode_emit.h"
#include "stack.h"

typedef struct {
    Stack stack;

    StackValue* constants;
    int constants_size;

    StackValue* locals;
    int locals_size;

    uint8_t* code;
    int code_size;
    int pc;
} VM;

void vm_init(VM* vm, BytecodeEmitter* b, int num_locals);

void vm_run(VM* vm);

// to be called after vm_run has finished execution
void vm_free(VM* vm);

#endif //GRBLANG_VIRTUAL_MACHINE_H
