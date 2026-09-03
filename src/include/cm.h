/** @file cm.h
 *  @brief Headder for C-Menu API library, libcm.so
 *  @author Bill Waller
 *  Copyright (c) 2025
 *  MIT License
 *  billxwaller@gmail.com
 *  @date 2026-02-09
 */
#ifndef _CM_H
#define _CM_H 1

#ifdef __cplusplus
extern "C" {
#endif

#define _GNU_SOURCE
#define _XOPEN_SOURCE_EXTENDED 1 /**< Enable wide character support */
#define NCURSES_WIDECHAR 1       /**< Enable wide character support */
#include "version.h"
#include <argp.h>
#ifdef UAL_UI
// #include "../ui/ui_ncurses_internal.h"
#include "ui_backend.h"
#include <ncursesw/ncurses.h>
#include <ncursesw/panel.h>
#endif
#ifdef NOTCURSES_UI
#include "ui_backend.h"
#include "ui_notcurses_internal.h"
#include <notcurses/notcurses.h>
#endif
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <time.h>
#include <wait.h>
#include <wchar.h>

#if __STDC_VERSION__ < 202311L
#define nullptr NULL
#endif

#define MAXWIN 30 /**< maximum number of windows that can be created */

#ifdef UI_NCURSES
extern SCREEN *screen;
#endif
extern FILE *tty_fp;
#define MAX_ARGS 64   /**< maximum number of arguments for external commands */
#define MAXLEN 256    /**< maximum length for strings and buffers */
#define MAXARGS 64    /**< maximum number of arguments */
#define SCR_COLS 1024 /**< maximum number of columns in the terminal screen */
#define MAX_DEPTH 3   /**< default depth for recursive file searching */
#define SCREEN_MAX_LINES 100
#define Ctrl(c) ((c) & 0x1f)
#include <stdio.h>

typedef struct {
    uint yyyy;
    uint mm;
    uint dd;
} Date;

typedef struct {
    uint hh;
    uint mm;
    uint ss;
} Time;

#define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))

/** @brief max macro evaluates two expressions, returning greatest result.
    @details These macros use compound statements to create local scopes for the
   temporary variables _x and _y, which store the values of x and y,
   respectively. This ensures that if x or y have side effects (such as being
   incremented), they will only be evaluated once when the macro is expanded.
    The use of typeof allows the macro to work with any data type.
    The line (void)(&_x == &_y) is a compile-time check to ensure that the
   types of the arguments are compatible.
    This implementation of the min and max macros provides a safer and
   more robust way to determine the minimum and maximum values between two
   expressions without risking unintended consequences from multiple
   evaluations.
 */
#define max(a, b)               \
    ({                          \
        __typeof__(a) _a = (a); \
        __typeof__(a) _b = (b); \
        _a > _b ? _a : _b;      \
    })
/** @brief min macro evaluates two expressions, returning least result */
#define min(x, y)           \
    ({                      \
        typeof(x) _x = (x); \
        typeof(x) _y = (y); \
        (void)(&_x == &_y); \
        _x < _y ? _x : _y;  \
    })
/**
 */
/** @brief MIN macro for compatibility with code that uses the same name,
   while avoiding multiple evaluations of the arguments.
   @details /usr/include/sys/param.h contains implementations of the MIN and MAX
   macros, which are simple but can lead to issues with multiple evaluations of
   the arguments if they have side effects. These macros provides a safer
   alternative to param.h Here are the macros from /usr/include/sys/param.h. You
   may comment out the macros defined herein and use the macros in param.h if
   you prefer.
    @code
    #define MIN(a, b) (((a) < (b)) ? (a) : (b))
    #define MAX(a, b) (((a) > (b)) ? (a) : (b))
    @endcode
    @note The following macro provides a safer alternative to param.h
 */
#define MAX(a, b)           \
    ({                      \
        typeof(a) _a = (a); \
        typeof(y) _b = (b); \
        _a > _b ? _a : _b;  \
    })
/** @brief MAX macro for compatibility with code that uses the same name,
   while avoiding multiple evaluations of the arguments.
    @note The following macro provides a safer alternative to param.h
 */
#define MIN(x, y)           \
    ({                      \
        typeof(x) _x = (x); \
        typeof(y) _y = (y); \
        (void)(&_x == &_y); \
        _x < _y ? _x : _y;  \
    })
/** @brief ABS macro for absolute value, which evaluates the expression once and
   returns the absolute value.
    @details This macro uses a compound statement to create a local scope for
   the temporary variable _a, which stores the value of x. This ensures that if
   x has side effects (such as being incremented), it will only be evaluated
   once when the macro is expanded. The use of typeof allows the macro to work
   with any data type that supports comparison with zero and negation.
 */
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

/** to_uppercase(c) - convert a lowercase letter to uppercase */
#define to_uppercase(c)       \
    if (c >= 'a' && c <= 'z') \
    c -= ' '
/** to_lowercase(c) - convert an uppercase letter to lowercase */
#define to_lowercase(c)       \
    if (c >= 'A' && c <= 'Z') \
    c += ' '
/**
    @brief Used for xterm256 color conversions
 */

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
 * dirent d_type to lf_type for reference
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

extern UiChyron *wait_mk_chyron();
extern bool wait_destroy(UiChyron *);
extern bool waitpid_with_timeout(pid_t, uint);
extern int wait_timeout;
extern int fork_detach_execvp(char **);
extern bool is_hex_str(char *, uint);
extern bool unstr_hex_clr(char *, char *);

extern bool f_debug; /**< a flag to indicate whether debug
output should be printed, for debugging purposes */

/**< see termios.h */
extern struct termios shell_tioctl, program_tioctl;
extern struct termios shell_in_tioctl, program_in_tioctl;
extern struct termios shell_out_tioctl, program_out_tioctl;
extern struct termios shell_err_tioctl, program_err_tioctl;

extern bool f_have_shell_tioctl;   /**< shell tioctl captured */
extern bool f_have_program_tioctl; /**< program tioctl captured */
extern bool f_program_open;        /**< program mode is active */
extern bool f_restore_screen;      /**< whether to restore the screen */

extern void dump_opts(); /**< dump options to stdout */

extern void dump_opts_by_use(char *, char *); /**< dump options to stdout */
extern bool capture_shell_tioctl();
extern bool restore_shell_tioctl();
extern bool capture_program_tioctl();
extern bool restore_program_tioctl();
extern bool mk_raw_tioctl(struct termios *);
extern bool set_sane_tioctl(struct termios *);
extern void win_resize(uint, uint, char *);
extern void signal_handler(int);
extern bool handle_signal(sig_atomic_t);
extern volatile sig_atomic_t sig_received;
extern void sig_prog_mode();
extern void sig_dfl_mode();
extern bool mk_dir(char *dir);
extern int segmentation_fault();
extern bool parse_local_timestamp(const char *, time_t *);
extern char *format_local_timestamp(time_t, char *, size_t);
extern char *get_local_timestamp();
extern char *get_user_str(char *, size_t);
extern char *get_ip_addresses(char *, uint);
extern bool is_newer(char *, char *);

#define KEY_ALTF0 0x138
#define KEY_ALTF(n) (KEY_ALTF0 + (n)) /**< define alt function keys */

extern uint n_lines; /**< number of lines in the terminal */
extern uint n_cols;  /**< number of columns in the terminal */
// extern uint lines;   current number of lines (may be less than n_lines if
// the terminal is resized)
// extern uint cols;    current number of columns (may be less than n_cols
// if the
//                       terminal is resized) extern int begx;
//                       beginning x coordinate of the terminal */
// extern uint begy;      beginning y coordinate of the terminal
//
#define MAXWIN 30 /**< maximum number of windows that can be created */
typedef unsigned char uchar;

extern void sig_prog_mode();
extern void sig_shell_mode();
extern char di_getch();
extern uint enter_option();

extern uint16_t win_attr; /**< Ncurses attributes for the current window, such as
                        color pair, bold, etc. */
// extern bool win_pair; /**< Flag to indicate whether the current window is
// part of a window pair */
extern uint
    mlines; /**< number of lines in the current window, which may be less than
               the total number of lines in the terminal if the window is
               resized or if multiple windows are being used. */
extern uint
    mcols; /**< number of columns in the current window, which may be less than
              the total number of columns in the terminal if the window is
              resized or if multiple windows are being used. */
extern uint
    mbegy; /**< beginning y coordinate of the current window, which can be used
              to determine the position of the window on the terminal screen. */
extern uint
    mbegx;             /**< beginning x coordinate of the current window, which can be used
                          to determine the position of the window on the terminal screen. */
extern uint mg_action; /**< action in progress, which can be used to keep track
                         of the current state of the program and determine how
                         to respond to user input or other events. */
extern uint mg_col;    /**< window column, which can be used to determine the
                         current column position in the window for displaying text
                         or other content. */
extern uint mg_line;   /**< window line, which can be used to determine the current line
                  position in the window for displaying text or other content. */
extern int stdin_fd;   /**< the file descriptor for the terminal, for error messages */
extern int stdout_fd;  /**< the file descriptor for the terminal, for error messages */
extern int stderr_fd;  /**< the file descriptor for the terminal, for error messages
                        and other output */
extern uint
    dbgfd;               /**< the file descriptor for debug output, for debugging purposes */
extern uint src_line;    /**< the line number of the source file being processed,
                           for error messages */
extern char *src_name;   /**< the name of the source file being processed, for
                            error messages */
extern char fn[MAXLEN];  /**< function name for error messages */
extern char em0[MAXLEN]; /**< error message string for error messages */
extern char em1[MAXLEN]; /**< error message string for error messages */
extern char em2[MAXLEN]; /**< error message string for error messages */
extern char em3[MAXLEN]; /**< error message string for error messages */

extern int exit_code; /**< the exit code for the program, for error messages and
                         other output */

extern void win_del();
extern void restore_wins();
extern void win_init_attrs();
extern void win_Toggle_Attrs();
extern int display_curses_keys();
extern void init_stdscr();
extern bool get_argp_doc_by_name(char *comment, const struct argp_option *, const char *);

typedef struct {
    char *s;
    size_t l;
} Arg;
typedef struct {
    Arg **v;
    size_t n;
} Argv;
typedef struct {
    char *s;
    size_t l;
} String;
typedef struct {
    wchar_t *s;
    size_t l;
} WCStr;
/** simple macro to convert a character to uppercase */
#define to_uppercase(c)       \
    if (c >= 'a' && c <= 'z') \
    c -= ' '

typedef struct {
    uint flin;
    uint fcol;
    uint flen;
    uint ff;
    char fill_char;
    bool cf_erase_remainder;
} CmField;
extern int a_toi(char *, bool *);
extern bool chrep(char *, char, char);
extern char *rep_substring(const char *, const char *, const char *);
extern size_t strip_ansi(char *, char *);
extern bool strip_quotes(char *);
extern bool stripz_quotes(char *);
extern int str_to_args(char **, char *, uint);
extern int destroy_argv(uint argc, char **argv);
extern bool str_to_bool(const char *);
extern bool str_to_lower(char *);
extern bool str_to_upper(char *);
extern bool strnfill(char *, char, uint);
extern bool str_subc(char *, char *, char, char *, uint);
extern size_t strnz(char *, size_t);
extern size_t strnlf(char *, size_t);
extern char *strnz_dup(char *, size_t);
extern size_t ssnprintf(char *, size_t, const char *, ...);
extern size_t strnz__cpy(char *, const char *, size_t);
extern size_t strnz__cat(char *, const char *, size_t);
extern double str_to_double(char *);
extern size_t string_cpy(String *, const String *);
extern size_t string_cat(String *, const String *);
extern size_t string_ncat(String *, const String *, size_t);
extern size_t string_ncpy(String *, const String *, size_t);
extern size_t trim(char *);
extern size_t rtrim(char *);
extern String to_string(const char *);
extern String mk_string(size_t);
extern String free_string(String);
extern int fork_exec(char **);
extern int full_screen_fork_exec(char **);
extern int full_screen_shell(char *);
extern int shell(char *);
extern char errmsg[];
extern void get_rfc3339_s(char *, size_t);
extern void display_argv_error_msg(char *, char **);
extern void user_end();
extern unsigned long a_to_ul(const char *);
extern size_t canonicalize_file_spec(char *);
extern bool construct_file_spec(char *, char *, char *, char *, char *, uint);
extern bool file_spec_path(char *, char *);
extern bool file_spec_name(char *, char *);
extern bool is_directory(const char *);
extern bool is_symlink_to_dir(const char *);
extern bool is_valid_regex(const char *);
extern bool dir_name(char *, char *);
extern bool base_name(char *, char *);
extern bool expand_tilde(char *, uint);
extern bool locate_file_in_path(char *, char *);
extern bool normalize_file_spec(char *);
extern bool trim_ext(char *, char *);
extern bool trim_path(char *);
extern bool verify_file(char *, uint);
extern bool verify_file_q(char *, uint);
extern bool verify_dir(char *, uint);
extern bool verify_dir_q(char *, uint);
extern bool verify_spec_arg(char *, char *, char *, char *, uint);
extern char *fdname(int, char *);
extern char *stdio_names(char *, char *);
extern char *stdio_fdnames(char *, char *);
extern char stdio_names_str[4096];
extern void left_justify(char *s);
extern void right_justify(char *, uint);

extern bool is_valid_date(uint yyyy, uint mm, uint dd);
extern bool is_valid_time(uint hh, uint mm, uint ss);
extern void numeric(char *d, char *s);
extern int cf_accept(UiSurface *, uint w, char *, uint, uint, uint);
extern char *fill_field(char *, char *, char, uint);

#ifdef __cplusplus
}
#endif
#endif
