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
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <time.h>
#include <wait.h>
#include <wchar.h>

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
    X(TRACE)              \
    X(ONLY_ERRORS)        \
    X(ERROR_COUNT)
#define AS_ENUM(NAME) NAME,
typedef enum {
    LOG_LEVEL_LIST(AS_ENUM)
        LOG_LEVEL_COUNT
} __attribute((packed)) LfLogLevel;
const char *log_s[] = {LOG_LEVEL_LIST(AS_STRINGS)};
const char *log_c[] = {
    [CONFIG] = "\033[34m",
    [INFO] = "\033[32m",
    [ERROR] = "\033[1;38;2;255;150;0m",
    [DEBUG] = "\033[33m",
    [TRACE] = "\033[35m",
};

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
    long start;
    long prev_nsec;
    long prev_sec;
    size_t thread_id;
    char file_spec[MAXLEN];
    bool local;
    int fd;
    int err_fd;
    LfLogLevel log_level;
    tss_e timestamp_style;
    char buf[MAX_TIMESTAMP_LEN];
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
extern void iso8601_timestamp(LogCtx *log_ctx);
void coder_timestamp(LogCtx *log_ctx);
extern void log_open(LogCtx *log_ctx, size_t thread_id);
void log_write(const LfLogLevel level, LogCtx *log_ctx, const char *file, const char *func, const int line, const char *fmt, ...) __attribute__((format(printf, 6, 7)));

#define lf_log(level, fmt, ...)                                                          \
    do {                                                                                 \
        if (level <= log_ctx->log_level) {                                               \
            log_write(level, log_ctx, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__); \
        }                                                                                \
    } while (0)

#ifdef __cplusplus
}
#endif
