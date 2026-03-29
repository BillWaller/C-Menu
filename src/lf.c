
/** @file lf.c
    @brief list files matching a regular expression
    @author Bill Waller
    Copyright (c) 2025
    MIT License
    billxwaller@gmail.com
    @date 2026-02-09
 */

#include <argp.h>
#include <cm.h>
#include <dirent.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/** @brief List files in a directory matching a regular expression
    @code
        lf [options ad:e:hit:v] [dir] [re]
             -a        List hidden files
             -d        maximum max_depth of subdirectories to examine
             -e        exclude files matching the regular expression
             -h        show this help message
             -i        ignore case in search
             -t [bcdplfsu]
                 b        block devices
                  c       character devices
                   d      directories
                    p     named pipes
                     l    symbolic links
                      f   regular files
                       s  sockets
                        u unknown
             -v        show version information
            dir Directory to search
            re  Regular expression to match
    @endcode
    @verbatim
        Capabilities:
             specify whether to list hidden files
             list files in a directory structure matching a regular expression
             exclude files matching a regular expression
             ignore case in search
             list files of specific types by specifying
               -t [b][c][d][p][l][f][s][u] (in any order or combination)
                   b block devices
                   c character devices
                   d directories
                   p named pipes
                   l symbolic links
                   s sockets
                   u unknown file types
             specify maximum depth of subdirectory descent
    @endverbatim
 */
const char *argp_program_version = "lf-0.2.9";
const char *argp_program_bug_address = "billxwaller@gmail.com";
static char doc[] = "lf list files";
static char args_doc[] = "[ARG1] [ARG2]";

static struct argp_option options[] = {
    {"f_all", 'a', 0, 0, "List hidden files", 0},
    {"max_depth", 'd', "number", 0, "Depth into directory", 0},
    {"exclude", 'e', "regex", 0, "Exclude regular expression", 0},
    {"help", 'h', 0, 0, "Display help", 0},
    {"f_ignore_case", 'i', 0, 0, "Search ignore case", 0},
    {"file_types", 't', "bcdplfsu", 0, "File types", 0},
    {"f_version", 'V', 0, 0, "Display version messages", 0},
    {"f_verbose", 'v', 0, 0, "verbose messages", 0},
    {0}};

struct arguments {
    bool f_all;
    int max_depth;
    char *exclude_p;
    bool help;
    bool f_ignore_case;
    int flags;
    char *file_types_p;
    bool f_version;
    bool f_verbose;
    char *args[2];
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct arguments *arguments = state->input;
    int i = 0;
    switch (key) {
    case 'a':
        arguments->flags |= LF_ALL;
        break;
    case 'd':
        arguments->max_depth = atoi(arg);
        break;
    case 'e':
        arguments->exclude_p = arg;
        arguments->flags |= LF_EXC_REGEX;
        break;
    case 'h':
        arguments->help = true;
        break;
    case 'i':
        arguments->flags |= LF_ICASE;
        break;
    case 't':
        arguments->file_types_p = arg;
        while (arguments->file_types_p[i]) {
            switch (arguments->file_types_p[i++]) {
            case 'b':
                arguments->flags |= FT_BLK << 8;
                break;
            case 'c':
                arguments->flags |= FT_CHR << 8;
                break;
            case 'd':
                arguments->flags |= FT_DIR << 8;
                break;
            case 'p':
                arguments->flags |= FT_FIFO << 8;
                break;
            case 'l':
                arguments->flags |= FT_LNK << 8;
                break;
            case 'f':
                arguments->flags |= FT_REG << 8;
                break;
            case 's':
                arguments->flags |= FT_SOCK << 8;
                break;
            case 'u':
                arguments->flags |= FT_UNKNOWN << 8;
                break;
            default:
                break;
            }
        }
        break;
    case 'V':
        arguments->f_version = true;
        break;
    case 'v':
        arguments->f_verbose = true;
        break;
    case ARGP_KEY_ARG:
        if (state->arg_num == 0 || state->arg_num == 1) {
            arguments->args[state->arg_num] = arg;
        } else {
            argp_usage(state);
        }
        break;
    case ARGP_KEY_END:
        break;
    default:
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}
static struct argp argp = {options, parse_opt, args_doc, doc,
                           nullptr, nullptr,   nullptr};

int main(int argc, char **argv) {
    char dir[MAXLEN];
    char re[MAXLEN];
    dir[0] = '\0';
    re[0] = '\0';

    struct arguments arguments = {0};
    arguments.f_all = false;
    arguments.max_depth = 4;
    arguments.exclude_p = "";
    arguments.help = false;
    arguments.f_ignore_case = false;
    arguments.file_types_p = "";
    arguments.f_version = false;
    arguments.args[0] = nullptr;
    arguments.args[1] = nullptr;

    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    if (arguments.args[0] != nullptr) {
        strncpy(dir, arguments.args[0], MAXLEN - 1);
    } else {
        strncpy(dir, ".", MAXLEN - 1);
    }
    if (arguments.args[1] != nullptr) {
        strncpy(re, arguments.args[1], MAXLEN - 1);
        arguments.flags |= LF_REGEX;
    } else {
        strncpy(re, ".*", MAXLEN - 1);
    }

    if (arguments.f_verbose) {
        // for debugging
        printf("arguments.f_all = %s\n", arguments.f_all ? "yes" : "no");
        printf("arguments.max_depth = %d\n", arguments.max_depth);
        printf("arguments.exclude_p = %s\n", arguments.exclude_p);
        printf("arguments.help = %s\n", arguments.help ? "yes" : "no");
        printf("arguments.f_ignore_case = %s\n",
               arguments.f_ignore_case ? "yes" : "no");
        printf("arguments.file_types_p = %s\n", arguments.file_types_p);
        printf("arguments.f_version = %s\n",
               arguments.f_version ? "yes" : "no");
        if (arguments.flags & LF_ALL)
            printf("hidden files\n");
        if (arguments.flags & LF_EXC_REGEX)
            printf("exclude_p regex: %s\n", arguments.exclude_p);
        if (arguments.flags & LF_ICASE)
            printf("ignore case\n");
        if (arguments.flags & FT_BLK)
            printf("block devices\n");
        if (arguments.flags & FT_CHR)
            printf("character devices\n");
        if (arguments.flags & FT_DIR)
            printf("directories\n");
        if (arguments.flags & FT_FIFO)
            printf("named pipes\n");
        if (arguments.flags & FT_LNK)
            printf("symbolic links\n");
        if (arguments.flags & FT_REG)
            printf("regular files\n");
        if (arguments.flags & FT_SOCK)
            printf("sockets\n");
        if (dir[0] != '\0')
            printf("dir: %s\n", dir);
        if (re[0] != '\0')
            printf("re: %s\n", re);
    }
    if (arguments.help) {
        argp_help(&argp, stdout, ARGP_HELP_STD_HELP, argv[0]);
        exit(EXIT_SUCCESS);
    }
    if (arguments.f_version) {
        printf("C-Menu-%s\n", CM_VERSION);
    }
    if (arguments.f_verbose) {
        printf("FT_BLK: %08b\n", FT_BLK);
        printf("FT_CHR: %08b\n", FT_CHR);
        printf("FT_DIR: %08b\n", FT_DIR);
        printf("FT_FIFO: %08b\n", FT_FIFO);
        printf("FT_LNK: %08b\n", FT_LNK);
        printf("FT_REG: %08b\n", FT_REG);
        printf("FT_SOCK: %08b\n", FT_SOCK);
        printf("FT_UNKNOWN: %08b\n", FT_UNKNOWN);
        printf("arguments.flags: %016b %08b %08b\n", arguments.flags,
               arguments.flags >> 8, arguments.flags & 0xff);
    }
    lf_find(dir, re, arguments.exclude_p, arguments.max_depth, arguments.flags);
    return true;
}
