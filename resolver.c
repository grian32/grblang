#include "resolver.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void resolver_init(Resolver* r) {
    r->capacity = 16;
    r->count = 0;
    r->names = malloc(sizeof(char*) * r->capacity);
}

void resolver_resize(Resolver* r) {
    r->capacity *= 2;
    char** new_names = realloc(r->names, sizeof(char*) * r->capacity);
    if (!new_names) {
        fprintf(stderr, "failed to realloc names in resolver\n");
        exit(1);
    }
    r->names = new_names;
}

int resolver_declare(Resolver* r, char* name) {
    for (int i = 0; i < r->count; i++) {
        if (strcmp(r->names[i], name) == 0) {
            fprintf(stderr, "variable `%s` already declared\n", name);
            exit(1);
        }
    }

    if (r->count >= r->capacity) {
        resolver_resize(r);
    }

    r->names[r->count] = strdup(name);
    return r->count++;
}

int resolver_lookup(Resolver* r, char* name) {
    int slot = -1;
    for (int i = 0; i < r->count; i++) {
        if (strcmp(r->names[i], name) == 0) {
            slot = i;
        }
    }

    if (slot == -1) {
        fprintf(stderr, "cannot reassign var `%s` as it doesnt exist\n", name);
    }

    return slot;
}

void resolve(ASTNode* node, Resolver* r) {
    if (!node) return;

    switch (node->type) {
        case AST_VAR_DECL:
            resolve(node->var_decl.value, r);
            node->var_decl.slot = resolver_declare(r, node->var_decl.name);
            break;
        case AST_VAR_ASSIGN:
            resolve(node->var_assign.value, r);
            node->var_assign.slot = resolver_lookup(r, node->var_assign.name);
            if (node->var_assign.slot == -1) {
                fprintf(stderr, "undefined variable `%s` when trying to reassign", node->var_assign.name);
                exit(1);
            }
            break;
        case AST_VAR_REF:
            node->var_ref.slot = resolver_lookup(r, node->var_ref.name);
            if (node->var_ref.slot == -1) {
                fprintf(stderr, "undefined variable `%s` when trying to reference", node->var_assign.name);
                exit(1);
            }
            break;
        case AST_PROGRAM:
            for (int i = 0; i < node->program.count; i++) {
                resolve(node->program.statements[i], r);
            }
            break;
        case AST_BINARY_OP:
            resolve(node->binary_op.left, r);
            resolve(node->binary_op.right, r);
            break;
        case AST_UNARY_OP:
            resolve(node->binary_op.right, r);
            break;
    }
}
