/* futil.c
 * Utility functions for MENU
 * Bill Waller
 * billxwaller@gmail.com
 */

#include "menu.h"
#include <stdlib.h>
#include <string.h>

int str_to_args(char **, char *);
void str_to_lower(char *);
void str_to_upper(char *);
void strnz_cpy(char *, char *, int);
void strnz_cat(char *, char *, int);
void strz(char *);
int strnz(char *, int);
char *strz_dup(char *);
char *strzdup(char *);
void str_subc(char *, char *, char, char *, int);
void normalize_file_spec(char *);
void file_spec_path(char *, char *);
void file_spec_name(char *, char *);
int parse_geometry_str(char *, int *, int *, int *, int *);
int get_color_number(char *s);
void list_colors();
void build_color_opt_str();
int verify_screen_geometry(int *, int *, int *, int *);
int full_screen_fork_exec(char **);
int full_screen_shell(char *);
int shell(char *);
void abend(int, char *);
char errmsg[MAXLEN + 1];
char color_opt_str[MAXLEN + 1];

int str_to_args(char **argv, char *strptr) {
    int i;

    for (i = 0; i < MAXARGS; i++) {
        if ((argv[i] = strtok(strptr, " \t")) == (char *)0)
            break;
        strptr = (char *)0;
    }
    argv[i] = NULL;
    return (i);
}

void str_to_lower(char *s) {
    while (*s != '\0') {
        if (*s >= 'A' && *s <= 'Z')
            *s = *s + 'a' - 'A';
        s++;
    }
}

void str_to_upper(char *s) {
    while (*s != '\0') {
        if (*s >= 'a' && *s <= 'z')
            *s = *s + 'A' - 'a';
        s++;
    }
}

void strnz_cpy(char *d, char *s, int l) {
    char *e;

    e = d + l;
    while (*s != '\0' && *s != '\n' && *s != '\r' && d < e)
        *d++ = *s++;
    *d = '\0';
}

void strnz_cat(char *d, char *s, int l) {
    char *e;

    e = d + l;
    while (*d != '\0' && *d != '\n' && *d != '\r' && d < e)
        d++;
    while (*s != '\0' && *s != '\n' && *s != '\r' && d < e)
        *d++ = *s++;
    *d = '\0';
}

void strz(char *s) {
    while (*s != '\0' && *s != '\n' && *s != '\r')
        s++;
    *s = '\0';
}

int strnz(char *s, int l) {
    char *e;
    int i;

    e = s + l;
    i = 0;
    while (*s != '\0' && *s != '\n' && *s != '\r' && s < e) {
        s++;
        i++;
    }
    *s = '\0';
    return (i);
}

char *strnz_dup(char *s, int l) {
    char *p, *rs, *e;
    int m;

    for (p = s, m = 1; *p != '\0'; p++, m++)
        ;
    rs = p = (char *)malloc(m);
    if (rs != NULL) {
        e = rs + l;
        while (*s != '\0' && *s != '\n' && *s != '\r' && p < e)
            *p++ = *s++;
        *p = '\0';
    }
    return (rs);
}

char *strz_dup(char *s) {
    char *p, *rs;
    int m;

    for (p = s, m = 1; *p != '\0'; p++, m++)
        ;
    rs = p = (char *)malloc(m);
    if (rs != NULL) {
        while (*s != '\0')
            *p++ = *s++;
        *p = '\0';
    }
    return (rs);
}

void str_subc(char *d, char *s, char ReplaceChr, char *Withstr, int l) {
    char *e;

    e = d + l;
    while (*s != '\0' && d < e) {
        if (*s == ReplaceChr) {
            while (*Withstr != '\0' && d < e)
                *d++ = *Withstr++;
            s++;
        } else
            *d++ = *s++;
    }
    *d = '\0';
}

void normalize_file_spec(char *fs) {
    while (*fs != '\0') {
        if (*fs == '/')
            *fs = '/';
        fs++;
    }
}

void file_spec_path(char *fp, char *fs) {
    char *d, *l, *s;

    s = fs;
    d = fp;
    l = NULL;
    while (*s != '\0') {
        if (*s == '/')
            l = d;
        *d++ = *s++;
    }
    if (l == NULL)
        *fp = '\0'; /* no slash, so no path */
    else
        *l = '\0';
}

void file_spec_name(char *fn, char *fs) {
    char *d, *l, *s;

    l = NULL;
    s = fs;
    while (*s != '\0') {
        if (*s == '/')
            l = s;
        s++;
    }
    if (l == NULL)
        s = fs;
    else
        s = ++l;
    d = fn;
    while (*s != '\0')
        *d++ = *s++;
    *d = '\0';
}

int parse_geometry_str(char *s, int *l, int *c, int *y, int *x) {
    char str[4], *d, *e;

    d = str;
    e = s + 3;
    while (*s != '\0' && s < e)
        *d++ = *s++;
    *d = '\0';
    if (*s == '\0')
        return (-1);
    d = str;
    *l = atoi(d);

    d = str;
    e = s + 3;
    while (*s != '\0' && s < e)
        *d++ = *s++;
    *d = '\0';
    if (*s == '\0')
        return (-1);
    d = str;
    *c = atoi(d);

    d = str;
    e = s + 3;
    while (*s != '\0' && s < e)
        *d++ = *s++;
    *d = '\0';
    if (*s == '\0')
        return (-1);
    d = str;
    *y = atoi(d);

    d = str;
    e = s + 3;
    while (*s != '\0' && s < e)
        *d++ = *s++;
    *d = '\0';
    d = str;
    *x = atoi(d);

    return (0);
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

void build_color_opt_str() {
    if (option->fg_color == option->bg_color || option->fg_color == -1 ||
        option->bg_color == -1) {
        option->bg_color = black;
        option->fg_color = white;
    }
    strncpy(color_opt_str, "-F ", MAXLEN);
    strncat(color_opt_str, colors_text[option->fg_color], MAXLEN);
    strncat(color_opt_str, " ", MAXLEN);
    strncat(color_opt_str, "-B ", MAXLEN);
    strncat(color_opt_str, colors_text[option->bg_color], MAXLEN);
}

int verify_screen_geometry(int *lines, int *cols, int *begy, int *begx) {
    int box;

    if (LINES == 0 || COLS == 0)
        return (1);
    box = 2;
    if (!*lines)
        *lines = DEF_LINES;
    if (*lines > LINES - box)
        *lines = LINES - box;
    if (*begy < 0 || *begy + *lines + box > LINES) /* bottom of screen */
        *begy = LINES - *lines - box;
    if (!*cols)
        *cols = DEF_COLS;
    if (*begx == -1)
        *begx = 0;
    if (*cols > COLS - box)
        *cols = COLS - box;
    if (*cols < MIN_COLS + box)
        *cols = MIN_COLS + box;
    if (*begx < 0 || *begx + *cols + box > COLS) /* left of screen   */
        *begx = COLS - *cols - box;
    return (0);
}

int full_screen_fork_exec(char **argv) {
    int rc;

    fprintf(stderr, "\n");
    fflush(stderr);
    wclear(stdscr);
    wmove(stdscr, LINES - 1, 0);
    wrefresh(stdscr);
    rc = fork_exec(argv);
    wclear(stdscr);
    wmove(stdscr, 0, 0);
    wrefresh(stdscr);
    restore_wins();
    return (rc);
}

int full_screen_shell(char *shellCmdPtr) {
    int rc;

    fprintf(stderr, "\n");
    fflush(stderr);
    wclear(stdscr);
    wmove(stdscr, 0, 0);
    wrefresh(stdscr);
    rc = shell(shellCmdPtr);
    restore_wins();
    return (rc);
}

int shell(char *shellCmdPtr) {
    int Eargc;
    char *Eargv[MAXARGS];
    char *shellPtr;
    int rc;

    Eargc = 0;
    shellPtr = getenv("SHELL");
    if (shellPtr == NULL || *shellPtr == '\0')
        shellPtr = DEFAULTSHELL;
    Eargv[Eargc++] = strdup(shellPtr);
    Eargv[Eargc++] = "-c";
    Eargv[Eargc++] = shellCmdPtr;
    Eargv[Eargc++] = NULL;
    rc = fork_exec(Eargv);
    return (rc);
}

void abend(int ec, char *s) {
    close_curses();
    reset_shell_mode();
    sig_shell_mode();
    fprintf(stderr, "ABEND: %s code: %d\n", s, ec);
    fprintf(stderr, "Press a key to exit program");
    di_getch();
    fprintf(stderr, "\n");
    exit(ec);
}
