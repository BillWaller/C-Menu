/* dwin.c
 * window support for MENU
 * Bill Waller
 * billxwaller@gmail.com
 */

#include "menu.h"
#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <wchar.h>

char *eargv[MAXARGS];

int lines = 10, cols = 40, begx = 10, begy = 4;

WINDOW *win;
WINDOW *win_win[MAXWIN];
WINDOW *win_box[MAXWIN];

void open_curses(Init *);
void close_curses();
int win_new(int, int, int, int, char *);
void win_redraw(WINDOW *);
void win_resize(int wlines, int wcols, char *title);
WINDOW *win_open_box(int, int, int, int, char *);
WINDOW *win_open_win(int, int, int, int);
WINDOW *win_del();
void win_close_win(WINDOW *);
void win_close_box(WINDOW *);
void restore_wins();
void dmvwaddstr(WINDOW *, int, int, char *);
void cbox(WINDOW *);
void win_init_attrs(WINDOW *, int, int, int);
int display_o_k_message(char *);
int Perror(char *);
int error_message(char **);
void free_error_message(char **);
void mvwaddstr_fill(WINDOW *, int, int, char *, int);
void display_argv_error_msg(char *, char **);
void abend(int, char *);
void user_end();
int nf_error(int, char *);
void list_colors();
int get_color_number(char *s);
int rgb_clr_to_cube(int);
void set_fkey(int, char *);
bool is_set_fkey(int);
void unset_fkey(int);
void def_clr_pairs();
RGB hex_clr_str_to_rgb(char *);
void init_hex_clr(int, char *);
void color_correction(RGB *);

int src_line;
char *src_name;
char fn[MAXLEN];
char em0[MAXLEN];
char em1[MAXLEN];
char em2[MAXLEN];
char em3[MAXLEN];
char *eargv[MAXARGS];

RGB StdColors[16] = {
    {0, 0, 0},       {128, 0, 0},   {0, 128, 0},   {128, 128, 0},
    {0, 0, 128},     {128, 0, 128}, {0, 128, 128}, {192, 192, 192},
    {128, 128, 128}, {255, 0, 0},   {0, 255, 0},   {255, 255, 0},
    {0, 0, 255},     {255, 0, 255}, {0, 255, 255}, {255, 255, 255}};

RGB xterm256_idx_to_rgb(int);
int rgb_to_xterm256_idx(RGB);
void init_clr_palette(Init *);
void apply_gamma(RGB *);
double rgb_to_linear(double);
double linear_to_rgb(double);
double GRAY_GAMMA = 1.0;
double RED_GAMMA;
double GREEN_GAMMA;
double BLUE_GAMMA;
char tmp_str[MAXLEN];
char *tmp_ptr;
int exit_code;
unsigned int cmd_key;

bool f_sigwench = false;
int win_attr_Odd;
int win_attr_Even;
int win_attr;
int box_attr;
int win_ptr;
int m_lines;
int m_cols;
int m_begy = -1;
int m_begx = -1;
int mouse_support;

enum colors_enum {
    CLR_BLACK = COLOR_BLACK,
    CLR_RED = COLOR_RED,
    CLR_GREEN = COLOR_GREEN,
    CLR_YELLOW = COLOR_YELLOW,
    CLR_BLUE = COLOR_BLUE,
    CLR_MAGENTA = COLOR_MAGENTA,
    CLR_CYAN = COLOR_CYAN,
    CLR_WHITE = COLOR_WHITE,
    CLR_BBLACK,
    CLR_BRED,
    CLR_BGREEN,
    CLR_BYELLOW,
    CLR_BBLUE,
    CLR_BMAGENTA,
    CLR_BCYAN,
    CLR_BWHITE,
    CLR_BORANGE,
    CLR_NCOLORS
};

char const colors_text[][10] = {
    "black",   "red",    "green", "yellow",   "blue",   "magenta", "cyan",
    "white",   "orange", "bg",    "abg",      "bblack", "bred",    "bgreen",
    "byellow", "bblue",  "bcyan", "bmagenta", "bwhite", "borange", ""};

int const ncolors[512] = {
    CLR_BLACK, CLR_RED,      CLR_GREEN,  CLR_YELLOW, CLR_BLUE,   CLR_MAGENTA,
    CLR_CYAN,  CLR_WHITE,    CLR_BBLACK, CLR_BRED,   CLR_BGREEN, CLR_BYELLOW,
    CLR_BBLUE, CLR_BMAGENTA, CLR_BCYAN,  CLR_BWHITE, CLR_NCOLORS};

int cp_default;
int cp_norm;
int cp_box;
int cp_reverse;

int clr_cnt = 0;
int clr_idx = 0;
int clr_pair_idx = 1;
int clr_pair_cnt = 1;

//  ╭───────────────────────────────────────────────────────────────╮
//  │ UNICODE BOX DRAWING CHARACTERS                                │
//  ╰───────────────────────────────────────────────────────────────╯
const wchar_t bw_ho = BW_HO;
const wchar_t bw_ve = BW_VE;
const wchar_t bw_tl = BW_RTL;
const wchar_t bw_tr = BW_RTR;
const wchar_t bw_bl = BW_RBL;
const wchar_t bw_br = BW_RBR;
const wchar_t bw_lt = BW_LT;
const wchar_t bw_rt = BW_RT;
const wchar_t bw_sp = BW_SP;

//  ╭───────────────────────────────────────────────────────────────╮
//  │ WIN_INIT_ATTRS                                                │
//  ╰───────────────────────────────────────────────────────────────╯
void win_init_attrs(WINDOW *win, int fg_color, int bg_color, int bo_color) {
    init_extended_pair(cp_norm, fg_color, bg_color);
    init_extended_pair(cp_box, bo_color, bg_color);
    return;
}
//  ╭───────────────────────────────────────────────────────────────╮
//  │ FKEY_CMD_TBL                                                  │
//  │ if text is "", key is not processed                           │
//  │ F_KEYS 0 - 10 are defined as a convenience                    │
//  ╰───────────────────────────────────────────────────────────────╯
key_cmd_tbl key_cmd[20] = {
    {"", KEY_F(0), 0},
    {"F1 Help", KEY_F(1), 0},
    {"", KEY_F(2), 0},
    {"", KEY_F(3), 0},
    {"F4 Query", KEY_F(4), 0},
    {"F5 Calculate", KEY_F(5), 0},
    {"F6 Edit", KEY_F(6), 0},
    {"", KEY_F(7), 0},
    {"", KEY_F(8), 0},
    {"F9 Cancel", KEY_F(9), 0},
    {"F10 Accept", KEY_F(10), 0},
    {"PgUp", KEY_PPAGE, 0},
    {"PgDn", KEY_NPAGE, 0},
    {"", 0x0, -1},
};

enum key_idx { F0, F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, PgUp, PgDn, END };

int tty_fd, pipe_in, pipe_out;
bool is_set_fkey(int k) {
    if (key_cmd[k].text[0] != '\0')
        return true;
    else
        return false;
}
//  ╭───────────────────────────────────────────────────────────────╮
//  │ SET_FKEY                                                      │
//  ╰───────────────────────────────────────────────────────────────╯
void set_fkey(int k, char *s) {
    if (*s != '\0')
        ssnprintf(key_cmd[k].text, MAXLEN - 1, "F%d %s", k, s);
    else
        key_cmd[k].text[0] = '\0';
}
//  ╭───────────────────────────────────────────────────────────────╮
//  │ SET_FKEY                                                      │
//  ╰───────────────────────────────────────────────────────────────╯
void unset_fkey(int k) { key_cmd[k].text[0] = '\0'; }
int chyron_mk(key_cmd_tbl *ck, char *chyron_s);
int get_chyron_key(key_cmd_tbl *ck, int choice);
//  ╭───────────────────────────────────────────────────────────────╮
//  │ CHYRON_MK                                                     │
//  ╰───────────────────────────────────────────────────────────────╯
int chyron_mk(key_cmd_tbl *fc, char *s) {
    int end_pos = 0;
    int i = 0;
    *s = '\0';
    while (fc[i].end_pos != -1) {
        if (fc[i].text[0] == '\0') {
            i++;
            continue;
        }
        if (end_pos == 0)
            strncat(s, " ", MAXLEN - strlen(s) - 1);
        else
            strncat(s, " | ", MAXLEN - strlen(s) - 1);
        strncat(s, fc[i].text, MAXLEN - strlen(s) - 1);
        end_pos = strlen(s) + 1;
        fc[i].end_pos = end_pos;
        i++;
    }
    if (end_pos > 0)
        strncat(s, " ", MAXLEN - strlen(s) - 1);
    return strlen(s);
}
//  ╭───────────────────────────────────────────────────────────────╮
//  │ GET_CHYRON_KEY                                                │
//  ╰───────────────────────────────────────────────────────────────╯
int get_chyron_key(key_cmd_tbl *fc, int x) {
    int i;
    for (i = 0; fc[i].end_pos != -1; i++)
        if (x < fc[i].end_pos)
            break;
    return fc[i].keycode;
}
//  ╭───────────────────────────────────────────────────────────────╮
//  │ OPEN_CURSES                                                   │
//  ╰───────────────────────────────────────────────────────────────╯
void open_curses(Init *init) {
    char tty_name[256];
    char tmp_str[MAXLEN];
    char emsg0[MAXLEN];

    if (ttyname_r(STDERR_FILENO, tty_name, sizeof(tty_name)) != 0) {
        strerror_r(errno, tmp_str, MAXLEN - 1);
        strcpy(emsg0, "ttyname_r failed ");
        strcat(emsg0, tmp_str);
        fprintf(stderr, "%s\n", tmp_str);
        exit(0);
    }
    init->stdin_fd = dup(STDIN_FILENO);
    init->stdout_fd = dup(STDOUT_FILENO);
    // Open the terminal device for reading and writing
    init->tty_fp = fopen(tty_name, "r+");
    if (init->tty_fp == NULL) {
        strerror_r(errno, tmp_str, MAXLEN - 1);
        strcpy(emsg0, "fopen(tty_name) failed ");
        strcat(emsg0, tmp_str);
        fprintf(stderr, "%s\n", tmp_str);
        exit(0);
    }
    // Attach the terminal descriptor to the STDERR_FILENO
    dup2(fileno(init->tty_fp), STDERR_FILENO);
    SCREEN *screen = newterm(NULL, init->tty_fp, init->tty_fp);
    //  ╭───────────────────────────────────────────────────────────────╮
    //  │ Beyond this point, NCurses has the con.                       │
    //  ╰───────────────────────────────────────────────────────────────╯
    if (screen == NULL) {
        strerror_r(errno, tmp_str, MAXLEN - 1);
        strcpy(emsg0, "newterm failed ");
        strcat(emsg0, tmp_str);
        fprintf(stderr, "%s\n", tmp_str);
        exit(0);
    }
    set_term(screen);

    f_curses_open = true;
    if (!has_colors()) {
        close_curses();
        abend(-1, "terminal color support required");
    }
    start_color();
    init_clr_palette(init);
    RED_GAMMA = init->red_gamma;
    GREEN_GAMMA = init->green_gamma;
    BLUE_GAMMA = init->blue_gamma;
    cp_norm = get_clr_pair(init->fg_color, init->bg_color);
    cp_reverse = get_clr_pair(init->bg_color, init->fg_color);
    cp_box = get_clr_pair(init->bo_color, init->bg_color);
    wcolor_set(stdscr, cp_norm, NULL);
    // immedok(stdscr, true);
    noecho();
    cbreak();
    keypad(stdscr, true);
    idlok(stdscr, false);
    idcok(stdscr, false);
}
//  ╭───────────────────────────────────────────────────────────────╮
//  │ GET_CLR_PAIR                                                  │
//  ╰───────────────────────────────────────────────────────────────╯
int get_clr_pair(int fg, int bg) {
    int rc, i, pfg, pbg;
    for (i = 0; i < clr_pair_cnt; i++) {
        extended_pair_content(i, &pfg, &pbg);
        if (pfg == fg && pbg == bg)
            return i;
    }
    if (i < MAX_COLOR_PAIRS - 1) {
        rc = init_extended_pair(i, fg, bg);
        if (rc == ERR)
            return ERR;
    }
    if (i < MAX_COLOR_PAIRS)
        clr_pair_cnt++;
    return i;
}
//  ╭───────────────────────────────────────────────────────────────╮
//  │ GET_CLR                                                       │
//  ╰───────────────────────────────────────────────────────────────╯
int get_clr(RGB rgb) {
    int i;
    int r, g, b;

    r = (rgb.r * 1000) / 255;
    g = (rgb.g * 1000) / 255;
    b = (rgb.b * 1000) / 255;
    for (i = 0; i < clr_cnt; i++) {
        extended_color_content(i, &r, &g, &b);
        if (rgb.r == r && rgb.g == g && rgb.b == b) {
            return i;
        }
    }
    if (i < COLORS) {
        init_extended_color(i, r, g, b);
        clr_cnt++;
        return clr_cnt - 1;
    }
    return ERR;
}
//  ╭───────────────────────────────────────────────────────────────╮
//  │ RGB_TO_XTERM256_IDX                                           │
//  ╰───────────────────────────────────────────────────────────────╯
int rgb_to_xterm256_idx(RGB rgb) {
    if (rgb.r == rgb.g && rgb.g == rgb.b) {
        if (rgb.r < 8)
            return 16;
        if (rgb.r > 248)
            return 231;
        return ((rgb.r - 8) / 10) + 232;
    } else {
        int r_index = (rgb.r < 45) ? 0 : (rgb.r - 60) / 40 + 1;
        int g_index = (rgb.g < 45) ? 0 : (rgb.g - 60) / 40 + 1;
        int b_index = (rgb.b < 45) ? 0 : (rgb.b - 60) / 40 + 1;
        return 16 + (36 * r_index) + (6 * g_index) + b_index;
    }
}
//  ╭───────────────────────────────────────────────────────────────╮
//  │ XTERM256_IDX_TO_RGB                                           │
//  ╰───────────────────────────────────────────────────────────────╯
RGB xterm256_idx_to_rgb(int code) {
    RGB rgb;
    if (code < 16) {
        rgb.r = StdColors[code].r;
        rgb.g = StdColors[code].g;
        rgb.b = StdColors[code].b;
    } else if (code >= 16 && code <= 231) {
        code -= 16;
        rgb.r = (code / 36) % 6 * 51;
        rgb.g = (code / 6) % 6 * 51;
        rgb.b = (code % 6) * 51;
    } else if (code >= 232 && code <= 255) {
        int gray = (code - 232) * 11;
        rgb.r = rgb.g = rgb.b = gray;
    }
    return rgb;
}
//  ╭───────────────────────────────────────────────────────────────╮
//  │ APPLY_GAMMA                                                   │
//  ╰───────────────────────────────────────────────────────────────╯
void apply_gamma(RGB *rgb) {
    if (rgb->r == rgb->g && rgb->r == rgb->b) {
        if (GRAY_GAMMA > 0.0f && GRAY_GAMMA != 1.0f) {
            rgb->r = (int)(pow((rgb->r / 255.0f), 1.0f / GRAY_GAMMA) * 255.0f);
            rgb->g = rgb->r;
            rgb->b = rgb->r;
        }
        return;
    }
    if (rgb->r != 0 && RED_GAMMA > 0.0f && RED_GAMMA != 1.0f)
        rgb->r = (int)(pow((rgb->r / 255.0f), 1.0f / RED_GAMMA) * 255.0f);
    if (rgb->g != 0 && GREEN_GAMMA > 0.0f && GREEN_GAMMA != 1.0f)
        rgb->g = (int)(pow((rgb->g / 255.0f), 1.0f / GREEN_GAMMA) * 255.0f);
    if (rgb->b != 0 && BLUE_GAMMA > 0.0f && BLUE_GAMMA != 1.0f)
        rgb->b = (int)(pow((rgb->b / 255.0f), 1.0f / BLUE_GAMMA) * 255.0f);
}
//  ╭───────────────────────────────────────────────────────────────╮
//  │ INIT_CLR_PALETTE                                              │
//  ╰───────────────────────────────────────────────────────────────╯
void init_clr_palette(Init *init) {
    int i;
    int rr, gg, bb;
    RGB rgb;

    for (i = 0; i < 256; i++) {
        rgb = xterm256_idx_to_rgb(i);
        rr = (rgb.r * 1000) / 255;
        gg = (rgb.g * 1000) / 255;
        bb = (rgb.b * 1000) / 255;
        init_extended_color(i, rr, gg, bb);
    }
    //  ╭───────────────────────────────────────────────────────────╮
    //  │ C-Menu colors override the standard palette               │
    //  ╰───────────────────────────────────────────────────────────╯
    if (init->black[0])
        init_hex_clr(CLR_BLACK, init->black);
    if (init->red[0])
        init_hex_clr(CLR_RED, init->red);
    if (init->green[0])
        init_hex_clr(CLR_GREEN, init->green);
    if (init->yellow[0])
        init_hex_clr(CLR_YELLOW, init->yellow);
    if (init->blue[0])
        init_hex_clr(CLR_BLUE, init->blue);
    if (init->magenta[0])
        init_hex_clr(CLR_MAGENTA, init->magenta);
    if (init->cyan[0])
        init_hex_clr(CLR_CYAN, init->cyan);
    if (init->white[0])
        init_hex_clr(CLR_WHITE, init->white);
    if (init->bblack[0])
        init_hex_clr(CLR_BBLACK, init->bblack);
    if (init->bred[0])
        init_hex_clr(CLR_BRED, init->bred);
    if (init->bgreen[0])
        init_hex_clr(CLR_BGREEN, init->bgreen);
    if (init->byellow[0])
        init_hex_clr(CLR_BYELLOW, init->byellow);
    if (init->bblue[0])
        init_hex_clr(CLR_BBLUE, init->bblue);
    if (init->bmagenta[0])
        init_hex_clr(CLR_BMAGENTA, init->bmagenta);
    if (init->bcyan[0])
        init_hex_clr(CLR_BCYAN, init->bcyan);
    if (init->bwhite[0])
        init_hex_clr(CLR_BWHITE, init->bwhite);
}
//  ╭───────────────────────────────────────────────────────────────╮
//  │ INIT_HEX_CLR                                                  │
//  │ Convert #ffffff to NCurses rgb                                │
//  ╰───────────────────────────────────────────────────────────────╯
void init_hex_clr(int idx, char *s) {
    RGB rgb;
    rgb = hex_clr_str_to_rgb(s);
    if (idx < 16) {
        StdColors[idx].r = rgb.r;
        StdColors[idx].g = rgb.g;
        StdColors[idx].g = rgb.g;
    }
    rgb.r = (rgb.r * 1000) / 255;
    rgb.g = (rgb.g * 1000) / 255;
    rgb.b = (rgb.b * 1000) / 255;
    init_extended_color(idx, rgb.r, rgb.g, rgb.b);
}
//  ╭───────────────────────────────────────────────────────────────╮
//  │ HEX_CLR_STR_TO_RGB                                            │
//  ╰───────────────────────────────────────────────────────────────╯
RGB hex_clr_str_to_rgb(char *s) {
    RGB rgb;
    sscanf(s, "#%02x%02x%02x", &rgb.r, &rgb.g, &rgb.b);
    return rgb;
}
//  ╭───────────────────────────────────────────────────────────────╮
//  │ CLOSE_CURSES                                                  │
//  ╰───────────────────────────────────────────────────────────────╯
void close_curses() {
    if (f_curses_open) {
        wclear(stdscr);
        wrefresh(stdscr);
        endwin();
        f_curses_open = false;
    }
    restore_shell_tioctl();
    sig_dfl_mode();
}
//  ╭───────────────────────────────────────────────────────────────╮
//  │ WIN_NEW                                                       │
//  ╰───────────────────────────────────────────────────────────────╯
int win_new(int wlines, int wcols, int wbegy, int wbegx, char *WTitle) {
    int maxx;
    if (win_ptr < MAXWIN) {
        if (win_ptr > 0)
            wrefresh(win_win[win_ptr]);
        else
            wrefresh(stdscr);
        win_ptr++;
        if (wbegy != 0 || wbegx != 0 || wlines < LINES - 2 ||
            wcols < COLS - 2) {
            win_box[win_ptr] = newwin(wlines + 2, wcols + 2, wbegy, wbegx);
            if (win_box[win_ptr] == NULL)
                return (1);
            wbkgd(win_box[win_ptr], COLOR_PAIR(cp_box) | ' ');
            cbox(win_box[win_ptr]);
            if (WTitle != NULL && *WTitle != '\0') {
                wmove(win_box[win_ptr], 0, 1);
                waddnstr(win_box[win_ptr], (const char *)&bw_rt, 1);
                wmove(win_box[win_ptr], 0, 2);
                waddnstr(win_box[win_ptr], (const char *)&bw_sp, 1);
                mvwaddnwstr(win_box[win_ptr], 0, 1, &bw_rt, 1);
                mvwaddnwstr(win_box[win_ptr], 0, 2, &bw_sp, 1);
                mvwaddstr(win_box[win_ptr], 0, 3, WTitle);
                maxx = getmaxx(win_box[win_ptr]);
                int s = strlen(WTitle);
                if ((s + 3) < maxx)
                    mvwaddch(win_box[win_ptr], 0, (s + 3), ' ');
                if ((s + 4) < maxx)
                    mvwaddnwstr(win_box[win_ptr], 0, (s + 4), &bw_lt, 1);
            }
            wnoutrefresh(win_box[win_ptr]);

            win_win[win_ptr] = newwin(wlines, wcols, wbegy + 1, wbegx + 1);
            wbkgd(win_win[win_ptr], COLOR_PAIR(cp_norm) | ' ');
        } else {
            win_box[win_ptr] = newwin(wlines, wcols, wbegy, wbegx);
            if (win_box[win_ptr] == NULL)
                return (1);
            wbkgd(win_box[win_ptr], COLOR_PAIR(cp_box) | ' ');
            win_win[win_ptr] = newwin(wlines, wcols, wbegy, wbegx);
            wbkgd(win_win[win_ptr], COLOR_PAIR(cp_norm) | ' ');
        }
        if (win_win[win_ptr] == NULL)
            return (1);
        keypad(win_win[win_ptr], TRUE);
        idlok(win_win[win_ptr], false);
        idcok(win_win[win_ptr], false);
    }
    // immedok(win_win[win_ptr], true);
    return (0);
}
void win_resize(int wlines, int wcols, char *title) {
    int maxx;

    wrefresh(stdscr);
    wresize(win_box[win_ptr], wlines + 2, wcols + 2);
    wbkgd(win_box[win_ptr], COLOR_PAIR(cp_box) | ' ');
    cbox(win_box[win_ptr]);
    if (title != NULL && *title != '\0') {
        wmove(win_box[win_ptr], 0, 1);
        waddnstr(win_box[win_ptr], (const char *)&bw_rt, 1);
        wmove(win_box[win_ptr], 0, 2);
        waddnstr(win_box[win_ptr], (const char *)&bw_sp, 1);
        mvwaddnwstr(win_box[win_ptr], 0, 1, &bw_rt, 1);
        mvwaddnwstr(win_box[win_ptr], 0, 2, &bw_sp, 1);
        mvwaddstr(win_box[win_ptr], 0, 3, title);
        maxx = getmaxx(win_box[win_ptr]);
        int s = strlen(title);
        if ((s + 3) < maxx)
            mvwaddch(win_box[win_ptr], 0, (s + 3), ' ');
        if ((s + 4) < maxx)
            mvwaddnwstr(win_box[win_ptr], 0, (s + 4), &bw_lt, 1);
    }
    wnoutrefresh(win_box[win_ptr]);
    wresize(win_win[win_ptr], wlines, wcols);

    wbkgd(win_win[win_ptr], COLOR_PAIR(cp_norm) | ' ');
    wsetscrreg(win_win[win_ptr], 0, wlines - 1);
    keypad(win_win[win_ptr], TRUE);
    idlok(win_win[win_ptr], false);
    idcok(win_win[win_ptr], false);
}

//  ╭───────────────────────────────────────────────────────────────╮
//  │ WIN_REDRAW                                                    │
//  ╰───────────────────────────────────────────────────────────────╯
void win_redraw(WINDOW *win) {
    werase(win);
    wnoutrefresh(win);
}
//  ╭───────────────────────────────────────────────────────────────╮
//  │ WIN_DEL                                                       │
//  ╰───────────────────────────────────────────────────────────────╯
WINDOW *win_del() {
    int i;

    if (win_ptr > 0) {
        wclear(win_win[win_ptr]);
        touchwin(win_win[win_ptr]);
        wnoutrefresh(win_win[win_ptr]);
        delwin(win_win[win_ptr]);
        wclear(win_box[win_ptr]);
        touchwin(win_box[win_ptr]);
        wnoutrefresh(win_box[win_ptr]);
        delwin(win_box[win_ptr]);
        touchwin(stdscr);
        wnoutrefresh(stdscr);
        for (i = 0; i < win_ptr; i++) {
            touchwin(win_box[i]);
            wnoutrefresh(win_box[i]);
            touchwin(win_win[i]);
            wnoutrefresh(win_win[i]);
        }
        win_ptr--;
    }
    return (0);
}
//  ╭───────────────────────────────────────────────────────────────╮
//  │ RESTORE_WINS                                                  │
//  ╰───────────────────────────────────────────────────────────────╯
void restore_wins() {
    int i;

    wclear(stdscr);
    touchwin(stdscr);
    wnoutrefresh(stdscr);
    for (i = 0; i <= win_ptr; i++) {
        touchwin(win_box[i]);
        wnoutrefresh(win_box[i]);
        touchwin(win_win[i]);
        wnoutrefresh(win_win[i]);
    }
}
//  ╭───────────────────────────────────────────────────────────────╮
//  │ CBOX                                                          │
//  ╰───────────────────────────────────────────────────────────────╯
void cbox(WINDOW *box) {
    int x, y;
    int maxx;
    int maxy;

    maxx = getmaxx(box);
    maxx--;
    mvwaddnwstr(box, 0, 0, &bw_tl, 1);
    for (x = 1; x < maxx; x++)
        waddnwstr(box, &bw_ho, 1);
    waddnwstr(box, &bw_tr, 1);
    maxy = getmaxy(box);
    maxy--;
    for (y = 1; y < maxy; y++) {
        mvwaddnwstr(box, y, 0, &bw_ve, 1);
        mvwaddnwstr(box, y, maxx, &bw_ve, 1);
    }
    mvwaddnwstr(box, maxy, 0, &bw_bl, 1);
    for (x = 1; x < maxx; x++)
        waddnwstr(box, &bw_ho, 1);
    waddnwstr(box, &bw_br, 1);
}
//  ╭───────────────────────────────────────────────────────────────╮
//  │ ERROR_MESSAGE                                                 │
//  ╰───────────────────────────────────────────────────────────────╯
int error_message(char **argv) {
    const int msg_max_len = 71;
    WINDOW *error_win;
    int len = 0, line, pos;
    int argc, i;
    char msg[72];
    char title[64];
    unsigned cmd_key;
    char c;

    argc = 0;
    while (argv[argc] && *argv[argc] != '\0') {
        len = strlen(argv[argc]);
        if (len > cols)
            cols = len;
        argc++;
    }
    lines = argc + 1;
    if (f_curses_open) {
        if (cols > msg_max_len - 4)
            cols = msg_max_len - 4;
        pos = ((COLS - len) - 4) / 2;
        line = (LINES - 4) / 2;

        strncpy(title, "Error", msg_max_len - 7);
        if (win_new(lines, cols + 2, line, pos, title)) {
            sprintf(tmp_str, "win_new(%d, %d, %d) failed", cols, line, pos);
            abend(-1, tmp_str);
        }
        error_win = win_win[win_ptr];
        i = 0;
        while (i < argc) {
            mvwaddstr(error_win, i, 1, argv[i]);
            i++;
        }
        wattron(error_win, A_REVERSE);
        mvwaddstr(error_win, i, 1,
                  " Type \"X\" to exit or any other key to continue ");
        wattroff(error_win, A_REVERSE);
        wrefresh(error_win);
        cmd_key = wgetch(error_win);
        c = (char)cmd_key;
        to_uppercase(c);
        if (c == 'X') {
            exit_code = -1;
            abend(-1, "terminated by user");
        }
        win_del();
    } else {
        i = 0;
        while (i++ < argc) {
            if (argv[i] == NULL)
                break;
            strncpy(msg, argv[i], msg_max_len - 1);
            fprintf(stderr, "%s\n", msg);
        }
        cmd_key = di_getch();
        if (cmd_key == 'X' || cmd_key == 'x') {
            exit_code = -1;
            abend(-1, "menu terminated by user");
        }
    }
    return (cmd_key);
}
//  ╭───────────────────────────────────────────────────────────────╮
//  │ DISPLAY_ERROR                                                 │
//  │     em0 - source file and line number (dwin.c 736)            │
//  │     em1 - predicate and subject (open file-name)              │
//  │     em2 - explanation (could not open file)                   │
//  │     to be added:                                              │
//  │     em3 - hint (check permissions)                            │
//  ╰───────────────────────────────────────────────────────────────╯
int display_error(char *em0, char *em1, char *em2, char *em3) {
    char title[64];
    WINDOW *error_win;
    int line, pos, em_l, em0_l, em1_l, em2_l, em3_l, cmd_l;
    char cmd[] = " F1 Help | F9 Cancel | F10 Continue ";

    if (!f_curses_open) {
        fprintf(stderr, "\n\n%s\n%s\n%s\n%s\n\n", em0, em1, em2, em3);
        return (1);
    }
    em_l = 0;
    cmd_l = strlen(cmd);
    em0_l = strlen(em0);
    em1_l = strlen(em1);
    em2_l = strlen(em2);
    em3_l = strlen(em1);
    if (em0_l > em_l)
        em_l = em0_l;
    if (em1_l > em_l)
        em_l = em1_l;
    if (em2_l > em_l)
        em_l = em2_l;
    if (em3_l > em_l)
        em_l = em3_l;
    if (em_l < cmd_l)
        em_l = cmd_l;
    pos = (COLS - em_l - 4) / 2;
    line = (LINES - 6) / 2;
    strcpy(title, "Notification");
    if (win_new(5, em_l + 2, line, pos, title)) {
        sprintf(title, "win_new(%d, %d, %d, %d) failed", 5, em_l + 2, line,
                pos);
        abend(-1, title);
    }
    error_win = win_win[win_ptr];
    mvwaddstr(error_win, 0, 1, em0);
    mvwaddstr(error_win, 1, 1, em1);
    mvwaddstr(error_win, 2, 1, em2);
    mvwaddstr(error_win, 3, 1, em3);
    wattron(error_win, A_REVERSE);
    mvwaddstr(error_win, 4, 1, cmd);
    wattroff(error_win, A_REVERSE);
    wmove(error_win, 4, cmd_l + 1);
    wrefresh(error_win);
    cmd_key = wgetch(error_win);
    switch (cmd_key) {
    case KEY_F(1):
        break;
    case KEY_F(9):
        break;
    case KEY_F(10):
        break;
    default:
        break;
    }
    win_del();
    return (cmd_key);
}
//  ╭───────────────────────────────────────────────────────────────╮
//  │ PERROR - simple one-line error                                │
//  ╰───────────────────────────────────────────────────────────────╯
int Perror(char *emsg_str) {
    char emsg[80];
    int emsg_max_len = 80;
    unsigned cmd_key;
    WINDOW *error_win;
    int len, line, pos;
    char title[64];

    strncpy(emsg, emsg_str, emsg_max_len - 1);
    len = strlen(emsg);
    if (!f_curses_open) {
        fprintf(stderr, "\n%s\n", emsg);
        return (1);
    }
    pos = (COLS - len - 4) / 2;
    line = (LINES - 4) / 2;
    if (len < 39)
        len = 39;
    strcpy(title, "Notification");
    if (win_new(2, len + 2, line, pos, title)) {
        sprintf(title, "win_new(%d, %d, %d, %d) failed", 4, line, line, pos);
        abend(-1, title);
    }
    error_win = win_win[win_ptr];
    mvwaddstr(error_win, 0, 1, emsg);
    wattron(error_win, A_REVERSE);
    mvwaddstr(error_win, 1, 0, " F9 Cancel | Any other key to continue ");
    wattroff(error_win, A_REVERSE);
    wmove(error_win, 1, 27);
    wrefresh(error_win);
    cmd_key = wgetch(error_win);
    win_del();
    return (cmd_key);
}
//  ╭───────────────────────────────────────────────────────────────╮
//  │ MVWADDSTR_FILL                                                │
//  ╰───────────────────────────────────────────────────────────────╯
void mvwaddstr_fill(WINDOW *w, int y, int x, char *s, int l) {
    char *d, *e;
    char tmp_str[MAXLEN];

    d = tmp_str;
    if (l > MAXLEN - 1)
        l = MAXLEN - 1;
    e = tmp_str + l;
    while (d < e)
        if (*s == '\0' || *s == '\n')
            *d++ = ' ';
        else
            *d++ = *s++;
    *d++ = '\0';
    mvwaddstr(w, y, x, tmp_str);
}
//  ╭───────────────────────────────────────────────────────────────╮
//  │ GET_COLOR_NUMBER                                              │
//  ╰───────────────────────────────────────────────────────────────╯
int get_color_number(char *s) {
    int i = 0;
    int n = NCOLORS;

    str_to_lower(s);
    while (i < n) {
        if (!strcmp(colors_text[i], s))
            break;
        i++;
    }
    if (i >= n)
        return (-1);
    return (i);
}
//  ╭───────────────────────────────────────────────────────────────╮
//  │ LIST_COLORS                                                   │
//  ╰───────────────────────────────────────────────────────────────╯
void list_colors() {
    int i, col;

    for (i = 0, col = 0; i < NCOLORS; i++, col++) {
        if (i < 8) {
            fprintf(stderr, " ");
        }
        if (i == 8) {
            col = 0;
            fprintf(stderr, "\n");
        } else if (col > 0)
            fprintf(stderr, " ");
        fprintf(stderr, "%s", colors_text[i]);
    }
    fprintf(stderr, "\n");
}
//  ╭───────────────────────────────────────────────────────────────╮
//  │ DISPLAY_ARGV_ERROR_MSG                                        │
//  ╰───────────────────────────────────────────────────────────────╯
void display_argv_error_msg(char *emsg, char **argv) {
    int argc;

    argc = 0;
    fprintf(stderr, "\r\n");
    while (*argv != NULL && **argv != '\0')
        fprintf(stderr, "argv[%d] - %s\r\n", argc++, *argv++);
    fprintf(stderr, "%s\r\n", emsg);
    fprintf(stderr, "%s", "Press any key to continue");
    wrefresh(stdscr);
    wgetch(stdscr);
}
//  ╭───────────────────────────────────────────────────────────────╮
//  │ NF_ERROR                                                      │
//  ╰───────────────────────────────────────────────────────────────╯
int nf_error(int ec, char *s) {
    fprintf(stderr, "ERROR: %s code: %d\n", s, ec);
    fprintf(stderr, "Press a key to continue");
    di_getch();
    fprintf(stderr, "\n");
    return ec;
}
//  ╭───────────────────────────────────────────────────────────────╮
//  │ USER_END                                                      │
//  ╰───────────────────────────────────────────────────────────────╯
void user_end() {
    close_curses();
    restore_shell_tioctl();
    sig_dfl_mode();
    fprintf(stderr, "Normal program exit");
    fprintf(stderr, "\n");
    exit(EXIT_SUCCESS);
}
//  ╭───────────────────────────────────────────────────────────────╮
//  │ ABEND                                                         │
//  ╰───────────────────────────────────────────────────────────────╯
void abend(int ec, char *s) {
    close_curses();
    sig_dfl_mode();
    fprintf(stderr, "\n\nABEND: %s code: %d\n", s, ec);
    fprintf(stderr, "Press a key to exit program");
    di_getch();
    close_init(init);
    win_del();
    close_curses();
    restore_shell_tioctl();
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}
