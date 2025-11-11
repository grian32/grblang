#include "parser.h"
#include "lexer.h"

#include <stdio.h>
#include <stdlib.h>

VarType tok_to_var_type(TokenType tt) {
    switch (tt) {
        case TOK_INT_TYPE:
            return VALUE_INT;
        case TOK_BOOL_TYPE:
            return VALUE_BOOL;
        default:
            return VALUE_UNKNOWN;
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
        case TOK_MULT: return "*";
        case TOK_PLUS: return "+";
        case TOK_MINUS: return "-";
        case TOK_MODULO: return "%";

        default: return "";
    }
}

char* var_type_string(VarType type) {
    switch (type) {
    case VALUE_INT:
        return "int";
    case VALUE_BOOL:
        return "bool";
    case VALUE_UNKNOWN:
        return "unknown";
    default:
        return "fucked";
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
        case AST_BINARY_OP:
            printf("AST_BINARY_OP(%s)", op_string(node->binary_op.op));
            if (newline) {
                printf("\n");
            }
            print_ast(node->binary_op.left, indent + 1, false);
            print_ast(node->binary_op.right, indent + 1, false);
            break;
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
        case AST_VAR_ASSIGN:
            printf("AST_VAR_ASSIGN(slot=%d,%s=", node->var_assign.slot, node->var_assign.name);
            print_ast(node->var_assign.value, 0, false);
            printf(";t=%s(%d))", var_type_string(node->var_type), node->var_type);
            if (newline) {
                printf("\n");
            }
            break;
        case AST_VAR_DECL:
            printf("AST_VAR_DECL(slot=%d,%s=", node->var_assign.slot,node->var_decl.name);
            print_ast(node->var_decl.value, 0, false);
            printf(";t=%s(%d))", var_type_string(node->var_type), node->var_type);
            if (newline) {
                printf("\n");
            }
            break;
        case AST_VAR_REF:
            printf("AST_VAR_REF(slot=%d,%s;t=%s(%d))", node->var_ref.slot, node->var_ref.name, var_type_string(node->var_type), node->var_type);
            if (newline) {
                printf("\n");
            }
            break;
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

ASTNode* make_binary_op(TokenType op, ASTNode* left, ASTNode* right) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = AST_BINARY_OP;
    node->binary_op.op = op;
    node->binary_op.left = left;
    node->binary_op.right = right;
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

    if (p->curr.type == TOK_IDENT) {
        char* name = p->curr.value.ident_val;
        parser_next(p);
        return make_var_ref(name);
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

ASTNode* parse_statement(Parser *p) {
    if (p->curr.type == TOK_VAR) {
        parser_next(p);
        if (p->curr.type != TOK_BOOL_TYPE && p->curr.type != TOK_INT_TYPE) {
            fprintf(stderr, "expected type ex:`bool`, `int` after `var` keyword in var declaration\n");
            exit(1);
        }
        VarType var_type = tok_to_var_type(p->curr.type);
        parser_next(p);

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

    if (p->curr.type == TOK_IDENT && p->peek.type == TOK_ASSIGN) {
        char* name = p->curr.value.ident_val;
        parser_next(p);
        parser_next(p);

        ASTNode* val = parse_expr(p);
        return make_var_assign(name, val);
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
