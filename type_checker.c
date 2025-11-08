#include "type_checker.h"
#include "lexer.h"
#include "parser.h"
#include "resolver.h"
#include <stdio.h>
#include <stdlib.h>

VarType get_expr_type(ASTNode* node, Resolver* r) {
    if (!node) return VALUE_UNKNOWN;

    switch (node->type) {
        case AST_INT:
            return VALUE_INT;
        case AST_BOOL:
            return VALUE_BOOL;
        case AST_BINARY_OP: {
            TokenType op = node->binary_op.op;
            if (op == TOK_PLUS || op == TOK_MINUS || op == TOK_MULT || op == TOK_DIV) {
                return VALUE_INT;
            } else if (op == TOK_LESS || op == TOK_GREATER || op == TOK_EQUALS) {
                return VALUE_BOOL;
            }
            return VALUE_UNKNOWN;
        }
        case AST_UNARY_OP: {
            TokenType op = node->unary_op.op;
            if (op == TOK_MINUS) {
                return VALUE_INT;
            } else if (op == TOK_EXCLAM) {
                return VALUE_BOOL;
            }
            return VALUE_UNKNOWN;
        }
        case AST_VAR_REF: {
            int slot = resolver_lookup(r, node->var_ref.name);
            if (slot == -1) {
                return VALUE_UNKNOWN;
            } else {
                return r->types[slot];
            }
        }
        default:
            return VALUE_UNKNOWN;
    }
}

void type_check(ASTNode *node, Resolver* r) {
    if (!node) return;

    switch (node->type) {
    case AST_BINARY_OP: {
        type_check(node->binary_op.left, r);
        type_check(node->binary_op.right, r);

        VarType left_type = get_expr_type(node->binary_op.left, r);
        VarType right_type = get_expr_type(node->binary_op.right, r);
        switch (node->binary_op.op) {
            case TOK_PLUS:
            case TOK_MINUS:
            case TOK_MULT:
            case TOK_DIV:
            case TOK_LESS:
            case TOK_GREATER:
                if (left_type != VALUE_INT || right_type != VALUE_INT) {
                    fprintf(stderr, "error: cannot use %s operator on %s and %s\n",
                        op_string(node->binary_op.op),
                        var_type_string(left_type),
                        var_type_string(right_type)
                    );
                    exit(1);
                }
                break;
            case TOK_EQUALS:
                if (
                    (left_type != VALUE_BOOL || right_type != VALUE_BOOL) &&
                    (left_type != VALUE_INT || right_type != VALUE_INT)
                ) {
                    fprintf(stderr, "error: cannot use %s operator on %s and %s\n",
                        op_string(node->binary_op.op),
                        var_type_string(left_type),
                        var_type_string(right_type)
                    );
                    exit(1);
                }
                break;
            default:
                break;
        }
        break;
    }
    case AST_UNARY_OP: {
        type_check(node->unary_op.right, r);
        VarType right_type = get_expr_type(node->unary_op.right, r);

        if (node->unary_op.op == TOK_EXCLAM && right_type != VALUE_BOOL) {
            fprintf(stderr, "error: cannot use ! operator on %s\n",
                var_type_string(right_type)
            );
            exit(1);
        } else if (node->unary_op.op == TOK_MINUS && right_type != VALUE_INT) {
            fprintf(stderr, "error: cannot use unary - operator on %s\n",
                var_type_string(right_type)
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
        if (node->var_decl.type_inferred) {
            // we can assume an inferred type is correct, and as such don't need to type check
            break;
        }
        VarType var_type = node->var_type;
        VarType value_type = get_expr_type(node->var_decl.value, r);
        if (var_type != value_type) {
            fprintf(stderr, "error: cannot assign %s to variable `%s` that was explicitly declared as type %s\n",
                var_type_string(value_type),
                node->var_decl.name,
                var_type_string(var_type)
            );
            exit(1);
        }
        break;
    }
    case AST_VAR_ASSIGN: {
        type_check(node->var_assign.value, r);
        VarType var_type = node->var_type;
        VarType value_type = get_expr_type(node->var_assign.value, r);
        if (var_type != value_type) {
            fprintf(stderr, "error: cannot assign %s to variable `%s` of type %s\n",
                var_type_string(value_type),
                node->var_assign.name,
                var_type_string(var_type)
            );
            exit(1);
        }
        break;
    }
    case AST_IF: {
        type_check(node->if_stmt.condition, r);
        VarType condition_type = get_expr_type(node->if_stmt.condition, r);

        if (condition_type != VALUE_BOOL) {
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
    default:
        break;
    }
}
