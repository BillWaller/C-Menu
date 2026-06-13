#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// read lines from a file and convert hex to decimal
// usage: fhex2dec <filename>
int main(int argc, char *argv[]) {
    FILE *fp;
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }
    int i = 1;
    fp = fopen(argv[i], "r");
    if (fp == NULL) {
        fprintf(stderr, "Error opening file: %s %s\n", argv[i],
                strerror(errno));
        return 1;
    }
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = 0;
        long decimal = strtol(line, NULL, 16);
        printf("%ld\n", decimal);
    }
    fclose(fp);
    return 0;
}
