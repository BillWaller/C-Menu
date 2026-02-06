//  cm.h
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
/// ╭───────────────────────────────────────────────────────────────╮
/// │ FLAGS                                                         │
/// ╰───────────────────────────────────────────────────────────────╯
/// Search flags
/// ALL     List all files including hidden files
/// RECURSE Recurse into subdirectories
/// ICASE   Ignore case in search
#define ALL 0x01
#define RECURSE 0x02
#define ICASE 0x04
#define W_BOX 0x02
#define COLOR_LEN 8
#define LIBCM_VERSION "libcm-0.2.8"

#define __end_pgm                                                              \
    static void end_pgm(void) {                                                \
        win_del();                                                             \
        destroy_curses();                                                      \
        restore_shell_tioctl();                                                \
        exit(EXIT_FAILURE);                                                    \
    }

#define __atexit                                                               \
    {                                                                          \
        int rc;                                                                \
        rc = atexit(end_pgm);                                                  \
        if (rc != 0) {                                                         \
            fprintf(stderr, "\nCannot set exit function\n");                   \
            exit(EXIT_FAILURE);                                                \
        }                                                                      \
    }

typedef struct {
    char text[32];
    int keycode;
    int end_pos;
} key_cmd_tbl;

extern key_cmd_tbl key_cmd[20];

typedef struct {
    int r;
    int g;
    int b;
} RGB;

#define FG_COLOR 2 // green
#define BG_COLOR 0 // black
#define BO_COLOR 1 // red

extern int cp_default;
extern int cp_norm;
extern int cp_box;
extern int cp_reverse;
extern int clr_idx;
extern int clr_cnt;
extern int clr_pair_idx;
extern int clr_pair_cnt;
extern char const colors_text[][10];

typedef struct {
    int fg;
    int bg;
    int pair_id;
} ColorPair;

/// ╭───────────────────────────────────────────────────────────────────╮
/// │ SCREEN I/O                                                        │
/// ╰───────────────────────────────────────────────────────────────────╯
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
/// ╭───────────────────────────────────────────────────────────────╮
/// │ Signal Processing                                             │
/// ╰───────────────────────────────────────────────────────────────╯
extern void signal_handler(int);
extern int handle_signal(sig_atomic_t);
extern volatile sig_atomic_t sig_received;
extern void sig_prog_mode();
extern void sig_dfl_mode();
/// ╭───────────────────────────────────────────────────────────────╮
/// │ NCurses                                                       │
/// ╰───────────────────────────────────────────────────────────────╯
#define REASSIGN_STDIN
extern cchar_t CCC_NORM;
extern cchar_t CCC_BOX;
extern cchar_t CCC_REVERSE;
/// ╭───────────────────────────────────────────────────────────────╮
/// │ NCurses Key Definitions                                       │
/// ╰───────────────────────────────────────────────────────────────╯
#undef key_left
#undef key_right
#undef key_down
#undef key_up

#define KEY_ALTF0 0x138
#define KEY_ALTF(n) (KEY_ALTF0 + (n))

// #define XTERM_GHOSTTY
// #define XTERM_GHOSTTY
// #define XTERM_KITTY
// #define ALACRITTY_DIRECT
// #define XTERM_KITTY
#define XTERM_256COLOR
/// ╭───────────────────────────────────────────────────────────────╮
/// │ EXTENDED NCURSES KEYS                                         │
/// ╰───────────────────────────────────────────────────────────────╯
/// Key code bindings are defined in terminfo
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

/// ╭───────────────────────────────────────────────────────────────╮
/// │ WIDE CHARACTER LINE DRAWING                                   │
/// ╰───────────────────────────────────────────────────────────────╯
#define BW_HO L'\x2500'
#define BW_VE L'\x2502'
#define BW_TL L'\x250C'
#define BW_TR L'\x2510'
#define BW_BL L'\x2514'
#define BW_BR L'\x2518'
#define BW_RTL L'\x256d'
#define BW_RTR L'\x256e'
#define BW_RBL L'\x2570'
#define BW_RBR L'\x256f'
#define BW_LT L'\x251C'
#define BW_TT L'\x252C'
#define BW_RT L'\x2524'
#define BW_CR L'\x253C'
#define BW_BT L'\x2534'
#define BW_SP L'\x20'

extern const wchar_t bw_ho;
extern const wchar_t bw_ve;
extern const wchar_t bw_tl;
extern const wchar_t bw_tr;
extern const wchar_t bw_bl;
extern const wchar_t bw_br;
extern const wchar_t bw_lt;
extern const wchar_t bw_tt;
extern const wchar_t bw_rt;
extern const wchar_t bw_cr;
extern const wchar_t bw_bt;

extern int n_lines;
extern int n_cols;
extern int lines;
extern int cols;
extern int begx;
extern int begy;

//  ╭───────────────────────────────────────────────────────────────╮
//  │ Windowing                                                     │
//  ╰───────────────────────────────────────────────────────────────╯
#define MAXWIN 20
typedef unsigned char uchar;
//  ╭───────────────────────────────────────────────────────────────╮
//  │ SCRIOU                                                        │
//  ╰───────────────────────────────────────────────────────────────╯
extern void destroy_curses();
extern void sig_prog_mode();
extern void sig_shell_mode();
extern char di_getch();
extern int enter_option();

extern WINDOW *win;
extern WINDOW *win_win[MAXWIN];
extern WINDOW *win_box[MAXWIN];

extern int win_attr;
extern int win_attr_odd;
extern int win_attr_even;
extern int win_ptr;
extern int mlines;
extern int mcols;
extern int mbegy;
extern int mbegx;
extern int mg_action, mg_col, mg_line;
extern int mouse_support;
//  ╭───────────────────────────────────────────────────────────────╮
//  │ WINDOWS FUNCTIONS                                             │
//  ╰───────────────────────────────────────────────────────────────╯
extern WINDOW *win_open_box(int, int, int, int, char *);
extern WINDOW *winOpenwin(int, int, int, int);
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
/// ╭───────────────────────────────────────────────────────────────╮
/// │ String Object types                                           │
/// ╰───────────────────────────────────────────────────────────────╯
typedef struct {
    char *s;
    size_t l; // allocated length
} Arg;
typedef struct {
    Arg **v;
    size_t n; // allocated array elements
} Argv;
typedef struct {
    char *s;
    size_t l; // allocated length
} String;
typedef struct {
    wchar_t *s;
    size_t l; // allocated length
} WCStr;
typedef struct {
    cchar_t *s;
    size_t l; // allocated length
} CCStr;

#define to_uppercase(c)                                                        \
    if (c >= 'a' && c <= 'z')                                                  \
    c -= ' '
//  ╭───────────────────────────────────────────────────────────────╮
//  │ SIO DATA STRUCTURE                                            │
//  ╰───────────────────────────────────────────────────────────────╯
typedef struct {
    int fg_color;
    int bg_color;
    int bo_color;
    double red_gamma;
    double green_gamma;
    double blue_gamma;
    double gray_gamma;
    char black[COLOR_LEN];
    char red[COLOR_LEN];
    char green[COLOR_LEN];
    char yellow[COLOR_LEN];
    char blue[COLOR_LEN];
    char magenta[COLOR_LEN];
    char cyan[COLOR_LEN];
    char white[COLOR_LEN];
    char orange[COLOR_LEN];
    char bblack[COLOR_LEN];
    char bred[COLOR_LEN];
    char bgreen[COLOR_LEN];
    char byellow[COLOR_LEN];
    char bblue[COLOR_LEN];
    char bmagenta[COLOR_LEN];
    char bcyan[COLOR_LEN];
    char bwhite[COLOR_LEN];
    char borange[COLOR_LEN];
    char bg[COLOR_LEN];
    char abg[COLOR_LEN];
    char tty_name[MAXLEN];
    FILE *stdin_fp;
    FILE *stdout_fp;
    FILE *stderr_fp;
    FILE *tty_fp;
    int stdin_fd;
    int stdout_fd;
    int stderr_fd;
    int tty_fd;
    int clr_cnt;
    int clr_pair_cnt;
    int clr_idx;
    int clr_pair_idx;
    int cp_default;
    int cp_norm;
    int cp_reverse;
    int cp_box;
    int cp_bold;
    int cp_title;
    int cp_highlight;
} SIO;
//  ╭───────────────────────────────────────────────────────────────╮
//  │ STRING UTILITIES                                              │
//  ╰───────────────────────────────────────────────────────────────╯
extern int a_toi(char *, bool);
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
extern char *str_tok_r(char *, const char *, char **, char *);
//  ╭───────────────────────────────────────────────────────────────╮
//  │ COLOR UTILITIES                                               │
//  ╰───────────────────────────────────────────────────────────────╯
extern void apply_gamma(RGB *);
extern int get_clr_pair(int, int);
extern int clr_name_to_idx(char *);
extern int rgb_to_xterm256_idx(RGB);
extern bool init_clr_palette(SIO *);
extern bool open_curses(SIO *);
extern int rgb_clr_to_cube(int);
extern int rgb_to_curses_clr(RGB rgb);
extern RGB xterm256_idx_to_rgb(int);
//  ╭───────────────────────────────────────────────────────────────╮
//  │ EXEC UTILITIES                                                │
//  ╰───────────────────────────────────────────────────────────────╯
extern int fork_exec(char **);
extern int full_screen_fork_exec(char **);
extern int full_screen_shell(char *);
extern int shell(char *);
//  ╭───────────────────────────────────────────────────────────────╮
//  │ MISCELANEOUS UTILITIES                                        │
//  ╰───────────────────────────────────────────────────────────────╯
extern char errmsg[];
extern void get_rfc3339_s(char *, size_t);
extern int open_log(char *);
extern void write_log(char *);
//  ╭───────────────────────────────────────────────────────────────╮
//  │ CHYRON FUNCTIONS                                              │
//  ╰───────────────────────────────────────────────────────────────╯
extern int chyron_mk(key_cmd_tbl *, char *);
extern int get_chyron_key(key_cmd_tbl *, int);
extern bool is_set_fkey(int);
extern void set_fkey(int, char *);
extern void unset_fkey(int);
//  ╭───────────────────────────────────────────────────────────────╮
//  │ ERROR HANDLING                                                │
//  ╰───────────────────────────────────────────────────────────────╯
extern void abend(int, char *);
extern void display_argv_error_msg(char *, char **);
extern int display_error(char *, char *, char *, char *);
extern int display_ok_message(char *);
extern int error_message(char **);
extern int Perror(char *);
extern void user_end();
//  ╭───────────────────────────────────────────────────────────────╮
//  │ FILE UTILITIES                                                │
//  ╰───────────────────────────────────────────────────────────────╯
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
