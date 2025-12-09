// menu
// Bill Waller
// billxwaller@gmail.com

#ifndef _MENU_H
#define _MENU_H 1

#define _XOPEN_SOURCE_EXTENDED 1
#define NCURSES_WIDECHAR 1

/*  ╭───────────────────────────────────────────────────────────────────╮
    │ definitions                                                       │
    ╰───────────────────────────────────────────────────────────────────╯*/
#include <ncursesw/ncurses.h>
#include <stddef.h>
#define DEBUG TRUE
#define USE_PAD TRUE
// MAXLEN is for variables known to be limited in length
#define MAXLEN 256
#define MIN_COLS 40
#define MAX_COLS 1024
#define MAX_WIDE_LEN 1024
#define COLOR_LEN 8
#define MAX_MENU_LINES 256
#define PICK_MAX_ARG_LEN 256
#define NCOLORS 16
#define MAXARGS 1024
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

/*  ╭───────────────────────────────────────────────────────────────────╮
    │ Miscelaneous                                                      │
    ╰───────────────────────────────────────────────────────────────────╯*/

extern int dbgfd;

extern char *eargv[MAXARGS];

enum Caller { VIEW, FORM, PICK, MENU };
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
    int type;        // 0=string, 1=int, 2=bool
    int group;       // 0=FILES, 1=SPECS, 2=MISC, 3=PARMS, 4=FLAGS
    const char *use; // which programs use this option
                     // m=menu, p=pick, f=form, v=view
    const char *desc;
} Opts;

extern void dump_opts_by_use(char *, char *);

#define TRUE 1
#define FALSE 0

extern const char *mapp_version;

#define to_uppercase(c)                                                        \
    if (c >= 'a' && c <= 'z')                                                  \
    c -= ' '

extern bool f_debug;
extern bool f_stop_on_error;

extern char const colors_text[][10];

/*  ╭───────────────────────────────────────────────────────────────────╮
    │ SCREEN I/O                                                        │
    ╰───────────────────────────────────────────────────────────────────╯*/

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

/*  ╭───────────────────────────────────────────────────────────────────╮
    │ Signal Processing                                                 │
    ╰───────────────────────────────────────────────────────────────────╯*/
extern void signal_handler(int);
extern void sig_prog_mode();
extern void sig_dfl_mode();

/*  ╭───────────────────────────────────────────────────────────────────╮
    │ NCurses                                                           │
    ╰───────────────────────────────────────────────────────────────────╯*/
#define REASSIGN_STDIN

#undef key_left
#undef key_right
#undef key_down
#undef key_up

#define KEY_ESC 0x01b
#define KEY_TAB 0x09

enum {
    KEY_NULL = 0x0000,
    KEY_CTLA,
    KEY_CTLB,
    KEY_CTLC,
    KEY_CTLD,
    KEY_CTLE,
    KEY_CTLF,
    KEY_CTLG,
    KEY_CTLH,
    KEY_CTLI,
    KEY_CTLJ,
    KEY_CTLK,
    KEY_CTLL,
    KEY_CTLM,
    KEY_CTLN,
    KEY_CTLO,
    KEY_CTLP,
    KEY_CTLQ,
    KEY_CTLR,
    KEY_CTLS,
    KEY_CTLT,
    KEY_CTLU,
    KEY_CTLV,
    KEY_CTLW,
    KEY_CTLX,
    KEY_CTLY,
    KEY_CTLZ,
    KEY_F1 = 0x109,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F5,
    KEY_F6,
    KEY_F7,
    KEY_F8,
    KEY_F9,
    KEY_F10,
    KEY_F11,
    KEY_F12,
    KEY_F13,
    KEY_F14,
    KEY_ALTA = 0x170,
    KEY_ALTB,
    KEY_ALTC,
    KEY_ALTD,
    KEY_ALTE,
    KEY_ALTF,
    KEY_ALTG,
    KEY_ALTH,
    KEY_ALTI,
    KEY_ALTJ,
    KEY_ALTK,
    KEY_ALTL,
    KEY_ALTM,
    KEY_ALTN,
    KEY_ALTO,
    KEY_ALTP,
    KEY_ALTQ,
    KEY_ALTR,
    KEY_ALTS,
    KEY_ALTT,
    KEY_ALTU,
    KEY_ALTV,
    KEY_ALTW,
    KEY_ALTX,
    KEY_ALTY,
    KEY_ALT,
    KEY_ALT0,
    KEY_ALT1,
    KEY_ALT2,
    KEY_ALT3,
    KEY_ALT4,
    KEY_ALT5,
    KEY_ALT6,
    KEY_ALT7,
    KEY_ALT8,
    KEY_ALT9,
    KEY_ALTDEL = 0x213,
    KEY_ALTDOWN = 0x219,
    KEY_ALTEND = 0x21e,
    KEY_ALTHOME = 0x223,
    KEY_ALTINS = 0x228,
    KEY_ALTLEFT = 0x22d,
    KEY_ALTPGDN = 0x232,
    KEY_ALTPGUP = 0x237,
    KEY_ALTRIGHT = 0x23c,
    KEY_ALTUP = 0x242
};

#define key_left 8   // ^H
#define key_down 10  // ^J
#define key_up 11    // ^K
#define key_right 12 // ^L
#define key_cr 13    // ^M

typedef struct {
    char text[32];
    int keycode;
    int end_pos;
} key_cmd_tbl;

extern key_cmd_tbl key_cmd[20];

#define FG_COLOR 2 // green
#define BG_COLOR 0 // black
#define BO_COLOR 1 // red

enum color_pairs_enum {
    CP_DEFAULT,
    CP_NORM,
    CP_REVERSE,
    CP_BOX,
    CP_BOLD,
    CP_TITLE,
    CP_HIGHLIGHT,
    CP_NPAIRS
};

// box Wide Unicode
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

/*  ╭───────────────────────────────────────────────────────────────────╮
    │ Windowing                                                         │
    ╰───────────────────────────────────────────────────────────────────╯*/
#define MAXWIN 20

typedef unsigned char uchar;

/*  ╭───────────────────────────────────────────────────────────────────╮
    │ SCRIOU                                                            │
    ╰───────────────────────────────────────────────────────────────────╯*/
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
extern int const ncolors[];

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

extern char tmp_str[MAXLEN];
extern char *tmp_ptr;
extern unsigned int cmd_key;

/*  ╭───────────────────────────────────────────────────────────────────╮
    │ WINDOWS FUNCTIONS                                                 │
    ╰───────────────────────────────────────────────────────────────────╯*/
extern int win_new(int, int, int, int, char *);
extern void win_redraw(WINDOW *, int, char *);
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

/*  ╭───────────────────────────────────────────────────────────────────╮
    │ MENU                                                              │
    ╰───────────────────────────────────────────────────────────────────╯*/

enum { MT_NULL, MT_CENTERED_TEXT, MT_LEFT_JUST_TEXT, MT_CHOICE };

enum { MA_INIT, MA_RETURN, MA_RETURN_MAIN, MA_DISPLAY_MENU, MA_ENTER_OPTION };

enum {
    CT_NULL,
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

enum { C_MAIN, C_MENU, C_OPTION };

enum { P_CONTINUE, P_ACCEPT, P_HELP, P_CANCEL, P_REFUSE, P_CALC, P_END };

extern int exit_code;

typedef struct {
    char type;
    char *raw_text;
    char *choice_text;
    char choice_letter;
    int letter_pos;
    char command_type;
    char *command_str;
    char *option_ptr[MAXOPTS];
    int option_col;
    int option_idx;
    int option_cnt;
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
    char mapp_spec[MAXLEN]; //    application description qualified path
    char help_spec[MAXLEN]; //    application help qualified path
    // file flags
    bool f_mapp_spec;
    bool f_help_spec;
    bool f_help;
    //
    int choice_max_len;
    int text_max_len;
    int option_offset;
    int option_max_len;
    int item_count;
    int line_idx;
    Line *line[MAX_MENU_LINES];
    bool f_stop_on_error;
} Menu;

extern Menu *menu;

/*  ╭───────────────────────────────────────────────────────────────────╮
    │ FORM                                                              │
    ╰───────────────────────────────────────────────────────────────────╯*/
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
    char blank_s[MAXLEN];
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
    char mapp_spec[MAXLEN];   //    description spec
    char answer_spec[MAXLEN]; //    answer spec
    char in_spec[MAXLEN];     //    output spec
    char out_spec[MAXLEN];    //    output spec
    char cmd_spec[MAXLEN];    // c: command executable
    char help_spec[MAXLEN];   //    help spec
    // file flags
    bool f_mapp_spec;
    bool f_answer_spec;
    bool f_in_spec;
    bool f_out_spec;
    bool f_cmd_spec;
    bool f_help_spec;
    bool f_erase_remainder;
    bool f_calculate;
    bool f_query;
    bool f_stop_on_error;
    bool f_help;
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
extern int form_display_screen(Form *);
extern int form_enter_fields(Form *);
extern int form_read_description(Form *);
extern int form_read_answer_file(Form *);
extern int form_write_answer(Form *);
extern int form_fmt_field(Form *, char *s);
extern void form_help(char *);

/*  ╭───────────────────────────────────────────────────────────────────╮
    │ PICK                                                              │
    ╰───────────────────────────────────────────────────────────────────╯*/
#define OBJ_MAXLEN 80
#define OBJ_MAXCNT 256

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
    // files
    char mapp_spec[MAXLEN]; //    application qualified path
    char in_spec[MAXLEN];
    char out_spec[MAXLEN];
    char cmd_spec[MAXLEN];  // c: command
    char help_spec[MAXLEN]; //    application help qualified path
    char chyron_s[MAXLEN];  // (ˈkī-ˌrän) a banner at the bottom of the screen
    // file flags
    bool f_mapp_spec;
    bool f_in_spec;
    bool f_out_spec;
    bool f_cmd_spec;
    bool f_help_spec;
    bool f_multiple_cmd_args;
    bool f_stop_on_error;
    bool f_selected[OBJ_MAXCNT];
    bool f_help;
    char in_buf[BUFSIZ];
    char *object[OBJ_MAXCNT];
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

/*  ╭───────────────────────────────────────────────────────────────────╮
    │ VIEW                                                              │
    ╰───────────────────────────────────────────────────────────────────╯*/
#define NPOS 256
#define NMARKS 256
#define MAXLEN 256
#define NULSL
#define NULL_POSITION -1
#define VBUFSIZ 8192

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
    // files
    char cmd_spec[MAXLEN];  // c: command executable
    char start_cmd[MAXLEN]; // S  command to execute at start of program
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
    int next_c;
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
    bool f_redraw_screen;
    bool f_displaying_help;
    bool f_stdout_is_tty;
    bool f_line_numbers;
    bool f_wrap;
    bool f_full_screen;
    bool f_help;
    //
    char start_cmd_all_files[MAXLEN];
    char cur_file_str[MAXLEN];
    char line_in_s[MAX_COLS];
    char line_out_s[MAX_COLS];
    cchar_t cmplx_buf[MAX_COLS];
    char *line_out_p;
    wchar_t line_w[MAX_COLS];
    cchar_t *line_p;
    unsigned int line_number;
    char line_number_s[20];
    char *line_in_beg_p;
    char *line_in_end_p;
    long srch_beg_pos;
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
    char *file_spec_ptr;
    char *next_file_spec_ptr;
    char *tmp_file_name_ptr;
    //
    FILE *fp;
    long file_size;
    long file_pos;
    long prev_file_pos;
    long page_top_pos;
    long page_bot_pos;
    // long pos_tbl[NPOS];
    long mark_tbl[NMARKS];
    //
    long buf_idx;
    int buf_last;
    char *buf;
    char *buf_curr_ptr;
    char *buf_end_ptr;
} View;
extern View *view;
extern int view_file(View *);

/*  ╭───────────────────────────────────────────────────────────────────╮
    │ INIT                                                              │
    ╰───────────────────────────────────────────────────────────────────╯*/
typedef struct {
    // colors & geometry
    int fg_color; // -F: foreground_color
    int bg_color; // -B: background_color
    int bo_color; // -O: border_color
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
    // window
    char start_cmd[MAXLEN]; // -S: command to execute at start of program
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
    bool f_help;
    // directories
    char mapp_home[MAXLEN]; // -m: home directory
    char mapp_data[MAXLEN]; //     --mapp_data
    char mapp_help[MAXLEN]; //     --mapp_help
    char mapp_msrc[MAXLEN]; //     --mapp_msrc
    char mapp_user[MAXLEN]; // -u: user directory
    // directory flags
    bool f_mapp_home; // -m: home directory
    bool f_mapp_data; //
    bool f_mapp_help; //
    bool f_mapp_msrc; //
    bool f_mapp_user; // -u: user directory
    // files
    char minitrc[MAXLEN];     // -a: main configuration file
    char mapp_spec[MAXLEN];   // -d: description qualified path
    char help_spec[MAXLEN];   // -H: help qualified path
    char answer_spec[MAXLEN]; // -A: answer qualified path
    char cmd_spec[MAXLEN];    // -c: answer qualified path
    char in_spec[MAXLEN];     // -i: input file qualified path
    char out_spec[MAXLEN];    // -o: output file qualified path
    // pick
    int select_max; // -n: maximum number of selections
    // view
    int tab_stop; // -t: number of spaces per tab
    // structures
    // ----------------------------------------------------
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
extern bool init_menu_files(Init *, int, char **);
extern bool init_pick_files(Init *, int, char **);
extern bool init_form_files(Init *, int, char **);
extern bool init_view_files(Init *, int, char **);
extern Menu *close_menu(Init *init);
extern Pick *close_pick(Init *init);
extern Form *close_form(Init *init);
extern View *close_view(Init *init);
extern Init *close_init(Init *init);
extern int parse_opt_args(Init *, int, char **);
extern int write_config(Init *);
extern bool derive_file_spec(char *, char *, char *);
extern void open_curses(Init *init);
extern bool ansi_to_cmplx(cchar_t *, const char *);
extern void parse_ansi_str(WINDOW *, char *, attr_t *, short *);

/*  ╭───────────────────────────────────────────────────────────────────╮
    │ PICK                                                              │
    ╰───────────────────────────────────────────────────────────────────╯*/
extern int init_pick(Init *, int, char **, int, int);
extern int open_pick_win(Pick *);
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

/*  ╭───────────────────────────────────────────────────────────────────╮
    │ MENU                                                              │
    ╰───────────────────────────────────────────────────────────────────╯*/
extern int menu_engine(Init *);
extern int menu_loop(Init *);
extern int parse_menu_description(Init *);
extern char get_command_type(char *);
extern void free_menu_line(Line *);

/*  ╭───────────────────────────────────────────────────────────────────╮
    │ FORM                                                              │
    ╰───────────────────────────────────────────────────────────────────╯*/
extern int form_engine(Init *);
extern bool form_answer_spec(Init *, int argc, char **argv);
extern bool form_help_spec(Init *, int argc, char **argv);

/*  ╭───────────────────────────────────────────────────────────────────╮
    │ VIEW                                                              │
    ╰───────────────────────────────────────────────────────────────────╯*/
extern int mview(Init *, int, char **, int, int, int, int);
extern int init_view_full_screen(Init *);
extern int init_view_boxwin(View *);
extern bool view_init_input(View *, char *);
extern int view_cmd_processor(View *);
extern int get_cmd_char(View *);
extern int get_cmd_spec(View *, char *);
extern void go_to_position(View *, long);
extern void cat_file(View *);
extern char err_msg[MAXLEN];

/*  ╭───────────────────────────────────────────────────────────────────╮
    │ STRING UTILITIES                                                  │
    ╰───────────────────────────────────────────────────────────────────╯*/
extern int trim(char *);
extern int ssnprintf(char *, size_t, const char *, ...);
extern bool str_to_bool(const char *);
extern int str_to_args(char **, char *);
extern void str_to_lower(char *);
extern void str_to_upper(char *);
extern void str(char *);
extern int strnz(char *, int);
extern int strnz__cpy(char *, char *, int);
extern int strnz__cat(char *, char *, int);
extern char *strz_dup(char *);
extern char *strnz_dup(char *, int);
extern void str_subc(char *, char *, char, char *, int);
extern void chrep(char *, char, char);
extern int get_color_number(char *);
extern void list_colors();

/*  ╭───────────────────────────────────────────────────────────────────╮
    │ EXEC UTILITIES                                                    │
    ╰───────────────────────────────────────────────────────────────────╯*/
extern int fork_exec(char **);
extern int full_screen_fork_exec(char **);
extern int full_screen_shell(char *);
extern int shell(char *);

/*  ╭───────────────────────────────────────────────────────────────────╮
    │ MISCELANEOUS UTILITIES                                            │
    ╰───────────────────────────────────────────────────────────────────╯*/
extern char errmsg[];
extern void get_rfc3339_s(char *, size_t);
extern int open_log(char *);
extern void write_log(char *);
/*  ╭───────────────────────────────────────────────────────────────────╮
    │ UTILITIES                                                         │
    ╰───────────────────────────────────────────────────────────────────╯*/
extern void set_fkey(int, char *);
extern int chyron_mk(key_cmd_tbl *, char *);
extern int get_chyron_key(key_cmd_tbl *, int);
/*  ╭───────────────────────────────────────────────────────────────────╮
    │ ERROR HANDLING                                                    │
    ╰───────────────────────────────────────────────────────────────────╯*/
extern void abend(int, char *);
extern void user_end();
extern int display_error(char *, char *, char *);
extern void display_error_msg(View *, char *);
extern int display_ok_message(char *);
extern void display_argv_error_msg(char *, char **);
extern int display_error_message(char *);
extern int error_message(char **);
extern int form_desc_error(int, char *, char *);
/*  ╭───────────────────────────────────────────────────────────────────╮
    │ FILE UTILITIES                                                    │
    ╰───────────────────────────────────────────────────────────────────╯*/
#define WC_OK (W_OK | 0x1000)
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

/*  ╭───────────────────────────────────────────────────────────────────╮
    │ DEPRECATED                                                        │
    ╰───────────────────────────────────────────────────────────────────╯*/
#endif
