#include "stack.h"
#include "parser.h"

#include <stdio.h>
#include <stdlib.h>

void stack_value_string(StackValue sv, char buffer[50]) {
    switch (sv.type) {
        case VALUE_INT:
            sprintf(buffer, "INT(%d)", sv.int_val);
            break;
        case VALUE_BOOL:
            sprintf(buffer, "BOOL(%s)", sv.bool_val ? "true" : "false");
            break;
        case VALUE_STRING:
            sprintf(buffer, "STRING(%s)", sv.string_val);
            break;
        default:
            sprintf(buffer, "UNKNOWN");
            break;
    }
}

void stack_init(Stack* s) {
    int initial_capacity = 128;
    s->data = malloc(initial_capacity* sizeof(StackValue));
    s->top = 0;
    s->capacity = initial_capacity;
}

void stack_push(Stack* s, StackValue val) {
    if (s->top+1 >= s->capacity) {
        s->capacity *= 2;
        StackValue* new_data = realloc(s->data, s->capacity * sizeof(StackValue));
        if (!new_data) {
            fprintf(stderr, "failed to realloc stack arr\n");
            exit(1);
        }
        s->data = new_data;
    }
    s->data[++s->top] = val;
}

StackValue stack_pop(Stack* s) {
    if (s->top < 0) {
        fprintf(stderr, "stack underflow\n");
        exit(1);
    }
    return s->data[s->top--];
}

StackValue stack_peek(Stack* s) {
    if (s->top < 0) {
        fprintf(stderr, "stack empty\n");
        exit(1);
    }
    return s->data[s->top];
}
