/** ANNOUNCEMENT: This file is part of the lf project, which is currently being
 * tested in anticipation of public release. The test suite consists of a test
 * script, lf_tests.sh, which uses diff to compare the output with find. There
 * is also an accompanying markdown file, lf_tests.md, that outlines the testing
 * methodology, cases, and expected results.
 *
 * As a design choice, lf segregates directory entries with fatal errors,
 * meaning those that do not provide functionality conforming to known
 * standards.
 *
 * Your feedback and suggestions are always welcome.
 *
 * Feel free to run the test script on your own system, and if you encounter any
 * issues, please report them to the author.
 */

/** @file lf4.c
    @brief list files matching a regular expression
    @author Bill Waller
    Copyright (c) 2025
    MIT License
    billxwaller@gmail.com
    @date 2026-02-09
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
#define DIR_BUF_SIZE 262144
#define CACHE_LINE_SIZE 64

typedef struct DevIno {
    // dev_t dev;
    //  ino_t ino;
    // struct DevIno *parent;
} DevIno;

typedef struct QueuePayload QueuePayload;
struct QueuePayload {
    size_t path_len;
    uint16_t depth;
    char path[MAX_PATH_LEN];
    dev_t dev;
    ino_t ino;
    QueuePayload *parent;
};

typedef struct {
    alignas(CACHE_LINE_SIZE) _Atomic size_t enqueue_pos;
    alignas(CACHE_LINE_SIZE) _Atomic size_t dequeue_pos;
    alignas(CACHE_LINE_SIZE) _Atomic atomic_int active_tasks;
    alignas(CACHE_LINE_SIZE) _Atomic atomic_int shut_down;
    alignas(CACHE_LINE_SIZE) _Atomic size_t sequence[QUEUE_CAPACITY];
    QueuePayload nodes[QUEUE_CAPACITY];
    DevIno dev_ino[QUEUE_CAPACITY];
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
const char doc[] = "lf list files\vIf specified, DIRECTORY is the top-level "
                   "directory to search. REGULAR_EXPRESSION is a properly "
                   "formatted regular expression for which matching files "
                   "will be listed.";
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
void sort_lf_output(LfContext *lf, int, char **);
MPMCQueue *mpmc_queue_init();
bool mpmc_enqueue(LfContext *, const QueuePayload *child_node);
QueuePayload *mpmc_dequeue(LfContext *);
void *worker(void *arg);
void *finder(LfContext *lf, QueuePayload *current_node, QueuePayload *child_node);
int scan_file(const char *file_spec, const size_t *path_len, LfContext *lf, const unsigned char,
              const struct stat *, OutputBuffer *);
bool build_full_path(char *, size_t, const char *, const char *, size_t *);
void flush_output_buffer(LfContext *, OutputBuffer *);
bool append_output_buffer(LfContext *, OutputBuffer *, const char *, size_t,
                          bool);
int err_out(LfContext *lf, const char *format, ...);
// ---------------------------------------------------------------

static struct argp_option options[] = {
    {"after", 'a', "time", 0, "Last Modified after YYYY-MM-DDTHH:MM:SS", 0},
    {"before", 'b', "time", 0, "Last Modified before YYYY-MM-DDTHH:MM:SS", 0},
    {"max_depth", 'd', "number", 0, "Depth into directory tree", 0},
    {"error_file_spec", 'E', "file_spec", 0, "Error message output file", 0},
    {"ere", 'e', "regex", 0, "Exclude regular expression", 0},
    {"ignore_case", 'i', 0, 0, "Search ignore case", 0},
    {"include_perms", 'p', "sgrwx", 0,
     "x-execute, w-write, r-read, s-setuid, g-setgid", 0},
    {"re", 'r', "regex", 0, "Regular expression to search for", 0},
    {"include_types", 't', "pcdbflsu", 0,
     "p-pipe, c-character_dev, d-directory, b-block_dev, f-regular_file, l-link, s-socket, u-unknown",
     0},
    {"file_size_min", 's', "size", 0,
     "No Suffix-bytes, K-kilobytes, M-Megabytes, or G-Gigabytes", 0},
    {"user", 'u', "user name", 0, "User Name of file owner ", 0},
    {"debug", 'D', "12345678", 0,
     "1-config, 2-info, 3-warnings, 4-errors, 5-badlinks, 6-trace, 7-all, "
     "8-only_errors, 9-report_error_count",
     0},
    {"include_hidden", 'H', "o", OPTION_ARG_OPTIONAL, "Include hidden files (o=hidden only)", 0},
    {"follow_links", 'L', 0, 0, "Follow symbolic links", 0},
    {"sort_reverse", 'R', 0, 0, "Sort in Reverse order", 0},
    {"sort", 'S', 0, 0, "Sort in Ascending order", 0},
    {"nthreads", 'T', "threads", 0, "Number of nthreads", 0},
    {"count", 'c', "s", OPTION_ARG_OPTIONAL, "Count (s only report count)", 0},
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
            fprintf(stderr, "-b time must be greater than -a time.\n");
            lf->after = 0;
        }
        break;
    case 'b':
        parse_local_timestamp(arg, &lf->before);
        if (lf->after && lf->before && lf->before < lf->after) {
            fprintf(stderr, "-b time must be greater than -a time.\n");
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
            fprintf(stderr, "User '%s' not found.\n", arg);
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
    lf->sbuffer = "10%";
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
                "lf: arg1: '%s' is neither a directory nor a valid regex.\n",
                lfargs[0]);
            exit(termination_status);
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
                    "lf: '%s' is neither a directory nor a valid regular "
                    "expression.\n",
                    lfargs[1]);
            exit(termination_status);
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
        sort_lf_output(lf, argc, argv);

    if (lf->count) {
        size_t count = atomic_load(&lf->file_count);
        fprintf(stderr, "Files: %zu\n", count);
    }
    atomic_load(&lf->error_count);
    if (lf->report_error_count && lf->error_count > 0) {
        fprintf(stderr, "Errors: %zu\n", lf->error_count);
        termination_status |= TS_ERROR;
    }
    free(lf);
    exit(termination_status);
}
/** @brief Sort the output of the lf command using the system's sort utility.
    @param lf A pointer to the LfContext struct containing the search settings.
    @param argc The number of command-line arguments.
    @param argv An array of command-line argument strings.
    @details This function sets up a pipeline to sort the output of the lf command. It creates a pipe, forks a child process to execute the sort command, and redirects the standard input and output streams accordingly. The sort command is executed with optional flags for parallel processing and buffer size, as well as a reverse sort option if specified in the LfContext. The function waits for the child process to complete before returning.
   */
void sort_lf_output(LfContext *lf, int argc, char **argv) {
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
        // Child's STDIN will be redirected to the read end of the pipe, so that
        // the sort process will read the output of the finder from the pipe.
        close(fds[1]);              // Close the write end of the pipe in the child
        dup2(fds[0], STDIN_FILENO); // Clone child's read pipe to STDIN_FILENO
        close(fds[0]);              // Close the original read end of the pipe
        execvp(eargv[0], eargv);    // Execute the sort command
        fprintf(stderr, "Failed to execute sort: %s\n", strerror(errno));
        exit(termination_status);
    }
    // fclose(stdout);
    dup2(fds[1], STDOUT_FILENO);         // Clone write pipe to STDOUT_FILENO
    stdout = fdopen(STDOUT_FILENO, "w"); // Reopen STDOUT as a stream
    setvbuf(stdout, NULL, _IOLBF, 0);    //  line buffering
    init_lf(lf, argc, argv);             // Initialize and transfer control to the finder
    fclose(stdout);
    close(fds[1]);
    wait(&wstatus);
    for (int i = 0; i < eargc; i++)
        free(eargv[i]);
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
            fprintf(stderr, "lf: '%s' Invalid pattern\n", lf->re);
            regfree(&lf->compiled_re);
            return false;
        }
    }
    if (lf->flags & LF_EXC_REGEX) {
        reti = regcomp(&lf->compiled_ere, lf->ere, lf->reg_flags);
        if (reti) {
            fprintf(stderr, "lf: '%s' Invalid exclude pattern\n", lf->ere);
            regfree(&lf->compiled_ere);
            return false;
        }
    }
    debug_out(lf, argc, argv);
    //--------------------------------------------------------------------
    // Create and enqueue the first QueuePayload
    termination_status = TS_SUCCESS;
    int rc = 0;
    struct stat st;
    rc = pthread_mutex_init(&lf->output_mutex, NULL);
    if (rc != 0)
        perror("Mutex initialization failed");
    if (stat(lf->base_path, &st) == 0) {
        lf->q = mpmc_queue_init();
        QueuePayload child_node;
        if (S_ISDIR(st.st_mode)) {
            child_node.path_len = strnz__cpy(child_node.path, lf->base_path, MAX_PATH_LEN - 1);
            child_node.depth = 0;
            child_node.dev = st.st_dev;
            child_node.ino = st.st_ino;
            child_node.parent = NULL;
            if (!mpmc_enqueue(lf, &child_node)) {
                fprintf(stderr, "Failed to enqueue initial directory\n");
                return false;
            }
            atomic_fetch_add_explicit(&lf->q->active_tasks, 1, memory_order_relaxed);
            //------------------------------------------------------------
            // INITIALIZE THREADS
            //------------------------------------------------------------
            lf->threads = calloc(lf->nthreads, sizeof(pthread_t));
            if (!lf->threads) {
                fprintf(stderr, "Out of memory allocating threads\n");
                return false;
            }
            for (unsigned int i = 0; i < lf->nthreads; i++) {
                rc = pthread_create(
                    &lf->threads[i],
                    NULL,
                    worker,
                    lf);
                if (rc != 0) {
                    fprintf(stderr, "Error: Unable to create thread %d\n", rc);
                    lf->q->shut_down = 1;
                    termination_status = TS_ERROR;
                    return false;
                }
            }
            for (unsigned int i = 0; i < lf->nthreads; i++)
                pthread_join(lf->threads[i], NULL);
            return true;
        } else {
            fprintf(stderr,
                    "Warning: Base path '%s' is not a directory. No "
                    "files will be found.\n",
                    lf->base_path);
            termination_status = TS_ERROR;
            return false;
        }
    }
    // ------------------------------------------------------------------
    // END PROGRAM
    // ------------------------------------------------------------------
    rc = pthread_mutex_destroy(&lf->output_mutex);
    if (rc != 0)
        perror("Mutex destroy failed");
    if (lf->flags & LF_REGEX) {
        regfree(&lf->compiled_re);
    }
    if (lf->flags & LF_EXC_REGEX) {
        regfree(&lf->compiled_ere);
    }
    free(lf->q);
    free(lf->base_path);
    free(lf->user_name);
    free(lf->re);
    free(lf->ere);
    free(lf->threads);
    if (reti)
        return false;
    return true;
}
// ----------------------------------------------------------------------
// DEBUG_OUT
// ----------------------------------------------------------------------
/** @brief Output debug information to stderr.
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
        fprintf(stderr, "lf debug      %s\n",
                lf->debug ? "true" : "     false");
        fprintf(stderr, "  1-config      %s\n",
                lf->report_config ? "true" : "|    false");
        fprintf(stderr, "  2-info        %s\n",
                lf->report_info ? "true" : "|    false");
        fprintf(stderr, "  3-warnings    %s\n",
                lf->report_warnings ? "true" : "|    false");
        fprintf(stderr, "  4-errors      %s\n",
                lf->report_errors ? "true" : "|    false");
        fprintf(stderr, "  5-badlinks    %s\n",
                lf->report_trace ? "true" : "|    false");
        fprintf(stderr, "  6-trace       %s\n",
                lf->report_trace ? "true" : "|    false");
        fprintf(stderr, "  8-only_errors %s\n",
                lf->only_errors ? "true" : "|    false");
        fprintf(stderr, "\n");
        fprintf(stderr, "Count files: %s\n", lf->count ? "true" : "false");
        fprintf(stderr, "Count only: %s\n", lf->count_silently ? "true" : "false");
        fprintf(stderr, "\n");
        fprintf(stderr, "Search directory: %s\n\n", lf->base_path);
        if (lf->max_depth == 0)
            fprintf(stderr, "Max depth 0 (unlimited)\n\n");
        else
            fprintf(stderr, "Max depth %d\n\n", lf->max_depth);
        fprintf(stderr, "Using %d threads\n\n", lf->nthreads);
        fprintf(stderr, "File types preceeded by an asterisk (\"*\") will be included:\n\n");
        fprintf(stderr, "  LF type        DT type\n");
        print_file_type(lf->include_types, LF_FIFO, DT_FIFO, "FIFO    p-named pipe");
        print_file_type(lf->include_types, LF_CHR, DT_CHR, "CHR     c-character device");
        print_file_type(lf->include_types, LF_DIR, DT_DIR, "DIR     d-directory");
        print_file_type(lf->include_types, LF_BLK, DT_BLK, "BLK     b-block device");
        print_file_type(lf->include_types, LF_REG, DT_REG, "REG     f-regular file");
        print_file_type(lf->include_types, LF_LNK, DT_LNK, "LINK    l-symbolic link");
        print_file_type(lf->include_types, LF_SOCK, DT_SOCK, "SOCK    s-socket");
        print_file_type(lf->include_types, LF_UNKNOWN, DT_UNKNOWN, "UNKNOWN u-unknown");
        fprintf(stderr, "\n");
        fprintf(stderr, "f->include_types  = %08b\n", lf->include_types);
        fprintf(stderr, "f->suppress_types = %08b\n", lf->suppress_types);
        fprintf(stderr, "\n");
        if (lf->flags & LF_USER)
            fprintf(stderr, "User: %s (%ju)\n", lf->user_name, lf->user_id);
        if (lf->include_perms) {
            if (lf->include_perms & LF_IXUSR)
                fprintf(stderr, "    %08b Execute\n", LF_IXUSR);
            if (lf->include_perms & LF_IWUSR)
                fprintf(stderr, "    %08b Write\n", LF_IWUSR);
            if (lf->include_perms & LF_IRUSR)
                fprintf(stderr, "    %08b Read\n", LF_IRUSR);
            if (lf->include_perms & LF_ISUID)
                fprintf(stderr, "    %08b SETUID\n", LF_ISUID);
            if (lf->include_perms & LF_ISGID)
                fprintf(stderr, "    %08b SETGID\n", LF_ISGID);
        }
        fprintf(stderr, "\n");
        if (lf->flags & LF_REGEX)
            fprintf(stderr, "Include regex: %s\n\n", lf->re);
        if (lf->flags & LF_EXC_REGEX)
            fprintf(stderr, "Exclude regex: %s\n\n", lf->ere);

        if (lf->after) {
            char buf[32];
            format_local_timestamp(lf->after, buf, sizeof(buf));
            fprintf(stderr, "Modified after: %s\n\n", buf);
        }

        if (lf->before) {
            char buf[32];
            format_local_timestamp(lf->before, buf, sizeof(buf));
            fprintf(stderr, "Modified before: %s\n\n", buf);
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
            fprintf(stderr, "Minimum file size: %s\n\n", buffer);
        }
        if (lf->max_depth)
            fprintf(stderr, "Max depth: %d\n\n", lf->max_depth);
        if (lf->ignore_case)
            fprintf(stderr, "Ignore case in regex matching.\n\n");
        if (lf->include_hidden)
            fprintf(stderr, "Include hidden files.\n\n");
        if (lf->follow_links)
            fprintf(stderr, "Follow symbolic links.\n\n");
        if (lf->sort)
            fprintf(stderr, "Sort output in ascending order.\n\n");
        if (lf->sort_reverse)
            fprintf(stderr, "Sort output in reverse order.\n\n");
    }
    return;
}
/** @brief Build a full file path by concatenating a directory path and a file name.
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
        if (diff == 0) {
            if (atomic_compare_exchange_weak_explicit(&lf->q->enqueue_pos, &pos, pos + 1,
                                                      memory_order_relaxed, memory_order_relaxed)) {
                break;
            }
        } else if (diff < 0)
            return false; // Queue is full, trigger recursion
        else
            pos++;
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
QueuePayload *mpmc_dequeue(LfContext *lf) {
    QueuePayload *current_node;
    size_t pos = atomic_load_explicit(&lf->q->dequeue_pos, memory_order_relaxed);
    while (true) {
        if (atomic_load_explicit(&lf->q->shut_down, memory_order_relaxed))
            return nullptr;
        size_t seq = atomic_load_explicit(&lf->q->sequence[pos & QUEUE_MASK], memory_order_acquire);
        intptr_t diff = (intptr_t)seq - (intptr_t)(pos + 1);
        if (diff == 0) {
            if (atomic_compare_exchange_strong_explicit(&lf->q->dequeue_pos, &pos, pos + 1,
                                                        memory_order_relaxed, memory_order_relaxed)) {
                break;
            }
        } else if (diff < 0) { // Queue is temporarily empty
            sched_yield();
        } else {
            pos = atomic_load_explicit(&lf->q->dequeue_pos, memory_order_relaxed);
        }
    }
    current_node = &lf->q->nodes[pos & QUEUE_MASK];
    atomic_store_explicit(&lf->q->sequence[pos & QUEUE_MASK], pos + QUEUE_CAPACITY, memory_order_release);
    return current_node;
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
    QueuePayload *current_node = nullptr;
    QueuePayload child_node;
    while (true) {
        current_node = mpmc_dequeue(lf);
        if (current_node != nullptr) {
            memset(&child_node, 0, sizeof(QueuePayload));
            finder(lf, current_node, &child_node);
            if (atomic_fetch_sub_explicit(&lf->q->active_tasks, 1, memory_order_acq_rel) == 1)
                atomic_store_explicit(&lf->q->shut_down, true, memory_order_relaxed);
        } else
            break;
    }
    return NULL;
}
// ----------------------------------------------------------------------
// FINDER
// ----------------------------------------------------------------------
/** @brief Process a directory and its entries, applying filters and enqueuing subdirectories.
    @param lf A pointer to the LfContext struct containing the queue and other context information.
    @param current_node A pointer to the QueuePayload struct representing the current directory to process.
    @return NULL upon completion.
    @details This function opens the specified directory, reads its entries, and processes each entry according to the specified filters (e.g., file types, hidden files, max depth). It uses fstatat to get metadata for each entry and determines the effective type. If an entry is a directory and meets the criteria, it is enqueued for further processing. The function handles errors gracefully, logging them if necessary, and ensures that output is written in a thread-safe manner.
   */
void *finder(LfContext *lf, QueuePayload *current_node, QueuePayload *child_node) {
    OutputBuffer output = {{'\0'}, 0};
    long nread;
    char dir_buf[DIR_BUF_SIZE];
    struct stat sb = {};
    struct linux_dirent64 *entry;
    unsigned char actual_type;
    unsigned char effective_type;
    char full_path[MAX_PATH_LEN] = {'\0'};
    int rc;
    int dir_fd = open(current_node->path, O_RDONLY | O_DIRECTORY);
    if (dir_fd == -1) {
        atomic_fetch_add(&lf->error_count, 1);
        if (lf->report_errors) {
            err_out(lf, "OPEN_FAIL,%s,%s\n", current_node->path, strerror(errno));
        }
        flush_output_buffer(lf, &output);
        return NULL;
    }
    while (1) {
        //--------------------------------------------------------------------
        // READ DIRECTORY
        //--------------------------------------------------------------------
        // Read the directory entries and process each one. We use readdir to
        // iterate over the entries in the directory. For each entry, we
        // construct the full path and use fstatat to get the metadata of
        // the entry. If the entry is a symbolic link, we check if the user
        // has chosen to follow links and get the metadata of the target it
        // points to. We then determine the effective type of the entry and
        // apply the specified filters (e.g., hidden files, max depth) to
        // decide whether to process it further or enqueue it for searching.
        nread = syscall(SYS_getdents64, dir_fd, dir_buf, DIR_BUF_SIZE);
        if (nread == -1) {
            atomic_fetch_add(&lf->error_count, 1);
            if (lf->report_errors) {
                err_out(lf, "READDIR_FAIL,%s,%s\n", current_node->path, strerror(errno));
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
            // Get link's metadata
            // We use fstatat with AT_SYMLINK_NOFOLLOW t  get the metadata
            // of the symbolic link itself, rather than the target it points
            // to. This allows us to determine if the entry is a symbolic
            // link and handle it according to the user's options (e.g.,
            // whether to follow links or not). If fstatat fails, we log the
            // error (if debugging is enabled) and continue to the next
            // entry without processing this one further.
            full_path[0] = '\0';
            effective_type = entry->d_type;
            if (effective_type == DT_DIR) {
                if (entry->d_name[0] == '.') {
                    if (entry->d_name[1] == '\0')
                        continue;
                    if (entry->d_name[1] == '.' && entry->d_name[2] == '\0')
                        continue;
                }
            }
            if (((lf->follow_links || lf->report_badlinks) &&
                 (effective_type == DT_LNK || effective_type == DT_DIR)) ||
                effective_type == DT_UNKNOWN) {
                //--------------------------------------------------------------------
                // GET ADVANCED METADATA
                //--------------------------------------------------------------------
                // We use fstatat with AT_SYMLINK_NOFOLLOW to get the metadata of the
                // symbolic link itself, rather than the target it points to. This allows us to determine if the entry is a symbolic link and handle it according to the user's options (e.g., whether to follow links or not). If fstatat fails, we log the error (if debugging is enabled) and continue to the next entry without processing this one further.
                rc = fstatat(dir_fd, entry->d_name, &sb, AT_SYMLINK_NOFOLLOW);
                if (rc == -1) {
                    atomic_fetch_add(&lf->error_count, 1);
                    if (lf->report_badlinks) {
                        err_out(lf, "LSTAT_FAIL,%s,%s\n", full_path,
                                strerror(errno));
                    }
                    continue;
                }
                effective_type = (sb.st_mode & S_IFMT) >> 12;
                actual_type = effective_type;
                if (S_ISLNK(sb.st_mode)) {
                    // Determine the real type of the entry. If the entry is a
                    // symbolic link, we set real_type to DT_LNK and then attempt to
                    // get the metadata of the target it points to using fstatat
                    // without AT_SYMLINK_NOFOLLOW. This allows us to determine the
                    // effective type of the entry based on the target's metadata,
                    // which is important for deciding how to process it (e.g.,
                    // whether it's a directory that we should enqueue for further
                    // searching). If fstatat fails when trying to get the target's
                    // metadata, we log the error (if debugging is enabled) but
                    // continue processing the entry based on its symbolic link
                    // metadata.
                    if (lf->follow_links) {
                        //------------------------------------------------------------
                        // GET LINK METADATA
                        //------------------------------------------------------------
                        char tmp_str[MAXLEN];
                        rc = fstatat(dir_fd, entry->d_name, &sb, 0);
                        if (rc == -1) {
                            atomic_fetch_add(&lf->error_count, 1);
                            if (lf->report_errors) {
                                err_out(lf, "STAT_FAIL,%s,%s\n", entry->d_name,
                                        strerror(errno));
                                ssnprintf(tmp_str, MAXLEN - 1, "FSTATAT_FAIL,%s/%s\n",
                                          entry->d_name, strerror(errno));
                            }
                            continue;
                        }
                        effective_type = (sb.st_mode & S_IFMT) >> 12;
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
                if (!build_full_path(child_node->path,
                                     sizeof(child_node->path),
                                     current_node->path,
                                     entry->d_name,
                                     &child_node->path_len)) {
                    atomic_fetch_add(&lf->error_count, 1);
                    if (lf->report_errors) {
                        err_out(lf, "PATH_TOO_LONG,%s/%s\n",
                                child_node->path, entry->d_name);
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
                //
                // Check for cycles by comparing the current
                // directory's dev/inode with the history of dev/inode pairs
                // from parent directories. If a match is found, it
                // indicates a cycle and we skip processing this directory.
                if (lf->follow_links && actual_type == DT_LNK) {
                    // -------------------------------------------------------
                    // CYCLE_DETECTION
                    // -------------------------------------------------------
                    if (lf->report_trace) {
                        err_out(lf, "current_node: %12p, %8lu, %12p, %s\n", current_node, current_node->ino, current_node->parent, current_node->path);
                        err_out(lf, "child_node:   %12p, %8lu, %12p, %s\n", child_node, child_node->ino, child_node->parent, child_node->path);
                    }
                    QueuePayload *parent = current_node;
                    while (parent) {
                        if (lf->report_trace)
                            err_out(lf, "parent:       %12p, %8lu, %12p, %s\n", parent, parent->ino, parent->parent, parent->path);
                        if (sb.st_dev == parent->dev && sb.st_ino == parent->ino) {
                            cycle_found = true;
                            if (lf->report_trace)
                                err_out(lf, "CYCLE: ==>     %8lu, %s\n", parent->ino, parent->path);
                            break;
                        }
                        parent = parent->parent;
                    }
                    if (cycle_found) {
                        if (lf->report_badlinks) {
                            atomic_fetch_add(&lf->error_count, 1);
                            char lnk_path[MAX_PATH_LEN] = {'\0'};
                            ssize_t len =
                                readlinkat(dir_fd, entry->d_name, lnk_path,
                                           sizeof(lnk_path) - 1);
                            if (len != -1) {
                                lnk_path[len] = '\0';
                                err_out(lf, "CYCLIC LINK:%s==>%s\n", child_node->path,
                                        lnk_path);
                            } else
                                err_out(lf, "CYCLIC LINK,%s\n", child_node->path);
                        }
                        child_node->dev = sb.st_dev;
                        child_node->ino = sb.st_ino;
                        child_node->parent = current_node;
                        child_node->depth = current_node->depth + 1;
                        scan_file(child_node->path, &child_node->path_len, lf, effective_type, &sb, &output);
                        continue;
                    }
                }
                // --------------------------------------------------------
                // BUILD AND ENQUEUE CHILD TASK
                // --------------------------------------------------------
                if (lf->max_depth != 0 && current_node->depth + 1 >= lf->max_depth)
                    continue;
                // --------------------------------------
                // Create the device/inode/parent history
                // Used to detect cyclic loops
                // --------------------------------------
                child_node->dev = sb.st_dev;
                child_node->ino = sb.st_ino;
                child_node->parent = current_node;
                child_node->depth = current_node->depth + 1;
                if (!cycle_found) {
                    if (mpmc_enqueue(lf, child_node)) {
                        atomic_fetch_add_explicit(&lf->q->active_tasks, 1, memory_order_relaxed);
                    } else {
                        // QUEUE FULL: Run finder inline recursively.
                        //  DO NOT touch the counter here because the thread is
                        // staying within its current execution bubble.
                        current_node->depth = child_node->depth;
                        memcpy(current_node->path, child_node->path, child_node->path_len + 1);
                        current_node->dev = child_node->dev;
                        current_node->ino = child_node->ino;
                        current_node->parent = child_node->parent;
                        memset(child_node, 0, sizeof(QueuePayload));
                        finder(lf, current_node, child_node);
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
                    scan_file(child_node->path, &child_node->path_len, lf, effective_type, &sb, &output);
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
                                         sizeof(full_path),
                                         current_node->path,
                                         entry->d_name,
                                         &path_len)) {
                        atomic_fetch_add(&lf->error_count, 1);
                        if (lf->report_errors) {
                            err_out(lf, "PATH_TOO_LONG,%s/%s\n",
                                    current_node->path, entry->d_name);
                        }
                        continue;
                    }
                }
                if (!lf->only_errors)
                    scan_file(full_path, &path_len, lf, effective_type, &sb, &output);
            }
        }
    }
    close(dir_fd);
    flush_output_buffer(lf, &output);
    return NULL;
}
// ----------------------------------------------------------------------
// SCAN_FILE
// ----------------------------------------------------------------------
/** @brief Scan a file or directory and apply filters based on the LfContext.
    @param file_spec The full path of the file or directory to scan.
    @param lf A pointer to the LfContext struct containing the search filters and options.
    @param effective_type The effective type of the file or directory (e.g., DT_REG, DT_DIR).
    @param cached_sb A pointer to a stat struct containing cached metadata for the file, or nullptr if not available.
    @param output A pointer to the OutputBuffer struct where matching paths will be appended.
    @return true if the file or directory passes all filters and is processed, false otherwise.
    @details This function checks various conditions based on the search filters specified in the LfContext. It evaluates whether the file or directory should be included in the output based on type, regex matching, ownership, permissions, modification time, and size. If all conditions are met, it appends the path to the output buffer. The function also handles caching of stat information to avoid redundant system calls when possible.
*/
int scan_file(const char *file_spec, const size_t *path_len, LfContext *lf,
              const unsigned char effective_type, const struct stat *cached_sb,
              OutputBuffer *output) {
    bool stat_cached = cached_sb != nullptr;
    struct stat sb = {};

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
            if (!stat_cached && stat(file_spec, &sb) == 0)
                stat_cached = true;
            if (!stat_cached || sb.st_uid != lf->user_id)
                break;
        }
        if (lf->include_perms) {
            if (!stat_cached && stat(file_spec, &sb) == 0)
                stat_cached = true;
            if (!stat_cached)
                break;
            if ((lf->include_perms & LF_IRUSR) && !(sb.st_mode & S_IRUSR))
                break;
            else if ((lf->include_perms & LF_IWUSR) && !(sb.st_mode & S_IWUSR))
                break;
            else if ((lf->include_perms & LF_IXUSR) && !(sb.st_mode & S_IXUSR))
                break;
            else if ((lf->include_perms & LF_ISUID) && !(sb.st_mode & S_ISUID))
                break;
            else if ((lf->include_perms & LF_ISGID) && !(sb.st_mode & S_ISGID))
                break;
        }
        if (lf->before) { // Last file modification
            if (!stat_cached && stat(file_spec, &sb) == 0)
                stat_cached = true;
            if (stat_cached && sb.st_mtime > lf->before)
                break;
        }
        if (lf->after) { // Last file modification
            if (!stat_cached && stat(file_spec, &sb) == 0)
                stat_cached = true;
            if (stat_cached && sb.st_mtime < lf->after)
                break;
        }
        if (lf->file_size_min) {
            if (!stat_cached && stat(file_spec, &sb) == 0)
                stat_cached = true;
            if (stat_cached && sb.st_size < lf->file_size_min)
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
int err_out(LfContext *lf, const char *format, ...) {

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
                fprintf(stderr, "Failed to open error file %s: %s\n",
                        lf->error_file_spec, strerror(errno));
                exit(EXIT_FAILURE);
            }
            lf->error_file_open = true;
            setvbuf(lf->err_fd, NULL, _IOLBF, 0); //  line buffering
        }
    }
    pthread_mutex_lock(&lf->output_mutex);
    int result = vfprintf(lf->err_fd, format, args);
    pthread_mutex_unlock(&lf->output_mutex);

    va_end(args);
    return result;
}
