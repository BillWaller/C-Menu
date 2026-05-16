/** @file lf.c
    @brief list files matching a regular expression
    @author Bill Waller
    Copyright (c) 2025
    MIT License
    billxwaller@gmail.com
    @date 2026-02-09
 */
#define _GNU_SOURCE

#include <argp.h>
#include <cm.h>
#include <dirent.h>
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
int threads;

static struct argp_option options[] = {
    {"after", 'a', "time", 0, "Modified after YYYY-MM-DDTHH:MM:SS", 0},
    {"before", 'b', "time", 0, "Modified before YYYY-MM-DDTHH:MM:SS", 0},
    {"max_depth", 'd', "number", 0, "Depth into directory tree", 0},
    {"ere", 'e', "regex", 0, "Exclude regular expression", 0},
    {"f_ignore_case", 'i', 0, 0, "Search ignore case", 0},
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
    {"debug", 'D', "level", OPTION_ARG_OPTIONAL, "1-info-only, 2-run", 0},
    {"f_hidden", 'H', 0, 0, "Include hidden files", 0},
    {"f_lnk_dir", 'L', 0, 0, "Follow symbolic links", 0},
    {"threads", 'T', "threads", 0, "Number of threads", 0},
    {"f_sort", 'S', "r", OPTION_ARG_OPTIONAL, "sort arg=r reverse", 0},
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
    bool f_hidden;
    bool f_lnk_dir;
    char *file_types_p;
    int reg_flags;
    char *args[5];
    int argc;
    char exec[MAXLEN];
    int include_types;
    int suppress_types;
    bool include;
    int permissions;
    bool suppress_hidden;
    bool blk;
    bool chr;
    bool dir;
    bool fifo;
    bool lnk;
    bool reg;
    bool sock;
    bool unknown;
    int debug;
} SearchFilters;

typedef struct Node {
    char path[PATH_MAX]; /**< Directory path to process */
    int depth;           /**< Current depth in the directory tree */
    struct Node *next;   /**< Pointer to the next node in the queue */
} Node;                  /**< Queue Node (for work-stealing) */

bool init_find(SearchFilters *);
int scan_file(char *, SearchFilters *, struct dirent *);

void enqueue_dir(const char *, int);
Node *dequeue_dir();
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
    case 'D':
        if (arg && arg[0] != '\0')
            f->debug = a_toi(arg, &a_toi_error);
        else
            f->debug = 1;
        break;
    case 'e':
        strnz__cpy(f->ere, arg, MAXLEN - 1);
        f->flags |= LF_EXC_REGEX;
        break;
    case 'H':
        f->f_hidden = true;     // Include hidden files
        f->flags &= ~(LF_HIDE); // Don't hide hidden files
        break;
    case 'L':
        f->f_lnk_dir = true; // follow symbolic links
        break;
    case 'T':
        if (arg && arg[0] != '\0')
            threads = a_toi(arg, &a_toi_error);
        break;
    case 'i':
        f->flags |= LF_ICASE;
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
    case 's':
        long long ull = a_to_ull(arg);
        f->file_size_min = (intmax_t)ull;
        break;
    case 't':
        f->file_types_p = arg;
        while (f->file_types_p[i]) {
            switch (f->file_types_p[i++]) {
            case 'b':
                f->flags |= FT_BLK << 8;
                break;
            case 'c':
                f->flags |= FT_CHR << 8;
                break;
            case 'd':
                f->flags |= FT_DIR << 8;
                break;
            case 'p':
                f->flags |= FT_FIFO << 8;
                break;
            case 'l':
                f->flags |= FT_LNK << 8;
                break;
            case 'f': // for regular files, 'f' is more intuitive than 'r'
            case 'r': // regular files are the most common type, so 'r' is also
                      // accepted
                f->flags |= FT_REG << 8;
                break;
            case 's':
                f->flags |= FT_SOCK << 8;
                break;
            case 'u':
                f->flags |= FT_UNKNOWN << 8;
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
    case 'S':
        f->f_sort = true;
        if (arg && (arg[0] == 'r' || arg[0] == 'R'))
            f->f_reverse = true;
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
Node *head = NULL, *tail = NULL;
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_var = PTHREAD_COND_INITIALIZER;
atomic_int active_tasks = 0;
int shutdown = 0;

int main(int argc, char **argv) {

    SearchFilters *f = (SearchFilters *)calloc(1, sizeof(SearchFilters));
    f->f_ignore_case = false;
    f->f_sort = false;
    f->f_reverse = false;
    f->f_hidden = false;  // By default, hidden files are suppressed. Use -H to
                          // include them.
    f->flags |= LF_HIDE;  // Hide hidden files
    f->f_lnk_dir = false; // By default, symbolic links are not followed. Use -L
                          // to follow them.
    char tmp_str[MAXLEN];
    argp_parse(&argp, argc, argv, 0, 0, f);
    if (f->argc > 0) {
        strnz__cpy(tmp_str, f->args[0], MAXLEN - 1);
        expand_tilde(tmp_str, MAXLEN - 1);
        if (is_directory(tmp_str) || is_symlink_to_dir(tmp_str))
            strnz__cpy(f->base_path, tmp_str, MAXLEN - 1);
        else if (is_valid_regex(f->args[0])) {
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
            else if (is_valid_regex(f->args[1])) {
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
    f->suppress_hidden = f->flags & LF_HIDE ? true : false;
    f->blk = f->suppress_types & FT_BLK ? true : false;
    f->chr = f->suppress_types & FT_CHR ? true : false;
    f->dir = f->suppress_types & FT_DIR ? true : false;
    f->fifo = f->suppress_types & FT_FIFO ? true : false;
    f->lnk = f->suppress_types & FT_LNK ? true : false;
    f->reg = f->suppress_types & FT_REG ? true : false;
    f->sock = f->suppress_types & FT_SOCK ? true : false;
    f->unknown = f->suppress_types & FT_UNKNOWN ? true : false;
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
    int thread_count = get_nprocs();
    if (threads == 0 && thread_count > 6)
        threads = 6; // Limit default thread count to 6 to avoid excessive
                     // resource usage
    else if (threads < 0)
        thread_count =
            max(1, thread_count + threads); // Allow negative values to
                                            // specify threads to leave idle
    thread_count = min(thread_count, max(1, threads));
    if (f->debug) {
        fprintf(stderr, "lf: debug=%d\n", f->debug);
        fprintf(stderr, "Starting search in: %s\n", f->base_path);
        fprintf(stderr, "Using %d threads\n", thread_count);
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
        if (f->f_hidden)
            fprintf(stderr, "Include hidden files.\n");
        if (f->f_lnk_dir)
            fprintf(stderr, "Follow symbolic links.\n");
        if (f->include_types) {
            fprintf(stderr, "Included file types:");
            if (f->include_types & FT_BLK)
                fprintf(stderr, " block");
            if (f->include_types & FT_CHR)
                fprintf(stderr, " character");
            if (f->include_types & FT_CHR)
                fprintf(stderr, " character");
            if (f->include_types & FT_DIR)
                fprintf(stderr, " directory");
            if (f->include_types & FT_FIFO)
                fprintf(stderr, " pipe");
            if (f->include_types & FT_LNK)
                fprintf(stderr, " link");
            if (f->include_types & FT_REG)
                fprintf(stderr, " regular");
            if (f->include_types & FT_SOCK)
                fprintf(stderr, " socket");
            if (f->include_types & FT_UNKNOWN)
                fprintf(stderr, " unknown");
            fprintf(stderr, "\n");
        }
        if (f->debug == 1)
            exit(EXIT_SUCCESS);
    }
    //--------------------------------------------------------------------
    // read directories concurrently and list files
    enqueue_dir(f->base_path, 0);
    pthread_t threads[thread_count];
    for (int i = 0; i < thread_count; i++)
        pthread_create(&threads[i], NULL, finder, f);
    pthread_mutex_lock(&queue_mutex);
    while (!shutdown) {
        pthread_cond_wait(&cond_var, &queue_mutex);
    }
    pthread_mutex_unlock(&queue_mutex);
    for (int i = 0; i < thread_count; i++)
        pthread_join(threads[i], NULL);
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
/** @brief Enqueue a directory path for processing by finder threads.
    @param path The directory path to enqueue.
    @param depth The current depth in the directory tree for the given path.
    @details This function creates a new Node containing the specified
   directory path and depth, and adds it to the global queue. It uses a
   mutex to ensure thread-safe access to the queue, and signals finder
   threads that new work is available. The function also checks for shutdown
   conditions to prevent enqueuing new work when the program is exiting.
   */
void enqueue_dir(const char *path, int depth) {
    Node *new_node = malloc(sizeof(Node));
    strnz__cpy(new_node->path, path, PATH_MAX - 1);
    new_node->depth = depth;
    new_node->next = NULL;

    pthread_mutex_lock(&queue_mutex);
    if (tail)
        tail->next = new_node;
    else
        head = new_node;
    tail = new_node;
    pthread_cond_signal(&cond_var);
    pthread_mutex_unlock(&queue_mutex);
}
/** @brief Dequeue a directory path for processing by finder threads.
    @return A pointer to a Node containing the directory path and depth, or
   NULL if the queue is empty and shutdown has been initiated.
    @details This function removes and returns the next Node from the global
   queue. It uses a mutex to ensure thread-safe access to the queue, and
   waits on a condition variable if the queue is empty. The function also
   checks for shutdown conditions to allow finder threads to exit gracefully
   when there is no more work to process.
   */
Node *dequeue_dir() {
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
    Node *temp = head;
    head = head->next;
    if (!head)
        tail = NULL;
    pthread_mutex_unlock(&queue_mutex);
    return temp;
}
/** @brief Worker thread function to process directories from the queue.
    @param arg Pointer to the SearchFilters struct containing the options
   and flags for filtering.
    @return NULL
    @details This function continuously dequeues directory paths from the
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
    struct stat st;
    char full_path[PATH_MAX] = {'\0'};
    // regmatch_t pmatch;

    while (1) {
        Node *current = dequeue_dir();
        if (!current)
            break;

        atomic_fetch_add(&active_tasks, 1);
        int dir_fd =
            openat(AT_FDCWD, current->path, O_RDONLY | O_DIRECTORY | O_NOATIME);
        if (dir_fd < 0) {
            fprintf(stderr, "openat failed for directory: %s\n", current->path);
            close(dir_fd);
            free(current);
            atomic_fetch_sub(&active_tasks, 1);
            continue;
        }
        DIR *dir = fdopendir(dir_fd);
        if (dir == NULL) {
            fprintf(stderr, "fdopen failed for directory: %s\n", current->path);
            closedir(dir);
            close(dir_fd);
            free(current);
            atomic_fetch_sub(&active_tasks, 1);
            continue;
        }
        if (dir) {
            struct dirent *entry;
            while ((entry = readdir(dir)) != NULL) {
                if (entry->d_name[0] == '.') {
                    if (entry->d_name[1] == '\0')
                        continue;
                    if (entry->d_name[1] == '.' && entry->d_name[2] == '\0')
                        continue;
                }
                if (entry->d_type == DT_LNK) {
                    if (fstatat(dir_fd, entry->d_name, &st, 0) == 0)
                        if (S_ISDIR(st.st_mode) && f->f_lnk_dir)
                            entry->d_type = DT_DIR;
                }
                if (entry->d_type == DT_DIR) {
                    if ((f->flags & LF_HIDE) && (entry->d_name[0] == '.'))
                        continue;
                    if (f->max_depth > 0 && current->depth + 1 >= f->max_depth)
                        continue;
                }
                ssnprintf(full_path, PATH_MAX - 1, "%s/%s", current->path,
                          entry->d_name);
                if (entry->d_type == DT_DIR)
                    enqueue_dir(full_path, current->depth + 1);
                scan_file(full_path, f, entry);
            }
        }
        closedir(dir);
        free(current);
        atomic_fetch_sub(&active_tasks, 1);
        // Wake up threads checking for idle exit
        pthread_cond_broadcast(&cond_var);
    }
    return NULL;
}
/** @brief Scan a single file against the search filters and print if it
   matches.
    @param file_spec The full specification of the file being scanned.
    @param f The SearchFilters struct containing the options and flags for
   filtering.
    @param dir_st The dirent struct representing the file being scanned.
    @return true if the file was processed successfully, false if an error
   occurred.
    @details This function checks the specified file against the various
   filters defined in the SearchFilters struct, including file type, regex
   patterns, ownership, permissions, modification time, and size. If the
   file matches all criteria, its path is printed to standard output. If any
   errors occur during processing (e.g., regex compilation failure), an
   error message is printed to standard error and the function returns
   false.
    @note This function is called by finder threads for each file
   encountered during the directory traversal. It is designed to be
   thread-safe and efficient, minimizing redundant system calls. For
   example, it only calls stat() when necessary for filters that require
   file metadata, and it caches the results to avoid multiple stat() calls
   for the same file.
    @note regex matching is opttimized by pre-compiling the regular
   expressions in init_find() and using regexec() for each file, which is
   more efficient than compiling the regex for each file. Additionally, the
   function minimizes calls to regexec() by first checking simpler filters
   (like file type and hidden status) before performing regex matching, thus
   avoiding unnecessary regex evaluations for files that are already
   excluded by other criteria.
    @todo Can we come up with a more efficient data structure for managing
   the queue of directories to process, such as a lock-free queue or
   work-stealing deque? A work stealing dequeue would allow idle threads to
   steal work from busier threads, improving load balancing and reducing
   idle time. This would involve implementing a thread-safe deque data
   structure that supports concurrent push and pop operations from both
   ends, allowing threads to efficiently share the workload without
   excessive locking or contention.
   */
int scan_file(char *file_spec, SearchFilters *f, struct dirent *dir_st) {

    char tmp_str[MAXLEN];

    regmatch_t pmatch[2];
    bool f_stat = false;

    while (1) {
        if (dir_st->d_name[0] == '.' && f->suppress_hidden)
            break;
        if (f->suppress_types) {
            if (f->reg && (dir_st->d_type == DT_REG))
                break;
            if (f->dir && (dir_st->d_type == DT_DIR))
                break;
            if (f->lnk && (dir_st->d_type == DT_LNK))
                break;
            if (f->blk && (dir_st->d_type == DT_BLK))
                break;
            if (f->chr && (dir_st->d_type == DT_CHR))
                break;
            if (f->fifo && (dir_st->d_type == DT_FIFO))
                break;
            if (f->sock && (dir_st->d_type == DT_SOCK))
                break;
            if (f->unknown && (dir_st->d_type == DT_UNKNOWN))
                break;
        }
        /* Exclude non-matching files */
        if (f->flags & LF_REGEX) {
            int reti =
                regexec(&f->compiled_re, file_spec, f->compiled_re.re_nsub + 1,
                        pmatch, f->reg_flags);
            if (reti == REG_NOMATCH) {
                break;
            } else if (reti) {
                regerror(reti, &f->compiled_re, tmp_str, sizeof(tmp_str));
                strnz__cpy(tmp_str, "Regex match failed: ", MAXLEN - 1);
                strnz__cat(tmp_str, tmp_str, MAXLEN - 1);
                perror(tmp_str);
                break;
            }
        }
        /* Exclude matching files */
        if (f->flags & LF_EXC_REGEX) {
            int reti =
                regexec(&f->compiled_ere, file_spec, f->compiled_re.re_nsub + 1,
                        pmatch, f->reg_flags);
            if (reti == 0) // Match
                break;
            else if (reti) {
                regerror(reti, &f->compiled_re, tmp_str, sizeof(tmp_str));
                strnz__cpy(tmp_str, "Regex match failed: ", MAXLEN - 1);
                strnz__cat(tmp_str, tmp_str, MAXLEN - 1);
                perror(tmp_str);
            }
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
        if (file_spec[0] == '.' && file_spec[1] == '/')
            printf("%s\n", &file_spec[2]);
        else
            printf("%s\n", file_spec);
        break;
    }
    return true;
}
