/** @file lf.h
 *  @brief Headder for lf
 *  @author Bill Waller
 *  Copyright (c) 2026
 *  MIT License
 *  billxwaller@gmail.com
 *  @date 2026-02-09
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define _GNU_SOURCE

#include "version.h"
#include <argp.h>

#include <pthread.h>
#include <regex.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <wait.h>

#define MAX_PATH_LEN _POSIX_PATH_MAX
#define MAX_DEPTH 64
#define DIR_BUF_SIZE 131072
#define CACHE_LINE_SIZE 64

#define QUEUE_NODES (2 * 1024 * 1024)       // 4194304 * 16 = 64 MB
#define CYCLE_NODES (2 * 1024 * 1024)       // 4194304 & 16 = 64 MB
#define DIRECTORY_NODES (2 * 1024 * 1024)   // 4194304 * 32 = 128 MB
#define PATH_ARENA_BYTES (32 * 1024 * 1024) // 33554432 = 32 MB

#define QUEUE_MASK (QUEUE_NODES - 1)
#define DIRECTORY_MASK (DIRECTORY_NODES - 1)
#define CYCLE_MASK (CYCLE_NODES - 1)
#define PATH_ARENA_MASK (PATH_ARENA_BYTES - 1)

#define STAGE_BUFFER_SIZE 256
#define DEBUG_CHILD_STAGING
#define MAX_LOG_RECORD_LEN 4096

#if defined(__has_c_attribute)
#if __has_c_attribute(gnu::format)
#define ATTR_FORMAT(archetype, fmt_idx, first_arg) [[gnu::format(archetype, fmt_idx, first_arg)]]
#endif
#endif

// Fallback for older compilers (C11, C99, etc.) using legacy GCC syntax
#ifndef ATTR_FORMAT
#if defined(__GNUC__) || defined(__clang__)
#define ATTR_FORMAT(archetype, fmt_idx, first_arg) __attribute__((format(archetype, fmt_idx, first_arg)))
#else
#define ATTR_FORMAT(archetype, fmt_idx, first_arg) /* Unsupported compiler, strip attribute */
#endif
#endif

// Don't set __USE_GNU_GETTEXT as the translations are not yet complete
#ifdef __USE_GNU_GETTEXT
#include <libintl.h>
#define _(String) gettext(String)
#define gettext_noop(String) String
#define N_(String) gettext_noop(String)
#else
#define _(String) (String)
#define N_(String) String
#define textdomain(Domain)
#define bindtextdomain(Package, Directory)
#endif

#if __STDC_VERSION__ < 202311L
#define nullptr NULL
#endif

#define MAX_ARGS 64 /**< maximum number of arguments for external commands */
#define MAXLEN 256  /**< maximum length for strings and buffers */
#define MAXARGS 64  /**< maximum number of arguments */

// ---------------------------------------------------------------
// Logging
// ---------------------------------------------------------------
#define ANSI_RESET "\033[0m"
#define MAX_LOG_SIZE (10 * 1024 * 1024)
#define MAX_LOG_REC_LEN 4096
#define AS_STRINGS(NAME) #NAME,
#define AS_ENUM(NAME) NAME,

#define LOG_LEVEL_LIST(X) \
    X(CONFIG)             \
    X(INFO)               \
    X(ERROR)              \
    X(DEBUG)              \
    X(CYCLE)              \
    X(ENQUEUE)            \
    X(DEQUEUE)            \
    X(ALL)                \
    X(COUNT_ERRORS)       \
    X(ONLY_ERRORS)        \
    X(SPIN_COUNT)         \
    X(STAGING)
const char *log_s[] = {LOG_LEVEL_LIST(AS_STRINGS)};
typedef enum : uint16_t {
    LOG_LEVEL_LIST(AS_ENUM)
        LOG_LEVEL_COUNT
} ll_e;

typedef struct {
    char key;
    const char *name;
    uint16_t bits;
    char SGR[64];
} log_level;
log_level ll_t[] = {
    [CONFIG] = {'1', "CONFIG", 0b1, "\033[37m"},
    [INFO] = {'2', "INFO", 0b10, "\033[1;30;46m"},
    [ERROR] = {'3', "ERROR", 0b100, "\033[1;38;2;255;150;0m"},
    [DEBUG] = {'4', "DEBUG", 0b1000, "\033[31m"},
    [CYCLE] = {'5', "CYCLE", 0b10000, "\033[33m"},
    [ENQUEUE] = {'6', "ENQUEUE", 0b100000, "\033[35m"},
    [DEQUEUE] = {'7', "DEQUEUE", 0b1000000, "\033[36m"},
    [ALL] = {'A', "ALL", 0b1111111, ""},
    [COUNT_ERRORS] = {'C', "COUNT_ERRORS", 0b100000000, ""},
    [ONLY_ERRORS] = {'O', "ONLY_ERRORS", 0b1000000000, ""},
    [SPIN_COUNT] = {'S', "SPIN_COUNT", 0b10000000000, "\033[1;40;35m"},
    [STAGING] = {'T', "STAGING", 0b100000000000, "\033[1;40;35m"},
    [LOG_LEVEL_COUNT] = {'\0', "LOG_LEVEL_COUNT", 0, ""},
};

typedef uint16_t LogLevel;

#define TIMESTAMP_STYLE(X) \
    X(ISO8601_ZULU)        \
    X(ISO8601_LOCAL)       \
    X(CODER)
typedef enum {
    TIMESTAMP_STYLE(AS_ENUM)
        TIMESTAMP_STYLE_COUNT
} __attribute((packed)) tss_e;
const char *tss_s[] = {TIMESTAMP_STYLE(AS_STRINGS)};

#define ANSI_COLOR_LIST(X) \
    X(BLACK)               \
    X(RED)                 \
    X(GREEN)               \
    X(YELLOW)              \
    X(BLUE)                \
    X(CYAN)                \
    X(MAGENTA)             \
    X(WHITE)               \
    X(BBLACK)              \
    X(BRED)                \
    X(BGREEN)              \
    X(BYELLOW)             \
    X(BBLUE)               \
    X(BCYAN)               \
    X(BMAGENTA)            \
    X(BWHITE)              \
    X(ORANGE)              \
    X(BORANGE)             \
    X(RESET)
typedef enum {
    ANSI_COLOR_LIST(AS_ENUM)
        ANSI_COLOR_COUNT
} __attribute((packed)) sgr_t;
const char *sgr_s[] = {ANSI_COLOR_LIST(AS_STRINGS)};
// sgr_c sgr_codes
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
#define MAX_TIMESTAMP_LEN 64
typedef struct {
    long start_nsec;
    long start_sec;
    long p_int_nsec;
    long p_int_sec;
    size_t thread_id;
    char time_buf[MAX_TIMESTAMP_LEN];
    char record[MAX_LOG_REC_LEN];
} LogCtx;

// NOTE: It should be safe to use lf_ctx as a global variable, as it is only
// used for input handling and does not contain any state that would be modified
// by multiple threads. However, if you plan to use this in a multi-threaded
// environment, consider using thread-local storage or mutexes to protect access
// to this structure.

int stdin_fd,
    stdout_fd, stderr_fd, tty_fd, pipe_in, pipe_out;
#define max(a, b)               \
    ({                          \
        __typeof__(a) _a = (a); \
        __typeof__(a) _b = (b); \
        _a > _b ? _a : _b;      \
    })
#define min(x, y)           \
    ({                      \
        typeof(x) _x = (x); \
        typeof(x) _y = (y); \
        (void)(&_x == &_y); \
        _x < _y ? _x : _y;  \
    })
#define MAX(a, b)           \
    ({                      \
        typeof(a) _a = (a); \
        typeof(y) _b = (b); \
        _a > _b ? _a : _b;  \
    })
#define MIN(x, y)           \
    ({                      \
        typeof(x) _x = (x); \
        typeof(y) _y = (y); \
        (void)(&_x == &_y); \
        _x < _y ? _x : _y;  \
    })
#define ABS(x)                  \
    ({                          \
        __typeof__(x) _a = (x); \
        _a < 0 ? -_a : _a;      \
    })
#define S_TOLOWER(c)                                            \
    ({                                                          \
        int __c = (c);                                          \
        (__c >= 'A' && __c <= 'Z') ? (__c + ('a' - 'A')) : __c; \
    })
#define S_TOUPPER(c)                                            \
    ({                                                          \
        int __c = (c);                                          \
        (__c >= 'a' && __c <= 'z') ? (__c - ('a' - 'A')) : __c; \
    })
#define to_uppercase(c)       \
    if (c >= 'a' && c <= 'z') \
    c -= ' '
#define to_lowercase(c)       \
    if (c >= 'A' && c <= 'Z') \
    c += ' '

typedef enum {
    /** byte 0 - bits 0-7  Selection Flags*/
    LF_HIDE = 0b00000001,      /**< 1 Don't list hidden files */
    LF_ICASE = 0b00000010,     /**< 2 Ignore case in search */
    LF_EXC_REGEX = 0b00000100, /**< 4 Exclude files matching regex */
    LF_REGEX = 0b00001000,     /**< 8 Include files matching regex */
    LF_EXEC = 0b00010000,      /**< 16 Execute command each file */
    LF_USER = 0b00100000,      /**< 32 Select User Name */
                               /** << 16 */
    /** byte 1 - bits 8-15 */
    LF_IXUSR = 0b00000001, /**< 1 Select Files with Execute Permission */
    LF_IWUSR = 0b00000010, /**< 2 Select Files with Write Permission */
    LF_IRUSR = 0b00000100, /**< 4 Select Files with Read Permission */
    LF_ISGID = 0b00010000, /**< 16 Select Setgid Files */
    LF_ISUID = 0b00100000, /**< 32 Select Setuid Files */
} LFFlags;

/** byte 2 - bits 16-23 File types*/
typedef enum {
    LF_FIFO = 0b00000001,   /**<   1 named pipe */
    LF_CHR = 0b00000010,    /**<   2 character device */
    LF_DIR = 0b00000100,    /**<   4 directory */
    LF_BLK = 0b00001000,    /**<   8 block */
    LF_REG = 0b00010000,    /**<  16 regular file */
    LF_LNK = 0b00100000,    /**<  32 link */
    LF_SOCK = 0b01000000,   /**<  64 socket */
    LF_UNKNOWN = 0b10000000 /**< 128 unknown */
} LFTypes;

typedef enum {
    F_F0,      // 0
    F_FIFO,    // 1
    F_CHR,     // 2
    F_F1,      // 3
    F_DIR,     // 4
    F_F2,      // 5
    F_BLK,     // 6
    F_F3,      // 7
    F_REG,     // 8
    F_F4,      // 9
    F_LNK,     // 10
    F_F5,      // 11
    F_SOCK,    // 12
    F_F6,      // 13
    F_UNKNOWN, // 14
} F_Type;

// int const lf_type[][15] = {
//      {0, 0b00000001, 0b00000010, 0, 0b00000100, 0, 0b00001000, 0, 0b00010000, 0,
//       0b00100000, 0,0b01000000, 0, 0b10000000}};

/*
 * dirent d_type to lf_type reference
------------------------   --------------------
d_type      binary   dec   lf_type     binary   dec ordinal
---------   -------- ---   ----------- -------- --- -------
DT_FIFO:    00000001   1   LF_FIFO:    00000001   1     1
DT_CHR:     00000010   2   LF_CHR:     00000010   2     2
DT_DIR:     00000100   4   LF_DIR:     00000100   4     3
DT_BLK:     00000110   6   LF_BLK:     00001000   8     4
DT_REG:     00001000   8   LF_REG:     00010000  16     5
DT_LNK:     00001010  10   LF_LNK:     00100000  32     6
DT_SOCK:    00001100  12   LF_SOCK:    01000000  64     7
DT_UNKNOWN: 00001110  14   LF_UNKNOWN: 10000000 128     8
*/

#define F_NO_STDERR 1

/**
                      Include     Exclude
                     ----------  ----------
    LF_FIFO       1  0 00000001  7 11111110 named pipe
    LF_CHR        2  1 00000010  6 11111101 character device
    LF_DIR        4  2 00000100  5 11111011 directory
    LF_BLK        8  3 00001000  4 11110111 block device
    LF_REG       16  4 00010000  3 11101111 regular file
    LF_LNK       32  5 00100000  2 11011111 link
    LF_SOCK      64  6 01000000  1 10111111 socket
    LF_UNKNOWN  128  7 10000000  0 01111111 unknown
*/

#define DEFAULTSHELL "/bin/bash"
#define S_WCOK 0x1000  /**< write or create permitted */
#define S_QUIET 0x2000 /**< quiet mode flag for file validation */

/** @brief This macro registers the end_pgm function to be called when the
   program exits.
    @details It checks the return value of atexit() to ensure that the
    registration was successful, and if not, it prints an error message and
    exits with a failure status. Programs using libcm should call __atexit in
    their main function to ensure that the end_pgm function is registered to be
    called on program exit. This will help ensure that the terminal is properly
    restored to its original state, even if the program encounters an error or
    is terminated unexpectedly. */
#define __atexit                                             \
    {                                                        \
        int rc;                                              \
        rc = atexit(end_pgm);                                \
        if (rc != 0) {                                       \
            fprintf(stderr, "\nCannot set exit function\n"); \
            exit(EXIT_FAILURE);                              \
        }                                                    \
    }

/** simple macro to convert a character to uppercase */
#define to_uppercase(c)       \
    if (c >= 'a' && c <= 'z') \
    c -= ' '

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

char em0[MAXLEN];
char em1[MAXLEN];
char em2[MAXLEN];
char em3[MAXLEN];
char em4[MAXLEN];

extern char *format_local_timestamp(time_t, char *, size_t);
extern bool parse_local_timestamp(const char *, time_t *);
extern unsigned long a_to_ul(const char *);
extern int a_toi(char *, bool *);
extern bool is_symlink_to_dir(const char *);
extern bool expand_tilde(char *, uint);
extern bool is_directory(const char *);
extern bool is_valid_regex(const char *);
extern char *get_local_timestamp();
extern char *get_user_str(char *, size_t);
extern char *get_ip_addresses(char *, uint);
extern bool is_newer(char *, char *);
extern int str_to_args(char **, char *, uint);
extern size_t ssnprintf(char *, size_t, const char *, ...);
size_t strnz__cpy(char *, const char *, size_t);
size_t strnz__cat(char *, const char *, size_t);
size_t strnz__pcat(char *d, size_t pos, const char *s, size_t max_len);
char *strerror__r(int errnum, char *buf, size_t buflen);
// ---------------------------------------------------------------
// Logging
// ---------------------------------------------------------------
typedef struct {
    MPMCQueue *q;
    alignas(CACHE_LINE_SIZE) _Atomic size_t thread_id;
    PathArena *path_arena;
    QueueArena *queue_arena;
    DirectoryArena *directory_arena;
    CycleArena *cycle_arena;
    atomic_size_t error_count;
    atomic_size_t log_error_count;
    pthread_t *threads;
    long start;
    unsigned int nthreads;
    atomic_size_t file_count;
    pthread_mutex_t output_mutex;
    uintmax_t user_id;
    off_t file_size_min;
    int err_fd;
    int log_fd;
    char log_file_spec[MAX_PATH_LEN];
    long flags;
    long start_nsec;
    long start_sec;
    long p_int_nsec;
    long p_int_sec;
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
    bool ignore_case;
    bool sort;
    bool sort_reverse;
    bool include_hidden;
    bool hidden_only;
    bool follow_links;
    tss_e timestamp_style;
    bool display_log;
    bool file_log;
    ll_e disp_level_mask;
    ll_e file_level_mask;
    bool only_errors;
    bool count_errors;
    bool count;
    bool count_silently;
} LfCtx;

// 1-CONFIG, 2- INFO, 3-ERROR, 4-DEBUG, 5-CYCLE, 6-QUEUE, 7-ONLY_ERRORS,
// 8-COUNT_ERRORS, 9-ALL, S-SPIN_COUNT, T-STAGE
extern void iso8601_timestamp(LfCtx *lf, LogCtx *log_ctx);
void coder_timestamp(LogCtx *log_ctx);
void initialize_timer(LfCtx *lf, LogCtx *log_ctx);
void initialize_thread_timer(LfCtx *lf, LogCtx *log_ctx);
extern void log_open(LfCtx *lf, LogCtx *log_ctx, size_t thread_id);
ATTR_FORMAT(printf, 7, 8)
void log_write(const ll_e level, LfCtx *lf, LogCtx *log_ctx, const char *file, const char *func, const int line, const char *fmt, ...);

#define lf_log(level, fmt, ...)                                     \
    {                                                               \
        log_write(level, lf, log_ctx, __FILE__, __func__, __LINE__, \
                  fmt, ##__VA_ARGS__);                              \
    }

#ifdef __cplusplus
}
#endif
