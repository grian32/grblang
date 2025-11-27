#include "stack.h"
#include "parser.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

void stack_value_string(StackValue sv, bool simple, char* buffer, size_t bufsize, size_t* len) {
    if (sv.type.nested != -1){
        *len += snprintf(buffer + *len, bufsize - *len, "[");
        for (int i = 0; i < sv.array_val->len; i++) {
            stack_value_string(sv.array_val->arr_val[i], simple, buffer, bufsize, len);
            if (i != sv.array_val->len - 1) {
                *len += snprintf(buffer + *len, bufsize - *len, ", ");
            }
        }
        *len += snprintf(buffer + *len, bufsize - *len, "]");
        return;
    }

    switch (sv.type.base_type) {
        case VALUE_INT:
            if (simple) {
                *len += snprintf(buffer + *len, bufsize - *len, "%d", sv.int_val);
                break;
            }
            *len += snprintf(buffer + *len, bufsize - *len, "INT(%d)", sv.int_val);
            break;
        case VALUE_BOOL:
            if (simple) {
                *len += snprintf(buffer + *len, bufsize - *len, "%s", sv.bool_val ? "true" : "false");
                break;
            }
            *len += snprintf(buffer + *len, bufsize - *len, "BOOL(%s)", sv.bool_val ? "true" : "false");
            break;
        case VALUE_STRING:
            if (simple) {
                *len += snprintf(buffer + *len, bufsize - *len, "%s", sv.string_val->string_val);
                break;
            }
            *len += snprintf(buffer + *len, bufsize - *len, "STRING(%s)", sv.string_val->string_val);
            break;
        default:
            *len += snprintf(buffer + *len, bufsize - *len, "UNKNOWN");
            break;
    }
}

void stack_init(Stack* s) {
    int initial_capacity = 128;
    s->data = malloc(initial_capacity* sizeof(StackValue));
    s->top = -1;
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
    } else if (val.type.nested != -1) {
        increment_ref_arr(val.array_val);
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
    } else if (sv.type.nested != -1) {
        decrement_ref_arr(sv.array_val);
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

void increment_ref_arr(ArrayValue* arrv) {
    if (!arrv) return;
    if (arrv->ref_count) {
        arrv->ref_count++;
    }

    for (int i = 0; i < arrv->len; i++) {
        if (arrv->arr_val && arrv->arr_val[i].array_val && arrv->arr_val[i].type.nested != -1) {
            increment_ref_arr(arrv->arr_val[i].array_val);
        } else if (arrv->arr_val && arrv->arr_val[i].string_val && arrv->arr_val[i].type.base_type == VALUE_STRING && arrv->arr_val[i].type.nested == -1) {
            increment_ref(arrv->arr_val[i].string_val);
        }
    }
}

void decrement_ref_arr(ArrayValue* arrv) {
    if (!arrv) return;
    arrv->ref_count--;

    if (arrv->ref_count == 0) {
        for (int i = 0; i < arrv->len; i++) {
            if (arrv->arr_val && arrv->arr_val[i].array_val && arrv->arr_val[i].type.nested != -1) {
                decrement_ref_arr(arrv->arr_val[i].array_val);
            } else if (arrv->arr_val && arrv->arr_val[i].string_val && arrv->arr_val[i].type.base_type == VALUE_STRING && arrv->arr_val[i].type.nested == -1) {
                decrement_ref(arrv->arr_val[i].string_val);
            }
        }
        free(arrv->arr_val);
        free(arrv);
    }
}
