#include "resolver.h"
#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void resolver_init(Resolver* r) {
    r->capacity = 16;
    r->count = 0;
    r->names = malloc(sizeof(char*) * r->capacity);
    r->types = malloc(sizeof(VarType) * r->capacity);
}

void resolver_resize(Resolver* r) {
    r->capacity *= 2;
    char** new_names = realloc(r->names, sizeof(char*) * r->capacity);
    if (!new_names) {
        fprintf(stderr, "failed to realloc names arr in resolver\n");
        exit(1);
    }
    r->names = new_names;

    VarType* new_types = realloc(r->types, sizeof(VarType) * r->capacity);
    if (!new_types) {
        fprintf(stderr, "failed to realloc types arr in resolver\n");
        exit(1);
    }
    r->types = new_types;
}

int resolver_declare(Resolver* r, char* name, VarType type) {
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
    r->types[r->count] = type;
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
            node->var_decl.slot = resolver_declare(r, node->var_decl.name, node->var_type);
            break;
        case AST_VAR_ASSIGN:
            resolve(node->var_assign.value, r);
            node->var_assign.slot = resolver_lookup(r, node->var_assign.name);
            if (node->var_assign.slot == -1) {
                fprintf(stderr, "undefined variable `%s` when trying to reassign\n", node->var_assign.name);
                exit(1);
            }
            node->var_type = r->types[node->var_assign.slot];
            break;
        case AST_VAR_REF:
            node->var_ref.slot = resolver_lookup(r, node->var_ref.name);
            if (node->var_ref.slot == -1) {
                fprintf(stderr, "undefined variable `%s` when trying to reference\n", node->var_ref.name);
                exit(1);
            }
            node->var_type = r->types[node->var_ref.slot];
            break;
        case AST_PROGRAM:
            for (int i = 0; i < node->program.count; i++) {
                resolve(node->program.statements[i], r);
            }
            break;
        case AST_IF:
            resolve(node->if_stmt.condition, r);
            for (int i = 0; i < node->if_stmt.success_count; i++) {
                resolve(node->if_stmt.success_statements[i], r);
            }
            if (node->if_stmt.fail_statements) {
                for (int i = 0; i < node->if_stmt.fail_count; i++) {
                    resolve(node->if_stmt.fail_statements[i], r);
                }
            }
            break;
        case AST_WHILE:
            resolve(node->while_stmt.condition, r);
            for (int i = 0; i < node->while_stmt.statements_count; i++) {
                resolve(node->while_stmt.statements[i], r);
            }
            break;
        case AST_BINARY_OP:
            resolve(node->binary_op.left, r);
            resolve(node->binary_op.right, r);
            break;
        case AST_COMPOUND_ASSIGNMENT:
            resolve(node->compound_assignment.value, r);
            node->compound_assignment.slot = resolver_lookup(r, node->compound_assignment.name);
            if (node->compound_assignment.slot == -1) {
                fprintf(stderr, "undefined variable `%s` when trying to reassign\n", node->var_assign.name);
                exit(1);
            }
            node->var_type = r->types[node->compound_assignment.slot];
            break;
        case AST_UNARY_OP:
            resolve(node->unary_op.right, r);
            break;
        case AST_ARRAY_INDEX:
            resolve(node->array_index.array_expr, r);
            resolve(node->array_index.index_expr, r);
            break;
        case AST_ARRAY_INDEX_ASSIGN:
            resolve(node->array_assign_expr.arr_index_expr, r);
            resolve(node->array_assign_expr.value, r);
            break;
        case AST_ARRAY:
            for (int i = 0; i < node->array_literal.len; i++) {
                resolve(node->array_literal.arr[i], r);
            }
            break;
    }
}

void free_resolver(Resolver* r) {
    for (int i = 0; i < r->count; i++) {
        free(r->names[i]);
    }
    free(r->names);

    free(r->types);

    // r itself is not free'd here as it's intended to be initalized as a stack value and as such will crash if we try to free it
}
