#ifndef GRBLANG_PARSER_H
#define GRBLANG_PARSER_H
#include <stdbool.h>

#include "lexer.h"

typedef enum {
    AST_INT,
    AST_BINARY_OP,
    AST_UNARY_OP,
} ASTNodeType;

typedef struct ASTNode {
    ASTNodeType type;
    union {
        int int_val;
        struct {
            TokenType op;
            struct ASTNode* left;
            struct ASTNode* right;
        } binary_op;
        struct {
            TokenType op;
            struct ASTNode* right;
        } unary_op;
    };
} ASTNode;

typedef struct {
    Token curr;
    Token peek;
    Lexer* lexer;
    int pos;
} Parser;

void parser_init(Parser* p, Lexer* l);
void parser_next(Parser* p);

void print_ast(ASTNode* node, int indent);

ASTNode* make_int(int value);
ASTNode* make_binary_op(TokenType op, ASTNode* left, ASTNode* right);
ASTNode* make_unary_op(TokenType op, ASTNode* right);

ASTNode* parse_addsub(Parser* p);
ASTNode* parse_primary(Parser* p);
ASTNode* parse_unary(Parser* p);
ASTNode* parse_muldiv(Parser* p);

/**
 * = parse_addsub; to be used as a top-level entry point
 */
ASTNode* parse_expr(Parser* p);

#endif //GRBLANG_PARSER_H