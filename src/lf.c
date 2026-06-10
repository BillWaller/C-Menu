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
#include <argp.h>
#include <cm.h>
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
static char args_doc[] = "[DIRECTORY] [REGULAR_EXPRESSION]";
unsigned int nthreads;

// Search Filters
typedef struct {
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
    bool follow_links;
    bool debug;
    bool report_config;
    bool report_info;
    bool report_warnings;
    bool report_errors;
    bool report_badlinks;
    bool report_trace;
    bool report_all;
    bool only_errors;
} SearchFilters;

#define DT_LNK_DIR 14
unsigned char const lf_mask[15] = {
    0, 0b00000001, 0b00000010, 0, 0b00000100, 0, 0b00001000, 0, 0b00010000, 0,
    0b00100000, 0, 0b01000000, 0, 0b10000000};
typedef struct {
    dev_t dev;
    ino_t ino;
} History;

typedef struct TaskNode TaskNode;
struct TaskNode {
    History *history;    /**< Array of dev/ino pairs for cycle detection */
    TaskNode *next_task; /**< Pointer to the next node in the queue */
    int depth;           /**< Current depth in the directory tree */
    char *dir_path;      /**< Directory path to process */
}; /**< Queue TaskNode (for work-stealing) */

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
TaskNode *qhead = NULL;
TaskNode *qtail = NULL;
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_var = PTHREAD_COND_INITIALIZER;
atomic_int active_tasks = 0;
int shutdown = 0;
int termination_status = EXIT_SUCCESS;
int lfargc;
char *lfargs[3];
char *exec;
char *file_types_p;
char *perms_p;
char *debug_p;
void debug_out(SearchFilters *, int, char **, int);
bool init_find(SearchFilters *, int, char **);
void sort_lf_output(SearchFilters *, int, char **);
void enqueue_dir(TaskNode *);
TaskNode *dequeue_dir();
void *finder(void *);
int scan_file(char *, const SearchFilters *, const unsigned char);
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
    {"include_hidden", 'H', 0, 0, "Include hidden files", 0},
    {"follow_links", 'L', 0, 0, "Follow symbolic links", 0},
    {"sort_reverse", 'R', 0, 0, "Sort in Reverse order", 0},
    {"sort", 'S', 0, 0, "Sort in Ascending order", 0},
    {"nthreads", 'T', "threads", 0, "Number of nthreads", 0},
    {0}};

/** @brief Parse a single option.  */
static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    SearchFilters *f = state->input;
    int i = 0;
    bool a_toi_error = false;

    switch (key) {
    case 'a':
        parse_local_timestamp(arg, &f->after);
        if (f->after && f->before && f->before < f->after) {
            fprintf(stderr, "-b time must be greater than -a time.\n");
            f->after = 0;
        }
        break;
    case 'b':
        parse_local_timestamp(arg, &f->before);
        if (f->after && f->before && f->before < f->after) {
            fprintf(stderr, "-b time must be greater than -a time.\n");
            f->before = 0;
        }
        break;
    case 'd':
        if (arg && arg[0] != '\0')
            f->max_depth = a_toi(arg, &a_toi_error);
        break;
    case 'D':
        f->debug = true;
        if (arg && arg[0] != '\0') {
            debug_p = strdup(arg);
            i = 0;
            while (debug_p[i]) {
                switch (debug_p[i]) {
                case '1':
                    f->report_config = true;
                    break;
                case '2':
                    f->report_info = true;
                    break;
                case '3':
                    f->report_warnings = true;
                    break;
                case '4':
                    f->report_errors = true;
                    break;
                case '5':
                    f->report_badlinks = true;
                    break;
                case '6':
                    f->report_trace = true;
                    break;
                case '7':
                    f->report_config = true;
                    f->report_info = true;
                    f->report_warnings = true;
                    f->report_errors = true;
                    f->report_badlinks = true;
                    f->report_all = true;
                    break;
                case '8':
                    f->only_errors = true;
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
        f->ere = strdup(arg);
        f->flags |= LF_EXC_REGEX;
        break;
    case 'H':
        f->include_hidden = true; // Include hidden files
        f->flags &= ~(LF_HIDE);   // Turn hide flag off
                                  // LF_HIDE = 0 - include hidden files,
                                  // LF_HIDE = 1 - suppress hidden files
        break;
    case 'i':
        f->flags |= LF_ICASE;
        break;
    case 'L':
        f->follow_links = true; // follow symbolic links
        break;
    case 'p':
        perms_p = arg;
        while (perms_p[i]) {
            switch (perms_p[i++]) {
            case 'g':
                f->include_perms |= LF_ISGID;
                break;
            case 'r':
                f->include_perms |= LF_IRUSR;
                break;
            case 's':
                f->include_perms |= LF_ISUID;
                break;
            case 'w':
                f->include_perms |= LF_IWUSR;
                break;
            case 'x':
                f->include_perms |= LF_IXUSR;
                break;
            default:
                break;
            }
        }
        break;
    case 'R':
        f->sort_reverse = true;
        break;
    case 'r':
        f->re = strdup(arg);
        f->flags |= LF_REGEX;
        break;
    case 'S':
        f->sort = true;
        break;
    case 's':
        f->file_size_min = (intmax_t)(a_to_ul(arg));
        break;
    case 'T':
        if (arg && arg[0] != '\0')
            nthreads = a_toi(arg, &a_toi_error);
        break;
    case 't':
        file_types_p = arg;
        while (file_types_p[i]) {
            switch (file_types_p[i++]) {
            case 'b':
                f->include_types |= LF_BLK;
                break;
            case 'c':
                f->include_types |= LF_CHR;
                break;
            case 'd':
                f->include_types |= LF_DIR;
                break;
            case 'p':
                f->include_types |= LF_FIFO;
                break;
            case 'l':
                f->include_types |= LF_LNK;
                break;
            case 'f': // for regular files, 'f' is more intuitive than 'r'
            case 'r': // regular files are the most common type, so 'r' is also
                      // accepted
                f->include_types |= LF_REG;
                break;
            case 's':
                f->include_types |= LF_SOCK;
                break;
            case 'u':
                f->include_types |= LF_UNKNOWN;
                break;
            default:
                break;
            }
        }
        break;
    case 'u':
        f->user_name = strdup(arg);
        struct passwd *pwd = getpwnam(arg);
        if (pwd) {
            f->user_id = (uintmax_t)pwd->pw_uid;
            f->flags |= LF_USER;
        } else {
            fprintf(stderr, "User '%s' not found.\n", arg);
            exit(EXIT_FAILURE);
        }
        break;
#ifdef EXPERIMENTAL
    case 'x':
        f->exec = strdup(arg);
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

    SearchFilters *f = (SearchFilters *)calloc(1, sizeof(SearchFilters));
    f->ignore_case = false;
    f->sort = false;
    f->sort_reverse = false;
    f->include_hidden = false; // By default, hidden files are suppressed. Use
                               // -H to include them.
    // LF_HIDE = 0 - include hidden files,
    // LF_HIDE = 1 - suppress hidden files
    f->flags |= LF_HIDE;     // Turn hide flag on
    f->follow_links = false; // By default, symbolic links are not followed. Use
                             // -L to follow them.
    char tmp_str[PATH_MAX];
    argp_parse(&argp, argc, argv, 0, 0, f);
    if (lfargc > 0) {
        strnz__cpy(tmp_str, lfargs[0], MAXLEN - 1);
        expand_tilde(tmp_str, MAXLEN - 1);
        if (is_directory(tmp_str) || is_symlink_to_dir(tmp_str)) {
            f->base_path = strdup(tmp_str);
        } else if (is_valid_regex(lfargs[0])) {
            f->re = strdup(lfargs[0]);
            f->flags |= LF_REGEX;
        } else {
            fprintf(
                stderr,
                "lf: arg1: '%s' is neither a directory nor a valid regex.\n",
                lfargs[0]);
            exit(EXIT_FAILURE);
        }
    }
    if (lfargc > 1) {
        if (!f->base_path || f->base_path[0] == '\0') {
            strnz__cpy(tmp_str, lfargs[1], MAXLEN - 1);
            expand_tilde(tmp_str, MAXLEN - 1);
            if (is_directory(tmp_str) || is_symlink_to_dir(tmp_str))
                f->base_path = strdup(tmp_str);
        }
        if ((!(f->flags & LF_REGEX)) && is_valid_regex(lfargs[1])) {
            f->re = strdup(lfargs[1]);
            f->flags |= LF_REGEX;
        } else {
            fprintf(stderr,
                    "lf: '%s' is neither a directory nor a valid regular "
                    "expression.\n",
                    lfargs[1]);
            exit(EXIT_FAILURE);
        }
    }
    if (f->base_path == nullptr || f->base_path[0] == '\0')
        f->base_path = strdup(".");
    if (!f->sort) {
        init_find(f, argc, argv);
    } else
        sort_lf_output(f, argc, argv);
    exit(termination_status);
}
// Initialize and transfer control to the finder
/** If sorting is requested, execute the finder and pipe its output
 * to the sort command. We can achieve this by creating a child
 * process that runs the sort command, and redirecting the output of
 * the file finder to the input of the sort command using a pipe.
 * The parent process will run the finder and write its output to
 * the pipe, while the child process will read from the pipe and
 * execute the sort command. */
void sort_lf_output(SearchFilters *f, int argc, char **argv) {
    char *eargv[MAXARGS];
    int eargc = 0;
    eargv[eargc++] = strdup("sort");
    if (f->sort_reverse)
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
    dup2(save_fd, STDOUT_FILENO); // restore STDOUT
    waitpid(pid1, &wstatus, 0);
    init_find(f, argc, argv); // Initialize and transfer control to the finder
}
/** @brief Initialize the file search based on the provided SearchFilters
   and start finder threads.
    @param f A pointer to a SearchFilters struct containing the options and
   flags for filtering.
    @param argc The number of command-line arguments.
    @param argv The array of command-line argument strings.
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
bool init_find(SearchFilters *f, int argc, char **argv) {
    /** suppress file types that aren't included */
    if (!f->include_types)
        f->include_types = 0xff;
    if (f->include_types)
        f->suppress_types = f->include_types ^ 0xff;
    // LF_HIDE = 0 - include hidden files,
    // LF_HIDE = 1 - suppress hidden files
    f->include_hidden = !(f->flags & LF_HIDE);
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
    unsigned int nprocs = get_nprocs();
    if (nthreads > 1 && nthreads < nprocs)
        nthreads += nprocs;
    else {
        if (nthreads == 0)
            nthreads = ((nprocs * 40) / 99) + 1;
    }
    if (nthreads > nprocs)
        nthreads = nprocs - 1;

    debug_out(f, argc, argv, nthreads);
    //--------------------------------------------------------------------
    // Create and enqueue the first TaskNode
    struct stat st;
    if (stat(f->base_path, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            TaskNode *child_task = malloc(sizeof(TaskNode));
            child_task->dir_path = strdup(f->base_path);
            child_task->depth = 0;
            child_task->next_task = NULL;
            child_task->history = malloc(sizeof(History));
            child_task->history[0].dev = st.st_dev;
            child_task->history[0].ino = st.st_ino;
            enqueue_dir(child_task);
            pthread_t threads[nthreads];
            for (unsigned int i = 0; i < nthreads; i++) {
                pthread_create(&threads[i], NULL, finder, f);
            }
            pthread_mutex_lock(&queue_mutex);
            while (!shutdown) {
                pthread_cond_wait(&cond_var, &queue_mutex);
            }
            pthread_mutex_unlock(&queue_mutex);
            for (unsigned int i = 0; i < nthreads; i++)
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
    if (f->flags & LF_REGEX) {
        regfree(&f->compiled_re);
    }
    if (f->flags & LF_EXC_REGEX) {
        regfree(&f->compiled_ere);
    }
    free(f->base_path);
    free(f->user_name);
    free(f->re);
    free(f->ere);
    free(f);
    if (reti)
        return false;
    return true;
}
/** @brief Output debug information about the search filters and configuration.
    @param f A pointer to a SearchFilters struct containing the options and
   flags for filtering.
    @param argc The number of command-line arguments.
    @param argv The array of command-line argument strings.
    @param nthreads The number of threads being used for the search.
    @details This function prints detailed information about the configuration and search filters as well as each of the options and arguments used on the command line invoking lf. The output is sent to the standard error stream, which can be redirected to standard output making it suitable as documentation for an audit trail.
   */

void debug_out(SearchFilters *f, int argc, char **argv, int nthreads) {
    char user_str[100];
    char ip_str[MAXLEN];
    int len = 0;
    int i;
    bool addspace_before = false;
    if (f->debug && (f->report_config || f->report_info || f->report_all)) {
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
                f->debug ? "true" : "     false");
        fprintf(stderr, "  1-config      %s\n",
                f->report_config ? "true" : "|    false");
        fprintf(stderr, "  2-info        %s\n",
                f->report_info ? "true" : "|    false");
        fprintf(stderr, "  3-warnings    %s\n",
                f->report_warnings ? "true" : "|    false");
        fprintf(stderr, "  4-errors      %s\n",
                f->report_errors ? "true" : "|    false");
        fprintf(stderr, "  5-badlinks    %s\n",
                f->report_trace ? "true" : "|    false");
        fprintf(stderr, "  6-trace       %s\n",
                f->report_trace ? "true" : "|    false");
        fprintf(stderr, "  7-all         %s\n",
                f->report_all ? "true" : "|    false");
        fprintf(stderr, "  8-only_errors %s\n",
                f->only_errors ? "true" : "|    false");
        fprintf(stderr, "\n");
        fprintf(stderr, "Search directory: %s\n\n", f->base_path);
        fprintf(stderr, "Using %d threads\n\n", nthreads);

        fprintf(stderr, "File types preceeded by an asterisk (\"*\") will be included:\n\n");
        fprintf(stderr, "  LF type        DT type\n");
        print_file_type(f->include_types, LF_FIFO, DT_FIFO, "FIFO    p-named pipe");
        print_file_type(f->include_types, LF_CHR, DT_CHR, "CHR     c-character device");
        print_file_type(f->include_types, LF_DIR, DT_DIR, "DIR     d-directory");
        print_file_type(f->include_types, LF_BLK, DT_BLK, "BLK     b-block device");
        print_file_type(f->include_types, LF_REG, DT_REG, "REG     f-regular file");
        print_file_type(f->include_types, LF_LNK, DT_LNK, "LINK    l-symbolic link");
        print_file_type(f->include_types, LF_SOCK, DT_SOCK, "SOCK    s-socket");
        print_file_type(f->include_types, LF_UNKNOWN, DT_UNKNOWN, "UNKNOWN u-unknown");
        fprintf(stderr, "\n");
        fprintf(stderr, "f->include_types  = %08b\n", f->include_types);
        fprintf(stderr, "f->suppress_types = %08b\n", f->suppress_types);
        fprintf(stderr, "\n");
        if (f->flags & LF_USER)
            fprintf(stderr, "User: %s (%ju)\n", f->user_name, f->user_id);
        if (f->include_perms) {
            if (f->include_perms & LF_IXUSR)
                fprintf(stderr, "    %08b Execute\n", LF_IXUSR);
            if (f->include_perms & LF_IWUSR)
                fprintf(stderr, "    %08b Write\n", LF_IWUSR);
            if (f->include_perms & LF_IRUSR)
                fprintf(stderr, "    %08b Read\n", LF_IRUSR);
            if (f->include_perms & LF_ISUID)
                fprintf(stderr, "    %08b SETUID\n", LF_ISUID);
            if (f->include_perms & LF_ISGID)
                fprintf(stderr, "    %08b SETGID\n", LF_ISGID);
        }
        fprintf(stderr, "\n");
        if (f->flags & LF_REGEX)
            fprintf(stderr, "Include regex: %s\n\n", f->re);
        if (f->flags & LF_EXC_REGEX)
            fprintf(stderr, "Exclude regex: %s\n\n", f->ere);

        if (f->after) {
            char buf[32];
            format_local_timestamp(f->after, buf, sizeof(buf));
            fprintf(stderr, "Modified after: %s\n\n", buf);
        }

        if (f->before) {
            char buf[32];
            format_local_timestamp(f->before, buf, sizeof(buf));
            fprintf(stderr, "Modified before: %s\n\n", buf);
        }

        if (f->file_size_min) {
            const char *units[] = {"b", "Kb", "Mb", "Gb", "Tb", "Pb", "Eb"};
            off_t size = f->file_size_min;
            int i = 0;
            while (size >= 1024 && i < 6) {
                size /= 1024;
                i++;
            }
            char buffer[32];
            ssnprintf(buffer, 32, "%ld %s", size, units[i]);
            fprintf(stderr, "Minimum file size: %s\n\n", buffer);
        }
        if (f->max_depth)
            fprintf(stderr, "Max depth: %d\n\n", f->max_depth);
        if (f->ignore_case)
            fprintf(stderr, "Ignore case in regex matching.\n\n");
        if (f->include_hidden)
            fprintf(stderr, "Include hidden files.\n\n");
        if (f->follow_links)
            fprintf(stderr, "Follow symbolic links.\n\n");
        if (f->sort)
            fprintf(stderr, "Sort output in ascending order.\n\n");
        if (f->sort_reverse)
            fprintf(stderr, "Sort output in reverse order.\n\n");
        if (f->report_config && !f->report_all)
            exit(EXIT_SUCCESS);
    }
    return;
}
/** @brief Enqueue a directory dir_path for processing by finder threads.
    @param new_task A pointer to a TaskNode containing the directory path
 and depth to be enqueued for processing by finder threads.
    @details This function adds a new TaskNode to the global queue of
 directories to be processed by finder threads.
 // It uses a mutex to ensure thread-safe access to the queue and signals
 one waiting thread that a new task is available.
 // If shutdown has been initiated, the signal will wake up any waiting
 threads so they can check the shutdown condition and exit gracefully.
   */
void enqueue_dir(TaskNode *new_task) {
    pthread_mutex_lock(&queue_mutex);
    if (qtail)
        // Add the new task to the end of the queue. If qtail is not NULL,
        // it means there are already tasks in the queue, so we set the
        // next_task pointer of the current tail to point to the new task.
        // This effectively adds the new task to the end of the queue.
        qtail->next_task = new_task;
    else
        // If qtail is NULL, it means the queue is currently empty, so we
        // set qhead to point to the new task, making it the first and only
        // task in the queue
        qhead = new_task;
    // Finally, we update qtail to point to the new task, ensuring that it
    // always points to the last task in the queue.
    qtail = new_task;
    // Signal one waiting thread that a new task is available. If shutdown
    // hasn't been initiated, this will wake up a finder thread to process
    // the new task. If shutdown has been initiated, the signal will wake up
    // any waiting threads so they can check the shutdown condition and exit
    // gracefully.
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

    // Wait until there is a task in the queue or shutdown has been
    // initiated. The loop condition checks if the queue is empty (qhead ==
    // NULL) and if shutdown has not been initiated (!shutdown). If both
    // conditions are true, it means there are no tasks to process and the
    // thread should wait. The thread will be woken up when a new task is
    // enqueued (via pthread_cond_signal in enqueue_dir) or when shutdown is
    // initiated (via pthread_cond_broadcast in enqueue_dir or when
    // active_tasks count reaches zero). This ensures that threads do not
    // wait indefinitely when there are no tasks left to process and allows
    // for a graceful shutdown of the program.
    while (qhead == NULL && !shutdown) {
        // If there are no active tasks and the queue is empty, we can
        // safely initiate shutdown. This check is necessary to prevent a
        // potential race condition where a thread could be waiting
        // indefinitely on the condition variable if all tasks have been
        // completed and no new tasks will be enqueued. By checking the
        // active_tasks count, we can determine when it's safe to signal
        // shutdown
        if (atomic_load(&active_tasks) == 0) {
            shutdown = 1;
            // and wake up any waiting threads so they can exit gracefully.
            pthread_cond_broadcast(&cond_var);
            break;
        }
        // Wait for a task to be enqueued or shutdown initiated. The thread
        // will be woken up when a new task is added to the queue (via
        // pthread_cond_signal in enqueue_dir) or when shutdown is initiated
        // (via pthread_cond_broadcast in enqueue_dir or when active_tasks
        // count reaches zero). This allows the thread to check the
        // conditions again and either process a new task or exit if
        // shutdown has been initiated.
        pthread_cond_wait(&cond_var, &queue_mutex);
    }
    if (shutdown && qhead == NULL) {
        pthread_mutex_unlock(&queue_mutex);
        return NULL;
    }
    TaskNode *temp = qhead;
    // Move the head pointer to the next task in the queue. If the queue
    // becomes empty after this operation (qhead becomes NULL), we also set
    // qtail to NULL to indicate that the queue is empty. This ensures that
    // both qhead and qtail accurately reflect the state of the queue,
    // preventing potential issues with enqueuing new tasks or checking for
    // an empty queue in future operations.
    qhead = qhead->next_task;
    if (!qhead)
        qtail = NULL;
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
    char lnk_path[PATH_MAX] = {'\0'};
    // regmatch_t pmatch;

    while (1) {
        TaskNode *current_task = dequeue_dir();
        if (!current_task)
            break;

        //-------------------------------------------------------------
        // Open the directory for reading. We use openat with AT_FDCWD to
        // open the directory specified by current_task->dir_path, and then
        // use fdopendir to get a DIR* stream for reading the directory
        // entries. This approach allows us to handle directories with
        // special characters in their names more robustly, as it avoids
        // issues that can arise with functions like opendir that take a
        // path string directly. If openat or fdopendir fails, we log the
        // error (if debugging is enabled), clean up resources for the
        // current task, and continue to the next iteration of the loop to
        // process another task.
        int dir_fd =
            openat(AT_FDCWD, current_task->dir_path, O_RDONLY | O_DIRECTORY);
        if (dir_fd == -1) {
            if (f->debug && (f->report_warnings || f->report_errors ||
                             f->report_badlinks || f->report_all))
                fprintf(stderr, "OPEN_FAIL,%s,%s\n", current_task->dir_path,
                        strerror(errno));
            termination_status = EXIT_FAILURE;
            free(current_task->dir_path);
            free(current_task->history);
            free(current_task);
            atomic_fetch_sub(&active_tasks, 1);
            pthread_cond_broadcast(&cond_var);
            continue;
        }
        DIR *dir = fdopendir(dir_fd);
        if (dir == NULL) {
            if (f->debug && (f->report_warnings || f->report_errors ||
                             f->report_badlinks || f->report_all))
                fprintf(stderr, "\nFDOPENDIR_FAIL,%s,%s\n",
                        current_task->dir_path, strerror(errno));
            close(dir_fd);
            termination_status = EXIT_FAILURE;
            free(current_task->dir_path);
            free(current_task->history);
            free(current_task);
            atomic_fetch_sub(&active_tasks, 1);
            pthread_cond_broadcast(&cond_var);
            continue;
        }
        // unsigned char real_type;
        unsigned char effective_type;
        //-------------------------------------------------------------
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            struct stat st;
            // real_type = '\0';
            if (entry->d_name[0] == '.') {
                if (entry->d_name[1] == '\0')
                    continue;
                if (entry->d_name[1] == '.' && entry->d_name[2] == '\0')
                    continue;
                if (!f->include_hidden)
                    continue;
            }
            char full_path[PATH_MAX] = {'\0'};
            stpcpy(stpcpy(stpcpy(full_path, current_task->dir_path), "/"),
                   entry->d_name);
            // Get link's metadata
            int rc;
            // We use fstatat with AT_SYMLINK_NOFOLLOW to get the metadata
            // of the symbolic link itself, rather than the target it points
            // to. This allows us to determine if the entry is a symbolic
            // link and handle it according to the user's options (e.g.,
            // whether to follow links or not). If fstatat fails, we log the
            // error (if debugging is enabled) and continue to the next
            // entry without processing this one further.
            rc = fstatat(AT_FDCWD, full_path, &st, AT_SYMLINK_NOFOLLOW);
            if (rc == -1) {
                if (f->debug && (f->report_errors || f->report_warnings ||
                                 f->report_badlinks || f->report_all))
                    fprintf(stderr, "LSTAT_FAIL,%s,%s\n", full_path,
                            strerror(errno));
                termination_status = EXIT_FAILURE;
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
            if (S_ISLNK(st.st_mode)) {
                // Get the target's metadata
                rc = fstatat(AT_FDCWD, full_path, &st, 0);
                if (rc == -1) {
                    if (f->debug && (f->report_all || f->report_warnings ||
                                     f->report_errors || f->report_badlinks)) {
                        fprintf(stderr, "STAT_FAIL,%s,%s\n", full_path,
                                strerror(errno));
                    }
                    termination_status = EXIT_FAILURE;
                    continue;
                }
                if (f->follow_links)
                    effective_type = (st.st_mode & S_IFMT) >> 12;
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
            if (effective_type == DT_DIR) {
                if (f->max_depth != 0 && current_task->depth == f->max_depth)
                    continue;
                // Check for cycles by comparing the current
                // directory's dev/inode with the history of dev/inode pairs
                // from parent directories. If a match is found, it
                // indicates a cycle and we skip processing this directory.
                bool cycle_found = false;
                if (f->debug && (f->report_trace || f->report_all))
                    fprintf(stderr, "Checking for cycles in: %s\n", full_path);
                for (int i = 0; i < current_task->depth; i++) {
                    if (f->debug && (f->report_trace || f->report_all)) {

                        if (current_task->history[i].ino == st.st_ino)
                            fprintf(stderr, "%3d %ju %ju<===========\n", i,
                                    current_task->history[i].ino, st.st_ino);
                        else
                            fprintf(stderr, "%3d %ju %ju\n", i,
                                    current_task->history[i].ino, st.st_ino);
                    }
                    if (current_task->history[i].dev == st.st_dev &&
                        current_task->history[i].ino == st.st_ino) {
                        cycle_found = true;
                        break;
                    }
                }
                if (cycle_found) {
                    if (f->debug && (f->report_warnings || f->report_errors ||
                                     f->report_trace || f->report_badlinks ||
                                     f->report_all)) {
                        ssize_t len =
                            readlink(full_path, lnk_path, sizeof(lnk_path) - 1);
                        if (len != -1) {
                            lnk_path[len] = '\0';
                            fprintf(stderr, "CYCLIC_LINK,%s,%s\n", full_path,
                                    lnk_path);
                        } else
                            fprintf(stderr, "CYCLIC_LINK,%s\n", full_path);
                    }
                    termination_status = EXIT_FAILURE;
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
                //
                // What about using realloc? NO! All the directories in this
                // subdirectory must share the same history array.
                TaskNode *child_task = malloc(sizeof(TaskNode));
                child_task->dir_path = strdup(full_path);
                child_task->depth = current_task->depth + 1;
                child_task->next_task = NULL;
                child_task->history =
                    malloc((child_task->depth) * sizeof(History));
                if (current_task->depth > 0) {
                    memcpy(child_task->history, current_task->history,
                           sizeof(History) * current_task->depth);
                }
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
    }
    return NULL;
}
/** @brief scan a single file against search filters
 * @param file_spec specification of file being scanned
 * @param f SearchFilters struct
 * @param effective_type type of file being scanned
 * @return true if file selected, false otherwise
 */
int scan_file(char *file_spec, const SearchFilters *f,
              const unsigned char effective_type) {
    regmatch_t pmatch[2];
    bool stat_cached = false;

    while (1) {
        if (f->debug && (f->report_trace || f->report_all)) {
            printf("suppress %08b, effective %08b, lf_mask %08b, & %08b %s\n",
                   f->suppress_types, effective_type, lf_mask[effective_type], f->suppress_types & lf_mask[effective_type], file_spec);
        }
        if (f->suppress_types & lf_mask[effective_type])
            break;
        // Exclude non-matching files
        if (f->flags & LF_REGEX) {
            int reti =
                regexec(&f->compiled_re, file_spec, f->compiled_re.re_nsub + 1,
                        pmatch, f->reg_flags);
            if (reti == REG_NOMATCH) {
                if (f->debug && (f->report_info || f->report_all))
                    fprintf(stderr, "Regex no match: %s\n", file_spec);
                break;
            } else if (reti) {
                char errbuf[MAXLEN];
                regerror(reti, &f->compiled_re, errbuf, sizeof(errbuf));
                if (f->debug && (f->report_errors || f->report_all))
                    fprintf(stderr, "regex error: %s\n", errbuf);
                termination_status = EXIT_FAILURE;
                return 0;
            }
        }
        // Exclude matching files
        if (f->flags & LF_EXC_REGEX) {
            int reti =
                regexec(&f->compiled_ere, file_spec, f->compiled_re.re_nsub + 1,
                        pmatch, f->reg_flags);
            if (reti == 0) // Match
                break;
            if (reti != REG_NOMATCH) {
                if (reti) {
                    char errbuf[MAXLEN];
                    regerror(reti, &f->compiled_ere, errbuf, sizeof(errbuf));
                    if (f->debug && (f->report_errors || f->report_all))
                        fprintf(stderr, "Exclude regex error: %s\n", errbuf);
                    termination_status = EXIT_FAILURE;
                    return 0;
                }
            }
        }
        stat_cached = false;
        struct stat sb;
        //  Exclude files not owned by specified user
        if ((f->flags & LF_USER) && stat(file_spec, &sb) == 0) {
            stat_cached = true;
            if (sb.st_uid != f->user_id)
                break;
        }
        if (f->include_perms) {
            if (!stat_cached) {
                if (stat(file_spec, &sb) == 0)
                    stat_cached = true;
            }
            if ((f->include_perms & LF_IRUSR) && !(sb.st_mode & S_IRUSR))
                break;
            else if ((f->include_perms & LF_IWUSR) && !(sb.st_mode & S_IWUSR))
                break;
            else if ((f->include_perms & LF_IXUSR) && !(sb.st_mode & S_IXUSR))
                break;
            else if ((f->include_perms & LF_ISUID) && !(sb.st_mode & S_ISUID))
                break;
            else if ((f->include_perms & LF_ISGID) && !(sb.st_mode & S_ISGID))
                break;
        }
        if (f->before) {
            if (!stat_cached) {
                if (stat(file_spec, &sb) == 0)
                    stat_cached = true;
            }
            if (stat_cached && sb.st_mtime > f->before)
                break;
        }
        if (f->after) {
            if (!stat_cached) {
                if (stat(file_spec, &sb) == 0)
                    stat_cached = true;
            }
            if (stat_cached && sb.st_mtime < f->after)
                break;
        }
        if (f->file_size_min) {
            if (!stat_cached) {
                if (stat(file_spec, &sb) == 0)
                    stat_cached = true;
            }
            if (stat_cached && sb.st_size < f->file_size_min)
                break;
        }
        if (effective_type == DT_DIR) {
            char *file_p = file_spec;
            while (*file_p++ != '\0')
                ;
            *file_p = '/';
        }
        if (f->only_errors)
            break;
        if (file_spec[0] == '.' && file_spec[1] == '/')
            printf("%s\n", &file_spec[2]);
        else
            printf("%s\n", file_spec);
        break;
    }
    return true;
}
