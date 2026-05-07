/** @file lf.c
    @brief list files matching a regular expression
    @author Bill Waller
    Copyright (c) 2025
    MIT License
    billxwaller@gmail.com
    @date 2026-02-09
 */

#include <stdint.h>
#include <stdio.h>
#define __USE_XOPEN
#include <time.h>

#include <argp.h>
#include <cm.h>
#include <dirent.h>
#include <fcntl.h>
#include <grp.h>
#include <linux/limits.h>
#include <pwd.h>
#include <regex.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

struct tm tm_info;
const char *argp_program_version = CM_VERSION;
const char *argp_program_bug_address = "billxwaller@gmail.com";
const char doc[] = "lf list files\vIf specified, DIRECTORY is the top-level "
                   "directory to search. REGULAR_EXPRESSION is a properly "
                   "formatted regular expression for which matching files "
                   "will be listed.";

static char args_doc[] = "[DIRECTORY] [REGULAR_EXPRESSION]";

static struct argp_option options[] = {
    {"after", 'a', "time", 0, "Modified after YYYY-MM-DDTHH:MM:SS", 0},
    {"before", 'b', "time", 0, "Modified before YYYY-MM-DDTHH:MM:SS", 0},
    {"max_depth", 'd', "number", 0, "Depth into directory tree", 0},
    {"exclude", 'e', "regex", 0, "Exclude regular expression", 0},
    {"f_ignore_case", 'i', 0, 0, "Search ignore case", 0},
    {"f_hide", 'n', 0, 0, "Do not list hidden files", 0},
    {"permissions", 'p', "sgrwx", 0,
     "s-setuid, g-setgid, r-read, w-write, x-execute", 0},
    {"file_size_min", 's', "size", 0,
     "No Suffix-bytes, K-kilobytes, M-Megabytes, or G-Gigabytes", 0},
    {"file_types", 't', "bcdplrsu", 0,
     "b-block, c-character, d-directory, p-pipe, l-link, r-regular, s-"
     "socket, u-unknown",
     0},
    {"user", 'u', "user name", 0, "User Name of file owner ", 0},
    {"exec", 'x', "command", 0, "execute external command", 0},
    {"f_sort", 'S', "r", OPTION_ARG_OPTIONAL, "sort arg=r reverse", 0},
    {0}};

struct LF {
    int max_depth;
    char *exclude;
    bool f_ignore_case;
    bool f_sort;
    bool f_reverse;
    int flags;
    bool f_hide;
    intmax_t file_size_min;
    char *file_types_p;
    char *args[2];
    int argc;
    char exec[MAXLEN];
    time_t after;
    time_t before;
};

void external_receiver_command(struct LF lf, char *, char *, char **);

/** @brief Parse a single option.  */
static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct LF *lf = state->input;
    int i = 0;

    switch (key) {
    case 'a':
        memset(&tm_info, 0, sizeof(struct tm));
        if (strptime(arg, "%Y-%m-%dT%H:%M:%S", &tm_info) == NULL) {
            fprintf(stderr, "-a Failed to parse time string.\n");
            return 1;
        }
        lf->after = mktime(&tm_info);
        if (lf->after && lf->before && lf->after > lf->before) {
            fprintf(stderr, "-a time must be before -b time.\n");
            return 1;
        }
        break;
    case 'b':
        memset(&tm_info, 0, sizeof(struct tm));
        if (strptime(arg, "%Y-%m-%dT%H:%M:%S", &tm_info) == NULL) {
            fprintf(stderr, "-b Failed to parse time string.\n");
            return 1;
        }
        lf->before = mktime(&tm_info);
        if (lf->after && lf->before && lf->after > lf->before) {
            fprintf(stderr, "-a time must be before -b time.\n");
            return 1;
        }
        break;
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
        lf->f_hide = true;
        lf->flags |= LF_HIDE;
        break;
    case 'p':
        for (i = 0; arg[i]; i++) {
            switch (arg[i]) {
            case 'g':
                lf->flags |= LF_SETGID << 16;
                break;
            case 'r':
                lf->flags |= LF_PERM_R << 16;
                break;
            case 's':
                lf->flags |= LF_SETUID << 16;
                break;
            case 'w':
                lf->flags |= LF_PERM_W << 16;
                break;
            case 'x':
                lf->flags |= LF_PERM_X << 16;
                break;
            default:
                break;
            }
        }
        break;
    case 's':
        long long ull = a_to_ull(arg);
        lf->file_size_min = (intmax_t)ull;
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
            case 'f': // for regular files, 'f' is more intuitive than 'r'
            case 'r': // regular files are the most common type, so 'r' is also
                      // accepted
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
    case 'u':
        struct passwd *pwd = getpwnam(arg);
        if (pwd) {
            if (pwd->pw_uid > 0xffff) {
                fprintf(stderr,
                        "User '%s' has UID %d which is too large for this "
                        "program.\n",
                        arg, pwd->pw_uid);
                exit(EXIT_FAILURE);
            }
            lf->flags |= (long)(pwd->pw_uid) << 24;
            lf->flags |= LF_USER;
        } else {
            fprintf(stderr, "User '%s' not found.\n", arg);
            exit(EXIT_FAILURE);
        }
        break;
    case 'x':
        strnz__cpy(lf->exec, arg, MAXLEN - 1);
        break;
    case 'S':
        lf->f_sort = true;
        if (arg && (arg[0] == 'r' || arg[0] == 'R'))
            lf->f_reverse = true;
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
    char dir[PATH_MAX];
    char re[PATH_MAX];
    dir[0] = '\0';
    re[0] = '\0';

    struct LF lf = {0};
    lf.max_depth = 0;
    lf.exclude = nullptr;
    lf.f_ignore_case = false;
    lf.f_sort = false;
    lf.f_reverse = false;
    lf.file_types_p = 0;
    lf.args[0] = nullptr;
    lf.args[1] = nullptr;
    lf.exec[0] = '\0';
    lf.after = 0;
    lf.before = 0;
    lf.file_size_min = 0;
    // setenv("ARGP_HELP_FMT", "opt-doc-col=30", 1);
    argp_parse(&argp, argc, argv, 0, 0, &lf);
    if (lf.argc > 0) {
        if (is_directory(lf.args[0]))
            strnz__cpy(dir, lf.args[0], MAXLEN - 1);
        else if (is_symlink_to_dir(lf.args[0]))
            strnz__cpy(dir, lf.args[0], MAXLEN - 1);
        else if (is_valid_regex(lf.args[0])) {
            strnz__cpy(re, lf.args[0], MAXLEN - 1);
            lf.flags |= LF_REGEX;
        } else {
            fprintf(stderr,
                    "arg1: '%s' is neither a directory nor a valid regex.\n",
                    lf.args[0]);
            exit(EXIT_FAILURE);
        }
    }
    if (lf.argc > 1) {
        if (dir[0] == '\0' && is_directory(lf.args[1]))
            strnz__cpy(dir, lf.args[1], MAXLEN - 1);
        else if (dir[0] == '\0' && is_symlink_to_dir(lf.args[1]))
            strnz__cpy(dir, lf.args[1], MAXLEN - 1);
        else if (is_valid_regex(lf.args[1])) {
            if (lf.flags & LF_REGEX) {
                fprintf(stderr, "lf: %s is not a valid directory.\n",
                        lf.args[0]);
                exit(EXIT_FAILURE);
            }
            strnz__cpy(re, lf.args[1], MAXLEN - 1);
            lf.flags |= LF_REGEX;
        } else {
            fprintf(stderr,
                    "arg2: '%s' is neither a directory nor a valid regex.\n",
                    lf.args[1]);
            exit(EXIT_FAILURE);
        }
    }
    int eargc = 0;
    char *eargv[MAXARGS];
    if (dir[0] == '\0')
        strncpy(dir, ".", MAXLEN - 1);
    if (lf.f_sort) {
        eargv[eargc++] = strdup("sort");
        if (lf.f_reverse)
            eargv[eargc++] = strdup("-r");
        eargv[eargc] = nullptr;
        external_receiver_command(lf, dir, re, eargv);
    } else
        lf_find(dir, re, lf.exclude, lf.max_depth, lf.flags, lf.after,
                lf.before, lf.file_size_min);
    exit(EXIT_SUCCESS);
}
/** @brief Execute an external command with the output of lf_find as input.
    @param lf The LF struct containing the options and flags for lf_find.
    @param dir The directory to search for files.
    @param re The regular expression to match file names against.
    @param eargv The argument vector for the external command to execute.
    @details This function creates a child process to execute the external
   command specified by the user. It sets up a pipe to redirect the output of
   lf_find to the standard input of the external command. The parent process
   runs lf_find and writes its output to the pipe, while the child process reads
   from the pipe and executes the external command. After lf_find completes, the
   parent process waits for the child process to finish before exiting.
   */
void external_receiver_command(struct LF lf, char *dir, char *re,
                               char **eargv) {
    int wstatus;
    int save_fd = dup(STDOUT_FILENO); // save current STDOUT
    int fds[2];
    pipe(fds); // Create the pipes
    pid_t pid1 = fork();
    if (pid1 == 0) {
        close(fds[1]);              // Close child write pipe
        dup2(fds[0], STDIN_FILENO); // Clone child read pipe to STDIN_FILENO
        close(fds[0]);              // Close child read pipe after cloning
        execvp(eargv[0], eargv);    // Execute the external command
    }
    close(fds[0]);               // Close read end of pipe
    dup2(fds[1], STDOUT_FILENO); // Clone write pipe to STDOUT_FILENO
    close(fds[1]);               // Close duplicated write pipe
    setvbuf(stdout, NULL, _IOLBF, 0);
    lf_find(dir, re, lf.exclude, lf.max_depth, lf.flags, lf.after, lf.before,
            lf.file_size_min);    // Generate the output for external command
    dup2(save_fd, STDOUT_FILENO); // restore STDOUT
    waitpid(pid1, &wstatus, 0);
}
