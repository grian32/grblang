#ifndef GRBLANG_STACK_H
#define GRBLANG_STACK_H
#include "parser.h"

typedef struct {
    VarType type;
    union {
        int int_val;
        bool bool_val;
    };
} StackValue;

void stack_value_string(StackValue sv, char buffer[50]);

typedef struct {
    StackValue* data;
    int top;
    int capacity;
} Stack;

void stack_init(Stack* s);
void stack_push(Stack* s, StackValue val);
StackValue stack_pop(Stack* s);
StackValue stack_peek(Stack* s);

#endif //GRBLANG_STACK_H