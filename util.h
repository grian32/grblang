#ifndef GRBLANG_UTIL_H
#define GRBLANG_UTIL_H
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

// read_file returns char* to the contents of the file & updates the int* size to the length of the string
static char* read_file(const char* filename, long* size_out) {
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
    *size_out = length;
    fclose(f);

    return buffer;
}

void print_visible(char *str) {
    while (*str) {
        unsigned char c = *str;

        if (isprint(c)) {
            putchar(c);
        }

        else if (c < 32 || c == 127) {
            if (c == 127) {
                printf("^?");
            } else {
                printf("^%c", c + 64);
            }
        }
        else {
            printf("\\x%02X", c);
        }

        str++;
    }
}

#endif //GRBLANG_UTIL_H
