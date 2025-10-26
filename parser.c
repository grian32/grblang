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

void print_ast(ASTNode* node, int indent) {
    if (!node) return;
    for (int i = 0; i < indent; i++) {
        printf(" ");
    }

    switch (node->type) {
        case AST_INT:
            printf("AST_INT(%d)\n",node->int_val);
            break;
        case AST_BINARY_OP:
            printf("AST_BINARY_OP(%c)\n", node->binary_op.op == TOK_PLUS ? '+' : node->binary_op.op == TOK_MINUS ? '-' : node->binary_op.op == TOK_MULT ? '*' : '/');
            print_ast(node->binary_op.left, indent + 1);
            print_ast(node->binary_op.right, indent + 1);
            break;
        case AST_UNARY_OP:
            printf("AST_UNARY_OP(%c)\n",node->binary_op.op == TOK_MINUS ? '-' : ' ');
            print_ast(node->unary_op.right, indent+1);
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

ASTNode* parse_primary(Parser* p) {
    if (p->curr.type == TOK_INT) {
        ASTNode* t = make_int(p->curr.value.int_val);
        parser_next(p);
        return t;
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
