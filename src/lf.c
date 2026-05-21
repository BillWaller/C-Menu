/** NOTICE: This file is part of the lf project, which is currently under
 * development. There are two reasons not to use it at this time. First,
 * it is not yet fully functional and may contain bugs or incomplete,
 * unoptimized code. Second, the API and features are still being finalized,
 * so using it now may lead to compatibility issues in the future as changes
 * are made. Once the project is more mature and stable, this file will be
 * ready for use. In the meantime, it serves as a work in progress and may
 * be subject to significant changes as development continues.
 *
 * Thank you for your patience and understanding.
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
// #define LF_DEBUG
#include <argp.h>
#include <cm.h>
#include <dirent.h>
#ifdef LF_DEBUG
#include <errno.h>
#endif
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

struct tm tm_info;
const char *argp_program_version = CM_VERSION;
const char *argp_program_bug_address = "billxwaller@gmail.com";
const char doc[] = "lf list files\vIf specified, DIRECTORY is the top-level "
                   "directory to search. REGULAR_EXPRESSION is a properly "
                   "formatted regular expression for which matching files "
                   "will be listed.";
static char args_doc[] = "[DIRECTORY] [REGULAR_EXPRESSION]";
int nthreads;

static struct argp_option options[] = {
    {"after", 'a', "time", 0, "Modified after YYYY-MM-DDTHH:MM:SS", 0},
    {"before", 'b', "time", 0, "Modified before YYYY-MM-DDTHH:MM:SS", 0},
    {"max_depth", 'd', "number", 0, "Depth into directory tree", 0},
    {"ere", 'e', "regex", 0, "Exclude regular expression", 0},
    {"f_ignore_case", 'i', 0, 0, "Search ignore case", 0},
    {"permissions", 'p', "sgrwx", 0,
     "s-setuid, g-setgid, r-read, w-write, x-execute", 0},
    {"re", 'r', "regex", 0, "Regular expression to search for", 0},
    {"file_types", 't', "bcdplrsu", 0,
     "b-block, c-character, d-directory, p-pipe, l-link, r-regular, s-"
     "socket, u-unknown",
     0},
    {"file_size_min", 's', "size", 0,
     "No Suffix-bytes, K-kilobytes, M-Megabytes, or G-Gigabytes", 0},
    {"user", 'u', "user name", 0, "User Name of file owner ", 0},
    {"exec", 'x', "command", 0, "execute external command", 0},
#ifdef LF_DEBUG
    {"debug", 'D', "1234567", 0,
     "1-config, 2-info, 3-warnings, 4-errors, 5-badlinks, 6-trace, 7-all", 0},
#endif
    {"f_include_hidden", 'H', 0, 0, "Include hidden files", 0},
    {"f_lnk_dir", 'L', 0, 0, "Follow symbolic links", 0},
    {"f_reverse", 'R', 0, 0, "Sort in Reverse order", 0},
    {"f_sort", 'S', 0, 0, "Sort in Ascending order", 0},
    {"nthreads", 'T', "threads", 0, "Number of nthreads", 0},
    {0}};

typedef struct {
    char base_path[PATH_MAX];
    char re[MAXLEN];
    char ere[MAXLEN];
    regex_t compiled_re;
    regex_t compiled_ere;
    long flags;
    time_t after;
    time_t before;
    uintmax_t user_id;
    intmax_t file_size_min;
    int max_depth;
    bool f_ignore_case;
    bool f_sort;
    bool f_reverse;
    bool f_include_hidden;
    bool f_lnk_dir;
    int reg_flags;
    char exec[MAXLEN];
    unsigned char include_types;
    unsigned char suppress_types;
    bool include;
    int permissions;
    // bool blk;
    // bool chr;
    // bool dir;
    // bool fifo;
    // bool lnk;
    // bool reg;
    // bool sock;
    // bool unknown;
    bool debug;
    bool report_config;
    bool report_info;
    bool report_warnings;
    bool report_errors;
    bool report_badlinks;
    bool report_trace;
    bool report_all;
    char *args[3];
    int argc;
    char *file_types_p;
} SearchFilters;

#define DT_LNK_DIR 14

typedef struct {
    dev_t dev;
    ino_t ino;
} PathNode;

typedef struct TaskNode {
    char *dir_path;        /**< Directory path to process */
    PathNode *history;     /**< Array of dev/ino pairs for cycle detection */
    int depth;             /**< Current depth in the directory tree */
    struct TaskNode *next; /**< Pointer to the next node in the queue */
} TaskNode;                /**< Queue TaskNode (for work-stealing) */

bool init_find(SearchFilters *);
int scan_file(char *, SearchFilters *, unsigned char);

void enqueue_dir(TaskNode *);
TaskNode *dequeue_dir();
void *finder(void *);
// void exec_external(SearchFilters *, char **);

/** @brief Parse a single option.  */
static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    SearchFilters *f = state->input;
    int i = 0;
    bool a_toi_error = false;

    switch (key) {
    case 'a':
        memset(&tm_info, 0, sizeof(struct tm));
        if (strptime(arg, "%Y-%m-%dT%H:%M:%S", &tm_info) == NULL) {
            fprintf(stderr, "-a Failed to parse time string.\n");
            return 1;
        }
        f->after = mktime(&tm_info);
        if (f->after && f->before && f->after > f->before) {
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
        f->before = mktime(&tm_info);
        if (f->after && f->before && f->after > f->before) {
            fprintf(stderr, "-a time must be before -b time.\n");
            return 1;
        }
        break;
    case 'd':
        if (arg && arg[0] != '\0')
            f->max_depth = a_toi(arg, &a_toi_error);
        break;
#ifdef LF_DEBUG
    case 'D':
        if (arg && arg[0] != '\0')
            f->debug = true;
        switch (a_toi(arg, &a_toi_error)) {
        case 1:
            f->report_config = true;
            break;
        case 2:
            f->report_info = true;
            break;
        case 3:
            f->report_warnings = true;
            break;
        case 4:
            f->report_errors = true;
            break;
        case 5:
            f->report_badlinks = true;
            break;
        case 6:
            f->report_trace = true;
            break;
        case 7:
            f->report_config = true;
            f->report_info = true;
            f->report_warnings = true;
            f->report_errors = true;
            f->report_badlinks = true;
            f->report_all = true;
            break;
        default:
            f->report_errors = true;
            break;
        }
        break;
#endif
    case 'e':
        strnz__cpy(f->ere, arg, MAXLEN - 1);
        f->flags |= LF_EXC_REGEX;
        break;
    case 'H':
        f->f_include_hidden = true; // Include hidden files
        f->flags &= ~(LF_HIDE);     // Turn hide flag off
                                    // LF_HIDE = 0 - include hidden files,
                                    // LF_HIDE = 1 - suppress hidden files
        break;
    case 'i':
        f->flags |= LF_ICASE;
        break;
    case 'L':
        f->f_lnk_dir = true; // follow symbolic links
        break;
    case 'p':
        for (i = 0; arg[i]; i++) {
            switch (arg[i]) {
            case 'g':
                f->flags |= LF_SETGID << 16;
                break;
            case 'r':
                f->flags |= LF_PERM_R << 16;
                break;
            case 's':
                f->flags |= LF_SETUID << 16;
                break;
            case 'w':
                f->flags |= LF_PERM_W << 16;
                break;
            case 'x':
                f->flags |= LF_PERM_X << 16;
                break;
            default:
                break;
            }
        }
        break;
    case 'R':
        f->f_reverse = true;
        break;
    case 'r':
        strnz__cpy(f->re, arg, MAXLEN - 1);
        f->flags |= LF_REGEX;
        break;
    case 'S':
        f->f_sort = true;
        break;
    case 's':
        long long ull = a_to_ull(arg);
        f->file_size_min = (intmax_t)ull;
        break;
    case 'T':
        if (arg && arg[0] != '\0')
            nthreads = a_toi(arg, &a_toi_error);
        break;
    case 't':
        f->file_types_p = arg;
        while (f->file_types_p[i]) {
            switch (f->file_types_p[i++]) {
            case 'b':
                f->flags |= DT_BLK << 8;
                break;
            case 'c':
                f->flags |= DT_CHR << 8;
                break;
            case 'd':
                f->flags |= DT_DIR << 8;
                break;
            case 'p':
                f->flags |= DT_FIFO << 8;
                break;
            case 'l':
                f->flags |= DT_LNK << 8;
                break;
            case 'f': // for regular files, 'f' is more intuitive than 'r'
            case 'r': // regular files are the most common type, so 'r' is also
                      // accepted
                f->flags |= DT_REG << 8;
                break;
            case 's':
                f->flags |= DT_SOCK << 8;
                break;
            case 'u':
                f->flags |= DT_UNKNOWN << 8;
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
            f->flags |= (long)(pwd->pw_uid) << 24;
            f->flags |= LF_USER;
        } else {
            fprintf(stderr, "User '%s' not found.\n", arg);
            exit(EXIT_FAILURE);
        }
        break;
    case 'x':
        strnz__cpy(f->exec, arg, MAXLEN - 1);
        break;
    case ARGP_KEY_ARG:
        if (state->arg_num == 0 || state->arg_num == 1) {
            f->args[state->arg_num] = arg;
            f->argc = state->arg_num + 1;
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

/** @brief Global Synchronization State */
TaskNode *head = NULL, *tail = NULL;
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_var = PTHREAD_COND_INITIALIZER;
atomic_int active_tasks = 0;
int shutdown = 0;

int main(int argc, char **argv) {

    SearchFilters *f = (SearchFilters *)calloc(1, sizeof(SearchFilters));
    f->f_ignore_case = false;
    f->f_sort = false;
    f->f_reverse = false;
    f->f_include_hidden = false; // By default, hidden files are suppressed. Use
                                 // -H to include them.
    // LF_HIDE = 0 - include hidden files,
    // LF_HIDE = 1 - suppress hidden files
    f->flags |= LF_HIDE;  // Turn hide flag on
    f->f_lnk_dir = false; // By default, symbolic links are not followed. Use -L
                          // to follow them.
    char tmp_str[MAXLEN];
    argp_parse(&argp, argc, argv, 0, 0, f);
    if (f->argc > 0) {
        strnz__cpy(tmp_str, f->args[0], MAXLEN - 1);
        expand_tilde(tmp_str, MAXLEN - 1);
        if (is_directory(tmp_str) || is_symlink_to_dir(tmp_str)) {
            strnz__cpy(f->base_path, tmp_str, MAXLEN - 1);
        } else if (is_valid_regex(f->args[0])) {
            strnz__cpy(f->re, f->args[0], MAXLEN - 1);
            f->flags |= LF_REGEX;
        } else {
            fprintf(
                stderr,
                "lf: arg1: '%s' is neither a directory nor a valid regex.\n",
                f->args[0]);
            exit(EXIT_FAILURE);
        }
    }
    if (f->argc > 1) {
        if (f->base_path[0] == '\0') {
            strnz__cpy(tmp_str, f->args[1], MAXLEN - 1);
            expand_tilde(tmp_str, MAXLEN - 1);
            if (is_directory(tmp_str) || is_symlink_to_dir(tmp_str))
                strnz__cpy(f->base_path, tmp_str, MAXLEN - 1);
            else if ((!(f->flags & LF_REGEX)) && is_valid_regex(f->args[1])) {
                strnz__cpy(f->re, f->args[1], MAXLEN - 1);
                f->flags |= LF_REGEX;
            } else {
                fprintf(stderr,
                        "lf: arg2: '%s' is neither a directory nor a valid "
                        "regular expression.\n",
                        f->args[1]);
                exit(EXIT_FAILURE);
            }
        }
    }
    if (f->base_path[0] == '\0')
        strnz__cpy(f->base_path, ".", MAXLEN - 1);
    if (f->f_sort) {
        /** If sorting is requested, execute the finder and pipe its output
         * to the sort command. We can achieve this by creating a child
         * process that runs the sort command, and redirecting the output of
         * the file finder to the input of the sort command using a pipe.
         * The parent process will run the finder and write its output to
         * the pipe, while the child process will read from the pipe and
         * execute the sort command. */
        char *eargv[MAXARGS];
        int eargc = 0;
        eargv[eargc++] = strdup("sort");
        if (f->f_reverse)
            eargv[eargc++] = strdup("-r");
        eargv[eargc] = nullptr;
        int wstatus;
        int save_fd = dup(STDOUT_FILENO); // save current STDOUT
        int fds[2];
        pipe(fds); // Create the pipes
        pid_t pid1 = fork();
        if (pid1 == 0) {   // Child process
            close(fds[1]); // Close child write pipe
            dup2(fds[0],
                 STDIN_FILENO);      // Clone child read pipe to STDIN_FILENO
            close(fds[0]);           // Close child read pipe after cloning
            execvp(eargv[0], eargv); // Execute the external command
        }
        // Parent process
        close(fds[0]);               // Close read end of pipe
        dup2(fds[1], STDOUT_FILENO); // Clone write pipe to STDOUT_FILENO
        close(fds[1]);               // Close duplicated write pipe
        setvbuf(stdout, NULL, _IOLBF, 0);
        // line buffering is essential to ensure that output is flushed to
        // the sort process in a timely manner, preventing deadlocks and
        // ensuring that the sort process can start processing input as soon
        // as it is available. Without line buffering, the output may be
        // buffered until the buffer is full, which could lead to delays in
        // processing and potential deadlocks if the sort process is waiting
        // for input that hasn't been flushed yet.
        init_find(f); // Initialize and transfer control to the finder
        dup2(save_fd, STDOUT_FILENO); // restore STDOUT
        waitpid(pid1, &wstatus, 0);
    } else
        init_find(f); // Initialize and transfer control to the finder
    exit(EXIT_SUCCESS);
}
/** @brief Initialize the file search based on the provided SearchFilters
   and start finder threads.
    @param f A pointer to a SearchFilters struct containing the options and
   flags for filtering.
    @return true if the search was initialized successfully, false if an
   error occurred during initialization (e.g., regex compilation failure).
    @details This function processes the flags and options specified in the
   SearchFilters struct to set up the search criteria. It compiles any
   regular expressions provided by the user and initializes the queue with
   the base directory. It then creates a number of finder threads equal to
   the number of CPU cores to perform the directory traversal and file
   scanning concurrently. The function waits for all finder threads to
   complete before cleaning up resources and returning.
   */
bool init_find(SearchFilters *f) {
    if (f->flags & LF_USER)
        f->user_id = f->flags >> 24 & 0xffff;
    f->include_types = f->flags >> 8 & 0xff;
    /** suppress file types that aren't included */
    if (f->include_types)
        f->suppress_types = f->include_types ^ 0xff;
    f->permissions = f->flags >> 16 & 0xff;
    // LF_HIDE = 0 - include hidden files,
    // LF_HIDE = 1 - suppress hidden files
    f->f_include_hidden = !(f->flags & LF_HIDE);
    int reti = 0;
    f->reg_flags = REG_EXTENDED;
    if (f->flags & LF_ICASE)
        f->reg_flags |= REG_ICASE;
    if (f->flags & LF_REGEX) {
        reti = regcomp(&f->compiled_re, f->re, f->reg_flags);
        if (reti) {
            fprintf(stderr, "lf: '%s' Invalid pattern\n", f->re);
            regfree(&f->compiled_re);
            return false;
        }
    }
    if (f->flags & LF_EXC_REGEX) {
        reti = regcomp(&f->compiled_ere, f->ere, f->reg_flags);
        if (reti) {
            fprintf(stderr, "lf: '%s' Invalid exclude pattern\n", f->ere);
            regfree(&f->compiled_ere);
            return false;
        }
    }
    int n = get_nprocs();
    if (nthreads == 0 && n > 6)
        nthreads = 6; // Limit default thread count to 6 to avoid excessive
                      // resource usage
    else if (nthreads < 0)
        n = max(1, n + nthreads); // Allow negative values to
                                  // specify threads to leave idle
    nthreads = min(n, max(1, nthreads));
    if (f->debug && (f->report_config || f->report_all)) {
        fprintf(stderr, "debug=%d\n", f->debug);
        fprintf(stderr, "  1-report_config %s\n",
                f->report_config ? "true" : "false");
        fprintf(stderr, "  2-report_info %s\n",
                f->report_info ? "true" : "false");
        fprintf(stderr, "  3-report_warnings %s\n",
                f->report_warnings ? "true" : "false");
        fprintf(stderr, "  4-report_errors %s\n",
                f->report_errors ? "true" : "false");
        fprintf(stderr, "  5-bad links %s\n",
                f->report_trace ? "true" : "false");
        fprintf(stderr, "  6-report_trace %s\n",
                f->report_trace ? "true" : "false");
        fprintf(stderr, "  7-report_all %s\n",
                f->report_all ? "true" : "false");
        fprintf(stderr, "Starting search in: %s\n", f->base_path);
        fprintf(stderr, "Using %d threads\n", nthreads);
        fprintf(stderr, " include_types=%08b\n", f->include_types);
        fprintf(stderr, "suppress_types=%08b\n", f->suppress_types);
        if (f->flags & LF_REGEX)
            fprintf(stderr, "Include regex: %s\n", f->re);
        if (f->flags & LF_EXC_REGEX)
            fprintf(stderr, "Exclude regex: %s\n", f->ere);
        if (f->flags & LF_USER)
            fprintf(stderr, "User ID filter: %ju\n", f->user_id);
        if (f->after)
            fprintf(stderr, "Modified after: %s", ctime(&f->after));
        if (f->before)
            fprintf(stderr, "Modified before: %s", ctime(&f->before));
        if (f->file_size_min)
            fprintf(stderr, "Minimum file size: %jd bytes\n",
                    (intmax_t)f->file_size_min);
        if (f->max_depth)
            fprintf(stderr, "Max depth: %d\n", f->max_depth);
        if (f->f_include_hidden)
            fprintf(stderr, "Include hidden files.\n");
        if (f->f_lnk_dir)
            fprintf(stderr, "Follow symbolic links.\n");
        if (f->include_types) {
            fprintf(stderr, "Included file types:");
            if (f->include_types & DT_BLK)
                fprintf(stderr, " block\n");
            if (f->include_types & DT_CHR)
                fprintf(stderr, " character\n");
            if (f->include_types & DT_DIR)
                fprintf(stderr, " directory\n");
            if (f->include_types & DT_FIFO)
                fprintf(stderr, " pipe\n");
            if (f->include_types & DT_LNK)
                fprintf(stderr, " link\n");
            if (f->include_types & DT_REG)
                fprintf(stderr, " regular\n");
            if (f->include_types & DT_SOCK)
                fprintf(stderr, " socket\n");
            if (f->include_types & DT_UNKNOWN)
                fprintf(stderr, " unknown\n");
        }
        if (f->report_config && !f->report_all)
            exit(EXIT_SUCCESS);
    }
    //--------------------------------------------------------------------
    // Create and enqueue the first TaskNode
    struct stat st;
    if (stat(f->base_path, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            TaskNode *child_task = malloc(sizeof(TaskNode));
            child_task->dir_path = strdup(f->base_path);
            child_task->depth = 0;
            child_task->next = NULL;
            child_task->history = malloc(sizeof(PathNode));
            child_task->history[0].dev = st.st_dev;
            child_task->history[0].ino = st.st_ino;
            enqueue_dir(child_task);
            pthread_t threads[nthreads];
            for (int i = 0; i < nthreads; i++) {
                pthread_create(&threads[i], NULL, finder, f);
            }
            pthread_mutex_lock(&queue_mutex);
            while (!shutdown) {
                pthread_cond_wait(&cond_var, &queue_mutex);
            }
            pthread_mutex_unlock(&queue_mutex);
            for (int i = 0; i < nthreads; i++)
                pthread_join(threads[i], NULL);
        } else {
            fprintf(stderr,
                    "Warning: Base path '%s' is not a directory. No "
                    "files will be found.\n",
                    f->base_path);
        }
    }
    //--------------------------------------------------------------------
    // End Program
    if (f->flags & LF_REGEX)
        regfree(&f->compiled_re);
    if (f->flags & LF_EXC_REGEX)
        regfree(&f->compiled_ere);
    free(f);
    if (reti)
        return false;
    return true;
}
/** @brief Enqueue a directory dir_path for processing by finder threads.
    @param dir_path The directory path to enqueue.
    @param depth The current depth in the directory tree for the given path.
    @details This function creates a new TaskNode containing the specified
   directory path and depth, and adds it to the global queue. It uses a
   mutex to ensure thread-safe access to the queue, and signals finder
   threads that new work is available. The function also checks for shutdown
   conditions to prevent enqueuing new work when the program is exiting.
   */
void enqueue_dir(TaskNode *new_task) {
    pthread_mutex_lock(&queue_mutex);
    if (tail)
        tail->next = new_task;
    else
        head = new_task;
    tail = new_task;
    pthread_cond_signal(&cond_var);
    pthread_mutex_unlock(&queue_mutex);
}
/** @brief Dequeue a directory dir_path for processing by finder threads.
    @return A pointer to a TaskNode containing the directory dir_path and
   depth, or NULL if the queue is empty and shutdown has been initiated.
    @details This function removes and returns the next TaskNode from the
   global queue. It uses a mutex to ensure thread-safe access to the queue,
   and waits on a condition variable if the queue is empty. The function
   also checks for shutdown conditions to allow finder threads to exit
   gracefully when there is no more work to process.
   */
TaskNode *dequeue_dir() {
    pthread_mutex_lock(&queue_mutex);
    while (head == NULL && !shutdown) {
        if (atomic_load(&active_tasks) == 0) {
            shutdown = 1;
            pthread_cond_broadcast(&cond_var);
            break;
        }
        pthread_cond_wait(&cond_var, &queue_mutex);
    }
    if (shutdown && head == NULL) {
        pthread_mutex_unlock(&queue_mutex);
        return NULL;
    }
    TaskNode *temp = head;
    head = head->next;
    if (!head)
        tail = NULL;
    atomic_fetch_add(&active_tasks, 1);
    pthread_mutex_unlock(&queue_mutex);
    return temp;
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
   synchronization and shutdown when all work is complete.
   */
void *finder(void *arg) {
    SearchFilters *f = (SearchFilters *)arg;
#ifdef LF_DEBUG
    char lnk_path[PATH_MAX] = {'\0'};
#endif
    // regmatch_t pmatch;

    while (1) {
        TaskNode *current_task = dequeue_dir();
        if (!current_task)
            break;

        //-------------------------------------------------------------
        // Open the directory using openat and fdopendir to minimize
        // redundant path Open
        int dir_fd =
            openat(AT_FDCWD, current_task->dir_path, O_RDONLY | O_DIRECTORY);
        if (dir_fd < 0) {
#ifdef LF_DEBUG
            if (f->debug && (f->report_warnings || f->report_errors ||
                             f->report_badlinks || f->report_all))
                fprintf(stderr, "\nopenat failed for directory: %s, %s\n",
                        current_task->dir_path, strerror(errno));
#endif
            free(current_task->dir_path);
            free(current_task->history);
            free(current_task);
            atomic_fetch_sub(&active_tasks, 1);
            pthread_cond_broadcast(&cond_var);
            continue;
        }
        DIR *dir = fdopendir(dir_fd);
        if (dir == NULL) {
#ifdef LF_DEBUG
            if (f->debug && (f->report_warnings || f->report_errors ||
                             f->report_badlinks || f->report_all))
                fprintf(stderr, "\nfdopendir failed for directory: %s, %s\n",
                        current_task->dir_path, strerror(errno));
#endif
            close(dir_fd);
            free(current_task->dir_path);
            free(current_task->history);
            free(current_task);
            atomic_fetch_sub(&active_tasks, 1);
            pthread_cond_broadcast(&cond_var);
            continue;
        }
        char *p;
        unsigned char real_type;
        unsigned char effective_type;
        //-------------------------------------------------------------
        // Read directory contents and process each entry
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            struct stat st;
            real_type = '\0';
            effective_type = '\0';
            char full_path[PATH_MAX] = {'\0'};
            if (entry->d_name[0] == '.') {
                if (entry->d_name[1] == '\0')
                    continue;
                if (entry->d_name[1] == '.' && entry->d_name[2] == '\0')
                    continue;
                if (!f->f_include_hidden)
                    continue;
            }
            // Get link's metadata
            if (fstatat(dir_fd, entry->d_name, &st, AT_SYMLINK_NOFOLLOW) != 0) {
#ifdef LF_DEBUG
                if (f->debug && (f->report_errors || f->report_warnings ||
                                 f->report_badlinks || f->report_all))
                    fprintf(stderr, "BADLNK, %s/%s\n", current_task->dir_path,
                            entry->d_name);
#endif
                continue;
            }
            if (S_ISLNK(st.st_mode)) {
                real_type = DT_LNK;
                // Get the target's metadata
                if (fstatat(dir_fd, entry->d_name, &st, 0) != 0) {
#ifdef LF_DEBUG
                    if (f->debug && (f->report_all || f->report_warnings ||
                                     f->report_errors || f->report_badlinks)) {
                        ssnprintf(full_path, PATH_MAX - 1, "%s/%s",
                                  current_task->dir_path, entry->d_name);
                        fprintf(stderr, "BADTGT, %s\n", full_path);
                        ssize_t len =
                            readlink(full_path, lnk_path, sizeof(lnk_path) - 1);
                        if (len != -1) {
                            lnk_path[len] = '\0';
                            fprintf(stderr, " => %s", lnk_path);
                        }
                        fprintf(stderr, "\n");
                    }
#endif
                    // continue;
                }
            }
            switch (st.st_mode & S_IFMT) {
            case S_IFBLK:
                effective_type = DT_BLK;
                break;
            case S_IFCHR:
                effective_type = DT_CHR;
                break;
            case S_IFDIR:
                if (!f->f_lnk_dir && real_type == DT_LNK)
                    effective_type = DT_LNK;
                else
                    effective_type = DT_DIR;
                break;
            case S_IFIFO:
                effective_type = DT_FIFO;
                break;
            case S_IFLNK:
                effective_type = DT_LNK;
                break;
            case S_IFREG:
                effective_type = DT_REG;
                break;
            case S_IFSOCK:
                effective_type = DT_SOCK;
                break;
            default:
                effective_type = 0;
                break;
            }
            p = full_path;
            p = stpcpy(p, current_task->dir_path);
            p = stpcpy(p, "/");
            stpcpy(p, entry->d_name);
            if (effective_type == DT_DIR) {
                if (f->max_depth != 0 && current_task->depth == f->max_depth)
                    continue;
                // Check for cycles by comparing the current
                // directory's dev/inode with the history of
                // dev/inode pairs from parent directories. If a
                // match is found, it indicates a cycle and we skip
                // processing this directory.
                bool cycle_found = false;
#ifdef LF_DEBUG
                if (f->debug && (f->report_trace || f->report_all))
                    fprintf(stderr, "Checking for cycles in: %s\n", full_path);
#endif
                for (int i = 0; i < current_task->depth; i++) {
#ifdef LF_DEBUG
                    if (f->debug && (f->report_trace || f->report_all)) {

                        if (current_task->history[i].ino == st.st_ino)
                            fprintf(stderr, "%3d %ju %ju<===========\n", i,
                                    current_task->history[i].ino, st.st_ino);
                        else
                            fprintf(stderr, "%3d %ju %ju\n", i,
                                    current_task->history[i].ino, st.st_ino);
                    }
#endif
                    if (current_task->history[i].dev == st.st_dev &&
                        current_task->history[i].ino == st.st_ino) {
                        cycle_found = true;
                        break;
                    }
                }
                if (cycle_found) {
#ifdef LF_DEBUG
                    if (f->debug && (f->report_warnings || f->report_errors ||
                                     f->report_trace || f->report_badlinks ||
                                     f->report_all)) {
                        fprintf(stderr, "CYCLIC, %s", full_path);
                        ssize_t len =
                            readlink(full_path, lnk_path, sizeof(lnk_path) - 1);
                        if (len != -1) {
                            lnk_path[len] = '\0';
                            fprintf(stderr, " => %s", lnk_path);
                        }
                        fprintf(stderr, "\n");
                    }
#endif
                    continue;
                }
                //-------------------------------------------------------
                // Prepare the new history array for the next level
                // of recursion, copying the current history and
                // adding the current directory's dev/ino array
                TaskNode *child_task = malloc(sizeof(TaskNode));
                child_task->dir_path = strdup(full_path);
                child_task->depth = current_task->depth + 1;
                child_task->next = NULL;
                child_task->history =
                    malloc(sizeof(PathNode) * child_task->depth);
                if (current_task->depth > 0) {
                    memcpy(child_task->history, current_task->history,
                           sizeof(PathNode) * current_task->depth);
                }
                // Here, we use the dev and inode of the link itself
                // This may be an error
                child_task->history[current_task->depth].dev = st.st_dev;
                child_task->history[current_task->depth].ino = st.st_ino;
                enqueue_dir(child_task);
            }
            scan_file(full_path, f, effective_type);
        }
        closedir(dir);
        free(current_task->dir_path);
        free(current_task->history);
        free(current_task);
        atomic_fetch_sub(&active_tasks, 1);
        pthread_cond_broadcast(&cond_var);
    }
    return NULL;
}
/** @brief Scan a single file against the search filters and print
   if it matches.
    @param file_spec The full specification of the file being
   scanned.
    @param f The SearchFilters struct containing the options and
   flags for filtering.
    @param dir_st The dirent struct representing the file being
   scanned.
    @return true if the file was processed successfully, false if an
   error occurred.
    @details This function checks the specified file against the
   various filters defined in the SearchFilters struct, including
   file type, regex patterns, ownership, permissions, modification
   time, and size. If the file matches all criteria, its path is
   printed to standard output. If any errors occur during processing
   (e.g., regex compilation failure), an error message is printed to
   standard error and the function returns false.
    @note This function is called by finder threads for each file
   encountered during the directory traversal. It is designed to be
   thread-safe and efficient, minimizing redundant system calls. For
   example, it only calls stat() when necessary for filters that
   require file metadata, and it caches the results to avoid
   multiple stat() calls for the same file.
    @note regex matching is opttimized by pre-compiling the regular
   expressions in init_find() and using regexec() for each file,
   which is more efficient than compiling the regex for each file.
   Additionally, the function minimizes calls to regexec() by first
   checking simpler filters (like file type and hidden status)
   before performing regex matching, thus avoiding unnecessary regex
   evaluations for files that are already excluded by other
   criteria.
    @todo Can we come up with a more efficient data structure for
   managing the queue of directories to process, such as a lock-free
   queue or work-stealing deque? A work stealing dequeue would allow
   idle threads to steal work from busier threads, improving load
   balancing and reducing idle time. This would involve implementing
   a thread-safe deque data structure that supports concurrent push
   and pop operations from both ends, allowing threads to
   efficiently share the workload without excessive locking or
   contention.
   */
int scan_file(char *file_spec, SearchFilters *f, unsigned char effective_type) {

    regmatch_t pmatch[2];
    bool f_stat = false;

    while (1) {
        if (f->suppress_types) {
            if (f->suppress_types & DT_REG & effective_type)
                break;
            if (f->suppress_types & DT_DIR & effective_type)
                break;
            if (f->suppress_types & DT_LNK & effective_type)
                break;
            if (f->suppress_types & DT_BLK & effective_type)
                break;
            if (f->suppress_types & DT_CHR & effective_type)
                break;
            if (f->suppress_types & DT_FIFO & effective_type)
                break;
            if (f->suppress_types & DT_SOCK & effective_type)
                break;
        }
        /* Exclude non-matching files */
        if (f->flags & LF_REGEX) {
            int reti =
                regexec(&f->compiled_re, file_spec, f->compiled_re.re_nsub + 1,
                        pmatch, f->reg_flags);
            if (reti == REG_NOMATCH) {
#ifdef LF_DEBUG
                if (f->debug && (f->report_info || f->report_all))
                    fprintf(stderr, "Regex no match: %s\n", file_spec);
#endif
                break;
            }
#ifdef LF_DEBUG
            else if (reti) {
                char errbuf[MAXLEN];
                regerror(reti, &f->compiled_re, errbuf, sizeof(errbuf));
                if (f->debug && (f->report_errors || f->report_all))
                    fprintf(stderr, "Regex error: %s\n", errbuf);
                break;
            }
#endif
        }
        /* Exclude matching files */
        if (f->flags & LF_EXC_REGEX) {
            int reti =
                regexec(&f->compiled_ere, file_spec, f->compiled_re.re_nsub + 1,
                        pmatch, f->reg_flags);
            if (reti == 0) // Match
                break;
#ifdef LF_DEBUG
            else if (reti) {
                char errbuf[MAXLEN];
                regerror(reti, &f->compiled_ere, errbuf, sizeof(errbuf));
                if (f->debug && (f->report_errors || f->report_all))
                    fprintf(stderr, "Exclude regex error: %s\n", errbuf);
                break;
            }
#endif
        }
        f_stat = false;
        struct stat sb;
        /** Exclude files not owned by specified user */
        if ((f->flags & LF_USER) && stat(file_spec, &sb) == 0) {
            f_stat = true;
            if (sb.st_uid != f->user_id)
                break;
        }
        if (f->permissions) {
            if (!f_stat) {
                if (stat(file_spec, &sb) == 0)
                    f_stat = true;
            }
            if ((f->permissions & LF_PERM_R) && !(sb.st_mode & S_IRUSR))
                break;
            else if ((f->permissions & LF_PERM_W) && !(sb.st_mode & S_IWUSR))
                break;
            else if ((f->permissions & LF_PERM_X) && !(sb.st_mode & S_IXUSR))
                break;
            else if ((f->permissions & LF_SETUID) && !(sb.st_mode & S_ISUID))
                break;
            else if ((f->permissions & LF_SETGID) && !(sb.st_mode & S_ISGID))
                break;
        }
        if (f->before) {
            if (!f_stat) {
                if (stat(file_spec, &sb) == 0)
                    f_stat = true;
            }
            if (f_stat && sb.st_mtime > f->before)
                break;
        }
        if (f->after) {
            if (!f_stat) {
                if (stat(file_spec, &sb) == 0)
                    f_stat = true;
            }
            if (f_stat && sb.st_mtime < f->after)
                break;
        }
        if (f->file_size_min) {
            if (!f_stat) {
                if (stat(file_spec, &sb) == 0)
                    f_stat = true;
            }
            if (f_stat && sb.st_size < f->file_size_min)
                break;
        }
        if (effective_type == DT_DIR) {
            char *file_p = file_spec;
            while (*file_p++ != '\0')
                ;
            *file_p = '/';
        }
        if (file_spec[0] == '.' && file_spec[1] == '/')
            printf("%s\n", &file_spec[2]);
        else
            printf("%s\n", file_spec);
        break;
    }
    return true;
}
