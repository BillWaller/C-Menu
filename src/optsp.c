/** @file optsp.c
    @brief print option list for C-Menu
    @author Bill Waller
    Copyright (c) 2025
    MIT License
    billxwaller@gmail.com
    @date 2026-02-09
 */

#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void dump_opts_by_short_opt();
void dump_opts_by_name();
#define MAXLEN 256
#define MASK "mpfv"

char tmp_str[MAXLEN];

/** @brief main function for optsp
    @param argc argument count
    @param argv argument vector
    @return exit status
    options:
     -n sort by name
     -s sort by short option
     -h show help message
 */
int main(int argc, char **argv) {
    bool f_help = false;
    bool f_version = false;
    int opt;

    while ((opt = getopt(argc, argv, "snh")) != -1) {
        switch (opt) {
        case 'h':
            f_help = true;
            break;
        case 's':
            dump_opts_by_short_opt();
            return 0;
        case 'n':
            dump_opts_by_name();
            return 0;
        case 'v':
            f_version = true;
            break;
        default:
            exit(EXIT_FAILURE);
        }
    }
    if (f_help) {
        printf("Usage: optsp [options]\n");
        printf("Options:\n");
        printf("  -n        sort by name\n");
        printf("  -s        sort by short option\n");
        printf("  -h        show this help message\n");
        exit(EXIT_SUCCESS);
    }
    if (f_version) {
        printf("optsp version 1.0\n");
        exit(EXIT_SUCCESS);
    }
    dump_opts_by_short_opt();
}
