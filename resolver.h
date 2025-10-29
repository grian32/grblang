#ifndef GRBLANG_RESOLVER_H
#define GRBLANG_RESOLVER_H
#include "parser.h"

typedef struct {
    char** names;
    int count;
    int capacity;
} Resolver;

void resolver_init(Resolver* r);
void resolver_resize(Resolver* r);

int resolver_declare(Resolver* r, char* name);
int resolver_lookup(Resolver* r, char* name);

void resolve(ASTNode* node, Resolver* r);

#endif //GRBLANG_RESOLVER_H