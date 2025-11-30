/* dgraph.c
 * Simple Charts for MENU
 * Bill Waller
 * billxwaller@gmail.com
 */

#include "menu.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define NELEMENTS 35
typedef double numtyp;
#define near
#define far

#define NSTRLEN 11
#define NFORM "%-10.2f"
#define GSTART 10
#define GWIDTH 69
#define YLINES 20

typedef unsigned char uchar;

char graph_file_name[MAXLEN];
char *graph_in[NELEMENTS * 2];
char *graph_title;
int nelements;
int nwidth;
char Nstr[NELEMENTS][NSTRLEN];
char far *nlabel[NELEMENTS];
numtyp far value[NELEMENTS];
numtyp nmax;
numtyp nmin;
numtyp norder;
int ny[NELEMENTS];
numtyp yincrement;
numtyp ymax;
numtyp f1;
numtyp f2;
char ytxt[YLINES][GSTART];
char yform[MAXLEN];
char form_str[MAXLEN + 1];
uchar print_char;
char graph_id[MAXLEN];
char graph_idFlag;
int graph_id_len;
bool f_print;
bool f_file;
bool f_pie;
bool f_bar;

int display_text_graph();
void print_graph();
void file_graph();
void stdin_graph();
char *gn_cpy(char *, int);

int main(int argc, char **argv) {
    extern int optind;
    extern char *optarg;
    int o;
    char errflg = 0;
    char *e, *d;
    int eargc;
    char *eargv[MAXARGS];
    int i, j;

    while (opt != EOF) {
        switch (opt) {
        case 'G':
            if (strcmp(optarg, "B") == 0)
                f_bar = TRUE;
            else if (strcmp(optarg, "P") == 0)
                f_pie = TRUE;
            break;
        case 'f':
            strncpy(graph_file_name, optarg, MAXLEN);
            f_file = TRUE;
            break;
        case 'p':
            if (strcmp(optarg, "7") == 0) {
                f_print = TRUE;
                print_char = '#';
            } else if (strcmp(optarg, "8") == 0) {
                f_print = TRUE;
                print_char = 219;
            } else
                errflg++;
            break;
        case 'r':
            strncpy(graph_id, optarg, MAXLEN);
            graph_id_len = strlen(graph_id);
            if (graph_id_len > 0)
                graph_idFlag++;
            else
                errflg++;
            break;
        default:
            errflg++;
            break;
        }
    }

    if (init->fg_color == -1)
        init->fg_color = 7;

    if (errflg) {
        fprintf(stderr,
                "usage: dgraph [-p{7||8}][-f file-name][-r record-id]\n");
        fprintf(stderr, "    -p 7    printable graph for 7 bit printers\n");
        fprintf(stderr, "    -p 8    printable graph for 8 bit printers\n");
        fprintf(stderr, "    -f file if input is to be taken from a file\n");
        fprintf(stderr, "    -r XXX  print only records prefixed with XXX\n\n");
        fprintf(stderr, "    -G B    bit mapped bar chart\n");
        fprintf(stderr, "    -G P    bit mapped pie chart\n");
        list_colors();
        exit(EXIT_SUCCESS);
    }

    if (f_file == FALSE)
        stdin_graph();
    else
        file_graph();

    graph_title = strnz_dup(graph_in[0], MAX_COLS);

    i = 1;
    j = 0;
    while (j < NELEMENTS && graph_in[i] != (char *)0 &&
           graph_in[i][0] != '\0') {
        nlabel[j] = (char far *)strnz_dup(graph_in[i], MAX_COLS);
        i++;
        if (graph_in[i][0] != '\0' && graph_in[i] != (char *)0)
            value[j] = atof(graph_in[i]);
        i++;
        j++;
    }

    if (i < 3)
        abend(1, "Not enough data items for graph");
    nelements = j;
    if (nelements <= 0)
        abend(1, "Not enough data items for graph");
    nmax = value[0];
    nmin = value[0];
    for (i = 0; i < nelements; i++) {
        strncpy(form_str, NFORM, MAXLEN);
        strncat(form_str, "\n", MAXLEN);
        sprintf(tmp_str, form_str, value[i]);
        strncpy(Nstr[i], tmp_str, NSTRLEN);
        Nstr[i][10] = '\0';
        if (value[i] < nmin)
            nmin = value[i];
        if (value[i] > nmax)
            nmax = value[i];
    }
    if (nmax <= 0)
        abend(1, "Can't graph 0");
    nwidth = ((GWIDTH - nelements + 1) / nelements);
    if (nwidth <= 0)
        abend(1, "Too many data items");
    f1 = nmax * YLINES;
    norder = floor(log10(f1));
    f2 = pow((numtyp)10, floor(norder) - (numtyp)1);
    ymax = ceil(f1 / f2) * f2 / YLINES;
    yincrement = ymax / YLINES;
    if (yincrement <= 0)
        abend(1, "Negative scaling");
    for (i = 0; i < nelements; i++)
        ny[i] = YLINES - ceil(value[i] / (numtyp)yincrement);
    strncpy(yform, "%8.0f", MAXLEN);
    i = 0;
    while (i < YLINES) {
        sprintf(ytxt[i], yform, (YLINES - i) * yincrement);
        i++;
        ytxt[i++][0] = '\0';
    }

    if (f_print == TRUE)
        print_graph();
    else {
        display_text_graph();
    }
}

int display_text_graph() {
    int x, y, e, i;
    open_curses();
    win_init_attrs(init->fg_color, init->bg_color, init->bo_color);
    werase(stdscr);
    for (y = 0; y < YLINES; y++) {
        mvaddstr(y, 1, ytxt[y]);
        mvwaddch(stdscr, y, 9, (char)ACS_RTEE);
        move(y, 10);
        for (e = 0; e < nelements; e++) {
            if (ny[e] > y) {
                for (i = 0; i < nwidth; i++)
                    addch(' ');
                addch(' ');
            } else {
                attron(A_REVERSE);
                for (i = 0; i < nwidth; i++)
                    addch(' ');
                attroff(A_REVERSE);
                addch(' ');
            }
        }
    }

    move(YLINES, GSTART - 1);
    addch((char)ACS_LLCORNER);
    for (e = 0; e < nelements; e++) {
        for (i = 0; i < nwidth; i++)
            addch((char)ACS_HLINE);
        if (e < (nelements - 1))
            addch((char)ACS_PLUS);
    }
    for (e = 0; e < nelements; e++) {
        x = (nwidth + 1) * e + (nwidth - strlen((char near *)nlabel[e])) / 2;
        mvaddstr(YLINES + 1, GSTART + x, (char near *)nlabel[e]);
    }
    x = (MAX_COLS - strlen(graph_title)) / 2;
    mvaddstr(YLINES + 3, x, graph_title);
    refresh();
    getch();
    close_curses();
    return (0);
}

void print_graph() {
    int i, e, y, l, r, j;

    fprintf(stdout, "\n");
    for (y = 0; y < YLINES; y++) {
        fprintf(stdout, "%-8s%s", ytxt[y], ":");
        for (e = 0; e < nelements; e++) {
            if (ny[e] > y) {
                for (i = 0; i < nwidth; i++)
                    fprintf(stdout, " ");
                fprintf(stdout, " ");
            } else {
                for (i = 0; i < nwidth; i++)
                    fprintf(stdout, "%c", print_char);
                fprintf(stdout, " ");
            }
        }
        fprintf(stdout, "\n");
    }

    fprintf(stdout, "        +");
    for (e = 0; e < nelements; e++) {
        for (i = 0; i < nwidth; i++)
            fprintf(stdout, "-");
        fprintf(stdout, "+");
    }
    fprintf(stdout, "\n         ");
    for (e = 0; e < nelements; e++) {
        l = (nwidth - strlen((char near *)nlabel[e])) / 2;
        r = nwidth - strlen((char near *)nlabel[e]) - l;
        for (j = 0; j < l; j++)
            fprintf(stdout, " ");
        fprintf(stdout, "%s", nlabel[e]);
        for (j = 0; j < r; j++)
            fprintf(stdout, " ");
        fprintf(stdout, " ");
    }
    fprintf(stdout, "\n\n");
    i = (MAX_COLS - strlen(graph_title)) / 2;
    for (j = 0; j < i; j++)
        fprintf(stdout, " ");
    fprintf(stdout, "%s\n", graph_title);
}

void file_graph() {
    FILE *graph_fp;
    char in_buf[BUFSIZ];
    int i;

    if ((graph_fp = fopen(graph_file_name, "r")) == NULL) {
        strncpy(tmp_str, graph_file_name, MAXLEN);
        strncat(tmp_str, " not found", MAXLEN);
        fprintf(stderr, "%s\n", tmp_str);
        exit(EXIT_FAILURE);
    }
    i = 0;
    while (fgets(in_buf, BUFSIZ, graph_fp) != NULL && i < (NELEMENTS * 2))
        if (graph_idFlag) {
            if (strncmp(in_buf, graph_id, graph_id_len) == 0)
                graph_in[i++] = gn_cpy(in_buf, graph_id_len);
        } else
            graph_in[i++] = gn_cpy(in_buf, graph_id_len);
    fclose(graph_fp);
    graph_in[i] = (char *)0;
}

void stdin_graph() {
    char in_buf[BUFSIZ];
    int i;

    i = 0;
    while ((fgets(in_buf, BUFSIZ, stdin)) != NULL && i < (NELEMENTS * 2))
        if (graph_idFlag) {
            if (strncmp(in_buf, graph_id, graph_id_len) == 0)
                graph_in[i++] = gn_cpy(in_buf, graph_id_len);
        } else
            graph_in[i++] = gn_cpy(in_buf, graph_id_len);
    graph_in[i] = (char *)0;
}

char *gn_cpy(char *s, int pos) {
    char *d, *rs;
    int p, l;

    rs = NULL;
    l = p = 0;
    d = tmp_str;
    while (*s != '\0' && *s != '\n') {
        if (p >= pos) {
            *d++ = *s;
            l++;
        }
        s++;
        p++;
    }
    *d = '\0';
    if (l > 0) {
        d = rs = (char *)malloc(l + 1);
        if (rs != NULL) {
            s = tmp_str;
            while (*s != '\0')
                *d++ = *s++;
            *d = '\0';
        }
    }
    return (rs);
}
