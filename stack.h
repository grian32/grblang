#ifndef GRBLANG_STACK_H
#define GRBLANG_STACK_H
#include "parser.h"
#include <stdint.h>

typedef struct {
    char* string_val;
    int len;
    int ref_count;
} StringValue;

void increment_ref(StringValue* strv);
void decrement_ref(StringValue* strv);


typedef struct {
    VarType type;
    union {
        int int_val;
        bool bool_val;
        StringValue* string_val;
        struct ArrayValue* array_val;
        struct FunctionValue* fn_val;
    };
} StackValue;

typedef struct ArrayValue {
    StackValue* arr_val;
    int len;
    int capacity;
    int ref_count;
} ArrayValue;

void increment_ref_arr(ArrayValue* arrv);
void decrement_ref_arr(ArrayValue* arrv);

void stack_value_string(StackValue sv, bool simple, char* buffer, size_t bufsize, size_t* len);

typedef struct Stack {
    StackValue* data;
    int top;
    int capacity;
} Stack;

typedef struct {
    StackValue* constants;
    int constants_size;

    StackValue* locals;
    int locals_size;

    uint8_t* code;
    int code_size;
} FunctionValue;

void stack_init(Stack* s);
void stack_push(Stack* s, StackValue val);
StackValue stack_pop(Stack* s);
StackValue stack_peek(Stack* s);

#endif //GRBLANG_STACK_H
