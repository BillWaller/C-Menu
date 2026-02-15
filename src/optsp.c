/** @file optsp.c
    @brief print option list for C-Menu
    @author Bill Waller
    Copyright (c) 2025
    MIT License
    billxwaller@gmail.com
    @date 2026-02-09
 */

#include "cm.h"
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
void dump_opts_by_short_opt();
void dump_opts_by_name();
#define MAXLEN 256

char tmp_str[MAXLEN];

/** @brief check if a string contains a specified character
    @param str the string to check
    @param c the character to look for
    @return true if the character is found, false otherwise */
bool contains(char *str, char c) {
    while (*str) {
        if (*str == c)
            return true;
        str++;
    }
    return false;
}

/** @brief main function for optsp
    @param argc argument count
    @param argv argument vector
    @return exit status
    options:
     -n sort by name
     -s sort by short option
     -u select only options in mask
     -h show help message
     -v show version
    mask: any combination in any order of m, f, p, and v, where
        m is menu,
        f is form,
        p is pick, and
        v is view
 */
int main(int argc, char **argv) {
    bool f_help = false;
    bool f_version = false;
    int opt;
    int i;
    char tab[] = "mfpv";
    char mask[] = "....";
    bool f_mask = false;
    while ((opt = getopt(argc, argv, "hm:nsv")) != -1) {
        switch (opt) {
        case 'h':
            f_help = true;
            break;
        case 'm':
            for (i = 0; i < 4; i++) {
                if (contains(optarg, tab[i]))
                    mask[i] = tab[i];
            }
            printf("mask: %s\n", mask);
            f_mask = true;
            break;
        case 'n':
            dump_opts_by_name();
            break;
        case 's':
            dump_opts_by_short_opt();
            break;
        case 'v':
            f_version = true;
            break;
        default:
            exit(EXIT_FAILURE);
        }
    }
    if (f_help) {
        printf("\nUsage: optsp [options]\n\n");
        printf("Options:\n");
        printf(" -n  sort by name\n");
        printf(" -s  sort by short option\n");
        printf(" -h  show this help message\n");
        printf(" -m  mask: any combination in any order of m, f, p, and v\n");
        printf("\nwhere m is menu,\n");
        printf("      f is form,\n");
        printf("      p is pick, and\n");
        printf("      v is view.\n");
        exit(EXIT_SUCCESS);
    }
    if (f_version) {
        printf("optsp version 1.0\n");
        exit(EXIT_SUCCESS);
    }
    if (!f_mask)
        dump_opts();
    else
        dump_opts_by_use("Usage: ", mask);
}
