#include "parser.h"
#include "lexer.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

VarType data_to_var_type(DataType tt) {
    switch (tt) {
        case DATA_INT:
            VarType int_type = { .base_type = VALUE_INT, .nested = -1 };
            return int_type;
        case DATA_BOOL:
            VarType bool_type = { .base_type = VALUE_BOOL, .nested = -1 };
            return bool_type;
        case DATA_STRING:
            VarType string_type = { .base_type = VALUE_STRING, .nested = -1 };
            return string_type;
        default:
            VarType unknown_type = { .base_type = VALUE_UNKNOWN, .nested = -1 };
            return unknown_type;
    }
}

void parser_init(Parser* p, Lexer* l) {
    p->lexer = l;

    p->curr.type = TOK_EOF;
    p->peek = lex_next(p->lexer);
    parser_next(p);
}

void parser_next(Parser* p) {
    p->curr = p->peek;
    p->peek = lex_next(p->lexer);
}

char* op_string(TokenType op) {
    switch (op) {
        case TOK_EQUALS: return "==";
        case TOK_NOT_EQUALS: return "!=";
        case TOK_GREATER: return ">";
        case TOK_LESS: return "<";
        case TOK_LESS_EQUALS: return "<=";
        case TOK_GREATER_EQUALS: return ">=";
        case TOK_AND: return "&&";
        case TOK_OR: return "||";
        case TOK_DIV: return "/";
        case TOK_DIV_EQUALS: return "/=";
        case TOK_MULT: return "*";
        case TOK_MULT_EQUALS: return "*=";
        case TOK_PLUS: return "+";
        case TOK_PLUS_EQUALS: return "+=";
        case TOK_MINUS: return "-";
        case TOK_MINUS_EQUALS: return "-=";
        case TOK_MODULO: return "%";

        default: return "";
    }
}

char* base_type_string(BaseType type) {
    // TODO: array shit
    switch (type) {
    case VALUE_INT:
        return "int";
    case VALUE_BOOL:
        return "bool";
    case VALUE_STRING:
        return "string";
    case VALUE_UNKNOWN:
        return "unknown";
    default:
        return "fucked";
    }
}

void var_type_string(VarType type, char buffer[50]) {
    switch (type.base_type) {
    case VALUE_INT:
        sprintf(buffer, "VAR_TYPE_INT(NESTED=%d)", type.nested);
        break;
    case VALUE_BOOL:
        sprintf(buffer, "VAR_TYPE_BOOL(NESTED=%d)", type.nested);
        break;
    case VALUE_STRING:
        sprintf(buffer, "VAR_TYPE_STRING(NESTED=%d)", type.nested);
        break;
    case VALUE_UNKNOWN:
        sprintf(buffer, "VAR_TYPE_UNKNOWN(NESTED=%d)", type.nested);
        break;
    default:
        sprintf(buffer, "fucked");
        break;
    }
}

void print_ast(ASTNode* node, int indent, bool newline) {
    if (!node) return;
    for (int i = 0; i < indent; i++) {
        printf(" ");
    }

    switch (node->type) {
        case AST_INT:
            printf("AST_INT(%d)",node->int_val);
            if (newline) {
                printf("\n");
            }
            break;
        case AST_BOOL:
            printf("AST_BOOL(%s)", node->bool_val ? "true" : "false");
            if (newline) {
                printf("\n");
            }
            break;
        case AST_STRING:
            printf("AST_STRING(%s)", node->string.string_val);
            if (newline) {
                printf("\n");
            }
            break;
        case AST_BINARY_OP:
            printf("AST_BINARY_OP(%s)", op_string(node->binary_op.op));
            if (newline) {
                printf("\n");
            }
            print_ast(node->binary_op.left, indent + 1, false);
            print_ast(node->binary_op.right, indent + 1, false);
            break;
        case AST_COMPOUND_ASSIGNMENT: {
            printf("AST_COMPOUND_ASSIGN(slot=%d,%s%s", node->compound_assignment.slot, node->compound_assignment.name, op_string(node->compound_assignment.op));
            print_ast(node->compound_assignment.value, 0, false);
            char buffer[50];
            var_type_string(node->var_type, buffer);
            printf(";t=%s(%d))", buffer, node->var_type.base_type);
            if (newline) {
                printf("\n");
            }
            break;
        }
        case AST_UNARY_OP:
            printf("AST_UNARY_OP(%c)",node->unary_op.op == TOK_MINUS ? '-' : '!');
            if (newline) {
                printf("\n");
            }
            print_ast(node->unary_op.right, indent+1, true);
            break;
        case AST_PROGRAM:
            printf("AST_PROGRAM");
            if (newline) {
                printf("\n");
            }
            for (int i = 0; i < node->program.count; i++) {
                print_ast(node->program.statements[i], indent+1, true);
            }
            break;
        case AST_IF:
            printf("AST_IF(condition =");
            print_ast(node->if_stmt.condition, indent, false);
            printf(", success =");
            for (int i = 0; i < node->if_stmt.success_count; i++) {
                print_ast(node->if_stmt.success_statements[i], indent, false);
                printf(";");
            }
            if (node->if_stmt.fail_statements) {
                printf(", fail =");
                for (int i = 0; i < node->if_stmt.fail_count; i++) {
                    print_ast(node->if_stmt.fail_statements[i], indent, false);
                    printf(";");
                }
            }
            printf(")");
            if (newline) {
                printf("\n");
            }
            break;
        case AST_WHILE:
            printf("AST_WHILE(condition =");
            print_ast(node->while_stmt.condition, indent, false);
            printf(", statements =");

            for (int i = 0; i < node->while_stmt.statements_count; i++) {
                print_ast(node->while_stmt.statements[i], indent, false);
                printf(";");
            }

            printf(")");
            if (newline) {
                printf("\n");
            }
            break;
        case AST_VAR_ASSIGN: {
            printf("AST_VAR_ASSIGN(slot=%d,%s=", node->var_assign.slot, node->var_assign.name);
            print_ast(node->var_assign.value, 0, false);
            char buffer[50];
            var_type_string(node->var_type, buffer);
            printf(";t=%s(%d))", buffer, node->var_type.base_type);
            if (newline) {
                printf("\n");
            }
            break;
        }
        case AST_VAR_DECL: {
            printf("AST_VAR_DECL(slot=%d,%s=", node->var_assign.slot,node->var_decl.name);
            print_ast(node->var_decl.value, 0, false);
            char buffer[50];
            var_type_string(node->var_type, buffer);
            printf(";t=%s(%d))", buffer, node->var_type.base_type);
            if (newline) {
                printf("\n");
            }
            break;
        }
        case AST_VAR_REF: {
            char buffer[50];
            var_type_string(node->var_type, buffer);
            printf("AST_VAR_REF(slot=%d,%s;t=%s(%d))", node->var_ref.slot, node->var_ref.name, buffer, node->var_type.base_type);
            if (newline) {
                printf("\n");
            }
            break;
        }
        case AST_ARRAY: {
            printf("AST_ARRAY([");
            for (int i = 0; i < node->array_literal.len; i++) {
                print_ast(node->array_literal.arr[i], indent, false);
                if (i != node->array_literal.len - 1) {
                    printf(", ");
                }
            }
            printf("])");
            if (newline) {
                printf("\n");
            }
            break;
        }
        case AST_ARRAY_INDEX: {
            printf("AST_ARRAY_INDEX(");
            print_ast(node->array_index.array_expr, 0, false);
            printf("[");
            print_ast(node->array_index.index_expr, 0, false);
            printf("])");
            if (newline) {
                printf("\n");
            }
            break;
        }
        case AST_ARRAY_INDEX_ASSIGN: {
            printf("AST_ARRAY_INDEX_ASSIGN(");
            print_ast(node->array_assign_expr.arr_index_expr, 0, false);
            printf("=");
            print_ast(node->array_assign_expr.value, 0, false);
            printf(")");
            if (newline) {
                printf("\n");
            }
            break;
        }
        case AST_FUNCTION_CALL: {
            printf("AST_FUNCTION_CALL(%s(", node->function_call.name);
            for (int i = 0; i < node->function_call.args_len; i++) {
                print_ast(node->function_call.args[i], indent, false);
                if (i != node->function_call.args_len - 1) {
                    printf(", ");
                }
            }
            printf("))");
            if (newline) {
                printf("\n");
            }
            break;
        }
        case AST_FUNCTION_DECL: {
            printf("AST_FUNCTION_DECL(%s(", node->function_decl.name);
            for (int i = 0; i < node->function_decl.params_len; i++) {
                FunctionParam param = node->function_decl.params[i];
                printf("%s(%d) %s", base_type_string(param.type.base_type), param.type.nested, param.name);
                if (i != node->function_decl.params_len - 1) {
                    printf(", ");
                }
            }
            printf(") ");
            printf("{");
            for (int i = 0; i < node->function_decl.stmts_len; i++) {
                print_ast(node->function_decl.stmts[i], indent, false);
                printf(";");
            }
            printf("}");
            if (newline) {
                printf("\n");
            }
            break;
        }
        case AST_RETURN_STMT: {
            printf("AST_RETURN(");
            print_ast(node->return_stmt.expr, indent, false);
            printf(")");
            if (newline) {
                printf("\n");
            }
            break;
        }
        default:
            printf("unknown ast type: %d\n", node->type);
    }
}

ASTNode* make_int(int value) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_INT;
    node->int_val = value;
    return node;
}

ASTNode* make_true_bool() {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_BOOL;
    node->bool_val = true;
    return node;
}

ASTNode* make_false_bool() {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_BOOL;
    node->bool_val = false;
    return node;
}

ASTNode* make_string(char* str_val, int len) {
    ASTNode* node = malloc(sizeof(ASTNode));

    node->type = AST_STRING;
    node->string.string_val = str_val;
    node->string.len = len;

    return node;
}

ASTNode* make_arr_literal(ASTNode** exprs, int len) {
    ASTNode* node = malloc(sizeof(ASTNode));

    node->type = AST_ARRAY;
    node->array_literal.arr = exprs;
    node->array_literal.len = len;

    return node;
}

ASTNode* make_arr_index(ASTNode* var_ref, ASTNode* index_expr) {
    ASTNode* node = malloc(sizeof(ASTNode));

    node->type = AST_ARRAY_INDEX;
    node->array_index.array_expr = var_ref;
    node->array_index.index_expr = index_expr;

    return node;
}

ASTNode* make_arr_index_assign(ASTNode* arr_index, ASTNode* value) {
    ASTNode* node = malloc(sizeof(ASTNode));

    node->type = AST_ARRAY_INDEX_ASSIGN;
    node->array_assign_expr.arr_index_expr = arr_index;
    node->array_assign_expr.value = value;

    return node;
}

ASTNode* make_binary_op(TokenType op, ASTNode* left, ASTNode* right) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = AST_BINARY_OP;
    node->binary_op.op = op;
    node->binary_op.left = left;
    node->binary_op.right = right;
    return node;
}

ASTNode* make_compound_assignment(TokenType op, char* name, ASTNode* value) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = AST_COMPOUND_ASSIGNMENT;
    node->compound_assignment.op = op;
    node->compound_assignment.name = name;
    node->compound_assignment.value = value;
    return node;
}

ASTNode* make_unary_op(TokenType op, ASTNode* right) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = AST_UNARY_OP;
    node->unary_op.op = op;
    node->unary_op.right = right;
    return node;
}

ASTNode* make_program(ASTNode** statements, int count) {
    ASTNode** new_statements = realloc(statements, count * sizeof(ASTNode*));
    if (!new_statements) {
        fprintf(stderr, "failed to realloc statements arr in make_program");
        exit(1);
    }

    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = AST_PROGRAM;
    node->program.statements = new_statements;
    node->program.count = count;

    return node;
}

ASTNode* make_var_decl(char* name, ASTNode* value, VarType type) {
    ASTNode* node = malloc(sizeof(ASTNode));

    node->type = AST_VAR_DECL;
    node->var_decl.name = name;
    node->var_decl.value = value;

    node->var_type = type;

    return node;
}

ASTNode* make_var_assign(char* name, ASTNode* value) {
    ASTNode* node = malloc(sizeof(ASTNode));

    node->type = AST_VAR_ASSIGN;
    node->var_assign.name = name;
    node->var_assign.value = value;

    return node;
}

ASTNode* make_var_ref(char* name) {
    ASTNode* node = malloc(sizeof(ASTNode));

    node->type = AST_VAR_REF;
    node->var_ref.name = name;

    return node;
}


ASTNode* make_if_statement(ASTNode* condition, ASTNode** success_statements, int success_count, ASTNode** fail_statements, int fail_count) {
    ASTNode* node = malloc(sizeof(ASTNode));

    node->type = AST_IF;
    node->if_stmt.condition = condition;
    node->if_stmt.success_statements = success_statements;
    node->if_stmt.success_count = success_count;
    node->if_stmt.fail_statements = fail_statements;
    node->if_stmt.fail_count = fail_count;

    return node;
}

ASTNode* make_while_statement(ASTNode* condition, ASTNode** statements, int statements_count) {
    ASTNode* node = malloc(sizeof(ASTNode));

    node->type = AST_WHILE;
    node->while_stmt.condition = condition;
    node->while_stmt.statements = statements;
    node->while_stmt.statements_count = statements_count;

    return node;
}

ASTNode* make_function_call(ASTNode** args, int args_len, char* value) {
    ASTNode* node = malloc(sizeof(ASTNode));

    node->type = AST_FUNCTION_CALL;
    ASTNode** new_args = realloc(args, sizeof(ASTNode*) * args_len);
    if (!new_args) {
        fprintf(stderr, "failed to realloc arguments array when making function call\n");
        exit(1);
    }

    node->function_call.args = new_args;
    node->function_call.args_len = args_len;
    node->function_call.name = value;

    return node;
}

ASTNode* make_function_decl(ASTNode** stmts, int stmts_len, FunctionParam* params, int param_len, VarType return_type, char* name) {
    ASTNode* node = malloc(sizeof(ASTNode));

    node->type = AST_FUNCTION_DECL;
    node->function_decl.return_type = return_type;
    node->function_decl.name = name;
    node->function_decl.stmts_len = stmts_len;
    node->function_decl.params_len = param_len;
    ASTNode** new_stmts = realloc(stmts, stmts_len * sizeof(ASTNode*));
    if (!new_stmts) {
        fprintf(stderr, "failed to realloc stmts when making function decl\n");
        exit(1);
    }
    node->function_decl.stmts = new_stmts;

    FunctionParam* new_params = realloc(params, param_len * sizeof(FunctionParam));
    if (!new_params) {
        fprintf(stderr, "failed to realloc params when making function decl\n");
        exit(1);
    }
    node->function_decl.params = new_params;

    return node;
}

ASTNode* make_return_stmt(ASTNode* expr) {
    ASTNode* node = malloc(sizeof(ASTNode));

    node->type = AST_RETURN_STMT;
    node->return_stmt.expr = expr;

    return node;
}

ASTNode* parse_primary(Parser* p) {
    if (p->curr.type == TOK_INT) {
        ASTNode* n = make_int(p->curr.value.int_val);
        parser_next(p);
        return n;
    }

    if (p->curr.type == TOK_TRUE) {
        ASTNode* n = make_true_bool();
        parser_next(p);
        return n;
    }

    if (p->curr.type == TOK_FALSE) {
        ASTNode* n = make_false_bool();
        parser_next(p);
        return n;
    }

    if (p->curr.type == TOK_STRING) {
        ASTNode* n = make_string(p->curr.value.string_val, p->curr.length);
        parser_next(p);
        return n;
    }

    if (p->curr.type == TOK_IDENT) {
        char* name = p->curr.value.ident_val;
        parser_next(p);
        if (p->curr.type == TOK_LPAREN) {
            return parse_function_call(p, name);
        }

        ASTNode* node = make_var_ref(name);

        while (p->curr.type == TOK_LBRACKET) {
            parser_next(p);
            ASTNode* index_expr = parse_expr(p);
            if (p->curr.type != TOK_RBRACKET) {
                fprintf(stderr, "expected ] after [ in array index\n");
                exit(1);
            }
            parser_next(p);
            node = make_arr_index(node, index_expr);
        }

        if (p->curr.type == TOK_ASSIGN) {
            parser_next(p);
            ASTNode* value = parse_expr(p);

            return make_arr_index_assign(node, value);
        } else {
            return node;
        }

        return node;
    }

    if (p->curr.type == TOK_LPAREN) {
        parser_next(p);
        ASTNode* expr = parse_expr(p);
        if (p->curr.type != TOK_RPAREN) {
            fprintf(stderr, "unexpected token (likely unmatched parens) in parse_primary\n");
            exit(1);
            // TODO: actual error handling
        }
        parser_next(p);
        return expr;
    }

    if (p->curr.type == TOK_LBRACKET) {
        return parse_array_literal(p);
    }

    fprintf(stderr, "unexpected token %d in parse_primary\n", p->curr.type);
    exit(1);
}

ASTNode* parse_unary(Parser* p) {
    if (p->curr.type == TOK_MINUS || p->curr.type == TOK_EXCLAM) {
        TokenType op = p->curr.type;
        parser_next(p);
        ASTNode* right = parse_unary(p);
        return make_unary_op(op, right);
    }

    return parse_primary(p);
}

ASTNode* parse_logical_or(Parser* p) {
    ASTNode* left = parse_logical_and(p);

    while (p->curr.type == TOK_OR) {
        parser_next(p);
        ASTNode* right = parse_logical_and(p);
        left = make_binary_op(TOK_OR, left, right);
    }

    return left;
}

ASTNode* parse_logical_and(Parser* p) {
    ASTNode* left = parse_comparison(p);

    while (p->curr.type == TOK_AND) {
        parser_next(p);
        ASTNode* right = parse_comparison(p);
        left = make_binary_op(TOK_AND, left, right);
    }

    return left;
}

ASTNode* parse_comparison(Parser* p) {
    ASTNode* left = parse_addsub(p);

    if (p->curr.type == TOK_LESS || p->curr.type == TOK_GREATER || p->curr.type == TOK_EQUALS || p->curr.type == TOK_NOT_EQUALS || p->curr.type == TOK_GREATER_EQUALS || p->curr.type == TOK_LESS_EQUALS) {
        TokenType op = p->curr.type;
        parser_next(p);
        ASTNode* right = parse_addsub(p);
        return make_binary_op(op, left, right);
    }

    return left;
}

ASTNode* parse_muldiv(Parser* p) {
    ASTNode* left = parse_unary(p);

    while (p->curr.type == TOK_MULT || p->curr.type == TOK_DIV || p->curr.type == TOK_MODULO) {
        TokenType op = p->curr.type;
        parser_next(p);
        ASTNode* right = parse_unary(p);
        left = make_binary_op(op, left, right);
    }

    return left;
}

ASTNode* parse_addsub(Parser* p) {
    ASTNode* left = parse_muldiv(p);

    while (p->curr.type == TOK_PLUS || p->curr.type == TOK_MINUS) {
        TokenType op = p->curr.type;
        parser_next(p);
        ASTNode* right = parse_muldiv(p);
        left = make_binary_op(op, left, right);
    }

    return left;
}

ASTNode* parse_expr(Parser* p) {
    return parse_logical_or(p);
}

VarType parse_type(Parser* p) {
    if (p->curr.type != TOK_TYPE) {
        printf("%d\n", p->curr.type);
        fprintf(stderr, "expected type ex:`bool`, `int` after keyword in declaration\n");
        exit(1);
    }
    VarType var_type = data_to_var_type(p->curr.value.type_val);
    parser_next(p);

    while (p->curr.type == TOK_LBRACKET) {
        parser_next(p);
        if (p->curr.type != TOK_RBRACKET) {
            fprintf(stderr, "expected ] after [ in decl type parsing");
            exit(1);
        }
        parser_next(p);
        var_type.nested++;
    }

    return var_type;
}

ASTNode* parse_var_decl(Parser* p) {
    parser_next(p);
    VarType var_type = parse_type(p);

    if (p->curr.type != TOK_IDENT) {
        fprintf(stderr, "expected identifier after type in var declaration\n");
        exit(1);
    }
    char* name = p->curr.value.ident_val;
    parser_next(p);

    if (p->curr.type != TOK_ASSIGN) {
        fprintf(stderr, "expected equals after type in var declaration\n");
        exit(1);
    }
    parser_next(p);

    ASTNode* val = parse_expr(p);
    return make_var_decl(name, val, var_type);
}


ASTNode* parse_statement(Parser *p) {
    if (p->curr.type == TOK_RETURN) {
        parser_next(p);
        ASTNode* expr = parse_expr(p);
        return make_return_stmt(expr);
    }

    if (p->curr.type == TOK_FN) {
        return parse_fn_decl(p);
    }

    if (p->curr.type == TOK_VAR) {
        return parse_var_decl(p);
    }

    if (p->curr.type == TOK_IDENT && p->peek.type == TOK_ASSIGN) {
        char* name = p->curr.value.ident_val;
        parser_next(p);
        parser_next(p);

        ASTNode* val = parse_expr(p);
        return make_var_assign(name, val);
    }

    if (p->curr.type == TOK_IDENT && (p->peek.type == TOK_PLUS_EQUALS || p->peek.type == TOK_MINUS_EQUALS || p->peek.type == TOK_MULT_EQUALS || p->peek.type == TOK_DIV_EQUALS)) {
        char* name = p->curr.value.ident_val;
        parser_next(p);
        TokenType op = p->curr.type; // we are at the assignment token as parse_expr skips
        parser_next(p);
        ASTNode* value = parse_expr(p);
        return make_compound_assignment(op, name, value);
    }

    if (p->curr.type == TOK_IF) {
        return parse_if_stmt(p);
    }

    if (p->curr.type == TOK_WHILE) {
        return parse_while_stmt(p);
    }

    return parse_expr(p);
}

ASTNode* parse_program(Parser *p) {
    int capacity = 128, count = 0;
    ASTNode** statements;
    parse_block(p, capacity, TOK_EOF, &statements, &count);

    return make_program(statements, count);
}

void parse_block(Parser* p, int initial_capacity, TokenType end_tok, ASTNode*** statements_out, int* count_out) {
    int count = 0;
    int capacity = initial_capacity;
    ASTNode** statements = malloc(sizeof(ASTNode*) * capacity);
    while (p->curr.type != end_tok) {
        if (count >= capacity) {
            capacity *= 2;
            ASTNode** new_statements = realloc(statements, sizeof(ASTNode*) * capacity);
            if (!new_statements) {
                fprintf(stderr, "failed to realloc statements arr\n");
                exit(1);
            }
            statements = new_statements;
        }

        statements[count++] = parse_statement(p);
        if (p->curr.type != TOK_SEMICOLON) {
            printf("%d", p->curr.type);
            fprintf(stderr, "semicolon expected after statement\n");
            exit(1);
        }
        parser_next(p);
    }

    *statements_out = statements;
    *count_out = count;
}

ASTNode* parse_if_stmt(Parser* p) {
    parser_next(p);
    ASTNode* condition = parse_expr(p);
    if (p->curr.type != TOK_LBRACE) {
        fprintf(stderr, "expected lbrace after if statement condition expression\n");
        exit(1);
    }
    parser_next(p);
    int success_capacity = 128, success_count = 0;
    ASTNode** success_statements;
    parse_block(p, success_capacity, TOK_RBRACE, &success_statements, &success_count);
    if (p->curr.type != TOK_RBRACE) {
        fprintf(stderr, "expected rbrace after if block in if statement\n");
        exit(1);
    }
    parser_next(p);

    if (p->curr.type != TOK_ELSE) {
        return make_if_statement(condition, success_statements, success_count, NULL, -1);
    }
    parser_next(p);
    if (p->curr.type != TOK_LBRACE) {
        fprintf(stderr, "expected lbrace after else keyword in if statement\n");
        exit(1);
    }
    parser_next(p);

    int fail_capacity = 128, fail_count = 0;
    ASTNode** fail_statements;
    parse_block(p, fail_capacity, TOK_RBRACE, &fail_statements, &fail_count);
    if (p->curr.type != TOK_RBRACE) {
        fprintf(stderr, "expected rbrace after else body in if statement\n");
        exit(1);
    }
    parser_next(p);

    return make_if_statement(condition, success_statements, success_count, fail_statements, fail_count);
}

ASTNode* parse_while_stmt(Parser* p) {
    parser_next(p);
    ASTNode* condition = parse_expr(p);
    if (p->curr.type != TOK_LBRACE) {
        fprintf(stderr, "expected lbrace after while statement condition\n");
        exit(1);
    }
    parser_next(p);
    int statements_capacity = 128, statements_count = 0;
    ASTNode** statements;
    parse_block(p, statements_capacity, TOK_RBRACE, &statements, &statements_count);
    if (p->curr.type != TOK_RBRACE) {
        fprintf(stderr, "expected rbrace after while statement block\n");
    }
    parser_next(p);

    return make_while_statement(condition, statements, statements_count);
}

ASTNode* parse_function_call(Parser* p, char* name) {
    parser_next(p);
    int capacity = 32;
    int size = 0;
    ASTNode** args = malloc(sizeof(ASTNode*) * capacity);

    while (p->curr.type != TOK_RPAREN) {
        ASTNode* arg = parse_expr(p);

        if (size >= capacity) {
            capacity *= 2;
            ASTNode** new_args = realloc(args, sizeof(ASTNode*) * capacity);
            if (!new_args) {
                fprintf(stderr, "failed to realloc when parsing function call\n");
                exit(1);
            }
            args = new_args;
        }

        if (p->curr.type == TOK_RPAREN) {
            args[size++] = arg;
            parser_next(p);
            return make_function_call(args, size, name);
        }

        if (p->curr.type != TOK_COMMA) {
            fprintf(stderr, "expected comma after argument in function call\n");
            exit(1);
        }

        args[size++] = arg;
    }

    if (p->curr.type != TOK_RPAREN) {
        fprintf(stderr, "expected ) after ( in function call\n");
        exit(1);
    }
    parser_next(p);

    return make_function_call(args, size, name);
}

ASTNode* parse_fn_decl(Parser* p) {
    parser_next(p);
    VarType return_type = parse_type(p);

    if (p->curr.type != TOK_IDENT) {
        fprintf(stderr, "expected identifier after type in function declaration\n");
        exit(1);
    }
    char* name = p->curr.value.ident_val;
    parser_next(p);

    if (p->curr.type != TOK_LPAREN) {
        fprintf(stderr, "expected ( after identifier in function declaration\n");
        exit(1);
    }
    parser_next(p);

    int args_capacity = 32;
    int args_size = 0;
    FunctionParam* args = malloc(sizeof(FunctionParam) * args_capacity);

    while (p->curr.type != TOK_RPAREN) {
        VarType type = parse_type(p);
        if (p->curr.type != TOK_IDENT) {
            fprintf(stderr, "expected identifier after type in function declaration arguments\n");
            exit(1);
        }
        char* arg_name = p->curr.value.ident_val;
        parser_next(p);

        if (args_size >= args_capacity) {
            args_capacity *= 2;
            FunctionParam* new_args = realloc(args, sizeof(FunctionParam) * args_capacity);
            if (!new_args) {
                fprintf(stderr, "failed to realloc when parsing function decl\n");
                exit(1);
            }
            args = new_args;
        }

        FunctionParam arg = { .type = type, .name = arg_name};
        if (p->curr.type == TOK_RPAREN) {
            args[args_size++] = arg;
            break;
        }

        if (p->curr.type != TOK_COMMA) {
            fprintf(stderr, "expected comma after argument in function decl\n");
            exit(1);
        }

        args[args_size++] = arg;
    }

    if (p->curr.type != TOK_RPAREN) {
        fprintf(stderr, "expected ) after ( in function decl\n");
        exit(1);
    }
    parser_next(p);

    if (p->curr.type != TOK_LBRACE) {
        fprintf(stderr, "expected { after argument list in function decl\n");
        exit(1);
    }
    parser_next(p);

    int stmts_capacity = 64;
    int stmts_size = 0;
    ASTNode** stmts = malloc(sizeof(ASTNode*) * stmts_capacity);
    parse_block(p, stmts_capacity, TOK_RBRACE, &stmts, &stmts_size);
    if (p->curr.type != TOK_RBRACE) {
        fprintf(stderr, "expected } after function decl block\n");
    }
    parser_next(p);

    return make_function_decl(stmts, stmts_size, args, args_size, return_type, name);
}

// TODO: add missing ast types to this
void free_ast(ASTNode* node) {
    if (!node) return;

    switch (node->type) {
    case AST_INT:
    case AST_BOOL:
        break;
    case AST_STRING:
        free(node->string.string_val);
        break;
    case AST_BINARY_OP:
        free_ast(node->binary_op.left);
        free_ast(node->binary_op.right);
        break;
    case AST_COMPOUND_ASSIGNMENT:
        free(node->compound_assignment.name);
        free_ast(node->compound_assignment.value);
        break;
    case AST_UNARY_OP:
        free_ast(node->unary_op.right);
        break;
    case AST_PROGRAM:
        for (int i = 0; i < node->program.count; i++) {
            free_ast(node->program.statements[i]);
        }
        free(node->program.statements);
        break;
    case AST_IF:
        free_ast(node->if_stmt.condition);

        for (int i = 0; i < node->if_stmt.success_count; i++) {
            free_ast(node->if_stmt.success_statements[i]);
        }
        free(node->if_stmt.success_statements);

        if (node->if_stmt.fail_statements) {
            for (int i = 0; i < node->if_stmt.fail_count; i++) {
                free_ast(node->if_stmt.fail_statements[i]);
            }
        }
        free(node->if_stmt.fail_statements);
        break;
    case AST_WHILE:
        free_ast(node->while_stmt.condition);

        for (int i = 0; i < node->while_stmt.statements_count; i++) {
            free_ast(node->while_stmt.statements[i]);
        }
        free(node->while_stmt.statements);
        break;
    case AST_VAR_DECL:
        free_ast(node->var_decl.value);
        free(node->var_decl.name);
        break;
    case AST_VAR_ASSIGN:
        free_ast(node->var_assign.value);
        free(node->var_assign.name);
        break;
    case AST_VAR_REF:
        free(node->var_ref.name);
        break;
    default:
      break;
    }

    free(node);
}

ASTNode* parse_array_literal(Parser* p) {
    parser_next(p);
    int capacity = 128, size = 0;
    ASTNode** exprs = malloc(sizeof(ASTNode*) * capacity);

    while (p->curr.type != TOK_RBRACKET) {
        ASTNode* expr = parse_expr(p);

        // otherwise will crash on last elem expecting comma :SSSSS
        if (size >= capacity) {
            capacity *= 2;
            ASTNode** new_exprs = realloc(exprs, sizeof(ASTNode*) * capacity);
            if (!new_exprs) {
                fprintf(stderr, "failed to reallocate memory in parse_array_literal\n");
                exit(1);
            }
            exprs = new_exprs;
        }

        exprs[size++] = expr;

        if (p->curr.type == TOK_RBRACKET) {
            parser_next(p);
            break;
        }
        if (p->curr.type != TOK_COMMA) {
            fprintf(stderr, "expected , after expression in array literal\n");
            exit(1);
        }
        parser_next(p);
    }

    return make_arr_literal(exprs, size);
}
