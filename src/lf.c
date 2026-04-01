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
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

const char *argp_program_version = CM_VERSION;
const char *argp_program_bug_address = "billxwaller@gmail.com";
const char doc[] = "lf list files\vIf specified, DIRECTORY is the top-level "
                   "directory to search. REGULAR_EXPRESSION is a properly "
                   "formatted regular expression for which matching files "
                   "will be listed.";

static char args_doc[] = "[DIRECTORY] [REGULAR_EXPRESSION]";

static struct argp_option options[] = {
    {"f_all", 'a', 0, 0, "List hidden files", 0},
    {"max_depth", 'd', "number", 0, "Depth into directory", 0},
    {"exclude", 'e', "regex", 0, "Exclude regular expression", 0},
    // {"help", 'h', 0, 0, "Display help", 0},
    {"f_ignore_case", 'i', 0, 0, "Search ignore case", 0},
    {"file_types", 't', "bcdplfsu", 0,
     "b - block device, c - character device,  d - directory, p - named pipe,  "
     "l - symbolic link,  f - regular file, s - socket, u - unknown",
     0},
    {0}};

struct lf_opts {
    bool f_all;
    int max_depth;
    char *exclude_p;
    bool f_ignore_case;
    int flags;
    char *file_types_p;
    char *args[2];
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct lf_opts *lf_opts = state->input;
    int i = 0;
    switch (key) {
    case 'a':
        lf_opts->flags |= LF_ALL;
        break;
    case 'd':
        lf_opts->max_depth = atoi(arg);
        break;
    case 'e':
        lf_opts->exclude_p = arg;
        lf_opts->flags |= LF_EXC_REGEX;
        break;
    case 'i':
        lf_opts->flags |= LF_ICASE;
        break;
    case 't':
        lf_opts->file_types_p = arg;
        while (lf_opts->file_types_p[i]) {
            switch (lf_opts->file_types_p[i++]) {
            case 'b':
                lf_opts->flags |= FT_BLK << 8;
                break;
            case 'c':
                lf_opts->flags |= FT_CHR << 8;
                break;
            case 'd':
                lf_opts->flags |= FT_DIR << 8;
                break;
            case 'p':
                lf_opts->flags |= FT_FIFO << 8;
                break;
            case 'l':
                lf_opts->flags |= FT_LNK << 8;
                break;
            case 'f':
                lf_opts->flags |= FT_REG << 8;
                break;
            case 's':
                lf_opts->flags |= FT_SOCK << 8;
                break;
            case 'u':
                lf_opts->flags |= FT_UNKNOWN << 8;
                break;
            default:
                break;
            }
        }
        break;
    case ARGP_KEY_ARG:
        if (state->arg_num == 0 || state->arg_num == 1) {
            lf_opts->args[state->arg_num] = arg;
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

    struct lf_opts lf_opts = {0};
    lf_opts.f_all = false;
    lf_opts.max_depth = 4;
    lf_opts.exclude_p = "";
    lf_opts.f_ignore_case = false;
    lf_opts.file_types_p = "";
    lf_opts.args[0] = nullptr;
    lf_opts.args[1] = nullptr;

    argp_parse(&argp, argc, argv, 0, 0, &lf_opts);

    if (lf_opts.args[0] != nullptr) {
        strncpy(dir, lf_opts.args[0], MAXLEN - 1);
    } else {
        strncpy(dir, ".", MAXLEN - 1);
    }
    if (lf_opts.args[1] != nullptr) {
        strncpy(re, lf_opts.args[1], MAXLEN - 1);
        lf_opts.flags |= LF_REGEX;
    } else {
        strncpy(re, ".*", MAXLEN - 1);
    }

    lf_find(dir, re, lf_opts.exclude_p, lf_opts.max_depth, lf_opts.flags);
    return true;
}
