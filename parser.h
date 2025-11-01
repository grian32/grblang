#ifndef GRBLANG_PARSER_H
#define GRBLANG_PARSER_H
#include <stdbool.h>

#include "lexer.h"

typedef enum {
    AST_INT,
    AST_BOOL,
    AST_BINARY_OP,
    AST_UNARY_OP,
    AST_PROGRAM,
    AST_VAR_DECL,
    AST_VAR_ASSIGN,
    AST_VAR_REF,
} ASTNodeType;

typedef struct ASTNode {
    ASTNodeType type;
    union {
        int int_val;
        bool bool_val;

        struct {
            TokenType op;
            struct ASTNode* left;
            struct ASTNode* right;
        } binary_op;
        struct {
            TokenType op;
            struct ASTNode* right;
        } unary_op;

        struct {
            struct ASTNode** statements;
            int count;
        } program;

        // currently same between var_decl/assign but in future might want to add types to var_decl etc, so keeping as is for now
        struct {
            char* name;
            struct ASTNode* value;
            int slot;
        } var_decl;
        struct {
            char* name;
            struct ASTNode* value;
            int slot;
        } var_assign;
        struct {
            char* name;
            int slot;
        } var_ref;
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

void print_ast(ASTNode* node, int indent, bool newline);

ASTNode* make_int(int value);
// separate true and false because i'd just be doubling up checks on tokentype if it was just one make_bool which is wasteful
ASTNode* make_true_bool();
ASTNode* make_false_bool();
ASTNode* make_binary_op(TokenType op, ASTNode* left, ASTNode* right);
ASTNode* make_unary_op(TokenType op, ASTNode* right);
ASTNode* make_program(ASTNode** statements, int count);
ASTNode* make_var_decl(char* name, ASTNode* value);
ASTNode* make_var_assign(char* name, ASTNode* value);
ASTNode* make_var_ref(char* name);

ASTNode* parse_addsub(Parser* p);
ASTNode* parse_primary(Parser* p);
ASTNode* parse_unary(Parser* p);
ASTNode* parse_muldiv(Parser* p);

/**
 * = parse_addsub; to be used as a top-level entry point for parsing expression
 */
ASTNode* parse_expr(Parser* p);
ASTNode* parse_statement(Parser *p);
// to be used as a general entry point
ASTNode* parse_program(Parser *p);

#endif //GRBLANG_PARSER_H