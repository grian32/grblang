#ifndef GRBLANG_PARSER_H
#define GRBLANG_PARSER_H
#include <stdbool.h>

#include "lexer.h"

typedef enum {
    AST_INT,
    AST_BOOL,
    AST_STRING,
    AST_BINARY_OP,
    AST_COMPOUND_ASSIGNMENT,
    AST_UNARY_OP,
    AST_PROGRAM,
    AST_IF,
    AST_WHILE,
    AST_ARRAY,
    AST_ARRAY_INDEX,
    AST_ARRAY_INDEX_ASSIGN,
    AST_VAR_DECL,
    AST_VAR_ASSIGN,
    AST_VAR_REF,
    AST_FUNCTION_CALL,
} ASTNodeType;

typedef enum {
    VALUE_INT,
    VALUE_BOOL,
    VALUE_STRING,
    VALUE_UNKNOWN,
} BaseType;

typedef struct {
    BaseType base_type;
    // -1 for non arrays
    int nested;
} VarType;

char* base_type_string(BaseType type);
void var_type_string(VarType type, char buffer[50]);

// basically purely used in resolver for the type checker
typedef struct {
    union {
        int int_val;
        bool bool_val;
        char* str_val;
    };
} VarValue;

typedef struct ASTNode {
    ASTNodeType type;
    VarType var_type;
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
            TokenType op;
            char* name;
            int slot;
            struct ASTNode* value;
        } compound_assignment;
        struct {
            char* name;
            int slot;
        } var_ref;

        struct {
            struct ASTNode* condition;
            struct ASTNode** success_statements;
            int success_count;
            struct ASTNode** fail_statements;
            int fail_count;
        } if_stmt;

        struct {
            struct ASTNode* condition;
            struct ASTNode** statements;
            int statements_count;
        } while_stmt;

        struct {
            char* string_val;
            int len;
        } string;

        struct {
            struct ASTNode** arr;
            int len;
            VarType type;
        } array_literal;

        struct {
            struct ASTNode* array_expr; // only AST_VAR_REF or nested AST_ARRAY_INDEX at this time
            struct ASTNode* index_expr;
        } array_index;

        struct {
            struct ASTNode* arr_index_expr; // only AST_ARRAY_INDEX
            struct ASTNode* value;
        } array_assign_expr;

        struct {
            char* name;
            struct ASTNode** args;
            int args_len;
        } function_call;
    };
} ASTNode;

typedef struct {
    Token curr;
    Token peek;
    Lexer* lexer;
    int pos;
} Parser;

// convert DATA_XXX DataType to VALUE_XXX VarType
VarType data_to_var_type(DataType tt);

void parser_init(Parser* p, Lexer* l);
void parser_next(Parser* p);

char* op_string(TokenType op);
void print_ast(ASTNode* node, int indent, bool newline);

ASTNode* make_int(int value);
// separate true and false because i'd just be doubling up checks on tokentype if it was just one make_bool which is wasteful
ASTNode* make_true_bool();
ASTNode* make_false_bool();
ASTNode* make_string(char* str_val, int len);
ASTNode* make_binary_op(TokenType op, ASTNode* left, ASTNode* right);
ASTNode* make_compound_assignment(TokenType op, char* name, ASTNode* value);
ASTNode* make_unary_op(TokenType op, ASTNode* right);
ASTNode* make_program(ASTNode** statements, int count);
ASTNode* make_if_statement(ASTNode* condition, ASTNode** success_statements, int success_count, ASTNode** fail_statements, int fail_count);
ASTNode* make_while_statement(ASTNode* condition, ASTNode** statements, int statements_count);
ASTNode* make_var_decl(char* name, ASTNode* value, VarType type);
ASTNode* make_var_assign(char* name, ASTNode* value);
ASTNode* make_var_ref(char* name);
ASTNode* make_arr_literal(ASTNode** exprs, int len);
ASTNode* make_arr_index(ASTNode* var_ref, ASTNode* index_expr);
ASTNode* make_arr_index_assign(ASTNode* arr_index, ASTNode* value);
ASTNode* make_function_call(ASTNode** args, int args_len, char* value);

ASTNode* parse_compound_assignment(Parser* p);
ASTNode* parse_logical_or(Parser* p);
ASTNode* parse_logical_and(Parser* p);
ASTNode* parse_comparison(Parser* p);
ASTNode* parse_addsub(Parser* p);
ASTNode* parse_primary(Parser* p);
ASTNode* parse_unary(Parser* p);
ASTNode* parse_muldiv(Parser* p);

// assumes p.curr == tok_if on call
ASTNode* parse_if_stmt(Parser* p);
// assumes p.curr == tok_while on call
ASTNode* parse_while_stmt(Parser* p);
// assumes p.curr == [ on call
ASTNode* parse_array_literal(Parser* p);
// assumes p.curr == tok_var
ASTNode* parse_var_decl(Parser* p);
// assumes p.curr == tok_lparen
ASTNode* parse_function_call(Parser* p, char* ident_name);

/**
 * = highest precendence parse; to be used as a top-level entry point for parsing expression
 */
ASTNode* parse_expr(Parser* p);
ASTNode* parse_statement(Parser *p);
void parse_block(Parser* p, int initial_capacity, TokenType end_tok, ASTNode*** statements_out, int* count_out);
// to be used as a general entry point
ASTNode* parse_program(Parser *p);

// free_ast takes in, ideally the `AST_PROGRAM` node, frees recursively, at the earliest, this should be called after resolving, type checking & bytecode generation
void free_ast(ASTNode* node);

#endif //GRBLANG_PARSER_H
