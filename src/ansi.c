//  ansi.c
//  Copyright Bill Waller 2025
//  ansi_to_cmplx
//
//  This is the code used by C-Menu View to render ANSI escape sequences
//
//  to compile:
//
//      cc ansi.c -o ansi -lncursesw
//
//  to run:
//
//      ./ansi file-name
//
//  ansi.c translates text with embedded ANSI escape sequences into complex
//  character strings and displays them with:
//
//      add_wchstr(cmplx_char_buf);
//
//
#define _XOPEN_SOURCE_EXTENDED 1
#define NCURSES_WIDECHAR 1
#include <locale.h>
#include <ncursesw/curses.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

// #define uchar unsigned char
#define MAXLEN 256

/*  ╭───────────────────────────────────────────────────────────────╮
    │ ANSI_TO_CMPLX function prototypes                             │
    ╰───────────────────────────────────────────────────────────────╯*/
bool ansi_to_cmplx(cchar_t *, const char *);
int parse_ansi_str(WINDOW *, char *, attr_t *, short *);
void default_color_pairs();

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
    CP_DEFAULT = 64,
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

int main(int argc, char **argv) {
    cchar_t cmplx_char_buf[1024];
    int i = 0;
    int j = 0;

    if (argc != 2) {
        printf("\n\nusage: ansi file-name\n\n");
        return 0;
    }
    setlocale(LC_ALL, "");
    initscr();
    clear();
    nonl();
    noecho();
    cbreak();
    keypad(stdscr, true);
    clearok(stdscr, false);
    scrollok(stdscr, true);
    setscrreg(0, LINES - 1);
    immedok(stdscr, TRUE);
    if (has_colors()) {
        default_color_pairs();
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

    FILE *file_ptr;
    char line_buffer[MAXLEN];
    file_ptr = fopen(argv[1], "r");
    if (file_ptr == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    while (fgets(line_buffer, MAXLEN, file_ptr) != NULL) {
        ansi_to_cmplx(cmplx_char_buf, line_buffer);
        wmove(stdscr, 49, 0);
        add_wchstr(cmplx_char_buf);
        scroll(stdscr);
        refresh();
    }
    getch();
    endwin();
    fclose(file_ptr);
}

/*  ╭───────────────────────────────────────────────────────────────╮
    │ ANSI_TO_CMPLX                                                 │
    ╰───────────────────────────────────────────────────────────────╯*/
bool ansi_to_cmplx(cchar_t *cmplx_buf, const char *in_str) {
    attr_t attr = A_NORMAL;
    char ansi_tok[32];
    const char *s;
    const char *start_s;
    short cp = CP_DEFAULT;
    int i = 0, j = 0, k = 0;
    size_t len;
    wchar_t wc;
    wchar_t null_wch = L'\0';
    cchar_t cc;
    mbstate_t state;
    mbtowc(NULL, NULL, 0);
    while (in_str[i] != '\0') {
        if (in_str[i] == '\033' && in_str[i + 1] == '[') {
            char *start_seq = (char *)&in_str[i];
            char *end_seq = strchr(start_seq, 'm');
            if (end_seq) {
                strncpy(ansi_tok, start_seq, end_seq - start_seq + 1);
                ansi_tok[end_seq - start_seq + 1] = '\0';
                parse_ansi_str(stdscr, ansi_tok, &attr, &cp);
                i = (end_seq - in_str) + 1;
            } else {
                i++;
            }
        } else {
            char *s = (char *)&in_str[i];
            len = mbtowc(&wc, s, MB_CUR_MAX);
            if (len > 0) {
                s += len;
                i += len;
                if (setcchar(&cc, &wc, attr, cp, NULL) != ERR)
                    cmplx_buf[j++] = cc;
            } else {
                if (len == -1) { // don't know what to do with these yet
                    break;
                } else {
                    if (len == -2) {
                        break;
                    }
                }
            }
        }
    }
    wc = L'\0';
    setcchar(&cc, &wc, A_NORMAL, CP_DEFAULT, NULL);
    cmplx_buf[j] = cc;
    return true;
}
/*  ╭───────────────────────────────────────────────────────────────╮
    │ PARSE_ANSI_STR                                                │
    ╰───────────────────────────────────────────────────────────────╯*/
int parse_ansi_str(WINDOW *win, char *ansi_str, attr_t *attr, short *cp) {
    char *tok;
    int i, len;
    short color_pair;
    short fg, bg;
    char *ansi_ptr = ansi_str + 2;
    pair_content(*cp, &fg, &bg);
    for (i = 1; i < 4; i++) {
        tok = strtok((char *)ansi_ptr, ";m");
        if (tok == NULL)
            break;
        ansi_ptr = NULL;
        len = strlen(tok);
        if (len == 2) {
            if (tok[0] == '3' || tok[0] == '4' ||
                (tok[1] >= '0' && tok[1] <= '7')) {
                if (tok[0] == '3')
                    fg = atoi(&tok[1]);
                else if (tok[0] == '4')
                    bg = atoi(&tok[1]);
            }
            *cp = fg * 8 + bg + 1; // color_pair(0) is reserved for default
        } else if (len == 1) {
            if (tok[0] == '0') {
                *attr = A_NORMAL;
                *cp = CP_DEFAULT;
            } else if (tok[0] == '1')
                *attr = A_BOLD;
            else if (tok[0] == '2')
                *attr = A_REVERSE;
        } else if (len == 0) {
            *attr = A_NORMAL;
            *cp = CP_DEFAULT;
        }
    }
    return 0;
}
/*  ╭───────────────────────────────────────────────────────────────╮
    │ DEFAULT_COLOR_PAIRS                                           │
    ╰───────────────────────────────────────────────────────────────╯*/
void default_color_pairs() {
    int i, j;
    start_color();
    for (i = 0; i < 8; i++)
        for (j = 0; j < 8; j++) {
            init_pair(i * 8 + j + 1, i,
                      j); // color_pair(0) is reserved for default
        }
}
