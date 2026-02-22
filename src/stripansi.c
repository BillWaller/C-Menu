/** @file stripansi.c
    @brief removes ansi escape sequences beginning with "\033["" and ending in
   "m" or "K"
    @author Bill Waller
    Copyright (c) 2025
    MIT License
    billxwaller@gmail.com
    @date 2026-02-09
 */

#include <cm.h>
#include <stdio.h>

/** C function - strip_ansi(out_str, in_str)
 * @details This function iterates through the input string, copying characters
 * to the output buffer while skipping over any ANSI escape sequences. It looks
 * for sequences that start with "\033[" and end with "m" or "K", and removes
 * them from the output. */

/** biniary executable - stripansi [input_file]
 * @param argc Argument count (should be 2 for the program name and input file)
 * @param argv Argument vector (argv[1] should be the input file name)
   @details ANSI escape sequences start with "\033[" and end with "m" or "K".
   This function removes those sequences from the input string and writes the
   cleaned string to stdout */
int main(int argc, char *argv[]) {
    char in_buf[2048]; /** Buffer to hold the input string read from the file */
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
