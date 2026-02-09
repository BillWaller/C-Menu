/// One large include file for C-Menu Menu, Form, Pick, and View
//  Bill Waller Copyright (c) 2025
//  MIT License
//  billxwaller@gmail.com

#ifndef _MENU_H
#define _MENU_H 1

#define _XOPEN_SOURCE_EXTENDED 1
#define NCURSES_WIDECHAR 1
#include "cm.h"
#include <ncursesw/ncurses.h>
#include <signal.h>
#include <stddef.h>
#include <stdlib.h>
#define C_MENU_VERSION "C-Menu-0.2.6"
#define USE_PAD TRUE
#define MIN_COLS 40
#define BUFSIZ 8192
#define LINE_IN_MAX_COLS 2048
#define MAX_COLS 1024
#define MAX_WIDE_LEN 1024
#define COLOR_LEN 8
#define MAX_MENU_LINES 256
#define PICK_MAX_ARG_LEN 256
#define MAX_ARGS 64
#define MAXFIELDS 50
#define MAX_PICK_OBJS 1024
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
#define F_VIEW 0x01
#define S_WCOK 0x1000
#define S_QUIET 0x2000
#define P_READ 0
#define P_WRITE 1
#define TRUE 1
#define to_uppercase(c)                                                        \
    if (c >= 'a' && c <= 'z')                                                  \
    c -= ' '
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
// extern int stdin_fd;
// extern int stdout_fd;
// extern int stderr_fd;
extern const char *mapp_version;
extern bool f_debug;
// extern bool f_stop_on_error;
extern char tmp_str[MAXLEN];
extern char *tmp_ptr;
extern unsigned int cmd_key;
extern int exit_code;

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
    unsigned int type;  // 0=string, 1=int, 2=bool
    unsigned int group; // 0=FILES, 1=SPECS, 2=MISC, 3=PARMS, 4=FLAGS
    const char *use;    // which programs use this option
                        // m=menu, p=pick, f=form, v=view
    const char *short_opt;
    const char *desc;
} Opts;

extern void dump_opts_by_use(char *, char *);

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
    off_t file_size;
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
typedef struct {
    SIO *sio;
    // colors & geometry
    int lines; // -L: lines
    int cols;  // -C: columns
    int begx;  // -X: lines
    int begy;  // -Y: lines
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
extern View *new_view(Init *init, int, char **);
extern Form *new_form(Init *init, int, char **, int, int);
extern Pick *new_pick(Init *init, int, char **, int, int);
extern Menu *new_menu(Init *init, int, char **, int, int);
extern Menu *destroy_menu(Init *init);
extern Pick *destroy_pick(Init *init);
extern Form *destroy_form(Init *init);
extern View *destroy_view(Init *init);
extern Init *destroy_init(Init *init);
extern int parse_opt_args(Init *, int, char **);
extern void zero_opt_args(Init *);
extern int write_config(Init *);
extern bool derive_file_spec(char *, char *, char *);

extern int view_file(Init *);
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
extern bool init_menu_files(Init *, int, char **);
extern unsigned int menu_engine(Init *);
extern unsigned int menu_loop(Init *);
extern unsigned int parse_menu_description(Init *);
extern unsigned int get_command_type(char *);
extern void free_menu_line(Line *);
extern int init_form(Init *, int, char **, int, int);
extern int form_accept_field(Form *);
extern int form_display_field(Form *);
extern int form_display_field_n(Form *, int);
extern int form_open_win(Form *);
extern int form_enter_fields(Form *);
extern int form_read_description(Form *);
extern int form_fmt_field(Form *, char *s);
extern int form_desc_error(int, char *, char *);
extern void form_help(char *);
extern int mview(Init *, int, char **);
extern int init_view_full_screen(Init *);
extern int init_view_boxwin(Init *, char *);
extern bool view_init_input(View *, char *);
extern int cmd_processor(Init *);
extern int get_cmd_spec(View *, char *);
extern void go_to_position(View *, long);
extern void cat_file(View *);
extern char err_msg[MAXLEN];
#endif
