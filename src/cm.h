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

#define _XOPEN_SOURCE_EXTENDED 1 /**< Enable wide character support */
#define NCURSES_WIDECHAR 1       /**< Enable wide character support */
#include <ncursesw/ncurses.h>
#include <signal.h>
#include <stddef.h>
#include <stdlib.h>

#define MAXLEN 256    /**< maximum length for strings and buffers */
#define MAXARGS 64    /**< maximum number of arguments */
#define SCR_COLS 1024 /**< maximum number of columns in the terminal screen */
#define MAX_DEPTH 3   /**< maximum depth for recursive file searching */
#define ALL 0x01      /**< List all files including hidden files */
#define RECURSE 0x02  /**< Recurse into subdirectories */
#define ICASE 0x04    /**< Ignore case in search */
#define W_BOX 0x02    /**< box window flag for win_new() */
#define COLOR_LEN 8   /**< length of color code strings */
#define DEFAULTSHELL "/bin/bash"
#define LIBCM_VERSION "libcm-0.2.8" /**< version string for libcm */
#define S_WCOK 0x1000               /**< write or create permitted */
#define S_QUIET 0x2000              /**< quiet mode flag for file validation */
/** @brief This macro defines the function, end_pgm, that is responsible
   for cleaning up the terminal state and exiting the program.
    @note This function is typically registered to be called when the program
   exits, ensuring that the terminal is properly restored to its original state,
   even if the program encounters an error or is terminated unexpectedly. The
   end_pgm function performs the following actions:
    1. It calls win_del() to delete any windows that may have been created
   during the program's execution.
    2. It calls destroy_curses() to clean up the ncurses library and restore the
   terminal to its normal state.
    3. It calls restore_shell_tioctl() to restore the terminal's input/output
   settings to their original state.
    4. Finally, it calls exit(EXIT_FAILURE) to terminate the program with a
   failure status, indicating that the program has encountered an error or is
   exiting due to an unexpected condition. By defining this function and
   registering it to be called on program exit, you can help ensure that the
   terminal is properly cleaned up and restored, preventing any issues with the
   terminal state after the program has exited. */
#define __end_pgm                                                              \
    static void end_pgm(void) {                                                \
        win_del();                                                             \
        destroy_curses();                                                      \
        restore_shell_tioctl();                                                \
        sig_dfl_mode();                                                        \
        exit(EXIT_FAILURE);                                                    \
    }
/** @brief This macro registers the end_pgm function to be called when the
   program exits.
    @note It checks the return value of atexit() to ensure that the
    registration was successful, and if not, it prints an error message and
    exits with a failure status. Programs using libcm should call __atexit in
    their main function to ensure that the end_pgm function is registered to be
    called on program exit. This will help ensure that the terminal is properly
    restored to its original state, even if the program encounters an error or
    is terminated unexpectedly. */
#define __atexit                                                               \
    {                                                                          \
        int rc;                                                                \
        rc = atexit(end_pgm);                                                  \
        if (rc != 0) {                                                         \
            fprintf(stderr, "\nCannot set exit function\n");                   \
            exit(EXIT_FAILURE);                                                \
        }                                                                      \
    }

/** @struct key_cmd_tbl
    @note The key_cmd_tbl structure is a simple structure that holds information
   about a command associated with a key. It includes the command text, the key
   code, and the end position of the command text in the chyron. This structure
   can be used to map specific keys to commands and their positions in the
   chyron, allowing for easy retrieval of the command associated with a
   particular key press. By using this structure, you can manage and organize
   the commands and their corresponding key codes in a structured way, making it
   easier to implement functionality that responds to specific key presses in
   the terminal interface. The key_cmd_tbl structure can be used in conjunction
   with an array of key_cmd_tbl instances to create a mapping of multiple keys
   to their respective commands and positions in the chyron, enabling a more
   dynamic and interactive terminal experience. */
typedef struct {
    char text[32]; /**< command text associated with the key */
    int keycode;   /**< key code associated with the command */
    int end_pos;   /**< end position of the command text in the chyron */
} key_cmd_tbl;

extern key_cmd_tbl key_cmd[20]; /**< for chyron */

/** @struct RGB */
typedef struct {
    int r; /**< red component (0-255) */
    int g; /**< green component (0-255) */
    int b; /**< blue component (0-255) */
} RGB;

#define FG_COLOR 2 /**< default foreground color */
#define BG_COLOR 0 /**< default background color */
#define BO_COLOR 1 /**< default bold foreground color */

extern int cp_default;   /**< default color pair index */
extern int cp_norm;      /**< normal color pair index */
extern int cp_box;       /**< box color pair index */
extern int cp_bold;      /**< bold color pair index */
extern int cp_title;     /**< title color pair index */
extern int cp_highlight; /**< highlight color pair index */
extern int cp_reverse;   /**< reverse color pair index */
extern int clr_idx;      /**< current color index */
extern int clr_cnt;      /**< number of colors used */
extern int clr_pair_idx; /**< current color pair index */
extern int clr_pair_cnt; /**< number of color pairs supported by the terminal */
extern char const colors_text[][10]; /**< color codes for the 16 basic colors */

/** @struct ColorPair
  @note The ColorPair structure is a simple structure that holds information
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

/**
 * @struct Opts
 * @brief Table elements for options.
 */
typedef struct {
    const char *name;      /**< long name of the option */
    unsigned int type;     /**< data type, string, int, bool */
    unsigned int group;    /**< file, misc, param, flag, etc */
    const char *use;       /**< menu, pick, form, view */
    const char *short_opt; /**< command line short option, e.g. -a -b -c */
    const char *desc;      /**< brief description */
} Opts;

extern void dump_opts_by_use(char *, char *); /**< dump options to stdout */
extern int xwgetch(WINDOW *);
extern bool capture_shell_tioctl();
extern bool restore_shell_tioctl();
extern bool capture_curses_tioctl();
extern bool restore_curses_tioctl();
extern bool mk_raw_tioctl(struct termios *);
extern bool set_sane_tioctl(struct termios *);
extern int win_new(int, int, int, int, char *, int);
extern void win_redraw(WINDOW *);
extern void win_resize(int, int, char *);
extern void signal_handler(int);
extern bool handle_signal(sig_atomic_t);
extern volatile sig_atomic_t sig_received;
extern void sig_prog_mode();
extern void sig_dfl_mode();

extern cchar_t CCC_NORM;    /**< normal color pair complex character */
extern cchar_t CCC_BOX;     /**< box color pair complex character */
extern cchar_t CCC_REVERSE; /**< reverse color pair complex character */

#define KEY_ALTF0 0x138
#define KEY_ALTF(n) (KEY_ALTF0 + (n)) /**< define alt function keys */
#define XTERM_256COLOR /**< use xterm-256color terminfo for altkey bindings */
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
#define BW_RTL L'\x256d' /**< rounded left tee */
#define BW_RTR L'\x256e' /**< rounded right tee */
#define BW_RBL L'\x2570' /**< rounded bottom left */
#define BW_RBR L'\x256f' /**< rounded bottom right */
#define BW_LT L'\x251C'  /**< left tee */
#define BW_TT L'\x252C'  /**< top tee */
#define BW_RT L'\x2524'  /**< right tee */
#define BW_CR L'\x253C'  /**< cross */
#define BW_BT L'\x2534'  /**< bottom tee */
#define BW_SP L'\x20'    /**< space */

/** The following are the actual wchar_t variables that will hold the box
    drawing characters. These correspond to the above Unicode code points. By
    defining them as wchar_t, we can use them in the ncurses library to draw
    boxes and lines in the terminal. The variables are named with a "bw_" prefix
    to indicate that they are box (wide) drawing characters, and they will be
    initialized with the corresponding Unicode characters defined above. */

extern const wchar_t bw_ho; /**< horizontal line */
extern const wchar_t bw_ve; /**< vertical line */
extern const wchar_t bw_tl; /**< top left corner */
extern const wchar_t bw_tr; /**< top right corner */
extern const wchar_t bw_bl; /**< bottom left corner */
extern const wchar_t bw_br; /**< bottom right corner */
extern const wchar_t bw_lt; /**< left tee */
extern const wchar_t bw_tt; /**< top tee */
extern const wchar_t bw_rt; /**< right tee */
extern const wchar_t bw_cr; /**< cross */
extern const wchar_t bw_bt; /**< bottom tee */

extern int n_lines; /**< number of lines in the terminal */
extern int n_cols;  /**< number of columns in the terminal */
extern int lines; /**< current number of lines (may be less than n_lines if the
                     terminal is resized) */
extern int cols;  /**< current number of columns (may be less than n_cols if the
                     terminal is resized) */
extern int begx;  /**< beginning x coordinate of the terminal */
extern int begy;  /**< beginning y coordinate of the terminal */

#define MAXWIN 30 /**< maximum number of windows that can be created */
typedef unsigned char uchar;

extern void destroy_curses();
extern void sig_prog_mode();
extern void sig_shell_mode();
extern char di_getch();
extern int enter_option();

extern WINDOW *win; /**< generic window pointer, used for various purposes */
extern WINDOW
    *win_win[MAXWIN]; /**< array of pointers to windows, indexed by window ID */
extern WINDOW *win_box[MAXWIN]; /**< array of pointers to box windows, indexed
                                   by window ID */

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
    mbegx; /**< beginning x coordinate of the current window, which can be used
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
#define to_uppercase(c)                                                        \
    if (c >= 'a' && c <= 'z')                                                  \
    c -= ' '
/** to_lowercase(c) - convert an uppercase letter to lowercase */
#define to_lowercase(c)                                                        \
    if (c >= 'A' && c <= 'Z')                                                  \
    c += ' '
extern int eargc; /**< general use argument count, for external commands or
                     error messages */
/** earg - general use argument string */
extern char earg_str[MAXLEN]; /**< general use argument string, for external
                                 commands or error messages */
/** eargv - argument vector for external commands, or error messages */
extern char *eargv[MAXARGS];
/** tty_fd - the file descriptor for the terminal, for error messages and other
 */
extern int tty_fd; /**< the file descriptor for the terminal, for error messages
                      and other output */
extern int
    dbgfd; /**< the file descriptor for debug output, for debugging purposes */
extern int src_line;    /**< the line number of the source file being processed,
                           for error messages */
extern char *src_name;  /**< the name of the source file being processed, for
                           error messages */
extern char fn[MAXLEN]; /**< function name for error messages */
extern char em0[MAXLEN]; /**< error message string for error messages */
extern char em1[MAXLEN]; /**< error message string for error messages */
extern char em2[MAXLEN]; /**< error message string for error messages */
extern char em3[MAXLEN]; /**< error message string for error messages */
extern bool f_debug;     /**< a flag to indicate whether debug output should be
                            printed, for debugging purposes */
extern char tmp_str[MAXLEN]; /**< a temporary string buffer for error messages
                                and other output */
extern char
    *tmp_ptr; /**< a temporary pointer for error messages and other output */
extern unsigned int cmd_key; /**< the command key for the current command, for
                                error messages and other output */
extern int exit_code; /**< the exit code for the program, for error messages and
                         other output */

extern WINDOW *win_del();
extern void destroy_win(WINDOW *);
extern void destroy_box(WINDOW *);
extern void restore_wins();
extern void cbox(WINDOW *);
extern void win_init_attrs(int, int, int);
extern void win_Toggle_Attrs();
extern void mvwaddstr_fill(WINDOW *, int, int, char *, int);
extern int display_curses_keys();
extern void init_stdscr();
extern void curskeys(WINDOW *);
extern void mouse_getch(int *, int *, int *, int *);
extern void w_mouse_getch(WINDOW *, int *, int *, int *, int *);

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
    Arg **v; /**< pointer to an array of Arg pointers, representing the argument
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
   @note The WCStr structure represents a wide character string with a pointer
   to the wide character string and its allocated length. */
typedef struct {
    wchar_t *s; /**< pointer to the wide character string */
    size_t l;   /**< @brief allocated length */
} WCStr;
/** @struct CCStr
   @brief complex character objectl with a pointer to the complex character
   string and its allocated length
   @note The CCStr structure represents a complex character string, which is a
   string that can contain both regular characters and attributes (such as
   color, bold, etc.) in the ncurses library. This structure includes a pointer
   to the complex character string and its allocated length, allowing for
   dynamic handling of complex character strings in the terminal interface. */
typedef struct {
    cchar_t *s; /**< pointer to the complex character string */
    size_t l;   /**< allocated length */
} CCStr;

/** simple macro to convert a character to uppercase */
#define to_uppercase(c)                                                        \
    if (c >= 'a' && c <= 'z')                                                  \
    c -= ' '

/** @struct SIO
    @brief The SIO structure encapsulates various aspects of the terminal's
   state and configuration, including color management, file pointers, and
   terminal device information. @note The SIO structure serves as a central
   repository for all relevant information needed to manage the terminal's
   appearance and behavior effectively, allowing for a highly customizable and
   visually appealing terminal experience. @note The SIO structure includes
   fields for managing colors, gamma correction values, color codes for
   different colors, file pointers and descriptors for standard
   input/output/error and the terminal device, as well as counters and indices
   for color management. Additionally, it contains a field for storing the name
   of the terminal device. This comprehensive structure allows for efficient
   management of the terminal's state and configuration in a structured way. */
typedef struct {
    int fg_color;             /**< foreground color index */
    int bg_color;             /**< background color index */
    int bo_color;             /**< bold color index */
    double red_gamma;         /**< red gamma correction value */
    double green_gamma;       /**< green gamma correction value */
    double blue_gamma;        /**< blue gamma correction value */
    double gray_gamma;        /**< gray gamma correction value */
    char black[COLOR_LEN];    /**< color code for black */
    char red[COLOR_LEN];      /**< color code for red */
    char green[COLOR_LEN];    /**< color code for green */
    char yellow[COLOR_LEN];   /**< color code for yellow */
    char blue[COLOR_LEN];     /**< color code for blue */
    char magenta[COLOR_LEN];  /**< color code for magenta */
    char cyan[COLOR_LEN];     /**< color code for cyan */
    char white[COLOR_LEN];    /**< color code for white */
    char orange[COLOR_LEN];   /**< color code for orange */
    char bblack[COLOR_LEN];   /**< color code for bold black */
    char bred[COLOR_LEN];     /**< color code for bold red */
    char bgreen[COLOR_LEN];   /**< color code for bold green */
    char byellow[COLOR_LEN];  /**< color code for bold yellow */
    char bblue[COLOR_LEN];    /**< color code for bold blue */
    char bmagenta[COLOR_LEN]; /**< color code for bold magenta */
    char bcyan[COLOR_LEN];    /**< color code for bold cyan */
    char bwhite[COLOR_LEN];   /**< color code for bold white */
    char borange[COLOR_LEN];  /**< color code for bold orange */
    char bg[COLOR_LEN];       /**< color code for background */
    char abg[COLOR_LEN];      /**< color code for background with alpha */
    char tty_name[MAXLEN];    /**< name of the terminal device */
    FILE *stdin_fp;           /**< stdin stream pointer */
    FILE *stdout_fp;          /**< stdout stream pointer */
    FILE *stderr_fp;          /**< stderr stream pointer */
    FILE *tty_fp;             /**< terminal device stream pointer */
    int stdin_fd;             /**< stdin file descriptor */
    int stdout_fd;            /**< stdout file descriptor */
    int stderr_fd;            /**< stderr file descriptor */
    int tty_fd;               /**< terminal device file descriptor */
    int clr_cnt;              /**< number of colors currently in use */
    int clr_pair_cnt;         /**< number of color pairs currently in use */
    int clr_idx;              /**< current color index */
    int clr_pair_idx;         /**< current color pair index */
    int cp_default;           /**< default color pair index */
    int cp_norm;              /**< normal color pair index */
    int cp_reverse;           /**< reverse color pair index */
    int cp_box;               /**< box color pair index */
    int cp_bold;              /**< bold color pair index */
    int cp_title;             /**< title color pair index */
    int cp_highlight;         /**< highlight color pair index */
} SIO;
extern int a_toi(char *, bool *);
extern bool chrep(char *, char, char);
extern char *rep_substring(const char *, const char *, const char *);
extern int strip_ansi(char *, char *);
extern bool strip_quotes(char *);
extern bool stripz_quotes(char *);
extern int str_to_args(char **, char *, int);
extern bool str_to_bool(const char *);
extern bool str_to_lower(char *);
extern bool str_to_upper(char *);
extern bool strnfill(char *, char, int);
extern bool str_subc(char *, char *, char, char *, int);
extern size_t strnz(char *, int);
extern char *strz_dup(char *);
extern char *strnz_dup(char *, int);
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
extern int rgb_to_xterm256_idx(RGB);
extern bool init_clr_palette(SIO *);
extern bool open_curses(SIO *);
extern int rgb_clr_to_cube(int);
extern int rgb_to_curses_clr(RGB rgb);
extern RGB xterm256_idx_to_rgb(int);
extern int fork_exec(char **);
extern int full_screen_fork_exec(char **);
extern int full_screen_shell(char *);
extern int shell(char *);
extern char errmsg[];
extern void get_rfc3339_s(char *, size_t);
extern int open_log(char *);
extern void write_log(char *);
extern int chyron_mk(key_cmd_tbl *, char *);
extern int get_chyron_key(key_cmd_tbl *, int);
extern bool is_set_fkey(int);
extern void set_fkey(int, char *);
extern void unset_fkey(int);
extern void abend(int, char *);
extern void display_argv_error_msg(char *, char **);
extern int display_error(char *, char *, char *, char *);
extern int display_ok_message(char *);
extern int error_message(char **);
extern int Perror(char *);
extern void user_end();
extern bool lf_find_dirs(char *, char *, int, int);
extern bool lf_find_files(char *, char *, int);
extern size_t canonicalize_file_spec(char *);
extern bool construct_file_spec(char *, char *, char *, char *, char *, int);
extern bool file_spec_path(char *, char *);
extern bool file_spec_name(char *, char *);
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

#endif
