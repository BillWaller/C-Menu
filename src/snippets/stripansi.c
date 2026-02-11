/** @file stripansi.c
    @brief removes ansi escape sequences beginning with "\033["" and ending in
   "m" or "K"
    @author Bill Waller
    Copyright (c) 2025
    MIT License
    billxwaller@gmail.com
    @date 2026-02-09
 */

#include "cm.h"
#include <stdio.h>

/** @param input file with ANSI codes
 *  @details ANSI escape sequences start with "\033[" and end with "m" or "K".
 * This function removes those sequences from the input string and writes the
 * cleaned string to stdout */

int main(int argc, char *argv[]) {
    char in_buf[2048];
    char out_buf[2048];
    if (argc != 2) {
        fprintf(stderr, "Usage: %s [file_with_ansi_codes]\n", argv[0]);
        return 1;
    }
    FILE *in_fp = fopen(argv[1], "r");
    if (!in_fp) {
        perror("Error opening file");
        return 1;
    }
    /** Read each line from the input file, strip ANSI codes, and print the
     * cleaned line to stdout */
    while (fgets(in_buf, sizeof(in_buf), in_fp)) {
        strip_ansi(out_buf, in_buf);
        fputs(out_buf, stdout);
    }
    fclose(in_fp);
    return 0;
}
