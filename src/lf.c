/** @file lf.c
    @brief list files matching a regular expression
    @author Bill Waller
    Copyright (c) 2025
    MIT License
    billxwaller@gmail.com
    @date 2026-02-09
 */

#include <cm.h>
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

/** @brief List files in a directory matching a regular expression
    @code
        lf [options ad:hit:v] [dir] [re]
             -a        List hidden files
             -d        maximum max_depth of subdirectories to examine
             -e        exclude files matching the regular expression
             -h        show this help message
             -i        ignore case in search
             -t d      list directories (exclude files)
             -t f      list files (exclude directories)
             -v        show version information
            dir Directory to search
            re  Regular expression to match
    @endcode
 */
int main(int argc, char **argv) {
    char dir[MAXLEN] = "";
    char re[MAXLEN] = "";
    char ere[MAXLEN] = "";
    bool f_help = false;
    bool f_version = false;
    int flags = 0;
    int opt;
    int max_depth = 3;
    while ((opt = getopt(argc, argv, "ad:e:hit:v")) != -1) {
        switch (opt) {
        case 'a':
            flags |= LF_ALL;
            break;
        case 'd':
            max_depth = atoi(optarg);
            break;
        case 'e':
            strnz__cpy(ere, optarg, MAXLEN - 1);
            break;
        case 'h':
            f_help = true;
            break;
        case 'i':
            flags |= LF_ICASE;
            break;
        case 't':
            switch (optarg[0]) {
            case 'd':
                flags |= LF_DIRS;
                break;
            case 'f':
                flags |= LF_FILES;
                break;
            default:
                break;
            }
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
        printf("  -a        List hidden files\n");
        printf("  -d        maximum depth of subdirectories to examine\n");
        printf("  -e        exclude files matching the regular expression\n");
        printf("  -h        show this help message\n");
        printf("  -i        ignore case in search\n");
        printf("  -t f      list files (exclude directories)\n");
        printf("  -t d      list directories (exclude files)\n");
        printf("  -v        show version information\n");
        exit(EXIT_SUCCESS);
    }
    if (f_version) {
        printf("lf version 1.0\n");
        exit(EXIT_SUCCESS);
    }
    struct stat sb;
    if (optind < argc) {
        if (stat(argv[optind], &sb) == -1 || (sb.st_mode & S_IFMT) != S_IFDIR) {
            Perror("First non-option argument must be a valid directory");
            exit(EXIT_FAILURE);
        }
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
    lf_find(dir, re, ere, max_depth, flags);
    return true;
}
