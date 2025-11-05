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

void open_curses();
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
void win_init_attrs(int, int, int);
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

char tmp_str[MAXLEN];
char *tmp_ptr;
int exit_code;
unsigned int cmd_key;

int win_attr_Odd;
int win_attr_Even;
int win_attr;
int box_Attr;
int win_ptr;
int m_lines;
int m_cols;
int m_begy = -1;
int m_begx = -1;
int mg_action, mg_col, mg_line;
int mouse_support;

char const colors_text[16][10] = {"black",  "red",   "green",    "yellow",
                                  "blue",   "cyan",  "magenta",  "white",
                                  "bblack", "bred",  "bgreen",   "byellow",
                                  "bblue",  "bcyan", "bmagenta", "bwhite"};

int const ncolors[16] = {COLOR_BLACK,
                         COLOR_RED,
                         COLOR_GREEN,
                         COLOR_YELLOW,
                         COLOR_BLUE,
                         COLOR_CYAN,
                         COLOR_MAGENTA,
                         COLOR_WHITE,
                         COLOR_BLACK | A_BOLD,
                         COLOR_RED | A_BOLD,
                         COLOR_GREEN | A_BOLD,
                         COLOR_YELLOW | A_BOLD,
                         COLOR_BLUE | A_BOLD,
                         COLOR_CYAN | A_BOLD,
                         COLOR_MAGENTA | A_BOLD,
                         COLOR_WHITE | A_BOLD};

const wchar_t bw_ho = BW_HO;
const wchar_t bw_ve = BW_VE;
const wchar_t bw_tl = BW_RTL;
const wchar_t bw_tr = BW_RTR;
const wchar_t bw_bl = BW_RBL;
const wchar_t bw_br = BW_RBR;
const wchar_t bw_lt = BW_LT;
const wchar_t bw_rt = BW_RT;
const wchar_t bw_sp = BW_SP;

void open_curses() {
    initscr();
    f_curses_open = true;
    clear();
    nonl(); // don't translate CR to LF
    noecho();
    cbreak(); // raw unbuffered
    keypad(stdscr, true);
    clearok(stdscr, false);
    scrollok(stdscr, true);
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
            werase(win_box[win_ptr]);
            wattrset(win_box[win_ptr], box_Attr);
            cbox(win_box[win_ptr]);
            if (WTitle != NULL && *WTitle != '\0') {
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
            wattrset(win_win[win_ptr], win_attr);
        } else {
            win_box[win_ptr] = newwin(wlines, wcols, wbegy, wbegx);
            if (win_box[win_ptr] == NULL)
                return (-1);
            wattrset(win_box[win_ptr], box_Attr);
            werase(win_box[win_ptr]);
            wnoutrefresh(win_box[win_ptr]);
            win_win[win_ptr] = newwin(wlines, wcols, wbegy, wbegx);
            wattrset(win_win[win_ptr], win_attr);
        }
        if (win_win[win_ptr] == NULL)
            return (-1);
        keypad(win_win[win_ptr], TRUE);
    }
    return (0);
}

void win_redraw(WINDOW *win, int Wattr, char *WTitle) {
    werase(win);
    wnoutrefresh(win);
}

WINDOW *win_open_box(int wlines, int wcols, int wbegy, int wbegx,
                     char *WTitle) {
    WINDOW *Wbox;
    int maxx;

    wrefresh(stdscr);
    if (wbegy != 0 || wbegx != 0 || wlines < LINES - 2 || wcols < COLS - 2) {
        Wbox = newwin(wlines + 2, wcols + 2, wbegy, wbegx);
        if (Wbox == (WINDOW *)0)
            return ((WINDOW *)0);
        werase(Wbox);
        wattrset(Wbox, box_Attr);
        cbox(Wbox);
        if (WTitle != (char *)0 && *WTitle != '\0') {
            mvwaddch(Wbox, 0, 1, ACS_URCORNER);
            mvwaddch(Wbox, 0, 2, ' ');
            mvwaddstr(Wbox, 0, 3, WTitle);
            maxx = getmaxx(Wbox);
            if ((strlen(WTitle) + 3) < (size_t)maxx)
                mvwaddch(Wbox, 0, strlen(WTitle) + 3, ' ');
            if ((strlen(WTitle) + 4) < (size_t)maxx)
                mvwaddch(Wbox, 0, strlen(WTitle) + 4, ACS_ULCORNER);
        }
        wnoutrefresh(Wbox);
    } else {
        Wbox = newwin(wlines, wcols, wbegy, wbegx);
        if (Wbox == (WINDOW *)0)
            return ((WINDOW *)0);
        werase(Wbox);
        wattrset(Wbox, box_Attr);
        wnoutrefresh(Wbox);
    }
    return (Wbox);
}

WINDOW *win_open_win(int wlines, int wcols, int wbegy, int wbegx) {
    WINDOW *W;

    if (wbegy != 0 || wbegx != 0 || wlines < LINES - 2 || wcols < COLS - 2)
        W = newwin(wlines, wcols, wbegy + 1, wbegx + 1);
    else
        W = newwin(wlines, wcols, wbegy, wbegx);
    wattrset(W, win_attr);
    return (W);
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

void win_close_win(WINDOW *W) { delwin(W); }

void win_close_box(WINDOW *Wbox) {
    int i;

    delwin(Wbox);
    if (win_ptr > 0) {
        touchwin(stdscr);
        wnoutrefresh(stdscr);
        for (i = 0; i < win_ptr; i++) {
            touchwin(win_box[i]);
            wnoutrefresh(win_box[i]);
            touchwin(win_win[i]);
            wnoutrefresh(win_win[i]);
        }
    } else {
        touchwin(stdscr);
        wnoutrefresh(stdscr);
    }
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

void win_init_attrs(int fg_color, int bg_color, int bo_color) {
    start_color();
    init_pair(1, ncolors[fg_color], ncolors[bg_color]);
    init_pair(2, ncolors[bo_color], ncolors[bg_color]);
    attr_on(COLOR_PAIR(1), NULL);
    win_attr = COLOR_PAIR(1);
    box_Attr = COLOR_PAIR(2);
}

int error_message(char **argv) {
    const int msg_max_len = 71;
    WINDOW *error_win;
    int len, line, pos;
    int argc, i;
    char msg[72];
    char title[64];
    unsigned cmd_key;
    char c;

    argc = 0;
    while (argv[argc]) {
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
            sprintf(tmp_str, "win_new(%d, %d, %d, %s) failed", cols, line, pos,
                    msg);
            abend(-1, tmp_str);
        }
        error_win = win_win[win_ptr];
        i = 0;
        while (i < argc)
            mvwaddstr(error_win, i, 1, argv[i++]);
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
        while (i++ < lines) {
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

int display_error_message(char *emsg_str) {
    char emsg[80];
    int emsg_max_len = 80;
    unsigned cmd_key;
    WINDOW *error_win;
    int len, line, pos;

    if (!f_curses_open) {
        fprintf(stderr, "\n%s\n", emsg);
        return (1);
    }

    len = strnz__cpy(emsg, emsg_str, emsg_max_len - 1);

    pos = (COLS - len - 4) / 2;
    line = (LINES - 4) / 2;

    strcpy(tmp_str, "Notification");
    if (win_new(1, len + 2, line, pos, tmp_str)) {
        sprintf(tmp_str, "win_new(%d, %d, %d, %d) failed", 4, line, line, pos);
        abend(-1, tmp_str);
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
