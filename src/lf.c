/** @file lf.c
    @brief list files based on matching criteria
    @author Bill Waller
    Copyright (c) 2026
    MIT License
    billxwaller@gmail.com
    @date 2026-09-22
 */
#define _GNU_SOURCE
#include "cm.h"
#include <argp.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <linux/limits.h>
#include <pthread.h>
#include <pwd.h>
#include <regex.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define QUEUE_CAPACITY 16384
#define QUEUE_MASK (QUEUE_CAPACITY - 1)
#define MAX_PATH_LEN _POSIX_PATH_MAX
#define MAX_DEPTH 64
// #define DIR_BUF_SIZE 131072
#define DIR_BUF_SIZE 262144
// #define DIR_BUF_SIZE 524288
#define CACHE_LINE_SIZE 64

// 32 Million directories max limit. Cost: 768MB Virtual Memory, 0MB RAM initially.
#define ARENA_MAX_NODES (32 * 1024 * 1024)

typedef struct CycleNode CycleNode;
struct CycleNode {
    dev_t dev;
    ino_t ino;
    CycleNode *parent;
};

typedef struct {
    CycleNode *nodes;
    _Atomic size_t index;
    size_t capacity;
} CycleArena;

static CycleArena g_cycle_arena = {NULL, 0, 0};

typedef struct QueuePayload QueuePayload;
struct QueuePayload {
    size_t path_len;
    uint16_t depth;
    char path[MAX_PATH_LEN];
    CycleNode *ctx;
};

typedef struct {
    alignas(CACHE_LINE_SIZE) _Atomic size_t enqueue_pos;
    alignas(CACHE_LINE_SIZE) _Atomic size_t dequeue_pos;
    alignas(CACHE_LINE_SIZE) _Atomic atomic_int active_tasks;
    alignas(CACHE_LINE_SIZE) _Atomic atomic_int shut_down;
    alignas(CACHE_LINE_SIZE) _Atomic size_t sequence[QUEUE_CAPACITY];
    QueuePayload nodes[QUEUE_CAPACITY];
} MPMCQueue;

typedef enum {
    TS_SUCCESS = 0,
    TS_ERROR = 1,
    TS_MATCH = 2,
    TS_MATCH_PLUS_ERROR = 3,
} TerminationStatus;

struct linux_dirent64 {
    unsigned long long d_ino; /* 64-bit inode number */
    long long d_off;          /* 64-bit offset to next structure */
    unsigned short d_reclen;  /* Size of this dirent */
    unsigned char d_type;     /* File type */
    char d_name[];            /* Filename (null-terminated) */
};

#define print_file_type(mask, lf_type, dt_type, name)           \
    {                                                           \
        fprintf(stderr, "%c %08b (%3d) %08b (%2d) %s\n",        \
                (mask & lf_type) ? '*' : ' ', lf_type, lf_type, \
                dt_type, dt_type, name);                        \
    }

struct tm tm_info;
const char *argp_program_version = CM_VERSION;
const char *argp_program_bug_address = "billxwaller@gmail.com";
const char doc[] = _("lf list files\nIf specified, DIRECTORY is the top-level\n directory to search. REGULAR_EXPRESSION is a properly\nformatted regular expression for which matching files\nwill be listed.");
bool is_hidden(const char *);
bool is_dirsys(const char *);
static char args_doc[] = "[DIRECTORY] [REGULAR_EXPRESSION]";

TerminationStatus termination_status;
typedef struct {
    MPMCQueue *q;
    pthread_t *threads;
    unsigned int nthreads;
    atomic_size_t file_count;
    atomic_size_t error_count;
    pthread_mutex_t output_mutex;
    uintmax_t user_id;
    off_t file_size_min;
    FILE *err_fd;
    char error_file_spec[MAX_PATH_LEN];
    bool error_file_open;
    long flags;
    time_t after;
    time_t before;
    int max_depth;
    int reg_flags;
    char *base_path;
    char *re;
    char *ere;
    char *user_name;
    char *sbuffer;
    regex_t compiled_re;
    regex_t compiled_ere;
    unsigned char include_perms;
    unsigned char include_types;
    unsigned char suppress_types;
    bool report_error_count;
    bool ignore_case;
    bool sort;
    bool sort_reverse;
    bool include_hidden;
    bool hidden_only;
    bool follow_links;
    bool debug;
    bool report_config;
    bool report_info;
    bool report_warnings;
    bool report_errors;
    bool report_badlinks;
    bool report_trace;
    bool report_all;
    bool count;
    bool count_silently;
    bool only_errors;
} LfContext;

typedef struct {
    char data[256 * MAX_PATH_LEN];
    size_t len;
} OutputBuffer;

#define DT_LNK_DIR 14
unsigned char const lf_mask[15] = {
    0, 0b00000001, 0b00000010, 0, 0b00000100, 0, 0b00001000, 0, 0b00010000, 0,
    0b00100000, 0, 0b01000000, 0, 0b10000000};

int lfargc;
char *lfargs[3];
char *exec;
char *file_types_p;
char *perms_p;
char *debug_p;
void debug_out(LfContext *lf, int, char **);
bool init_lf(LfContext *lf, int, char **);
int sort_lf_output(LfContext *lf, int, char **);
MPMCQueue *mpmc_queue_init();
bool mpmc_enqueue(LfContext *, const QueuePayload *child_node);
bool mpmc_dequeue(LfContext *lf, QueuePayload *output_node);
void *worker(void *arg);
void *finder(LfContext *lf, QueuePayload *current_node, QueuePayload *child_node, OutputBuffer *output, char *dir_buf, char *full_path);
int scan_file(const char *file_spec, const size_t *path_len, LfContext *lf, const unsigned char, struct stat *, bool stat_cached, OutputBuffer *);
bool build_full_path(char *, size_t, const char *, const char *, size_t *);
void flush_output_buffer(LfContext *, OutputBuffer *);
bool append_output_buffer(LfContext *, OutputBuffer *, const char *, size_t,
                          bool);
int err_out(LfContext *lf, const char *format, ...);
void cycle_arena_init(void);
void cycle_arena_destroy(void);
static CycleNode *cycle_arena_alloc(dev_t dev, ino_t ino, CycleNode *parent);
bool is_link_cycle(dev_t dev, ino_t ino, CycleNode *parent);
// ---------------------------------------------------------------

static struct argp_option options[] = {
    {_("after"), 'a', _("time"), 0, _("Last Modified after YYYY-MM-DDTHH:MM:SS"), 0},
    {_("before"), 'b', _("time"), 0, _("Last Modified before YYYY-MM-DDTHH:MM:SS"), 0},
    {_("max_depth"), 'd', _("number"), 0, _("Depth into directory tree"), 0},
    {_("error_file_spec"), 'E', _("file_spec"), 0, _("Error message output file"), 0},
    {_("ere"), 'e', _("regex"), 0, _("Exclude regular expression"), 0},
    {_("ignore_case"), 'i', 0, 0, _("Search ignore case"), 0},
    {_("include_perms"), 'p', _("sgrwx"), 0,
     _("x-execute, w-write, r-read, s-setuid, g-setgid"), 0},
    {_("re"), 'r', _("regex"), 0, _("Regular expression to search for"), 0},
    {_("include_types"), 't', _("pcdbflsu"), 0,
     _("p-pipe, c-character_dev, d-directory, b-block_dev, f-regular_file, l-link, s-socket, u-unknown"),
     0},
    {_("file_size_min"), 's', _("size"), 0,
     _("No Suffix-bytes, K-kilobytes, M-Megabytes, or G-Gigabytes"), 0},
    {_("user"), 'u', _("user name"), 0, _("User Name of file owner "), 0},
    {_("debug"), 'D', "123456789", 0,
     _("1-config, 2-info, 3-warnings, 4-errors, 5-badlinks, 6-trace, 7-all, 8-only_errors, 9-report_error_count"),
     0},
    {_("include_hidden"), 'H', "o", OPTION_ARG_OPTIONAL, _("Include hidden files (o=hidden only)"), 0},
    {_("follow_links"), 'L', 0, 0, _("Follow symbolic links"), 0},
    {_("sort_reverse"), 'R', 0, 0, _("Sort in Reverse order"), 0},
    {_("sort"), 'S', 0, 0, _("Sort in Ascending order"), 0},
    {_("nthreads"), 'T', "threads", 0, _("Number of nthreads"), 0},
    {_("count"), 'c', "s", OPTION_ARG_OPTIONAL, _("Count (s only report count)"), 0},
    {0}};

/** @brief Parse a single option.  */
static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    LfContext *lf = state->input;
    int i = 0;
    bool a_toi_error = false;

    switch (key) {
    case 'a':
        parse_local_timestamp(arg, &lf->after);
        if (lf->after && lf->before && lf->before < lf->after) {
            fprintf(stderr, _("-b time must be greater than -a time.\n"));
            lf->after = 0;
        }
        break;
    case 'b':
        parse_local_timestamp(arg, &lf->before);
        if (lf->after && lf->before && lf->before < lf->after) {
            fprintf(stderr, _("-b time must be greater than -a time.\n"));
            lf->before = 0;
        }
        break;
    case 'c':
        lf->count = true;
        if (arg && arg[0] != '\0') {
            if (strcmp(arg, "s") == 0)
                lf->count_silently |= true;
        }
        break;
    case 'd':
        if (arg && arg[0] != '\0')
            lf->max_depth = a_toi(arg, &a_toi_error);
        break;
    case 'D':
        lf->debug = true;
        if (arg && arg[0] != '\0') {
            debug_p = strdup(arg);
            i = 0;
            lf->report_errors = true;
            while (debug_p[i]) {
                switch (debug_p[i]) {
                case '1': // CONFIG
                    lf->report_config = true;
                    lf->only_errors = true;
                    break;
                case '2': // INFO
                    lf->report_info = true;
                    break;
                case '3': // WARNINGS
                    lf->report_warnings = true;
                    break;
                case '4': // ERRORS
                    lf->report_errors = true;
                    break;
                case '5': // BADLINKS
                    lf->report_badlinks = true;
                    lf->report_errors = true;
                    break;
                case '6': // TRACE
                    lf->report_trace = true;
                    lf->report_badlinks = true;
                    lf->report_errors = true;
                    break;
                case '7': // ALL
                    lf->report_config = true;
                    lf->report_info = true;
                    lf->report_warnings = true;
                    lf->report_errors = true;
                    lf->report_badlinks = true;
                    break;
                case '8': // ONLY ERRORS
                    lf->only_errors = true;
                    lf->report_errors = true;
                    break;
                case '9': // ERROR COUNT
                    lf->report_error_count = true;
                    break;
                default:
                    break;
                }
                i++;
            }
            free(debug_p);
        }
        break;
    case 'e':
        lf->ere = strdup(arg);
        lf->flags |= LF_EXC_REGEX;
        break;
    case 'E':
        strncpy(lf->error_file_spec, arg, MAX_PATH_LEN - 1);
        break;
    case 'H':
        lf->include_hidden = true; // Include hidden files
        if (arg && arg[0] == 'o') {
            lf->hidden_only = true;
            lf->include_hidden = true;
        }
        lf->flags &= ~(LF_HIDE); // Turn hide flag off
                                 // LF_HIDE = 0 - include hidden files,
                                 // LF_HIDE = 1 - suppress hidden files
        break;
    case 'i':
        lf->ignore_case = true;
        break;
    case 'L':
        lf->follow_links = true; // follow symbolic links
        break;
    case 'p':
        perms_p = arg;
        while (perms_p[i]) {
            switch (perms_p[i++]) {
            case 'g':
                lf->include_perms |= LF_ISGID;
                break;
            case 'r':
                lf->include_perms |= LF_IRUSR;
                break;
            case 's':
                lf->include_perms |= LF_ISUID;
                break;
            case 'w':
                lf->include_perms |= LF_IWUSR;
                break;
            case 'x':
                lf->include_perms |= LF_IXUSR;
                break;
            default:
                break;
            }
        }
        break;
    case 'R':
        lf->sort_reverse = true;
        break;
    case 'r':
        lf->re = strdup(arg);
        lf->flags |= LF_REGEX;
        break;
    case 'S':
        lf->sort = true;
        break;
    case 's':
        lf->file_size_min = (intmax_t)(a_to_ul(arg));
        break;
    case 'T':
        if (arg && arg[0] != '\0')
            lf->nthreads = a_toi(arg, &a_toi_error);
        break;
    case 't':
        file_types_p = arg;
        while (file_types_p[i]) {
            switch (file_types_p[i++]) {
            case 'b':
                lf->include_types |= LF_BLK;
                break;
            case 'c':
                lf->include_types |= LF_CHR;
                break;
            case 'd':
                lf->include_types |= LF_DIR;
                break;
            case 'p':
                lf->include_types |= LF_FIFO;
                break;
            case 'l':
                lf->include_types |= LF_LNK;
                break;
            case 'f': // for regular files, 'f' is more intuitive than 'r'
            case 'r': // regular files are the most common type, so 'r' is also
                      // accepted
                lf->include_types |= LF_REG;
                break;
            case 's':
                lf->include_types |= LF_SOCK;
                break;
            case 'u':
                lf->include_types |= LF_UNKNOWN;
                break;
            default:
                break;
            }
        }
        break;
    case 'u':
        lf->user_name = strdup(arg);
        struct passwd *pwd = getpwnam(arg);
        if (pwd) {
            lf->user_id = (uintmax_t)pwd->pw_uid;
            lf->flags |= LF_USER;
        } else {
            fprintf(stderr, _("User '%s' not found.\n"), arg);
            exit(EXIT_FAILURE);
        }
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
    LfContext *lf = (LfContext *)calloc(1, sizeof(LfContext));

    lf->count = 0;
    termination_status = TS_ERROR;
    lf->error_count = 0;
    lf->report_error_count = false;
    lf->nthreads = 0;
    lf->ignore_case = false;
    lf->sort = false;
    lf->sort_reverse = false;
    lf->include_hidden = false; // By default, hidden files are suppressed.
    lf->hidden_only = false;
    lf->flags |= LF_HIDE; // Turn hide flag on
    lf->follow_links = false;
    lf->count = false;
    lf->count_silently = false;
    lf->report_badlinks = false;
    lf->sbuffer = _("4G");
    lf->error_file_spec[0] = '\0';
    lf->error_file_open = false;

    char tmp_str[MAX_PATH_LEN];
    argp_parse(&argp, argc, argv, 0, 0, lf);
    if (lfargc > 0) {
        strnz__cpy(tmp_str, lfargs[0], MAXLEN - 1);
        expand_tilde(tmp_str, MAXLEN - 1);
        if (is_directory(tmp_str) || is_symlink_to_dir(tmp_str)) {
            lf->base_path = strdup(tmp_str);
        } else if (is_valid_regex(lfargs[0])) {
            lf->re = strdup(lfargs[0]);
            lf->flags |= LF_REGEX;
        } else {
            fprintf(
                stderr,
                _("lf: arg1: '%s' is neither a directory nor a valid regex.\n"),
                lfargs[0]);
            exit(EXIT_FAILURE);
        }
    }
    if (lfargc > 1) {
        if (!lf->base_path || lf->base_path[0] == '\0') {
            strnz__cpy(tmp_str, lfargs[1], MAXLEN - 1);
            expand_tilde(tmp_str, MAXLEN - 1);
            if (is_directory(tmp_str) || is_symlink_to_dir(tmp_str))
                lf->base_path = strdup(tmp_str);
        }
        if ((!(lf->flags & LF_REGEX)) && is_valid_regex(lfargs[1])) {
            lf->re = strdup(lfargs[1]);
            lf->flags |= LF_REGEX;
        } else {
            fprintf(stderr,
                    _("lf: '%s' is neither a directory nor a valid regular expression.\n"),
                    lfargs[1]);
            exit(EXIT_FAILURE);
        }
    }
    if (lf->base_path == nullptr || lf->base_path[0] == '\0')
        lf->base_path = strdup(".");
    unsigned int nprocs = get_nprocs();
    if (nprocs == 0)
        nprocs = 1;

    if (lf->nthreads == 0)
        lf->nthreads = (nprocs > 1) ? (nprocs - 1) : 1;
    else {
        if (lf->nthreads < 1)
            lf->nthreads = 1;
        if (lf->nthreads > nprocs)
            lf->nthreads = nprocs;
    }
    termination_status = TS_SUCCESS;
    if (!lf->sort) {
        init_lf(lf, argc, argv);
    } else
        termination_status = sort_lf_output(lf, argc, argv);

    if (lf->count) {
        size_t count = atomic_load(&lf->file_count);
        fprintf(stderr, _("Files: %zu\n"), count);
    }
    atomic_load(&lf->error_count);
    if (lf->report_error_count && lf->error_count > 0) {
        fprintf(stderr, _("Errors: %zu\n"), lf->error_count);
        termination_status |= TS_ERROR;
    }
    free(lf->q);
    free(lf->base_path);
    free(lf->user_name);
    free(lf->re);
    free(lf->ere);
    free(lf->threads);
    free(lf);
    exit(termination_status);
}
/** @brief Sort the output of the lf command using the system's sort utility.
    @param lf A pointer to the LfContext struct containing the search settings.
    @param argc The number of command-line arguments.
    @param argv An array of command-line argument strings.
    @details This function sets up a pipeline to sort the output of the lf command. It creates a pipe, forks a child process to execute the sort command, and redirects the standard input and output streams accordingly. The sort command is executed with optional flags for parallel processing and buffer size, as well as a reverse sort option if specified in the LfContext. The function waits for the child process to complete before returning.
   */
int sort_lf_output(LfContext *lf, int argc, char **argv) {
    // char tmp_str[MAXLEN];
    char *eargv[MAXARGS];
    int eargc = 0;
    eargv[eargc++] = strdup("sort");
    // snprintf(tmp_str, MAXLEN - 1, "--parallel=%d", lf->nthreads);
    // eargv[eargc++] = strdup(tmp_str);
    //  snprintf(tmp_str, MAXLEN - 1, "--buffer-size=%s", lf->sbuffer);
    //  eargv[eargc++] = strdup(tmp_str);
    if (lf->sort_reverse)
        eargv[eargc++] = strdup("-r");
    eargv[eargc] = nullptr;

    setenv("LC_ALL", "C", 1); // Set locale to C for consistent sorting
    int wstatus;

    int fds[2];
    pipe(fds); // Create the pipes

    pid_t pid1 = fork();
    if (pid1 == 0) { // Child process
        // Child's STDIN will be redirected to the read end of the pipe, so that the sort process will read the output of the finder from the pipe.
        // Close the write end of the pipe in the child
        close(fds[1]);
        dup2(fds[0], STDIN_FILENO); // Clone child's read pipe to STDIN_FILENO
        close(fds[0]);              // Close the original read end of the pipe
        execvp(eargv[0], eargv);    // Execute the sort command
        fprintf(stderr, _("Failed to execute sort: %s\n"), strerror(errno));
        return TS_ERROR;
    }
    // fclose(stdout);
    dup2(fds[1], STDOUT_FILENO);         // Clone write pipe to STDOUT_FILENO
    stdout = fdopen(STDOUT_FILENO, "w"); // Reopen STDOUT as a stream
    setvbuf(stdout, NULL, _IOLBF, 0);    //  line buffering
    init_lf(lf, argc, argv);
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
    @param lf A pointer to the LfContext struct containing the debug settings.
    @param argc The number of command-line arguments.
    @param argv An array of command-line argument strings.
    @details This function outputs debug information to stderr if debugging is enabled in the LfContext. It includes a timestamp, user information, IP addresses, command-line arguments, and various configuration settings. The output is formatted for readability and includes details about file types, permissions, regex patterns, and other relevant settings.
   */
void debug_out(LfContext *lf, int argc, char **argv) {
    char user_str[100];
    char ip_str[MAXLEN];
    int len = 0;
    int i;
    bool addspace_before = false;
    if (lf->debug && (lf->report_config || lf->report_info)) {
        fprintf(stderr, "%s,%s,%s,", get_local_timestamp(), get_user_str(user_str, 100), get_ip_addresses(ip_str, MAXLEN));
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
        fprintf(stderr, "\n\n");
        fprintf(stderr, "%s\n\n", CM_VERSION);
        fprintf(stderr, _("lf debug      %s\n"),
                lf->debug ? _("true") : _("     false"));
        fprintf(stderr, _("  1-config      %s\n"),
                lf->report_config ? _("true") : _("|    false"));
        fprintf(stderr, _("  2-info        %s\n"),
                lf->report_info ? _("true") : _("|    false"));
        fprintf(stderr, _("  3-warnings    %s\n"),
                lf->report_warnings ? _("true") : _("|    false"));
        fprintf(stderr, _("  4-errors      %s\n"),
                lf->report_errors ? _("true") : _("|    false"));
        fprintf(stderr, _("  5-badlinks    %s\n"),
                lf->report_errors ? _("true") : _("|    false"));
        fprintf(stderr, _("  6-trace       %s\n"),
                lf->report_errors ? _("true") : _("|    false"));
        fprintf(stderr, _("  8-only_errors %s\n"),
                lf->report_errors ? _("true") : _("|    false"));
        fprintf(stderr, "\n");
        fprintf(stderr, _("Count files: %s\n"),
                lf->report_errors ? _("true") : _("|    false"));
        fprintf(stderr, _("Count only: %s\n"),
                lf->report_errors ? _("true") : _("|    false"));
        fprintf(stderr, "\n");
        fprintf(stderr, _("Search directory: %s\n\n"), lf->base_path);
        if (lf->max_depth == 0)
            fprintf(stderr, _("Max depth 0 (unlimited)\n\n"));
        else
            fprintf(stderr, _("Max depth %d\n\n"), lf->max_depth);
        fprintf(stderr, _("Using %d threads\n\n"), lf->nthreads);
        fprintf(stderr, _("File types preceeded by an asterisk (\"*\") will be included:\n\n"));
        fprintf(stderr, _("  LF type        DT type\n"));
        print_file_type(lf->include_types, LF_FIFO, DT_FIFO, _("FIFO    p-named pipe"));
        print_file_type(lf->include_types, LF_CHR, DT_CHR, _("CHR     c-character device"));
        print_file_type(lf->include_types, LF_DIR, DT_DIR, _("DIR     d-directory"));
        print_file_type(lf->include_types, LF_BLK, DT_BLK, _("BLK     b-block device"));
        print_file_type(lf->include_types, LF_REG, DT_REG, _("REG     f-regular file"));
        print_file_type(lf->include_types, LF_LNK, DT_LNK, _("LINK    l-symbolic link"));
        print_file_type(lf->include_types, LF_SOCK, DT_SOCK, _("SOCK    s-socket"));
        print_file_type(lf->include_types, LF_UNKNOWN, DT_UNKNOWN, _("UNKNOWN u-unknown"));
        fprintf(stderr, "\n");
        fprintf(stderr, "f->include_types  = %08b\n", lf->include_types);
        fprintf(stderr, "f->suppress_types = %08b\n", lf->suppress_types);
        fprintf(stderr, "\n");
        if (lf->flags & LF_USER)
            fprintf(stderr, _("User: %s (%ju)\n"), lf->user_name, lf->user_id);
        if (lf->include_perms) {
            if (lf->include_perms & LF_IXUSR)
                fprintf(stderr, _("    %08b Execute\n"), LF_IXUSR);
            if (lf->include_perms & LF_IWUSR)
                fprintf(stderr, _("    %08b Write\n"), LF_IWUSR);
            if (lf->include_perms & LF_IRUSR)
                fprintf(stderr, _("    %08b Read\n"), LF_IRUSR);
            if (lf->include_perms & LF_ISUID)
                fprintf(stderr, _("    %08b SETUID\n"), LF_ISUID);
            if (lf->include_perms & LF_ISGID)
                fprintf(stderr, _("    %08b SETGID\n"), LF_ISGID);
        }
        fprintf(stderr, "\n");
        if (lf->flags & LF_REGEX)
            fprintf(stderr, _("Include regex: %s\n\n"), lf->re);
        if (lf->flags & LF_EXC_REGEX)
            fprintf(stderr, _("Exclude regex: %s\n\n"), lf->ere);

        if (lf->after) {
            char buf[32];
            format_local_timestamp(lf->after, buf, sizeof(buf));
            fprintf(stderr, _("Modified after: %s\n\n"), buf);
        }

        if (lf->before) {
            char buf[32];
            format_local_timestamp(lf->before, buf, sizeof(buf));
            fprintf(stderr, _("Modified before: %s\n\n"), buf);
        }

        if (lf->file_size_min) {
            const char *units[] = {"b", "Kb", "Mb", "Gb", "Tb", "Pb", "Eb"};
            off_t size = lf->file_size_min;
            i = 0;
            while (size >= 1024 && i < 6) {
                size /= 1024;
                i++;
            }
            char buffer[32];
            ssnprintf(buffer, 32, "%ld %s", size, units[i]);
            fprintf(stderr, _("Minimum file size: %s\n\n"), buffer);
        }
        if (lf->max_depth)
            fprintf(stderr, _("Max depth: %d\n\n"), lf->max_depth);
        if (lf->ignore_case)
            fprintf(stderr, _("Ignore case in regex matching.\n\n"));
        if (lf->include_hidden)
            fprintf(stderr, _("Include hidden files.\n\n"));
        if (lf->follow_links)
            fprintf(stderr, _("Follow symbolic links.\n\n"));
        if (lf->sort)
            fprintf(stderr, _("Sort output in ascending order.\n\n"));
        if (lf->sort_reverse)
            fprintf(stderr, _("Sort output in reverse order.\n\n"));
    }
    return;
}
// ----------------------------------------------------------------------
// INIT_FIND
// ----------------------------------------------------------------------
/** @brief Initialize the file search process.
    @param lf A pointer to the LfContext struct containing the search settings.
    @param argc The number of command-line arguments.
    @param argv An array of command-line argument strings.
    @return true if initialization is successful, false otherwise.
    @details This function initializes the file search process based on the
    provided settings in the LfContext struct. It sets up file type inclusion
    and suppression, compiles regular expressions if specified, and creates a
    multi-producer, multi-consumer queue for managing directory traversal tasks.
    It also initializes threads for concurrent processing of directory entries.
   */
bool init_lf(LfContext *lf, int argc, char **argv) {
    /** suppress file types that aren't included */
    if (!lf->include_types)
        lf->include_types = 0xff;
    if (lf->include_types)
        lf->suppress_types = lf->include_types ^ 0xff;
    // LF_HIDE = 0 - include hidden files,
    // LF_HIDE = 1 - suppress hidden files
    lf->include_hidden = !(lf->flags & LF_HIDE);
    int reti = 0;
    lf->reg_flags = REG_EXTENDED;
    if (lf->ignore_case)
        lf->reg_flags |= REG_ICASE;
    if (lf->flags & LF_REGEX) {
        reti = regcomp(&lf->compiled_re, lf->re, lf->reg_flags);
        if (reti) {
            fprintf(stderr, _("lf: '%s' Invalid pattern\n"), lf->re);
            regfree(&lf->compiled_re);
            return false;
        }
    }
    if (lf->flags & LF_EXC_REGEX) {
        reti = regcomp(&lf->compiled_ere, lf->ere, lf->reg_flags);
        if (reti) {
            fprintf(stderr, _("lf: '%s' Invalid exclude pattern\n"), lf->ere);
            regfree(&lf->compiled_ere);
            return false;
        }
    }
    debug_out(lf, argc, argv);
    //--------------------------------------------------------------------
    // Create and enqueue the first QueuePayload
    termination_status = TS_SUCCESS;
    int rc = 0;
    struct stat sb;
    rc = pthread_mutex_init(&lf->output_mutex, NULL);
    if (rc != 0)
        perror(_("Mutex initialization failed"));
    rc = stat(lf->base_path, &sb);
    if (rc != 0)
        return false;
    lf->q = mpmc_queue_init();
    QueuePayload root_node;
    cycle_arena_init();
    if (S_ISDIR(sb.st_mode)) {
        root_node.path_len = strnz__cpy(root_node.path, lf->base_path, MAX_PATH_LEN - 1);
        root_node.depth = 0;
        CycleNode *root_ctx = cycle_arena_alloc(sb.st_dev, sb.st_ino, nullptr);
        root_node.ctx = root_ctx;
        if (!mpmc_enqueue(lf, &root_node)) {
            fprintf(stderr, _("Failed to enqueue initial directory\n"));
            return false;
        }
        atomic_fetch_add_explicit(&lf->q->active_tasks, 1, memory_order_relaxed);
        //------------------------------------------------------------
        // INITIALIZE THREADS
        //------------------------------------------------------------
        lf->threads = calloc(lf->nthreads, sizeof(pthread_t));
        if (!lf->threads) {
            fprintf(stderr, _("Out of memory allocating threads\n"));
            return false;
        }
        for (unsigned int i = 0; i < lf->nthreads; i++) {
            rc = pthread_create(
                &lf->threads[i],
                NULL,
                worker,
                lf);
            if (rc != 0) {
                fprintf(stderr, _("Error: Unable to create thread %d\n"), rc);
                lf->q->shut_down = 1;
                termination_status = TS_ERROR;
                return false;
            }
        }
        //------------------------------------------------------------
        // END THREADS
        //------------------------------------------------------------
        for (unsigned int i = 0; i < lf->nthreads; i++)
            pthread_join(lf->threads[i], NULL);
        rc = pthread_mutex_destroy(&lf->output_mutex);
        if (rc != 0)
            perror(_("Mutex destroy failed"));
        if (lf->flags & LF_REGEX)
            regfree(&lf->compiled_re);
        if (lf->flags & LF_EXC_REGEX)
            regfree(&lf->compiled_ere);
        return true;
    } else {
        fprintf(stderr,
                _("Warning: Base path '%s' is not a directory. No "
                  "files will be found.\n"),
                lf->base_path);
        termination_status = TS_ERROR;
        return false;
    }
    if (reti)
        return false;
    return true;
}
// ----------------------------------------------------------------------
// CYCLE_ARENA_INIT
// ----------------------------------------------------------------------
/** @brief Initialize the cycle arena for tracking visited directories.
    @details This function initializes a global cycle arena structure that is used to track visited directories during the file search process. It reserves virtual memory space for the arena using mmap, allowing for efficient allocation of CycleNode structures without immediate physical memory allocation. The arena is designed to handle a maximum number of nodes defined by ARENA_MAX_NODES.
   */
void cycle_arena_init(void) {
    g_cycle_arena.capacity = ARENA_MAX_NODES;
    size_t total_bytes = g_cycle_arena.capacity * sizeof(CycleNode);

    // Reserve virtual space. The OS marks it as yours but allocates NO physical RAM yet. MAP_ANONYMOUS guarantees the memory block is zero-initialized automatically.
    g_cycle_arena.nodes = mmap(NULL, total_bytes,
                               PROT_READ | PROT_WRITE,
                               MAP_ANONYMOUS | MAP_PRIVATE,
                               -1, 0);

    if (g_cycle_arena.nodes == MAP_FAILED) {
        perror(_("mmap failed to reserve tracking arena memory"));
        exit(EXIT_FAILURE);
    }

    atomic_init(&g_cycle_arena.index, 0);
}
// ----------------------------------------------------------------------
// CYCLE_ARENA_DESTROY
// ----------------------------------------------------------------------
/** @brief Destroy the cycle arena and release allocated resources.
    @details This function releases the resources allocated for the global cycle arena structure. It unmaps the virtual memory space reserved for the arena using munmap, allowing the operating system to reclaim the memory. This function should be called when the cycle arena is no longer needed to prevent memory leaks.
   */
void cycle_arena_destroy(void) {
    if (g_cycle_arena.nodes != NULL && g_cycle_arena.nodes != MAP_FAILED) {
        size_t total_bytes = g_cycle_arena.capacity * sizeof(CycleNode);
        munmap(g_cycle_arena.nodes, total_bytes);
    }
}
// ----------------------------------------------------------------------
// CYCLE_ARENA_ALLOC
// ----------------------------------------------------------------------
/** @brief Allocate a new CycleNode in the cycle arena.
    @param dev The device ID of the directory being allocated.
    @param ino The inode number of the directory being allocated.
    @param parent A pointer to the parent CycleNode, or nullptr if this is the root.
    @return A pointer to the newly allocated CycleNode, or nullptr if allocation fails (e.g., if the arena is full).
    @details This function allocates a new CycleNode structure in the global cycle arena. It uses atomic operations to ensure thread-safe allocation of nodes. If the arena has reached its maximum capacity, the function returns nullptr to indicate that no more nodes can be allocated. The allocated node is initialized with the provided device ID, inode number, and parent pointer.
   */
static CycleNode *cycle_arena_alloc(dev_t dev, ino_t ino, CycleNode *parent) {
    size_t idx = atomic_fetch_add_explicit(&g_cycle_arena.index, 1, memory_order_relaxed);
    // Bounds check to ensure we don't breach our massive virtual limit
    if (idx >= g_cycle_arena.capacity) {
        return NULL;
    }
    // As soon as this thread hits an unmapped block of memory, the OS transparently provisions a 4KB hardware page of physical RAM for it on the fly.
    CycleNode *node = &g_cycle_arena.nodes[idx];
    node->dev = dev;
    node->ino = ino;
    node->parent = parent;
    return node;
}
// ----------------------------------------------------------------------
// BUILD_FULL_PATH
// ----------------------------------------------------------------------
/** @brief Build a full file path by concatenating a directory path and a file
 * name.
    @param dst A pointer to the destination buffer where the full path will be stored.
    @param dst_size The size of the destination buffer.
    @param dir_path The directory path to be concatenated.
    @param name The file name to be concatenated.
    @param out_len A pointer to a size_t variable where the length of the resulting full path will be stored (optional).
    @return true if the full path was successfully built, false if an error occurred (e.g., null pointers or insufficient buffer size).
    @details This function constructs a full file path by concatenating the provided directory path and file name. It ensures that there is a single '/' separator between the directory and file name, and it checks for sufficient buffer size before performing the concatenation. If successful, it null-terminates the resulting string and optionally returns its length.
   */
bool build_full_path(char *dst, size_t dst_size, const char *dir_path,
                     const char *name, size_t *out_len) {
    if (dst == nullptr || dir_path == nullptr || name == nullptr || dst_size == 0)
        return false;
    size_t dir_len = strlen(dir_path);
    size_t name_len = strlen(name);
    bool need_sep = dir_len > 0 && dir_path[dir_len - 1] != '/';
    size_t full_len = dir_len + (need_sep ? 1 : 0) + name_len;
    if (full_len >= dst_size)
        return false;
    memcpy(dst, dir_path, dir_len);
    size_t pos = dir_len;
    if (need_sep)
        dst[pos++] = '/';
    memcpy(dst + pos, name, name_len);
    dst[full_len] = '\0';
    if (out_len)
        *out_len = full_len;
    return true;
}
// ----------------------------------------------------------------------
// FLUSH_OUTPUT_BUFFER
// ----------------------------------------------------------------------
/** @brief Flush the output buffer to stdout in a thread-safe manner.
    @param lf A pointer to the LfContext struct containing the output mutex.
    @param output A pointer to the OutputBuffer struct containing the data to be flushed.
    @details This function writes the contents of the output buffer to stdout using fwrite_unlocked for efficiency. It uses a mutex to ensure that only one thread can write to stdout at a time, preventing interleaved output from multiple threads. After flushing, it resets the length of the output buffer to zero.
   */
void flush_output_buffer(LfContext *lf, OutputBuffer *output) {
    if (lf == nullptr || output == nullptr || output->len == 0)
        return;
    pthread_mutex_lock(&lf->output_mutex);
    fwrite_unlocked(output->data, 1, output->len, stdout);
    pthread_mutex_unlock(&lf->output_mutex);
    output->len = 0;
}
// ----------------------------------------------------------------------
// APPEND_OUTPUT_BUFFER
// ----------------------------------------------------------------------
/** @brief Append a path to the output buffer, flushing if necessary.
    @param lf A pointer to the LfContext struct containing the output mutex.
    @param output A pointer to the OutputBuffer struct where the path will be appended.
    @param path The path string to append to the output buffer.
    @param path_len The length of the path string.
    @param append_slash A boolean indicating whether to append a slash after the path.
    @return true if the path was successfully appended or printed, false if an error occurred (e.g., null pointers).
    @details This function appends a given path to an output buffer. If the buffer is full, it flushes the buffer to stdout. If the path is too long to fit in the buffer, it prints it directly to stdout. The function uses a mutex to ensure thread-safe access to stdout when printing directly.
   */
bool append_output_buffer(LfContext *lf, OutputBuffer *output, const char *path,
                          size_t path_len, bool append_slash) {
    if (lf == nullptr || output == nullptr || path == nullptr)
        return false;
    size_t line_len = path_len + (append_slash ? 1 : 0) + 1;
    if (line_len > sizeof(output->data)) {
        pthread_mutex_lock(&lf->output_mutex);
        fwrite_unlocked(path, 1, path_len, stdout);
        if (append_slash)
            fputc_unlocked('/', stdout);
        fputc_unlocked('\n', stdout);
        pthread_mutex_unlock(&lf->output_mutex);
        return true;
    }
    if (output->len + line_len > sizeof(output->data))
        flush_output_buffer(lf, output);
    memcpy(output->data + output->len, path, path_len);
    output->len += path_len;
    if (append_slash)
        output->data[output->len++] = '/';
    output->data[output->len++] = '\n';
    return true;
}
// ----------------------------------------------------------------------
// MPMC_QUEUE_INIT
// ----------------------------------------------------------------------
/** @brief Initialize the MPMCQueue.
    @param q A pointer to the MPMCQueue struct representing the queue.
    @details This function initializes the MPMCQueue by setting up the sequence numbers for each node in the queue and initializing the enqueue and dequeue indexes. It ensures that the queue is ready for concurrent access by multiple producer and consumer threads.
   */
MPMCQueue *mpmc_queue_init() {
    MPMCQueue *q = calloc(1, sizeof(MPMCQueue));
    if (q == nullptr)
        return nullptr;
    for (size_t i = 0; i < QUEUE_CAPACITY; i++)
        atomic_init(&q->sequence[i], i);
    atomic_init(&q->enqueue_pos, 0);
    atomic_init(&q->dequeue_pos, 0);
    atomic_init(&q->active_tasks, 0);
    atomic_init(&q->shut_down, 0);
    return q;
}
// ----------------------------------------------------------------------
// MPMC_ENQUEUE
// ----------------------------------------------------------------------
/** @brief Enqueue a task into the MPMCQueue.
    @param lf A pointer to the LfContext struct containing the queue.
    @param dir A pointer to the QueuePayload struct representing the task to enqueue.
    @return true if the task was successfully enqueued, false if the queue is full or an error occurred.
    @details This function attempts to enqueue a task into the MPMCQueue. It uses atomic operations to ensure thread-safe access to the queue's enqueue position and sequence numbers. If the queue is full, it returns false, allowing for potential recursion or other handling by the caller. The function also includes a spin-wait mechanism to reduce contention when multiple threads are trying to enqueue tasks simultaneously.
   */
bool mpmc_enqueue(LfContext *lf, const QueuePayload *child_node) {
    size_t pos = atomic_load_explicit(&lf->q->enqueue_pos,
                                      memory_order_relaxed);
    int spin_count = 0;
    const int SPIN_LIMIT = 16;
    while (true) {
        size_t seq = atomic_load_explicit(&lf->q->sequence[pos & QUEUE_MASK], memory_order_acquire);
        intptr_t diff = (intptr_t)seq - (intptr_t)pos;
        if (diff == 0) { // Queue is not full, ready to enqueue
            if (atomic_compare_exchange_weak_explicit(&lf->q->enqueue_pos, &pos, pos + 1,
                                                      memory_order_relaxed, memory_order_relaxed)) {
                break;
            }
        } else if (diff < 0) // Queue is full
            return false;    // trigger recursion
        else {
            pos++;
            // pos = atomic_load_explicit(&lf->q->enqueue_pos,
            // memory_order_relaxed);
        }
        if (++spin_count > SPIN_LIMIT)
            return false;
#if defined(__x86_64__)
        __builtin_ia32_pause();
#endif
    }
    memcpy(&lf->q->nodes[pos & QUEUE_MASK], child_node, sizeof(QueuePayload));
    atomic_store_explicit(&lf->q->sequence[pos & QUEUE_MASK], pos + 1, memory_order_release);
    return true;
}
// ----------------------------------------------------------------------
// MPMC_DEQUEUE
// ----------------------------------------------------------------------
/** @brief Dequeue a task from the MPMCQueue.
    @param lf A pointer to the LfContext struct containing the queue.
    @param task A pointer to the QueuePayload struct where the dequeued task will be stored.
    @return true if a task was successfully dequeued, false if the queue is empty or shut down.
    @details This function attempts to dequeue a task from the MPMCQueue. It uses atomic operations to ensure thread-safe access to the queue's dequeue position and sequence numbers. If the queue is empty, it yields the processor to reduce contention. If the queue has been shut down, it returns false, allowing for graceful termination of worker threads.
   */
bool mpmc_dequeue(LfContext *lf, QueuePayload *out_node) {
    size_t pos = atomic_load_explicit(&lf->q->dequeue_pos, memory_order_relaxed);
    while (true) {
        if (atomic_load_explicit(&lf->q->shut_down, memory_order_relaxed))
            return false;
        size_t seq = atomic_load_explicit(&lf->q->sequence[pos & QUEUE_MASK], memory_order_acquire);
        intptr_t diff = (intptr_t)seq - (intptr_t)(pos + 1);
        if (diff == 0) { // Full queue, ready to dequeue
            if (atomic_compare_exchange_strong_explicit(&lf->q->dequeue_pos, &pos, pos + 1,
                                                        memory_order_relaxed, memory_order_relaxed)) {
                break;
            }
        } else if (diff < 0) { // Queue is temporarily empty
            if (atomic_load_explicit(&lf->q->active_tasks,
                                     memory_order_relaxed) == 0) {
                atomic_store_explicit(&lf->q->shut_down, 1,
                                      memory_order_release);
                return false;
            }
            sched_yield();
            pos = atomic_load_explicit(&lf->q->dequeue_pos, memory_order_relaxed);
        } else {
            pos = atomic_load_explicit(&lf->q->dequeue_pos, memory_order_relaxed);
        }
    }
    // memcpy(output_node, &lf->q->nodes[pos & QUEUE_MASK],
    // sizeof(QueuePayload));
    if (out_node)
        *out_node = lf->q->nodes[pos & QUEUE_MASK];
    // current_node = &lf->q->nodes[pos & QUEUE_MASK];
    atomic_store_explicit(&lf->q->sequence[pos & QUEUE_MASK], pos + QUEUE_CAPACITY, memory_order_release);
    return true;
}
// ----------------------------------------------------------------------
// WORKER
// ----------------------------------------------------------------------
/** @brief Worker thread function that processes tasks from the MPMCQueue.
    @param arg A pointer to the LfContext struct containing the queue and other context information.
    @return NULL upon completion.
    @details This function runs in a loop, dequeuing tasks from the MPMCQueue and processing them using the finder function. It decrements the active task count after processing each task. If the active task count reaches zero, it sets the shut_down flag to true, signaling other threads to terminate. The function exits when there are no more tasks to process or when a shutdown is detected.
   */
void *worker(void *arg) {
    LfContext *lf = (LfContext *)arg;
    // current_node is a pointer to the current task being processed, while child_node is a local variable used to hold the next task to be enqueued. The worker thread continuously dequeues tasks from the queue and processes them using the finder function. If the finder function returns NULL, indicating that there are no more tasks to process, the active task count is decremented. If the active task count reaches zero, the shut_down flag is set to true, signaling other threads to terminate.
    //
    // thread_local variables
    char dir_buf[DIR_BUF_SIZE];
    // = calloc(1, DIR_BUF_SIZE);
    OutputBuffer output;
    // = calloc(1, sizeof(OutputBuffer));
    // char *full_path = calloc(1, MAX_PATH_LEN);
    char full_path[MAX_PATH_LEN];
    QueuePayload current_node;
    QueuePayload child_node;
    // if (child_node == nullptr) {
    //     fprintf(stderr, _("Out of memory allocating child_node\n"));
    //     return NULL;
    // }
    while (true) {
        if (mpmc_dequeue(lf, &current_node) == true) {
            memset(&child_node, 0, sizeof(QueuePayload));
            if (finder(lf, &current_node, &child_node, &output, dir_buf, full_path) == NULL) {
                if (atomic_fetch_sub_explicit(&lf->q->active_tasks, 1, memory_order_acq_rel) == 1) {
                    atomic_store_explicit(&lf->q->shut_down, 1, memory_order_release);
                }
            }
        } else
            break;
    }
    // free(child_node);
    // free(current_node);
    // free(full_path);
    // free(output);
    // free(dir_buf);
    return NULL;
}
// ----------------------------------------------------------------------
// FINDER
// ----------------------------------------------------------------------
/** @brief Process a directory and its entries, enqueueing subdirectories for further searching.
    @param lf A pointer to the LfContext struct containing the search settings.
    @param current_node A pointer to the QueuePayload struct representing the current directory being processed.
    @param child_node A pointer to the QueuePayload struct where new tasks will be enqueued.
    @param output A pointer to the OutputBuffer struct for buffering output.
    @param dir_buf A buffer for reading directory entries.
    @param full_path A buffer for constructing full file paths.
    @return NULL if processing is complete or an error occurred, otherwise returns a pointer to the next task to be enqueued.
    @details This function reads the contents of the current directory, processes each entry, and applies filters based on the user's settings (e.g., file types, regex patterns). It uses fstatat to retrieve metadata for each entry and determines whether to enqueue subdirectories for further searching. The function also handles symbolic links according to the user's options and manages output buffering for efficient writing to stdout.
   */
void *finder(LfContext *lf, QueuePayload *current_node, QueuePayload *child_node, OutputBuffer *output, char *dir_buf, char *full_path) {
    long nread;
    struct stat sb = {};
    bool stat_cached = false;
    struct linux_dirent64 *entry;
    dev_t actual_dev = 0, effective_dev = 0;
    ino_t actual_ino = 0, effective_ino = 0;
    unsigned char actual_type;
    unsigned char effective_type;
    CycleNode *child_ctx;
    int rc;
    int dir_fd = open(current_node->path, O_RDONLY | O_DIRECTORY);
    if (dir_fd == -1) {
        if (lf->report_errors) {
            err_out(lf, _("OPEN_FAIL,%s,%s\n"), current_node->path, strerror(errno));
        }
        flush_output_buffer(lf, output);
        return NULL;
    }
    while (1) {
        //--------------------------------------------------------------------
        // READ DIRECTORY
        //--------------------------------------------------------------------
        // Read the directory entries and process each one. We use SYS_getdents
        // to iterate over the entries in the directory. For each entry, we
        // construct the full path and use fstatat to get the metadata of the
        // entry. If the entry is a symbolic link, we check if the user has
        // chosen to follow links and get the metadata of the target it points
        // to. We then determine the effective type of the entry and apply the
        // specified filters (e.g., hidden files, max depth) to decide whether
        // to process it further or enqueue it for searching.
        nread = syscall(SYS_getdents64, dir_fd, dir_buf, DIR_BUF_SIZE);
        if (nread == -1) {
            atomic_fetch_add(&lf->error_count, 1);
            if (lf->report_errors) {
                err_out(lf, _("READDIR_FAIL,%s,%s\n"), current_node->path, strerror(errno));
            }
            break;
        }
        if (nread == 0)
            break;
        for (size_t bpos = 0; bpos < (size_t)nread;) {
            entry = (struct linux_dirent64 *)(dir_buf + bpos);
            bpos += entry->d_reclen;
            //--------------------------------------------------------------------
            // PROCESS DIRECTORY ENTRIES
            //--------------------------------------------------------------------
            // Get link's metadata We use fstatat with AT_SYMLINK_NOFOLLOW to get the metadata of the symbolic link itself, rather than the target it points to. This allows us to determine if the entry is a symbolic link and handle it according to the user's options (e.g., whether to follow links or not). If fstatat fails, we log the error (if debugging is enabled) and continue to the next entry without processing this one further.
            full_path[0] = '\0';
            actual_type = entry->d_type;
            effective_type = actual_type;
            if (actual_type == DT_DIR) {
                if (entry->d_name[0] == '.') {
                    if (entry->d_name[1] == '\0')
                        continue;
                    if (entry->d_name[1] == '.' && entry->d_name[2] == '\0')
                        continue;
                }
            }
            if (((lf->follow_links || lf->report_badlinks) &&
                 (actual_type == DT_LNK || actual_type == DT_DIR)) ||
                actual_type == DT_UNKNOWN) {
                //--------------------------------------------------------------------
                // GET ADVANCED METADATA
                //--------------------------------------------------------------------
                // We use fstatat with AT_SYMLINK_NOFOLLOW to get the metadata of the symbolic link itself, rather than the target it points to. This allows us to determine if the entry is a symbolic link and handle it according to the user's options (e.g., whether to follow links or not). If fstatat fails, we log the error (if debugging is enabled) and continue to the next entry without processing this one further.
                memset(&sb, 0, sizeof(struct stat));
                rc = fstatat(dir_fd, entry->d_name, &sb, AT_SYMLINK_NOFOLLOW);
                if (rc == -1) {
                    atomic_fetch_add(&lf->error_count, 1);
                    if (lf->report_badlinks) {
                        err_out(lf, _("LSTAT_FAIL,%s/%s,%s\n"), current_node->path, entry->d_name, strerror(errno));
                    }
                } else {

                    actual_ino = sb.st_ino;
                    actual_dev = sb.st_dev;
                    actual_type = (sb.st_mode & S_IFMT) >> 12;
                    effective_dev = actual_dev;
                    effective_ino = actual_ino;
                    effective_type = actual_type;
                    stat_cached = true;
                }
                if (S_ISLNK(sb.st_mode)) {
                    // Determine the real type of the entry. If the entry is a symbolic link, we set actual_type to DT_LNK and then attempt to get the metadata of the target it points to using fstatat without AT_SYMLINK_NOFOLLOW. This allows us to determine the effective type of the entry based on the target's metadata, which is important for deciding how to process it (e.g., whether it's a directory that we should enqueue for further searching). If fstatat fails when trying to get the target's metadata, we log the error (if debugging is enabled) but continue processing the entry based on its symbolic link metadata.
                    if (lf->follow_links) {
                        //------------------------------------------------------------
                        // GET LINK METADATA
                        //------------------------------------------------------------
                        char tmp_str[MAXLEN];
                        rc = fstatat(dir_fd, entry->d_name, &sb, 0);
                        if (rc == -1) {
                            atomic_fetch_add(&lf->error_count, 1);
                            if (lf->report_errors) {
                                err_out(lf, _("STAT_FAIL,%s,%s\n"), entry->d_name,
                                        strerror(errno));
                                ssnprintf(tmp_str, MAXLEN - 1, _("FSTATAT_FAIL,%s/%s\n"),
                                          entry->d_name, strerror(errno));
                            }
                        } else {
                            effective_ino = sb.st_ino;
                            effective_dev = sb.st_dev;
                            effective_type = (sb.st_mode & S_IFMT) >> 12;
                        }
                    } else {
                        effective_type = DT_REG;
                    }
                }
            }
            if (effective_type == DT_DIR) {
                //--------------------------------------------------------------------
                // PROCESS DIRECTORY ENTRY - CREATE CHILD_NODE
                //--------------------------------------------------------------------
                // We build the full path for the child directory and check if it exceeds the maximum path length. If it does, we log an error and continue to the next entry. If the child directory is within the allowed depth, we create a new QueuePayload for it, copying the current history of dev/inode pairs for cycle detection. We then attempt to enqueue the child directory for further processing. If the queue is full, we handle it by running the finder function inline recursively.
                child_ctx = cycle_arena_alloc(effective_dev, effective_ino, current_node->ctx);
                child_node->ctx = child_ctx;
                if (!build_full_path(child_node->path,
                                     MAX_PATH_LEN,
                                     current_node->path,
                                     entry->d_name,
                                     &child_node->path_len)) {
                    atomic_fetch_add(&lf->error_count, 1);
                    if (lf->report_errors) {
                        err_out(lf, _("PATH_TOO_LONG,%s\n"),
                                child_node->path);
                    }
                    continue;
                }
                bool cycle_found = false;
                // Determine the effective type of the entry. We use the st_mode
                // field from the stat struct to determine the file type by
                // applying the S_IFMT mask and shifting it to get a value that
                // corresponds to the DT_* constants. If the entry is a symbolic
                // link and the user has chosen not to follow links, we treat it
                // as a directory for the purpose of deciding whether to enqueue
                // it for further searching. This allows us to handle symbolic
                // links that point to directories in a way that respects the
                // user's options while still allowing for traversal of linked
                // directories if desired.
                if (lf->follow_links && actual_type == DT_LNK) {
                    // -------------------------------------------------------
                    // CYCLE_DETECTION
                    // -------------------------------------------------------
                    // Check for cycles by comparing the current directory's dev/inode with the history of dev/inode pairs from parent directories. If a match is found, it indicates a cycle and we skip processing this directory.
                    char target_path[MAX_PATH_LEN] = {'\0'};
                    if (lf->report_trace || lf->report_badlinks) {
                        ssize_t len = readlinkat(dir_fd, entry->d_name, target_path, sizeof(target_path) - 1);
                        if (len != -1)
                            target_path[len] = '\0';
                    }
                    child_ctx = cycle_arena_alloc(effective_dev, effective_ino, current_node->ctx);
                    if (child_ctx == nullptr) {
                        atomic_fetch_add(&lf->error_count, 1);
                        if (lf->report_errors) {
                            err_out(lf, _("CYCLE_ARENA_ALLOC_FAIL,%s\n"), current_node->path);
                        }
                        continue;
                    }
                    child_node->ctx = child_ctx;
                    if (lf->report_trace) {
                        err_out(lf, _("---CYCLE_DETECTION---\n"));
                        err_out(lf, "ctx:   %12p, %8lu/%8lu, %s\n", child_ctx, child_ctx->dev, child_ctx->ino, child_node->path);
                        err_out(lf, _("effective: %8lu/%8lu, %s\n"), effective_dev, effective_ino, target_path);
                    }
                    // is_link_cycle(effective_dev, effective_ino,
                    // current_node->ctx);
                    CycleNode *parent = current_node->ctx;
                    while (parent != nullptr) {
                        if (lf->report_trace) {
                            err_out(lf, "ctx:       %12p, %8lu/%8lu, %12p, %s\n", parent, parent->dev, parent->ino);
                        }
                        if (effective_dev == parent->dev && effective_ino == parent->ino) {
                            cycle_found = true;
                            break;
                        }
                        parent = parent->parent;
                    }
                    if (cycle_found) {
                        if (lf->report_badlinks) {
                            err_out(lf, _("CYCLER,%lu/%lu,%s,%lu/%lu,%s\n"), child_ctx->dev, child_ctx->ino, child_node->path,
                                    effective_dev, effective_ino, target_path);
                        }
                        atomic_fetch_add(&lf->error_count, 1);
                        child_node->depth = current_node->depth + 1;
                        scan_file(child_node->path, &child_node->path_len, lf, effective_type, &sb, stat_cached, output);
                        continue;
                    }
                }
                // --------------------------------------------------------
                // BUILD AND ENQUEUE CHILD TASK
                // --------------------------------------------------------
                if (lf->max_depth != 0 && current_node->depth + 1 >= lf->max_depth)
                    continue;
                child_node->depth = current_node->depth + 1;
                if (!cycle_found) {
                    if (mpmc_enqueue(lf, child_node)) {
                        atomic_fetch_add_explicit(&lf->q->active_tasks, 1, memory_order_relaxed);
                    } else {
                        QueuePayload *local_node = calloc(1, sizeof(QueuePayload));
                        if (local_node == nullptr) {
                            atomic_fetch_add(&lf->error_count, 1);
                            if (lf->report_errors) {
                                err_out(lf, _("local_node calloc failed,%s\n"), strerror(errno));
                            }
                            continue;
                        }
                        // TODO: add error handling for remaining calloc
                        // failures
                        OutputBuffer *local_output = calloc(1, sizeof(OutputBuffer));
                        char *local_dir_buf = calloc(1, DIR_BUF_SIZE);
                        char *local_full_path = calloc(1, MAX_PATH_LEN);
                        finder(lf, child_node, local_node, local_output, local_dir_buf, local_full_path);
                        free(local_full_path);
                        free(local_output);
                        free(local_dir_buf);
                        free(local_node);
                    }
                }
                // --------------------------------------------------------
                // CONDITIONALLY PRINT THE DIRECTORY
                // --------------------------------------------------------
                if (entry->d_name[0] == '.') {
                    if (!lf->include_hidden && !lf->hidden_only)
                        continue;
                } else {
                    if (lf->hidden_only)
                        continue;
                }
                if (!lf->only_errors)
                    scan_file(child_node->path, &child_node->path_len, lf, effective_type, &sb, stat_cached, output);
            } else {
                // --------------------------------------------------------
                // NOT DIRECTORY - CONDITIONALLY PRINT THE ENTRY
                // --------------------------------------------------------
                if (entry->d_name[0] == '.') {
                    if (!lf->include_hidden && !lf->hidden_only)
                        continue;
                } else {
                    if (lf->hidden_only)
                        continue;
                }
                size_t path_len;
                if (full_path[0] == '\0') {
                    if (!build_full_path(full_path,
                                         MAX_PATH_LEN,
                                         current_node->path,
                                         entry->d_name,
                                         &path_len)) {
                        atomic_fetch_add(&lf->error_count, 1);
                        if (lf->report_errors) {
                            err_out(lf, _("PATH_TOO_LONG,%s/%s\n"),
                                    current_node->path, entry->d_name);
                        }
                        continue;
                    }
                }
                if (!lf->only_errors)
                    scan_file(full_path, &path_len, lf, effective_type, &sb, stat_cached, output);
            }
        }
    }
    close(dir_fd);
    flush_output_buffer(lf, output);
    return NULL;
}
// ----------------------------------------------------------------------
// IS_LINK_CYCLE
// ----------------------------------------------------------------------
bool is_link_cycle(dev_t dev, ino_t ino, CycleNode *parent) {
    const CycleNode *current = parent;
    while (current != NULL) {
        if (current->ino == ino && current->dev == dev) {
            return true; // We hit a cycle!
        }
        current = current->parent; // Safe to dereference: addresses never change
    }
    return false;
}
// ----------------------------------------------------------------------
// SCAN_FILE
// ----------------------------------------------------------------------
/** @brief Scan a file or directory and apply filters based on the user's settings.
    @param file_spec The path to the file or directory to scan.
    @param path_len A pointer to the length of the file_spec string.
    @param lf A pointer to the LfContext struct containing the search settings.
    @param effective_type The effective type of the file (e.g., DT_REG, DT_DIR).
    @param cached_sb A pointer to a struct stat containing cached metadata for the file.
    @param stat_cached A boolean indicating whether the cached_sb contains valid data.
    @param output A pointer to the OutputBuffer struct for buffering output.
    @return true if the file was successfully scanned and processed, false if an error occurred or if the file was excluded by filters.
    @details This function applies various filters to determine whether a given file or directory should be included in the output. It checks for matching and non-matching regex patterns, user ownership, permissions, modification times, and file sizes. If the file passes all filters, it is added to the output buffer. The function also handles caching of file metadata to avoid redundant system calls.
   */
int scan_file(const char *file_spec, const size_t *path_len, LfContext *lf,
              const unsigned char effective_type, struct stat *cached_sb, bool stat_cached,
              OutputBuffer *output) {
    while (1) {
        if (lf->suppress_types & lf_mask[effective_type])
            break;
        // Exclude non-matching files
        if (lf->flags & LF_REGEX) {
            int reti =
                regexec(&lf->compiled_re, file_spec, 0, NULL, 0);
            if (reti == REG_NOMATCH)
                break;
            termination_status |= TS_MATCH;
        }
        // Exclude matching files
        if (lf->flags & LF_EXC_REGEX) {
            int reti =
                regexec(&lf->compiled_ere, file_spec, 0, NULL, 0);
            if (reti == 0) {
                termination_status |= TS_MATCH;
                break;
            }
        }
        //  Exclude files not owned by specified user
        if (lf->flags & LF_USER) {
            if (!stat_cached && stat(file_spec, cached_sb) == 0)
                stat_cached = true;
            if (!stat_cached || cached_sb->st_uid != lf->user_id)
                break;
        }
        if (lf->include_perms) {
            if (!stat_cached && stat(file_spec, cached_sb) == 0)
                stat_cached = true;
            if (!stat_cached)
                break;
            if ((lf->include_perms & LF_IRUSR) && !(cached_sb->st_mode & S_IRUSR))
                break;
            else if ((lf->include_perms & LF_IWUSR) && !(cached_sb->st_mode & S_IWUSR))
                break;
            else if ((lf->include_perms & LF_IXUSR) && !(cached_sb->st_mode & S_IXUSR))
                break;
            else if ((lf->include_perms & LF_ISUID) && !(cached_sb->st_mode & S_ISUID))
                break;
            else if ((lf->include_perms & LF_ISGID) && !(cached_sb->st_mode & S_ISGID))
                break;
        }
        if (lf->before) { // Last file modification
            if (!stat_cached && stat(file_spec, cached_sb) == 0)
                stat_cached = true;
            if (stat_cached && cached_sb->st_mtime > lf->before)
                break;
        }
        if (lf->after) { // Last file modification
            if (!stat_cached && stat(file_spec, cached_sb) == 0)
                stat_cached = true;
            if (stat_cached && cached_sb->st_mtime < lf->after)
                break;
        }
        if (lf->file_size_min) {
            if (!stat_cached && stat(file_spec, cached_sb) == 0)
                stat_cached = true;
            if (stat_cached && cached_sb->st_size < lf->file_size_min)
                break;
        }
        if (lf->only_errors)
            break;
        atomic_fetch_add(&lf->file_count, 1);
        if (!lf->count_silently) {
            const char *display_path = file_spec;
            size_t display_len = *path_len;
            if (display_len > 2 && display_path[0] == '.' && display_path[1] == '/') {
                display_path += 2;
                display_len -= 2;
            }
            append_output_buffer(lf, output, display_path, display_len,
                                 effective_type == DT_DIR);
        }
        break;
    }
    return true;
}
/** @brief Thread-safe error output function that writes formatted error messages to the specified error file or stderr.
    @param lf A pointer to the LfContext struct containing the output mutex and error file information.
    @param format A printf-style format string for the error message.
    @param ... Additional arguments corresponding to the format string.
    @return The number of characters written, or a negative value if an error occurred.
    @details This function locks the output mutex to ensure that only one thread can write to the error output at a time. It checks if an error file has been specified; if not, it defaults to stderr. If an error file is specified but not yet opened, it attempts to open it in append mode. The function uses vfprintf to write the formatted error message and then unlocks the mutex before returning.
   */
int err_out(LfContext *lf, const char *format, ...) {
    pthread_mutex_lock(&lf->output_mutex);
    va_list args;
    va_start(args, format);
    if (lf->error_file_spec[0] == '\0') {
        if (lf->err_fd == nullptr) {
            lf->err_fd = stderr;
            lf->error_file_open = true;
            setvbuf(lf->err_fd, NULL, _IOLBF, 0); //  line buffering
        }
    } else {
        if (lf->error_file_open == false) {
            lf->err_fd = fopen(lf->error_file_spec, "a");
            if (lf->err_fd == nullptr) {
                lf->err_fd = stderr;
                fprintf(stderr, _("Failed to open error file %s: %s\n"),
                        lf->error_file_spec, strerror(errno));
                return TS_ERROR;
            }
            lf->error_file_open = true;
            setvbuf(lf->err_fd, NULL, _IOLBF, 0); //  line buffering
        }
    }
    int result = vfprintf(lf->err_fd, format, args);
    pthread_mutex_unlock(&lf->output_mutex);
    va_end(args);
    return result;
}
