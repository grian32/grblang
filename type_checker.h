#ifndef GRBLANG_TYPE_CHECKER_H
#define GRBLANG_TYPE_CHECKER_H

#include "parser.h"
#include "resolver.h"

VarType get_expr_type(ASTNode* node, Resolver* r);
void type_check(ASTNode *node, Resolver* r);

#endif
