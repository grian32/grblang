#include "parser.h"

#include <stdio.h>
#include <stdlib.h>

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
        case AST_BINARY_OP:
            printf("AST_BINARY_OP(%c)", node->binary_op.op == TOK_PLUS ? '+' : node->binary_op.op == TOK_MINUS ? '-' : node->binary_op.op == TOK_MULT ? '*' : '/');
            if (newline) {
                printf("\n");
            }
            print_ast(node->binary_op.left, indent + 1, true);
            print_ast(node->binary_op.right, indent + 1, true);
            break;
        case AST_UNARY_OP:
            printf("AST_UNARY_OP(%c)",node->binary_op.op == TOK_MINUS ? '-' : ' ');
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
        case AST_VAR_ASSIGN:
            printf("AST_VAR_ASSIGN(slot=%d,%s=", node->var_assign.slot, node->var_assign.name);
            print_ast(node->var_assign.value, 0, false);
            printf(")");
            if (newline) {
                printf("\n");
            }
            break;
        case AST_VAR_DECL:
            printf("AST_VAR_DECL(slot=%d,%s=", node->var_assign.slot,node->var_decl.name);
            print_ast(node->var_decl.value, 0, false);
            printf(")");
            if (newline) {
                printf("\n");
            }
            break;
        case AST_VAR_REF:
            printf("AST_VAR_REF(slot=%d,%s)", node->var_ref.slot, node->var_ref.name);
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

ASTNode* make_var_decl(char* name, ASTNode* value) {
    ASTNode* node = malloc(sizeof(ASTNode));

    node->type = AST_VAR_DECL;
    node->var_decl.name = name;
    node->var_decl.value = value;

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

ASTNode* parse_primary(Parser* p) {
    if (p->curr.type == TOK_INT) {
        ASTNode* t = make_int(p->curr.value.int_val);
        parser_next(p);
        return t;
    }

    if (p->curr.type == TOK_IDENT) {
        char* name = p->curr.value.ident_val;
        parser_next(p);
        return make_var_ref(name);
    }

    if (p->curr.type == TOK_LPAREN) {
        parser_next(p);
        ASTNode* expr = parse_addsub(p);
        if (p->curr.type != TOK_RPAREN) {
            fprintf(stderr, "unexpected token (likely unmatched parens) in parse_primary\n");
            exit(1);
            // TODO: actual error handling
        }
        parser_next(p);
        return expr;
    }

    fprintf(stderr, "unexpected token in parse_primary\n");
    exit(1);
}

ASTNode* parse_unary(Parser* p) {
    if (p->curr.type == TOK_MINUS) {
        parser_next(p);
        ASTNode* right = parse_unary(p);
        return make_unary_op(TOK_MINUS, right);
    }

    return parse_primary(p);
}

ASTNode* parse_muldiv(Parser* p) {
    ASTNode* left = parse_unary(p);

    while (p->curr.type == TOK_MULT || p->curr.type == TOK_DIV) {
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
    return parse_addsub(p);
}

ASTNode* parse_statement(Parser *p) {
    if (p->curr.type == TOK_VAR) {
        parser_next(p);
        if (p->curr.type != TOK_IDENT) {
            fprintf(stderr, "expected identifier after `var`\n");
            exit(1);
        };

        char* name = p->curr.value.ident_val;
        parser_next(p);

        if (p->curr.type != TOK_EQUALS) {
            fprintf(stderr, "expected equals after identifer in var declaration\n");
            exit(1);
        }
        parser_next(p);

        ASTNode* val = parse_expr(p);
        return make_var_decl(name, val);
    }

    if (p->curr.type == TOK_IDENT && p->peek.type == TOK_EQUALS) {
        char* name = p->curr.value.ident_val;
        parser_next(p);
        parser_next(p);

        ASTNode* val = parse_expr(p);
        return make_var_assign(name, val);
    }

    return parse_expr(p);
}

ASTNode* parse_program(Parser *p) {
    int capacity = 128, count = 0;
    ASTNode** statements = malloc(sizeof(ASTNode*) * capacity);

    while (p->curr.type != TOK_EOF) {
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
            fprintf(stderr, "semicolon expected after statement\n");
            exit(1);
        }
        parser_next(p);
    }

    return make_program(statements, count);
}
