#include "bytecode_emit.h"

void bytecode_init(BytecodeEmitter* b) {
    int initial_capacity = 100;
    b->code = malloc(initial_capacity * sizeof(uint8_t));
    b->code_size = 0;
    b->code_capacity = 100;

    b->constants = malloc(initial_capacity * sizeof(int));
    b->const_count = 0;
    b->const_capacity = 100;
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
