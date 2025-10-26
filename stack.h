#ifndef GRBLANG_STACK_H
#define GRBLANG_STACK_H

typedef struct {
    int* data;
    int top;
    int capacity;
} Stack;

void stack_init(Stack* s);
void stack_push(Stack* s, int val);
int stack_pop(Stack* s);
int stack_peek(Stack* s);

#endif //GRBLANG_STACK_H