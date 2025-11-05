// menu.h
// Bill Waller
// billxwaller@gmail.com

#ifndef _MENU_H
#define _MENU_H 1

#include <ncursesw/ncurses.h>
#include <setjmp.h>
#include <stddef.h>

#define MAXLEN 256
#define MIN_COLS 40
#define MAX_COLS 256
#define DEF_COLS 40
#define MAX_LINES 128
#define PICK_MAX_ARGS 200
#define PICK_MAX_ARG_LEN 256
#define VIEW_MAX_ARGS 200
#define VIEW_MAX_ARG_LEN 256
#define DEF_LINES 10
#define NCOLORS 16
#define MAXARGS 50
#define MAXFIELDS 50
#define ACCEPT_PROMPT_CHAR '_'
#define DEFAULTEDITOR "vi"
#define EDITOR "vi"
#define MENU_HELP_FILE "~/menuapp/help/menu.help"
#define PICK_HELP_FILE "~/menuapp/help/pick.help"
#define FORM_EXEC_HELP_FILE "~/menuapp/help/formexec.help"
#define FORM_WRITE_HELP_FILE "~/menuapp/help/formwrite.help"
#define VIEW_HELP_FILE "~/menuapp/help/view.help"
// #define HELP_CMD "view -g010050003004 -s -x -z -PL"
#define HELP_CMD "view"
#define VIEW_PRT_FILE "~/menuapp/data/prtout"
#define VIEW_PRT_CMD "lp"
#define DEFAULTSHELL "/bin/bash"
#define MINITRC "~/.minitrc"
#define MAPP_DIR "~/menuapp"
#define PRINTCMD "lp -c -s"
#define MAXOPTS 50
#define P_ERROR -1
#define F_NOMETAS 1
#define F_NOTBLANK 2
#define F_NOECHO 4
#define EIGHT 8

extern char *eargv[MAXARGS];

enum Caller { VIEW, FORM, PICK, MENU };
enum OptType {
    OT_STRING,
    OT_INT,
    OT_BOOL,
};
enum OptGroup { OG_FILES, OG_DIRS, OG_SPECS, OG_MISC, OG_PARMS, OG_FLAGS };
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

// futil.c verify_file flags
#define WC_OK (W_OK | 0x1000)

// CURSES
// ----------------------------------------------------------
#define REASSIGN_STDIN

#undef key_left
#undef key_right
#undef key_down
#undef key_up

#define TRUE 1
#define FALSE 0

#ifndef BUFSIZ
#define BUFSIZ 4096
#endif

#define to_uppercase(c)                                                        \
    if (c >= 'a' && c <= 'z')                                                  \
    c -= ' '

extern const char *mapp_version;

extern bool f_debug;
extern bool f_stop_on_error;

extern char const colors_text[16][10];
extern struct termios shell_tioctl, curses_tioctl;
extern struct termios shell_in_tioctl, curses_in_tioctl;
extern struct termios shell_out_tioctl, curses_out_tioctl;
extern struct termios shell_err_tioctl, curses_err_tioctl;

// ----------------------------------------------------------
// scriou.c
// ----------------------------------------------------------
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
// scriou.c end

// ----------------------------------------------------------
// sig.c
// ----------------------------------------------------------
extern void signal_handler(int);
extern void sig_prog_mode();
extern void sig_dfl_mode();

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
#define KEY_TAB 0x09

enum {
    KEY_NULL,
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
    KEY_CTLZ
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

// WINDOWING
// ---------------------------------------------------------
#define MAXWIN 20

typedef unsigned char uchar;

// scriou.c
extern void open_curses();
extern void close_curses();
extern void sig_prog_mode();
extern void sig_shell_mode();
extern int fork_exec(char **);
extern void display_argv_error_msg(char *, char **);
extern char di_getch();
extern int enter_option();
//
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

extern char tmp_str[MAXLEN];
extern char *tmp_ptr;
extern unsigned int cmd_key;

// dwin.c
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
extern int form_desc_error(int, char *, char *);
extern int error_message(char **);
extern void mvwaddstr_fill(WINDOW *, int, int, char *, int);
extern int display_curses_keys();
extern void init_stdscr();
extern void curskeys(WINDOW *);
extern void mouse_getch(int *, int *, int *, int *);
extern void w_mouse_getch(WINDOW *, int *, int *, int *, int *);
extern void abend(int, char *);
extern void user_end();

// MENU
// --------------------------------------------------------------------------

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

enum { P_ACCEPT, P_HELP, P_CANCEL, P_REFUSE, P_CONTINUE };

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
    //
    int choice_max_len;
    int text_max_len;
    int option_offset;
    int option_max_len;
    int item_count;
    int line_idx;
    Line *line[MAX_LINES];
    bool f_stop_on_error;
} Menu;

extern Menu *menu;

// FORMS
//----------------------------------------------------------

extern int accept_field(WINDOW *, int, int, int, char *, int, char, int, bool);
extern void display_field(WINDOW *, int, int, char *, int, char, int);
extern int format_field(char *, int, char, int);
extern int validate_field(char *, int);

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
    int val;
    char str[MAX_COLS];
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
    bool f_stop_on_error;
    int fidx;
    int didx;
    int field_pos;
    Text *text[MAXFIELDS];
    Field *field[MAXFIELDS];
    int item_count;
    int choice_max_len;
    int text_max_len;
    int option_offset;
    int option_max_len;
    int line_idx;
    int op;
} Form;

extern Form *form;

extern int open_form_win(Form *);
extern int display_form_screen(Form *);
extern int form_enter_fields(Form *);
extern int read_form_description(Form *);
extern int read_form_answer_file(Form *);
extern int write_form_answer(Form *);
extern void help(char *);

// PICK
// -----------------------------------------------------------
#define OBJ_MAXLEN 80
#define OBJ_MAXCNT 256

typedef struct {
    // colors & geometry
    int fg_color;  // F: foreground_color
    int bg_color;  // B: background_color
    int bo_color;  // O: border_color
    int scr_lines; // L: lines
    int scr_cols;  // C: columns
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
    FILE *in_fp, *out_fp;
    // files
    char mapp_spec[MAXLEN]; //    application qualified path
    char in_spec[MAXLEN];
    char out_spec[MAXLEN];
    char cmd_spec[MAXLEN];  // c: command
    char help_spec[MAXLEN]; //    application help qualified path
    // file flags
    bool f_mapp_spec;
    bool f_in_spec;
    bool f_out_spec;
    bool f_cmd_spec;
    bool f_help_spec;
    bool f_multiple_cmd_args;
    bool f_stop_on_error;
    bool f_selected[OBJ_MAXCNT];
    char in_buf[BUFSIZ];
    char *object[OBJ_MAXCNT];
    int select_idx;
    int select_cnt;
    int obj_cnt;
    int idx;
    int tab_pages;
    int tab_page;
    int tab_lines;
    int tab_cols;
    int tab_col;
    int tab_width;
} Pick;

extern Pick *pick;

// VIEW
// --------------------------------------------------------------------------

#define VBUFSIZ 4096
#define PIPEBUFS 16
#define FILEBUFS 4
#define NPOS 32
#define NMARKS 27
#define MAXLEN 256

typedef struct {
    long block_no;
    char data[VBUFSIZ];
    char *end_ptr;
} block_struct;

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
    char prompt[MAXLEN]; // -P: prompt (S-Short, L-Long, N-None)[string]
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
    int curx, cury, max_col, next_c, scroll_lines, cmd_line;
    int first_column, last_column, last_line, ptop, pbot;
    int attr, line_mode;
    int fd;
    long blk_no, last_blk_no, size_bytes, prev_file_pos, file_pos;
    long pos_tbl[NPOS], mark_tbl[NMARKS];
    int blk_offset;
    bool f_beg_of_file, f_eof, f_forward, f_is_pipe, f_new_file,
        f_pipe_processed, f_redraw_screen, f_displaying_help, f_stdout_is_tty;
    char cur_file_str[MAXLEN];
    char line_str[MAX_COLS];
    char line_out_str[MAX_COLS];
    char start_cmd_all_files[MAXLEN];
    char *file_spec_ptr;
    char *next_file_spec_ptr, *tmp_file_name_ptr, *line_start_ptr, *line_ptr,
        *buf_start, *buf_ptr, *buf_end;
    block_struct *blk_first, *blk_last, *blk_curr;
    jmp_buf cmd_jmp;
} View;

extern View *view;

// INIT
//----------------------------------------------------------
extern int view_file(View *);

typedef struct {
    // colors & geometry
    int fg_color; // -F: foreground_color
    int bg_color; // -B: background_color
    int bo_color; // -O: border_color
    int lines;    // -L: lines
    int cols;     // -C: columns
    int begx;     // -X: lines
    int begy;     // -Y: lines
    // window
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
    int selections; // -n: number of selections
    // view
    char start_cmd[MAXLEN]; // -S: command to execute at start of program
    char prompt[MAXLEN];    // -P: prompt (S-Short, L-Long, N-None)[string]
    int tab_stop;           // -t: number of spaces per tab
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

extern bool init_menu_files(Init *, int, char **);
extern bool init_pick_files(Init *, int, char **);
extern bool init_form_files(Init *, int, char **);
extern bool init_view_files(Init *, int, char **);

extern char minitrc[MAXLEN];

extern void mapp_initialization(Init *init, int, char **);
extern Menu *new_menu(Init *init, int, char **, int, int);
extern void parse_opt_args(Init *, int, char **);
extern int write_config(Init *);
extern bool derive_file_spec(char *, char *, char *);

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
extern int init_cnt;

// PICK
//-----------------------------------------------------------
extern int init_pick(Init *, int, char **, int, int);
extern int pick_engine(Init *);
extern void save_object(Pick *, char *);
extern bool picker(Pick *);
extern void display_page(Pick *);
extern void reverse_object(Pick *);
extern void toggle_object(Pick *);
extern int output_objects(Pick *);
;
extern int exec_objects(Pick *, char *);
extern int open_pick_win(Pick *);
extern int mpick(int, char **, int, int, int, int, char *, int);

// FORM
//-----------------------------------------------------------
extern int form_process(Init *);
// end fform.c

// MENU
//-----------------------------------------------------------

extern int menu_engine(Init *);
extern int menu_loop(Init *);
extern int parse_menu_description(Init *);
extern char get_command_type(char *);
extern void free_menu_line(Line *);

// VIEW
//--------------------------------------------------------------
extern int mview(Init *, int, char **, int, int, int, int);
extern int init_view_stdscr(View *);
extern int init_view_boxwin(View *);
extern int view_init_input(View *, char *);
extern int view_cmd_processor(View *);
extern int get_cmd_char(View *);
extern int get_cmd_spec(View *, char *);
extern void build_prompt(View *, char, char *);
extern void cat_file(View *);
extern void lp(char *, char *);
extern void go_to_mark(View *, int);
extern void go_to_eof(View *);
extern void go_to_line(View *, int);
extern void go_to_percent(View *, int);
extern void go_to_position(View *, long);
extern void search(View *, int, char *, int);
extern void read_forward_from_current_file_pos(View *, int);
extern void read_forward_from_file_pos(View *, int, long);
extern long read_line_forward(View *, long);
extern long read_line_forward_raw(View *, long);
extern void read_backward_from_current_file_pos(View *, int);
extern void read_backward_from_file_pos(View *, int, long);
extern long read_line_backward(View *, long);
extern long read_line_backward_raw(View *, long);
extern int initialize_buffers(View *, int);
extern int get_char_next_block(View *);
extern int get_char_prev_block(View *);
extern int get_char_buffer(View *);
extern int locate_byte_pipe(View *, long);
extern void put_line(View *);
extern void display_error_msg(View *, char *);
extern void mk_cmd_prompt(char *);
extern void remove_file(View *);
extern void view_display_help(View *);
extern char err_msg[MAXLEN];

// UTILITIES
// --------------------------------------------------------------
extern bool str_to_bool(const char *);
extern int str_to_args(char **, char *);
extern void str_to_lower(char *);
extern void str_to_upper(char *);
extern void str(char *);
extern int strn(char *, int);
extern int strnz__cpy(char *, char *, int);
extern int strnz(char *, int);
extern void strnz_cpy(char *, char *, int);
extern char *strz_dup(char *);
extern char *strnz_dup(char *, int);
extern void str_subc(char *, char *, char, char *, int);
extern void normalize_file_spec(char *);
extern void file_spec_path(char *, char *);
extern void file_spec_name(char *, char *);
extern int get_color_number(char *);
extern void list_colors();
extern int full_screen_fork_exec(char **);
extern int full_screen_shell(char *);
extern int shell(char *);

extern char errmsg[];
extern void get_rfc3339_s(char *, size_t);
extern int open_log(char *);
extern void write_log(char *);
extern bool dir_name(char *, char *);
extern bool base_name(char *, char *);
extern bool trim_ext(char *, char *);
extern bool trim_path(char *);
extern bool expand_tilde(char *, int);
extern bool verify_file(char *, int);
extern bool verify_file_q(char *, int);
extern bool verify_dir(char *, int);
extern bool verify_dir_q(char *, int);
extern bool form_answer_spec(Init *, int argc, char **argv);
extern bool form_help_spec(Init *, int argc, char **argv);
extern bool pick_help_spec(Init *, int argc, char **argv);
extern bool pick_in_spec(Init *, int argc, char **argv);
extern bool pick_out_spec(Init *, int argc, char **argv);
extern bool construct_file_spec(char *, char *, char *, char *, char *, int);
extern bool locate_file_in_path(char *, char *);

// DEPRECATED
// ---------------------------------------------------------
extern int display_text_graph();
extern void print_graph();
extern void file_graph();
extern void stdin_graph();
extern char *gn_cpy(char *, int);

#endif
