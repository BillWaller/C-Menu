/** @file cm.h
 *  @brief Headder for C-Menu NCurses, libcmui.so
 *  @author Bill Waller
 *  Copyright (c) 2026
 *  MIT License
 *  billxwaller@gmail.com
 *  @date 2026-02-09
 */

#ifndef _UI_NCURSES_H
#define _UI_NCURSES_H 1

#define _GNU_SOURCE
#define _XOPEN_SOURCE_EXTENDED
#define NCURSES_WIDECHAR 1

#include <ncursesw/ncurses.h>
#include <ncursesw/panel.h>

#ifdef UAL_UI
#include <ui_backend.h>
#include <ui_ncurses_internal.h>
#endif

#define MAXWIN 30 /**< maximum number of windows that can be created */
extern SCREEN *screen;
extern FILE *tty_fp;

#ifdef UAL_UI
extern UiRuntime *ui_runtime;
extern UiSurface *ui_box[MAXWIN];
extern UiSurface *ui_win[MAXWIN];
extern UiSurface *ui_win2[MAXWIN];
#endif
#define MAX_ARGS 64   /**< maximum number of arguments for external commands */
#define MAXLEN 256    /**< maximum length for strings and buffers */
#define MAXARGS 64    /**< maximum number of arguments */
#define SCR_COLS 1024 /**< maximum number of columns in the terminal screen */
#define MAX_DEPTH 3   /**< default depth for recursive file searching */
#define SCREEN_MAX_LINES 100
#define Ctrl(c) ((c) & 0x1f)

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
    CLR_IND_FG,
    CLR_IND_BG,
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

#define COLOR_LEN 8 /**< length of color code strings */

extern int vgetch(WINDOW *, int);
extern int xwgetch(WINDOW *, Chyron *, int);
extern int dxwgetch(WINDOW *, WINDOW *, WINDOW *, WINDOW *, WINDOW *, Chyron *, int);

extern int click_y;       /**< the y coordinate of a mouse click */
extern int click_x;       /**< the x coordinate of a mouse click */
extern WINDOW *mouse_win; /**< input from window n */

typedef struct {
    int r, g, b;
} RGB;

#define FG_COLOR 2    /**< default foreground color */
#define BG_COLOR 0    /**< default background color */
#define BO_COLOR 1    /**< default bold foreground color */
#define LN_COLOR 4    /**< default line number color */
#define LN_BG_COLOR 7 /**< default line number background */

extern int cp_default;   /**< default color pair index */
extern int cp_box;       /**< box color pair index */
extern int cp_ind;       /**< indicator color pair index */
extern int cp_bold;      /**< bold color pair index */
extern int cp_title;     /**< title color pair index */
extern int cp_highlight; /**< highlight color pair index */
extern int cp_fill_char; /**< fill character color pair index */
extern int cp_brackets;  /**< color pair index for field brackets */
extern int cp_nt;        /**< normal color pair index */
extern int cp_nt_rev;    /**< reverse color pair index */
extern int cp_nt_hl;     /**< highlight color pair index */
extern int cp_nt_hl_rev; /**< highlight reverse color pair index */
extern int cp_ln_fg;     /**< line number color pair index */
extern int cp_ln_bg;     /**< line number background color pair index */
extern int cp_cmdln_fg;  /**< command line number color pair index */
extern int cp_cmdln_bg;  /**< command line number background color pair index */
extern int cp_red;       /**< red background color pair index */
extern int cp_green;     /**< green background color pair index */
extern int cp_yellow;    /**< yellow background color pair index */
extern int cp_blue;      /**< blue background color pair index */
extern int clr_idx;      /**< current color index */
extern int clr_cnt;      /**< number of colors used */
extern int clr_pair_idx; /**< current color pair index */
extern int clr_pair_cnt; /**< number of color pairs supported by the terminal */

extern char const colors_text[][10]; /**< color codes for the 16 basic colors */

typedef struct {
    int fg;      /**< foreground color index */
    int bg;      /**< background color index */
    int pair_id; /**< color pair index */
} ColorPair;

/**< see termios.h */

extern struct termios shell_in_tioctl, curses_in_tioctl;
extern struct termios shell_out_tioctl, curses_out_tioctl;
extern struct termios shell_err_tioctl, curses_err_tioctl;

extern bool mk_raw_tioctl(struct termios *);
extern bool set_sane_tioctl(struct termios *);
extern int box_new(int, int, int, int, char *);
extern int box2_new(int, int, int, int, char *);
extern int box_vsplit_new(int, int, int, int, int, char *);
extern int win_new(int, int);
extern int win2_new(int, int, int, int);
extern int border_draw(WINDOW *);
extern int border_title(WINDOW *, char *);
extern int border_vsplit(WINDOW *, int);
extern int border_vsplit_text(WINDOW *, char *, int);
extern void win_redraw(WINDOW *);
extern void win_resize(int, int, char *);

extern cchar_t mkcc(int, attr_t, const char *);

extern cchar_t CC_REVERSE;   /**< curses default reverse */
extern cchar_t CC_NT;        /**< C-Menu normal text */
extern cchar_t CC_NT_REV;    /**< reverse */
extern cchar_t CC_NT_HL;     /**< highlight */
extern cchar_t CC_NT_HL_REV; /**< highlight reverse */
extern cchar_t CC_FILL_CHAR; /**< fill character */
extern cchar_t CC_BRKTL;     /**< left bracket */
extern cchar_t CC_BRKTR;     /**< right bracket */
extern cchar_t CC_BOX;       /**< indicator colors */
extern cchar_t CC_IND;       /**< box colors */
extern cchar_t CC_CMDLN;
extern cchar_t CC_LN;
extern cchar_t CC_DATA1;
extern cchar_t CC_DATA2;
extern cchar_t CC_DATA3;
extern cchar_t CC_TITLE; /**< title colors */

extern cchar_t CC_BRKTL;  /**< left field bracket */
extern cchar_t CC_BRKTR;  /**< right field bracket */
extern cchar_t CC_RED;    /**< red background */
extern cchar_t CC_GREEN;  /**< green background */
extern cchar_t CC_YELLOW; /**< yellow background */
extern cchar_t CC_BLUE;   /**< blue background */

extern cchar_t ls, rs, ts, bs, tl, tr, bl, br;
#define KEY_ALTF0 0x138
#define KEY_ALTF(n) (KEY_ALTF0 + (n)) /**< define alt function keys */
#define XTERM_256COLOR                /**< use xterm-256color terminfo for altkey bindings */
/**
    EXTENDED NCURSES KEYS
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
#define BW_CHK L'\x2611' /**< left_angle */

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
extern const wchar_t bw_chk; /**< right piointing angle */

extern cchar_t ls, rs, ts, bs, tl, tr, bl, br, lt, rt, sp, ra, la, ua, da, ran, chk;

#define MAXWIN 30 /**< maximum number of windows that can be created */

extern char di_getch();
extern WINDOW *win_win[MAXWIN];  /**< array of pointers to windows */
extern WINDOW *win_win2[MAXWIN]; /**< array of pointers to windows */
extern WINDOW *win_box[MAXWIN];  /**< array of pointers to box windows */
extern PANEL *panel_win[MAXWIN];
extern PANEL *panel_win2[MAXWIN];
extern PANEL *panel_box[MAXWIN];

extern int tty_fd; /**< the file descriptor for the terminal, for error messages
                      and other output */

typedef struct {
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
    char ind_fg[COLOR_LEN];       /**< indicator foreground */
    char ind_bg[COLOR_LEN];       /**< indicator background */
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
    int cp_ind;                  /**< box color pair index */
    int cp_bold;                 /**< bold color pair index */
    int cp_title;                /**< title color pair index */
    int cp_highlight;            /**< highlight color pair index */
    int cp_ln;                   /**< line number color pair index */
    int cp_cmdln;                /**< line number color pair index */
} SIO;
extern void destroy_curses();
extern void apply_gamma(RGB *);
extern int get_clr_pair(int, int);
extern int clr_name_to_idx(char *);
extern int rgb_to_xterm256_idx(RGB *);
extern bool init_clr_palette(SIO *);
extern bool open_curses(SIO *);
extern int rgb_clr_to_cube(int);
extern int rgb_to_curses_clr(RGB *);
extern RGB xterm256_idx_to_rgb(int);
extern size_t mk_cmplx_str(cchar_t *, char *, attr_t, int);
extern size_t str_to_cc(cchar_t *, const char *, attr_t, int, size_t);
extern void display_cmplx_str(WINDOW *, cchar_t *, int, int);
extern int wccp_to_str(wchar_t, uint8_t *);
extern void check_panels(int);
extern int bare_box_new(int, int, int, int, char *);
extern int win2_box_new(int, int, int, int, char *);
extern void resize_panel(PANEL *, int, int, int, int);
extern void win_del();
extern void destroy_win(WINDOW *);
extern void destroy_box(WINDOW *);
extern void restore_wins();
extern void win_init_attrs();
extern void win_Toggle_Attrs();
extern void mvwaddstr_fill(WINDOW *, int, int, char *, int);
extern int display_curses_keys();
extern void init_stdscr();
extern void mouse_getch(int *, int *, int *, int *);
extern void w_mouse_getch(WINDOW *, int *, int *, int *, int *);
#endif
