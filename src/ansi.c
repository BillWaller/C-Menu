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

#define BUFLEN 1024

typedef struct {
    int r;
    int g;
    int b;
} RGB;

RGB StdColors[16] = {
    {0, 0, 0},       {128, 0, 0},   {0, 128, 0},   {128, 128, 0},
    {0, 0, 128},     {128, 0, 128}, {0, 128, 128}, {192, 192, 192},
    {128, 128, 128}, {255, 0, 0},   {0, 255, 0},   {255, 255, 0},
    {0, 0, 255},     {255, 0, 255}, {0, 255, 255}, {255, 255, 255}};

RGB xterm256_idx_to_rgb(int);
int rgb_to_xterm256_idx(RGB);
void init_clr_palette();
short get_clr_pair(short, short);
int clr_idx;
int clr_pair_idx;

#define MAX_COLOR_PAIRS 512
typedef struct {
    short fg;
    short bg;
    short pair_id;
} ColorPair;

ColorPair clr_pairs[MAX_COLOR_PAIRS];
int pair_cnt = 0;

void def_clr_pairs();
bool ansi_to_cmplx(cchar_t *, const char *);
int parse_ansi_str(WINDOW *, char *, attr_t *, short *);

/*  ╭───────────────────────────────────────────────────────────────╮
    │ ANSI_TO_CMPLX function prototypes                             │
    ╰───────────────────────────────────────────────────────────────╯*/

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
    CLR_BLACK,   CLR_RED,  CLR_GREEN, CLR_YELLOW,  CLR_BLUE,
    CLR_MAGENTA, CLR_CYAN, CLR_WHITE, CLR_NCOLORS,
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
int cp_default;
int cp_norm;
int cp_reverse;

typedef struct {
    short fg;
    short bg;
} Pair;

Pair pairs[CP_NCOLOR_PAIRS] = {
    {CLR_WHITE, CLR_BLACK},   // CP_DEFAULT
    {CLR_GREEN, CLR_BLACK},   // CP_NORM
    {CLR_CYAN, CLR_BLACK},    // CP_BOX
    {CLR_BLACK, CLR_YELLOW},  // CP_REVERSE
    {CLR_YELLOW, CLR_BLACK},  // CP_TITLE
    {CLR_BLACK, CLR_CYAN},    // CP_HIGHLIGHT
    {CLR_RED, CLR_BLACK},     // CP_RED
    {CLR_GREEN, CLR_BLACK},   // CP_GREEN
    {CLR_YELLOW, CLR_BLACK},  // CP_YELLOW
    {CLR_BLUE, CLR_BLACK},    // CP_BLUE
    {CLR_MAGENTA, CLR_BLACK}, // CP_MAGENTA
    {CLR_CYAN, CLR_BLACK},    // CP_CYAN
    {CLR_WHITE, CLR_BLACK},   // CP_WHITE
};

int main(int argc, char **argv) {
    cchar_t cmplx_char_buf[1024];
    int i = 0;
    int j = 0;
    FILE *file_ptr;
    char line_buffer[BUFLEN];
    short r, g, b;

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
    if (has_colors() && start_color() == OK) {
        clr_idx = CLR_NCOLORS + 1;
        clr_pair_idx = CP_NCOLOR_PAIRS + 1;
        def_clr_pairs();
        cp_default = get_clr_pair(CLR_WHITE, CLR_BLACK);
        cp_norm = get_clr_pair(CLR_GREEN, CLR_BLACK);
        cp_reverse = get_clr_pair(CLR_BLACK, CLR_YELLOW);
        wcolor_set(stdscr, CP_NORM, NULL);
    }
    file_ptr = fopen(argv[1], "r");
    if (file_ptr == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    while (fgets(line_buffer, BUFLEN, file_ptr) != NULL) {
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
    char ansi_tok[64];
    const char *s;
    const char *start_s;
    short cp = CP_DEFAULT;
    int i = 0, j = 0, k = 0;
    size_t len;
    wchar_t wc;
    wchar_t null_wch = L'\0';
    cchar_t cc;
    // mbstate_t state;
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
            s = &in_str[i];
            len = mbtowc(&wc, s, MB_CUR_MAX);
            if (len > 0) {
                if (setcchar(&cc, &wc, attr, cp, NULL) != ERR) {
                    cmplx_buf[j++] = cc;
                }
                s += len;
                i += len;
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
    int i, len, x_idx;
    short r, g, b;
    bool f_fg = false, f_bg = false;
    short color_pair;
    short fg, bg;
    char *ansi_p = ansi_str + 2;
    pair_content(*cp, &fg, &bg);
    RGB rgb;
    for (i = 1; i < 4; i++) {
        tok = strtok((char *)ansi_p, ";m");
        if (tok == NULL)
            break;
        ansi_p = NULL;
        len = strlen(tok);
        if (len == 2) {
            if (tok[0] == '3' && tok[1] == '8') {
                /*  ╭───────────────────────────────────────────────╮
                    │ 38 - FOREGROUND                               │
                    ╰───────────────────────────────────────────────╯*/
                tok = strtok((char *)ansi_p, ";m");
                ansi_p = 0;
                if (tok != NULL && strcmp(tok, "5") == 0) {
                    /*  ╭───────────────────────────────────────────╮
                        │ 5 - 256 CLR PALETTE                       │
                        ╰───────────────────────────────────────────╯*/
                    tok = strtok((char *)ansi_p, ";m");
                    ansi_p = NULL;
                    if (tok != NULL) {
                        /*  ╭───────────────────────────────────────╮
                            │ XTERM_IDX_TO_RGB                      │
                            ╰───────────────────────────────────────╯*/
                        x_idx = atoi(tok);
                        if (fg != x_idx) {
                            fg = x_idx;
                            f_fg = true;
                        }
                    }
                } else {
                    if (tok != NULL && strcmp(tok, "2") == 0) {
                        /*  ╭───────────────────────────────────────────╮
                            │ 2 - RGB                                   │
                            ╰───────────────────────────────────────────╯*/
                        tok = strtok((char *)ansi_p, ";m");
                        ansi_p = NULL;
                        rgb.r = atoi(tok);
                        tok = strtok((char *)ansi_p, ";m");
                        ansi_p = NULL;
                        rgb.g = atoi(tok);
                        tok = strtok((char *)ansi_p, ";m");
                        ansi_p = NULL;
                        rgb.b = atoi(tok);
                        x_idx = rgb_to_xterm256_idx(rgb);
                        if (fg != x_idx) {
                            fg = x_idx;
                            f_fg = true;
                        }
                    }
                }
            } else if (tok[0] == '4' && tok[1] == '8') {
                /*  ╭───────────────────────────────────────────────╮
                    │ 48 - BACKGROUND                               │
                    ╰───────────────────────────────────────────────╯*/
                tok = strtok((char *)ansi_p, ";m");
                ansi_p = NULL;
                if (tok != NULL && strcmp(tok, "5") == 0) {
                    /*  ╭───────────────────────────────────────────╮
                        │ 5 - 256 CLR PALETTE                       │
                        ╰───────────────────────────────────────────╯*/
                    tok = strtok((char *)ansi_p, ";m");
                    ansi_p = NULL;
                    if (tok != NULL) {
                        /*  ╭───────────────────────────────────────╮
                            │ XTERM_IDX_TO_RGB                      │
                            ╰───────────────────────────────────────╯*/
                        int x_idx = atoi(tok);
                        if (bg != x_idx) {
                            bg = x_idx;
                            f_bg = true;
                        }
                    }
                } else {
                    if (tok != NULL && strcmp(tok, "2") == 0) {
                        /*  ╭───────────────────────────────────────────╮
                            │ 2 - RGB COLOR                             │
                            ╰───────────────────────────────────────────╯*/
                        tok = strtok((char *)ansi_p, ";m");
                        ansi_p = NULL;
                        rgb.r = atoi(tok);
                        tok = strtok((char *)ansi_p, ";m");
                        ansi_p = NULL;
                        rgb.g = atoi(tok);
                        tok = strtok((char *)ansi_p, ";m");
                        ansi_p = NULL;
                        rgb.b = atoi(tok);
                        x_idx = rgb_to_xterm256_idx(rgb);
                        if (bg != x_idx) {
                            bg = x_idx;
                            f_bg = true;
                        }
                    }
                }
            } else if (tok[0] == '3' && tok[1] == '9') {
                if (fg != COLOR_WHITE) {
                    fg = COLOR_WHITE;
                    f_fg = true;
                }
            } else if (tok[0] == '4' && tok[1] == '9') {
                if (bg != COLOR_BLACK) {
                    bg = COLOR_BLACK;
                    f_bg = true;
                }
            } else if ((tok[0] == '3' || tok[1] == '4') && tok[1] >= '0' &&
                       tok[1] <= '7') {
                if (tok[0] == '3')
                    i = atoi(&tok[1]);
                if (fg != i) {
                    fg = i;
                    f_fg = true;
                } else if (tok[0] == '4')
                    i = atoi(&tok[1]);
                if (bg != i) {
                    bg = i;
                    f_bg = true;
                }
            } else if (tok[0] == '0' && tok[1] == '1') {
                *attr = A_NORMAL;
            } else if (tok[0] == '0' && tok[1] == '1')
                *attr = A_BOLD;
            else if (tok[0] == '0' && tok[1] == '2')
                *attr = A_REVERSE;
            else if (tok[0] == '0' && tok[1] == '3')
                *attr = A_ITALIC;
        } else if (len == 1) {
            if (tok[0] == '0') {
                *attr = A_NORMAL;
            } else if (tok[0] == '1')
                *attr = A_BOLD;
            else if (tok[0] == '2')
                *attr = A_REVERSE;
            else if (tok[0] == '3')
                *attr = A_ITALIC;
        } else if (len == 0) {
            *attr = A_NORMAL;
        }
    }
    if (f_fg == true || f_bg == true) {
        clr_pair_idx = get_clr_pair(fg, bg);
        *cp = clr_pair_idx++;
    }
    return 0;
}

// RGB -> xterm 256 color
int rgb_to_xterm256_idx(RGB rgb) {
    if (rgb.r == rgb.g && rgb.g == rgb.b) {
        if (0 <= rgb.r && rgb.r <= 23) {
            return 232 + (rgb.r / 11);
        }
    }
    rgb.r = rgb.r / 51;
    rgb.g = rgb.g / 51;
    rgb.b = rgb.b / 51;
    return 16 + (36 * rgb.r) + (6 * rgb.g) + rgb.b;
}

// xterm 256 color -> RGB
RGB xterm256_idx_to_rgb(int idx) {
    RGB rgb;
    if (idx < 16) {
        rgb.r = StdColors[idx].r;
        rgb.g = StdColors[idx].g;
        rgb.b = StdColors[idx].b;
        return rgb;
    } else if (idx >= 16 && idx <= 231) {
        idx -= 16;
        rgb.r = (idx / 36) % 6 * 51;
        rgb.g = (idx / 6) % 6 * 51;
        rgb.b = (idx % 6) * 51;
        return rgb;
    } else if (idx >= 232 && idx <= 255) {
        int gray = (idx - 232) * 11;
        rgb.r = rgb.g = rgb.b = gray;
        return rgb;
    }
    return rgb;
}

void init_clr_palette() {
    int i;
    uint8_t rr, gg, bb;
    RGB rgb;

    for (i = 0; i < 256; i++) {
        rgb = xterm256_idx_to_rgb(i);
        rr = (rgb.r * 1000) / 255;
        gg = (rgb.g * 1000) / 255;
        bb = (rgb.b * 1000) / 255;
        init_color(i + 1, rr, gg, bb);
    }
}

void def_clr_pairs() {
    int i, j, id;
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 8; j++) {
            id = get_clr_pair(i, j);
            clr_pairs[pair_cnt].fg = i;
            clr_pairs[pair_cnt].bg = j;
            clr_pairs[pair_cnt].pair_id = id;
            pair_cnt++;
        }
    }
    clr_pair_idx = id + 1;
}

short get_clr_pair(short fg, short bg) {
    for (int i = 0; i < pair_cnt; i++) {
        if (clr_pairs[i].fg == fg && clr_pairs[i].bg == bg) {
            return clr_pairs[i].pair_id;
        }
    }
    if (pair_cnt < MAX_COLOR_PAIRS) {
        short id = pair_cnt + 1;
        init_pair(id, fg, bg);
        clr_pairs[pair_cnt].fg = fg;
        clr_pairs[pair_cnt].bg = bg;
        clr_pairs[pair_cnt].pair_id = id;
        pair_cnt++;
        return id;
    }
    return ERR;
}
