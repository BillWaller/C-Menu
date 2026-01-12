//  stripansi.c
//  Bill Waller Copyright (c) 2025
//  MIT License
//  billxwaller@gmail.com
//
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
/// int strip_ansi(char *d, char *s)
/// Strips ANSI SGR escape sequences (ending in 'm') from string s to d
/// Returns length of stripped string
/// @param d Destination string
/// @param s Source string
/// @return Length of stripped string
/// Example:
/// char dest[1024];
/// char src[] = "\033[31mThis is red text\033[0m
/// int len = strip_ansi(dest, src);
/// Result: dest = "This is red text", len = 17
/// Note: Only handles SGR sequences ending in 'm'
/// Skips non-ASCII characters
/// @note The caller must ensure that d has enough space to hold the stripped
/// string
/// @note This function does not allocate memory; it assumes d is pre-allocated
/// @note This function processes the entire string until the null terminator
/// @note This function does not modify the source string s
/// @note This function returns the length of the resulting string, including
/// the null terminator
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
    return l + 1;
}
