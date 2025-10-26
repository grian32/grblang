#include "stack.h"

#include <stdio.h>
#include <stdlib.h>

void stack_init(Stack* s) {
    int initial_capacity = 128;
    s->data = malloc(initial_capacity* sizeof(int));
    s->top = 0;
    s->capacity = initial_capacity;
}

void stack_push(Stack* s, int val) {
    if (s->top+1 >= s->capacity) {
        s->capacity *= 2;
        int* new_data = realloc(s->data, s->capacity * sizeof(int));
        if (!new_data) {
            fprintf(stderr, "failed to realloc stack arr\n");
            exit(1);
        }
        s->data = new_data;
    }
    s->data[++s->top] = val;
}

int stack_pop(Stack* s) {
    if (s->top < 0) {
        fprintf(stderr, "stack underflow\n");
        exit(1);
    }
    return s->data[s->top--];
}

int stack_peek(Stack* s) {
    if (s->top < 0) {
        fprintf(stderr, "stack empty\n");
        exit(1);
    }
    return s->data[s->top];
}
