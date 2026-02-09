/// cm.h
/// Headder for C-Menu API, libcm.so
//  Bill Waller Copyright (c) 2025
//  MIT License
//  includes for libcm.so
//  billxwaller@gmail.com

#ifndef _CM_H
#define _CM_H 1

#define _XOPEN_SOURCE_EXTENDED 1
#define NCURSES_WIDECHAR 1
#include <ncursesw/ncurses.h>
#include <signal.h>
#include <stddef.h>
#include <stdlib.h>

#define MAXLEN 256
#define MAXARGS 64
#define MAX_DEPTH 3
/// List all files including hidden files
#define ALL 0x01
/// Recurse into subdirectories
#define RECURSE 0x02
/// Ignore case in search
#define ICASE 0x04
#define W_BOX 0x02
#define COLOR_LEN 8
#define LIBCM_VERSION "libcm-0.2.8"

/// This macro defines a function end_pgm() that is registered with atexit() to
/// ensure that when the program exits, it will clean up the ncurses environment
/// and restore the terminal settings before exiting. This is important to
/// prevent the terminal from being left in an unusable state after the program
/// terminates.shell_out_tioctl
///
/// Programs using libcm should call __atexit in their main function to ensure
/// that the end_pgm function is registered to be called on program exit. This
/// will help ensure that the terminal is properly restored to its original
/// state, even if the program encounters an error or is terminated
/// unexpectedly.
#define __end_pgm                                                              \
    static void end_pgm(void) {                                                \
        win_del();                                                             \
        destroy_curses();                                                      \
        restore_shell_tioctl();                                                \
        exit(EXIT_FAILURE);                                                    \
    }

/// This macro registers the end_pgm function to be called when the program
/// exits. It checks the return value of atexit() to ensure that the
/// registration was successful, and if not, it prints an error message and
/// exits with a failure status. Programs using libcm should call __atexit in
/// their main function to ensure that the end_pgm function is registered to be
/// called on program exit. This will help ensure that the terminal is properly
/// restored to its original state, even if the program encounters an error or
/// is terminated unexpectedly.
#define __atexit                                                               \
    {                                                                          \
        int rc;                                                                \
        rc = atexit(end_pgm);                                                  \
        if (rc != 0) {                                                         \
            fprintf(stderr, "\nCannot set exit function\n");                   \
            exit(EXIT_FAILURE);                                                \
        }                                                                      \
    }

/// The key_cmd_tbl structure is a simple structure that holds information about
/// a command associated with a key. It includes the command text, the key code,
/// and the end position of the command text in the chyron. This structure can
/// be used to map specific keys to commands and their positions in the chyron,
/// allowing for easy retrieval of the command associated with a particular key
/// press. By using this structure, you can manage and organize the commands and
/// their corresponding key codes in a structured way, making it easier to
/// implement functionality that responds to specific key presses in the
/// terminal interface. The key_cmd_tbl structure can be used in conjunction
/// with an array of key_cmd_tbl instances to create a mapping of multiple keys
/// to their respective commands and positions in the chyron, enabling a more
/// dynamic and interactive terminal experience.
typedef struct {
    /// command text
    char text[32];
    /// key code
    int keycode;
    /// end position of text in chyron
    int end_pos;
} key_cmd_tbl;

/// key_cmd_table
extern key_cmd_tbl key_cmd[20];

/// RGB color structure
typedef struct {
    /// red component (0-255)
    int r;
    /// green component (0-255)
    int g;
    /// blue component (0-255)
    int b;
} RGB;

/// default foreground color
#define FG_COLOR 2
/// default background color
#define BG_COLOR 0
/// default bold foreground color
#define BO_COLOR 1

/// default color pair index
extern int cp_default;
/// normal color pair index
extern int cp_norm;
/// box color pair index
extern int cp_box;
/// bold color pair index
extern int cp_bold;
/// title color pair index
extern int cp_title;
/// highlight color pair index
extern int cp_highlight;
/// reverse color pair index
extern int cp_reverse;
/// current color index
extern int clr_idx;
/// number of colors used
extern int clr_cnt;
/// current color pair index
extern int clr_pair_idx;
/// number of color pairs supported by the terminal
extern int clr_pair_cnt;
/// color codes for the 16 basic colors (black, red, green, yellow, blue,
/// magenta, cyan, white, and their bold variants)
extern char const colors_text[][10];

// The ColorPair structure is a simple structure that holds information about a
// color pair, including the foreground color index, background color index, and
// the color pair index. This structure can be used to manage and apply color
// pairs in the ncurses library, allowing for easy customization of the
// terminal's appearance. By using this structure, you can easily keep track of
// the different color pairs you have defined and apply them to various elements
// in your terminal interface.
typedef struct {
    /// foreground color index
    int fg;
    /// background color index
    int bg;
    /// color pair index
    int pair_id;
} ColorPair;

extern struct termios shell_tioctl, curses_tioctl;
extern struct termios shell_in_tioctl, curses_in_tioctl;
extern struct termios shell_out_tioctl, curses_out_tioctl;
extern struct termios shell_err_tioctl, curses_err_tioctl;

extern bool f_have_shell_tioctl;
extern bool f_have_curses_tioctl;
extern bool f_curses_open;
extern bool f_restore_screen;
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
extern int handle_signal(sig_atomic_t);
extern volatile sig_atomic_t sig_received;
extern void sig_prog_mode();
extern void sig_dfl_mode();

#define REASSIGN_STDIN
/// normal color pair complex character
extern cchar_t CCC_NORM;
/// box color pair complex character
extern cchar_t CCC_BOX;
/// reverse color pair complex character
extern cchar_t CCC_REVERSE;

#undef key_left
#undef key_right
#undef key_down
#undef key_up

#define KEY_ALTF0 0x138
#define KEY_ALTF(n) (KEY_ALTF0 + (n))
/// assume terminfo name for altkey definitinons
#define XTERM_256COLOR
/// EXTENDED NCURSES KEYS
///
/// Key code bindings are customarily defined in terminfo.
/// xterm-256color tends to be the most complete, and indeed, it seems to
/// work with Ghostty, Kitty, and Alacritty. If you want to use the
/// altkey bindings, you may need to set your terminal's TERM environment
/// variable to xterm-256color. Here's why:
///
/// Ghostty, Kitty, and Alacritty all come with their own terminfo files:
///
/// Ghostty     with xterm-ghostty,
/// Kitty       with xterm-kitty, and
/// Alacritty   with alacritty
///
/// None of the three produced the correct extended altkey bindings when
/// paired with the terminfo provided in their distribution, yet all
/// three worked with xterm-256color.
///
/// The extended altkeys were not defined in any of the terminfo files, not
/// even xterm-256color. Yet, xterm-256color produced consistent keycode
/// bindings with all three emulators. The standard key bindings, ins, delete,
/// up, down arrows, etc., were the same for all three terminfos, and all
/// three emulators produced the correct key bindings for those keys.
///
/// The fact that xterm-256color produced consistent keycode bindings for the
/// altkeys, while the other three terminfos did not, is likely due to the fact
/// that xterm-256color is more widely used and has better support for extended
/// key bindings, even if they are not explicitly defined in the terminfo file.
/// It's possible that xterm-256color has some default behavior or fallback
/// mechanism that allows it to recognize and produce key codes for the altkeys,
/// while the other terminfos do not have such a mechanism in place. This could
/// explain why xterm-256color produced consistent keycode bindings for the
/// altkeys across all three emulators, while the other terminfos did not.
///
/// mapped                  altkey   ncurses
/// 2nd fld - 1    name     binding  keycode
/// ------------- --------- -------- -------
/// kIC=\E[2;2~,  key_ins   \E[2;3~,   223
/// kHOM=\E[1;2H, key_home  \E[1;3H,   21e
/// kPRV=\E[5;2~, key_pgup  \E[5;3~,   232
/// kDC=\E[3;2~,  key_del   \E[3;3~,   20e
/// kEND=\E[1;2F, key_end   \E[1;3F,   219
/// kNXT=\E[6;2~, key_pgdn  \E[6;3~,   22d
/// kri=\E[1;2A,  key_up    \E[1;3A,   23d
/// kind=\E[1;2B, key_down  \E[1;3B,   214
/// kRIT=\E[1;2C, key_right \E[1;3C,   237
/// kLFT=\E[1;2D, key_left  \E[1;3D,   228

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

/// UNICODE BOX DRAWING SYMBOLS
/// horizontal line
#define BW_HO L'\x2500'
/// vertical line
#define BW_VE L'\x2502'
/// top left
#define BW_TL L'\x250C'
/// top right
#define BW_TR L'\x2510'
/// bottom left
#define BW_BL L'\x2514'
/// bottom right
#define BW_BR L'\x2518'
/// rounded left tee
#define BW_RTL L'\x256d'
/// rounded right tee
#define BW_RTR L'\x256e'
/// rounded bottom left
#define BW_RBL L'\x2570'
/// rounded bottom right
#define BW_RBR L'\x256f'
/// left tee
#define BW_LT L'\x251C'
/// top tee
#define BW_TT L'\x252C'
/// right tee
#define BW_RT L'\x2524'
/// cross
#define BW_CR L'\x253C'
/// bottom tee
#define BW_BT L'\x2534'
/// space
#define BW_SP L'\x20'

/// The following are the actual wchar_t variables that will hold the box
/// drawing characters. These correspond to the above Unicode code points. By
/// defining them as wchar_t, we can use them in the ncurses library to draw
/// boxes and lines in the terminal. The variables are named with a "bw_" prefix
/// to indicate that they are box (wide) drawing characters, and they will be
/// initialized with the corresponding Unicode characters defined above. By
/// using wchar_t, we can ensure that the characters are properly represented
/// and displayed in the terminal, especially when using wide character support
/// in ncurses. The variables are declared as extern, which means they will be
/// defined and initialized in the corresponding source file (e.g., cm.c) where
/// the actual values will be assigned to them. This allows other parts of the
/// program to use these variables to draw boxes and lines in the terminal using
/// the appropriate Unicode characters.
/// Note that the actual initialization of these variables will need to be done
/// in the source file, where they will be assigned the corresponding Unicode
/// characters defined above. This way, they can be used throughout the program
/// to draw boxes and lines in the terminal interface using the appropriate box
/// drawing characters. Note also that the use of wchar_t and wide character
/// support in ncurses is essential for properly displaying these Unicode box
/// drawing characters, as they may not be represented correctly using regular
/// char types. By using wchar_t, we can ensure that the characters are
/// displayed correctly in the terminal, especially when using a terminal
/// emulator that supports Unicode and wide characters. Overall, these variables
/// will serve as convenient references to the box drawing characters, allowing
/// for easy use in the ncurses library to create visually appealing terminal
/// interfaces with boxes and lines. Horizontal line
extern const wchar_t bw_ho;
/// Vertical line
extern const wchar_t bw_ve;
/// Top left corner
extern const wchar_t bw_tl;
/// Top right corner
extern const wchar_t bw_tr;
/// Bottom left corner
extern const wchar_t bw_bl;
/// Bottom right corner
extern const wchar_t bw_br;
/// Left tee
extern const wchar_t bw_lt;
/// Top tee
extern const wchar_t bw_tt;
/// Right tee
extern const wchar_t bw_rt;
/// Cross
extern const wchar_t bw_cr;
/// Bottom tee
extern const wchar_t bw_bt;

/// number of lines in the terminal
extern int n_lines;
/// number of columns in the terminal
extern int n_cols;
/// current number of lines (may be less than n_lines if the
extern int lines;
/// current number of columns (may be less than n_cols if the
extern int cols;
/// beginning x coordinate of the terminal
extern int begx;
/// beginning y coordinate of the terminal
extern int begy;

#define MAXWIN 20
typedef unsigned char uchar;

extern void destroy_curses();
extern void sig_prog_mode();
extern void sig_shell_mode();
extern char di_getch();
extern int enter_option();

/// generic windows pointer, used for various purposes
extern WINDOW *win;
/// array of pointers to windows, indexed by window ID
extern WINDOW *win_win[MAXWIN];
/// array of pointers to box windows, indexed by window ID
extern WINDOW *win_box[MAXWIN];

/// Ncurses attributes for the current window
extern int win_attr;
/// Ncurses attributes for the current window odd lines
extern int win_attr_odd;
/// Ncurses attributes for the current window even lines
extern int win_attr_even;
/// Pointer to the current window pair, box and window
extern int win_ptr;
/// number of lines
extern int mlines;
/// number of columns
extern int mcols;
/// beginning y coordinate of the current window
extern int mbegy;
/// beginning x coordinate of the current window
extern int mbegx;
/// action in progress
extern int mg_action;
/// window column
extern int mg_col;
/// window line
extern int mg_line;

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

/// The Arg structure represents a string argument with a pointer to the string
/// and its allocated length. This structure can be used to manage and
/// manipulate string arguments in the terminal interface, allowing for dynamic
/// handling of user input and output. By using this structure, you can easily
/// manage memory allocation for string arguments, as well as keep track of
/// their lengths, which can be useful for various operations such as argument
/// parsing and displaying text in the terminal interface. Overall, the Arg
/// structure provides a convenient way to handle string arguments in the
/// terminal interface, allowing for more flexible and dynamic interactions with
/// the user.
typedef struct {
    char *s;
    size_t l; /// allocated length
} Arg;
/// The Argv structure represents an array of arguments, containing a pointer to
/// an array of Arg pointers and the number of allocated array elements. This
/// structure can be used to manage and manipulate arrays of arguments in the
/// terminal interface, allowing for dynamic handling of user input and output.
/// By using this structure, you can easily manage memory allocation for arrays
/// of arguments, as well as keep track of their lengths, which can be useful
/// for various operations such as argument parsing and displaying text in the
/// terminal interface. Overall, the Argv structure provides a convenient way to
/// handle arrays of arguments in the terminal interface, allowing for more
/// flexible and dynamic interactions with the user.
typedef struct {
    Arg **v;
    size_t n; /// allocated array elements
} Argv;
/// The String structure represents a string with a pointer to the string and
/// its allocated length.
typedef struct {
    char *s;
    size_t l; /// allocated length
} String;
/// The WCStr structure represents a wide character string with a pointer to the
/// wide character string and its allocated length.
typedef struct {
    wchar_t *s;
    size_t l; /// allocated length
} WCStr;
/// The CCStr structure represents a complex character string with a pointer to
/// the complex character string and its allocated length.
typedef struct {
    cchar_t *s;
    size_t l; /// allocated length
} CCStr;

#define to_uppercase(c)                                                        \
    if (c >= 'a' && c <= 'z')                                                  \
    c -= ' '

/// SIO structure
/// The SIO structure is a comprehensive structure that encapsulates various
/// aspects of the terminal's state and configuration. It includes fields for
/// managing colors, gamma correction values, color codes for different colors,
/// file pointers and descriptors for standard input/output/error and the
/// terminal device, as well as counters and indices for color management.
/// Additionally, it contains a field for storing the name of the terminal
/// device. This structure serves as a central repository for all relevant
/// information needed to manage the terminal's appearance and behavior
/// effectively.
///
/// @note When referring to the terminal colors 0-15, we often use black, red,
/// gree, yellow, blue, magenta, cyan and white as labels for the color indexes.
/// The actual colors can be customized by the user, and the color codes
/// occupying index-0, may be referred to by C-Menu code as black, when it
/// actually some other color, having been redefined by the user.
/// @note The gamma correction values can also be adjusted to achieve the
/// desired color output. Gamma correction is a technique used to adjust the
/// brightness and contrast of colors, and it can be particularly useful for
/// fine-tuning the appearance of colors in the terminal. By adjusting the gamma
/// values for red, green, blue, and gray, users can achieve a more visually
/// appealing and personalized terminal experience.
/// @note The SIO structure also includes fields for managing the terminal's
/// file pointers and descriptors, which are essential for input/output
/// operations. Overall, the SIO structure provides a comprehensive framework
/// for managing the terminal's colors and related settings, allowing for a
/// highly customizable and visually appealing terminal experience.

typedef struct {
    /// foreground color index
    int fg_color;
    /// background color index
    int bg_color;
    /// bold color index
    int bo_color;
    /// red gamma correction value
    double red_gamma;
    /// green gamma correction value
    double green_gamma;
    /// blue gamma correction value
    double blue_gamma;
    /// gray gamma correction value
    double gray_gamma;
    /// color code for black
    char black[COLOR_LEN];
    /// color code for red
    char red[COLOR_LEN];
    /// color code for green
    char green[COLOR_LEN];
    /// color code for yellow
    char yellow[COLOR_LEN];
    /// color code for blue
    char blue[COLOR_LEN];
    /// color code for magenta
    char magenta[COLOR_LEN];
    /// color code for cyan
    char cyan[COLOR_LEN];
    /// color code for white
    char white[COLOR_LEN];
    /// color code for orange
    char orange[COLOR_LEN];
    /// color code for bold black
    char bblack[COLOR_LEN];
    /// color code for bold red
    char bred[COLOR_LEN];
    /// color code for bold green
    char bgreen[COLOR_LEN];
    /// color code for bold yellow
    char byellow[COLOR_LEN];
    /// color code for bold blue
    char bblue[COLOR_LEN];
    /// color code for bold magenta
    char bmagenta[COLOR_LEN];
    /// color code for bold cyan
    char bcyan[COLOR_LEN];
    /// color code for bold white
    char bwhite[COLOR_LEN];
    /// color code for bold orange
    char borange[COLOR_LEN];
    /// color code for background
    char bg[COLOR_LEN];
    /// color code for background with alpha
    char abg[COLOR_LEN];
    /// name of the terminal device
    char tty_name[MAXLEN];
    /// stdin stream pointer
    FILE *stdin_fp;
    /// stdout stream pointer
    FILE *stdout_fp;
    /// stderr stream pointer
    FILE *stderr_fp;
    /// terminal device stream pointer
    FILE *tty_fp;
    /// stdin file descriptor
    int stdin_fd;
    /// stdout file descriptor
    int stdout_fd;
    /// stderr file descriptor
    int stderr_fd;
    /// terminal device file descriptor
    int tty_fd;
    /// number of colors currently in use
    int clr_cnt;
    /// number of color pairs currently in use
    int clr_pair_cnt;
    /// current color index
    int clr_idx;
    /// current color pair index
    int clr_pair_idx;
    /// default color pair
    int cp_default;
    /// normal color pair
    int cp_norm;
    /// reverse color pair
    int cp_reverse;
    /// box color pair
    int cp_box;
    /// bold color pair
    int cp_bold;
    /// title color pair
    int cp_title;
    /// highlight color pair
    int cp_highlight;
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
extern bool free_string(String);
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
