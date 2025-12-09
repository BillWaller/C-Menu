// readfile.c
//
#include <stdio.h>
#include <stdlib.h>
#define MAXLEN 256

int main(int argc, char **argv) {
    FILE *file_ptr;
    char line_buffer[MAXLEN];

    file_ptr = fopen("ls.out", "r");

    if (file_ptr == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    while (fgets(line_buffer, MAXLEN, file_ptr) != NULL) {
        printf("%s", line_buffer);
    }

    fclose(file_ptr);

    return 0;
}
