/** ANNOUNCEMENT: This file is part of the lf project, which is currently being
 * tested in anticipation of public release. The test suite consists of a test
 * script, lf_tests.sh, which uses diff to compare the output with find. There
 * is also an accompanying markdown file, lf_tests.md, that outlines the testing
 * methodology, cases, and expected results.
 *
 * One test run resulted in six files out of 600,000 listed by find that weren't
 * listed by lf. These were temporary files created by Google browser, Microsoft
 * Edge, and Thunderbird. lf rejected them because their inodes were fictitious.
 * find listed them without distinguising them from the other 599,993 normal
 * files. With find, you would never know about them. As a design choice, lf
 * segregates those files as errors. We aren't necessarily locked into our
 * design choices. Your feedback and suggestions are always welcome.
 *
 * Feel free to run the test script on your own system, and if you encounter any
 * issues, please report them to the author.
 */

/** @file lf.c
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
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define QUEUE_CAPACITY 4096
#define QUEUE_MASK (QUEUE_CAPACITY - 1)
#define MAX_PATH_LEN 1024
#define MAX_DEPTH 64

typedef struct {
    dev_t dev;
    ino_t ino;
} DevIno;

typedef struct {
    DevIno dev_ino[MAX_DEPTH];
    uint16_t depth;
    char dir_path[1024];
} TaskNode;

typedef struct {
    _Atomic size_t sequence; /**< Sequence number for the cell */
    TaskNode task;
} QueueCell;

typedef struct {
    QueueCell cells[QUEUE_CAPACITY];
    // Aligned to prevent false sharing between producers and consumers
    alignas(64) _Atomic size_t enqueue_pos;
    alignas(64) _Atomic size_t dequeue_pos;
    alignas(64) pthread_mutex_t queue_mutex;
    alignas(64) pthread_cond_t cond_var;
} MPMCQueue;

typedef enum {
    TS_SUCCESS = 0,
    TS_ERROR = 1,
    TS_MATCH = 2,
    TS_MATCH_PLUS_ERROR = 3,
} TerminationStatus;

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

// typedef struct {
//     dev_t dev;
//     ino_t ino;
// } History;
// typedef struct TaskNode TaskNode;
// struct TaskNode {
//     History *history;    /**< Array of dev/ino pairs for cycle detection */
//     TaskNode *next_task; /**< Pointer to the next node in the queue */
//     int depth;           /**< Current depth in the directory tree */
//     char *dir_path;      /**< Directory path to process */
// }; /**< Queue TaskNode (for work-stealing) */

typedef struct {
    MPMCQueue q;
    TaskNode *qhead;
    TaskNode *qtail;
    alignas(64) _Atomic atomic_int active_tasks;
    int shut_down;
    pthread_t *threads;
    atomic_size_t file_count;
    atomic_size_t error_count;
    TerminationStatus termination_status;
    unsigned int nthreads;
    pthread_mutex_t output_mutex;
    uintmax_t user_id;
    off_t file_size_min;
    long flags;
    time_t after;
    time_t before;
    int max_depth;
    int reg_flags;
    char *base_path;
    char *re;
    char *ere;
    char *user_name;
    regex_t compiled_re;
    regex_t compiled_ere;
    unsigned char include_perms;
    unsigned char include_types;
    unsigned char suppress_types;
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
    char data[64 * 1024];
    size_t len;
} OutputBuffer;

#define DT_LNK_DIR 14
unsigned char const lf_mask[15] = {
    0, 0b00000001, 0b00000010, 0, 0b00000100, 0, 0b00001000, 0, 0b00010000, 0,
    0b00100000, 0, 0b01000000, 0, 0b10000000};

// typedef struct { /** not used yet */
//     TaskNode *qhead;
//     TaskNode *qtail;
// } TaskQueue;
// ---------------------------------------------------------------
//                              ╭───────────╮
// ╭───────────╮     ╭──────────╯ dir_path  ╰───────────╮
// │ TaskQueue ├─────┤ TaskNode   history     dev/inode │
// ╰───────────╯     ╰──────────╮ depth     ╭───────────╯
//                              │ next_task │
//                              ╰───────────╯
// ---------------------------------------------------------------

int lfargc;
char *lfargs[3];
char *exec;
char *file_types_p;
char *perms_p;
char *debug_p;
void debug_out(LfContext *lf, int, char **);

bool init_find(LfContext *lf, int, char **);
void sort_lf_output(LfContext *lf, int, char **);
// void enqueue_dir(LfContext *lf, TaskNode *, LfContext *);
// TaskNode *dequeue_dir(LfContext *lf);
void queue_init(MPMCQueue *q);
bool enqueue_dir(LfContext *, const TaskNode *item);
bool dequeue_dir(LfContext *, TaskNode *item);

void *finder(void *);
int scan_file(const char *, LfContext *lf, const unsigned char,
              const struct stat *, OutputBuffer *);
bool build_full_path(char *, size_t, const char *, const char *, size_t *);
void flush_output_buffer(LfContext *, OutputBuffer *);
bool append_output_buffer(LfContext *, OutputBuffer *, const char *, size_t,
                          bool);
// ---------------------------------------------------------------

static struct argp_option options[] = {
    {"after", 'a', "time", 0, "Modified after YYYY-MM-DDTHH:MM:SS", 0},
    {"before", 'b', "time", 0, "Modified before YYYY-MM-DDTHH:MM:SS", 0},
    {"max_depth", 'd', "number", 0, "Depth into directory tree", 0},
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
#ifdef EXPERIMENTAL
    {"exec", 'x', "command", 0, "execute external command", 0},
#endif
    {"debug", 'D', "12345678", 0,
     "1-config, 2-info, 3-warnings, 4-errors, 5-badlinks, 6-trace, 7-all, "
     "8-only_errors",
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
                case '1':
                    lf->report_config = true;
                    break;
                case '2':
                    lf->report_info = true;
                    break;
                case '3':
                    lf->report_warnings = true;
                    break;
                case '4':
                    lf->report_errors = true;
                    break;
                case '5':
                    lf->report_badlinks = true;
                    break;
                case '6':
                    lf->report_trace = true;
                    break;
                case '7':
                    lf->report_config = true;
                    lf->report_info = true;
                    lf->report_warnings = true;
                    lf->report_errors = true;
                    lf->report_badlinks = true;
                    lf->report_all = true;
                    break;
                case '8':
                    lf->only_errors = true;
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
#ifdef EXPERIMENTAL
    case 'x':
        lf->exec = strdup(arg);
        break;
#endif
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

    lf->file_count = 0;
    lf->error_count = 0;
    lf->termination_status = TS_ERROR;
    lf->nthreads = 0;
    lf->ignore_case = false;
    lf->sort = false;
    lf->sort_reverse = false;
    lf->include_hidden = false; // By default, hidden files are suppressed. Use
    // LF_HIDE = 0 - include hidden files,
    // LF_HIDE = 1 - suppress hidden files
    lf->hidden_only = false;
    lf->flags |= LF_HIDE;     // Turn hide flag on
    lf->follow_links = false; // By default, symbolic links are not followed. Use
                              // -L to follow them.
    lf->count = false;
    lf->count_silently = false;

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
            exit(lf->termination_status);
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
            exit(lf->termination_status);
        }
    }
    if (lf->base_path == nullptr || lf->base_path[0] == '\0')
        lf->base_path = strdup(".");
    lf->termination_status = TS_SUCCESS;
    if (!lf->sort) {
        init_find(lf, argc, argv);
    } else
        sort_lf_output(lf, argc, argv);
    if (lf->count) {
        size_t count = atomic_load(&lf->file_count);
        fprintf(stderr, "Files: %zu\n", count);
    }
    atomic_load(&lf->error_count);
    if (lf->error_count > 0) {
        fprintf(stderr, "Errors: %zu\n", lf->error_count);
        lf->termination_status |= TS_ERROR;
    }
    exit(lf->termination_status);
}
// Initialize and transfer control to the finder
/** If sorting is requested, execute the finder and pipe its output
 * to the sort command. We can achieve this by creating a child
 * process that runs the sort command, and redirecting the output of
 * the file finder to the input of the sort command using a pipe.
 * The parent process will run the finder and write its output to
 * the pipe, while the child process will read from the pipe and
 * execute the sort command. */
void sort_lf_output(LfContext *lf, int argc, char **argv) {
    char *eargv[MAXARGS];
    int eargc = 0;
    eargv[eargc++] = strdup("sort");
    if (lf->sort_reverse)
        eargv[eargc++] = strdup("-r");
    eargv[eargc] = nullptr;
    int wstatus;

    // int save_fd = dup(STDOUT_FILENO); // save current STDOUT
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
        exit(lf->termination_status);
    }
    // fclose(stdout);
    dup2(fds[1], STDOUT_FILENO);         // Clone write pipe to STDOUT_FILENO
    stdout = fdopen(STDOUT_FILENO, "w"); // Reopen STDOUT as a stream
    setvbuf(stdout, NULL, _IONBF, 0);    //  line buffering
    init_find(lf, argc, argv);           // Initialize and transfer control to the finder
    fclose(stdout);
    close(fds[1]);
    wait(&wstatus);
    for (int i = 0; i < eargc; i++)
        free(eargv[i]);
}
bool init_find(LfContext *lf, int argc, char **argv) {
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
    unsigned int nprocs = get_nprocs();
    if (nprocs == 0)
        nprocs = 1;

    if (lf->nthreads == 0) {
        lf->nthreads = (nprocs > 1) ? (nprocs - 1) : 1;
    } else {
        if (lf->nthreads < 1)
            lf->nthreads = 1;
        if (lf->nthreads > nprocs)
            lf->nthreads = nprocs;
    }
    debug_out(lf, argc, argv);
    //--------------------------------------------------------------------
    // Create and enqueue the first TaskNode
    lf->termination_status = TS_SUCCESS;
    int rc = 0;
    struct stat st;
    if (stat(lf->base_path, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            TaskNode child_task = {};
            strnz__cpy(child_task.dir_path, lf->base_path, MAX_PATH_LEN - 1);
            child_task.depth = 0;
            child_task.dev_ino[0].dev = st.st_dev;
            child_task.dev_ino[0].ino = st.st_ino;
            queue_init(&lf->q);
            if (!enqueue_dir(lf, &child_task)) {
                fprintf(stderr, "Failed to enqueue initial directory\n");
                return false;
            }
            lf->threads = calloc(lf->nthreads, sizeof(*lf->threads));
            if (!lf->threads) {
                fprintf(stderr, "Out of memory allocating threads\n");
                return false;
            }
            for (unsigned int i = 0; i < lf->nthreads; i++) {
                rc = pthread_create(
                    &lf->threads[i],
                    NULL,
                    finder,
                    lf);

                if (rc != 0) {
                    fprintf(stderr, "Error: Unable to create thread %d\n", rc);
                    lf->shut_down = 1;
                    lf->termination_status = TS_ERROR;
                    return false;
                }
            }
            rc = pthread_mutex_init(&lf->output_mutex, NULL);
            if (rc != 0)
                perror("Mutex initialization failed");
            pthread_mutex_lock(&lf->q.queue_mutex);
            while (!lf->shut_down)
                pthread_cond_wait(&lf->q.cond_var, &lf->q.queue_mutex);
            pthread_mutex_unlock(&lf->q.queue_mutex);

            for (unsigned int i = 0; i < lf->nthreads; i++)
                pthread_join(lf->threads[i], NULL);
        } else {
            fprintf(stderr,
                    "Warning: Base path '%s' is not a directory. No "
                    "files will be found.\n",
                    lf->base_path);
            lf->termination_status = TS_ERROR;
            return false;
        }
    }
    //--------------------------------------------------------------------
    // End Program
    rc = pthread_mutex_destroy(&lf->q.queue_mutex);
    rc = pthread_cond_destroy(&lf->q.cond_var);
    rc = pthread_mutex_destroy(&lf->output_mutex);
    if (rc != 0)
        perror("Mutex initialization failed");
    if (lf->flags & LF_REGEX) {
        regfree(&lf->compiled_re);
    }
    if (lf->flags & LF_EXC_REGEX) {
        regfree(&lf->compiled_ere);
    }
    free(lf->base_path);
    free(lf->user_name);
    free(lf->re);
    free(lf->ere);
    free(lf->threads);
    free(lf);
    if (reti)
        return false;
    return true;
}
void debug_out(LfContext *lf, int argc, char **argv) {
    char user_str[100];
    char ip_str[MAXLEN];
    int len = 0;
    int i;
    bool addspace_before = false;
    if (lf->debug && (lf->report_config || lf->report_info || lf->report_all)) {
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
        fprintf(stderr, "  7-all         %s\n",
                lf->report_all ? "true" : "|    false");
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
        if (lf->report_config && !lf->report_all)
            exit(TS_ERROR);
    }
    return;
}
// ----------------------------------------------------------------------
// QUEUE
// ----------------------------------------------------------------------
/** @brief Initialize the MPMCQueue.
    @param q A pointer to the MPMCQueue struct representing the queue.
    @details This function initializes the MPMCQueue by setting up the sequence numbers for each cell in the queue and initializing the enqueue and dequeue positions. It ensures that the queue is ready for concurrent access by multiple producer and consumer threads.
   */
void queue_init(MPMCQueue *q) {
    int rc;
    for (size_t i = 0; i < QUEUE_CAPACITY; i++) {
        atomic_init(&q->cells[i].sequence, i);
    }
    atomic_init(&q->enqueue_pos, 0);
    atomic_init(&q->dequeue_pos, 0);
    rc = pthread_mutex_init(&q->queue_mutex, NULL);
    if (rc != 0)
        perror("Mutex initialization failed");
    rc = pthread_cond_init(&q->cond_var, NULL);
    if (rc != 0)
        perror("Mutex initialization failed");
}
/** @brief Enqueue a directory task into the MPMCQueue.
    @param q A pointer to the MPMCQueue struct representing the queue.
    @param item A pointer to the TaskNode struct representing the directory task to be enqueued.
    @return true if the task was successfully enqueued, false if the queue is full.
    @details This function attempts to enqueue a directory task into the MPMCQueue in a lock-free manner. It uses atomic operations to ensure thread safety and avoid race conditions. If the queue is full, it returns false; otherwise, it copies the task data into the reserved slot and updates the sequence number to indicate that the slot is now occupied.
   */
bool enqueue_dir(LfContext *lf, const TaskNode *item) {
    QueueCell *cell;
    pthread_mutex_lock(&lf->q.queue_mutex);
    size_t pos = lf->q.enqueue_pos;
    while (true) {
        cell = &lf->q.cells[pos & QUEUE_MASK];
        size_t seq = cell->sequence;
        intptr_t diff = (intptr_t)seq - (intptr_t)pos;
        if (diff == 0) {
            if (lf->q.enqueue_pos == pos) {
                lf->q.enqueue_pos = pos + 1;
                break;
            } else
                pos = lf->q.enqueue_pos;
        } else if (diff < 0) {
            return false;
        } else {
            pos = lf->q.enqueue_pos;
        }
    }
    lf->q.cells[pos & QUEUE_MASK].task = *item;
    cell->sequence = pos + 1;
    pthread_cond_signal(&lf->q.cond_var);
    pthread_mutex_unlock(&lf->q.queue_mutex);
    return true;
}

// ----------------------------------------------------------------------
// DEQUEUE
// ----------------------------------------------------------------------
/** @brief Dequeue a directory task from the MPMCQueue.
    @param q A pointer to the MPMCQueue struct representing the queue.
    @param item A pointer to a TaskNode struct where the dequeued task will be stored.
    @return true if a task was successfully dequeued, false if the queue is empty.
    @details This function attempts to dequeue a directory task from the MPMCQueue in a lock-free manner. It uses atomic operations to ensure thread safety and avoid race conditions. If the queue is empty, it returns false; otherwise, it copies the task data from the reserved slot into the provided TaskNode and updates the sequence number to indicate that the slot is now free for future enqueues.
   */
bool dequeue_dir(LfContext *lf, TaskNode *item) {
    QueueCell *cell;
    pthread_mutex_lock(&lf->q.queue_mutex);
    size_t pos = lf->q.dequeue_pos;
    while (true) {
        cell = &lf->q.cells[pos & QUEUE_MASK];
        // size_t seq = atomic_load_explicit(&cell->sequence, memory_order_acquire);
        size_t seq = cell->sequence;
        intptr_t diff = (intptr_t)seq - (intptr_t)(pos + 1);
        if (diff == 0) {
            if (lf->q.dequeue_pos == pos) {
                lf->q.dequeue_pos = pos + 1;
                break;
            } else
                pos = lf->q.dequeue_pos;
        } else if (diff < 0) {
            if (!lf->shut_down) {
                if (atomic_fetch_add(&lf->active_tasks, 1) == 0) {
                    lf->shut_down = 1;
                    pthread_cond_broadcast(&lf->q.cond_var);
                    break;
                }
                pthread_cond_wait(&lf->q.cond_var, &lf->q.queue_mutex);
                break;
            }
        } else {
            pos = lf->q.dequeue_pos;
        }
    }
    if (lf->shut_down) {
        pthread_mutex_unlock(&lf->q.queue_mutex);
        return false;
    }
    *item = lf->q.cells[pos & QUEUE_MASK].task;
    cell->sequence = pos + QUEUE_CAPACITY;
    atomic_fetch_add(&lf->active_tasks, 1);
    pthread_mutex_unlock(&lf->q.queue_mutex);
    return true;
}
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
/** @brief Worker thread function to process directories from the queue.
    @param arg Pointer to the SearchFilters struct containing the options
   and flags for filtering.
    @return NULL
    @details This function continuously dequeues directory dir_path from the
   global queue and processes them. For each directory, it lists its
   contents and applies the specified filters to each file. If a
   subdirectory is found and it meets the criteria for further searching
   (e.g., not hidden if hidden files are suppressed, and within max depth),
   it is enqueued for processing. The function uses atomic operations to
   track active tasks and condition variables to manage thread
   synchronization and shut_down when all work is complete.
   */
void *finder(void *arg) {
    LfContext *lf = (LfContext *)arg;
    char lnk_path[MAX_PATH_LEN] = {'\0'};
    OutputBuffer output = {{'\0'}, 0};
    TaskNode current_task = {};
    while (1) {
        if (dequeue_dir(lf, &current_task) == false)
            break;
        // --------------------------------------------------------------------
        // INITIALIZE DIRECTORY - PRIMING READ
        // --------------------------------------------------------------------
        int dir_fd =
            openat(AT_FDCWD, current_task.dir_path, O_RDONLY | O_DIRECTORY);
        if (dir_fd == -1) {
            atomic_fetch_add(&lf->error_count, 1);
            if (lf->debug && (lf->report_warnings || lf->report_errors ||
                              lf->report_badlinks || lf->report_all)) {
                pthread_mutex_lock(&lf->output_mutex);
                fprintf(stderr, "OPEN_FAIL,%s,%s\n", current_task.dir_path, strerror(errno));
                pthread_mutex_unlock(&lf->output_mutex);
            }
            atomic_fetch_sub(&lf->active_tasks, 1);
            pthread_cond_broadcast(&lf->q.cond_var);
            continue;
        }
        DIR *dir = fdopendir(dir_fd);
        if (dir == NULL) {
            atomic_fetch_add(&lf->error_count, 1);
            if (lf->debug && (lf->report_warnings || lf->report_errors ||
                              lf->report_badlinks || lf->report_all)) {
                pthread_mutex_lock(&lf->output_mutex);
                fprintf(stderr, "\nFDOPENDIR_FAIL,%s,%s\n",
                        current_task.dir_path, strerror(errno));
                pthread_mutex_unlock(&lf->output_mutex);
            }
            close(dir_fd);
            atomic_fetch_sub(&lf->active_tasks, 1);
            pthread_cond_broadcast(&lf->q.cond_var);
            continue;
        }
        //--------------------------------------------------------------------
        // MAIN LOOP - READ DIRECTORY ENTRIES
        //--------------------------------------------------------------------
        // Read the directory entries and process each one. We use readdir to
        // iterate over the entries in the directory. For each entry, we
        // construct the full path and use fstatat to get the metadata of
        // the entry. If the entry is a symbolic link, we check if the user
        // has chosen to follow links and get the metadata of the target it
        // points to. We then determine the effective type of the entry and
        // apply the specified filters (e.g., hidden files, max depth) to
        // decide whether to process it further or enqueue it for searching.
        //--------------------------------------------------------------------
        unsigned char effective_type;
        char full_path[MAX_PATH_LEN] = {'\0'};
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            struct stat st;
            // Get link's metadata
            int rc;
            // We use fstatat with AT_SYMLINK_NOFOLLOW to get the metadata
            // of the symbolic link itself, rather than the target it points
            // to. This allows us to determine if the entry is a symbolic
            // link and handle it according to the user's options (e.g.,
            // whether to follow links or not). If fstatat fails, we log the
            // error (if debugging is enabled) and continue to the next
            // entry without processing this one further.
            effective_type = entry->d_type;
            rc = fstatat(dir_fd, entry->d_name, &st, AT_SYMLINK_NOFOLLOW);
            if (rc == -1) {
                atomic_fetch_add(&lf->error_count, 1);
                if (lf->debug && (lf->report_errors || lf->report_warnings ||
                                  lf->report_badlinks || lf->report_all)) {
                    if (!build_full_path(full_path, sizeof(full_path),
                                         current_task.dir_path,
                                         entry->d_name, nullptr)) {
                        ssnprintf(full_path, sizeof(full_path), "%s/%s",
                                  current_task.dir_path, entry->d_name);
                    }
                    pthread_mutex_lock(&lf->output_mutex);
                    fprintf(stderr, "LSTAT_FAIL,%s,%s\n", full_path,
                            strerror(errno));
                    pthread_mutex_unlock(&lf->output_mutex);
                }
                continue;
            }
            effective_type = (st.st_mode & S_IFMT) >> 12;
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
            // if (effective_type == DT_LNK) {
            // Get the target's metadata
            if (S_ISLNK(st.st_mode)) {
                rc = fstatat(dir_fd, entry->d_name, &st, 0);
                if (rc == -1) {
                    atomic_fetch_add(&lf->error_count, 1);
                    if (lf->debug && (lf->report_all || lf->report_warnings ||
                                      lf->report_errors || lf->report_badlinks)) {
                        if (!build_full_path(full_path, sizeof(full_path),
                                             current_task.dir_path,
                                             entry->d_name, nullptr)) {
                            ssnprintf(full_path, sizeof(full_path), "%s/%s",
                                      current_task.dir_path, entry->d_name);
                        }
                        pthread_mutex_lock(&lf->output_mutex);
                        fprintf(stderr, "STAT_FAIL,%s,%s\n", full_path,
                                strerror(errno));
                        pthread_mutex_unlock(&lf->output_mutex);
                    }
                    continue;
                }
                if (lf->follow_links)
                    effective_type = (st.st_mode & S_IFMT) >> 12;
            }
            if (effective_type != DT_DIR) {
                if (is_hidden(entry->d_name)) {
                    if (!lf->include_hidden)
                        continue;
                } else if (lf->hidden_only)
                    continue;
            } else {
                if (is_dirsys(entry->d_name))
                    continue;
                if (!build_full_path(full_path, sizeof(full_path),
                                     current_task.dir_path,
                                     entry->d_name, nullptr)) {
                    atomic_fetch_add(&lf->error_count, 1);
                    if (lf->debug && (lf->report_errors || lf->report_warnings ||
                                      lf->report_badlinks || lf->report_all)) {
                        pthread_mutex_lock(&lf->output_mutex);
                        fprintf(stderr, "PATH_TOO_LONG,%s/%s\n",
                                current_task.dir_path, entry->d_name);
                        pthread_mutex_unlock(&lf->output_mutex);
                    }
                    continue;
                }
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
                bool cycle_found = false;
                if (lf->debug && (lf->report_trace || lf->report_all)) {
                    pthread_mutex_lock(&lf->output_mutex);
                    fprintf(stderr, "Checking for cycles in: %s\n", full_path);
                    pthread_mutex_unlock(&lf->output_mutex);
                }
                for (int i = 0; i <= current_task.depth; i++) {
                    if (lf->debug && (lf->report_trace || lf->report_all)) {
                        pthread_mutex_lock(&lf->output_mutex);
                        if (current_task.dev_ino[i].ino == st.st_ino)
                            fprintf(stderr, "%3d %ju %ju<===========\n", i,
                                    current_task.dev_ino[i].ino, st.st_ino);
                        else
                            fprintf(stderr, "%3d %ju %ju\n", i,
                                    current_task.dev_ino[i].ino, st.st_ino);
                        pthread_mutex_unlock(&lf->output_mutex);
                    }
                    if (current_task.dev_ino[i].dev == st.st_dev &&
                        current_task.dev_ino[i].ino == st.st_ino) {
                        cycle_found = true;
                        break;
                    }
                }
                if (cycle_found) {
                    atomic_fetch_add(&lf->error_count, 1);
                    if (lf->debug && (lf->report_warnings || lf->report_errors ||
                                      lf->report_trace || lf->report_badlinks ||
                                      lf->report_all)) {
                        ssize_t len =
                            readlinkat(dir_fd, entry->d_name, lnk_path,
                                       sizeof(lnk_path) - 1);
                        pthread_mutex_lock(&lf->output_mutex);
                        if (len != -1) {
                            lnk_path[len] = '\0';
                            fprintf(stderr, "CYCLIC_LINK,%s,%s\n", full_path,
                                    lnk_path);
                        } else
                            fprintf(stderr, "CYCLIC_LINK,%s\n", full_path);
                        pthread_mutex_unlock(&lf->output_mutex);
                    }
                    continue;
                }
                //-------------------------------------------------------
                // Create a new TaskNode for the subdirectory and enqueue it
                // for processing by finder threads. We duplicate the
                // current history of dev/inode pairs and add the current
                // directory's dev/inode to the new history for the child
                // task. This allows us to maintain a record of the
                // directories we've visited in the current path, which is
                // essential for cycle detection. By checking this history
                // for each new directory we encounter, we can effectively
                // prevent infinite loops caused by symbolic links or hard
                // links that create cycles in the directory structure.
                if (lf->max_depth != 0 && current_task.depth + 1 == lf->max_depth)
                    continue;
                TaskNode child_task;
                strnz__cpy(child_task.dir_path, full_path, MAX_PATH_LEN - 1);
                child_task.depth = current_task.depth + 1;
                int i;
                for (i = 0; i <= current_task.depth; i++) {
                    child_task.dev_ino[i].dev = current_task.dev_ino[i].dev;
                    child_task.dev_ino[i].ino = current_task.dev_ino[i].ino;
                }
                child_task.dev_ino[i + 1].dev = st.st_dev;
                child_task.dev_ino[i + 1].ino = st.st_ino;
                if (!enqueue_dir(lf, &child_task)) {
                    atomic_fetch_add(&lf->error_count, 1);
                    if (lf->debug && (lf->report_errors || lf->report_warnings ||
                                      lf->report_badlinks || lf->report_all)) {
                        pthread_mutex_lock(&lf->output_mutex);
                        fprintf(stderr, "QUEUE_FULL,%s\n", full_path);
                        pthread_mutex_unlock(&lf->output_mutex);
                    }
                }
                scan_file(full_path, lf, effective_type, &st, &output);
                continue;
            }
            if (lf->hidden_only && !is_hidden(entry->d_name))
                continue;
            if (!build_full_path(full_path, sizeof(full_path),
                                 current_task.dir_path,
                                 entry->d_name, nullptr)) {
                atomic_fetch_add(&lf->error_count, 1);
                if (lf->debug && (lf->report_errors || lf->report_warnings ||
                                  lf->report_badlinks || lf->report_all)) {
                    pthread_mutex_lock(&lf->output_mutex);
                    fprintf(stderr, "PATH_TOO_LONG,%s/%s\n",
                            current_task.dir_path, entry->d_name);
                    pthread_mutex_unlock(&lf->output_mutex);
                }
                continue;
            }
            scan_file(full_path, lf, effective_type, &st, &output);
        }
        closedir(dir);
        atomic_fetch_sub(&lf->active_tasks, 1);
    }
    flush_output_buffer(lf, &output);
    return NULL;
}
/** @brief Check if a file or directory is hidden based on its name.
    @param name The name of the file or directory to check.
    @return true if the name indicates a hidden file or directory, false otherwise.
    @details A file or directory is considered hidden if its name starts with a
   dot ('.') character, except for the special cases of '.' and '..' which
   represent the current and parent directories, respectively. This function
   is used to determine whether to include or exclude hidden files and
   directories during the search process based on user-specified options.
   */
bool is_hidden(const char *name) {
    if (name[0] == '.') {
        if (name[1] == '\0')
            return false;
        if (name[1] == '.' && name[2] == '\0')
            return false;
        return true;
    }
    return false;
}
/** @brief Check if a file or directory is a system directory based on its name.
    @param name The name of the file or directory to check.
    @return true if the name indicates a system directory ('.' or '..'), false otherwise.
    @details A system directory is defined as either the current directory ('.') or
   the parent directory ('..'). This function is used to determine whether to
   include or exclude these special directories during the search process,
   as they are typically not relevant for most file searches and can lead to
   infinite loops if followed.
   */
bool is_dirsys(const char *name) {
    if (name[0] == '.') {
        if (name[1] == '\0')
            return true;
        if (name[1] == '.' && name[2] == '\0')
            return true;
    }
    return false;
}
/** @brief Scan a file or directory and apply filters based on the LfContext.
    @param file_spec The full path of the file or directory to scan.
    @param lf A pointer to the LfContext struct containing the search filters and options.
    @param effective_type The effective type of the file or directory (e.g., DT_REG, DT_DIR).
    @param cached_sb A pointer to a stat struct containing cached metadata for the file, or nullptr if not available.
    @param output A pointer to the OutputBuffer struct where matching paths will be appended.
    @return true if the file or directory passes all filters and is processed, false otherwise.
    @details This function checks various conditions based on the search filters specified in the LfContext. It evaluates whether the file or directory should be included in the output based on type, regex matching, ownership, permissions, modification time, and size. If all conditions are met, it appends the path to the output buffer. The function also handles caching of stat information to avoid redundant system calls when possible.
*/
int scan_file(const char *file_spec, LfContext *lf,
              const unsigned char effective_type, const struct stat *cached_sb,
              OutputBuffer *output) {
    bool stat_cached = cached_sb != nullptr;
    struct stat sb = {0};
    if (cached_sb)
        sb = *cached_sb;

    while (1) {
        if (lf->debug && (lf->report_trace || lf->report_all)) {
            pthread_mutex_lock(&lf->output_mutex);
            printf("suppress %08b, effective %08b, lf_mask %08b, & %08b %s\n",
                   lf->suppress_types, effective_type, lf_mask[effective_type], lf->suppress_types & lf_mask[effective_type], file_spec);
            pthread_mutex_unlock(&lf->output_mutex);
        }
        if (lf->suppress_types & lf_mask[effective_type])
            break;
        // Exclude non-matching files
        if (lf->flags & LF_REGEX) {
            int reti =
                regexec(&lf->compiled_re, file_spec, 0, NULL, 0);
            if (reti == REG_NOMATCH)
                break;
            lf->termination_status |= TS_MATCH;
        }
        // Exclude matching files
        if (lf->flags & LF_EXC_REGEX) {
            int reti =
                regexec(&lf->compiled_ere, file_spec, 0, NULL, 0);
            if (reti == 0) {
                lf->termination_status |= TS_MATCH;
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
        if (lf->before) {
            if (!stat_cached && stat(file_spec, &sb) == 0)
                stat_cached = true;
            if (stat_cached && sb.st_mtime > lf->before)
                break;
        }
        if (lf->after) {
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
            size_t display_len = strlen(display_path);
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
