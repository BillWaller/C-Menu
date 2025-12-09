#define _XOPEN_SOURCE_EXTENDED 1
#define NCURSES_WIDECHAR 1
#include <locale.h>
#include <ncursesw/curses.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

bool parse_ansi_str(WINDOW *, char *, attr_t *, short *);
bool ansi_to_cmplx_str(cchar_t *, const char *);

enum colors_enum {
    CLR_BLACK = COLOR_BLACK,
    CLR_RED = COLOR_RED,
    CLR_GREEN = COLOR_GREEN,
    CLR_YELLOW = COLOR_YELLOW,
    CLR_BLUE = COLOR_BLUE,
    CLR_MAGENTA = COLOR_MAGENTA,
    CLR_CYAN = COLOR_CYAN,
    CLR_WHITE = COLOR_WHITE,
    CLR_NCOLORS
};

int const ncolors[] = {
    CLR_BLACK,   CLR_RED,  CLR_GREEN, CLR_YELLOW, CLR_BLUE,
    CLR_MAGENTA, CLR_CYAN, CLR_WHITE, 0,
};

enum color_pairs_enum {
    CP_DEFAULT = 50,
    CP_NORM,
    CP_BOX,
    CP_REVERSE,
    CP_TITLE,
    CP_HIGHLIGHT,
    CP_RED,
    CP_GREEN,
    CP_YELLOW,
    CP_BLUE,
    CP_MAGENTA,
    CP_CYAN,
    CP_WHITE,
    CP_NCOLOR_PAIRS
};

int main() {
    cchar_t cmplx_char_buf[256];
    int i = 0;
    int j = 0;

    setlocale(LC_ALL, "");
    initscr();
    clear();
    nonl();
    noecho();
    cbreak();
    keypad(stdscr, true);
    clearok(stdscr, false);
    scrollok(stdscr, true);
    immedok(stdscr, TRUE);
    if (has_colors()) {
        start_color();
        for (i = 0; i < 8; i++) {
            for (j = 0; j < 8; j++) {
                init_pair(i * 8 + j + 1, i, j);
            }
        }
        init_pair(CP_DEFAULT, CLR_WHITE, CLR_BLACK);
        init_pair(CP_NORM, CLR_GREEN, CLR_BLACK);
        init_pair(CP_REVERSE, CLR_BLACK, CLR_YELLOW);
        init_pair(CP_RED, CLR_RED, CLR_BLACK);
        init_pair(CP_GREEN, CLR_GREEN, CLR_BLACK);
        init_pair(CP_YELLOW, CLR_YELLOW, CLR_BLACK);
        init_pair(CP_BLUE, CLR_BLUE, CLR_BLACK);
        init_pair(CP_MAGENTA, CLR_MAGENTA, CLR_BLACK);
        init_pair(CP_CYAN, CLR_CYAN, CLR_BLACK);
        init_pair(CP_WHITE, CLR_WHITE, CLR_BLACK);
        wcolor_set(stdscr, CP_NORM, NULL);
    }

    const char *ansi_string = "This is \033[32mbold red\033[0m text.";
    ansi_to_cmplx_str(cmplx_char_buf, ansi_string);
    move(2, 0);
    add_wchstr(cmplx_char_buf);
    move(3, 0);
    refresh();
    getch();
    endwin();
}

bool ansi_to_cmplx_str(cchar_t *cmplx_buf, const char *ansi_string) {
    wchar_t wide_char_buffer[256];
    cchar_t cmplx_char_buf[256];
    cchar_t cc;
    attr_t attr = A_NORMAL;
    char ansi_tok[32];
    short cp = CP_DEFAULT;
    int i = 0, j = 0;
    while (ansi_string[i] != '\0') {
        if (ansi_string[i] == '\033' && ansi_string[i + 1] == '[') {
            // CSI sequence detected
            char *start_seq = (char *)&ansi_string[i];
            char *end_seq = strchr(start_seq, 'm');
            if (end_seq) {
                strncpy(ansi_tok, start_seq, end_seq - start_seq + 1);
                ansi_tok[end_seq - start_seq + 1] = '\0';
                parse_ansi_str(stdscr, ansi_tok, &attr, &cp);
                i = (end_seq - ansi_string) + 1;
            } else {
                i++;
            }
        } else {
            mbtowc(&wide_char_buffer[j], &ansi_string[i], MB_CUR_MAX);
            setcchar(&cc, &wide_char_buffer[j], attr, cp, NULL);
            cmplx_char_buf[j] = cc;
            add_wch(&cc);
            refresh();
            j++;
            i++;
        }
    }
    return 0;
}

bool parse_ansi_str(WINDOW *win, char *ansi_str, attr_t *attr, short *cp) {
    char *token;
    int i, c;
    short color_pair;
    short fg, bg;
    char *ansi_ptr = ansi_str + 2;
    pair_content(*cp, &fg, &bg);
    for (i = 1; i < 4; i++) {
        token = strtok((char *)ansi_ptr, ";m");
        if (token == NULL)
            break;
        ansi_ptr = NULL;
        int l = strlen(token);
        if (l == 2) {
            if (token[0] == '3' ||
                token[0] == '4' && token[1] >= '0' && token[1] <= '7') {
                if (token[0] == '3')
                    fg = atoi(&token[1]);
                else if (token[0] == '4')
                    bg = atoi(&token[1]);
            }
            *cp = fg * 8 + bg + 1;
        } else if (l == 1) {
            if (token[0] == '0') {
                *attr = A_NORMAL;
                *cp = CP_DEFAULT;
            } else if (token[0] == '1')
                *attr = A_BOLD;
            else if (token[0] == '2')
                *attr = A_REVERSE;
        } else if (l == 0) {
            *attr = A_NORMAL;
            *cp = CP_DEFAULT;
        }
    }
    return true;
}
