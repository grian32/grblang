#include "lexer.h"
#include "util.h"

int main(void) {
    char* src = read_file("../test.grb");
    Lexer l;
    lexer_init(&l, src);
    printf("%s\n", src);
    return 0;
}