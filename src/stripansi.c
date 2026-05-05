/** @file stripansi.c
    @brief removes ansi escape sequences beginning with "\033["" and ending in
   "m" or "K"
    @author Bill Waller
    Copyright (c) 2025, 2026
    MIT License
    billxwaller@gmail.com
    @date 2026-02-09
 */

#include <cm.h>
#include <stdio.h>

/** @brief strip ANSI SGR sequences
   @param argc Argument count (should be 2 for the program name and input file)
   @param argv Argument vector (argv[1] should be the input file name)
   @details This function iterates through the input string, copying characters
   to the output buffer while skipping over any ANSI escape sequences. It looks
   for sequences that start with "\033[" and end with "m" or "K", and removes
   them from the output. */
int main(int argc, char *argv[]) {
    char
        in_buf[2048]; /**< Buffer to hold the input string read from the file */
    char out_buf[2048]; /**< Buffer to hold the cleaned output string */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s [file_with_ansi_codes]\n", argv[0]);
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
