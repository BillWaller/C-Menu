//  stripansi.c
//  Bill Waller Copyright (c) 2025
//  MIT License
//  billxwaller@gmail.com
//
/// Strips ANSI escape codes from a string
/// Returns length of stripped string
/// Usage: stripansi <file_with_ansi_codes>
/// Outputs stripped text to stdout
/// Example ANSI escape code: \033[31m (red text)
/// Only handles basic SGR codes ending with 'm'

#include <stdint.h>
#include <stdio.h>
int strip_ansi(char *, char *);

int main(int argc, char *argv[]) {
    char in_buf[1024];
    char out_buf[1024];
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file_with_ansi_codes>\n", argv[0]);
        return 1;
    }
    FILE *in_fp = fopen(argv[1], "r");
    if (!in_fp) {
        perror("Error opening file");
        return 1;
    }
    while (fgets(in_buf, sizeof(in_buf), in_fp)) {
        strip_ansi(out_buf, in_buf);
        fputs(out_buf, stdout);
    }
    fclose(in_fp);
    return 0;
}

int strip_ansi(char *d, char *s) {
    int l;
    while (*s) {
        if (*s == '\033') {
            while (*s && *s != 'm')
                s++;
            if (*s == 'm')
                s++;
            continue;
        } else {
            if ((unsigned char)*s <= 127) {
                *d++ = *s++;
                l++;
            } else
                s++;
        }
    }
    *d = '\0';
    return l;
}
