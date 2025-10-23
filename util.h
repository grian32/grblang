#ifndef GRBLANG_UTIL_H
#define GRBLANG_UTIL_H
#include <stdio.h>
#include <stdlib.h>

static char* read_file(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) {
        perror("Failed to open file");
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    long length = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *buffer = malloc(length + 1);
    if (!buffer) {
        perror("failed to malloc");
        fclose(f);
        return NULL;
    }

    fread(buffer, 1, length, f);
    buffer[length] = '\0';
    fclose(f);

    return buffer;
}

#endif //GRBLANG_UTIL_H