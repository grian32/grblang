#include "stack.h"
#include "parser.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

void stack_value_string(StackValue sv, bool simple, char buffer[50]) {
    if (sv.type.nested != -1){
        fprintf(stderr, "array types not supported in stack_value_string");
        exit(1);
    }
    switch (sv.type.base_type) {
        case VALUE_INT:
            if (simple) {
                sprintf(buffer, "%d", sv.int_val);
                break;
            }
            sprintf(buffer, "INT(%d)", sv.int_val);
            break;
        case VALUE_BOOL:
            if (simple) {
                sprintf(buffer, "%s", sv.bool_val ? "true" : "false");
                break;
            }
            sprintf(buffer, "BOOL(%s)", sv.bool_val ? "true" : "false");
            break;
        case VALUE_STRING:
            if (simple) {
                sprintf(buffer, "%s", sv.string_val->string_val);
                break;
            }
            sprintf(buffer, "STRING(%s)", sv.string_val->string_val);
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
    if (val.type.base_type == VALUE_STRING && val.type.nested == -1) {
        increment_ref(val.string_val);
    }
    s->data[++s->top] = val;
}

StackValue stack_pop(Stack* s) {
    if (s->top < 0) {
        fprintf(stderr, "stack underflow\n");
        exit(1);
    }

    StackValue sv = s->data[s->top--];
    if (sv.type.base_type == VALUE_STRING && sv.type.nested == -1) {
        decrement_ref(sv.string_val);
    }
    return sv;
}

StackValue stack_peek(Stack* s) {
    if (s->top < 0) {
        fprintf(stderr, "stack empty\n");
        exit(1);
    }
    return s->data[s->top];
}

void increment_ref(StringValue* strv) {
    if (strv) {
        strv->ref_count++;
    }
}

void decrement_ref(StringValue* strv) {
    if (strv) {
        strv->ref_count--;
        if (strv->ref_count == 0) {
            free(strv->string_val);
            free(strv);
        }
    }
}
