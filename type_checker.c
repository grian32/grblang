#include "type_checker.h"
#include "lexer.h"
#include "parser.h"
#include "resolver.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

VarType get_expr_type(ASTNode* node, Resolver* r) {
    VarType unknown_type = {.base_type = VALUE_UNKNOWN, .nested = -1};
    VarType int_type = {.base_type = VALUE_INT, .nested = -1};
    VarType bool_type = {.base_type = VALUE_BOOL, .nested = -1};
    VarType string_type = {.base_type = VALUE_STRING, .nested = -1};
    if (!node) return unknown_type;

    switch (node->type) {
        case AST_INT:
            return int_type;
        case AST_BOOL:
            return bool_type;
        case AST_STRING:
            return string_type;
        case AST_ARRAY:
            if (node->array_literal.len == 0) {
                return unknown_type;
            }
            VarType first_elem_type = get_expr_type(node->array_literal.arr[0], r);
            for (int i = 1; i < node->array_literal.len; i++) {
                VarType elem_type = get_expr_type(node->array_literal.arr[0], r);
                if (elem_type.base_type != first_elem_type.base_type || elem_type.nested != first_elem_type.nested) {
                    fprintf(stderr, "error: array elements must be of the same type\n");
                    exit(1);
                }
            }

            VarType array_type = first_elem_type;
            array_type.nested++;
            return array_type;

        case AST_BINARY_OP: {
            TokenType op = node->binary_op.op;
            BaseType left_type = get_expr_type(node->binary_op.left, r).base_type;
            BaseType right_type = get_expr_type(node->binary_op.left, r).base_type;
            if (op == TOK_PLUS) {
                if (left_type == VALUE_INT && right_type == VALUE_INT) {
                    return int_type;
                } else if (left_type == VALUE_STRING && right_type == VALUE_STRING) {
                    return string_type;
                }
            }

            if (op == TOK_MINUS || op == TOK_MULT || op == TOK_DIV || op == TOK_MODULO) {
                return int_type;
            } else if (op == TOK_LESS || op == TOK_GREATER || op == TOK_EQUALS || op == TOK_NOT_EQUALS || op == TOK_GREATER_EQUALS || op == TOK_LESS_EQUALS || op == TOK_AND || op == TOK_OR) {
                return bool_type;
            }
            return unknown_type;
        }
        case AST_UNARY_OP: {
            TokenType op = node->unary_op.op;
            if (op == TOK_MINUS) {
                return int_type;
            } else if (op == TOK_EXCLAM) {
                return bool_type;
            }
            return unknown_type;
        }
        case AST_VAR_REF: {
            int slot = resolver_lookup(r, node->var_ref.name);
            if (slot == -1) {
                return unknown_type;
            } else {
                return r->types[slot];
            }
        }
        default:
            return unknown_type;
    }
}

void type_check(ASTNode *node, Resolver* r) {
    if (!node) return;

    switch (node->type) {
    case AST_BINARY_OP: {
        type_check(node->binary_op.left, r);
        type_check(node->binary_op.right, r);

        VarType left_var_type = get_expr_type(node->binary_op.left, r);
        VarType right_var_type = get_expr_type(node->binary_op.right, r);
        if (left_var_type.nested != -1 || right_var_type.nested != -1) {
            fprintf(stderr, "binary operation %s not allowed on array", op_string(node->binary_op.op));
            exit(1);
        }
        BaseType left_type = get_expr_type(node->binary_op.left, r).base_type;
        BaseType right_type = get_expr_type(node->binary_op.right, r).base_type;
        switch (node->binary_op.op) {
            case TOK_MINUS:
            case TOK_MULT:
            case TOK_DIV:
            case TOK_LESS:
            case TOK_LESS_EQUALS:
            case TOK_GREATER:
            case TOK_GREATER_EQUALS:
            case TOK_MODULO:
                if (left_type != VALUE_INT || right_type != VALUE_INT) {
                    fprintf(stderr, "error: cannot use %s operator on %s and %s\n",
                        op_string(node->binary_op.op),
                        base_type_string(left_type),
                        base_type_string(right_type)
                    );
                    exit(1);
                }
                break;
            case TOK_PLUS:
                if (
                    (left_type != VALUE_INT || right_type != VALUE_INT) &&
                    (left_type != VALUE_STRING || right_type != VALUE_STRING)
                ) {
                    fprintf(stderr, "error: cannot use %s operator on %s and %s\n",
                        op_string(node->binary_op.op),
                        base_type_string(left_type),
                        base_type_string(right_type)
                    );
                    exit(1);
                }
                break;
            case TOK_NOT_EQUALS:
            case TOK_EQUALS:
                if (
                    (left_type != VALUE_BOOL || right_type != VALUE_BOOL) &&
                    (left_type != VALUE_INT || right_type != VALUE_INT)
                ) {
                    fprintf(stderr, "error: cannot use %s operator on %s and %s\n",
                        op_string(node->binary_op.op),
                        base_type_string(left_type),
                        base_type_string(right_type)
                    );
                    exit(1);
                }
                break;
            case TOK_AND:
            case TOK_OR:
                if (left_type != VALUE_BOOL || right_type != VALUE_BOOL) {
                    fprintf(stderr, "error: cannot use %s operator on %s and %s\n",
                        op_string(node->binary_op.op),
                        base_type_string(left_type),
                        base_type_string(right_type)
                    );
                    exit(1);
                }

            default:
                break;
        }
        break;
    }
    case AST_UNARY_OP: {
        type_check(node->unary_op.right, r);
        VarType right_var_type = get_expr_type(node->unary_op.right, r);
        if (right_var_type.nested != -1) {
            fprintf(stderr, "unary operation %s not allowed on array", op_string(node->binary_op.op));
            exit(1);
        }
        BaseType right_type = right_var_type.base_type;

        if (node->unary_op.op == TOK_EXCLAM && right_type != VALUE_BOOL) {
            fprintf(stderr, "error: cannot use ! operator on %s\n",
                base_type_string(right_type)
            );
            exit(1);
        } else if (node->unary_op.op == TOK_MINUS && right_type != VALUE_INT) {
            fprintf(stderr, "error: cannot use unary - operator on %s\n",
                base_type_string(right_type)
            );
            exit(1);
        }
        break;
    }
    case AST_PROGRAM:
        for (int i = 0; i < node->program.count; i++) {
            type_check(node->program.statements[i], r);
        }
        break;
    case AST_VAR_DECL: {
        type_check(node->var_decl.value, r);
        VarType var_type = node->var_type;
        VarType value_type = get_expr_type(node->var_decl.value, r);
        if (var_type.base_type != value_type.base_type || var_type.nested != value_type.nested) {
            char value_buffer[50];
            char var_buffer[50];
            var_type_string(value_type, value_buffer);
            var_type_string(var_type, var_buffer);
            fprintf(stderr, "error: cannot assign %s to variable `%s` that was explicitly declared as type %s\n",
                value_buffer,
                node->var_decl.name,
                var_buffer
            );
            exit(1);
        }
        break;
    }
    case AST_VAR_ASSIGN: {
        type_check(node->var_assign.value, r);
        VarType var_type = node->var_type;
        VarType value_type = get_expr_type(node->var_assign.value, r);
        if (var_type.base_type != value_type.base_type || var_type.nested != value_type.nested) {
            char value_buffer[50];
            char var_buffer[50];
            var_type_string(value_type, value_buffer);
            var_type_string(var_type, var_buffer);
            fprintf(stderr, "error: cannot assign %s to variable `%s` of type %s\n",
                value_buffer,
                node->var_assign.name,
                var_buffer
            );
            exit(1);
        }
        break;
    }
    case AST_COMPOUND_ASSIGNMENT: {
        type_check(node->compound_assignment.value, r);
        VarType var_type = node->var_type;
        VarType value_type = get_expr_type(node->compound_assignment.value, r);
        if (var_type.base_type != value_type.base_type || var_type.nested != value_type.nested) {
            char value_buffer[50];
            char var_buffer[50];
            var_type_string(value_type, value_buffer);
            var_type_string(var_type, var_buffer);
            fprintf(stderr, "error: cannot assign %s to variable `%s` of type %s\n",
                value_buffer,
                node->compound_assignment.name,
                var_buffer
            );
            exit(1);
        }
        break;
    }
    case AST_IF: {
        type_check(node->if_stmt.condition, r);
        VarType condition_type = get_expr_type(node->if_stmt.condition, r);

        if (condition_type.base_type != VALUE_BOOL || condition_type.nested != -1) {
            fprintf(stderr, "error: cannot use a non-bool as if condition\n");
            exit(1);
        }

        for (int i = 0; i < node->if_stmt.success_count; i++) {
            type_check(node->if_stmt.success_statements[i], r);
        }

        if (node->if_stmt.fail_statements) {
            for (int i = 0; i < node->if_stmt.fail_count; i++) {
                type_check(node->if_stmt.fail_statements[i], r);
            }
        }

        break;
    }
    case AST_WHILE: {
        type_check(node->while_stmt.condition, r);
        VarType condition_type = get_expr_type(node->while_stmt.condition, r);

        if (condition_type.base_type != VALUE_BOOL || condition_type.nested != -1) {
            fprintf(stderr, "error: cannot use a non-bool as while condition\n");
            exit(1);
        }

        for (int i = 0; i < node->while_stmt.statements_count; i++) {
            type_check(node->while_stmt.statements[i], r);
        }
    }
    case AST_ARRAY_INDEX: {
        type_check(node->array_index.array_expr, r);
        if (node->array_index.array_expr->type != AST_VAR_REF && node->array_index.array_expr->type != AST_ARRAY_INDEX) {
            fprintf(stderr, "error: array expression in array index node must be either variable reference or array index\n");
            exit(1);
        }

        VarType index_expr = get_expr_type(node->array_index.index_expr, r);
        if (index_expr.base_type != VALUE_INT || index_expr.nested != -1) {
            fprintf(stderr, "error: index expression in array index node must be integer\n");
            exit(1);
        }
    }
    default:
        break;
    }
}
