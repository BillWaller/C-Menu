/* dwin.c
 * window support for MENU
 * Bill Waller
 * billxwaller@gmail.com
 */

#include "menu.h"
#include <stdlib.h>
#include <string.h>

char *eargv[MAXARGS];

int lines = 10, cols = 40, begx = 10, begy = 4;

WINDOW *win;
WINDOW *win_win[MAXWIN];
WINDOW *win_box[MAXWIN];

void open_curses(Init *);
void close_curses();
int win_new(int, int, int, int, char *);
void win_redraw(WINDOW *, int, char *);
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
int display_error_message(char *);
int error_message(char **);
void free_error_message(char **);
void mvwaddstr_fill(WINDOW *, int, int, char *, int);
void list_colors();
int get_color_number(char *s);
void display_argv_error_msg(char *, char **);
void abend(int, char *);
void user_end();
int nf_error(int, char *);
void set_color(int color, char *color_str);

char tmp_str[MAXLEN];
char *tmp_ptr;
int exit_code;
unsigned int cmd_key;

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
    CLR_ORANGE,
    CLR_BBLACK,
    CLR_BRED,
    CLR_BGREEN,
    CLR_BYELLOW,
    CLR_BBLUE,
    CLR_BMAGENTA,
    CLR_BCYAN,
    CLR_BWHITE,
    CLR_BORANGE,
    CLR_BG,
    CLR_ABG,
    CLR_NCOLORS
};

char const colors_text[][10] = {
    "black",   "red",    "green", "yellow",   "blue",   "magenta", "cyan",
    "white",   "orange", "bg",    "abg",      "bblack", "bred",    "bgreen",
    "byellow", "bblue",  "bcyan", "bmagenta", "bwhite", "borange", ""};

int const ncolors[] = {
    CLR_BLACK,
    CLR_RED,
    CLR_GREEN,
    CLR_YELLOW,
    CLR_BLUE,
    CLR_MAGENTA,
    CLR_CYAN,
    CLR_WHITE,
    CLR_ORANGE,
    CLR_BG,
    CLR_ABG,
    CLR_BBLACK | A_BOLD,
    CLR_BRED | A_BOLD,
    CLR_BGREEN | A_BOLD,
    CLR_BYELLOW | A_BOLD,
    CLR_BBLUE | A_BOLD,
    CLR_BMAGENTA | A_BOLD,
    CLR_BCYAN | A_BOLD,
    CLR_BWHITE | A_BOLD,
    CLR_BORANGE | A_BOLD,
    0,
};

const wchar_t bw_ho = BW_HO;
const wchar_t bw_ve = BW_VE;
const wchar_t bw_tl = BW_RTL;
const wchar_t bw_tr = BW_RTR;
const wchar_t bw_bl = BW_RBL;
const wchar_t bw_br = BW_RBR;
const wchar_t bw_lt = BW_LT;
const wchar_t bw_rt = BW_RT;
const wchar_t bw_sp = BW_SP;

void set_color(int color_n, char *s);

// CP_DEFAULT,
// CP_NORM,
// CP_REVERSE,
// CP_BOX,
// CP_BOLD,
// CP_TITLE,
// CP_HIGHLIGHT,
// CP_NPAIRS

void win_init_attrs(WINDOW *win, int fg_color, int bg_color, int bo_color) {
    // init_pair(CP_NORM, fg_color, bg_color);
    // init_pair(CP_BOX, bo_color, bg_color);
    // wcolor_set(win, CP_NORM, NULL);
    return;
}

void open_curses(Init *init) {
    initscr();
    f_curses_open = true;
    clear();
    nonl();
    noecho();
    cbreak();
    // meta(stdscr, true);
    keypad(stdscr, true);
    clearok(stdscr, false);
    scrollok(stdscr, true);
    // immedok(stdscr, TRUE);
    if (!has_colors()) {
        close_curses();
        abend(-1, "terminal color support required");
    }
    start_color();
    set_color(CLR_BLACK, init->black);
    set_color(CLR_RED, init->red);
    set_color(CLR_GREEN, init->green);
    set_color(CLR_YELLOW, init->yellow);
    set_color(CLR_BLUE, init->blue);
    set_color(CLR_MAGENTA, init->magenta);
    set_color(CLR_CYAN, init->cyan);
    set_color(CLR_WHITE, init->white);
    set_color(CLR_ORANGE, init->orange);
    set_color(CLR_BG, init->bg);
    init_pair(CP_DEFAULT, init->fg_color, CLR_BLACK);
    init_pair(CP_NORM, init->fg_color, CLR_BG);
    init_pair(CP_BOX, init->bo_color, CLR_BG);
    init_pair(CP_REVERSE, init->bg_color, init->fg_color);
    init_pair(CP_TITLE, init->bo_color, init->bg_color);
    init_pair(CP_HIGHLIGHT, init->bg_color, init->fg_color);
    // bkgd(COLOR_PAIR(CP_NORM) | ' ');
    // attr_on(COLOR_PAIR(CP_NORM), NULL);
    box_attr = COLOR_PAIR(CP_BOX);
    win_attr = COLOR_PAIR(CP_NORM);
    wcolor_set(stdscr, CP_NORM, NULL);
}

void set_color(int color_n, char *s) {
    char rx[3], gx[3], bx[3];
    unsigned int r, g, b;
    rx[0] = s[1];
    rx[1] = s[2];
    rx[2] = '\0';
    gx[0] = s[3];
    gx[1] = s[4];
    gx[2] = '\0';
    bx[0] = s[5];
    bx[1] = s[6];
    bx[2] = '\0';
    sscanf(rx, "%x", &r);
    sscanf(gx, "%x", &g);
    sscanf(bx, "%x", &b);
    r = r * 1000 / 255;
    g = g * 1000 / 255;
    b = b * 1000 / 255;
    init_color(color_n, r, g, b);
}

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
                return (-1);
            wbkgd(win_box[win_ptr], COLOR_PAIR(CP_BOX) | ' ');
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
            wbkgd(win_win[win_ptr], COLOR_PAIR(CP_NORM) | ' ');
        } else {
            win_box[win_ptr] = newwin(wlines, wcols, wbegy, wbegx);
            if (win_box[win_ptr] == NULL)
                return (-1);
            wbkgd(win_box[win_ptr], COLOR_PAIR(CP_BOX) | ' ');
            win_win[win_ptr] = newwin(wlines, wcols, wbegy, wbegx);
            wbkgd(win_win[win_ptr], COLOR_PAIR(CP_NORM) | ' ');
        }
        if (win_win[win_ptr] == NULL)
            return (-1);
    }
    return (0);
}

void win_redraw(WINDOW *win, int Wattr, char *WTitle) {
    werase(win);
    wnoutrefresh(win);
}

WINDOW *win_del() {
    int i;

    if (win_ptr > 0) {
        delwin(win_win[win_ptr]);
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

void dmvwaddstr(WINDOW *win, int y, int x, char *s) {
    wmove(win, y, x);
    while (*s != '\0') {
        if (*(s + 1) == '\b') {
            if (*(s + 2) == *s) {
                s += 2;
                wattr_on(win, A_BOLD, NULL);
                waddch(win, *s++);
                wattr_off(win, A_BOLD, NULL);
            }
        } else
            waddch(win, *s++);
    }
}

void cbox(WINDOW *win) {
    int x, y;
    int maxx;
    int maxy;

    maxx = getmaxx(win);
    maxx--;
    mvwaddnwstr(win, 0, 0, &bw_tl, 1);
    for (x = 1; x < maxx; x++)
        waddnwstr(win, &bw_ho, 1);
    waddnwstr(win, &bw_tr, 1);
    maxy = getmaxy(win);
    maxy--;
    for (y = 1; y < maxy; y++) {
        mvwaddnwstr(win, y, 0, &bw_ve, 1);
        mvwaddnwstr(win, y, maxx, &bw_ve, 1);
    }
    mvwaddnwstr(win, maxy, 0, &bw_bl, 1);
    for (x = 1; x < maxx; x++)
        waddnwstr(win, &bw_ho, 1);
    waddnwstr(win, &bw_br, 1);
}

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
        // wmove(error_win, i - 1, len + 1);
        // touchwin(error_win);
        // wnoutrefresh(error_win);
        wrefresh(error_win);
        cmd_key = wgetch(error_win);
        c = (char)cmd_key;
        to_uppercase(c);
        if (c == 'X') {
            exit_code = -1;
            abend(-1, "menu terminated by user");
        }
        win_del();
    } else {
        i = 0;
        while (i++ < argc) {
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

int display_error(char *emsg0, char *emsg1, char *emsg2) {
    char title[64];
    WINDOW *error_win;
    int line, pos, emsg0_l, emsg1_l, emsg_l;

    if (!f_curses_open) {
        fprintf(stderr, "\n\n%s\n%s\n%s\n\n", emsg0, emsg1, emsg2);
        return (1);
    }
    emsg0_l = strlen(emsg0);
    emsg1_l = strlen(emsg1);
    emsg_l = strlen(emsg2);
    if (emsg0_l > emsg_l)
        emsg_l = emsg0_l;
    if (emsg1_l > emsg_l)
        emsg_l = emsg1_l;
    if (emsg_l < 26)
        emsg_l = 26;
    pos = (COLS - emsg_l - 4) / 2;
    line = (LINES - 5) / 2;

    strcpy(title, "Notification");
    if (win_new(3, emsg_l + 2, line, pos, title)) {
        sprintf(title, "win_new(%d, %d, %d, %d) failed", 4, line, line, pos);
        abend(-1, title);
    }
    error_win = win_win[win_ptr];
    mvwaddstr(error_win, 0, 1, emsg0);
    mvwaddstr(error_win, 1, 1, emsg1);
    mvwaddstr(error_win, 1, 1, emsg2);
    mvwaddstr(error_win, 2, 1, "Press any key to continue");
    wmove(error_win, 2, 26);
    wrefresh(error_win);
    cmd_key = wgetch(error_win);
    win_del();
    return (cmd_key);
}

int display_error_message(char *emsg_str) {
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
    if (len < 26)
        len = 26;
    strcpy(title, "Notification");
    if (win_new(1, len + 2, line, pos, title)) {
        sprintf(title, "win_new(%d, %d, %d, %d) failed", 4, line, line, pos);
        abend(-1, title);
    }
    error_win = win_win[win_ptr];
    mvwaddstr(error_win, 0, 1, emsg);
    wmove(error_win, 0, len + 1);
    wrefresh(error_win);
    cmd_key = wgetch(error_win);
    win_del();
    return (cmd_key);
}

void mvwaddstr_fill(WINDOW *w, int y, int x, char *s, int l) {
    char *d, *e;

    d = tmp_str;
    e = tmp_str + l;
    while (d < e)
        if (*s == '\0' || *s == '\n')
            *d++ = ' ';
        else
            *d++ = *s++;
    *d++ = '\0';
    mvwaddstr(w, y, x, tmp_str);
}

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

int nf_error(int ec, char *s) {
    fprintf(stderr, "ERROR: %s code: %d\n", s, ec);
    fprintf(stderr, "Press a key to continue");
    di_getch();
    fprintf(stderr, "\n");
    return ec;
}

void user_end() {
    close_curses();
    restore_shell_tioctl();
    sig_dfl_mode();
    fprintf(stderr, "Normal program exit");
    fprintf(stderr, "\n");
    exit(EXIT_SUCCESS);
}

void abend(int ec, char *s) {
    close_curses();
    restore_shell_tioctl();
    sig_dfl_mode();
    fprintf(stderr, "\nABEND: %s code: %d\n", s, ec);
    fprintf(stderr, "Press a key to exit program");
    // di_getch();
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}
