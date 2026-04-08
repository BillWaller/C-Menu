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
    {"max_depth", 'd', "number", 0, "Depth into directory", 0},
    {"exclude", 'e', "regex", 0, "Exclude regular expression", 0},
    {"f_ignore_case", 'i', "bool", 0, "Search ignore case", 0},
    {"f_hide", 'n', 0, 0, "Don't list hidden files", 0},
    {"file_types", 't', "bcdplfsu", 0,
     "b - block device, c - character device,  d - directory, p - named pipe,  "
     "l - symbolic link,  f - regular file, s - socket, u - unknown",
     0},
    {0}};

struct lf {
    int max_depth;
    char *exclude;
    bool f_ignore_case;
    int flags;
    char *file_types_p;
    char *args[2];
    int argc;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct lf *lf = state->input;
    int i = 0;
    switch (key) {
    case 'd':
        lf->max_depth = atoi(arg);
        break;
    case 'e':
        lf->exclude = arg;
        lf->flags |= LF_EXC_REGEX;
        break;
    case 'i':
        lf->flags |= LF_ICASE;
        break;
    case 'n':
        lf->flags |= LF_HIDE;
        break;
    case 't':
        lf->file_types_p = arg;
        while (lf->file_types_p[i]) {
            switch (lf->file_types_p[i++]) {
            case 'b':
                lf->flags |= FT_BLK << 8;
                break;
            case 'c':
                lf->flags |= FT_CHR << 8;
                break;
            case 'd':
                lf->flags |= FT_DIR << 8;
                break;
            case 'p':
                lf->flags |= FT_FIFO << 8;
                break;
            case 'l':
                lf->flags |= FT_LNK << 8;
                break;
            case 'f':
                lf->flags |= FT_REG << 8;
                break;
            case 's':
                lf->flags |= FT_SOCK << 8;
                break;
            case 'u':
                lf->flags |= FT_UNKNOWN << 8;
                break;
            default:
                break;
            }
        }
        break;
    case ARGP_KEY_ARG:
        if (state->arg_num == 0 || state->arg_num == 1) {
            lf->args[state->arg_num] = arg;
            lf->argc = state->arg_num + 1;
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

    struct lf lf = {0};
    lf.max_depth = 0;
    lf.exclude = nullptr;
    lf.f_ignore_case = false;
    lf.file_types_p = 0;
    lf.args[0] = nullptr;
    lf.args[1] = nullptr;
    argp_parse(&argp, argc, argv, 0, 0, &lf);
    if (lf.argc > 0) {
        if (is_directory(lf.args[0])) {
            strnz__cpy(dir, lf.args[0], MAXLEN - 1);
        } else {
            if (is_valid_regex(lf.args[0])) {
                strnz__cpy(re, lf.args[0], MAXLEN - 1);
                lf.flags |= LF_REGEX;
            } else {
                printf("arg1: '%s' is neither a directory nor a valid regex.\n",
                       lf.args[0]);
            }
        }
    }
    if (lf.argc > 1) {
        if (dir[0] == '\0' && is_directory(lf.args[1])) {
            strnz__cpy(dir, lf.args[1], MAXLEN - 1);
        } else {
            if (re[0] == '\0' && is_valid_regex(lf.args[1])) {
                strnz__cpy(re, lf.args[1], MAXLEN - 1);
                lf.flags |= LF_REGEX;
            } else {
                printf("arg2: '%s' is neither a directory nor a valid regex.\n",
                       lf.args[1]);
            }
        }
    }
    if (dir[0] == '\0')
        strncpy(dir, ".", MAXLEN - 1);
    lf_find(dir, re, lf.exclude, lf.max_depth, lf.flags);
    return 0;
}
