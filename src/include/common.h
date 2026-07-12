/** @file common.h
    @brief Headder for C-Menu Menu, Form, Pick, and View components
    @author Bill Waller
    Copyright (c) 2025
    MIT License
    billxwaller@gmail.com
    @date 2026-02-09
 */

#ifndef _COMMON_H
#define _COMMON_H 1

#define _GNU_SOURCE
// #define _XOPEN_SOURCE_EXTENDED 1
#define NCURSES_WIDECHAR 1
#include "cm.h"
#include <form.h>
#include <menu.h>
#include <pick.h>
#include <view.h>

#include <stddef.h>
#define USE_PAD TRUE
#define MIN_COLS 40

#define MAX_WIDE_LEN 1024
#define COLOR_LEN 8
#define PICK_MAX_ARG_LEN 256
#define MAX_PICK_OBJS 1024
#define ACCEPT_PROMPT_CHAR '_'

#define MENU_HELP_FILE "menu.help"
#define FORM_HELP_FILE "form.help"
#define PICK_HELP_FILE "pick.help"
#define VIEW_HELP_FILE "view.help"
#define VIEW_PRT_FILE "prtout"
#define DEFAULTEDITOR "vi"

#define MAX_VIEWS 10
#define MAX_FORMS 10
#define MAX_MENUS 10
#define MAX_PICKS 10

#define MINITRC ".minitrc"
#define ABOUT ".about"
#define MAPP_DIR "~/menuapp"
#define PRINTCMD "lp -c -s"
#define MAXOPTS 50
#define EIGHT 8
#define P_READ 0
#define P_WRITE 1
#define TRUE 1

#define new_cmenu()                        \
    __end_pgm;                             \
    int main(int argc, char **argv) {      \
        __atexit;                          \
        capture_shell_tioctl();            \
        Init *init = new_init(argc, argv); \
        SIO *sio = init->sio;              \
        mapp_initialization(init);         \
        open_curses(sio);                  \
        sig_prog_mode();                   \
        capture_curses_tioctl();           \
        win_init_attrs();

#define destroy_cmenu() \
    destroy_init(init); \
    win_del();          \
    curs_set(1);        \
    destroy_curses();   \
    return 0;           \
    }

typedef enum {
    VIEW,
    FORM,
    PICK,
    MENU
} Caller;

/** @brief option types */
typedef enum {
    OT_STRING,
    OT_INT,
    OT_BOOL,
    OT_HEX,
} OptType;

/** @brief option groups */
typedef enum {
    OG_FILES,
    OG_DIRS,
    OG_SPECS,
    OG_MISC,
    OG_PARMS,
    OG_FLAGS,
    OG_COL
} OptGroup;

typedef Menu Menu;
typedef Form Form;
typedef Pick Pick;
typedef View View;

/**
 * @struct Init
 * @brief Gathers runtime information for C-Menu Menu, Form, Pick, and View
 * components, used for passing common data and state during initialization and
 * processing of these componentsi.
 */
typedef struct
{
    SIO *sio;
    int lines; /**< number of lines for window size */
    int cols;  /**< number of columns for window size */
    int begy;  /**< screen line upper left corner of window */
    int begx;  /**< screen column upper left corner of window */

    char provider_cmd[MAXLEN]; /**< command provides input */
    char receiver_cmd[MAXLEN]; /**< command receives output */
    char cmd[MAXLEN];          /**< command to execute in foreground, e.g. an editor */
    char cmd_all[MAXLEN];      /**< View - command to execute at start of program */
    char parent_cmd[MAXLEN];   /**< command to be executed by parent */
    char prompt_str[MAXLEN];   /**< prompt string for chyron */
    int prompt_type;           /**< View - prompt type for chyron, e.g. 0=short, 1=long,
                                  2=none */
    char chyron_s[MAXLEN];     /**< chyron string */
    char title[MAXLEN];        /**< display on top line of box window */
    int argc;                  /**< command line arguments count */
    char **argv;               /**< command line arguments vector */
    int optind;                /**< getopt pointer to non-option arguments in argv */
    bool f_ignore_case;        /**< View - ignore case in search */
    bool p_view_files;         /**< Pick - View Pick Files */
    bool f_at_end_remove;      /**< obsolete, unneeded */
    bool f_strip_ansi;         /**< strip ansi escape sequences when writing buffer */
    bool f_squeeze;            /**< View - print one line for each group of blank lines
                                */
    bool f_multiple_cmd_args;  /**< View - put multiple arguments in a single
                                  string */
    bool f_erase_remainder;    /**< Form - erase remainder of field on enter */
    bool f_read_theme;         /**< Pick - read and process theme */
    bool f_ln;                 /**< View - show line numbers */
    bool wrap;                 /**< View - wrap lines */
    char brackets[3];          /**< Form - left and right enclosing characters */
    char fill_char[4];         /**< Form - fill character for fields */
    char mapp_home[MAXLEN];    /**< home directory */
    char mapp_data[MAXLEN];    /**< data directory */
    char mapp_help[MAXLEN];    /**< help directory */
    char mapp_msrc[MAXLEN];    /**< source directory */
    char mapp_user[MAXLEN];    /**< user directory */
    char mapp_theme[MAXLEN];   /**< default theme file */

    bool f_mapp_home; /**< flag - mapp_home verified */
    bool f_mapp_data; /**< flag - mapp_data verified */
    bool f_mapp_spec; /**< flag - mapp_spec verified */
    bool f_mapp_help; /**< flag - mapp_help verified */
    bool f_mapp_msrc; /**< flag - mapp_msrc verified */
    bool f_mapp_user; /**< flag - mapp_user verified */
    // file flags
    bool f_mapp_desc;       /**< flag - mapp_desc verified */
    bool f_provider_cmd;    /**< flag - provider_cmd verified */
    bool f_receiver_cmd;    /**< flag - receiver_cmd verified */
    bool f_cmd;             /**< flag - cmd verified */
    bool f_cmd_all;         /**< flag - cmd_all verified */
    bool f_title;           /**< flag - title verified */
    bool f_help_spec;       /**< flag - help_spec verified */
    char in_spec[MAXLEN];   /**< input file spec */
    char out_spec[MAXLEN];  /**< output file spec */
    bool f_in_spec;         /**< in_spec verified */
    bool f_out_spec;        /**< out_spec verified */
    char editor[MAXLEN];    /**< default editor */
    char menuapp[MAXLEN];   /**< main configuration file, e.g. cmenu/.minitrc */
    char minitrc[MAXLEN];   /**< main configuration file, e.g. cmenu/.minitrc */
    char about_fn[MAXLEN];  /**< about information file, e.g. cmenu/.about */
    char mapp_spec[MAXLEN]; /**< description file */
    char help_spec[MAXLEN]; /**< help file */
    // Pick
    int select_max; /**< Pick maximum number of selections */
    // View
    int tab_stop; /**< View - number of spapaces per tab */
    int h_shift;  /**< View - horizontal scroll shift width */
    Menu *menu;   /**< menu data structure */
    int menu_cnt; /**< number of menu data structures allocated */
    Form *form;   /**< form data structure */
    int form_cnt; /**< number of form data structures allocated */
    Pick *pick;   /**< pick data structure */
    int pick_cnt; /**< number of pick data structures allocated */
    View *view;   /**< view data structure */
    int view_cnt; /**< number of view data structures allocated */
} Init;

// extern Init *init;
extern int init_cnt; /** number of Init data structures allocated */
extern char minitrc[MAXLEN];
extern void mapp_initialization(Init *, int, char **);
extern Init *new_init(int, char **);
extern View *new_view(Init *);
extern Form *new_form(Init *, int, char **, int, int);
extern Pick *new_pick(Init *, int, char **, int, int);
extern Menu *new_menu(Init *, int, char **, int, int);
extern int popup_menu(Init *, int, char **, int, int);
extern int popup_form(Init *, int, char **, int, int);
extern int popup_pick(Init *, int, char **, int, int);
extern int popup_view(Init *, int, char **, int, int, int, int);
extern int popup_ckeys();
extern Menu *destroy_menu(Init *init);
extern Pick *destroy_pick(Init *init);
extern Form *destroy_form(Init *init);
extern View *destroy_view(Init *init);
extern Init *destroy_init(Init *init);
extern int parse_opt_args(Init *, int, char **);
extern void zero_opt_args(Init *);
extern int write_config(Init *);
extern bool derive_file_spec(char *, char *, char *);
extern bool init_menu_files(Init *, int, char **);
extern unsigned int menu_engine(Init *);
extern unsigned int parse_menu_description(Init *);
extern int init_form(Init *, int, char **, int, int);
extern int init_pick(Init *, int, char **, int, int);
extern int open_pick_win(Init *);
extern int pick_engine(Init *);
extern int cmd_processor(Init *);
extern int view_file(Init *);
extern int init_view_full_screen(Init *);
extern int init_view_boxwin(Init *);
extern int view_init_input(Init *, char *);
extern void view_calc_full_screen_dimensions(Init *);
extern void view_full_screen_resize(Init *);
extern void view_calc_boxwin_dimensions(Init *);
extern void view_boxwin_resize(Init *);
extern int process_config_file(char *, Init *);
extern void initialize_local_colors(SIO *);
extern void initialize_line_table(View *);
extern void next_page(View *);
extern int view_cmd_processor(Init *);
extern int pad_refresh(View *);
extern void build_prompt(View *);          // View build prompt
extern int display_prompt(View *, char *); // view display prompt
extern ViewStack view_stack;
extern bool view_stack_init(ViewStack *, size_t);
extern bool view_stack_push(ViewStack *, View);
extern bool view_stack_pop(ViewStack *, View *);
extern bool view_stack_peek(const ViewStack *, View *);
extern void view_stack_free(ViewStack *);
extern int view_cmd_processor(Init *);
extern void destroy_view_win(Init *);
extern void destroy_line_table(View *);
#endif
