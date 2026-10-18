/** @file lf3.c
    @brief list files based on matching criteria
    @author Bill Waller
    Copyright (c) 2026
    MIT License
    billxwaller@gmail.com
    @date 2026-09-22
 */
#include <ncursesw/panel.h>
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

#define MAX_PATH_LEN _POSIX_PATH_MAX
#define MAX_DEPTH 64
#define DIR_BUF_SIZE 65536
#define CACHE_LINE_SIZE 64

#define QUEUE_NODES (4 * 1024 * 1024)       // 4194304 * 16 = 64 MB
#define CYCLE_NODES (1024 * 1024)           // 1048576 * 24 = 24 MB
#define DIRECTORY_NODES (1024 * 1024)       // 1048576 * 32 = 32 MB
#define PATH_ARENA_BYTES (32 * 1024 * 1024) // 33554432 = 32 MB

#define QUEUE_MASK (QUEUE_NODES - 1)
#define DIRECTORY_MASK (DIRECTORY_NODES - 1)
#define CYCLE_MASK (CYCLE_NODES - 1)
#define PATH_ARENA_MASK (PATH_ARENA_BYTES - 1)

#define STAGE_BUFFER_SIZE 64
#define DEBUG_CHILD_STAGING false
#define MAX_LOG_RECORD_LEN 4096
// on linux, write is atomic if record length <= 4096 bytes

typedef struct QueuePayload QueuePayload;
typedef struct DirectoryNode DirectoryNode;
typedef struct CycleNode CycleNode;
typedef struct PathArena PathArena;
typedef struct DirectoryArena DirectoryArena;
typedef struct CycleArena CycleArena;

// ---------------------------------------------------------------
// QUEUE STRUCTURES
// ---------------------------------------------------------------

#define NO_PARENT UINT32_MAX

struct DirectoryNode {
    size_t path_offset;
    size_t path_len;
    _Atomic size_t child_dirs_remaining;
    uint16_t depth;
    uint32_t parent_idx; // <-- CHANGED: Array index of parent node
};

struct CycleNode {
    dev_t dev;
    ino_t ino;
    uint32_t parent_idx; // <-- CHANGED: Array index for cycle walking
};

struct QueuePayload {
    uint32_t self_idx; // Track where this node lives in the QueueArena
    DirectoryNode *dir_ctx;
    CycleNode *cycle_ctx;
};

// ---------------------------------------------------------------

struct PathArena {
    char *bytes;
    _Atomic size_t cursor;
    size_t capacity;
};

struct DirectoryArena {
    DirectoryNode *nodes;
    _Atomic size_t index;
    size_t capacity;
};

struct CycleArena {
    CycleNode *nodes;
    _Atomic size_t index;
    size_t capacity;
};

typedef struct {
    QueuePayload *nodes;
    _Atomic size_t cursor;
    size_t capacity;
} QueueArena;

// ---------------------------------------------------------------

// static QueuePayload queue_nodes[QUEUE_NODES];
typedef struct {
    alignas(CACHE_LINE_SIZE) _Atomic size_t enqueue_pos;
    alignas(CACHE_LINE_SIZE) _Atomic size_t dequeue_pos;
    alignas(CACHE_LINE_SIZE) _Atomic atomic_int active_tasks;
    alignas(CACHE_LINE_SIZE) _Atomic atomic_int shut_down;
    alignas(CACHE_LINE_SIZE) _Atomic size_t sequence[QUEUE_NODES];
    QueuePayload nodes[QUEUE_NODES];
} MPMCQueue;

// ---------------------------------------------------------------

#define TM_RESET 1

long timer_start(LogCtx *log_ctx);
long timer_mark(LogCtx *log_ctx, char *ts_buf);
long timer_elapsed(LogCtx *log_ctx, int flag, char *ts_buf);

#define MAX_THREADS 64

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
    alignas(CACHE_LINE_SIZE) _Atomic size_t thread_id;
    PathArena *path_arena;
    QueueArena *queue_arena;
    DirectoryArena *directory_arena;
    CycleArena *cycle_arena;
    pthread_t *threads;
    long start;
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
    char log_record[MAX_LOG_RECORD_LEN];
    bool unlink;
    bool depth_first;
    bool report_error_count;
    bool ignore_case;
    bool sort;
    bool sort_reverse;
    bool include_hidden;
    bool hidden_only;
    bool follow_links;
    bool log_to_display;
    bool report_config;
    bool report_info;
    bool report_debug;
    bool report_errors;
    bool report_badlinks;
    bool report_trace;
    bool report_all;
    int timestamp_style;
    int log_level;
    bool log_to_file;
    bool log_config;
    bool log_info;
    bool log_errors;
    bool log_badlinks;
    bool log_debug;
    bool log_trace;
    bool log_all;
    bool count;
    bool count_silently;
    bool only_errors;
} LfCtx;

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
char *log_p;
void debug_out(LfCtx *lf, int, char **);
bool init_lf(LfCtx *lf, int, char **);
int sort_lf_output(LfCtx *lf, int, char **);
void *worker(void *arg);
void *finder(LfCtx *lf, QueuePayload *dequeued_node, OutputBuffer *output, char *dir_buf, char *current_dir_path, char *full_path, size_t thread_id, LogCtx *log_ctx);
int print_file(LfCtx *lf, const unsigned char effective_type, struct stat *cached_sb, bool stat_cached, OutputBuffer *output, char *full_path);
bool build_current_dir_path(char *, size_t, const char *, const char *, size_t *);
void flush_output_buffer(LfCtx *, OutputBuffer *);
bool append_output_buffer(LfCtx *, OutputBuffer *, const char *, size_t,
                          bool);
__attribute__((format(printf, 6, 7))) void log_write(const LfLogLevel level, LogCtx *log_ctx, const char *file, const char *func, const int line, const char *fmt, ...);
void get_filename(PathArena *path_arena, QueuePayload *qnode, char *current_dir_path, size_t len);
void finalize_directory_node(LfCtx *lf, size_t node_offset);

void queue_arena_create(LfCtx *lf, size_t max_nodes);
void queue_arena_destroy(LfCtx *lf);
uint32_t queue_arena_push_root(LfCtx *lf, size_t path_offset, size_t path_len, dev_t dev, ino_t ino);
bool mpmc_enqueue(LfCtx *, const QueuePayload *new_node, size_t thread_id, LogCtx *log_ctx);
bool mpmc_dequeue(LfCtx *lf, QueuePayload *dequeued_node, size_t thread_id);

void path_arena_init(LfCtx *lf, LogCtx *log_ctx);
void path_arena_destroy(LfCtx *lf);
static size_t path_arena_alloc(LfCtx *lf, const char *path, size_t len);

void directory_arena_init(LfCtx *lf);
void directory_arena_destroy(LfCtx *lf);

DirectoryNode *directory_arena_alloc(LfCtx *lf, size_t path_offset, size_t path_len, uint16_t depth, uint32_t parent_idx);

void cycle_arena_init(LfCtx *lf);
void cycle_arena_destroy(LfCtx *lf);
CycleNode *cycle_arena_alloc(LfCtx *lf, dev_t dev, ino_t ino, size_t parent_idx);
void flush_staged_children(LfCtx *lf, QueuePayload *stage, size_t stage_buffer_count, size_t thread_id, LogCtx *log_ctx);
void iso8601_timestamp(LogCtx *log_ctx);
void coder_timestamp(LogCtx *log_ctx);
// ---------------------------------------------------------------

static struct argp_option options[] = {
    {_("after"), 'a', _("time"), 0, _("Last Modified after YYYY-MM-DDTHH:MM:SS"), 0},
    {_("before"), 'b', _("time"), 0, _("Last Modified before YYYY-MM-DDTHH:MM:SS"), 0},
    {_("max_depth"), 'd', _("number"), 0, _("Depth into directory tree"), 0},
    {_("depth_first"), 'W', _("bool"), 0, _("Depth first order (widdershins)"), 0},
    {_("unlink"), 'U', _("bool"), 0, _("unlink (delete) implies depth_first"), 0},
    {_("error_file_spec"), 'E', _("file_spec"), 0, _("Error message output file"), 0},
    {_("ere"), 'e', _("regex"), 0, _("Exclude regular expression"), 0},
    {_("ignore_case"), 'i', 0, 0, _("Search ignore case"), 0},
    {_("log_level"), 'l', _("level"), 0, _("1-config, 2-info, 3-errors, 4-debug, 5-trace"), 0},
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
     _("1-config, 2-info, 3-errors 4-debug, 5-trace, 5-trace, 6-only_errors, 7-error_count"),
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
    LfCtx *lf = state->input;
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
        lf->log_to_display = true;
        if (arg && arg[0] != '\0') {
            debug_p = strdup(arg);
            lf->report_errors = true;
            while (debug_p[i]) {
                a_toi_error = false;
                char c[2];
                c[0] = debug_p[i];
                c[1] = '\0';
                int n = a_toi(c, &a_toi_error);
                lf->report_errors = true;
                switch (n) {
                case CONFIG:
                    lf->report_config = true;
                    lf->only_errors = true;
                    break;
                case INFO:
                    lf->report_info = true;
                    break;
                case ERROR:
                    lf->report_errors = true;
                    break;
                case DEBUG:
                    lf->report_debug = true;
                    break;
                case TRACE:
                    lf->report_trace = true;
                    break;
                case ONLY_ERRORS:
                    lf->only_errors = true;
                    lf->report_errors = true;
                    break;
                case ERROR_COUNT:
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
    case 'l':
        lf->log_to_file = true;
        if (arg && arg[0] != '\0') {
            log_p = strdup(arg);
            while (log_p[i]) {
                a_toi_error = false;
                char c[2];
                c[0] = log_p[i];
                c[1] = '\0';
                int n = a_toi(c, &a_toi_error);
                if (n > lf->log_level)
                    lf->log_level = n;
                switch (n) {
                case INFO:
                    lf->log_info = true;
                    break;
                case ERROR:
                    lf->log_info = true;
                    lf->log_errors = true;
                    lf->log_badlinks = true;
                    break;
                case DEBUG:
                    lf->log_info = true;
                    lf->log_errors = true;
                    lf->log_debug = true;
                    break;
                case TRACE:
                    lf->log_info = true;
                    lf->log_errors = true;
                    lf->log_badlinks = true;
                    lf->log_debug = true;
                    lf->log_trace = true;
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
    case 'W':
        lf->depth_first = true;
        break;
    case 'U':
        lf->unlink = true;
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
        } else
            fprintf(stderr, _("User '%s' not found.\n"), arg);
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
    LfCtx *lf = (LfCtx *)calloc(1, sizeof(LfCtx));

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
    lf->depth_first = false;
    lf->unlink = false;
    lf->follow_links = false;
    lf->log_to_file = false;
    lf->log_info = false;
    lf->log_config = false;
    lf->log_debug = false;
    lf->log_errors = false;
    lf->log_trace = false;
    lf->log_all = false;
    lf->count = false;
    lf->count_silently = false;
    lf->report_badlinks = false;
    lf->sbuffer = _("4G");
    lf->error_file_spec[0] = '\0';
    lf->error_file_open = false;

    setlocale(LC_NUMERIC, "");
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
    @param lf A pointer to the LfCtx struct containing the search settings.
    @param argc The number of command-line arguments.
    @param argv An array of command-line argument strings.
    @details This function sets up a pipeline to sort the output of the lf command. It creates a pipe, forks a child process to execute the sort command, and redirects the standard input and output streams accordingly. The sort command is executed with optional flags for parallel processing and buffer size, as well as a reverse sort option if specified in the LfCtx. The function waits for the child process to complete before returning.
   */
int sort_lf_output(LfCtx *lf, int argc, char **argv) {
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
    @param lf A pointer to the LfCtx struct containing the debug settings.
    @param argc The number of command-line arguments.
    @param argv An array of command-line argument strings.
    @details This function outputs debug information to stderr if debugging is enabled in the LfCtx. It includes a timestamp, user information, IP addresses, command-line arguments, and various configuration settings. The output is formatted for readability and includes details about file types, permissions, regex patterns, and other relevant settings.
   */
void debug_out(LfCtx *lf, int argc, char **argv) {
    char user_str[100];
    char ip_str[MAXLEN];
    int len = 0;
    int i;
    bool addspace_before = false;
    if (lf->log_to_display && (lf->report_config || lf->report_info)) {
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
                lf->log_to_display ? _("true") : _("     false"));
        fprintf(stderr, _("  1-config      %s\n"),
                lf->report_config ? _("true") : _("|    false"));
        fprintf(stderr, _("  2-info        %s\n"),
                lf->report_info ? _("true") : _("|    false"));
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
    @param lf A pointer to the LfCtx struct containing the search settings.
    @param argc The number of command-line arguments.
    @param argv An array of command-line argument strings.
    @return true if initialization is successful, false otherwise.
    @details This function initializes the file search process based on the
    provided settings in the LfCtx struct. It sets up file type inclusion
    and suppression, compiles regular expressions if specified, and creates a
    multi-producer, multi-consumer queue for managing directory traversal tasks.
    It also initializes threads for concurrent processing of directory entries.
   */
bool init_lf(LfCtx *lf, int argc, char **argv) {
    /** suppress file types that aren't included */
    LogCtx x_log_ctx = {
        .fd = -1,
        .err_fd = -1,
        .start = 0,
        .prev_nsec = 0,
        .prev_sec = 0,
        .log_level = lf->log_level,
        .timestamp_style = CODER,
    };
    LogCtx *log_ctx = &x_log_ctx;
    if (lf->log_to_display)
        log_ctx->err_fd = STDERR_FILENO;
    if (lf->log_to_file) {
        timer_start(log_ctx);
        log_open(log_ctx, 0);
    }
    if (!lf->include_types)
        lf->include_types = 0xff;
    if (lf->include_types)
        lf->suppress_types = lf->include_types ^ 0xff;
    lf->include_hidden = !(lf->flags & LF_HIDE);
    int reti = 0;
    lf->reg_flags = REG_EXTENDED;
    if (lf->ignore_case)
        lf->reg_flags |= REG_ICASE;
    if (lf->flags & LF_REGEX) {
        reti = regcomp(&lf->compiled_re, lf->re, lf->reg_flags);
        if (reti) {
            ssnprintf(lf->log_record, MAX_LOG_REC_LEN - 1, _("lf->re: '%s' Invalid pattern\n"), lf->re);
            fprintf(stderr, "%s", lf->log_record);
            lf_log(ERROR, "%s", lf->log_record);
            regfree(&lf->compiled_re);
            return false;
        }
    }
    if (lf->flags & LF_EXC_REGEX) {
        reti = regcomp(&lf->compiled_ere, lf->ere, lf->reg_flags);
        if (reti) {
            ssnprintf(lf->log_record, MAX_LOG_REC_LEN - 1, _("lf->ere: '%s' Invalid pattern\n"), lf->ere);
            fprintf(stderr, "%s", lf->log_record);
            lf_log(ERROR, "%s", lf->log_record);
            regfree(&lf->compiled_ere);
            return false;
        }
    }
    debug_out(lf, argc, argv);
    termination_status = TS_SUCCESS;
    int rc = 0;
    struct stat sb;
    rc = pthread_mutex_init(&lf->output_mutex, NULL);
    if (rc != 0)
        perror(_("Mutex initialization failed"));
    rc = stat(lf->base_path, &sb);
    if (rc != 0)
        return false;
    // --------------------------------------------------------------------
    // INITIALIZE ARENAS
    // --------------------------------------------------------------------
    lf_log(INFO, "Initializing MPMC queue");
    lf->q = (MPMCQueue *)calloc(1, sizeof(MPMCQueue));
    if (lf->q == nullptr)
        return false;
    for (size_t i = 0; i < QUEUE_NODES; i++)
        atomic_init(&lf->q->sequence[i], i);
    atomic_init(&lf->q->enqueue_pos, 0);
    atomic_init(&lf->q->dequeue_pos, 0);
    atomic_init(&lf->q->active_tasks, 0);
    atomic_init(&lf->q->shut_down, 0);
    lf_log(INFO, _("Initializing Queue Arena"));
    queue_arena_create(lf, QUEUE_NODES);
    lf_log(INFO, _("Initializing Path Arena"));
    path_arena_init(lf, log_ctx);
    lf_log(INFO, _("Initializing Directory Arena"));
    directory_arena_init(lf);
    lf_log(INFO, _("Initializing Cycle Arena"));
    cycle_arena_init(lf);
    atomic_init(&lf->thread_id, 0);
    if (!lf->path_arena || !lf->directory_arena ||
        !lf->cycle_arena || !lf->queue_arena) {
        termination_status = TS_ERROR;
        goto endlf;
    }
    // --------------------------------------------------------------------
    // CREATE_ROOT_NODE
    // --------------------------------------------------------------------

    // 4. Resolve the root path string and stat details to populate slot 0
    size_t path_len = strlen(lf->base_path);
    if (stat(lf->base_path, &sb) == -1) {
        perror("Failed to stat base_path");
        queue_arena_destroy(lf);
        termination_status = TS_ERROR;
        goto endlf;
    }
    if (!S_ISDIR(sb.st_mode)) {
        ssnprintf(lf->log_record, MAX_LOG_REC_LEN - 1,
                  _("Warning: Base path '%s' is not a directory. No "
                    "files will be found.\n"),
                  lf->base_path);
        fprintf(stderr, "%s\n", lf->log_record);
        lf_log(ERROR, "%s", lf->log_record);
        termination_status = TS_ERROR;
        goto endlf;
    }
    lf_log(INFO, _("Root path: %s"), lf->base_path);

    // 5. Store the root path string in the path arena
    size_t offset = path_arena_alloc(lf, lf->base_path, path_len);
    // 6. Push the root node into the queue arena
    uint32_t root_node_idx = queue_arena_push_root(lf, offset,
                                                   path_len, sb.st_dev, sb.st_ino);
    // 7. Enqueue the root node index task into the MPMC Queue to wake up worker
    // 1
    QueuePayload root_node = lf->queue_arena->nodes[root_node_idx];
    // Mark global active tasks as 1
    atomic_store_explicit(&lf->q->active_tasks, 1, memory_order_release);
    mpmc_enqueue(lf, &root_node, 0, log_ctx);

    // ------------------------------------------------------------
    // LOOSE THE THREADS
    // ------------------------------------------------------------
    lf_log(INFO, _("Starting threads"));
    lf->threads = calloc(lf->nthreads, sizeof(pthread_t));
    if (!lf->threads) {
        lf_log(ERROR, _("Out of memory allocating threads"));
        fprintf(stderr, _("Out of memory allocating threads\n"));
        goto endlf;
    }
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    lf->start = tp.tv_nsec;
    for (unsigned int i = 0; i < lf->nthreads; i++) {
        rc = pthread_create(&lf->threads[i], NULL,
                            worker, lf);
        if (rc != 0) {
            lf_log(ERROR, _("Error: Unable to create thread %d\n"), rc);
            fprintf(stderr, _("Error: Unable to create thread %d\n"), rc);
            lf->q->shut_down = 1;
            termination_status = TS_ERROR;
            goto endlf;
        }
    }
    // ------------------------------------------------------------
    // THREAD SHUTDOWN
    // ------------------------------------------------------------
endlf:
    lf_log(INFO, "Waiting for threads to finish...");
    for (unsigned int i = 0; i < lf->nthreads; i++)
        pthread_join(lf->threads[i], NULL);
    rc = pthread_mutex_destroy(&lf->output_mutex);
    if (rc != 0)
        perror(_("Mutex destroy failed"));
    if (lf->flags & LF_REGEX)
        regfree(&lf->compiled_re);
    if (lf->flags & LF_EXC_REGEX)
        regfree(&lf->compiled_ere);
    path_arena_destroy(lf);
    cycle_arena_destroy(lf);
    directory_arena_destroy(lf);
    queue_arena_destroy(lf);
    return true;
}
// ----------------------------------------------------------------------
// PATH_ARENA
// ----------------------------------------------------------------------
void path_arena_init(LfCtx *lf, LogCtx *log_ctx) {
    lf->path_arena = calloc(PATH_ARENA_BYTES, sizeof(char *));
    lf->path_arena->capacity = PATH_ARENA_BYTES;
    size_t total_bytes = lf->path_arena->capacity;
    lf->path_arena->bytes = mmap(NULL, total_bytes, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (lf->path_arena->bytes == MAP_FAILED) {
        lf_log(ERROR, "mmap failed to reserve path_arena memory");
        perror(_("mmap failed to reserve path_arena memory"));
    }
    atomic_init(&lf->path_arena->cursor, 0);
}
void path_arena_destroy(LfCtx *lf) {
    if (lf->path_arena->bytes != NULL && lf->path_arena->bytes != MAP_FAILED)
        munmap(lf->path_arena->bytes, PATH_ARENA_BYTES);
    if (lf->path_arena != NULL)
        free(lf->path_arena);
}
static size_t path_arena_alloc(LfCtx *lf, const char *path, size_t len) {
    // Reserve len + 1 bytes to safely encapsulate the null terminator
    size_t offset = atomic_fetch_add_explicit(&lf->path_arena->cursor, len + 1, memory_order_relaxed);
    if (offset + len + 1 >= PATH_ARENA_BYTES) {
        fprintf(stderr, "Fatal: Path arena exhausted (%zu / %d bytes)\n", offset + len + 1, PATH_ARENA_BYTES);
    }
    memcpy(&lf->path_arena->bytes[offset], path, len);
    lf->path_arena->bytes[offset + len] = '\0';
    return offset; // Return back to worker to record in dir_ctx.path_offset
}
// ----------------------------------------------------------------------
// DIRECTORY_ARENA
// ----------------------------------------------------------------------
void directory_arena_init(LfCtx *lf) {
    lf->directory_arena = (DirectoryArena *)calloc(DIRECTORY_NODES, sizeof(DirectoryNode));
    size_t total_bytes = DIRECTORY_NODES * sizeof(DirectoryNode);
    lf->directory_arena->nodes = mmap(NULL, total_bytes, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (lf->directory_arena->nodes == MAP_FAILED) {
        perror(_("mmap failed to reserve directory_arena memory"));
    }
    atomic_init(&lf->directory_arena->index, 0);
}
void directory_arena_destroy(LfCtx *lf) {
    if (lf->directory_arena->nodes != NULL && lf->directory_arena->nodes != MAP_FAILED)
        munmap(lf->directory_arena->nodes, DIRECTORY_NODES);
    if (lf->directory_arena != NULL)
        free(lf->directory_arena);
}
DirectoryNode *directory_arena_alloc(LfCtx *lf, size_t path_offset, size_t path_len, uint16_t depth, uint32_t parent_idx) {
    size_t idx = atomic_fetch_add_explicit(&lf->directory_arena->index, 1, memory_order_relaxed);
    if (idx >= DIRECTORY_NODES)
        return NULL;
    DirectoryNode *node = &lf->directory_arena->nodes[idx];
    node->parent_idx = parent_idx;
    node->depth = depth ? depth + 1 : 0;
    node->child_dirs_remaining = 0;
    node->path_offset = path_offset;
    node->path_len = path_len;
    return node;
}
// ----------------------------------------------------------------------
// CYCLE_ARENA
// ----------------------------------------------------------------------
void cycle_arena_init(LfCtx *lf) {
    lf->cycle_arena = (CycleArena *)calloc(CYCLE_NODES, sizeof(CycleNode));
    size_t total_bytes = CYCLE_NODES * sizeof(CycleNode);
    lf->cycle_arena->nodes = mmap(NULL, total_bytes, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (lf->cycle_arena->nodes == MAP_FAILED) {
        perror(_("mmap failed to reserve cycle_arena memory"));
    }
    atomic_init(&lf->cycle_arena->index, 0);
}
void cycle_arena_destroy(LfCtx *lf) {
    if (lf->cycle_arena->nodes != NULL && lf->cycle_arena->nodes != MAP_FAILED)
        munmap(lf->cycle_arena->nodes, CYCLE_NODES);
    if (lf->cycle_arena != NULL)
        free(lf->cycle_arena);
}
CycleNode *cycle_arena_alloc(LfCtx *lf, dev_t dev, ino_t ino, size_t parent_idx) {
    size_t idx = atomic_fetch_add_explicit(&lf->cycle_arena->index, 1, memory_order_relaxed);
    if (idx >= CYCLE_NODES)
        return NULL;
    CycleNode *node = &lf->cycle_arena->nodes[idx];
    node->parent_idx = parent_idx;
    node->dev = dev;
    node->ino = ino;
    return node;
}
// ----------------------------------------------------------------------
// QUEUE_ARENA
// ----------------------------------------------------------------------
void queue_arena_create(LfCtx *lf, size_t max_nodes) {
    lf->queue_arena = (QueueArena *)malloc(sizeof(QueueArena));
    if (!lf->queue_arena) {
        perror("Failed to allocate QueueArena management struct");
        return;
    }
    lf->queue_arena->capacity = max_nodes;
    atomic_store_explicit(&lf->queue_arena->cursor, 0, memory_order_relaxed);
    lf->queue_arena->nodes = (QueuePayload *)mmap(NULL, max_nodes * sizeof(QueuePayload), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (lf->queue_arena->nodes == MAP_FAILED) {
        perror("Failed to mmap memory for QueueArena elements");
        free(lf->queue_arena);
    }
    return;
}
// Push the very first root directory into the queue arena
uint32_t queue_arena_push_root(LfCtx *lf, size_t path_offset, size_t path_len, dev_t dev, ino_t ino) {
    // Claim slot 0
    size_t idx = atomic_fetch_add_explicit(&lf->queue_arena->cursor, 1, memory_order_relaxed);
    QueuePayload *root_node = &lf->queue_arena->nodes[idx];
    root_node->self_idx = (uint32_t)idx;
    root_node->dir_ctx = directory_arena_alloc(lf, path_offset, path_len, 0, NO_PARENT);
    root_node->cycle_ctx = cycle_arena_alloc(lf, dev, ino, NO_PARENT);
    // Directory node setup
    root_node->dir_ctx->path_offset = path_offset;
    root_node->dir_ctx->path_len = path_len;
    root_node->dir_ctx->depth = 0;
    root_node->dir_ctx->parent_idx = NO_PARENT; // Sentinel value (UINT32_MAX)
    atomic_init(&root_node->dir_ctx->child_dirs_remaining, 0);

    // Cycle tracking root setup
    root_node->cycle_ctx->dev = dev;
    root_node->cycle_ctx->ino = ino;
    root_node->cycle_ctx->parent_idx = NO_PARENT;

    return (uint32_t)idx;
}
void queue_arena_destroy(LfCtx *lf) {
    if (!lf->queue_arena)
        return;
    munmap(lf->queue_arena->nodes, lf->queue_arena->capacity * sizeof(QueuePayload));
    free(lf->queue_arena);
}
// ----------------------------------------------------------------------
// FLUSH_OUTPUT_BUFFER
// ----------------------------------------------------------------------
/** @brief Flush the output buffer to stdout in a thread-safe manner.
    @param lf A pointer to the LfCtx struct containing the output mutex.
    @param output A pointer to the OutputBuffer struct containing the data to be flushed.
    @details This function writes the contents of the output buffer to stdout using fwrite_unlocked for efficiency. It uses a mutex to ensure that only one thread can write to stdout at a time, preventing interleaved output from multiple threads. After flushing, it resets the length of the output buffer to zero.
   */
void flush_output_buffer(LfCtx *lf, OutputBuffer *output) {
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
    @param lf A pointer to the LfCtx struct containing the output mutex.
    @param output A pointer to the OutputBuffer struct where the path will be appended.
    @param path The path string to append to the output buffer.
    @param path_len The length of the path string.
    @param append_slash A boolean indicating whether to append a slash after the path.
    @return true if the path was successfully appended or printed, false if an error occurred (e.g., null pointers).
    @details This function appends a given path to an output buffer. If the buffer is full, it flushes the buffer to stdout. If the path is too long to fit in the buffer, it prints it directly to stdout. The function uses a mutex to ensure thread-safe access to stdout when printing directly.
   */
bool append_output_buffer(LfCtx *lf, OutputBuffer *output, const char *path,
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
// MPMC_ENQUEUE
// ----------------------------------------------------------------------
bool mpmc_enqueue(LfCtx *lf, const QueuePayload *new_node, size_t thread_id, LogCtx *log_ctx) {
    size_t pos = atomic_load_explicit(&lf->q->enqueue_pos, memory_order_acquire);
    int spin_count = 0;
    const int SPIN_LIMIT = 16;
    while (true) {
        size_t seq = atomic_load_explicit(&lf->q->sequence[pos & QUEUE_MASK], memory_order_acquire);
        intptr_t diff = (intptr_t)seq - (intptr_t)pos;
        if (diff == 0) {
            if (atomic_compare_exchange_weak_explicit(&lf->q->enqueue_pos, &pos, pos + 1, memory_order_release, memory_order_relaxed))
                break;
        } else if (diff < 0) { // Queue is full
            return false;
            if (lf->log_to_display || lf->log_to_file) {
                char log_record[MAXLEN];
                ssnprintf(log_record, MAXLEN - 1, "%3lu <%10s %12p %6u %6u %6lu/%-6lu %s",
                          thread_id,
                          "queue full",
                          new_node,
                          new_node->self_idx,
                          new_node->dir_ctx->parent_idx != NO_PARENT ? new_node->dir_ctx->parent_idx : 0, new_node->dir_ctx->path_offset,
                          new_node->dir_ctx->path_len,
                          &lf->path_arena->bytes[new_node->dir_ctx->path_offset]);
                lf_log(DEBUG, "%s", log_record);
                lf_log(DEBUG, "Queue full, thread %zu waiting to enqueue", thread_id);
            }
        } else {
            pos = atomic_load_explicit(&lf->q->enqueue_pos, memory_order_acquire);
            // pos++;
        }
        if (++spin_count > SPIN_LIMIT)
            return false;
#if defined(__x86_64__)
        __builtin_ia32_pause();
#endif
    }
    memcpy(&lf->q->nodes[pos & QUEUE_MASK], new_node, sizeof(QueuePayload));
    // 3. This release store pairs with dequeue's acquire load of the sequence
    atomic_store_explicit(&lf->q->sequence[pos & QUEUE_MASK], pos + 1, memory_order_release);
    return true;
}

bool mpmc_dequeue(LfCtx *lf, QueuePayload *dequeued_node, size_t thread_id) {
    (void)thread_id;
    // 4. Change to acquire to see valid changes to dequeue_pos
    size_t pos = atomic_load_explicit(&lf->q->dequeue_pos, memory_order_acquire);
    while (true) {
        if (atomic_load_explicit(&lf->q->shut_down, memory_order_relaxed))
            return false;
        size_t seq = atomic_load_explicit(&lf->q->sequence[pos & QUEUE_MASK], memory_order_acquire);
        intptr_t diff = (intptr_t)seq - (intptr_t)(pos + 1);
        if (diff == 0) {
            // 5. Change to release on success to publish the step forward
            //
            if (atomic_compare_exchange_weak_explicit(&lf->q->dequeue_pos, &pos, pos + 1, memory_order_release, memory_order_relaxed)) {
                break;
            }
        } else if (diff < 0) {
            if (atomic_load_explicit(&lf->q->active_tasks, memory_order_relaxed) == 0) {
                atomic_store_explicit(&lf->q->shut_down, 1, memory_order_release);
                return false;
            }
            sched_yield();
            pos = atomic_load_explicit(&lf->q->dequeue_pos, memory_order_acquire);
        } else {
            pos = atomic_load_explicit(&lf->q->dequeue_pos, memory_order_acquire);
        }
    }
    if (dequeued_node)
        *dequeued_node = lf->q->nodes[pos & QUEUE_MASK];
    // 6. Release tells producers this slot is safely evacuated and ready to re-fill
    atomic_store_explicit(&lf->q->sequence[pos & QUEUE_MASK], pos + QUEUE_NODES, memory_order_release);
    return true;
}
// ----------------------------------------------------------------------
// WORKER
// ----------------------------------------------------------------------
void *worker(void *arg) {
    LfCtx *lf = (LfCtx *)arg;
    QueuePayload dqnode;
    OutputBuffer output = {};
    char full_path[MAX_PATH_LEN];
    alignas(CACHE_LINE_SIZE) char directory_path[MAX_PATH_LEN];
    alignas(struct linux_dirent64) char dir_buf[DIR_BUF_SIZE];
    size_t thread_id = atomic_fetch_add_explicit(&lf->thread_id, 1, memory_order_relaxed);
    LogCtx x_log_ctx = {
        .fd = -1,
        .err_fd = -1,
        .start = 0,
        .prev_nsec = 0,
        .prev_sec = 0,
        .log_level = lf->log_level,
        .timestamp_style = CODER,
    };
    if (lf->log_to_display)
        x_log_ctx.err_fd = STDERR_FILENO;
    timer_start(&x_log_ctx);
    if (lf->log_to_file)
        log_open(&x_log_ctx, lf->thread_id);
    LogCtx *log_ctx = &x_log_ctx;
    while (1) {
        if (mpmc_dequeue(lf, &dqnode, lf->thread_id) == false)
            break;
        if (lf->log_to_display || lf->log_to_file) {
            char log_record[MAX_LOG_REC_LEN];
            ssnprintf(log_record, MAX_LOG_REC_LEN - 1, "thread %lu %s dqnode %12p, <self_idx %6u, parent_idx %6u, offset/len %6lu/%-6lu, path %s",
                      thread_id,
                      "dequeue",
                      &dqnode,
                      dqnode.self_idx,
                      dqnode.dir_ctx->parent_idx != NO_PARENT ? dqnode.dir_ctx->parent_idx : 0,
                      dqnode.dir_ctx->path_offset,
                      dqnode.dir_ctx->path_len,
                      &lf->path_arena->bytes[dqnode.dir_ctx->path_offset]);
            // %lu    thread_id
            // %10s   "dequeue"
            // %12p   &dqnode
            // %6u    dqnode.self_idx
            // %6u    dqnode.dir_ctx->parent_idx
            // %6lu   dqnode.dir_ctx->path_offset
            // %-6lu  dqnode.dir_ctx->path_len
            // %s     &lf->path_arena->bytes[dqnode.dir_ctx->path_offset]
            lf_log(DEBUG, "%s", log_record);
        }
        // ----------------------------------------------------------------
        // STEP 1: Calculate Total Path Length via Index Backtracking
        // ----------------------------------------------------------------
        size_t total_path_len = dqnode.dir_ctx->path_len;
        uint32_t current_parent = dqnode.dir_ctx->parent_idx;
        while (current_parent != NO_PARENT) {
            // Safely resolve the parent out of the global node arena
            DirectoryNode *dir_ctx = lf->queue_arena->nodes[current_parent].dir_ctx;
            total_path_len += dir_ctx->path_len + 1; // +1 for the '/' separator
            current_parent = dir_ctx->parent_idx;
        }
        if (total_path_len >= MAX_PATH_LEN) {
            atomic_fetch_sub_explicit(&lf->q->active_tasks, 1, memory_order_relaxed);
            continue;
        }
        // ----------------------------------------------------------------
        // STEP 2: Reconstruct String Right-to-Left into Local Buffer
        // ----------------------------------------------------------------
        directory_path[total_path_len] = '\0';
        size_t write_cursor = total_path_len;
        // Copy the leaf payload directory first
        write_cursor -= dqnode.dir_ctx->path_len;
        memcpy(&directory_path[write_cursor], &lf->path_arena->bytes[dqnode.dir_ctx->path_offset], dqnode.dir_ctx->path_len);
        current_parent = dqnode.dir_ctx->parent_idx;
        while (current_parent != NO_PARENT) {
            DirectoryNode *dir_ctx = lf->queue_arena->nodes[current_parent].dir_ctx;
            // Add the separator
            write_cursor -= 1;
            directory_path[write_cursor] = '/';
            // Copy parent segment
            write_cursor -= dir_ctx->path_len;
            memcpy(&directory_path[write_cursor], &lf->path_arena->bytes[dir_ctx->path_offset], dir_ctx->path_len);
            current_parent = dir_ctx->parent_idx;
        }
        // ----------------------------------------------------------------
        // STEP 3: Execute Directory Scanning
        // ----------------------------------------------------------------
        // We pass the global index of this node so finder can attach it to children
        finder(lf, &dqnode, &output, dir_buf, directory_path, full_path, thread_id, log_ctx);
        atomic_fetch_sub_explicit(&lf->q->active_tasks, 1, memory_order_relaxed);
    }
    return NULL;
}
// ----------------------------------------------------------------------
// FINDER
// ----------------------------------------------------------------------
/** @brief Process a directory and its entries, enqueueing subdirectories for further searching.
    @param lf A pointer to the LfCtx struct containing the search settings.
    @param dequeued_node A pointer to the QueuePayload struct representing the current directory being processed.
    @param output A pointer to the OutputBuffer struct for buffering output.
    @param dir_buf A buffer for reading directory entries.
    @param current_dir_path A buffer for constructing full file paths.
    @return NULL if processing is complete or an error occurred, otherwise returns a pointer to the next task to be enqueued.
    @details This function reads the contents of the current directory, processes each entry, and applies filters based on the user's settings (e.g., file types, regex patterns). It uses fstatat to retrieve metadata for each entry and determines whether to enqueue subdirectories for further searching. The function also handles symbolic links according to the user's options and manages output buffering for efficient writing to stdout.
   */

void *finder(LfCtx *lf, QueuePayload *dequeued_node, OutputBuffer *output, char *dir_buf, char *current_dir_path, char *full_path, pthread_t thread_id, LogCtx *log_ctx) {
    long nread;
    struct stat sb = {};
    bool stat_cached = false;
    struct linux_dirent64 *entry;
    dev_t actual_dev = 0, effective_dev = 0;
    ino_t actual_ino = 0, effective_ino = 0;
    unsigned char actual_type;
    unsigned char effective_type;
    int rc;
    // Track subdirectories discovered during this syscall iteration
    QueuePayload stage[STAGE_BUFFER_SIZE];
    size_t directory_count = 0;
    size_t stage_buffer_count = 0;

    int dir_fd = open(current_dir_path, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (dir_fd == -1) {
        if (lf->report_errors) {
            lf_log(ERROR, "OPEN_FAIL,%s,%s", current_dir_path, strerror(errno));
        }
        flush_output_buffer(lf, output);
        // NOT_IMPLEMENTED
        // if (lf->unlink)
        //     finalize_directory_node(&dequeued_node->dir_ctx);
        return NULL;
    }

    while (1) {
        nread = syscall(SYS_getdents64, dir_fd, dir_buf, DIR_BUF_SIZE);
        if (nread == -1) {
            atomic_fetch_add(&lf->error_count, 1);
            if (lf->report_errors) {
                lf_log(ERROR, "READDIR_FAIL,%s,%s", current_dir_path, strerror(errno));
            }
            break;
        }
        if (nread == 0)
            break;

        for (size_t bpos = 0; bpos < (size_t)nread;) {
            entry = (struct linux_dirent64 *)(dir_buf + bpos);
            bpos += entry->d_reclen;

            actual_type = entry->d_type;
            effective_type = actual_type;
            // -------------------------------------------------------------------
            // PROCESS DIRECTORY ENTRIES
            // -------------------------------------------------------------------
            if (actual_type == DT_DIR) {
                if (entry->d_name[0] == '.') {
                    if (entry->d_name[1] == '\0')
                        continue;
                    if (entry->d_name[1] == '.' && entry->d_name[2] == '\0')
                        continue;
                }
            }
            size_t current_dir_len = strlen(current_dir_path);
            size_t entry_name_len = strlen(entry->d_name);
            memcpy(full_path, current_dir_path, current_dir_len);
            full_path[current_dir_len] = '/';
            memcpy(full_path + current_dir_len + 1, entry->d_name, entry_name_len);
            size_t full_path_len = current_dir_len + 1 + entry_name_len;
            full_path[full_path_len] = '\0';

            // -------------------------------------------------------------------
            // GET ADVANCED METADATA
            // -------------------------------------------------------------------
            if (((lf->follow_links || lf->report_badlinks) &&
                 (actual_type == DT_LNK || actual_type == DT_DIR)) ||
                actual_type == DT_UNKNOWN) {
                memset(&sb, 0, sizeof(struct stat));
                rc = fstatat(dir_fd, entry->d_name, &sb, AT_SYMLINK_NOFOLLOW);
                if (rc == -1) {
                    atomic_fetch_add(&lf->error_count, 1);
                    if (lf->report_badlinks) {
                        lf_log(ERROR, "LSTAT_FAIL,%s/%s,%s", current_dir_path, entry->d_name, strerror(errno));
                    }
                } else {
                    actual_ino = sb.st_ino;
                    actual_dev = sb.st_dev;
                    actual_type = (sb.st_mode & S_IFMT) >> 12;
                    effective_dev = actual_dev;
                    effective_ino = actual_ino;
                    effective_type = actual_type;
                }
                if (S_ISLNK(sb.st_mode)) {
                    if (lf->follow_links) {
                        // -------------------------------------------------------
                        // GET LINK METADATA
                        // -------------------------------------------------------
                        rc = fstatat(dir_fd, entry->d_name, &sb, 0);
                        if (rc == -1) {
                            atomic_fetch_add(&lf->error_count, 1);
                            if (lf->report_errors) {
                                lf_log(ERROR, _("STAT_FAIL,%s,%s\n"), full_path, strerror(errno));
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
            // --------------------------------------------------------------------
            // DIRECTORY
            // --------------------------------------------------------------------
            if (effective_type == DT_DIR) {
                uint16_t next_depth = dequeued_node->dir_ctx->depth + 1;
                if (lf->max_depth > 0 && next_depth > lf->max_depth)
                    continue;
                bool cycle_found = false;
                // ----------------------------------------------------------------
                // CYCLE DETECTION (Index-Based)
                // ----------------------------------------------------------------
                if (lf->follow_links && actual_type == DT_LNK) {
                    char target_path[MAX_PATH_LEN] = {'\0'};
                    if (lf->report_trace || lf->report_badlinks) {
                        ssize_t len = readlinkat(dir_fd, entry->d_name, target_path, sizeof(target_path) - 1);
                        if (len != -1)
                            target_path[len] = '\0';
                    }
                    if (lf->report_trace) {
                        ssnprintf(lf->log_record, MAX_LOG_REC_LEN - 1,
                                  "Target item: dev: %lu, ino: %lu, Name: %s/%s\n",
                                  effective_dev, effective_ino, current_dir_path, entry->d_name);
                        lf_log(INFO, "%s", lf->log_record);
                    }
                    // ------------------------------------------------------------
                    // BACKTRACK TO CHECK FOR CYCLE
                    // ------------------------------------------------------------
                    uint32_t current_parent_idx = dequeued_node->self_idx;
                    while (current_parent_idx != NO_PARENT) {
                        // Resolve the parent node safely from the stable global arena
                        QueuePayload *ancestor = &lf->queue_arena->nodes[current_parent_idx];
                        if (lf->report_trace) {
                            ssnprintf(lf->log_record,
                                      MAX_LOG_REC_LEN - 1,
                                      "Checking Ancestor Index [%u] -> dev: %lu, ino: %lu\n",
                                      current_parent_idx,
                                      ancestor->cycle_ctx->dev,
                                      ancestor->cycle_ctx->ino);
                            lf_log(TRACE, "%s", lf->log_record);
                        }
                        // If both the device and inode match an ancestor, we
                        // found a cycle loop!
                        if (effective_dev == ancestor->cycle_ctx->dev && effective_ino == ancestor->cycle_ctx->ino) {
                            cycle_found = true;
                            break;
                        }
                        current_parent_idx = ancestor->cycle_ctx->parent_idx;
                    }
                    if (cycle_found) {
                        if (lf->report_badlinks) {
                            ssnprintf(lf->log_record, MAX_LOG_REC_LEN - 1, ("CYCLER,%lu/%lu,%s/%s,%lu/%lu,%s\n"),
                                      effective_dev,
                                      effective_ino,
                                      current_dir_path,
                                      entry->d_name,
                                      effective_dev, effective_ino, target_path);
                            lf_log(ERROR, "%s", lf->log_record);
                        }
                        atomic_fetch_add(&lf->error_count, 1);
                        print_file(lf, effective_type, &sb, stat_cached, output, full_path);
                        continue;
                    }
                }
                // ----------------------------------------------------------------
                // STAGE CHILD TASKS
                // ----------------------------------------------------------------
                if (stage_buffer_count >= STAGE_BUFFER_SIZE) {
                    flush_staged_children(lf, stage, stage_buffer_count, thread_id, log_ctx);
                    stage_buffer_count = 0;
                }
                size_t child_offset = path_arena_alloc(lf, entry->d_name, entry_name_len);
                // 1. Reserve a permanent slot in the node arena for child
                size_t global_node_idx = atomic_fetch_add_explicit(&lf->queue_arena->cursor, 1, memory_order_relaxed);
                // 2. Create child task
                QueuePayload *child_task = &stage[stage_buffer_count++];
                directory_count++;
                child_task->self_idx = (uint32_t)global_node_idx;
                child_task->dir_ctx = directory_arena_alloc(lf, child_offset, entry_name_len, next_depth, NO_PARENT);
                child_task->dir_ctx->parent_idx = dequeued_node->self_idx;
                child_task->cycle_ctx = cycle_arena_alloc(lf, effective_dev, effective_ino, dequeued_node->self_idx);
                // 3. Write child task into the global arena so future
                // generations can resolve it
                lf->queue_arena->nodes[global_node_idx] = *child_task;
                if ((lf->log_to_display || lf->log_to_file) && DEBUG_CHILD_STAGING) {
                    ssnprintf(lf->log_record, MAX_LOG_REC_LEN - 1, "%3lu >%10s %12p %6u %6u %6lu/%-6lu %s",
                              thread_id,
                              "child_task",
                              (void *)&child_task,
                              child_task->self_idx,
                              child_task->dir_ctx->parent_idx,
                              child_task->dir_ctx->path_offset,
                              child_task->dir_ctx->path_len,
                              &lf->path_arena->bytes[child_task->dir_ctx->path_offset]);

                    lf_log(DEBUG, "%s", lf->log_record);
                }
                // --------------------------------------------------------
                // CONDITIONALLY PRINT AS DIRECTORY
                // --------------------------------------------------------
                if (entry->d_name[0] == '.') {
                    if (!lf->include_hidden && !lf->hidden_only)
                        continue;
                } else {
                    if (lf->hidden_only)
                        continue;
                }
                if (!lf->only_errors)
                    print_file(lf, effective_type, &sb, stat_cached, output, full_path);
            } else {
                // --------------------------------------------------------
                // CONDITIONALLY PRINT AS FILE
                // --------------------------------------------------------
                if (entry->d_name[0] == '.') {
                    if (!lf->include_hidden && !lf->hidden_only)
                        continue;
                } else {
                    if (lf->hidden_only)
                        continue;
                }
                if (full_path[0] == '\0')
                    continue;
                if (!lf->only_errors)
                    print_file(lf, effective_type, &sb, stat_cached, output, full_path);
            }
        }
    }
    close(dir_fd);
    flush_output_buffer(lf, output);
    if (stage_buffer_count > 0)
        flush_staged_children(lf, stage, stage_buffer_count, thread_id, log_ctx);
    atomic_store_explicit(
        &dequeued_node->dir_ctx->child_dirs_remaining,
        directory_count, memory_order_release);

    // atomic_store_explicit(
    //     &lf->queue_arena->nodes[dequeued_node->self_idx].dir_ctx->child_dirs_remaining,
    //     directory_count, memory_order_release);

    if ((lf->unlink || lf->depth_first) && directory_count == 0) {
        // --------------------------------------------------------------------
        // BOTTOM-UP PROCESSING
        // --------------------------------------------------------------------
        uint32_t current_idx = dequeued_node->self_idx;
        while (current_idx != NO_PARENT) {
            QueuePayload *node = &lf->queue_arena->nodes[current_idx];
            // IF ACTIVE CHILDREN, BREAK
            if (atomic_fetch_sub_explicit(&node->dir_ctx->child_dirs_remaining, 1, memory_order_acq_rel) > 1)
                break;
            // This directory and all its subdirectories are 100% processed.
            // You can safely process or remove the directory here.
            // Cascade up to the parent index
            current_idx = node->dir_ctx->parent_idx;
        }
    }
    return NULL;
}

void flush_staged_children(LfCtx *lf, QueuePayload *stage, size_t stage_buffer_count, size_t thread_id, LogCtx *log_ctx) {
    char var_str[MAXLEN];
    if (stage_buffer_count > 0) {
        // Increment the global active task pool for the incoming children
        atomic_fetch_add_explicit(&lf->q->active_tasks, stage_buffer_count, memory_order_relaxed);
        // --------------------------------------------------------------------
        // mpmc_enqueue
        // --------------------------------------------------------------------
        // Push staged directory payloads smoothly to ring nodes buffer
        for (size_t i = 0; i < stage_buffer_count; ++i) {
            //  if (mpmc_bulk_enqueue(lf, stage, stage_buffer_count)) {
            //    SUCCESS: The slots are safely inside the queue structure.
            //    Publish the count so the bottom-up cleaner can track it.
            //    atomic_store_explicit(&parent_node->child_dirs_remaining,
            //    stage_buffer_count, memory_order_release);
            //  } else {
            // FALLBACK: Queue is full. Because we haven't touched
            // parent_node->child_dirs_remaining or modified the queue,
            // we can safely handle these elements manually or process this
            // thread's backlog using a thread-local overflow list without
            // breaking our bottom-up counter tree.
            //      handle_queue_overflow_fallback(stage, stage_buffer_count);
            //  }
            if (lf->log_to_display || lf->log_to_file) {
                ssnprintf(lf->log_record, MAX_LOG_REC_LEN - 1, "%3lu >%10s %12p %6u %6u %6lu/%-6lu %s",
                          thread_id,
                          var_str,
                          (void *)&stage[i],
                          stage[i].self_idx,
                          stage[i].dir_ctx->parent_idx,
                          stage[i].dir_ctx->path_offset,
                          stage[i].dir_ctx->path_len,
                          &lf->path_arena->bytes[stage[i].dir_ctx->path_offset]);
                lf_log(DEBUG, "%s", lf->log_record);
            }
            if (!mpmc_enqueue(lf, &stage[i], thread_id, log_ctx)) {
                // If an enqueue ever fails due to buffer overflow limits,
                // handle it or drop safely while decrementing counters
                atomic_fetch_sub_explicit(&lf->q->active_tasks, 1, memory_order_relaxed);
            }
        }
    } else {
        // NOT_IMPLEMENTED
        // if (lf->unlink)
        //     finalize_directory_node(&dequeued_node->dir_ctx);
    }
}
// ----------------------------------------------------------------------
// FINALIZE_DIRECTORY_NODE
// ----------------------------------------------------------------------
// void finalize_directory_node(LfCtx *lf, size_t node_offset) {
//     while (node_offset != ROOT_MARKER) {
//         DirectoryNode *node = (DirectoryNode
//         *)&lf->path_arena->bytes[node_offset];
//
// If this node still has active children running, stop bubbling up
//         if (atomic_fetch_sub_explicit(&node->child_dirs_remaining, 1,
//         memory_order_acq_rel) > 1) {
//             break;
//         }

// --- SUCCESS: This directory is now completely empty! ---
// Reconstruct its absolute path safely and execute your post-order processing:
// rmdir(absolute_path);

// Bubble up to this folder's parent
//         node_offset = node->parent_node_offset;
//     }
// }
// ----------------------------------------------------------------------
// PRINT_FILE
// ----------------------------------------------------------------------
/** @brief Scan a file or directory and apply filters based on the user's settings.
    @param dir_path The path to the file or directory to scan.
    @param path_len A pointer to the length of the dir_path string.
    @param lf A pointer to the LfCtx struct containing the search settings.
    @param effective_type The effective type of the file (e.g., DT_REG, DT_DIR).
    @param cached_sb A pointer to a struct stat containing cached metadata for the file.
    @param stat_cached A boolean indicating whether the cached_sb contains valid data.
    @param output A pointer to the OutputBuffer struct for buffering output.
    @return true if the file was successfully scanned and processed, false if an error occurred or if the file was excluded by filters.
    @details This function applies various filters to determine whether a given file or directory should be included in the output. It checks for matching and non-matching regex patterns, user ownership, permissions, modification times, and file sizes. If the file passes all filters, it is added to the output buffer. The function also handles caching of file metadata to avoid redundant system calls.
   */
int print_file(LfCtx *lf, const unsigned char effective_type, struct stat *cached_sb, bool stat_cached, OutputBuffer *output, char *full_path) {
    size_t full_path_len = strlen(full_path);
    while (1) {
        if (lf->suppress_types & lf_mask[effective_type])
            break;
        // Exclude non-matching files
        if (lf->flags & LF_REGEX) {
            int reti =
                regexec(&lf->compiled_re, full_path, 0, NULL, 0);
            if (reti == REG_NOMATCH)
                break;
            termination_status |= TS_MATCH;
        }
        // Exclude matching files
        if (lf->flags & LF_EXC_REGEX) {
            int reti =
                regexec(&lf->compiled_ere, full_path, 0, NULL, 0);
            if (reti == 0) {
                termination_status |= TS_MATCH;
                break;
            }
        }
        //  Exclude files not owned by specified user
        if (lf->flags & LF_USER) {
            if (!stat_cached && stat(full_path, cached_sb) == 0)
                stat_cached = true;
            if (!stat_cached || cached_sb->st_uid != lf->user_id)
                break;
        }
        if (lf->include_perms) {
            if (!stat_cached && stat(full_path, cached_sb) == 0)
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
            if (!stat_cached && stat(full_path, cached_sb) == 0)
                stat_cached = true;
            if (stat_cached && cached_sb->st_mtime > lf->before)
                break;
        }
        if (lf->after) { // Last file modification
            if (!stat_cached && stat(full_path, cached_sb) == 0)
                stat_cached = true;
            if (stat_cached && cached_sb->st_mtime < lf->after)
                break;
        }
        if (lf->file_size_min) {
            if (!stat_cached && stat(full_path, cached_sb) == 0)
                stat_cached = true;
            if (stat_cached && cached_sb->st_size < lf->file_size_min)
                break;
        }
        if (lf->only_errors)
            break;
        atomic_fetch_add(&lf->file_count, 1);
        if (!lf->count_silently) {
            full_path_len = strlen(full_path);
            if (full_path_len > 2 && full_path[0] == '.' && full_path[1] == '/') {
                full_path += 2;
                full_path_len -= 2;
            }
            if (full_path_len > 0) {
                append_output_buffer(lf, output, full_path, full_path_len,
                                     effective_type == DT_DIR);
            }
            break;
        }
    }
    return true;
}

// -----------------------------------------------------------------------
// Logging
// -----------------------------------------------------------------------

long timer_start(LogCtx *log_ctx) {
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    log_ctx->prev_nsec = tp.tv_nsec;
    log_ctx->prev_sec = tp.tv_sec;
    log_ctx->start = tp.tv_sec;
    return tp.tv_nsec;
}

long timer_mark(LogCtx *log_ctx, char *ts_buf) {
    struct timespec tp;

    clock_gettime(CLOCK_MONOTONIC, &tp);
    long elapsed = tp.tv_nsec - log_ctx->start;
    long interval = tp.tv_nsec - log_ctx->prev_nsec;
    log_ctx->prev_nsec = tp.tv_nsec;
    long ms = (elapsed / 1000000) % 1000;
    long us = (elapsed / 1000) % 1000;
    long ns = elapsed % 1000;
    ssnprintf(ts_buf, 64, "[%s.%03ld_%03ld_%03ld%s]", log_c[INFO], ms, us, ns, sgr_c[RESET]);
    size_t len = strlen(ts_buf);
    ms = (interval / 1000000) % 1000;
    us = (interval / 1000) % 1000;
    ns = interval % 1000;
    ssnprintf(&ts_buf[len + 1], 64, "[%s.%03ld_%03ld_%03ld%s]", sgr_c[YELLOW], ms, us, ns, sgr_c[RESET]);
    return interval;
}

long timer_elapsed(LogCtx *log_ctx, int flag, char *ts_buf) {
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    long elapsed = tp.tv_nsec - log_ctx->start;
    log_ctx->prev_nsec = tp.tv_nsec;
    if (flag == TM_RESET) {
        log_ctx->start = tp.tv_nsec;
        log_ctx->prev_nsec = tp.tv_nsec;
    }
    long ms = (elapsed / 1000000) % 1000;
    long us = (elapsed / 1000) % 1000;
    long ns = elapsed % 1000;
    ssnprintf(ts_buf, 64, "%s%03ld_%03ld_%03ld%s", sgr_c[YELLOW], ms, us, ns, sgr_c[RESET]);
    return elapsed;
}

/** iso8601_timestamp
    @brief Generate an ISO 8601 timestamp string
    @ingroup logging
    @param buf Buffer to store the timestamp string
    @param n Size of the buffer
    @param local If true, use local time; if false, use UTC
    @return Pointer to the buffer containing the timestamp string */
void iso8601_timestamp(LogCtx *log_ctx) {
    time_t tm = time(NULL);
    struct tm *tp = log_ctx->local ? localtime(&tm) : gmtime(&tm);
    if (log_ctx->local)
        strftime(log_ctx->buf, 64, _("%Y-%m-%dT%H:%M:%S%z"), tp);
    else
        strftime(log_ctx->buf, 64, _("%Y-%m-%dT%H:%M:%SZ"), tp);
    return;
}

void coder_timestamp(LogCtx *log_ctx) {
    char secs_buf[6];
    char buf2[64];
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    struct tm *tm = gmtime(&tp.tv_sec);
    strftime(secs_buf, sizeof(secs_buf), "%S", tm);
    long ms = tp.tv_nsec / 1000000;
    long us = (tp.tv_nsec / 1000) % 1000;
    long ns = tp.tv_nsec % 1000;
    ssnprintf(log_ctx->buf, 64, "[%s.%s%03ld_%03ld_%03ld%s]", secs_buf, sgr_c[YELLOW], ms, us, ns, sgr_c[RESET]);
    long interval_secs = tp.tv_sec - log_ctx->prev_sec;
    long interval_nsecs = tp.tv_nsec - log_ctx->prev_nsec;
    ms = interval_nsecs / 1000000;
    us = (interval_nsecs / 1000) % 1000;
    ns = interval_nsecs % 1000;
    ssnprintf(buf2, 64, "[%02ld.%s%03ld_%03ld_%03ld%s]", interval_secs, sgr_c[YELLOW], ms, us, ns, sgr_c[RESET]);
    strnz__cat(log_ctx->buf, buf2, 64);
    log_ctx->prev_nsec = tp.tv_nsec;
    log_ctx->prev_sec = tp.tv_sec;
    return;
}
/** log_open
    @brief Open the log file for writing
    @ingroup logging
    @return FILE pointer to the opened log file */
void log_open(LogCtx *log_ctx, size_t thread_id) {
    char log_record[MAXLEN];
    if (log_ctx->fd == -1) {
        char *user = getenv("USER");
        time_t t = time(NULL);
        struct tm tp;
        localtime_r(&t, &tp);
        char time_s[16];
        strftime(time_s, 100, "%Y%m%d%H%M%S", &tp);
        ssnprintf(log_ctx->file_spec, MAXLEN - 1, "/tmp/%s_lf_%s.log", user, time_s);
        log_ctx->fd = open(log_ctx->file_spec, O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (log_ctx->fd == -1) {
            ssnprintf(em0, MAXLEN - 1, _("Failed to open log file: %s"), log_ctx->file_spec);
            ssnprintf(em1, MAXLEN - 1, _("Error: %s"), strerror(errno));
            ssnprintf(em2, MAXLEN - 1, "%s: line: %d", __FILE__, __LINE__ - 4);
            ssnprintf(em3, MAXLEN - 1, _("remedy: delete %s and try again"), log_ctx->file_spec);
            fprintf(stderr, "%s\n%s\n%s\n%s\n", em0, em1, em2, em3);
            exit(EXIT_FAILURE);
        }
    }
    char ttyname[MAXLEN];
    char cmenu_user[MAXLEN];
    char *p = getenv(_("USER"));
    strnz__cpy(cmenu_user, p, MAXLEN - 1);
    if (ttyname_r(STDERR_FILENO, ttyname, sizeof(ttyname)) == 0)
        strnz__cpy(log_record, ttyname, MAXLEN - 1);
    ssnprintf(log_record, MAXLEN - 1, _("Lf_Log started by user %s, thread %lu, on terminal '%s'"), cmenu_user, thread_id, ttyname);
    lf_log(INFO, "%s:", log_record);
    return;
}

void log_write(const LfLogLevel level, LogCtx *log_ctx, const char *file, const char *func, const int line, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    if (log_ctx->fd == -1)
        log_open(log_ctx, log_ctx->thread_id);
    LfLogLevel safe_level = (level >= LOG_LEVEL_COUNT) ? INFO : level;
    struct stat st;
    if (log_ctx->fd == -1 || (stat(log_ctx->file_spec, &st) == 0 && st.st_size >= MAX_LOG_SIZE)) {
        if (log_ctx->fd != -1) {
            close(log_ctx->fd);
            log_ctx->fd = -1;
        }
        char old_archive[MAXLEN];
        strnz__cpy(old_archive, log_ctx->file_spec, MAXLEN - 1);
        strnz__cat(old_archive, ".closed", MAXLEN - 1);
        rename(log_ctx->file_spec, old_archive);
        log_open(log_ctx, log_ctx->thread_id);
    }
    switch (log_ctx->timestamp_style) {
    case ISO8601_ZULU:
        iso8601_timestamp(log_ctx);
        break;
    case ISO8601_LOCAL:
        iso8601_timestamp(log_ctx);
        break;
    case CODER:
        coder_timestamp(log_ctx);
        break;
    case TIMESTAMP_STYLE_COUNT:
        break;
    }
    char log_record[MAX_LOG_REC_LEN];
    ssnprintf(log_record, sizeof(log_record), "%s[%s%s%s]<%s:%s:%d> ",
              log_ctx->buf,
              log_c[safe_level],
              log_s[safe_level],
              ANSI_RESET,
              file, func, line);
    size_t len = strlen(log_record);
    vsnprintf(log_record + len, sizeof(log_record) - len, fmt, args);
    len = strlen(log_record);
    log_record[len++] = '\n';
    if (write(log_ctx->fd, log_record, len) == -1)
        perror("Failed to write atomic log record");
    if (log_ctx->err_fd != -1)
        write(log_ctx->err_fd, log_record, len);
    va_end(args);
    return;
}
