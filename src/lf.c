/** @file lf.c
    @v0.3.3
    @brief list files based on matching criteria
    @author Bill Waller
    Copyright (c) 2026
    MIT License
    billxwaller@gmail.com
    @date 2026-09-22
 */
#define _GNU_SOURCE
#include "lf.h"
#include <argp.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <inttypes.h>
#include <linux/limits.h>
#include <locale.h>
#include <pthread.h>
#include <pwd.h>
#include <regex.h>
#include <stdarg.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <valgrind/helgrind.h>

MemMetrics mm;
char em0[MAXLEN];
char em1[MAXLEN];
char em2[MAXLEN];
char em3[MAXLEN];
char em4[MAXLEN];

struct tm tm_info;

// char *xpath;
const char *argp_program_version = LF_VERSION;
const char *argp_program_bug_address = "billxwaller@gmail.com";
const char doc[] =
    _("lf list files\nIf specified, DIRECTORY is the top-level\n directory to "
      "search. REGULAR_EXPRESSION is a properly\nformatted regular expression "
      "for which matching files\nwill be listed.");
bool is_hidden(const char *);
bool is_dirsys(const char *);
static char args_doc[] = "[DIRECTORY] [REGULAR_EXPRESSION]";
TerminationStatus termination_status;
const char *log_s[] = {LOG_LEVEL_LIST(AS_STRINGS)};
int stdin_fd, stdout_fd, stderr_fd, tty_fd, pipe_in, pipe_out;
log_level ll_t[] = {
    [NONE] = {'1', "NONE", 0, "\033[37m"},
    [CONFIG] = {'1', "CONFIG", 0b1, "\033[37m"},
    [INFO] = {'2', "INFO", 0b10, "\033[1;30;46m"},
    [ERROR] = {'3', "ERROR", 0b100, "\033[1;38;2;255;150;0m"},
    [DEBUG] = {'4', "DEBUG", 0b1000, "\033[31m"},
    [CYCLE] = {'5', "CYCLE", 0b10000, "\033[33m"},
    [ENQUEUE] = {'6', "ENQUEUE", 0b100000, "\033[35m"},
    [DEQUEUE] = {'7', "DEQUEUE", 0b1000000, "\033[36m"},
    [ALL] = {'A', "ALL", 0b1111111, "\033[1;47;30m"},
    [COUNT_ERRORS] = {'C', "COUNT_ERRORS", 0b100000000, ""},
    [ONLY_ERRORS] = {'O', "ONLY_ERRORS", 0b1000000000, ""},
    [SPIN_COUNT] = {'S', "SPIN_COUNT", 0b10000000000, "\033[1;40;35m"},
    [STAGING] = {'T', "STAGING", 0b100000000000, "\033[1;40;35m"},
    [LOG_LEVEL_COUNT] = {'T', "LOG_LEVEL_COUNT", 0b1000000000000, ""},
};
const char *tss_s[] = {TIMESTAMP_STYLE(AS_STRINGS)};
const char *sgr_s[] = {ANSI_COLOR_LIST(AS_STRINGS)};
const char *sgr_c[] = {
    [BLACK] = "\033[30m",
    [RED] = "\033[31m",
    [GREEN] = "\033[32m",
    [YELLOW] = "\033[33m",
    [BLUE] = "\033[34m",
    [CYAN] = "\033[35m",
    [MAGENTA] = "\033[36m",
    [WHITE] = "\033[37m",
    [BBLACK] = "\033[1;30m",
    [BRED] = "\033[1;31m",
    [BGREEN] = "\033[1;32m",
    [BYELLOW] = "\033[1;33m",
    [BBLUE] = "\033[1;34m",
    [BCYAN] = "\033[1;35m",
    [BMAGENTA] = "\033[1;36m",
    [BWHITE] = "\033[1;37m",
    [ORANGE] = "\033[38;5;214m",
    [BORANGE] = "\033[1;38;2;255;150;0m",
    [RESET] = "\033[0m",
};
int lfargc;
char *lfargs[3];
char *exec;
char *file_types_p;
char *perms_p;
char *debug_p;
char *log_p;
static size_t path_data_alloc(GlobalContext *__G, const char *path, size_t len);
unsigned char const lf_type_mask[15] = {
    0, 0b00000001, 0b00000010, 0, 0b00000100, 0, 0b00001000, 0,
    0b00010000, 0, 0b00100000, 0, 0b01000000, 0, 0b10000000};
// ---------------------------------------------------------------
static struct argp_option options[] = {
    {_("after"), 'a', _("time"), 0,
     _("Last Modified after YYYY-MM-DDTHH:MM:SS"), 0},
    {_("before"), 'b', _("time"), 0,
     _("Last Modified before YYYY-MM-DDTHH:MM:SS"), 0},
    {_("max_depth"), 'd', _("number"), 0, _("Depth into directory tree"), 0},
    {_("depth_first"), 'W', _("bool"), 0, _("Depth first order (widdershins)"),
     0},
    {_("unlink"), 'U', _("bool"), 0, _("unlink (delete) implies depth_first"),
     0},
    {_("xdev"), 'x', _("bool"), 0, _("Do not traverse mount points"),
     0},
    {_("ere"), 'e', _("regex"), 0, _("Exclude regular expression"), 0},
    {_("ignore_case"), 'i', 0, 0, _("Search ignore case"), 0},
    {_("include_perms"), 'p', _("sgrwx"), 0,
     _("x-execute, w-write, r-read, s-setuid, g-setgid"), 0},
    {_("re"), 'r', _("regex"), 0, _("Regular expression to search for"), 0},
    {_("include_types"), 't', _("pcdbflsu"), 0,
     _("p-pipe, c-character_dev, d-directory, b-block_dev, f-regular_file, "
       "l-link, s-socket, u-unknown"),
     0},
    {_("file_size_min"), 's', _("size"), 0,
     _("No Suffix-bytes, K-kilobytes, M-Megabytes, or G-Gigabytes"), 0},
    {_("user"), 'u', _("user name"), 0, _("User Name of file owner "), 0},
    {_("file_level_mask"), 'l', "2345EDAC", 0,
     _("2- INFO, 3-ERROR, 4-DEBUG, 5-CYCLE, E-ENQUEUE, D-DEQUEUE, A-ALL, "
       "C-COUNT_ERRORS"),
     0},
    {_("timestamp_style"), 'y', "CZL", 0,
     _("timestamp_style for logging 1-CODER, 2-ISO8601_ZULU, 3-ISO8601_LOCAL"),
     0},
    {_("disp_level_mask"), 'D', "12345EDACO", 0,
     _("1-CONFIG, 2- INFO, 3-ERROR, 4-DEBUG, 5-CYCLE, E-ENQUEUE, D-DEQUEUE, "
       "A-ALL, C-COUNT_ERRORS, O-ONLY_ERRORS"),
     0},
    {_("include_hidden"), 'H', "o", OPTION_ARG_OPTIONAL,
     _("Include hidden files (o=hidden only)"), 0},
    {_("follow_links"), 'L', 0, 0, _("Follow symbolic links"), 0},
    {_("sort_reverse"), 'R', 0, 0, _("Sort in Reverse order"), 0},
    {_("sort"), 'S', 0, 0, _("Sort in Ascending order"), 0},
    {_("nthreads"), 'T', "threads", 0, _("Number of nthreads"), 0},
    {_("count"), 'c', "s", OPTION_ARG_OPTIONAL,
     _("Count (s only report count)"), 0},
    {0}};
/** @brief Parse a single option.
    @param key The option key.
    @param arg The argument associated with the option.
    @param state The current state of the argument parser.
    @return 0 on success, ARGP_ERR_UNKNOWN for unknown options.
    @details This function is called by the argp library to handle each
    command-line option. It updates the GlobalContext structure based on the
    provided options and arguments. It also performs validation and error
    handling for specific options, such as ensuring that the before time is
    greater than the after time.
  */
static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    GlobalContext *__G = state->input;
    int i = 0;
    bool a_toi_error = false;
    switch (key) {
    case 'a':
        parse_local_timestamp(arg, &__G->before);
        if (__G->after && __G->before && __G->before < __G->after) {
            fprintf(stderr, _("-b time must be greater than -a time.\n"));
            __G->before = 0;
        }
        break;
    case 'c':
        __G->count = true;
        if (arg && arg[0] != '\0') {
            if (strcmp(arg, "s") == 0)
                __G->count_silently |= true;
        }
        break;
    case 'd':
        if (arg && arg[0] != '\0')
            __G->max_depth = a_toi(arg, &a_toi_error);
        break;
    case 'e':
        __G->ere = strdup(arg);
        __G->flags |= LF_EXC_REGEX;
        break;
    case 'H':
        __G->include_hidden = true; // Include hidden files
        if (arg && arg[0] == 'o') {
            __G->hidden_only = true;
            __G->include_hidden = true;
        }
        __G->flags &= ~(LF_HIDE); // Turn hide flag off
                                  // LF_HIDE = 0 - include hidden files,
                                  // LF_HIDE = 1 - suppress hidden files
        break;
    case 'i':
        __G->ignore_case = true;
        break;
    case 'D':
        // "1-CONFIG, 2-INFO, 3-ERROR, 4-DEBUG, 5-CYCLE, E-ENQUEUE
        // D-DEQUEUE, A-ALL, C-COUNT_ERRORS, O-ONLY_ERRORS"
        // "12345EDACO"
        if (arg && arg[0] != '\0') {
            log_p = strdup(arg);
            while (log_p[i]) {
                switch (log_p[i]) {
                case '1': // CONFIG
                    __G->disp_level_mask |= ll_t[CONFIG].bits;
                    break;
                case '2': // INFO:
                    __G->disp_level_mask |= ll_t[INFO].bits;
                    break;
                case '3': // ERROR
                    __G->disp_level_mask |= ll_t[ERROR].bits;
                    break;
                case '4': // DEBUG
                    __G->disp_level_mask |= ll_t[DEBUG].bits;
                    break;
                case '5': // CYCLE
                    __G->disp_level_mask |= ll_t[CYCLE].bits;
                    break;
                case 'E': // ENQUEUE
                    __G->disp_level_mask |= ll_t[ENQUEUE].bits;
                    break;
                case 'D': // DEQUEUE
                    __G->disp_level_mask |= ll_t[DEQUEUE].bits;
                    break;
                case 'A': // ALL
                    __G->disp_level_mask |= ll_t[ALL].bits;
                    break;
                case 'C': // COUNT_ERRORS
                    __G->disp_level_mask |= ll_t[COUNT_ERRORS].bits;
                    break;
                case 'O': // ONLY_ERRORS
                    __G->disp_level_mask |= ll_t[ONLY_ERRORS].bits;
                    break;
                default:
                    break;
                }
                i++;
            }
            free(log_p);
        }
        break;
    case 'l':
        // "          2-INFO, 3-ERROR, 4-DEBUG, 5-CYCLE, E-ENQUEUE
        // D-DEQUEUE, A-ALL, C-COUNT_ERRORS, O-ONLY_ERRORS"
        // "12345EDACO"
        if (arg && arg[0] != '\0') {
            log_p = strdup(arg);
            while (log_p[i]) {
                switch (log_p[i]) {
                case '2': // INFO:
                    __G->file_level_mask |= ll_t[INFO].bits;
                    break;
                case '3': // ERROR
                    __G->file_level_mask |= ll_t[ERROR].bits;
                    break;
                case '4': // DEBUG
                    __G->file_level_mask |= ll_t[DEBUG].bits;
                    break;
                case '5': // CYCLE
                    __G->file_level_mask |= ll_t[CYCLE].bits;
                    break;
                case 'E': // ENQUEUE
                    __G->file_level_mask |= ll_t[ENQUEUE].bits;
                    break;
                case 'D': // DEQUEUE
                    __G->file_level_mask |= ll_t[DEQUEUE].bits;
                    break;
                case 'A': // ALL
                    __G->file_level_mask |= ll_t[ALL].bits;
                    break;
                case 'C': // COUNT_ERRORS
                    __G->file_level_mask |= ll_t[COUNT_ERRORS].bits;
                    break;
                case 'O': // ONLY_ERRORS
                    __G->file_level_mask |= ll_t[ONLY_ERRORS].bits;
                    break;
                default:
                    break;
                }
                i++;
            }
            free(log_p);
        }
        break;
    case 'L':
        __G->follow_links = true; // follow symbolic links
        break;
    case 'p':
        perms_p = arg;
        while (perms_p[i]) {
            switch (perms_p[i++]) {
            case 'g':
                __G->include_perms |= LF_ISGID;
                break;
            case 'r':
                __G->include_perms |= LF_IRUSR;
                break;
            case 's':
                __G->include_perms |= LF_ISUID;
                break;
            case 'w':
                __G->include_perms |= LF_IWUSR;
                break;
            case 'x':
                __G->include_perms |= LF_IXUSR;
                break;
            default:
                break;
            }
        }
        break;
    case 'R':
        __G->sort_reverse = true;
        break;
    case 'r':
        __G->re = strdup(arg);
        __G->flags |= LF_REGEX;
        break;
    case 'S':
        __G->sort = true;
        break;
    case 'W':
        __G->depth_first = true;
        break;
    case 'U':
        __G->unlink = true;
        break;
    case 's':
        __G->file_size_min = (intmax_t)(a_to_ul(arg));
        break;
    case 'T':
        if (arg && arg[0] != '\0')
            __G->nthreads = a_toi(arg, &a_toi_error);
        break;
    case 't':
        file_types_p = arg;
        while (file_types_p[i]) {
            switch (file_types_p[i++]) {
            case 'b':
                __G->include_types |= LF_BLK;
                break;
            case 'c':
                __G->include_types |= LF_CHR;
                break;
            case 'd':
                __G->include_types |= LF_DIR;
                break;
            case 'p':
                __G->include_types |= LF_FIFO;
                break;
            case 'l':
                __G->include_types |= LF_LNK;
                break;
            case 'f': // for regular files, 'f' is more intuitive than 'r'
            case 'r': // regular files are the most common type, so 'r' is also
                      // accepted
                __G->include_types |= LF_REG;
                break;
            case 's':
                __G->include_types |= LF_SOCK;
                break;
            case 'u':
                __G->include_types |= LF_UNKNOWN;
                break;
            default:
                break;
            }
        }
        break;
    case 'u':
        __G->user_name = strdup(arg);
        struct passwd *pwd = getpwnam(arg);
        if (pwd) {
            __G->user_id = (uintmax_t)pwd->pw_uid;
            __G->flags |= LF_USER;
        } else
            fprintf(stderr, _("User '%s' not found.\n"), arg);
        break;
    case 'x':
        __G->xdev = true;
        break;
    case 'y':
        if (arg && arg[0] == 'Z')
            __G->timestamp_style = ISO8601_ZULU;
        else if (arg && arg[0] == 'L')
            __G->timestamp_style = ISO8601_LOCAL;
        else
            __G->timestamp_style = CODER;
        break;
    case ARGP_KEY_ARG:
        if (state->arg_num == 0 || state->arg_num == 1) {
            lfargs[state->arg_num] = arg;
            lfargc = state->arg_num + 1;
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
                           nullptr, nullptr, nullptr};
int main(int argc, char **argv) {
    int stack_base;
    mm.stack_base = (uintptr_t)&stack_base;
    GlobalContext *__G = (GlobalContext *)calloc(1, sizeof(GlobalContext));
    __G->count = 0;
    termination_status = TS_ERROR;
    __G->error_count = 0;
    __G->disp_level_mask = 0;
    __G->file_level_mask = 0;
    __G->f_logging = false;
    __G->log_fd = -1;
    __G->log_error_count = false;
    __G->nthreads = 0;
    __G->ignore_case = false;
    __G->sort = false;
    __G->sort_reverse = false;
    __G->timestamp_style = CODER;
    __G->include_hidden = false; // By default, hidden files are suppressed.
    __G->hidden_only = false;
    __G->flags |= LF_HIDE; // Turn hide flag on
    __G->depth_first = false;
    __G->unlink = false;
    __G->follow_links = false;
    __G->count = false;
    __G->count_silently = false;
    __G->sbuffer = _("4G");
    __G->xdev = false;
    setlocale(LC_NUMERIC, "");
    char tmp_str[MAX_PATH_LEN];
    argp_parse(&argp, argc, argv, 0, 0, __G);
    if (lfargc > 0) {
        strnz__cpy(tmp_str, lfargs[0], MAXLEN - 1);
        expand_tilde(tmp_str, MAXLEN - 1);
        if (is_directory(tmp_str) || is_symlink_to_dir(tmp_str)) {
            __G->base_path = strdup(tmp_str);
        } else if (is_valid_regex(lfargs[0])) {
            __G->re = strdup(lfargs[0]);
            __G->flags |= LF_REGEX;
        } else {
            fprintf(stderr,
                    _("lf: arg1: '%s' is neither a directory nor a valid regex.\n"),
                    lfargs[0]);
            exit(EXIT_FAILURE);
        }
    }
    if (lfargc > 1) {
        if (!__G->base_path || __G->base_path[0] == '\0') {
            strnz__cpy(tmp_str, lfargs[1], MAXLEN - 1);
            expand_tilde(tmp_str, MAXLEN - 1);
            if (is_directory(tmp_str) || is_symlink_to_dir(tmp_str))
                __G->base_path = strdup(tmp_str);
        }
        if ((!(__G->flags & LF_REGEX)) && is_valid_regex(lfargs[1])) {
            __G->re = strdup(lfargs[1]);
            __G->flags |= LF_REGEX;
        } else {
            fprintf(stderr,
                    _("lf: '%s' is neither a directory nor a valid regular "
                      "expression.\n"),
                    lfargs[1]);
            exit(EXIT_FAILURE);
        }
    }
    if (__G->base_path == nullptr || __G->base_path[0] == '\0')
        __G->base_path = strdup(".");
    unsigned int nprocs = get_nprocs();
    if (nprocs == 0)
        nprocs = 1;
    if (__G->nthreads == 0)
        __G->nthreads = (nprocs > 1) ? (nprocs - 1) : 1;
    else {
        if (__G->nthreads < 1)
            __G->nthreads = 1;
        if (__G->nthreads > nprocs)
            __G->nthreads = nprocs;
    }
    termination_status = TS_SUCCESS;
    if (!__G->sort) {
        init_lf(__G, argc, argv);
    } else
        termination_status = sort_lf_output(__G, argc, argv);
    if (__G->count) {
        size_t count = atomic_load(&__G->file_count);
        fprintf(stderr, _("Files: %zu\n"), count);
    }
    atomic_load(&__G->error_count);
    if (__G->log_error_count && __G->error_count > 0) {
        fprintf(stderr, _("Errors: %zu\n"), __G->error_count);
        termination_status |= TS_ERROR;
    }
    free(__G->base_path);
    free(__G->user_name);
    free(__G->re);
    free(__G->ere);
    free(__G->threads);
    free(__G);
    exit(termination_status);
}
/** @brief Sort the output of the lf command using the system's sort utility.
    @param lf A pointer to the GlobalContext struct containing the search settings.
    @param argc The number of command-line arguments.
    @param argv An array of command-line argument strings.
    @details This function sets up a pipeline to sort the output of the lf
   command. It creates a pipe, forks a child process to execute the sort
   command, and redirects the standard input and output streams accordingly. The
   sort command is executed with optional flags for parallel processing and
   buffer size, as well as a reverse sort option if specified in the GlobalContext. The
   function waits for the child process to complete before returning.
   */
int sort_lf_output(GlobalContext *__G, int argc, char **argv) {
    // char tmp_str[MAXLEN];
    char *eargv[MAX_ARGS];
    int eargc = 0;
    eargv[eargc++] = strdup("sort");
    // ssnprintf(tmp_str, MAXLEN - 1, "--parallel=%d", __G->nthreads);
    // eargv[eargc++] = strdup(tmp_str);
    //  ssnprintf(tmp_str, MAXLEN - 1, "--buffer-size=%s", __G->sbuffer);
    //  eargv[eargc++] = strdup(tmp_str);
    if (__G->sort_reverse)
        eargv[eargc++] = strdup("-r");
    eargv[eargc] = nullptr;
    setenv("LC_ALL", "C", 1); // Set locale to C for consistent sorting
    int wstatus;
    int fds[2];
    pipe(fds); // Create the pipes
    pid_t pid1 = fork();
    if (pid1 == 0) { // Child process
        // Child's STDIN will be redirected to the read end of the pipe, so that the
        // sort process will read the output of the finder from the pipe. Close the
        // write end of the pipe in the child
        close(fds[1]);
        dup2(fds[0], STDIN_FILENO); // Clone child's read pipe to STDIN_FILENO
        close(fds[0]);              // Close the original read end of the pipe
        execvp(eargv[0], eargv);    // Execute the sort command
        fprintf(stderr, _("Failed to execute sort: %s\n"), strerror(errno));
        return TS_ERROR;
    }
    fclose(stdout);
    dup2(fds[1], STDOUT_FILENO);         // Clone write pipe to STDOUT_FILENO
    stdout = fdopen(STDOUT_FILENO, "w"); // Reopen STDOUT as a stream
    setvbuf(stdout, NULL, _IOLBF, 0);    //  line buffering
    init_lf(__G, argc, argv);
    fclose(stdout);
    close(fds[1]);
    wait(&wstatus);
    for (int i = 0; i < eargc; i++)
        free(eargv[i]);
    return 0;
}
// ----------------------------------------------------------------------
// DEBUG_OUT
// ----------------------------------------------------------------------
/** @brief Display everything and the kitchen sink on stderr
    @param lf A pointer to the GlobalContext struct containing the debug settings.
    @param argc The number of command-line arguments.
    @param argv An array of command-line argument strings.
    @details This function outputs debug information to stderr if debugging is
   enabled in the GlobalContext. It includes a timestamp, user information, IP
   addresses, command-line arguments, and various configuration settings. The
   output is formatted for readability and includes details about file types,
   permissions, regex patterns, and other relevant settings.
   */
void debug_out(GlobalContext *__G, int argc, char **argv) {
    int len = 0;
    int i;
    bool addspace_before = false;
    if (!(__G->disp_level_mask & ll_t[CONFIG].bits))
        return;
    for (i = 0; i < argc; i++) {
        len = len + strlen(argv[i]);
        if (len > 72) {
            fprintf(stderr, "\n");
            len = strlen(argv[i]);
            addspace_before = false;
        }
        if (addspace_before) {
            fprintf(stderr, " ");
            len++;
        }
        fprintf(stderr, "%s", argv[i]);
        addspace_before = true;
    }
    // "1-CONFIG, 2-INFO, 3-ERROR, 4-DEBUG, 5-CYCLE, E-ENQUEUE
    // D-DEQUEUE, C-COUNT_ERRORS, A-ALL, O-ONLY_ERRORS"
    // "12345EDCAO"
    fprintf(stderr, "\n\n");
    fprintf(stderr, "%s\n\n", LF_VERSION);
    fprintf(stderr, "%s\n", _("Display Log Level"));
    fprintf(stderr, _("  1-config       %s\n"),
            __G->disp_level_mask & ll_t[CONFIG].bits ? _("true")
                                                     : _("|    false"));
    fprintf(stderr, _("  2-info         %s\n"),
            __G->disp_level_mask & ll_t[INFO].bits ? _("true") : _("|    false"));
    fprintf(stderr, _("  3-errors       %s\n"),
            __G->disp_level_mask & ll_t[ERROR].bits ? _("true") : _("|    false"));
    fprintf(stderr, _("  4-debug        %s\n"),
            __G->disp_level_mask & ll_t[DEBUG].bits ? _("true") : _("|    false"));
    fprintf(stderr, _("  5-cycle        %s\n"),
            __G->disp_level_mask & ll_t[CYCLE].bits ? _("true") : _("|    false"));
    fprintf(stderr, _("  E-enqueue      %s\n"),
            __G->disp_level_mask & ll_t[ENQUEUE].bits ? _("true")
                                                      : _("|    false"));
    fprintf(stderr, _("  D-dequeue      %s\n"),
            __G->disp_level_mask & ll_t[DEQUEUE].bits ? _("true")
                                                      : _("|    false"));
    fprintf(stderr, _("  A-all           %s\n"),
            __G->disp_level_mask & ll_t[ALL].bits ? _("true") : _("|    false"));
    fprintf(stderr, _("  C-count errors %s\n"),
            __G->disp_level_mask & ll_t[COUNT_ERRORS].bits ? _("true")
                                                           : _("|    false"));
    fprintf(stderr, _("  O-only errors   %s\n"),
            __G->disp_level_mask & ll_t[ONLY_ERRORS].bits ? _("true")
                                                          : _("|    false"));
    fprintf(stderr, "\n");
    fprintf(stderr, _("Search directory: %s\n\n"), __G->base_path);
    if (__G->max_depth == 0)
        fprintf(stderr, _("Max depth 0 (unlimited)\n\n"));
    else
        fprintf(stderr, _("Max depth %d\n\n"), __G->max_depth);
    fprintf(stderr, _("Using %d threads\n\n"), __G->nthreads);
    fprintf(
        stderr,
        _("File types preceeded by an asterisk (\"*\") will be included:\n\n"));
    fprintf(stderr, _("  LF type        DT type\n"));
    print_file_type(__G->include_types, LF_FIFO, DT_FIFO,
                    _("FIFO    p-named pipe"));
    print_file_type(__G->include_types, LF_CHR, DT_CHR,
                    _("CHR     c-character device"));
    print_file_type(__G->include_types, LF_DIR, DT_DIR, _("DIR     d-directory"));
    print_file_type(__G->include_types, LF_BLK, DT_BLK,
                    _("BLK     b-block device"));
    print_file_type(__G->include_types, LF_REG, DT_REG,
                    _("REG     f-regular file"));
    print_file_type(__G->include_types, LF_LNK, DT_LNK,
                    _("LINK    l-symbolic link"));
    print_file_type(__G->include_types, LF_SOCK, DT_SOCK, _("SOCK    s-socket"));
    print_file_type(__G->include_types, LF_UNKNOWN, DT_UNKNOWN,
                    _("UNKNOWN u-unknown"));
    fprintf(stderr, "\n");
    fprintf(stderr, "f->include_types  = %08b\n", __G->include_types);
    fprintf(stderr, "f->suppress_types = %08b\n", __G->suppress_types);
    fprintf(stderr, "\n");
    if (__G->flags & LF_USER)
        fprintf(stderr, _("User: %s (%ju)\n"), __G->user_name, __G->user_id);
    if (__G->include_perms) {
        if (__G->include_perms & LF_IXUSR)
            fprintf(stderr, _("    %08b Execute\n"), LF_IXUSR);
        if (__G->include_perms & LF_IWUSR)
            fprintf(stderr, _("    %08b Write\n"), LF_IWUSR);
        if (__G->include_perms & LF_IRUSR)
            fprintf(stderr, _("    %08b Read\n"), LF_IRUSR);
        if (__G->include_perms & LF_ISUID)
            fprintf(stderr, _("    %08b SETUID\n"), LF_ISUID);
        if (__G->include_perms & LF_ISGID)
            fprintf(stderr, _("    %08b SETGID\n"), LF_ISGID);
    }
    fprintf(stderr, "\n");
    if (__G->flags & LF_REGEX)
        fprintf(stderr, _("Include regex: %s\n\n"), __G->re);
    if (__G->flags & LF_EXC_REGEX)
        fprintf(stderr, _("Exclude regex: %s\n\n"), __G->ere);
    if (__G->after) {
        char buf[32];
        format_local_timestamp(__G->after, buf, sizeof(buf));
        fprintf(stderr, _("Modified after: %s\n\n"), buf);
    }
    if (__G->before) {
        char buf[32];
        format_local_timestamp(__G->before, buf, sizeof(buf));
        fprintf(stderr, _("Modified before: %s\n\n"), buf);
    }
    if (__G->file_size_min) {
        const char *units[] = {"b", "Kb", "Mb", "Gb", "Tb", "Pb", "Eb"};
        off_t size = __G->file_size_min;
        i = 0;
        while (size >= 1024 && i < 6) {
            size /= 1024;
            i++;
        }
        char buffer[32];
        ssnprintf(buffer, 32, "%ld %s", size, units[i]);
        fprintf(stderr, _("Minimum file size: %s\n\n"), buffer);
    }
    if (__G->max_depth)
        fprintf(stderr, _("Max depth: %d\n\n"), __G->max_depth);
    if (__G->ignore_case)
        fprintf(stderr, _("Ignore case in regex matching.\n\n"));
    if (__G->include_hidden)
        fprintf(stderr, _("Include hidden files.\n\n"));
    if (__G->follow_links)
        fprintf(stderr, _("Follow symbolic links.\n\n"));
    if (__G->sort)
        fprintf(stderr, _("Sort output in ascending order.\n\n"));
    if (__G->sort_reverse)
        fprintf(stderr, _("Sort output in reverse order.\n\n"));
    return;
}
// ----------------------------------------------------------------------
// INIT_FIND
// ----------------------------------------------------------------------
/** @brief Initialize the file search process.
    @param lf A pointer to the GlobalContext struct containing the search settings.
    @param argc The number of command-line arguments.
    @param argv An array of command-line argument strings.
    @return true if initialization is successful, false otherwise.
    @details This function initializes the file search process based on the
    provided settings in the GlobalContext struct. It sets up file type inclusion
    and suppression, compiles regular expressions if specified, and creates a
    multi-producer, multi-consumer queue for managing directory traversal tasks.
    It also initializes threads for concurrent processing of directory entries.
   */
bool init_lf(GlobalContext *__G, int argc, char **argv) {
    if (__G->disp_level_mask || __G->file_level_mask)
        __G->f_logging = true;
    LogCtx x_log_ctx = {
        .start_nsec = 0,
        .start_sec = 0,
        .p_int_nsec = 0,
        .p_int_sec = 0,
        .record[0] = '\0',
    };
    LogCtx *log_ctx = &x_log_ctx;
    if (!__G->include_types)
        __G->include_types = 0xff;
    if (__G->include_types)
        __G->suppress_types = __G->include_types ^ 0xff;
    __G->include_hidden = !(__G->flags & LF_HIDE);
    int reti = 0;
    __G->reg_flags = REG_EXTENDED;
    if (__G->ignore_case)
        __G->reg_flags |= REG_ICASE;
    if (__G->flags & LF_REGEX) {
        reti = regcomp(&__G->compiled_re, __G->re, __G->reg_flags);
        if (reti) {
            ssnprintf(__G->log_record, MAX_LOG_REC_LEN - 1,
                      _("__G->re: '%s' Invalid pattern\n"), __G->re);
            fprintf(stderr, "%s", __G->log_record);
            if (__G->f_logging)
                lf_log(ERROR, "%s", __G->log_record);
            regfree(&__G->compiled_re);
            return false;
        }
    }
    if (__G->flags & LF_EXC_REGEX) {
        reti = regcomp(&__G->compiled_ere, __G->ere, __G->reg_flags);
        if (reti) {
            ssnprintf(__G->log_record, MAX_LOG_REC_LEN - 1,
                      _("__G->ere: '%s' Invalid pattern\n"), __G->ere);
            fprintf(stderr, "%s", __G->log_record);
            if (__G->f_logging)
                lf_log(ERROR, "%s", __G->log_record);
            regfree(&__G->compiled_ere);
            return false;
        }
    }
    debug_out(__G, argc, argv);
    termination_status = TS_SUCCESS;
    int rc = 0;
    struct stat sb;
    rc = pthread_mutex_init(&__G->output_mutex, NULL);
    if (rc != 0)
        perror(_("Mutex initialization failed"));
    rc = stat(__G->base_path, &sb);
    if (rc != 0)
        return false;
    // --------------------------------------------------------------------
    // INITIALIZE ARENAS
    // --------------------------------------------------------------------
    if (__G->f_logging)
        lf_log(INFO, "Initializing lf Memory");
    atomic_init(&__G->enqueue_pos, 0);
    atomic_init(&__G->dequeue_pos, 0);
    atomic_init(&__G->shut_down, 0);
    if (!global_arena_create(__G)) {
        termination_status = TS_ERROR;
        goto endlf;
    }
    atomic_init(&__G->thread_id, 0);
    // --------------------------------------------------------------------
    // CREATE_ROOT_NODE
    // --------------------------------------------------------------------
    // 4. Resolve the root path string and stat details to populate slot 0
    size_t path_len = strlen(__G->base_path);
    if (stat(__G->base_path, &sb) == -1) {
        perror("Failed to stat base_path");
        global_arena_destroy(__G);
        termination_status = TS_ERROR;
        goto endlf;
    }
    if (!S_ISDIR(sb.st_mode)) {
        ssnprintf(__G->log_record, MAX_LOG_REC_LEN - 1,
                  _("Warning: Base path '%s' is not a directory. No "
                    "files will be found.\n"),
                  __G->base_path);
        fprintf(stderr, "%s\n", __G->log_record);
        if (__G->f_logging)
            lf_log(ERROR, "%s", __G->log_record);
        termination_status = TS_ERROR;
        goto endlf;
    }
    if (__G->f_logging)
        lf_log(INFO, _("Root path: %s"), __G->base_path);
    // Push the root node into the queue arena
    push_root_node(__G, sb.st_dev, sb.st_ino);
    // ------------------------------------------------------------
    // LOOSE THE THREADS
    // ------------------------------------------------------------
    if (__G->f_logging)
        lf_log(INFO, _("Starting threads"));
    __G->threads = calloc(__G->nthreads, sizeof(pthread_t));
    if (!__G->threads) {
        if (__G->f_logging)
            lf_log(ERROR, _("Out of memory allocating threads"));
        fprintf(stderr, _("Out of memory allocating threads\n"));
        goto endlf;
    }
    for (unsigned int i = 0; i < __G->nthreads; i++) {
        rc = pthread_create(&__G->threads[i], NULL, worker, __G);
        if (rc != 0) {
            if (__G->f_logging)
                lf_log(ERROR, _("Error: Unable to create thread %d\n"), rc);
            fprintf(stderr, _("Error: Unable to create thread %d\n"), rc);
            __G->shut_down = 1;
            termination_status = TS_ERROR;
            goto endlf;
        }
    }
// ------------------------------------------------------------
// THREAD SHUTDOWN
// ------------------------------------------------------------
endlf:
    if (__G->f_logging)
        lf_log(INFO, "Waiting for threads to finish...");
    for (unsigned int i = 0; i < __G->nthreads; i++)
        pthread_join(__G->threads[i], NULL);
    rc = pthread_mutex_destroy(&__G->output_mutex);
    if (rc != 0)
        perror(_("Mutex destroy failed"));
    if (__G->flags & LF_REGEX)
        regfree(&__G->compiled_re);
    if (__G->flags & LF_EXC_REGEX)
        regfree(&__G->compiled_ere);
    global_arena_destroy(__G);
    return true;
}
// ----------------------------------------------------------------------
// GLOBAL ARENA
// ----------------------------------------------------------------------
/** @brief Create a queue arena for managing directory traversal tasks.
    @param lf A pointer to the GlobalContext struct where the queue arena will be
   created.
    @param max_nodes The maximum number of nodes that the queue arena can hold.
    @details This function allocates memory for a QueueArena structure and
   initializes its properties. It uses mmap to allocate a contiguous block of
   memory for the queue nodes, allowing for efficient management of directory
   traversal tasks in a multi-threaded environment. The cursor is initialized to
   zero, indicating that no nodes have been added yet.
   */
bool global_arena_create(GlobalContext *__G) {
    __G->queue_bytes = sizeof(QueueNode) * QUEUE_NODES;
    __G->path_bytes = 32 * QUEUE_NODES;
    __G->arena_bytes = (__G->queue_bytes + __G->path_bytes);
    __G->arena = mmap(NULL, __G->arena_bytes, PROT_NONE,
                      MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
    mprotect(__G->arena, __G->arena_bytes, PROT_READ | PROT_WRITE);
    if (!__G->arena || __G->arena == MAP_FAILED) {
        perror(_("mmap failed to reserve queue arena memory"));
        return false;
    }
    __G->queue = (QueueNode *)__G->arena;
    __G->path_data = (char *)__G->arena + __G->queue_bytes;
    for (size_t i = 0; i < QUEUE_NODES; i++)
        atomic_init(&__G->sequence[i], i);
    return true;
}
/** @brief Push the root directory node into the queue arena.
    @param lf A pointer to the GlobalContext struct containing the queue arena.
    @param path_offset The offset of the root path in the path arena.
    @param path_len The length of the root path string.
    @param dev The device ID of the root directory.
    @param ino The inode number of the root directory.
    @return The index of the newly created root node in the queue arena.
    @details This function initializes and pushes the root directory node into
   the queue arena. It allocates a new directory context and cycle context for
   the root node, sets up its properties, and returns its index in the queue
   arena. This is typically called at the start of a file search operation to
   establish the starting point for traversal.
   */
void push_root_node(GlobalContext *__G, dev_t dev, ino_t ino) {
    QueueNode root_node = {
        .parent_idx = NO_PARENT,
        .path_offset = 0,
        .path_len = strlen(__G->base_path),
        .child_dirs_remaining = 0,
        .depth = 0,
        .dev = dev,
        .ino = ino,
    };
    __G->queue_idx = 0;
    atomic_init(&__G->active_tasks, 1);
    mpmc_enqueue(__G, &root_node, __G->base_path, strlen(__G->base_path));
    return;
}
/** @brief Destroy the queue arena and free associated resources.
    @param lf A pointer to the GlobalContext struct containing the queue arena.
    @details This function unmaps the memory allocated for the queue arena using
   munmap. It checks if the arena pointer is valid before attempting to unmap it,
   ensuring that resources are properly released when they are no longer needed.
   */
void global_arena_destroy(GlobalContext *__G) {
    if (!__G->arena)
        return;
    munmap(__G->arena, __G->arena_bytes);
}
// ----------------------------------------------------------------------
// FLUSH_OUTPUT_BUFFER
// ----------------------------------------------------------------------
/** @brief Flush the output_buffer to stdout in a thread-safe manner.
    @param lf A pointer to the GlobalContext struct containing the output mutex.
    @param output_buffer A pointer to the OutputBuffer struct containing the data to be
   flushed.
    @details This function writes the contents of the output_buffer to stdout
   using fwrite_unlocked for efficiency. It uses a mutex to ensure that only one
   thread can write to stdout at a time, preventing interleaved output from
   multiple threads. After flushing, it resets the length of the output_buffer
   to zero.
   */
void flush_output_buffer(GlobalContext *__G, OutputBuffer *output_buffer) {
    if (__G == nullptr || output_buffer == nullptr, output_buffer->len == 0)
        return;
    pthread_mutex_lock(&__G->output_mutex);
    fwrite_unlocked(output_buffer->data, 1, output_buffer->len, stdout);
    pthread_mutex_unlock(&__G->output_mutex);
    output_buffer->len = 0;
}
// ----------------------------------------------------------------------
// APPEND_OUTPUT_BUFFER
// ----------------------------------------------------------------------
/** @brief Append a path to the output_buffer, flushing if necessary.
    @param lf A pointer to the GlobalContext struct containing the output mutex.
    @param output_buffer A pointer to the OutputBuffer struct where the path will be appended.
    @param path The path string to append to the output_buffer.
    @param path_len The length of the path string.
    @param append_slash A boolean indicating whether to append a slash after the path.
    @return true if the path was successfully appended or flushed, false otherwise.
    @details This function appends a given path to the output_buffer. If the
   combined length of the current buffer and the new path exceeds the buffer's
   capacity, it flushes the buffer to stdout before appending. It also handles
   appending a slash if specified. The function ensures thread-safe access to
   stdout using a mutex during flushing.
*/
bool append_output_buffer(GlobalContext *__G, OutputBuffer *output_buffer, const char *path,
                          size_t path_len, bool append_slash) {
    if (__G == nullptr || output_buffer == nullptr || path == nullptr)
        return false;
    size_t line_len = path_len + (append_slash ? 1 : 0) + 1;
    if (line_len > sizeof(OutputBuffer)) {
        pthread_mutex_lock(&__G->output_mutex);
        fwrite_unlocked(path, 1, path_len, stdout);
        if (append_slash)
            fputc_unlocked('/', stdout);
        fputc_unlocked('\n', stdout);
        pthread_mutex_unlock(&__G->output_mutex);
        return true;
    }
    if (output_buffer->len + line_len > sizeof(output_buffer->data))
        flush_output_buffer(__G, output_buffer);
    memcpy(output_buffer->data + output_buffer->len, path, path_len);
    output_buffer->len += path_len;
    if (append_slash)
        output_buffer->data[output_buffer->len++] = '/';
    output_buffer->data[output_buffer->len++] = '\n';
    return true;
}
// ----------------------------------------------------------------------
// VYUKOV MPMC QUEUE
// ----------------------------------------------------------------------
bool mpmc_enqueue(GlobalContext *__G, QueueNode *node, const char *path, size_t len) {
    size_t pos = atomic_load_explicit(&__G->enqueue_pos, memory_order_relaxed);
    int spin_count = 0;
    const int SPIN_LIMIT = 8;
    while (true) {
        size_t seq = atomic_load_explicit(&__G->sequence[pos & QUEUE_MASK],
                                          memory_order_acquire);
        intptr_t diff = (intptr_t)seq - (intptr_t)pos;
        if (diff == 0) { // Success
            if (atomic_compare_exchange_weak_explicit(&__G->enqueue_pos, &pos,
                                                      pos + 1, memory_order_acquire,
                                                      memory_order_relaxed))
                break;
        } else if (diff < 0) { // Queue full
            // fprintf(stderr, "line: %d Insufficient contiguous nodes:
            // queue_idx %lu, seq %lu, seq - pos %lu\n", __LINE__, seq, pos, seq
            // - pos);
            return false;
        } else { // Contended, try again
            pos = atomic_load_explicit(&__G->enqueue_pos, memory_order_relaxed);
        }
        if (++spin_count > SPIN_LIMIT)
            return false;
        else {
            if (spin_count > SPIN_LIMIT / 2)
                sched_yield();
        }
#if defined(__x86_64__)
        __builtin_ia32_pause();
#endif
    }
    __G->queue[pos] = *node;
    uint32_t offset = atomic_fetch_add_explicit(&__G->path_offset, len + 1, memory_order_acquire);
    memcpy(&__G->path_data[offset], path, len);
    atomic_store_explicit(&__G->sequence[pos],
                          pos + 1, memory_order_release);
    atomic_thread_fence(memory_order_release);
    return true;
}
/** @brief Bulk enqueue multiple nodes into the MPMC queue.
    @param lf A pointer to the GlobalContext struct containing the queue.
    @param tl A pointer to the ThreadLocal struct containing the nodes to enqueue.
    @return true if the bulk enqueue was successful, false if the queue is full.
    @details This function attempts to enqueue multiple nodes from a thread-local
   buffer into a multi-producer, multi-consumer (MPMC) queue. It checks for
   available slots in the queue and uses atomic operations to reserve space for
   the nodes. If successful, it copies the nodes and their associated path data
   into the global queue and updates the sequence numbers accordingly. If the
   queue is full, it returns false, indicating that the caller should fall back
   to local stack or single enqueue.
   */
bool mpmc_bulk_enqueue(GlobalContext *__G, ThreadLocal *__tl) {
    if (__tl->stage_idx == 0)
        return true;
    uint_fast32_t spin_count = 0;
    uint_fast32_t base_idx = atomic_load_explicit(&__G->enqueue_pos, memory_order_relaxed);
    while (true) {
        bool block_available = true;
        bool queue_full = false;
        uint_fast32_t i;
        uint_fast32_t pos;
        uint_fast32_t seq;
        uint_fast32_t queue_idx;
        for (i = 0; i < __tl->stage_idx; ++i) {
            pos = base_idx + i;
            queue_idx = pos & QUEUE_MASK;
            seq = atomic_load_explicit(&__G->sequence[queue_idx], memory_order_acquire);
            intptr_t dif = (intptr_t)seq - (intptr_t)pos;
            if (dif < 0) {
                // Slot is still occupied from the previous lap -> genuinely full
                block_available = false;
                queue_full = true;
                break;
            } else if (dif > 0) {
                // Another producer already claimed this slot -> base_idx is stale
                block_available = false;
                break;
            }
        }
        if (queue_full) {
            return false; // Caller should fall back to local stack or single enqueue
        }
        if (block_available) { // Success
            if (atomic_compare_exchange_weak_explicit(
                    &__G->enqueue_pos, &base_idx, base_idx + __tl->stage_idx,
                    memory_order_relaxed, memory_order_relaxed)) {
                break; // Reservation finalized; base_idx holds our starting position
            }
            // CAS failed: base_idx was automatically updated with the current enqueue_pos
        } else {
            // dif > 0: Reload fresh base_idx before backing off/retrying
            base_idx = atomic_load_explicit(&__G->enqueue_pos, memory_order_relaxed);
        }
        if (++spin_count < 64) {
            for (i = 0; i < spin_count; i++) {
#if defined(__x86_64__)
                __builtin_ia32_pause();
#endif
            }
        } else {
            sched_yield();
        }
    }
    for (uint_fast32_t sidx = 0; sidx < __tl->stage_idx; ++sidx) {
        uint_fast32_t enqueue_pos = (base_idx + sidx) & QUEUE_MASK;
        __G->queue[enqueue_pos] = __tl->stage_node[sidx];
        uint_fast32_t len_to_add = __tl->stage_node[sidx].path_len + 1;
        uint_fast32_t offset = atomic_fetch_add_explicit(&__G->path_offset,
                                                         len_to_add, memory_order_acquire);
        if (__builtin_expect(offset + len_to_add >= PATH_BYTES, 0)) {
            fprintf(stderr, "Fatal: Path arena exhausted (%lu / %d bytes)\n", offset + len_to_add, PATH_BYTES);
            return false;
        }
        memcpy(&__G->path_data[offset],
               &__tl->path_data[__tl->stage_node[sidx].path_offset],
               __tl->stage_node[sidx].path_len);
#ifdef DEBUGZ
        char *path = &__tl->path_data[__tl->stage_node[sidx].path_offset];
        fprintf(stderr, "line: %d, %s, enqueued pos %lu, sidx %lu, path: %s\n", __LINE__, __func__, enqueue_pos, sidx, path);
#endif
        __G->queue[enqueue_pos].path_offset = offset;
        __G->path_data[offset + __tl->stage_node[sidx].path_len] = '\0';
        // ANNOTATE_HAPPENS_BEFORE(&__G->queue[enqueue_pos]);
        // forces unconditional atomic read-write that Helgrind can't
        // misinterpret
        uint_fast32_t target_seq = base_idx + sidx + 1;
        atomic_exchange_explicit(&__G->sequence[enqueue_pos],
                                 target_seq, memory_order_release);
    }
    __tl->stage_idx = 0;
    __tl->cum_path_offset = 0;
    __tl->path_data[0] = '\0';
    return true;
}
/** @brief Dequeue a node from the MPMC queue.
    @param lf A pointer to the GlobalContext struct containing the queue.
    @param dequeue_pos A pointer to a uint_fast32_t variable where the dequeued
   position will be stored.
    @return true if a node was successfully dequeued, false if the queue is empty or
   shut down.
    @details This function attempts to dequeue a node from a multi-producer,
   multi-consumer (MPMC) queue. It checks for available nodes in the queue and
   uses atomic operations to claim a node for processing. If successful, it
   updates the sequence number for the dequeued position and returns true. If the
   queue is empty or has been shut down, it returns false, indicating that there
   are no more nodes to process.
   */
bool mpmc_dequeue(GlobalContext *__G, uint_fast32_t *dequeue_pos) {
    alignas(CACHE_LINE_SIZE) uint_fast32_t pos = atomic_load_explicit(&__G->dequeue_pos, memory_order_relaxed);
    int spin_count = 0;
    while (true) {
        if (atomic_load_explicit(&__G->shut_down, memory_order_acquire)) {
            ANNOTATE_HAPPENS_AFTER(&__G->shut_down);
            return false;
        }
        alignas(CACHE_LINE_SIZE) uint_fast32_t seq = __G->sequence[pos & QUEUE_MASK];
        intptr_t diff = (intptr_t)seq - (intptr_t)(pos + 1);
        if (diff == 0) { // Catch!
            if (atomic_compare_exchange_weak_explicit(&__G->dequeue_pos, &pos,
                                                      pos + 1, memory_order_acquire, memory_order_relaxed)) {
                ANNOTATE_HAPPENS_AFTER(&__G->sequence[pos & QUEUE_MASK]);
                *dequeue_pos = pos;
                break;
            }
            spin_count = 0;
        } else if (diff < 0) { // No Joy!
            if (atomic_load_explicit(&__G->active_tasks, memory_order_acquire) == 0) {
                uint_fast32_t check_pos =
                    atomic_load_explicit(&__G->dequeue_pos, memory_order_acquire);

                uint_fast32_t enq_pos = atomic_load_explicit(&__G->enqueue_pos, memory_order_acquire);
                if (enq_pos == check_pos) {
                    uint_fast32_t check_seq = atomic_load_explicit(
                        &__G->sequence[check_pos & QUEUE_MASK], memory_order_acquire);
                    if ((intptr_t)check_seq - (intptr_t)(check_pos + 1) < 0) {
                        atomic_exchange_explicit(&__G->shut_down, 1,
                                                 memory_order_release);
                        return false;
                    }
                }
            }
            if (++spin_count < 64) {
                for (int i = 0; i < spin_count; i++) {
#if defined(__x86_64__)
                    __builtin_ia32_pause();
#endif
                }
            } else {
                sched_yield();
            }
            pos = atomic_load_explicit(&__G->dequeue_pos, memory_order_acquire);
        } else {
            // Snatched by another thread, try again
            pos = atomic_load_explicit(&__G->dequeue_pos, memory_order_acquire);
        }
        *dequeue_pos = pos;
    }
#ifdef DEBUGX
    fprintf(stderr, "line: %d, %s, dequeued pos %lu\n", __LINE__, __func__, *dequeue_pos);
#endif
    // atomic_store_explicit(&__G->sequence[pos & QUEUE_MASK],
    //                       pos + QUEUE_NODES, memory_order_release);
    uint_fast32_t reset_seq = pos + QUEUE_NODES;
    atomic_exchange_explicit(&__G->sequence[pos & QUEUE_MASK], reset_seq, memory_order_release);
    return true;
}
/** @brief Release a slot in the MPMC queue after processing.
    @param lf A pointer to the GlobalContext struct containing the queue.
    @param pos The position of the slot to release.
    @details This function updates the sequence number for a given slot in the
   multi-producer, multi-consumer (MPMC) queue, indicating that the slot is now
   available for reuse. It uses an atomic store operation with memory_order_release
   to ensure proper synchronization between threads.
   */
static inline void mpmc_release_slot(GlobalContext *__G, uint_fast32_t pos) {
    atomic_store_explicit(&__G->sequence[pos & QUEUE_MASK],
                          pos + QUEUE_NODES, memory_order_release);
    // mpmc_release_slot(__G, pos);
}
// ----------------------------------------------------------------------
/** @brief Worker thread function for processing directory nodes.
    @param arg A pointer to the GlobalContext struct containing the search settings.
    @return NULL upon completion.
    @details This function is executed by each worker thread. It dequeues
   directory nodes from the global queue, reconstructs their paths, and invokes
   the finder function to process them. Each thread maintains its own local
   context, including a stage arena for temporary storage of nodes and path
   data. The function continues processing until there are no more nodes to
   dequeue or the search is shut down.
   */
void *worker(void *arg) {
    GlobalContext *__G = (GlobalContext *)arg;
    ThreadLocal __tl;
    __tl.stage_queue_size = sizeof(QueueNode) * STAGE_NODES;
    __tl.stage_path_size = 32 * STAGE_NODES;
    __tl.stage_arena_size = (__tl.stage_queue_size + __tl.stage_path_size);
    void *stage_arena = mmap(NULL, __tl.stage_arena_size, PROT_NONE,
                             MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
    mprotect(stage_arena, __tl.stage_arena_size, PROT_READ | PROT_WRITE);
    __tl.stage_arena = stage_arena;
    __tl.stage_node = (QueueNode *)__tl.stage_arena;
    __tl.path_data = (char *)__tl.stage_arena + __tl.stage_queue_size;
    __tl.cum_path_offset = 0;
    __tl.stage_idx = 0;
    __tl.thread_id = atomic_fetch_add_explicit(&__G->thread_id, 1, memory_order_relaxed);
    char dir_buf[DIR_BUF_SIZE];
    __tl.dir_buf = dir_buf;
    OutputBuffer output_buffer;
    __tl.output_buffer = &output_buffer;
    __tl.output_buffer->len = 0;
    uint_fast32_t q_pos = 0;
    LogCtx log_ctx = {};
    __tl.log_ctx = log_ctx;
    log_ctx.thread_id = __tl.thread_id;
    if (__G->f_logging && __G->log_fd == -1)
        initialize_timer(__G, &log_ctx);
    else
        initialize_thread_timer(__G, &log_ctx);
    while (1) {
        if (mpmc_dequeue(__G, &q_pos) == false) {
            break;
        }
        atomic_thread_fence(memory_order_acquire);
        // ----------------------------------------------------------------
        // Reconstruct the relative path for the dequeued directory node
        // ----------------------------------------------------------------
        __tl.queue_idx = q_pos;
        uint_fast32_t q_path_len = __G->queue[q_pos].path_len;
        uint_fast32_t q_path_offset = __G->queue[q_pos].path_offset;
        // ----------------------------------------------------------------
        // Get the length of the parent path
        // ----------------------------------------------------------------
        uint_fast32_t cum_path_len = q_path_len; // length of dequeued dir_node path
        uint_fast32_t parent_idx = __G->queue[q_pos].parent_idx;
        while (parent_idx != NO_PARENT) {
            cum_path_len += __G->queue[parent_idx].path_len + 1;
            parent_idx = __G->queue[parent_idx].parent_idx;
        }
        if (cum_path_len >= MAX_PATH_LEN) {
            atomic_fetch_sub_explicit(&__G->active_tasks, 1, memory_order_relaxed);
            continue;
        }
        // ----------------------------------------------------------------
        // Reconstruct the relative path for the dequeued directory node
        // ----------------------------------------------------------------
        uint_fast32_t write_cursor = cum_path_len;
        write_cursor -= q_path_len;
        memcpy(&__tl.path[write_cursor],
               &__G->path_data[q_path_offset],
               q_path_len);
        __tl.path[cum_path_len] = '\0';
#ifdef DEBUGX
        fprintf(stderr, "line: %d, %s, T%lu, dequeued path %s\n", __LINE__, __func__, __tl.thread_id, __tl.path);
#endif
        parent_idx = __G->queue[q_pos].parent_idx;
        uint_fast32_t p_path_len;
        uint_fast32_t p_path_offset;
        while (parent_idx != NO_PARENT) {
            write_cursor -= 1;
            __tl.path[write_cursor] = '/';
            p_path_len = __G->queue[parent_idx].path_len;
            p_path_offset = __G->queue[parent_idx].path_offset;
            write_cursor -= p_path_len;
            memcpy(&__tl.path[write_cursor],
                   &__G->path_data[p_path_offset],
                   p_path_len);
            parent_idx = __G->queue[parent_idx].parent_idx;
        }
#ifdef DEBUGY
        fprintf(stderr, "line: %d, %s, T%lu constructed path %s\n", __LINE__, __func__, __tl.thread_id, __tl.path);
#endif
        __tl.parent_path_len = cum_path_len;
        // ----------------------------------------------------------------
        // __tl->path_data now contains a relative path to the directory to open
        // ----------------------------------------------------------------
        __tl.child_dirs_remaining = 0;
        finder(__G, &__tl);
        __G->queue[q_pos].child_dirs_remaining = __tl.child_dirs_remaining;
        atomic_fetch_sub_explicit(&__G->active_tasks, 1, memory_order_relaxed);
    }
    munmap(stage_arena, __tl.stage_arena_size);
    return NULL;
}
// ----------------------------------------------------------------------
// FINDER
// ----------------------------------------------------------------------
/** @brief Traverse a directory and process its entries.
    @param lf A pointer to the GlobalContext struct containing the search settings.
    @param tl A pointer to the ThreadLocal struct containing the current directory context.
    @return NULL upon completion.
    @details This function opens a directory specified in the ThreadLocal context,
   reads its entries, and processes each entry based on the provided settings in
   the GlobalContext. It handles filtering of directory entries, retrieves
   advanced metadata if necessary, and manages the traversal of subdirectories.
   The function also logs errors and flushes output buffers as needed. It is
   designed to be called by worker threads for concurrent processing of
   directories.
   */
void *finder(GlobalContext *__G, ThreadLocal *__tl) {
    long nread;
    struct stat sb = {};
    struct linux_dirent64 *entry;
    dev_t actual_dev = 0, effective_dev = 0;
    ino_t actual_ino = 0, effective_ino = 0;
    unsigned char actual_type;
    int rc;
    LogCtx *log_ctx = &__tl->log_ctx;
    __tl->stage_idx = 0;
    __tl->stage_node->path_offset = 0;
    struct open_how how = {
        .flags = O_RDONLY | O_DIRECTORY | O_CLOEXEC,
        .mode = 0};
    how.resolve = RESOLVE_NO_MAGICLINKS;
    if (!__G->follow_links)
        how.resolve |= RESOLVE_NO_SYMLINKS;
    if (__G->xdev)
        how.resolve |= RESOLVE_NO_XDEV;
    int dir_fd = openat2(AT_FDCWD, __tl->path, &how, sizeof(struct open_how));
    if (dir_fd == -1) {
        if (__G->f_logging) {
            lf_log(ERROR, "OPEN_FAIL,%s,%s", __tl->path, strerror(errno));
        } else
            fprintf(stderr, "OPEN_FAIL,%s,%d:%s\n", __tl->path, errno, strerror(errno));
        flush_output_buffer(__G, __tl->output_buffer);
        return NULL;
    }
    while (1) {
        nread = getdents64(dir_fd, __tl->dir_buf, DIR_BUF_SIZE);
        if (__builtin_expect(nread == -1, 0)) {
            atomic_fetch_add(&__G->error_count, 1);
            if (__G->f_logging)
                lf_log(ERROR, "READDIR_FAIL,%s,%s", __tl->path, strerror(errno));
            break;
        }
        if (nread == 0)
            break;
        for (uint_fast32_t bpos = 0; bpos < (size_t)nread;) {
            entry = (struct linux_dirent64 *)(__tl->dir_buf + bpos);
            bpos += entry->d_reclen;
#ifdef DEBUGT
            fprintf(stderr, "line: %d, %s, T%lu entry d_name %s, d_type %d\n", __LINE__, __func__, __tl->thread_id, entry->d_name, entry->d_type);
#endif
            actual_type = entry->d_type;
            __tl->d_name_len = strlen(entry->d_name);
            __tl->effective_type = actual_type;
            // -------------------------------------------------------------------
            // FILTER DIRECTORY ENTRIES
            // -------------------------------------------------------------------
            if (__builtin_expect(actual_type == DT_DIR, 0)) {
                if (entry->d_name[0] == '.') {
                    if (entry->d_name[1] == '\0')
                        continue;
                    if (entry->d_name[1] == '.' && entry->d_name[2] == '\0')
                        continue;
                }
            }
            // -------------------------------------------------------------------
            // GET ADVANCED METADATA
            // -------------------------------------------------------------------
            bool need_metadata = (__G->follow_links && actual_type == DT_LNK);
            // Only evaluate the complex logging mask arrays if logging is
            // explicitly active
            if (__builtin_expect(__G->f_logging, 0)) {
                if ((__G->file_level_mask & ll_t[CYCLE].bits) ||
                    (__G->disp_level_mask & ll_t[CYCLE].bits)) {
                    need_metadata = true;
                }
            }
            if (__builtin_expect(need_metadata, 0)) {
                rc = fstatat(dir_fd, entry->d_name, &sb, AT_SYMLINK_NOFOLLOW);
                if (rc == -1) {
                    atomic_fetch_add(&__G->error_count, 1);
                    if (__G->f_logging)
                        lf_log(ERROR, "LSTAT_FAIL,%s/%s,%s", __tl->path, entry->d_name,
                               strerror(errno));
                } else {
                    actual_ino = sb.st_ino;
                    actual_dev = sb.st_dev;
                    actual_type = (sb.st_mode & S_IFMT) >> 12;
                    effective_dev = actual_dev;
                    effective_ino = actual_ino;
                    __tl->effective_type = actual_type;
                }
                if (S_ISLNK(sb.st_mode)) {
                    if (__G->follow_links) {
                        // -------------------------------------------------------
                        // GET LINK METADATA
                        // -------------------------------------------------------
                        rc = fstatat(dir_fd, entry->d_name, &sb, 0);
                        if (rc == -1) {
                            atomic_fetch_add(&__G->error_count, 1);
                            if (__G->f_logging)
                                lf_log(ERROR, _("FSTATAT_FAIL,%s/%s,%s\n"), __tl->path, entry->d_name,
                                       strerror(errno));
                        } else {
                            effective_ino = sb.st_ino;
                            effective_dev = sb.st_dev;
                            __tl->effective_type = (sb.st_mode & S_IFMT) >> 12;
                        }
                    } else {
                        __tl->effective_type = DT_REG;
                    }
                }
            }
            // --------------------------------------------------------------------
            // DIRECTORY PROCESSING
            // --------------------------------------------------------------------
            if (__tl->effective_type == DT_DIR) {
                __tl->depth = __G->queue[__tl->queue_idx].depth + 1;
                if (__G->max_depth > 0 && __tl->depth + 1 > __G->max_depth)
                    continue;
                // ----------------------------------------------------------------
                // HIGH-PERFORMANCE CYCLE DETECTION (Only if links followed)
                // ----------------------------------------------------------------
                if (__G->follow_links && actual_type == DT_LNK) {
                    bool cycle_found = false;
                    char target_path[MAX_PATH_LEN];
                    //  = {'\0'};
                    if (__builtin_expect(__G->f_logging, 0)) {
                        if ((__G->file_level_mask & ll_t[CYCLE].bits) ||
                            (__G->disp_level_mask & ll_t[CYCLE].bits)) {
                            size_t len = readlinkat(dir_fd, entry->d_name, target_path,
                                                    sizeof(target_path) - 1);
                            if (len != (size_t)-1) {
                                target_path[len] = '\0';
                                atomic_fetch_add_explicit(&__G->error_count, 1,
                                                          memory_order_relaxed);
                                lf_log(ERROR, "READLINK_FAIL,%s/%s,%s\n",
                                       __tl->path, entry->d_name, strerror(errno));
                            }
                            lf_log(CYCLE, "Target item: dev: %lu, ino: %lu, Name: %s/%s\n",
                                   effective_dev, effective_ino, __tl->path, entry->d_name);
                        }
                    }
                    // CYCLE Backtrack verification
                    uint_fast32_t current_parent_idx = __tl->queue_idx;
                    while (current_parent_idx != NO_PARENT) {
                        QueueNode *ancestor = &__G->queue[current_parent_idx];
                        if (effective_dev == ancestor->dev &&
                            effective_ino == ancestor->ino) {
                            cycle_found = true;
                            break;
                        }
                        current_parent_idx = __G->queue->parent_idx;
                    }
                    if (cycle_found) {
                        atomic_fetch_add_explicit(&__G->error_count, 1,
                                                  memory_order_relaxed);
                        build_path(__tl, entry->d_name);
                        print_file(__G, __tl);
                        continue;
                    }
                }
                // ----------------------------------------------------------------
                // STAGE BUFFERING
                // ----------------------------------------------------------------
                if ((__tl->stage_idx >= STAGE_NODES - 1) ||
                    __tl->cum_path_offset >= STAGE_PATH_BYTES - MAX_PATH_LEN) {
                    atomic_fetch_add_explicit(&__G->active_tasks, __tl->stage_idx, memory_order_relaxed);
                    if (mpmc_bulk_enqueue(__G, __tl) == false) {
                        atomic_fetch_sub_explicit(&__G->active_tasks, __tl->stage_idx, memory_order_relaxed);
                        atomic_fetch_add_explicit(&__G->error_count, 1, memory_order_relaxed);
                        if (__G->f_logging)
                            lf_log(ERROR, "STAGE_ENQUEUE_FAIL,%s\n",
                                   __tl->path);
                        return NULL;
                    }
                    __tl->stage_idx = 0;
                }
                __tl->d_name = entry->d_name;
#ifdef DEBUGT
                fprintf(stderr, "line: %d, %s, T%lu, path %s, d_name %s, tl->stage_idx %lu\n", __LINE__, __func__, __tl->thread_id, __tl->path, entry->d_name, __tl->stage_idx);
#endif
                QueueNode *stage_node = &__tl->stage_node[__tl->stage_idx];
                stage_node->parent_idx = __tl->queue_idx;
                stage_node->path_offset = __tl->cum_path_offset;
                stage_node->path_len = __tl->d_name_len;
                stage_node->depth = __tl->depth;
                stage_node->child_dirs_remaining = 0;
                stage_node->dev = effective_dev;
                stage_node->ino = effective_ino;
                memcpy(__tl->path_data + stage_node->path_offset,
                       __tl->d_name,
                       __tl->d_name_len);
                __tl->cum_path_offset += __tl->d_name_len;
                __tl->path_data[__tl->cum_path_offset++] = '\0';
                __tl->stage_idx++;
                // --------------------------------------------------------
                // CONDITIONALLY PRINT AS DIRECTORY
                // --------------------------------------------------------
                if (entry->d_name[0] == '.') {
                    if (!__G->include_hidden && !__G->hidden_only)
                        continue;
                } else {
                    if (__G->hidden_only)
                        continue;
                }
                __tl->child_dirs_remaining++;
                if (!__G->only_errors) {
                    build_path(__tl, entry->d_name);
                    print_file(__G, __tl);
                }
            } else {
                // --------------------------------------------------------
                // FILE PROCESSING FAST-PATH
                // --------------------------------------------------------
                if (entry->d_name[0] == '.') {
                    if (!__G->include_hidden && !__G->hidden_only)
                        continue;
                } else {
                    if (__G->hidden_only)
                        continue;
                }
                if (__tl->path[0] != '\0' && !__G->only_errors) {
                    build_path(__tl, entry->d_name);
                    print_file(__G, __tl);
                }
            }
        }
    }
    close(dir_fd);
    flush_output_buffer(__G, __tl->output_buffer);
    if (__tl->stage_idx > 0) {
        atomic_fetch_add_explicit(&__G->active_tasks, __tl->stage_idx, memory_order_relaxed);
        if (mpmc_bulk_enqueue(__G, __tl) == false) {
            atomic_fetch_sub_explicit(&__G->active_tasks, __tl->stage_idx, memory_order_relaxed);
            atomic_fetch_add_explicit(&__G->error_count, 1,
                                      memory_order_relaxed);
            if (__G->f_logging)
                lf_log(ERROR, "STAGE_ENQUEUE_FAIL,%s\n",
                       __tl->path);
            return NULL;
        }
        __tl->stage_idx = 0;
    }
    return NULL;
}
/** @brief Build the full path for a file or directory entry.
    @param tl A pointer to the ThreadLocal struct containing the current directory context.
    @param file_name The name of the file or directory entry to append to the path.
    @details This function constructs the full path for a given file or directory
   entry by appending its name to the parent path stored in the ThreadLocal
   context. It updates the path length and ensures that the resulting path is
   null-terminated. This function is used during directory traversal to prepare
   paths for processing and output.
   */
void build_path(ThreadLocal *__tl, char *file_name) {
    __tl->path_len = __tl->parent_path_len;
    __tl->path[__tl->path_len++] = '/';
    __tl->d_name = file_name;
    memcpy(__tl->path + __tl->path_len, file_name, __tl->d_name_len);
    __tl->path_len += __tl->d_name_len;
    __tl->path[__tl->path_len] = '\0';
}
/** @brief Print the path of a file or directory entry based on filtering criteria.
    @param lf A pointer to the GlobalContext struct containing the search settings.
    @param tl A pointer to the ThreadLocal struct containing the current directory context.
    @return true if the file or directory entry was printed, false otherwise.
    @details This function checks various filtering criteria specified in the
   GlobalContext, such as type suppression, regex matching, user ownership,
   permissions, modification time, and file size. If the entry passes all
   filters, it increments the file count and appends the path to the output
   buffer for printing. The function returns true if the entry was printed and
   false if it was excluded based on any of the filtering criteria.
   */
int print_file(GlobalContext *__G, ThreadLocal *__tl) {
    while (1) {
        if (__G->suppress_types & lf_type_mask[__tl->effective_type])
            break;
        // Exclude non-matching files
        if (__G->flags & LF_REGEX) {
            int reti = regexec(&__G->compiled_re, __tl->path, 0, NULL, 0);
            if (reti == REG_NOMATCH)
                break;
            termination_status |= TS_MATCH;
        }
        // Exclude matching files
        if (__G->flags & LF_EXC_REGEX) {
            int reti = regexec(&__G->compiled_ere, __tl->path, 0, NULL, 0);
            if (reti == 0) {
                termination_status |= TS_MATCH;
                break;
            }
        }
        //  Exclude files not owned by specified user
        if (__G->flags & LF_USER) {
            if (!__tl->stat_cached && stat(__tl->path, __tl->cached_sb) == 0)
                __tl->stat_cached = true;
            if (!__tl->stat_cached || __tl->cached_sb->st_uid != __G->user_id)
                break;
        }
        if (__G->include_perms) {
            if (!__tl->stat_cached && stat(__tl->path, __tl->cached_sb) == 0)
                __tl->stat_cached = true;
            if (!__tl->stat_cached)
                break;
            if ((__G->include_perms & LF_IRUSR) && !(__tl->cached_sb->st_mode & S_IRUSR))
                break;
            else if ((__G->include_perms & LF_IWUSR) &&
                     !(__tl->cached_sb->st_mode & S_IWUSR))
                break;
            else if ((__G->include_perms & LF_IXUSR) &&
                     !(__tl->cached_sb->st_mode & S_IXUSR))
                break;
            else if ((__G->include_perms & LF_ISUID) &&
                     !(__tl->cached_sb->st_mode & S_ISUID))
                break;
            else if ((__G->include_perms & LF_ISGID) &&
                     !(__tl->cached_sb->st_mode & S_ISGID))
                break;
        }
        if (__G->before) { // Last file modification
            if (!__tl->stat_cached && stat(__tl->path, __tl->cached_sb) == 0)
                __tl->stat_cached = true;
            if (__tl->stat_cached && __tl->cached_sb->st_mtime > __G->before)
                break;
        }
        if (__G->after) { // Last file modification
            if (!__tl->stat_cached && stat(__tl->path, __tl->cached_sb) == 0)
                __tl->stat_cached = true;
            if (__tl->stat_cached && __tl->cached_sb->st_mtime < __G->after)
                break;
        }
        if (__G->file_size_min) {
            if (!__tl->stat_cached && stat(__tl->path, __tl->cached_sb) == 0)
                __tl->stat_cached = true;
            if (__tl->stat_cached && __tl->cached_sb->st_size < __G->file_size_min)
                break;
        }
        if (__G->only_errors)
            break;
        atomic_fetch_add(&__G->file_count, 1);
        char *path = __tl->path;
        size_t path_len = __tl->path_len;
        if (!__G->count_silently) {
            if (path_len > 2 && path[0] == '.' && path[1] == '/') {
                path += 2;
                path_len -= 2;
            }
            if (path_len > 0) {
                append_output_buffer(__G, __tl->output_buffer, path, path_len,
                                     __tl->effective_type == DT_DIR);
            }
            break;
        }
    }
    return true;
}
// -----------------------------------------------------------------------
// Logging
// -----------------------------------------------------------------------
/** iso8601_timestamp
    @brief Generate an ISO 8601 timestamp string
    @ingroup logging
    @param buf Buffer to store the timestamp string
    @param n Size of the buffer
    @param local If true, use local time; if false, use UTC
    @return Pointer to the buffer containing the timestamp string */
void iso8601_timestamp(GlobalContext *__G, LogCtx *log_ctx) {
    char buf[64];
    time_t tm = time(NULL);
    struct tm *tp =
        __G->timestamp_style == ISO8601_LOCAL ? localtime(&tm) : gmtime(&tm);
    if (__G->timestamp_style == ISO8601_LOCAL)
        strftime(buf, 64, _("%Y-%m-%dT%H:%M:%S%z"), tp);
    else
        strftime(buf, 64, _("%Y-%m-%dT%H:%M:%SZ"), tp);
    ssnprintf(log_ctx->time_buf, 64, "[%s]", buf);
    return;
}
static const char DIV100_TABLE[] = "0001020304050607080910111213141516171819"
                                   "2021222324252627282930313233343536373839"
                                   "4041424344454647484950515253545556575859"
                                   "6061626364656667686970717273747576777879"
                                   "8081828384858687888990919293949596979899";
#define WRITE_STR(p, str)      \
    do {                       \
        const char *s = (str); \
        while (*s)             \
            *p++ = *s++;       \
    } while (0)
/** @brief Write a 3-digit unsigned integer to a buffer.
    @ingroup logging
    @param p Pointer to the buffer where the integer will be written.
    @param val The unsigned integer value to write (0-999).
    @return Pointer to the next position in the buffer after writing the integer.
    @details This function writes a 3-digit unsigned integer to a provided
   buffer. It calculates the hundreds, tens, and units digits and uses a lookup
   table for efficient conversion of the last two digits. The function ensures
   that the value is correctly formatted as a 3-digit string, even if the value
   is less than 100, by padding with leading zeros as necessary.
   */
static inline char *write_uint3d(char *p, long val) {
    long hundreds = val / 100;
    long remainder = val % 100;
    *p++ = (char)('0' + hundreds);
    *p++ = DIV100_TABLE[remainder * 2];
    *p++ = DIV100_TABLE[remainder * 2 + 1];
    return p;
}
/** @brief Write a 2-digit unsigned integer to a buffer.
    @ingroup logging
    @param p Pointer to the buffer where the integer will be written.
    @param val The unsigned integer value to write (0-99).
    @return Pointer to the next position in the buffer after writing the integer.
    @details This function writes a 2-digit unsigned integer to a provided
   buffer. It uses a lookup table for efficient conversion of the value into its
   string representation. The function ensures that the value is correctly
   formatted as a 2-digit string, even if the value is less than 10, by padding
   with a leading zero as necessary.
   */
static inline char *write_uint2d(char *p, long val) {
    *p++ = DIV100_TABLE[val * 2];
    *p++ = DIV100_TABLE[val * 2 + 1];
    return p;
}
/** @brief Write an interval in seconds to a buffer, handling values greater
   than 99.
    @ingroup logging
    @param p Pointer to the buffer where the interval will be written.
    @param val The interval value in seconds to write.
    @return Pointer to the next position in the buffer after writing the
   interval.
    @details This function writes an interval value in seconds to a provided
   buffer. If the value is 99 or less, it uses a fast path to write two digits.
   For larger values, it constructs the string representation in reverse order
   and then copies it forward into the buffer. This approach ensures efficient
   handling of both small and large interval values while maintaining correct
   formatting.
   */
static inline char *write_interval_sec(char *p, long val) {
    if (val <= 99) {
        return write_uint2d(p, val);
    }
    // Fallback loop for large intervals: builds string backwards, then copies
    // forward
    char temp[24];
    int i = 0;
    while (val > 0) {
        temp[i++] = (char)('0' + (val % 10));
        val /= 10;
    }
    // Reverse copy into the main buffer
    while (i > 0) {
        *p++ = temp[--i];
    }
    return p;
}
/** @brief Initialize the timer for a thread, capturing the start time and
   setting up interval tracking.
    @ingroup logging
    @param lf A pointer to the GlobalContext struct containing the search settings.
    @param log_ctx A pointer to the LogCtx struct where timing information will
   be stored.
    @return void
    @details This function retrieves the current time using clock_gettime and
   initializes both the start time and the previous interval time in the
   provided LogCtx structure. It also sets the corresponding values in the GlobalContext
   structure for global timing reference. This setup is essential for accurate
   timestamping of log entries and interval calculations during program
   execution.
   */
void initialize_thread_timer(GlobalContext *__G, LogCtx *log_ctx) {
    struct timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);
    log_ctx->start_sec = __G->start_sec;
    log_ctx->start_nsec = __G->start_nsec;
    log_ctx->p_int_sec = tp.tv_sec;
    log_ctx->p_int_nsec = tp.tv_nsec;
}
/** @brief Initialize the timer for logging, capturing the start time and
   setting up interval tracking.
    @ingroup logging
    @param lf A pointer to the GlobalContext struct containing the search settings.
    @param log_ctx A pointer to the LogCtx struct where timing information will
   be stored.
    @return void
    @details This function retrieves the current time using clock_gettime and
   initializes both the start time and the previous interval time in the
   provided LogCtx structure. It also sets the corresponding values in the GlobalContext
   structure for global timing reference. This setup is essential for accurate
   timestamping of log entries and interval calculations during program
   execution.
   */
void initialize_timer(GlobalContext *__G, LogCtx *log_ctx) {
    struct timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);
    log_ctx->start_sec = tp.tv_sec;
    log_ctx->start_nsec = tp.tv_nsec;
    log_ctx->p_int_sec = tp.tv_sec;
    log_ctx->p_int_nsec = tp.tv_nsec;
    __G->start_sec = tp.tv_sec;
    __G->start_nsec = tp.tv_nsec;
    __G->p_int_sec = tp.tv_sec;
    __G->p_int_nsec = tp.tv_nsec;
}
/** coders_timestamp
    @brief Generate a timestamp string for logging, including both current time
   and interval since last log entry.
    @ingroup logging
    @param log_ctx A pointer to the LogCtx struct containing timing information.
    @return void
    @details This function calculates the current time since the start of the
   program and the interval since the last log entry. It formats these values
   into a timestamp string stored in log_ctx->time_buf. The timestamp includes
   seconds, milliseconds, microseconds, and nanoseconds, with color-coded
   formatting for better visibility in logs. If you are wondering about the
   seemingly complex output section, it is designed to mitigate the classical
   and unavoidable observation error.
   */
void coders_timestamp(LogCtx *log_ctx) {
    struct timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);
    long cur_sec = (tp.tv_sec - log_ctx->start_sec) % 60;
    long cur_nsec = (tp.tv_nsec - log_ctx->start_nsec);
    long cur_ms = (cur_nsec / 1000000);
    long cur_us = (cur_nsec / 1000) % 1000;
    long cur_ns = cur_nsec % 1000;
#ifdef DEBUG_INTERVAL_LOGGING
    fprintf(
        stderr,
        "DEBUG: thread: %2ld, start_sec: %010ld, start_nsec: %09ld, tp.tv_sec: "
        "%010ld, tp.tv_nsec: %09ld, cur: %02ld.%03ld_%03ld_%03ld\n",
        log_ctx->thread_id, log_ctx->start_sec, log_ctx->start_nsec, tp.tv_sec,
        tp.tv_nsec, cur_sec, cur_ms, cur_us, cur_ns);
#endif
    // Interval calculation: time since last log entry
    long int_sec = tp.tv_sec - log_ctx->p_int_sec;
    long int_nsec = tp.tv_nsec - log_ctx->p_int_nsec;
    if (int_nsec < 0) {
        int_sec -= 1;
        int_nsec += 1000000000L;
    }
    long int_ms = (int_nsec / 1000000);
    long int_us = (int_nsec / 1000) % 1000;
    long int_ns = int_nsec % 1000;
#ifdef DEBUG_INTERVAL_LOGGING
    fprintf(
        stderr,
        "DEBUG: thread: %2ld, p_int_sec: %010ld, p_int_nsec: %09ld, tp.tv_sec: "
        "%010ld, tp.tv_nsec: %09ld, int: %02ld.%03ld_%03ld_%03ld\n",
        log_ctx->thread_id, log_ctx->p_int_sec, log_ctx->p_int_nsec, tp.tv_sec,
        tp.tv_nsec, int_sec, int_ms, int_us, int_ns);
#endif
    log_ctx->p_int_sec = tp.tv_sec;
    log_ctx->p_int_nsec = tp.tv_nsec;
#ifdef DEBUG_INTERVAL_LOGGING
    fprintf(stderr, "DEBUG: thread: %2ld, p_int_sec: %010ld, p_int_nsec: %09ld\n",
            log_ctx->thread_id, log_ctx->p_int_sec, log_ctx->p_int_nsec);
#endif
    char *p = log_ctx->time_buf;
    // --- Block 1: Current Time Timestamp ---
    *p++ = '[';
    p = write_uint2d(p, cur_sec);
    *p++ = '.';
    WRITE_STR(p, sgr_c[YELLOW]);
    p = write_uint3d(p, cur_ms);
    *p++ = '_';
    p = write_uint3d(p, cur_us);
    *p++ = '_';
    p = write_uint3d(p, cur_ns);
    WRITE_STR(p, sgr_c[RESET]);
    *p++ = ']';
    // --- Block 2: Interval Time Timestamp ---
    *p++ = '[';
    p = write_interval_sec(p, int_sec); // Handles arbitrary size safely
    *p++ = '.';
    WRITE_STR(p, sgr_c[YELLOW]);
    p = write_uint3d(p, int_ms);
    *p++ = '_';
    p = write_uint3d(p, int_us);
    *p++ = '_';
    p = write_uint3d(p, int_ns);
    WRITE_STR(p, sgr_c[RESET]);
    *p++ = ']';
    *p = '\0';
}
/** log_open
    @brief Open the log file for writing
    @ingroup logging
    @return FILE pointer to the opened log file */
void log_open(GlobalContext *__G, LogCtx *log_ctx, size_t thread_id) {
    bool local = true;
    if (!__G->disp_level_mask && !__G->file_level_mask)
        return;
    if (__G->log_fd == -1) {
        char *user = getenv("USER");
        char tz_buf[16];
        char time_s[16];
        struct timespec tp;
        clock_gettime(CLOCK_REALTIME, &tp);
        log_ctx->start_sec = tp.tv_sec;
        log_ctx->start_nsec = tp.tv_nsec;
        log_ctx->p_int_sec = tp.tv_sec;
        log_ctx->p_int_nsec = tp.tv_nsec;
        __G->start_sec = tp.tv_sec;
        __G->start_nsec = tp.tv_nsec;
        __G->p_int_sec = tp.tv_sec;
        __G->p_int_nsec = tp.tv_nsec;
        struct tm *tm = local ? localtime(&tp.tv_sec) : gmtime(&tp.tv_sec);
        strftime(time_s, sizeof(time_s), "%Y%m%dT%H%M%S", tm);
        if (local) {
            strftime(tz_buf, sizeof(tz_buf), "%z", tm);
        } else {
            tz_buf[0] = 'Z';
            tz_buf[1] = '\0';
        }
        ssnprintf(__G->log_file_spec, MAXLEN - 1, "/tmp/%s_lf_%s%s.log", user,
                  time_s, tz_buf);
        __G->log_fd = open(__G->log_file_spec, O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (__G->log_fd == -1) {
            ssnprintf(em0, MAXLEN - 1, _("Failed to open log file: %s"),
                      __G->log_file_spec);
            ssnprintf(em1, MAXLEN - 1, _("Error: %s"), strerror(errno));
            ssnprintf(em2, MAXLEN - 1, "%s: line: %d", __FILE__, __LINE__ - 4);
            ssnprintf(em3, MAXLEN - 1, _("remedy: delete %s and try again"),
                      __G->log_file_spec);
            fprintf(stderr, "%s\n%s\n%s\n%s\n", em0, em1, em2, em3);
            exit(EXIT_FAILURE);
        }
    }
    char ttyname[MAXLEN];
    char cmenu_user[MAXLEN];
    char *p = getenv(_("USER"));
    strnz__cpy(cmenu_user, p, MAXLEN - 1);
    if (ttyname_r(STDERR_FILENO, ttyname, sizeof(ttyname)) == 0)
        if (__G->f_logging)
            lf_log(ALL, _("Lf_Log started by user %s, thread %lu, on terminal '%s'"),
                   cmenu_user, thread_id, ttyname);
    return;
}
/** @brief Write a log message to the log file and/or display
    @ingroup logging
    @param level The log level (e.g., INFO, ERROR)
    @param lf Pointer to the GlobalContext struct containing logging settings
    @param log_ctx Pointer to the LogCtx struct for the current thread
    @param file The source file name where the log is generated
    @param func The function name where the log is generated
    @param line The line number in the source file where the log is generated
    @param fmt The format string for the log message
    @param ... Additional arguments for the format string
    @details This function checks if the specified log level is enabled for file
   or display output. If enabled, it formats the log message with a timestamp,
   source information, and user-provided message. It handles opening the log
   file if necessary and manages log rotation based on size limits. The
   formatted message is written to the appropriate output destinations.
 */
void log_write(const ll_e level, GlobalContext *__G, LogCtx *log_ctx, const char *file,
               const char *func, const int line, const char *fmt, ...) {
    if (!(__G->file_level_mask & ll_t[level].bits) &&
        !(__G->disp_level_mask & ll_t[level].bits))
        return;
    va_list args;
    va_start(args, fmt);
    if ((__G->file_level_mask & ll_t[level].bits) && __G->log_fd == -1)
        log_open(__G, log_ctx, log_ctx->thread_id);
    struct stat st;
    fstat(__G->log_fd, &st);
    if (__G->log_fd == -1 ||
        ((fstat(__G->log_fd, &st) == 0) && (st.st_size >= MAX_LOG_SIZE))) {
        if (__G->log_fd != -1) {
            close(__G->log_fd);
            __G->log_fd = -1;
        }
        char old_archive[MAXLEN];
        strnz__cpy(old_archive, __G->log_file_spec, MAXLEN - 1);
        strnz__cat(old_archive, ".closed", MAXLEN - 1);
        rename(__G->log_file_spec, old_archive);
        log_open(__G, log_ctx, log_ctx->thread_id);
    }
    switch (__G->timestamp_style) {
    case ISO8601_ZULU:
    case ISO8601_LOCAL:
        iso8601_timestamp(__G, log_ctx);
        break;
    case CODER:
        coders_timestamp(log_ctx);
        break;
    case TIMESTAMP_STYLE_COUNT:
        break;
    }
    char log_record[MAX_LOG_REC_LEN];
    ssnprintf(log_record, sizeof(log_record), "%s[%s%-8s%s]<%s:%-12s:%5d> ",
              log_ctx->time_buf, ll_t[level].SGR, ll_t[level].name, ANSI_RESET,
              file, func, line);
    size_t len = strlen(log_record);
    vssnprintf(log_record + len, sizeof(log_record) - len, fmt, args);
    len = strlen(log_record);
    log_record[len++] = '\n';
    if (__G->file_level_mask & ll_t[level].bits) {
        if (write(__G->log_fd, log_record, len) == -1)
            perror("Failed to write atomic log record");
    }
    if (__G->disp_level_mask & ll_t[level].bits) {
        if (write(__G->err_fd, log_record, len) == -1)
            perror("Failed to write atomic error record");
        va_end(args);
        return;
    }
}
/** @brief Parse a local timestamp string into a time_t value
    @ingroup utility_functions
    @param s - the string containing the local timestamp in the format
   "YYYY-MM-DDTHH:MM:SS"
    @param out - pointer to a time_t variable where the parsed value will be
   stored
    @returns true if parsing was successful, false otherwise
    @details This function uses strptime to parse a local timestamp string into
   a struct tm, and then converts it to a time_t value using mktime. The caller
   is responsible for providing a valid buffer for the output time_t value.
 */
bool parse_local_timestamp(const char *s, time_t *out) {
    struct tm tmv;
    memset(&tmv, 0, sizeof tmv);
    tmv.tm_isdst = -1;
    if (strptime(s, "%Y-%m-%dT%H:%M:%S", &tmv) == NULL)
        return false;
    time_t t = mktime(&tmv);
    if (t == (time_t)-1)
        return false;
    *out = t;
    return true;
}
/** @brief Format a time_t value into a local timestamp string
    @ingroup utility_functions
    @param t - the time_t value to format
    @param buf - the buffer to store the formatted timestamp
    @param n - the size of the buffer
    @returns a pointer to the buffer containing the formatted timestamp
    @details This function converts a time_t value into a local timestamp string
   in the format "YYYY-MM-DDTHH:MM:SS". It uses localtime_r to convert the
   time_t value into a struct tm, and then formats it using strftime. The caller
   is responsible for ensuring that the buffer is large enough to hold the
   resulting string.
 */
char *format_local_timestamp(time_t t, char *buf, size_t n) {
    struct tm tmv;
    localtime_r(&t, &tmv);
    strftime(buf, n, "%Y-%m-%dT%H:%M:%S", &tmv);
    return buf;
}
/** @brief Expand a tilde (~) in a file path to the user's home directory
    @ingroup utility_functions
    @param str - the string containing the path to expand
    @param path_maxlen - the maximum length of the path buffer
    @returns true if expansion was successful, false otherwise
    @details This function checks if the input string starts with a tilde (~)
   and replaces it with the user's home directory obtained from the HOME
   environment variable. It ensures that the resulting path does not exceed the
   specified maximum length. The caller is responsible for ensuring that the
   input string has enough space to accommodate the expanded path.
 */
bool expand_tilde(char *str, uint path_maxlen) {
    if (str == nullptr || *str == '\0')
        return false;
    const char tgt[3] = "~/";
    char path[MAXLEN];
    char *e = getenv("HOME");
    strnz__cpy(path, e, MAXLEN - 1);
    strnz__cat(path, "/", MAXLEN - 1);
    char *tmp;
    tmp = rep_substring(str, tgt, path);
    strnz__cpy(str, tmp, path_maxlen - 1);
    free(tmp);
    return true;
}
/** @brief Replace all occurrences of a target substring in a string with a
   replacement substring
    @ingroup utility_functions
    @param org_s - the original string
    @param tgt_s - the target substring to be replaced
    @param rep_s - the replacement substring
    @returns a newly allocated string with replacements made, or nullptr on
   error
    @details This function searches for all occurrences of the target substring
   in the original string and replaces them with the replacement substring. It
   handles edge cases such as empty strings and overlapping substrings. The
   caller is responsible for freeing the returned string.
 */
char *rep_substring(const char *org_s, const char *tgt_s, const char *rep_s) {
    if (*org_s == '\0' || *tgt_s == '\0' || *rep_s == '\0')
        return nullptr;
    if (strstr(org_s, tgt_s) == nullptr)
        return strdup(org_s);
    if (strstr(rep_s, tgt_s) != nullptr)
        return nullptr;
    if (tgt_s == rep_s || tgt_s == org_s || rep_s == org_s)
        return strdup(org_s);
    if (strcmp(org_s, tgt_s) == 0)
        return strdup(rep_s);
    char *out_s, *ip, *tmp;
    uint tgt_l = strlen(tgt_s);
    uint rep_l = strlen(rep_s);
    uint head_l;
    uint n = 0;
    ip = (char *)org_s;
    while ((tmp = strstr(ip, tgt_s)) != nullptr) {
        n++;
        ip = tmp + tgt_l;
    }
    out_s = malloc(strlen(org_s) + (rep_l - tgt_l) * n + 1);
    if (!out_s) {
        return nullptr;
    }
    tmp = out_s;
    ip = (char *)org_s;
    while (n--) {
        char *p = strstr(ip, tgt_s);
        head_l = p - ip;
        strnz__cpy(tmp, ip, head_l);
        tmp += head_l;
        strnz__cpy(tmp, rep_s, MAXLEN - 1);
        tmp += rep_l;
        ip += head_l + tgt_l;
    }
    strnz__cpy(tmp, ip, MAXLEN - 1);
    return out_s;
}
/** @brief A safer vsnprintf that ensures null-termination and handles edge
   cases
    @ingroup utility_functions
    @param buf - the buffer to write to
    @param buf_size - the size of the buffer
    @param format - the format string
    @param args - the variable argument list for formatting
    @returns the number of characters written, excluding the null terminator
    @details This function wraps vsnprintf to provide a safer interface for
   formatted string output. It ensures that the output is always null-terminated
   and handles cases where the buffer is NULL or has zero size. It also returns
   the number of characters that would have been written if enough space had
   been available, allowing for proper buffer management.
 */
size_t vssnprintf(char *buf, size_t buf_size, const char *format,
                  va_list args) {
    /* Handle edge cases: an explicit NULL buffer or zero size */
    if (buf == NULL || buf_size == 0)
        return 0;
    int result = vsnprintf(buf, buf_size, format, args);
    if (result < 0) {
        buf[0] = '\0';
        return 0;
    }
    size_t bytes_written = (size_t)result;
    /* Check if the output was truncated due to buffer limits.
     * vsnprintf returns the total bytes *needed*, not bytes written. */
    if (bytes_written >= buf_size)
        return buf_size - 1; // Return the exact length of the truncated string
    return bytes_written;
}
/** @brief A safer snprintf that ensures null-termination and handles edge cases
    @ingroup utility_functions
    @param buf - the buffer to write to
    @param buf_size - the size of the buffer
    @param format - the format string
    @param ... - additional arguments for formatting
    @returns the number of characters written, excluding the null terminator
    @details This function wraps vsnprintf to provide a safer interface for
   formatted string output. It ensures that the output is always null-terminated
   and handles cases where the buffer is NULL or has zero size. It also returns
   the number of characters that would have been written if enough space had
   been available, allowing for proper buffer management.
 */
ATTR_FORMAT(printf, 3, 4)
size_t ssnprintf(char *buf, size_t buf_size, const char *format, ...) {
    if (buf == nullptr || format == nullptr || buf_size == 0)
        return 0;
    va_list args;
    size_t result;
    va_start(args, format);
    result = vssnprintf(buf, buf_size, format, args);
    va_end(args);
    return result;
}
/** @brief Check if a given path is a symbolic link that points to a directory
    @ingroup utility_functions
    @param path - the path to check
    @returns true if the path is a symbolic link to a directory, false otherwise
    @details This function uses lstat to determine if the specified path is a
   symbolic link. If it is, it then uses stat to check if the target of the
   symbolic link is a directory. This is useful for file system traversal and
   validation, especially when handling symbolic links that may point to
   directories.
 */
bool is_symlink_to_dir(const char *path) {
    struct stat link_stat;
    struct stat target_stat;
    if (lstat(path, &link_stat) == 0)
        if (S_ISLNK(link_stat.st_mode))
            if (stat(path, &target_stat) == 0)
                if (S_ISDIR(target_stat.st_mode))
                    return true; // symbolic link to a directory
    return false;
}
/** @brief Convert a string to an unsigned long with optional size suffixes
    @ingroup utility_functions
    @param str - the string to convert
    @returns the converted unsigned long value, or 0 if conversion fails
    @details This function attempts to convert the input string str to an
   unsigned long integer. It supports optional size suffixes: 'k' for
   kilobytes, 'm' for megabytes, and 'g' for gigabytes. The conversion is done
   using strtoull, and if the conversion fails (e.g., due to invalid input),
   it returns 0. This is useful for parsing user input or configuration values
   that may include size specifications.
 */
unsigned long a_to_ul(const char *str) {
    char *endptr;
    unsigned long value = (unsigned long)strtoull(str, &endptr, 10);
    if (endptr == str)
        return 0;
    switch (tolower(*endptr)) {
    case 'g':
        return value * 1024ULL * 1024ULL * 1024ULL;
    case 'm':
        return value * 1024ULL * 1024ULL;
    case 'k':
        return value * 1024ULL;
    default:
        return value;
    }
}
/** @brief Convert a string to an integer with error checking
    @ingroup utility_functions
    @param s - the string to convert
    @param a_toi_error - pointer to a boolean that will be set to true if an
   error occurs
    @returns the converted integer, or -1 if an error occurs
    @details This function attempts to convert the input string s to an integer.
   If the conversion fails (e.g., due to invalid input or overflow), it sets
   the boolean pointed to by a_toi_error to true and returns -1. Otherwise, it
   returns the converted integer and sets a_toi_error to false. This is useful
   for safely parsing user input or configuration values where invalid data may
   be encountered.
 */
int a_toi(char *s, bool *a_toi_error) {
    if (s == nullptr || *s == '\0') {
        *a_toi_error = true;
        return -1;
    }
    int rc = -1;
    *a_toi_error = false;
    errno = 0;
    if (*s != 0)
        rc = (uint)strtol(s, nullptr, 10);
    if (rc < 0 || errno) {
        rc = -1;
        *a_toi_error = true;
    }
    return rc;
}
/** @brief safer alternative to strncpy
    @ingroup utility_functions
    @param d - destination string
    @param s - source string
    @param max_len - maximum length to copy
    @returns length of resulting string
    @details Copy string s to d, ensuring that the total length of d does not
   exceed max_len, and that the resulting string is null-terminated. It also
   treats newline and carriage return characters as string terminators,
   preventing them from being included in the result. This is particularly
   useful when copying user input or file data, where embedded newlines could
   cause issues.
 */
size_t strnz__cpy(char *d, const char *s, size_t max_len) {
    if (d == nullptr || s == nullptr)
        return 0;
    char *e;
    size_t len = 0;
    if (max_len == 0) {
        if (max_len > 0)
            *d = '\0';
        return 0;
    }
    e = d + max_len;
    while (*s != '\0' && *s != '\n' && *s != '\r' && d < e) {
        *d++ = *s++;
        len++;
    }
    *d = '\0';
    return len;
}
/** @brief safer alternative to strncat
    @ingroup utility_functions
    @param d - destination string
    @param s - source string
    @param max_len - maximum length to copy
    @returns length of resulting string
    @details Append string s to d, ensuring that the total length of d does not
   exceed max_len, and that the resulting string is null-terminated. It also
   treats newline and carriage return characters as string terminators,
   preventing them from being included in the result. This is particularly
   useful when concatenating user input or file data, where embedded newlines
   could cause issues.
 */
size_t strnz__cat(char *d, const char *s, size_t max_len) {
    if (d == nullptr || s == nullptr)
        return 0;
    if (max_len == 0)
        return 0;
    char *e;
    size_t len = 0;
    e = d + max_len;
    while (*d != '\0' && *d != '\n' && *d != '\r' && d < e) {
        d++;
        len++;
    }
    while (*s != '\0' && *s != '\n' && *s != '\r' && d < e) {
        *d++ = *s++;
        len++;
    }
    *d = '\0';
    return len;
}
/** @brief Check if a given path is a directory
    @ingroup utility_functions
    @param path - the file system path to check
    @returns true if the path is a directory, false otherwise
    @details This function uses the stat system call to retrieve information
   about the file at the specified path. It then checks the file mode to
   determine if it is a directory. This is useful for filtering or validating
   paths in file system operations.
 */
bool is_directory(const char *path) {
    struct stat statbuf;
    if (stat(path, &statbuf) == 0)
        if (S_ISDIR(statbuf.st_mode))
            return true;
    return false;
}
/** @brief Check if a given path is a regular file
    @ingroup utility_functions
    @param path - the file system path to check
    @returns true if the path is a regular file, false otherwise
    @details This function uses the stat system call to retrieve information
   about the file at the specified path. It then checks the file mode to
   determine if it is a regular file. This is useful for filtering or validating
   paths in file system operations.
 */
bool is_valid_regex(const char *pattern) {
    regex_t regex;
    uint ret = regcomp(&regex, pattern, REG_EXTENDED);
    regfree(&regex);
    if (ret == 0)
        return true;
    return false;
}
