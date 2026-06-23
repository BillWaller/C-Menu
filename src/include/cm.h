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

// #define DEBUG_IMMEDOK true
// #define _XOPEN_SOURCE_EXTENDED 1 /**< Enable wide character support */
#define NCURSES_WIDECHAR 1 /**< Enable wide character support */
#define _GNU_SOURCE
#include "ui_backend.h"
#include "version.h"
#include <argp.h>
#include <ncursesw/ncurses.h>
#include <ncursesw/panel.h>
#include <signal.h>
#include <stddef.h>
#include <stdlib.h>

extern int cmenu_log_fd;

#if __STDC_VERSION__ < 202311L
#define nullptr NULL
#endif

#define MAXWIN 30 /**< maximum number of windows that can be created */
extern UiSurface *ui_box[MAXWIN];
extern UiSurface *ui_win[MAXWIN];
extern UiSurface *ui_win2[MAXWIN];
extern SCREEN *screen;
extern FILE *tty_fp;

#define MAX_ARGS 64   /**< maximum number of arguments for external commands */
#define MAXLEN 256    /**< maximum length for strings and buffers */
#define MAXARGS 64    /**< maximum number of arguments */
#define SCR_COLS 1024 /**< maximum number of columns in the terminal screen */
#define MAX_DEPTH 3   /**< default depth for recursive file searching */
#define SCREEN_MAX_LINES 100
#define Ctrl(c) ((c) & 0x1f)
#include <stdio.h>

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
#define max(a, b)           \
    ({                      \
        typeof(a) _a = (a); \
        typeof(b) _b = (b); \
        _a > _b ? _a : _b;  \
    })
/** @brief min macro evaluates two expressions, returning least result */
#define min(x, y)           \
    ({                      \
        typeof(x) _x = (x); \
        typeof(y) _y = (y); \
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
        typeof(b) _b = (b); \
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

/**
    @brief Used for xterm256 color conversions
 */
typedef enum {
    CLR_BLACK,
    CLR_RED,
    CLR_GREEN,
    CLR_YELLOW,
    CLR_BLUE,
    CLR_MAGENTA,
    CLR_CYAN,
    CLR_WHITE,
    CLR_BBLACK,
    CLR_BRED,
    CLR_BGREEN,
    CLR_BYELLOW,
    CLR_BBLUE,
    CLR_BMAGENTA,
    CLR_BCYAN,
    CLR_BWHITE,
    CLR_BORANGE,
    CLR_FG,
    CLR_BG,
    CLR_BOX_FG,
    CLR_BOX_BG,
    CLR_BRACKETS_FG,
    CLR_BRACKETS_BG,
    CLR_FILL_CHAR_FG,
    CLR_FILL_CHAR_BG,
    CLR_LN_FG,
    CLR_LN_BG,
    CLR_CMDLN_FG,
    CLR_CMDLN_BG,
    CLR_NT_FG,
    CLR_NT_BG,
    CLR_NT_REV_FG,
    CLR_NT_REV_BG,
    CLR_NT_HL_FG,
    CLR_NT_HL_BG,
    CLR_NT_HL_REV_FG,
    CLR_NT_HL_REV_BG,
    CLR_TITLE_FG,
    CLR_TITLE_BG,
    CLR_NCOLORS
} ColorsEnum;

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

#define COLOR_LEN 8 /**< length of color code strings */
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

/** @struct Chyron
    @details The Chyron structure represents a key binding for a command in the
   chyron, which is a status line or message area in the terminal interface.
    The ChyronKey structure includes fields for displayable text,
    the key code, and the end position of the command text in the chyron.
    This structure allows xwgetch to map mouse clicks to key codes
    as defined by NCursesw and the C-Menu program.
    The text field holds the displayable text associated with the command.
    xwgetch() translates a mouse click on the chyron with a particular key code
    stored in this table.
    The use case is when the developer wants a dynamic chyron that can be
   easily changed depending on the context of the program without having to
   worry about translating mouse click positions to key codes.
    While many key codes are defined by NCursesw, the developer is free to
   define their own key codes for custom commands in the chyron. However,
   xwgetch() will translate mouse clicks to the key codes defined in this table,
   even if they interfere with standard NCurses key codes, Unicode code points,
   or ASCII characters. Just use common sense.
    In addition to returning the key code associated with mouse clicks on
   the chyron, xwgetch() also returns the key codes for standard keyboard input,
   and sets the global variables cliek_y and click_x to the coordinates of the
   mouse click.
 */
#define CHYRON_KEY_MAXLEN                                                \
    64                 /**< maximum length of the command text for a key \
                          binding in the chyron */
#define CHYRON_KEYS 20 /**< maximum number of key bindings for the chyron */

typedef struct {
    char text[CHYRON_KEY_MAXLEN]; /**< command text associated with the key code
                                   */
    int keycode;                  /**< key code associated with the command */
    int end_pos;                  /**< end position of the command text in the chyron */
    int cp;                       /**< color pair index for the command text in the chyron */
} ChyronKey;

typedef struct {
    ChyronKey *key[CHYRON_KEYS]; /**< array of key bindings for the chyron */
    char s[MAXLEN];              /**< the chyron string, for displaying messages in */
    cchar_t cmplx_buf[MAXLEN];   /**< the chyron wide character string */
    int l;                       /**< length of the chyron string, for display purposes */
} Chyron;

extern int vgetch(WINDOW *, int);
extern int xwgetch(WINDOW *, Chyron *, int);
extern int dxwgetch(WINDOW *, WINDOW *, Chyron *, int);

extern int click_y;       /**< the y coordinate of a mouse click */
extern int click_x;       /**< the x coordinate of a mouse click */
extern WINDOW *mouse_win; /**< input from window n */

extern Chyron *wait_mk_chyron();
extern WINDOW *wait_mk_win(Chyron *, char *);
extern int wait_continue(WINDOW *, Chyron *, int);
extern bool wait_destroy(Chyron *);
extern bool waitpid_with_timeout(pid_t, int);
extern int wait_timeout;
extern bool action_disposition(char *title, char *action_str);
extern int fork_detach_execvp(char **);
extern WINDOW *message_win(char *);
extern bool is_hex_str(char *, int);
extern bool unstr_hex_clr(char *, char *);

extern bool f_debug; /**< a flag to indicate whether debug
output should be printed, for debugging purposes */
// extern unsigned int cmd_key; /**< the command key for the current command,
// for
//                              error messages and other output */
/** @struct RGB */
typedef struct {
    int r, g, b;
} RGB;

#define FG_COLOR 2    /**< default foreground color */
#define BG_COLOR 0    /**< default background color */
#define BO_COLOR 1    /**< default bold foreground color */
#define LN_COLOR 4    /**< default line number color */
#define LN_BG_COLOR 7 /**< default line number background */

extern const wchar_t bw_ho;
extern const wchar_t bw_ho;
extern const wchar_t bw_ve;
extern const wchar_t bw_tl;
extern const wchar_t bw_tr;
extern const wchar_t bw_bl;
extern const wchar_t bw_br;
extern const wchar_t bw_lt;
extern const wchar_t bw_rt;
extern const wchar_t bw_sp;
extern const wchar_t bw_ra;
extern const wchar_t bw_la;
extern const wchar_t bw_ua;
extern const wchar_t bw_da;
extern const wchar_t bw_ran;
extern const wchar_t bw_lan;

extern int cp_default;               /**< default color pair index */
extern int cp_box;                   /**< box color pair index */
extern int cp_bold;                  /**< bold color pair index */
extern int cp_title;                 /**< title color pair index */
extern int cp_highlight;             /**< highlight color pair index */
extern int cp_fill_char;             /**< fill character color pair index */
extern int cp_brackets;              /**< color pair index for field brackets */
extern int cp_nt;                    /**< normal color pair index */
extern int cp_nt_rev;                /**< reverse color pair index */
extern int cp_nt_hl;                 /**< highlight color pair index */
extern int cp_nt_hl_rev;             /**< highlight reverse color pair index */
extern int cp_ln_fg;                 /**< line number color pair index */
extern int cp_ln_bg;                 /** line number background color pair index */
extern int cp_cmdln_fg;              /**< command line number color pair index */
extern int cp_cmdln_bg;              /** command line number background color pair index */
extern int clr_idx;                  /**< current color index */
extern int clr_cnt;                  /**< number of colors used */
extern int clr_pair_idx;             /**< current color pair index */
extern int clr_pair_cnt;             /**< number of color pairs supported by the terminal */
extern char const colors_text[][10]; /**< color codes for the 16 basic colors */

/** @struct ColorPair
  @details The ColorPair structure is a simple structure that holds information
  about a color pair, including the foreground color index, background color
  index, and the color pair index. This structure can be used to manage and
  apply color pairs in the ncurses library, allowing for easy customization of
  the terminal's appearance. By using this structure, you can easily keep track
  of the different color pairs you have defined and apply them to various
  elements in your terminal interface. */
typedef struct {
    int fg;      /**< foreground color index */
    int bg;      /**< background color index */
    int pair_id; /**< color pair index */
} ColorPair;

/**< see termios.h */
extern struct termios shell_tioctl, curses_tioctl;
extern struct termios shell_in_tioctl, curses_in_tioctl;
extern struct termios shell_out_tioctl, curses_out_tioctl;
extern struct termios shell_err_tioctl, curses_err_tioctl;

extern bool f_have_shell_tioctl;  /**< shell tioctl captured */
extern bool f_have_curses_tioctl; /**< curses tioctl captured */
extern bool f_curses_open;        /**< curses mode is active */
extern bool f_restore_screen;     /**< whether to restore the screen */

extern void dump_opts(); /**< dump options to stdout */

extern void dump_opts_by_use(char *, char *); /**< dump options to stdout */
extern bool capture_shell_tioctl();
extern bool restore_shell_tioctl();
extern bool capture_curses_tioctl();
extern bool restore_curses_tioctl();
extern bool mk_raw_tioctl(struct termios *);
extern bool set_sane_tioctl(struct termios *);
extern int box_new(int, int, int, int, char *);
extern int box2_new(int, int, int, int, char *);
extern int win_new(int, int);
extern int win2_new(int, int, int, int);
extern int box_title(WINDOW *, char *);
extern void win_redraw(WINDOW *);
extern void win_resize(int, int, char *);
extern void signal_handler(int);
extern bool handle_signal(sig_atomic_t);
extern volatile sig_atomic_t sig_received;
extern void sig_prog_mode();
extern void sig_dfl_mode();
extern bool mk_dir(char *dir);
extern int segmentation_fault();
extern cchar_t mkcc(int, attr_t, char *);
extern char *iso8601_time(char *, int, time_t *, bool);
extern bool parse_local_timestamp(const char *, time_t *);
extern char *format_local_timestamp(time_t, char *, size_t);
extern char *get_local_timestamp();
extern char *get_user_str(char *, size_t);
extern char *get_ip_addresses(char *, int);
extern bool is_newer(char *, char *);
extern cchar_t CC_REVERSE;   /**< curses default reverse */
extern cchar_t CC_NT;        /**< C-Menu normal text */
extern cchar_t CC_NT_REV;    /**< reverse */
extern cchar_t CC_NT_HL;     /**< highlight */
extern cchar_t CC_NT_HL_REV; /**< highlight reverse */
extern cchar_t CC_FILL_CHAR; /**< fill character */
extern cchar_t CC_BRKTL;     /**< left bracket */
extern cchar_t CC_BRKTR;     /**< right bracket */
extern cchar_t CC_BOX;       /**< box colors */
extern cchar_t CC_CMDLN;
extern cchar_t CC_LN;
extern cchar_t CC_DATA1;
extern cchar_t CC_DATA2;
extern cchar_t CC_DATA3;
extern cchar_t CC_TITLE; /**< box colors */

extern cchar_t CC_BRKTL; /* left field bracket */
extern cchar_t CC_BRKTR; /* right field bracket */

extern cchar_t ls, rs, ts, bs, tl, tr, bl, br;
#define KEY_ALTF0 0x138
#define KEY_ALTF(n) (KEY_ALTF0 + (n)) /**< define alt function keys */
#define XTERM_256COLOR                /**< use xterm-256color terminfo for altkey bindings */
/**
    EXTENDED NCURSES KEYS

    Key code bindings are customarily defined in terminfo.
    xterm-256color tends to be the most complete, and indeed, it seems to
    work with Ghostty, Kitty, and Alacritty. If you want to use the
    altkey bindings, you may need to set your terminal's TERM environment
    variable to xterm-256color. Here's why:

    Ghostty, Kitty, and Alacritty all come with their own terminfo files:

    Ghostty     with xterm-ghostty,
    Kitty       with xterm-kitty, and
    Alacritty   with alacritty

    None of the three produced the correct extended altkey bindings when
    paired with the terminfo provided in their distribution, yet all
    three worked with xterm-256color.

    The extended altkeys were not defined in any of the terminfo files, not
    even xterm-256color. Yet, xterm-256color produced consistent keycode
    bindings with all three emulators. The standard key bindings, ins, delete,
    up, down arrows, etc., were the same for all three terminfos, and all
    three emulators produced the correct key bindings for those keys.

    The fact that xterm-256color produced consistent keycode bindings for the
    altkeys, while the other three terminfos did not, is likely due to the fact
    that xterm-256color is more widely used and has better support for extended
    key bindings, even if they are not explicitly defined in the terminfo file.
    It's possible that xterm-256color has some default behavior or fallback
    mechanism that allows it to recognize and produce key codes for the altkeys,
    while the other terminfos do not have such a mechanism in place. This could
    explain why xterm-256color produced consistent keycode bindings for the
    altkeys across all three emulators, while the other terminfos did not.
    @code
    mapped                  altkey   ncurses
    2nd fld - 1    name     binding  keycode
    ------------- --------- -------- -------
    kIC=\E[2;2~,  key_ins   \E[2;3~,   223
    kHOM=\E[1;2H, key_home  \E[1;3H,   21e
    kPRV=\E[5;2~, key_pgup  \E[5;3~,   232
    kDC=\E[3;2~,  key_del   \E[3;3~,   20e
    kEND=\E[1;2F, key_end   \E[1;3F,   219
    kNXT=\E[6;2~, key_pgdn  \E[6;3~,   22d
    kri=\E[1;2A,  key_up    \E[1;3A,   23d
    kind=\E[1;2B, key_down  \E[1;3B,   214
    kRIT=\E[1;2C, key_right \E[1;3C,   237
    kLFT=\E[1;2D, key_left  \E[1;3D,   228
    @endcode
 */
#if defined(XTERM_256COLOR)
#define KEY_ALTINS 0x223
#define KEY_ALTHOME 0x21e
#define KEY_ALTPGUP 0x232
#define KEY_ALTDEL 0x20e
#define KEY_ALTEND 0x219
#define KEY_ALTPGDN 0x22d
#define KEY_ALTUP 0x23d
#define KEY_ALTLEFT 0x228
#define KEY_ALTDOWN 0x214
#define KEY_ALTRIGHT 0x237
#elif defined(XTERM_GHOSTTY)
#define KEY_ALTINS 0x228
#define KEY_ALTHOME 0x223
#define KEY_ALTPGUP 0x237
#define KEY_ALTDEL 0x213
#define KEY_ALTEND 0x21e
#define KEY_ALTPGDN 0x232
#define KEY_ALTUP 0x242
#define KEY_ALTLEFT 0x22d
#define KEY_ALTDOWN 0x219
#define KEY_ALTRIGHT 0x23c
#endif

/** UNICODE BOX DRAWING SYMBOLS */
#define BW_HO L'\x2500'  /**< horizontal line */
#define BW_VE L'\x2502'  /**< vertical line */
#define BW_TL L'\x250C'  /**< top left */
#define BW_TR L'\x2510'  /**< top right */
#define BW_BL L'\x2514'  /**< bottom left */
#define BW_BR L'\x2518'  /**< bottom right */
#define BW_RTL L'\x256d' /**< rounded top left */
#define BW_RTR L'\x256e' /**< rounded top right */
#define BW_RBL L'\x2570' /**< rounded bottom left */
#define BW_RBR L'\x256f' /**< rounded bottom right */
#define BW_LT L'\x251C'  /**< left tee */
#define BW_TT L'\x252C'  /**< top tee */
#define BW_RT L'\x2524'  /**< right tee */
#define BW_CR L'\x253C'  /**< cross */
#define BW_BT L'\x2534'  /**< bottom tee */
#define BW_SP L'\x20'    /**< space */
#define BW_RA L'\x2192'  /**< large right arrow */
#define BW_LA L'\x2190'  /**< large left arrow */
#define BW_UA L'\x2191'  /**< large up arrow */
#define BW_DA L'\x2193'  /**< large down arrow */
#define BW_RAN L'\x276F' /**< right_angle */
#define BW_LAN L'\x276E' /**< left_angle */

/** The following are the actual wchar_t variables that will hold the box
    drawing characters. These correspond to the above Unicode code points. By
    defining them as wchar_t, we can use them in the ncurses library to draw
    boxes and lines in the terminal. The variables are named with a "bw_" prefix
    to indicate that they are box (wide) drawing characters, and they will be
    initialized with the corresponding Unicode characters defined above. */

extern const wchar_t bw_ho;  /**< horizontal line */
extern const wchar_t bw_ve;  /**< vertical line */
extern const wchar_t bw_tl;  /**< top left corner */
extern const wchar_t bw_tr;  /**< top right corner */
extern const wchar_t bw_bl;  /**< bottom left corner */
extern const wchar_t bw_br;  /**< bottom right corner */
extern const wchar_t bw_lt;  /**< left tee */
extern const wchar_t bw_tt;  /**< top tee */
extern const wchar_t bw_rt;  /**< right tee */
extern const wchar_t bw_cr;  /**< cross */
extern const wchar_t bw_bt;  /**< bottom tee */
extern const wchar_t bw_sp;  /**< space */
extern const wchar_t bw_ra;  /**< right arrow */
extern const wchar_t bw_la;  /**< left arrow */
extern const wchar_t bw_ua;  /**< up arrow */
extern const wchar_t bw_da;  /**< down arrow */
extern const wchar_t bw_ran; /**< right piointing angle */

extern cchar_t ls, rs, ts, bs, tl, tr, bl, br, lt, rt, sp, ra, la, ua, da, ran;

extern void write_cmenu_log_nt(char *);
extern void write_cmenu_log(char *);
extern void open_cmenu_log();
extern FILE *cmenu_log_fp;
extern int n_lines; /**< number of lines in the terminal */
extern int n_cols;  /**< number of columns in the terminal */
// extern int lines;   current number of lines (may be less than n_lines if
// the terminal is resized)
// extern int cols;    current number of columns (may be less than n_cols
// if the
//                       terminal is resized) extern int begx;
//                       beginning x coordinate of the terminal */
// extern int begy;      beginning y coordinate of the terminal
//
#define MAXWIN 30 /**< maximum number of windows that can be created */
typedef unsigned char uchar;

extern void sig_prog_mode();
extern void sig_shell_mode();
extern char di_getch();
extern int enter_option();

// extern PANEL *std_panel;

// extern WINDOW *win_main;
// extern PANEL *panel_main;

extern int win_flags[MAXWIN];
extern WINDOW *win_win[MAXWIN];  /**< array of pointers to windows */
extern WINDOW *win_win2[MAXWIN]; /**< array of pointers to windows */
extern WINDOW *win_box[MAXWIN];  /**< array of pointers to box windows */
extern PANEL *panel_win[MAXWIN];
extern PANEL *panel_win2[MAXWIN];
extern PANEL *panel_box[MAXWIN];

extern int win_attr; /**< Ncurses attributes for the current window, such as
                        color pair, bold, etc. */
extern int
    win_attr_odd; /**< Ncurses attributes for the current window odd lines,
                     which may be different from the attributes for even lines
                     to create a striped effect in the window. */
extern int
    win_attr_even;  /**< Ncurses attributes for the current window even lines,
                       which may be different from the attributes for odd lines
                       to create a striped effect in the window. */
extern int win_ptr; /**< Pointer to the current window pair, box and window,
                       which can be used to keep track of the currently active
                       window and its associated box. */
// extern bool win_pair; /**< Flag to indicate whether the current window is
// part of a window pair */
extern int
    mlines; /**< number of lines in the current window, which may be less than
               the total number of lines in the terminal if the window is
               resized or if multiple windows are being used. */
extern int
    mcols; /**< number of columns in the current window, which may be less than
              the total number of columns in the terminal if the window is
              resized or if multiple windows are being used. */
extern int
    mbegy; /**< beginning y coordinate of the current window, which can be used
              to determine the position of the window on the terminal screen. */
extern int
    mbegx;            /**< beginning x coordinate of the current window, which can be used
                         to determine the position of the window on the terminal screen. */
extern int mg_action; /**< action in progress, which can be used to keep track
                         of the current state of the program and determine how
                         to respond to user input or other events. */
extern int mg_col;    /**< window column, which can be used to determine the
                         current column position in the window for displaying text
                         or other content. */
extern int
    mg_line; /**< window line, which can be used to determine the current line
                position in the window for displaying text or other content. */
             /** to_uppercase(c) - convert a lowercase letter to uppercase */
#define to_uppercase(c)       \
    if (c >= 'a' && c <= 'z') \
    c -= ' '
/** to_lowercase(c) - convert an uppercase letter to lowercase */
#define to_lowercase(c)       \
    if (c >= 'A' && c <= 'Z') \
    c += ' '
extern int tty_fd; /**< the file descriptor for the terminal, for error messages
                      and other output */
extern int
    dbgfd;               /**< the file descriptor for debug output, for debugging purposes */
extern int src_line;     /**< the line number of the source file being processed,
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

extern int win_del();
extern void destroy_win(WINDOW *);
extern void destroy_box(WINDOW *);
extern void restore_wins();
extern void win_init_attrs();
extern void win_Toggle_Attrs();
extern void mvwaddstr_fill(WINDOW *, int, int, char *, int);
extern int display_curses_keys();
extern void init_stdscr();
extern void curskeys(WINDOW *);
extern void mouse_getch(int *, int *, int *, int *);
extern void w_mouse_getch(WINDOW *, int *, int *, int *, int *);
extern bool get_argp_doc_by_name(char *comment, const struct argp_option *, const char *);
extern void cbox_hsplit(WINDOW *, int);
extern void cbox_hsplit_text(WINDOW *, char *, int);

/** @struct Arg
   @brief The Arg structure represents a string argument with a pointer to the
   string and its allocated length. */
typedef struct {
    char *s;  /**< pointer to the string argument */
    size_t l; /**< allocated length */
} Arg;
/** @struct Argv
   @brief The Argv structure represents an argument vector, which is an array of
   Arg structures, along with the number of allocated elements in the array. */
typedef struct {
    Arg **v;  /**< pointer to an array of Arg pointers, representing the argument
                 vector */
    size_t n; /**< allocated array elements */
} Argv;
/** @struct String
    @brief The String structure represents a string object with a pointer to the
   string and its allocated length. */
typedef struct {
    char *s;  /**< pointer to the string */
    size_t l; /**< allocated length */
} String;
/** @struct WCStr
   @brief wide character string object with a pointer to the wide character
   string and its allocated length
   @details The WCStr structure represents a wide character string with a
   pointer to the wide character string and its allocated length. */
typedef struct {
    wchar_t *s; /**< pointer to the wide character string */
    size_t l;   /**< @brief allocated length */
} WCStr;
/** @struct CCStr
   @brief complex character objectl with a pointer to the complex character
   string and its allocated length
   @details The CCStr structure represents a complex character string, which is
   a string that can contain both regular characters and attributes (such as
   color, bold, etc.) in the ncurses library. This structure includes a pointer
   to the complex character string and its allocated length, allowing for
   dynamic handling of complex character strings in the terminal interface. */
typedef struct {
    cchar_t *s; /**< pointer to the complex character string */
    size_t l;   /**< allocated length */
} CCStr;
/** simple macro to convert a character to uppercase */
#define to_uppercase(c)       \
    if (c >= 'a' && c <= 'z') \
    c -= ' '
/** @struct SIO
    @brief The SIO structure encapsulates various aspects of the terminal's
   state and configuration, including color management, file pointers, and
   terminal device information.
   @details The SIO structure serves as a central repository for all relevant
   information needed to manage the terminal's appearance and behavior
   effectively, allowing for a highly customizable and visually appealing
   terminal experience. The SIO structure includes fields for managing colors,
   gamma correction values, color codes for different colors, file pointers and
   descriptors for standard input/output/error and the terminal device, as well
   as counters and indices for color management. Additionally, it contains a
   field for storing the name of the terminal device. This comprehensive
   structure allows for efficient management of the terminal's state and
   configuration in a structured way. */
typedef struct {
    UiRuntime *ui;
    UiConfig *ui_cfg;
    SCREEN *screen;
    double red_gamma;             /**< red gamma correction value */
    double green_gamma;           /**< green gamma correction value */
    double blue_gamma;            /**< blue gamma correction value */
    double gray_gamma;            /**< gray gamma correction value */
    char black[COLOR_LEN];        /**< color code for black */
    char red[COLOR_LEN];          /**< color code for red */
    char green[COLOR_LEN];        /**< color code for green */
    char yellow[COLOR_LEN];       /**< color code for yellow */
    char blue[COLOR_LEN];         /**< color code for blue */
    char magenta[COLOR_LEN];      /**< color code for magenta */
    char cyan[COLOR_LEN];         /**< color code for cyan */
    char white[COLOR_LEN];        /**< color code for white */
    char orange[COLOR_LEN];       /**< color code for orange */
    char bblack[COLOR_LEN];       /**< color code for bold black */
    char bred[COLOR_LEN];         /**< color code for bold red */
    char bgreen[COLOR_LEN];       /**< color code for bold green */
    char byellow[COLOR_LEN];      /**< color code for bold yellow */
    char bblue[COLOR_LEN];        /**< color code for bold blue */
    char bmagenta[COLOR_LEN];     /**< color code for bold magenta */
    char bcyan[COLOR_LEN];        /**< color code for bold cyan */
    char bwhite[COLOR_LEN];       /**< color code for bold white */
    char borange[COLOR_LEN];      /**< color code for bold orange */
    char abg[COLOR_LEN];          /**< color code for background with alpha */
    char fg[COLOR_LEN];           /**< foreground color index */
    char bg[COLOR_LEN];           /**< background color index */
    char box_fg[COLOR_LEN];       /**< box foreground */
    char box_bg[COLOR_LEN];       /**< box background */
    char brackets_fg[COLOR_LEN];  /**< brackets foreground */
    char brackets_bg[COLOR_LEN];  /**< brackets background */
    char fill_char_fg[COLOR_LEN]; /**< fill character foreground */
    char fill_char_bg[COLOR_LEN]; /**< fill character background */
    char ln_fg[COLOR_LEN];        /**< line number color index */
    char ln_bg[COLOR_LEN];        /**< line number background index */
    char cmdln_fg[COLOR_LEN];     /**< line number color index */
    char cmdln_bg[COLOR_LEN];     /**< line number background index */
    char nt_fg[COLOR_LEN];        /**< color code for normal text foreground */
    char nt_bg[COLOR_LEN];        /**< color code for normal text background */
    char nt_rev_fg[COLOR_LEN];    /**< normal text reverse foreground */
    char nt_rev_bg[COLOR_LEN];    /**< normal text reverse background */
    char nt_hl_fg[COLOR_LEN];     /**< normal text highlight foreground */
    char nt_hl_bg[COLOR_LEN];     /**< normal text highlight background */
    char
        nt_hl_rev_fg[COLOR_LEN]; /**< normal text highlight reverse foreground */
    char
        nt_hl_rev_bg[COLOR_LEN]; /**< normal text highlight reverse background */
    char title_fg[COLOR_LEN];    /**< title foreground */
    char title_bg[COLOR_LEN];    /**< title background */
    char tty_name[MAXLEN];       /**< name of the terminal device */
    FILE *stdin_fp;              /**< stdin stream pointer */
    FILE *stdout_fp;             /**< stdout stream pointer */
    FILE *stderr_fp;             /**< stderr stream pointer */
    FILE *tty_fp;                /**< terminal device stream pointer */
    int stdin_fd;                /**< stdin file descriptor */
    int stdout_fd;               /**< stdout file descriptor */
    int stderr_fd;               /**< stderr file descriptor */
    int tty_fd;                  /**< terminal device file descriptor */
    int clr_cnt;                 /**< number of colors currently in use */
    int clr_pair_cnt;            /**< number of color pairs currently in use */
    int clr_idx;                 /**< current color index */
    int clr_pair_idx;            /**< current color pair index */
    int cp_default;              /**< default color pair index */
    int cp_norm;                 /**< normal color pair index */
    int cp_win;                  /**< window color pair index */
    int cp_nt_rev;               /**< reverse color pair index */
    int cp_nt_hl;                /**< highlight color pair index */
    int cp_nt_hl_rev;            /**< reverse highlight color pair index */
    int cp_box;                  /**< box color pair index */
    int cp_bold;                 /**< bold color pair index */
    int cp_title;                /**< title color pair index */
    int cp_highlight;            /**< highlight color pair index */
    int cp_ln;                   /**< line number color pair index */
    int cp_cmdln;                /**< line number color pair index */
} SIO;
extern void destroy_curses();
extern int a_toi(char *, bool *);
extern bool chrep(char *, char, char);
extern char *rep_substring(const char *, const char *, const char *);
extern size_t strip_ansi(char *, char *);
extern bool strip_quotes(char *);
extern bool stripz_quotes(char *);
extern int str_to_args(char **, char *, int);
extern int destroy_argv(int argc, char **argv);
extern bool str_to_bool(const char *);
extern bool str_to_lower(char *);
extern bool str_to_upper(char *);
extern bool strnfill(char *, char, int);
extern bool str_subc(char *, char *, char, char *, int);
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
extern void apply_gamma(RGB *);
extern int get_clr_pair(int, int);
extern int clr_name_to_idx(char *);
extern int rgb_to_xterm256_idx(RGB *);
extern bool init_clr_palette(SIO *);
extern bool open_curses(SIO *);
extern int rgb_clr_to_cube(int);
extern int rgb_to_curses_clr(RGB *);
extern RGB xterm256_idx_to_rgb(int);
extern int fork_exec(char **);
extern int full_screen_fork_exec(char **);
extern int full_screen_shell(char *);
extern int shell(char *);
extern char errmsg[];
extern void get_rfc3339_s(char *, size_t);
extern int open_log(char *);
extern void write_log(char *);
extern void compile_chyron(Chyron *);
extern void display_chyron(WINDOW *, Chyron *, int, int);
extern int get_chyron_key(Chyron *, int);
extern bool is_set_chyron_key(Chyron *, int);
extern void set_chyron_key(Chyron *, int, char *, int);
extern void set_chyron_key_cp(Chyron *, int, char *, int, int);
extern void unset_chyron_key(Chyron *, int);
extern Chyron *new_chyron();
extern Chyron *destroy_chyron(Chyron *chyron);
extern void abend(int, char *);
extern void display_argv_error_msg(char *, char **);
extern int display_error(char *, char *, char *, char *);
extern int answer_yn(char *, char *, char *, char *);
extern int display_ok_message(char *);
extern int Perror(char *);
extern void user_end();
extern unsigned long a_to_ul(const char *);
extern size_t canonicalize_file_spec(char *);
extern bool construct_file_spec(char *, char *, char *, char *, char *, int);
extern bool file_spec_path(char *, char *);
extern bool file_spec_name(char *, char *);
extern bool is_directory(const char *);
extern bool is_symlink_to_dir(const char *);
extern bool is_valid_regex(const char *);
extern bool dir_name(char *, char *);
extern bool base_name(char *, char *);
extern bool expand_tilde(char *, int);
extern bool locate_file_in_path(char *, char *);
extern bool normalize_file_spec(char *);
extern bool trim_ext(char *, char *);
extern bool trim_path(char *);
extern bool verify_file(char *, int);
extern bool verify_file_q(char *, int);
extern bool verify_dir(char *, int);
extern bool verify_dir_q(char *, int);
extern bool verify_spec_arg(char *, char *, char *, char *, int);
extern size_t mk_cmplx_str(cchar_t *, char *, attr_t, int);
extern size_t str_to_cc(cchar_t *, const char *, attr_t, int, size_t);
extern void display_cmplx_str(WINDOW *, cchar_t *, int, int);
extern int wccp_to_str(wchar_t, uint8_t *);

#endif
