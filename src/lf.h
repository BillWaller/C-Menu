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
#define LF_VERSION_MAJOR 0
#define LF_VERSION_MINOR 3
#define LF_VERSION_PATCH 3
#define LF_VERSION "v0.3.3"
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
// #define DEBUGT 1
// #define DEBUGF 1
// #define DEBUGX 1
// #define DEBUGY 1
// #define DEBUGZ 1
#if __STDC_VERSION__ < 202311L
#define nullptr NULL
#endif
#define MAX_ARGS 64 /**< maximum number of arguments for external commands */
#define MAXLEN 256  /**< maximum length for various strings and buffers */
// #define MAX_PATH_LEN _POSIX_PATH_MAX
#define MAX_THREADS 64
#define MAX_DEPTH 64
#define CACHE_LINE_SIZE 64
#define STAGE_NODES 8
#define STAGE_PATH_BYTES (STAGE_NODES * 1024)
#define MAX_PATH_LEN PATH_MAX
#define MAX_LOG_RECORD_LEN MAX_PATH_LEN
#define DIR_BUF_SIZE 65536
#define QUEUE_NODES (1024 * 1024)
#define PATH_BYTES (32 * 1024 * 1024)
#define MAX_LOG_REC_LEN 1024
#define MAX_LOG_SIZE (1024 * 1024)
#define OUTPUT_BUFFER_BYTES (16 * MAX_PATH_LEN)
#define QUEUE_MASK (QUEUE_NODES - 1)
// #define DEBUG_CHILD_STAGING
#if defined(__has_c_attribute)
#if __has_c_attribute(gnu::format)
#define ATTR_FORMAT(archetype, fmt_idx, first_arg) \
    [[gnu::format(archetype, fmt_idx, first_arg)]]
#endif
#endif
// Fallback for older compilers (C11, C99, etc.) using legacy GCC syntax
#ifndef ATTR_FORMAT
#if defined(__GNUC__) || defined(__clang__)
#define ATTR_FORMAT(archetype, fmt_idx, first_arg) \
    __attribute__((format(archetype, fmt_idx, first_arg)))
#else
#define ATTR_FORMAT(archetype, fmt_idx, \
                    first_arg) /* Unsupported compiler, strip attribute */
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
// ---------------------------------------------------------------
// Logging
// ---------------------------------------------------------------
#define ANSI_RESET "\033[0m"
#define AS_STRINGS(NAME) #NAME,
#define AS_ENUM(NAME) NAME,
#define LOG_LEVEL_LIST(X) \
    X(NONE)               \
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
typedef enum : uint16_t { LOG_LEVEL_LIST(AS_ENUM) LOG_LEVEL_COUNT } ll_e;
typedef struct {
    char key;
    const char *name;
    uint16_t bits;
    char SGR[64];
} log_level;
typedef uint16_t LogLevel;
#define TIMESTAMP_STYLE(X) \
    X(ISO8601_ZULU)        \
    X(ISO8601_LOCAL)       \
    X(CODER)
typedef enum {
    TIMESTAMP_STYLE(AS_ENUM) TIMESTAMP_STYLE_COUNT
} __attribute((packed)) tss_e;
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
    ANSI_COLOR_LIST(AS_ENUM) ANSI_COLOR_COUNT
} __attribute((packed)) sgr_t;
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
// HELPER MACROS
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
// ---------------------------------------------------------------
// DIRECTORY DATA
// ---------------------------------------------------------------
struct linux_dirent64 {
    unsigned long long d_ino; /* 64-bit inode number */
    long long d_off;          /* 64-bit offset to next structure */
    unsigned short d_reclen;  /* Size of this dirent */
    unsigned char d_type;     /* File type */
    char d_name[];            /* Filename (null-terminated) */
};
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
/*
    dirent d_type to lf_type reference
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
/*
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
#define to_uppercase(c)       \
    if (c >= 'a' && c <= 'z') \
    c -= ' '
// ---------------------------------------------------------------
// QUEUE STRUCTURES
// ---------------------------------------------------------------
#define NO_PARENT UINT32_MAX
typedef struct {
    uint_fast32_t parent_idx;           //   4
    uint_fast32_t path_offset;          //   4
    uint_fast32_t path_len;             //   4
    uint_fast32_t child_dirs_remaining; //   4
    uint_fast32_t depth;                //   4
    dev_t dev;                          //   8
    ino_t ino;                          //   8
} QueueNode;                            //  36 bytes total
// ---------------------------------------------------------------
// OUTPUT BUFFER
// ---------------------------------------------------------------
typedef struct {
    char data[OUTPUT_BUFFER_BYTES];
    size_t len;
} OutputBuffer;
// ---------------------------------------------------------------
// THREAD LOCAL
// ---------------------------------------------------------------
typedef struct {
    void *stage_arena;
    QueueNode *stage_node;
    char *path_data;
    size_t stage_arena_size;
    size_t stage_queue_size;
    size_t stage_path_size;
    OutputBuffer *output;
    uint_fast32_t parent_idx;           //   4
    uint_fast32_t path_offset;          //   4
    uint_fast32_t path_len;             //   4
    uint_fast32_t child_dirs_remaining; //   4
    uint_fast32_t depth;                //   4
    dev_t dev;                          //   8
    ino_t ino;                          //   8
    uint_fast32_t cum_path_offset;
    uint_fast32_t queue_idx;
    uint_fast32_t stage_idx;
    uint_fast32_t thread_id;
    uint_fast32_t parent_path_len;
    uint_fast32_t d_name_len;
    char *d_name;
    char path[MAX_PATH_LEN];
    struct stat *cached_sb;
    bool stat_cached;
    unsigned char effective_type;
    char *dir_buf;
    LogCtx log_ctx;
} ThreadLocal;
// ---------------------------------------------------------------
// MEMORY USAGE STATISTICS
// ---------------------------------------------------------------
typedef struct {
    uintptr_t stack_max;
    uintptr_t stack_base;
    uintptr_t current_sp;
    void *heap_base;
    long heap_used;
    long stack_used;
} MemMetrics;
// ---------------------------------------------------------------
// MISCELANEOUS
// ---------------------------------------------------------------
typedef enum {
    TS_SUCCESS = 0,
    TS_ERROR = 1,
    TS_MATCH = 2,
    TS_MATCH_PLUS_ERROR = 3,
} TerminationStatus;
// ---------------------------------------------------------------
// GLOBAL LF CONTEXT
// ---------------------------------------------------------------
typedef struct {
    void *arena;
    alignas(CACHE_LINE_SIZE) QueueNode *queue;
    alignas(CACHE_LINE_SIZE) char *path_data;
    alignas(CACHE_LINE_SIZE) atomic_uint_fast32_t path_offset;
    alignas(CACHE_LINE_SIZE) atomic_uint_fast32_t shut_down;
    alignas(CACHE_LINE_SIZE) atomic_uint_fast32_t enqueue_pos;
    alignas(CACHE_LINE_SIZE) atomic_uint_fast32_t dequeue_pos;
    alignas(CACHE_LINE_SIZE) atomic_uint_fast32_t active_tasks;
    alignas(CACHE_LINE_SIZE) atomic_uint_fast32_t queue_idx;
    alignas(CACHE_LINE_SIZE) atomic_uint_fast32_t thread_id;
    alignas(CACHE_LINE_SIZE) atomic_uint_fast32_t sequence[QUEUE_NODES];
    size_t arena_bytes;
    size_t queue_bytes;
    size_t path_bytes;
    pthread_mutex_t output_mutex;
    atomic_size_t error_count;
    atomic_size_t log_error_count;
    pthread_t *threads;
    long start;
    unsigned int nthreads;
    atomic_size_t file_count;
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
    uint32_t max_depth;
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
    bool xdev;
    bool depth_first;
    bool ignore_case;
    bool sort;
    bool sort_reverse;
    bool include_hidden;
    bool hidden_only;
    bool follow_links;
    tss_e timestamp_style;
    bool f_logging;
    ll_e disp_level_mask;
    ll_e file_level_mask;
    bool only_errors;
    bool count_errors;
    bool count;
    bool count_silently;
} GlobalContext;
// LOG LEVELS
// 1-CONFIG, 2- INFO, 3-ERROR, 4-DEBUG, 5-CYCLE, 6-QUEUE, 7-ONLY_ERRORS,
// 8-COUNT_ERRORS, 9-ALL, S-SPIN_COUNT, T-STAGE
#define print_file_type(mask, lf_type, dt_type, name)                             \
    {                                                                             \
        fprintf(stderr, "%c %08b (%3d) %08b (%2d) %s\n",                          \
                (mask & lf_type) ? '*' : ' ', lf_type, lf_type, dt_type, dt_type, \
                name);                                                            \
    }

// ---------------------------------------------------------------
// FUNCTION PROTOTYPES
// ---------------------------------------------------------------
unsigned long a_to_ul(const char *);
int a_toi(char *, bool *);
bool is_symlink_to_dir(const char *);
bool expand_tilde(char *, uint);
bool is_directory(const char *);
bool is_valid_regex(const char *);
char *get_local_timestamp();
char *get_user_str(char *, size_t);
char *get_ip_addresses(char *, uint);
bool is_newer(char *, char *);
int str_to_args(char **, char *, uint);
size_t ssnprintf(char *, size_t, const char *, ...);
size_t strnz__cpy(char *, const char *, size_t);
size_t strnz__cat(char *, const char *, size_t);
size_t strnz__pcat(char *d, size_t pos, const char *s, size_t max_len);
char *strerror__r(int errnum, char *buf, size_t buflen);
void get_proc_memory();
void iso8601_timestamp(GlobalContext *__G, LogCtx *log_ctx);
void coders_timestamp(LogCtx *log_ctx);
void initialize_timer(GlobalContext *__G, LogCtx *log_ctx);
void initialize_thread_timer(GlobalContext *__G, LogCtx *log_ctx);
void debug_out(GlobalContext *__G, int, char **);
bool init_lf(GlobalContext *__G, int, char **);
int sort_lf_output(GlobalContext *__G, int, char **);
void *worker(void *arg);
void *finder(GlobalContext *__G, ThreadLocal *tl);
int print_file(GlobalContext *__G, ThreadLocal *tl);
void flush_output_buffer(GlobalContext *, OutputBuffer *);
bool append_output_buffer(GlobalContext *__G, OutputBuffer *output, const char *path, size_t path_len, bool append_slash);
void finalize_dir_node(GlobalContext *__G, size_t node_offset);
bool global_arena_create(GlobalContext *__G);
void global_arena_destroy(GlobalContext *__G);
void push_root_node(GlobalContext *__G, dev_t dev, ino_t ino);
bool mpmc_enqueue(GlobalContext *__G, QueueNode *node, const char *path, size_t len);
bool mpmc_enqueue_staged(GlobalContext *, ThreadLocal *__tl);
bool mpmc_bulk_enqueue(GlobalContext *__G, ThreadLocal *__tl);
bool mpmc_dequeue(GlobalContext *__G, uint_fast32_t *dequeue_pos);
char *path_data_init(GlobalContext *__G, LogCtx *log_ctx);
void path_data_destroy(GlobalContext *__G);
size_t flush_ch(GlobalContext *__G, ThreadLocal *tl);
void iso8601_timestamp(GlobalContext *__G, LogCtx *log_ctx);
bool parse_local_timestamp(const char *s, time_t *out);
char *format_local_timestamp(time_t t, char *buf, size_t n);
bool expand_tilde(char *str, uint path_maxlen);
char *rep_substring(const char *org_s, const char *tgt_s, const char *rep_s);
size_t vssnprintf(char *buf, size_t buf_size, const char *format, va_list args);
size_t ssnprintf(char *buf, size_t buf_size, const char *format, ...);
bool is_symlink_to_dir(const char *path);
unsigned long a_to_ul(const char *str);
int a_toi(char *s, bool *a_toi_error);
size_t strnz__cpy(char *d, const char *s, size_t max_len);
size_t strnz__cat(char *d, const char *s, size_t max_len);
bool is_directory(const char *path);
bool is_valid_regex(const char *pattern);
void build_path(ThreadLocal *__tl, char *file_name);
// ---------------------------------------------------------------
// LOGGING
// ---------------------------------------------------------------
void log_open(GlobalContext *__G, LogCtx *log_ctx, size_t thread_id);
ATTR_FORMAT(printf, 7, 8)
void log_write(const ll_e level, GlobalContext *__G, LogCtx *log_ctx, const char *file,
               const char *func, const int line, const char *fmt, ...);
#define lf_log(level, fmt, ...)                                      \
    {                                                                \
        log_write(level, __G, log_ctx, __FILE__, __func__, __LINE__, \
                  fmt, ##__VA_ARGS__);                               \
    }

#ifdef __cplusplus
}
#endif
