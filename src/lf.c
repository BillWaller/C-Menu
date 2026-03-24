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
    @verbatim
        lf [options ad:e:hit:v] [dir] [re]
           -a  List hidden files
           -d  maximum max_depth of subdirectories to examine
           -e  exclude files matching the regular expression
           -h  show this help message
           -i  ignore case in search
           -t  [bcdplfsu]
               b          block devices
                c         character devices
                 d        directories
                  p       named pipes
                   l      symbolic links
                    f     regular files
                     s    sockets
                      u   unknown
                          (in any order or combination)
           -v  show version information
           dir Directory to search
           re  Regular expression to match
    @endverbatim
 */

int main(int argc, char **argv) {
    char dir[MAXLEN];
    char re[MAXLEN];
    char ere[MAXLEN];
    dir[0] = '\0';
    re[0] = '\0';
    ere[0] = '\0';
    bool f_help = false;
    bool f_version = false;
    int flags = 0;
    int opt;
    int max_depth = 3;
    int i;

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
            flags |= LF_EXC_REGEX;
            break;
        case 'h':
            f_help = true;
            break;
        case 'i':
            flags |= LF_ICASE;
            break;
        case 't':
            i = 0;
            while (optarg[i]) {
                switch (optarg[i++]) {
                case 'b':
                    flags |= FT_BLK << 8;
                    break;
                case 'c':
                    flags |= FT_CHR << 8;
                    break;
                case 'd':
                    flags |= FT_DIR << 8;
                    break;
                case 'p':
                    flags |= FT_FIFO << 8;
                    break;
                case 'l':
                    flags |= FT_LNK << 8;
                    break;
                case 'f':
                    flags |= FT_REG << 8;
                    break;
                case 's':
                    flags |= FT_SOCK << 8;
                    break;
                case 'u':
                    flags |= FT_UNKNOWN << 8;
                    break;
                default:
                    break;
                }
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
        printf("  -t   [bcdplfsu]\n");
        printf("       b          block devices\n");
        printf("        c         character devices\n");
        printf("         d        directories\n");
        printf("          p       named pipes\n");
        printf("           l      symbolic links\n");
        printf("            f     regular files\n");
        printf("             s    sockets\n");
        printf("              u   unknown\n");
        printf("                  (in any order or combination)\n");
        printf("  -v        show version information\n");
        exit(EXIT_SUCCESS);
    }
    if (f_version) {
        printf("lf version: %s\n", CM_VERSION);
        exit(EXIT_SUCCESS);
    }
    struct stat sb;
    if (optind < argc) {
        if (stat(argv[optind], &sb) == -1 || (sb.st_mode & S_IFMT) != S_IFDIR) {
            strnz__cpy(dir, ".", MAXLEN - 1);
        } else {
            strnz__cpy(dir, argv[optind], MAXLEN - 1);
            optind++;
        }
    }
    if (optind < argc) {
        strnz__cpy(re, argv[optind], MAXLEN - 1);
        optind++;
    }
    if (re[0] == '\0')
        strnz__cpy(re, ".*", MAXLEN - 1);
    if (dir[0] == '\0')
        strnz__cpy(dir, ".", MAXLEN - 1);
    flags |= LF_REGEX;
    lf_find(dir, re, ere, max_depth, flags);
    return true;
}
