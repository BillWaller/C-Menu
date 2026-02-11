/** @file lf.c
 *  @brief Recursively list files matching a regular expression
 *  @author Bill Waller
 *  Copyright (c) 2025
 *  MIT License
 *  billxwaller@gmail.com
 *  @date 2026-02-09
 */

#include "cm.h"
#include <dirent.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

char tmp_str[MAXLEN];

int main(int argc, char **argv) {
    /// list files matching a regular expression.
    /// Usage: lf [options] [directory] [regexp]
    /// Options:
    ///            -a        List all files (including hidden files)
    ///            -d        maximum depth of subdirectories to examine
    ///            -h        show this help message
    ///            -i        ignore case in search
    ///            -r        recurse into subdirectories
    ///            -v        show version information
    char dir[MAXLEN] = "";
    char re[MAXLEN] = "";
    bool f_help = false;
    bool f_version = false;
    int flags = 0;
    int opt;
    int depth = MAX_DEPTH;
    while ((opt = getopt(argc, argv, "ad:hrv")) != -1) {
        switch (opt) {
        case 'a':
            flags |= ALL;
            break;
        case 'd':
            depth = atoi(optarg);
            break;
        case 'h':
            f_help = true;
            break;
        case 'i':
            flags |= ICASE;
            break;
        case 'r':
            flags |= RECURSE;
            break;
        case 'v':
            f_version = true;
            break;
        default:
            exit(EXIT_FAILURE);
        }
    }
    if (f_help) {
        printf("Usage: lf [options] [directory] [regexp]\n");
        printf("Options:\n");
        printf("  -a        List all files (including hidden files)\n");
        printf("  -d        maximum depth of subdirectories to examine\n");
        printf("  -h        show this help message\n");
        printf("  -i        ignore case in search\n");
        printf("  -r        recurse into subdirectories\n");
        printf("  -v        show version information\n");
        exit(EXIT_SUCCESS);
    }
    if (f_version) {
        printf("lf version 1.0\n");
        exit(EXIT_SUCCESS);
    }
    if (optind < argc) {
        strnz__cpy(dir, argv[optind], MAXLEN - 1);
        optind++;
    }
    if (optind < argc) {
        strnz__cpy(re, argv[optind], MAXLEN - 1);
        optind++;
    }
    if (re[0] == '\0')
        strnz__cpy(re, ".*", MAXLEN - 1);
    if (dir[0] == '\0')
        strnz__cpy(dir, ".", MAXLEN - 1);
    if (flags & RECURSE) {
        lf_find_files(dir, re, flags);
        lf_find_dirs(dir, re, depth, flags);
    } else {
        lf_find_files(dir, re, flags);
    }
    return true;
}
