#ifndef GRBLANG_RESOLVER_H
#define GRBLANG_RESOLVER_H
#include "parser.h"

typedef struct {
    char** names;
    VarType* types;
    int count;
    int capacity;
} Resolver;

void resolver_init(Resolver* r);
void resolver_resize(Resolver* r);

int resolver_declare(Resolver* r, char* name, VarType type);
int resolver_lookup(Resolver* r, char* name);

void resolve(ASTNode* node, Resolver* r);

// this can be used after the bytecode generation step as long as the number of locals is stored in a variable for the vm to use, this is reccomended
void free_resolver(Resolver* r);

#endif //GRBLANG_RESOLVER_H
