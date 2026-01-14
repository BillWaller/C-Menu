//  meextern nu
//  Bill Waller Copyright (c) 2025
//  MIT License
//  One large include file for C-Menu Menu, Form, Pick, and View
//  billxwaller@gmail.com

#ifndef _MENU_H
#define _MENU_H 1

#define _XOPEN_SOURCE_EXTENDED 1
#define NCURSES_WIDECHAR 1

//  ╭───────────────────────────────────────────────────────────────────╮
//  │ definitions                                                       │
//  ╰───────────────────────────────────────────────────────────────────╯
#include <ncursesw/ncurses.h>
#include <stddef.h>
#define C_MENU_VERSION "C-Menu-0.2.6"
// #define DEBUG TRUE
#define USE_PAD TRUE
// MAXLEN is for variables known to be limited in length
#define MAXLEN 256
#define MIN_COLS 40
#define BUFSIZ 8192
#define LINE_IN_MAX_COLS 2048
#define MAX_COLS 1024
#define MAX_WIDE_LEN 1024
#define COLOR_LEN 8
#define MAX_MENU_LINES 256
#define PICK_MAX_ARG_LEN 256
#define NCOLORS 16
#define MAXARGS 1024
#define MAX_ARGS 64
#define MAXFIELDS 50
#define ACCEPT_PROMPT_CHAR '_'
#define DEFAULTEDITOR "vi"
#define MENU_HELP_FILE "~/menuapp/help/menu.help"
#define PICK_HELP_FILE "~/menuapp/help/pick.help"
#define VIEW_HELP_FILE "~/menuapp/help/view.help"
#define HELP_CMD "view"
#define VIEW_PRT_FILE "~/menuapp/data/prtout"
#define DEFAULTSHELL "/bin/bash"
#define MINITRC "~/.minitrc"
#define MAPP_DIR "~/menuapp"
#define PRINTCMD "lp -c -s"
#define MAXOPTS 50
#define F_NOMETAS 1
#define F_NOTBLANK 2
#define F_NOECHO 4
#define EIGHT 8
#define MAX_COLOR_PAIRS 512
#define MAX_COLORS 512
#define F_VIEW 0x01
#define S_WCOK 0x1000
#define S_QUIET 0x2000
#define P_READ 0
#define P_WRITE 1
#define TRUE 1
#define FALSE 0
/// ╭───────────────────────────────────────────────────────────────────╮
/// │ Structures for Future Refinements                                 │
/// ╰───────────────────────────────────────────────────────────────────╯
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

/// ╭───────────────────────────────────────────────────────────────────╮
/// │ GLOBAL VARIABLES                                                  │
/// ╰───────────────────────────────────────────────────────────────────╯
extern int src_line;
extern char *src_name;
extern char fn[MAXLEN];
extern char em0[MAXLEN];
extern char em1[MAXLEN];
extern char em2[MAXLEN];
extern char em3[MAXLEN];
extern char *eargv[MAXARGS];
extern int tty_fd;
extern int dbgfd;
extern int stdin_fd;
extern int stdout_fd;
extern int stderr_fd;
extern const char *mapp_version;
extern bool f_debug;
extern bool f_stop_on_error;
extern char tmp_str[MAXLEN];
extern char *tmp_ptr;
extern unsigned int cmd_key;
extern int exit_code;

enum Caller { VIEW, FORM, PICK, MENU };

//  ╭───────────────────────────────────────────────────────────────────╮
//  │ OPTION PROCESSING                                                 │
//  ╰───────────────────────────────────────────────────────────────────╯

enum OptType {
    OT_STRING,
    OT_INT,
    OT_BOOL,
    OT_HEX,
};
enum OptGroup {
    OG_FILES,
    OG_DIRS,
    OG_SPECS,
    OG_MISC,
    OG_PARMS,
    OG_FLAGS,
    OG_COL
};

extern void dump_opts();

typedef struct {
    const char *name;
    unsigned int type;  // 0=string, 1=int, 2=bool
    unsigned int group; // 0=FILES, 1=SPECS, 2=MISC, 3=PARMS, 4=FLAGS
    const char *use;    // which programs use this option
                        // m=menu, p=pick, f=form, v=view
    const char *short_opt;
    const char *desc;
} Opts;

extern void dump_opts_by_use(char *, char *);

//  ╭───────────────────────────────────────────────────────────────────╮
//  │ OPTION PROCESSING                                                 │
//  ╰───────────────────────────────────────────────────────────────────╯

#define to_uppercase(c)                                                        \
    if (c >= 'a' && c <= 'z')                                                  \
    c -= ' '

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

extern bool capture_shell_tioctl();
extern bool restore_shell_tioctl();
extern bool capture_curses_tioctl();
extern bool restore_curses_tioctl();
extern bool mk_raw_tioctl(struct termios *);
extern bool set_sane_tioctl(struct termios *);

/// ╭───────────────────────────────────────────────────────────────────╮
/// │ Signal Processing                                                 │
/// ╰───────────────────────────────────────────────────────────────────╯
extern void signal_handler(int);
extern void sig_prog_mode();
extern void sig_dfl_mode();

/// ╭───────────────────────────────────────────────────────────────────╮
/// │ NCurses                                                           │
/// ╰───────────────────────────────────────────────────────────────────╯
#define REASSIGN_STDIN

/// ╭───────────────────────────────────────────────────────────────────╮
/// │ NCurses Key Definitions                                           │
/// ╰───────────────────────────────────────────────────────────────────╯
#undef key_left
#undef key_right
#undef key_down
#undef key_up
#define KEY_ALTF0 0x138
#define KEY_ALTF(n) (KEY_ALTF0 + (n))
#define KEY_ALTINS 0x223
#define KEY_ALTDEL 0x20e
#define KEY_ALTHOME 0x21e
#define KEY_ALTEND 0x219
#define KEY_ALTPGDN 0x22d
#define KEY_ALTPGUP 0x232
#define KEY_ALTLEFT 0x228
#define KEY_ALTRIGHT 0x237
#define KEY_ALTUP 0x23d
#define KEY_ALTDOWN 0x214
#define KEY_ALTR 0x12d
// enum {
//     KEY_CTLA = 0x001,
//     KEY_CTLB,
//     KEY_CTLC,
//     KEY_CTLD,
//     KEY_CTLE,
//     KEY_CTLF,
//     KEY_CTLG,
//     KEY_CTLH,
//     KEY_CTLI,
//     KEY_CTLJ,
//     KEY_CTLK,
//     KEY_CTLL,
//     KEY_CTLM,
//     KEY_CTLN,
//     KEY_CTLO,
//     KEY_CTLP,
//     KEY_CTLQ,
//     KEY_CTLR,
//     KEY_CTLS,
//     KEY_CTLT,
//     KEY_CTLU,
//     KEY_CTLV,
//     KEY_CTLW,
//     KEY_CTLX,
//     KEY_CTLY,
//     KEY_CTLZ,
//     KEY_ALT0 = 0x60,
//     KEY_ALTA,
//     KEY_ALTB,
//     KEY_ALTC,
//     KEY_ALTD,
//     KEY_ALTE,
//     KEY_ALTF,
//     KEY_ALTG,
//     KEY_ALTH,
//     KEY_ALTI,
//     KEY_ALTJ,
//     KEY_ALTK,
//     KEY_ALTL,
//     KEY_ALTM,
//     KEY_ALTN,
//     KEY_ALTO,
//     KEY_ALTP,
//     KEY_ALTQ,
//     KEY_ALTR,
//     KEY_ALTS,
//     KEY_ALTT,
//     KEY_ALTU,
//     KEY_ALTV,
//     KEY_ALTW,
//     KEY_ALTX,
//     KEY_ALTY,
//     KEY_ALTZ,
// };

typedef struct {
    char text[32];
    int keycode;
    int end_pos;
} key_cmd_tbl;

extern key_cmd_tbl key_cmd[20];

/// ╭───────────────────────────────────────────────────────────────────╮
/// │ COLOR PROCESSING                                                  │
/// ╰───────────────────────────────────────────────────────────────────╯

enum Color {
    black,
    red,
    green,
    yellow,
    blue,
    magenta,
    cyan,
    white,
    bblack,
    bred,
    bgreen,
    byellow,
    bblue,
    bmagenta,
    bcyan,
    bwhite,
    orange
};

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
extern void apply_gamma(RGB *);
extern void color_correction(RGB *);
extern char const colors_text[][10];

typedef struct {
    int fg;
    int bg;
    int pair_id;
} ColorPair;

extern ColorPair clr_pairs[MAX_COLOR_PAIRS];

/// ╭───────────────────────────────────────────────────────────────────╮
/// │ WIDE CHARACTER SUPPORT                                            │
/// ╰───────────────────────────────────────────────────────────────────╯

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

//  ╭───────────────────────────────────────────────────────────────────╮
//  │ Windowing                                                         │
//  ╰───────────────────────────────────────────────────────────────────╯
#define MAXWIN 20

typedef unsigned char uchar;

//  ╭───────────────────────────────────────────────────────────────────╮
//  │ SCRIOU                                                            │
//  ╰───────────────────────────────────────────────────────────────────╯
extern void close_curses();
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

//  ╭───────────────────────────────────────────────────────────────────╮
//  │ WINDOWS FUNCTIONS                                                 │
//  ╰───────────────────────────────────────────────────────────────────╯
extern WINDOW *win_open_box(int, int, int, int, char *);
extern WINDOW *winOpenwin(int, int, int, int);
extern WINDOW *win_del();
extern void win_close_win(WINDOW *);
extern void win_close_box(WINDOW *);
extern void restore_wins();
extern void cbox(WINDOW *);
extern void win_init_attrs(WINDOW *, int, int, int);
extern void win_Toggle_Attrs();
extern void mvwaddstr_fill(WINDOW *, int, int, char *, int);
extern int display_curses_keys();
extern void init_stdscr();
extern void curskeys(WINDOW *);
extern void mouse_getch(int *, int *, int *, int *);
extern void w_mouse_getch(WINDOW *, int *, int *, int *, int *);

//  ╭───────────────────────────────────────────────────────────────────╮
//  │ MENU                                                              │
//  ╰───────────────────────────────────────────────────────────────────╯

enum { C_MAIN = 283, C_MENU, C_OPTION };

enum {
    P_CONTINUE = 302,
    P_ACCEPT,
    P_HELP,
    P_CANCEL,
    P_REFUSE,
    P_CALC,
    P_EDIT,
    P_END
};

enum { MT_NULL = 0x320, MT_TEXT, MT_CHOICE };

enum {
    MA_INIT = 350,
    MA_RETURN,
    MA_RETURN_MAIN,
    MA_DISPLAY_MENU,
    MA_ENTER_OPTION
};

enum {
    CT_NULL = 0x396,
    CT_RETURNMAIN,
    CT_EXEC,
    CT_HELP,
    CT_FORM,
    CT_FORM_EXEC,
    CT_FORM_WRITE,
    CT_MENU,
    CT_PICK,
    CT_VIEW,
    CT_CKEYS,
    CT_RETURN,
    CT_TOGGLE,
    CT_WRITE_CONFIG,
    CT_UNDEFINED
};

//  ╭───────────────────────────────────────────────────────────────────╮
//  │ MENU DATA STRUCTURE                                               │
//  ╰───────────────────────────────────────────────────────────────────╯
typedef struct {
    unsigned int type;
    char *raw_text;
    char *choice_text;
    char choice_letter;
    int letter_pos;
    unsigned int command_type;
    char *command_str;
} Line;

typedef struct {
    // colors & geometry
    int fg_color; // F: foreground_color
    int bg_color; // B: background_color
    int bo_color; // O: border_color
    int lines;    // L: lines
    int cols;     // C: columns
    int begy;     // Y: lines
    int begx;     // X: lines
    // window
    WINDOW *win, *box;
    char title[MAXLEN]; // T: title
    // argument processing
    int argc;
    char **argv;
    // files
    char mapp_spec[MAXLEN];    //    application description qualified path
    char help_spec[MAXLEN];    //    application help qualified path
    char provider_cmd[MAXLEN]; // -S: command to execute at start of program
    char receiver_cmd[MAXLEN]; // -R: command to execute at start of program
    char cmd[MAXLEN];          // -c: command to execute at start of program
    // file flags
    bool f_mapp_spec;
    bool f_help_spec;
    bool help;
    bool f_provider_cmd;
    bool f_receiver_cmd;
    bool f_cmd;
    //
    int choice_max_len;
    int text_max_len;
    int item_count;
    int line_idx;
    Line *line[MAX_MENU_LINES];
    bool f_stop_on_error;
} Menu;

extern Menu *menu;

//  ╭───────────────────────────────────────────────────────────────────╮
//  │ FORM DATA STRUCTURE                                               │
//  ╰───────────────────────────────────────────────────────────────────╯
enum FieldFormat {
    FF_STRING,
    FF_DECIMAL_INT,
    FF_HEX_INT,
    FF_FLOAT,
    FF_DOUBLE,
    FF_CURRENCY,
    FF_YYYYMMDD,
    FF_HHMMSS,
    FF_APR,
    FF_INVALID
};

extern char ff_tbl[][26];

typedef struct {
    int line;
    int col;
    char str[MAX_COLS];
    int len;
} Text;

// form->field[i]->line,
// form->field[i]->col,
// form->field[i]->str,
// form->field[i]->len,
// form->field[i]->val);

typedef struct {
    int line;
    int col;
    int len;
    int ff;
    char input_s[MAXLEN];
    char accept_s[MAXLEN];
    char display_s[MAXLEN];
    char filler_s[MAXLEN];
} Field;

typedef struct {
    // colors & geometry
    int fg_color; // F: foreground_color
    int bg_color; // B: background_color
    int bo_color; // O: border_color
    int lines;    // L: lines
    int cols;     // C: columns
    int begy;     // Y: lines
    int begx;     // X: lines
    // window
    WINDOW *win, *box;
    char title[MAXLEN]; // T: title
    char chyron_s[MAXLEN];
    int title_line;
    // argument processing
    // files
    FILE *in_fp;
    FILE *out_fp;
    int in_fd;
    int out_fd;
    char mapp_spec[MAXLEN];    //    description spec
    char in_spec[MAXLEN];      //    input spec
    char out_spec[MAXLEN];     //    output spec
    char help_spec[MAXLEN];    //    help spec
    char provider_cmd[MAXLEN]; // -S: command to execute at start of program
    char receiver_cmd[MAXLEN]; // -R: command to execute at start of program
    char cmd[MAXLEN];          // -c: command to execute at start of program
    // file flags
    bool f_mapp_spec;
    bool f_in_spec;
    bool f_out_spec;
    bool f_in_pipe;
    bool f_out_pipe;
    bool f_help_spec;
    bool f_erase_remainder;
    bool f_calculate;
    bool f_query;
    bool f_stop_on_error;
    bool help;
    bool f_provider_cmd;
    bool f_receiver_cmd;
    bool f_cmd;
    char brackets[3];
    char fill_char[2];
    int fidx;
    int fcnt;
    int didx;
    int dcnt;
    Text *text[MAXFIELDS];
    Field *field[MAXFIELDS];
} Form;

extern Form *form;

extern int form_accept_field(Form *);
extern int form_display_field(Form *);
extern int form_display_field_n(Form *, int);
extern int form_open_win(Form *);
extern int form_enter_fields(Form *);
extern int form_read_description(Form *);
extern int form_fmt_field(Form *, char *s);
extern void form_help(char *);

//  ╭───────────────────────────────────────────────────────────────────╮
//  │ PICK DATA STRUCTURE                                               │
//  ╰───────────────────────────────────────────────────────────────────╯
#define OBJ_MAXLEN 80
#define OBJ_MAXCNT 1024

typedef struct {
    // colors & geometry
    int fg_color;  // F: foreground_color
    int bg_color;  // B: background_color
    int bo_color;  // O: border_color
    int win_lines; // L: lines
    int win_width; // C: columns
    int begy;      // Y: placement line
    int begx;      // X: placement column
    int y;         // line
    int x;         // column
    // window
    WINDOW *win, *box;
    char title[MAXLEN]; // T: title
    // argument processing
    int argc;
    char **argv;
    FILE *in_fp;
    FILE *out_fp;
    int in_fd;
    int out_fd;
    // files
    char mapp_spec[MAXLEN]; //    application qualified path
    char in_spec[MAXLEN];
    char out_spec[MAXLEN];
    char help_spec[MAXLEN]; //    application help qualified path
    char chyron_s[MAXLEN];  // (ˈkī-ˌrän) a banner at the bottom of the screen
    //
    char provider_cmd[MAXLEN]; // -S: provider command at start
    char receiver_cmd[MAXLEN]; // -R: receiver command at end
    char cmd[MAXLEN];
    // file flags
    bool f_mapp_spec;
    bool f_in_spec;
    bool f_out_spec;
    bool f_in_pipe;
    bool f_out_pipe;
    bool f_help_spec;
    bool f_multiple_cmd_args;
    bool f_stop_on_error;
    bool f_selected[OBJ_MAXCNT];
    bool help;
    bool f_provider_cmd;
    bool f_receiver_cmd;
    bool f_cmd;
    char in_buf[BUFSIZ];
    char **object;
    int select_idx;
    int select_cnt;
    int select_max;
    int obj_cnt;
    int obj_idx;
    int pg_line;
    int pg_lines;
    int pg_objs;
    int tab_idx;
    int tbl_pages;
    int tbl_page;
    int tbl_line;
    int tbl_lines;
    int tbl_cols;
    int tbl_col;
    int tbl_col_width;
} Pick;

extern Pick *pick;

//  ╭───────────────────────────────────────────────────────────────────╮
//  │ VIEW DATA STRUCTURE                                               │
//  ╰───────────────────────────────────────────────────────────────────╯
#define NPOS 256
#define NMARKS 256
#define MAXLEN 256
#define NULSL
#define NULL_POSITION -1
#define VBUFSIZ 65536

enum PROMPT_TYPE { PT_NONE, PT_SHORT, PT_LONG, PT_STRING };

typedef struct {
    // colors & geometry
    int fg_color; // F: foreground_color
    int bg_color; // B: background_color
    int bo_color; // O: border_color
    // window
    WINDOW *win, *box;
    char prompt_str[MAXLEN];
    char tmp_prompt_str[MAXLEN];
    int prompt_type; // PT_NONE, PT_SHORT, PT_LONG, PT_STRING
    char title[MAXLEN];
    // files
    char provider_cmd[MAXLEN]; // -S: provider command at start
    char receiver_cmd[MAXLEN]; // -R: receiver command at end
    char cmd_arg[MAXLEN];
    // argument processing
    int argc;
    char **argv;
    int curr_argc;
    char arg_str[MAXLEN];
    // init
    int tab_stop;         // -t: number of spaces per tab
    bool f_ignore_case;   // -x: ignore case in search
    bool f_at_end_clear;  // -z  clear screen at end of program
    bool f_at_end_remove; // -r: remove file at end of program
    bool f_squeeze;       // -s  squeeze multiple blank lines
    bool f_stop_on_error; // Z  stop on error
    //
    int next_cmd_char;
    int line_mode;
    //--------------------------------
    bool f_bod;
    bool f_eod;
    bool f_eob;
    bool f_eof;
    bool f_forward;
    bool f_is_pipe;
    bool f_new_file;
    bool f_pipe_processed;
    bool f_redisplay_page;
    bool f_displaying_help;
    bool f_stdout_is_tty;
    bool f_line_numbers;
    bool f_wrap;
    bool f_full_screen;
    bool help;
    bool f_timer;
    bool f_provider_cmd;
    bool f_receiver_cmd;
    bool f_in_spec;
    bool f_out_spec;
    bool f_cmd;
    bool f_cmd_all;
    char cmd[MAXLEN];
    char cmd_all[MAXLEN];
    char cur_file_str[MAXLEN];
    char line_in_s[LINE_IN_MAX_COLS];
    char line_out_s[LINE_IN_MAX_COLS];
    char stripped_line_out[MAX_COLS];
    cchar_t cmplx_buf[MAX_COLS];
    char *line_out_p;
    wchar_t line_w[MAX_COLS];
    cchar_t *line_p;
    unsigned int line_number;
    char line_number_s[20];
    char *line_in_beg_p;
    char *line_in_end_p;
    off_t srch_beg_pos;
    //
    WINDOW *pad;
    // *newpad(int plines, int pcols);
    // *subpad(WINDOW *orig, int nlines, int ncols, int begy, int begx);
    // prefresh(
    // pnoutrefresh(
    // WINDOW *, int p_minrow, int p_mincol,
    //                    int ps_minrow, int ps_mincol,
    //                    int ps_maxrow, int ps_maxcol);
    // pechochar(WINDOW *pad, char ch);
    // pecho_wchar(WINDOW *pad, wchar_t wch);
    //
    int lines; // L: lines
    int cols;  // C: columns
    int begy;  // Y: lines
    int begx;  // X: lines
    //
    int cury;
    int curx;
    int scroll_lines;
    int cmd_line;
    int first_column;
    int last_column;
    //
    int maxcol;
    //
    // pad coordinates
    int pminrow;
    int pmincol;
    // screen coordinates
    int sminrow;
    int smincol;
    int smaxrow;
    int smaxcol;
    //
    int first_match_x;
    int last_match_x;
    //
    char in_spec[MAXLEN];
    char out_spec[MAXLEN];
    char *file_spec_ptr;
    char *next_file_spec_ptr;
    char *tmp_file_name_ptr;
    //
    ssize_t file_size;
    off_t file_pos;
    off_t prev_file_pos;
    off_t page_top_pos;
    off_t page_bot_pos;
    // off_t pos_tbl[NPOS];
    off_t mark_tbl[NMARKS];
    //
    bool f_in_pipe;
    int in_fd;
    int out_fd;
    FILE *in_fp;
    int stdin_fd;
    FILE *stdin_fp;
    int stdout_fd;
    FILE *stdout_fp;
    long buf_idx;
    int buf_last;
    char *buf;
    char *buf_curr_ptr;
    char *buf_end_ptr;
} View;
extern View *view;

//  ╭───────────────────────────────────────────────────────────────────╮
//  │ INIT DATA STRUCTURE                                               │
//  ╰───────────────────────────────────────────────────────────────────╯
typedef struct {
    // colors & geometry
    int fg_color; // -F: foreground_color
    int bg_color; // -B: background_color
    int bo_color; // -O: border_color
    double red_gamma;
    double green_gamma;
    double blue_gamma;
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
    // Future Implementation of Enhanced Color Options
    // char bg[COLOR_LEN];
    // char hfg[COLOR_LEN];
    // char title_fg[COLOR_LEN];
    // char title_bg[COLOR_LEN];
    // char title_bold[COLOR_LEN];
    // char prompt_fg[COLOR_LEN];
    // char prompt_bg[COLOR_LEN];
    // char prompt_bold[COLOR_LEN];
    // char select_fg[COLOR_LEN];
    // char select_bg[COLOR_LEN];
    // char select_bold[COLOR_LEN];
    // char normal_fg[COLOR_LEN];
    // char normal_bg[COLOR_LEN];
    // char normal_bold[COLOR_LEN];
    // char help_fg[COLOR_LEN];
    // char help_bg[COLOR_LEN];
    // char help_bold[COLOR_LEN];
    // char border_fg[COLOR_LEN];
    // char border_bg[COLOR_LEN];
    // char border_bold[COLOR_LEN];
    // char cmd_fg[COLOR_LEN];
    // char cmd_bg[COLOR_LEN];
    // char cmd_bold[COLOR_LEN];
    // char mapp_fg[COLOR_LEN];
    // char mapp_bg[COLOR_LEN];
    int lines; // -L: lines
    int cols;  // -C: columns
    int begx;  // -X: lines
    int begy;  // -Y: lines
    int stdin_fd;
    FILE *stdin_fp;
    int stdout_fd;
    FILE *stdout_fp;
    int stderr_fd;
    FILE *stderr_fp;
    int tty_fd;
    FILE *tty_fp;
    // window
    WINDOW *active_window;
    char cmd[MAXLEN];          // -V: command to execute at start of program
    char cmd_all[MAXLEN];      // -V: command to execute at start of program
    char provider_cmd[MAXLEN]; // -S: receiver
    char receiver_cmd[MAXLEN]; // -R: receiver
    char prompt_str[MAXLEN];
    int prompt_type;    // PT_LONG, PT_SHORT, PT_NONE, PT_STRING
    char title[MAXLEN]; // -T: title
    // argument processing
    int argc;
    char **argv;
    int optind;
    // flags
    bool f_ignore_case;       // -x: ignore case in search
    bool f_at_end_clear;      // -z  clear screen at end of program
    bool f_at_end_remove;     // -r: remove file at end of program
    bool f_squeeze;           // -s  squeeze multiple blank lines
    bool f_stop_on_error;     // -Z  stop on error
    bool f_multiple_cmd_args; // -M  multiple command arguments
    bool f_erase_remainder;   // -e: erase remainder of line on enter
    char brackets[3];         // -f: field_brackets
    bool help;
    // directories
    char mapp_home[MAXLEN]; // -m: home directory
    char mapp_data[MAXLEN]; //     --mapp_data
    char mapp_help[MAXLEN]; //     --mapp_help
    char mapp_msrc[MAXLEN]; //     --mapp_msrc
    char mapp_user[MAXLEN]; // -u: user directory
    bool cd_mapp_home;
    // directory flags
    bool f_mapp_home; // -m: home directory
    bool f_mapp_data; //
    bool f_mapp_help; //
    bool f_mapp_msrc; //
    bool f_mapp_user; // -u: user directory
    // file flags
    bool f_mapp_desc;
    bool f_provider_cmd;
    bool f_receiver_cmd;
    bool f_cmd;
    bool f_cmd_all;
    bool f_title;
    bool f_help_spec;
    bool f_in_spec;
    bool f_out_spec;
    char fill_char[2];
    // files
    char minitrc[MAXLEN];   // -a: main configuration file
    char mapp_spec[MAXLEN]; // -d: description qualified path
    char help_spec[MAXLEN]; // -H: help qualified path
    char in_spec[MAXLEN];   // -i: input file qualified path
    char out_spec[MAXLEN];  // -o: output file qualified path
    // Pick
    int select_max; // -n: maximum number of selections
    // View
    int tab_stop; // -t: number of spaces per tab
    // Structures
    Menu *menu;
    int menu_cnt;
    Form *form;
    int form_cnt;
    Pick *pick;
    int pick_cnt;
    View *view;
    int view_cnt;
} Init;

enum { IC_MENU, IC_PICK, IC_FORM, IC_VIEW };

extern Init *init;
extern int init_cnt;

extern char minitrc[MAXLEN];

extern void mapp_initialization(Init *init, int, char **);
extern Init *new_init(int, char **);
extern View *new_view(Init *init, int, char **, int, int);
extern Form *new_form(Init *init, int, char **, int, int);
extern Pick *new_pick(Init *init, int, char **, int, int);
extern Menu *new_menu(Init *init, int, char **, int, int);
extern Menu *close_menu(Init *init);
extern Pick *close_pick(Init *init);
extern Form *close_form(Init *init);
extern View *close_view(Init *init);
extern Init *close_init(Init *init);
extern int parse_opt_args(Init *, int, char **);
extern void zero_opt_args(Init *);
extern int write_config(Init *);
extern bool derive_file_spec(char *, char *, char *);
extern void open_curses(Init *init);
extern int win_new(int, int, int, int, char *, int);
extern void win_redraw(WINDOW *);
extern void win_resize(int, int, char *);
extern int rgb_to_xterm256_idx(RGB);
extern RGB xterm256_idx_to_rgb(int);
extern void init_clr_palette(Init *);
extern int get_clr_pair(int, int);
extern int view_file(Init *);

//  ╭───────────────────────────────────────────────────────────────────╮
//  │ PICK                                                              │
//  ╰───────────────────────────────────────────────────────────────────╯
extern int init_pick(Init *, int, char **, int, int);
extern int open_pick_win(Init *);
extern int pick_engine(Init *);
extern bool pick_help_spec(Init *, int argc, char **argv);
extern bool pick_in_spec(Init *, int argc, char **argv);
extern bool pick_out_spec(Init *, int argc, char **argv);
extern void save_object(Pick *, char *);
extern void display_page(Pick *);
extern void reverse_object(Pick *);
extern void toggle_object(Pick *);
extern int output_objects(Pick *);
extern int mpick(int, char **, int, int, int, int, char *, int);

//  ╭───────────────────────────────────────────────────────────────────╮
//  │ MENU                                                              │
//  ╰───────────────────────────────────────────────────────────────────╯
extern bool init_menu_files(Init *, int, char **);
extern unsigned int menu_engine(Init *);
extern unsigned int menu_loop(Init *);
extern unsigned int parse_menu_description(Init *);
extern unsigned int get_command_type(char *);
extern void free_menu_line(Line *);

//  ╭───────────────────────────────────────────────────────────────────╮
//  │ FORM                                                              │
//  ╰───────────────────────────────────────────────────────────────────╯
extern int init_form(Init *, int, char **, int, int);
//  ╭───────────────────────────────────────────────────────────────────╮
//  │ VIEW                                                              │
//  ╰───────────────────────────────────────────────────────────────────╯
extern int mview(Init *, int, char **, int, int, int, int, char *);
extern int init_view_full_screen(Init *);
extern int init_view_boxwin(Init *, char *);
extern bool view_init_input(View *, char *);
extern int cmd_processor(Init *);
extern int get_cmd_spec(View *, char *);
extern void go_to_position(View *, long);
extern void cat_file(View *);
extern char err_msg[MAXLEN];

//  ╭───────────────────────────────────────────────────────────────────╮
//  │ STRING UTILITIES                                                  │
//  ╰───────────────────────────────────────────────────────────────────╯
extern int trim(char *);
extern int rtrim(char *);
extern bool stripz_quotes(char *);
extern void strip_quotes(char *);
extern int ssnprintf(char *, size_t, const char *, ...);
extern bool str_to_bool(const char *);
extern int str_to_args(char **, char *, int);
extern void str_to_lower(char *);
extern void str_to_upper(char *);
extern void str(char *);
extern int strnz(char *, int);
extern int strnz__cpy(char *, const char *, int);
extern int strnz__cat(char *, const char *, int);
extern char *strz_dup(char *);
extern char *strnz_dup(char *, int);
extern char *rep_substring(const char *, const char *, const char *);
extern void strnfill(char *, char, int);
extern void str_subc(char *, char *, char, char *, int);
extern void chrep(char *, char, char);
extern void string_cpy(String *, const String *);
extern void string_cat(String *, const String *);
extern String to_string(const char *);
extern String mk_string(size_t);
extern String free_string(String);
extern char *str_tok(char *, const char *, char);
extern double str_to_double(char *);

extern int get_color_number(char *);
extern int rgb_clr_to_cube(int);
extern void list_colors();

//  ╭───────────────────────────────────────────────────────────────────╮
//  │ EXEC UTILITIES                                                    │
//  ╰───────────────────────────────────────────────────────────────────╯
extern int fork_exec(char **);
extern int bg_fork_exec_pipe(char **, int *, pid_t);
extern int full_screen_fork_exec(char **);
extern int full_screen_shell(char *);
extern int shell(char *);

//  ╭───────────────────────────────────────────────────────────────────╮
//  │ MISCELANEOUS UTILITIES                                            │
//  ╰───────────────────────────────────────────────────────────────────╯
extern char errmsg[];
extern void get_rfc3339_s(char *, size_t);
extern int open_log(char *);
extern void write_log(char *);
extern void set_fkey(int, char *);
extern bool is_set_fkey(int);
extern void unset_fkey(int);
extern int chyron_mk(key_cmd_tbl *, char *);
extern int get_chyron_key(key_cmd_tbl *, int);
//  ╭───────────────────────────────────────────────────────────────────╮
//  │ ERROR HANDLING                                                    │
//  ╰───────────────────────────────────────────────────────────────────╯
extern void abend(int, char *);
extern void user_end();
extern int display_error(char *, char *, char *, char *);
extern void display_error_msg(View *, char *);
extern int display_ok_message(char *);
extern void display_argv_error_msg(char *, char **);
extern int Perror(char *);
extern int error_message(char **);
extern int form_desc_error(int, char *, char *);
//  ╭───────────────────────────────────────────────────────────────────╮
//  │ FILE UTILITIES                                                    │
//  ╰───────────────────────────────────────────────────────────────────╯
extern void normalize_file_spec(char *);
extern void file_spec_path(char *, char *);
extern void file_spec_name(char *, char *);
extern bool dir_name(char *, char *);
extern bool base_name(char *, char *);
extern bool trim_ext(char *, char *);
extern bool trim_path(char *);
extern bool expand_tilde(char *, int);
extern bool verify_file(char *, int);
extern bool verify_file_q(char *, int);
extern bool verify_dir(char *, int);
extern bool verify_dir_q(char *, int);
extern bool verify_spec_arg(char *, char *, char *, char *, int);
extern bool construct_file_spec(char *, char *, char *, char *, char *, int);
extern bool locate_file_in_path(char *, char *);
extern int canonicalize_file_spec(char *);

#endif
