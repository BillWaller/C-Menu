// menu.h
// Bill Waller
// billxwaller@gmail.com

#ifndef _MENU_H
#define _MENU_H 1

#define MAXLEN 256
#define MIN_COLS 40
#define MAX_COLS 256
#define DEF_COLS 40
#define MAX_LINES 128
#define DEF_LINES 10
#define NCOLORS 16
#define MAXARGS 50
#define MAXFIELDS 50
#define ACCEPT_PROMPT_CHAR '_'
#define DEFAULTEDITOR "vi"
#define EDITOR "vi"
#define VIEWHELPCMD "~/menuapp/doc/view.help"
#define VIEWPRTCMD "view -g010050003004 -r -i -s -PL"
#define VIEWPRTFILE "prtout"
#define DEFAULTSHELL "/bin/sh"
#define MINITRC "~/.minitrc"
#define MARGS "-d ~/menuapp -m main.m"
#define MAPP_DIR "~/menuapp"
#define PRINTCMD "lp -c -s"
#define MAXOPTS 25
#define P_ERROR -1
#define F_NOMETAS 1
#define F_NOTBLANK 2
#define F_NOECHO 4

enum { MT_NULL, MT_CENTERED_TEXT, MT_LEFT_JUST_TEXT, MT_CHOICE };

enum { MA_INIT, MA_RETURN, MA_RETURN_MAIN, MA_DISPLAY_MENU, MA_ENTER_OPTION };

enum {
    CT_NULL,
    CT_RETURNMAIN,
    CT_EXEC,
    CT_HELP,
    CT_PAINTFILE,
    CT_PAINT,
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

enum { P_ACCEPT, P_HELP, P_CANCEL, P_REFUSE, P_CONTINUE };

/* curs.h */
#include <ncursesw/ncurses.h>

#define REASSIGN_STDIN

#undef key_left
#undef key_right
#undef key_down
#undef key_up

extern const char mapp_version[20];
extern struct termios shell_ioctl;
extern bool f_debug;
extern int f_erase_remainder;
extern int f_stop_on_error;
extern int f_shell_ioctl;
extern int f_curses_open;
extern int f_restore_screen;

extern void capture_shell_ioctl();
extern void restore_shell_ioctl();

enum {
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
};

enum {
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
    KEY_ALT9
};

#define KEY_ESC 0x01b

enum {
    key_null,
    key_ctla,
    key_ctlb,
    key_ctlc,
    key_ctld,
    key_ctle,
    key_ctlf,
    key_ctlg,
    key_ctlh,
    key_ctli,
    key_ctlj,
    key_ctlk,
    key_ctll,
    key_ctlm,
    key_ctln,
    key_ctlo,
    key_ctlp,
    key_ctlq,
    key_ctlr,
    key_ctls,
    key_ctlt,
    key_ctlu,
    key_ctlv,
    key_ctlw,
    key_ctlx,
    key_ctly,
    key_ctl,
};

#define key_left 8   // ^H
#define key_down 10  // ^J
#define key_up 11    // ^K
#define key_right 12 // ^L
#define key_cr 13    // ^M

#define FG_COLOR 2
#define BG_COLOR 0
#define BO_COLOR 2
#define COLOR_LEN 32

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
// end curs.h

// globals
// --------------------------------------------------------------------------

extern int m_menu(int, char **, int, int, int, int, int, int);
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

} line_;

typedef struct {
    WINDOW *win;
    WINDOW *box;
    int lines;
    int cols;
    int begy;
    int begx;
    char *title;
    int choice_max_len;
    int text_max_len;
    int option_offset;
    int option_max_len;
    int item_count;
    int line_idx;
    char caller;
    int argc;
    char *argv[MAXARGS];
    int rargc;
    char *rargv[MAXARGS];
    line_ *line[MAX_LINES];

} menu_;

extern menu_ *menu;

//--------------------------------------------------------------------------

// log.c
void get_rfc3339_s(char *, size_t);
int open_log(char *);
void write_log(char *);
// end log.c

// init.c
typedef struct {
    char cmd_str[MAXLEN];   // c: command
    char mapp_dir[MAXLEN];  // d: application directory
    char mapp_desc[MAXLEN]; // e: application description file
    char mapp_spec[MAXLEN]; //    application description qualified path
    char geometry[MAXLEN];  // g: geometry (lllcccLLLCCC)
    char in_file[MAXLEN];   // i: input file
    int selections;         // n: number of selections
    char out_file[MAXLEN];  // o: output file
    bool f_at_end_remove;   // r  remove file at end of program
    bool f_squeeze;         // s  squeeze multiple blank lines
    int tab_stop;           // t: number of spaces per tab
    bool f_ignore_case;     // x: ignore case in search
    bool f_at_end_clear;    // z  clear screen at end of program
    int bg_color;           // B: background color
    int fg_color;           // F: foreground color
    int bo_color;           // O: border color
    char prompt[MAXLEN];    // P: prompt (S-Short, L-Long, N-None)[string]
    bool f_stop_on_error;   // Z  stop on error
    char start_cmd[MAXLEN]; // +  command to execute at start of program
} opt;

extern opt *option;

extern char minitrc[MAXLEN];
extern int initialization(int, char **);
extern int opt_process_cmdline(int, char **);
extern int write_config();
// end init.c

// ckeys.c
extern int ckeys(int, int);
// end ckeys.c

// futil.c
extern int str_to_args(char **, char *);
extern void str_to_lower(char *);
extern void str_to_upper(char *);
extern void str(char *);
extern int strn(char *, int);
extern int strnz(char *, int);
extern void strnz_cpy(char *, char *, int);
extern char *strz_dup(char *);
extern char *strnz_dup(char *, int);
extern void str_subc(char *, char *, char, char *, int);
extern void normalize_file_spec(char *);
extern void file_spec_path(char *, char *);
extern void file_spec_name(char *, char *);
extern int parse_geometry_str(char *, int *, int *, int *, int *);
extern int get_color_number(char *s);
extern void list_colors();
extern void build_color_opt_str();
extern int verify_screen_geometry(int *, int *, int *, int *);
extern int full_screen_fork_exec(char **);
extern int full_screen_shell(char *);
extern int shell(char *);
extern void abend(int, char *);
extern char errmsg[];
extern char color_opt_str[MAXLEN + 1];
// end futil.c

// dgraph.c

extern int display_text_graph();
extern void print_graph();
extern void file_graph();
extern void stdin_graph();
extern char *gn_cpy(char *, int);
// end dgraph.c

// dwin.h
#define MAXWIN 20

typedef unsigned char uchar;

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
extern int const ncolors[16];
extern char const colors_text[16][10];

enum Color {
    black,
    red,
    green,
    yellow,
    blue,
    cyan,
    magenta,
    white,
    bblack,
    bred,
    bgreen,
    byellow,
    bblue,
    bcyan,
    bmagenta,
    bwhite
};

extern char tmp_str[MAXLEN + 1];
extern char *tmp_ptr;
extern unsigned int cmd_key;

// begin dwin.c
extern int win_new(int, int, int, int, char *);
extern void win_redraw(WINDOW *, int, char *);
extern WINDOW *win_open_box(int, int, int, int, char *);
extern WINDOW *winOpenwin(int, int, int, int);
extern WINDOW *win_del();
extern void win_close_win(WINDOW *);
extern void win_close_box(WINDOW *);
extern void restore_wins();
extern void dmvwaddstr(WINDOW *, int, int, char *);
extern void cbox(WINDOW *);
extern void win_init_attrs(int, int, int);
extern void win_Toggle_Attrs();
extern int display_ok_message(char *);
extern int display_error_message(char *);
extern void mvwaddstr_fill(WINDOW *, int, int, char *, int);
// end dwin.c

// fields.c
extern int accept_field(WINDOW *, int, int, int, char *, int, char, int);
extern void display_field(WINDOW *, int, int, char *, int, char, int);
extern int format_field(char *, int, char, int);
extern int validate_field(char *, int);
// end fields.c

// fmenu.c
extern int disp_menu(menu_ *);
extern int menu_loop(menu_ *);
// end fmenu.c

// fpaint.c
typedef struct {
    int line;
    int col;
    char str[MAX_COLS + 1];
} disp_;

typedef struct {
    int line;
    int col;
    int len;
    int val;
    char str[MAX_COLS + 1];
} field_;

typedef struct {
    WINDOW *win;
    WINDOW *box;
    int lines;
    int cols;
    int begx;
    int begy;
    int didx;
    int title_line;
    int fidx;
    int field_pos;
    char title[MAX_COLS + 1];
    char cmd_str[MAXLEN + 1];
    char desc_file_name[MAXLEN + 1];
    char answer_file_name[MAXLEN + 1];
    char help_file_name[MAXLEN + 1];
    disp_ *disp[MAXFIELDS];
    field_ *field[MAXFIELDS];

} paint_;

extern paint_ *paint;

extern int paint_form(char *, char *, int, int);
extern int paintfile(char *, char *, int, int);
extern void init_paint_struct();
extern void free_paint_struct();
extern int display_paint_screen();
extern int paint_enter_fields();
extern int read_description_file();
extern int get_disp_str(char *);
extern int read_answer_file();
extern int write_answer_file();
extern void help(char *);
// end fpaint.c

// cpick.h
#define OBJ_MAXLEN 80
#define OBJ_MAXCNT 256

typedef struct {
    WINDOW *win;
    WINDOW *box;
    FILE *in_fp;
    FILE *out_fp;
    int lines;
    int cols;
    int begy;
    int begx;
    int select_idx;
    int f_append_cmd_args;
    int argc;
    char *argv[MAXARGS];
    char *title;
    char in_file[MAXLEN];
    char out_file[MAXLEN];
    char cmd_str[MAXLEN];
    char *object[OBJ_MAXCNT];
    int f_selected[OBJ_MAXCNT];
    char in_buf[BUFSIZ];
    int select_cnt;
    int obj_cnt;
    int page_cnt;
    int line_cnt;
    int row_cnt;
    int col_cnt;
    int idx;
    int page;
    int line;
    int col;
    int x;
    int width;

} pick_;

extern pick_ *pick;

// end cpick.h

// fpick.c
extern int pick_obj();
extern void save_object(char *);
extern int pick_object();
extern void display_page();
extern void reverse_object();
extern void toggle_object();
extern int output_objects();
extern int exec_objects(char *);
extern int open_pick_win();
// end fpick.c

// mpick.c
extern int Mpick(int, char **, int, int, int, int, char *, int);
// end mpick.c

// fview.c
extern void init_view_struct();
extern int open_view_stdscr();
extern int open_view_win();
extern int initialize_file(char *);
extern int command_processor();
extern int get_cmd_char();
extern int get_cmd_str(char *);
extern void build_prompt(char, char *);
extern void cat_file();
extern void lp(char *, char *);
extern void go_to_mark(int);
extern void go_to_eof();
extern void go_to_line(int);
extern void go_to_percent(int);
extern void go_to_position(long);
extern void search(int, char *, int);
extern void read_forward_from_current_file_pos(int);
extern void read_forward_from_file_pos(int, long);
extern long read_line_forward(long);
extern long read_line_forward_raw(long);
extern void read_backward_from_current_file_pos(int);
extern void read_backward_from_file_pos(int, long);
extern long read_line_backward(long);
extern long read_line_backward_raw(long);
extern int initialize_buffers(int);
extern int get_char_next_block();
extern int get_char_prev_block();
extern int get_char_buffer();
extern int locate_byte_pipe(long);
extern void put_line();
extern void display_error_msg(char *);
extern void command_line_prompt(char *);
extern void remove_file();
extern void viewDisplayhelp();
// end fview.c

// mousefn.c
extern void mouse_getch(int *, int *, int *, int *);
extern void w_mouse_getch(WINDOW *, int *, int *, int *, int *);
// end mousefn.c

// mview.c
extern char err_msg[MAXLEN];
extern int mview(int, char **, int, int, int, int, char *, int);
// end mview.c

// rmenu.c
extern int read_menu_file(menu_ *);
extern char get_command_type(char *);
extern void free_menu_line(line_ *);
// end rmenu.c

// curskeys.c
extern void init_stdscr();
extern void curskeys(WINDOW *);
// end curskeys.c

// scriou.c
extern void open_curses();
extern void close_curses();
extern void end_pgm();
extern void sig_prog_mode();
extern void sig_shell_mode();
extern int fork_exec(char **);
extern void display_argv_error_msg(char *, char **);
extern char di_getch();
extern void Displaymenu();
extern int enter_option();
// end scriou.c

#endif
