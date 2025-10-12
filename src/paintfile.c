/* paintfile.c
 * Bill Waller
 * billxwaller@gmail.com
 */

#include "menu.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv) {
    int i, j;
    extern int optind;
    extern char *optarg;
    char *optstring = "F:B:O:w:s";
    int Eo;
    int o;
    char errflg = 0;
    char *e, *d;
    int Eargc;
    char *Eargv[MAXARGS];
    int lines, cols, begy = 2, begx = 2;
    char *helpFile, *AnswerFile;

    init_paint_struct();
    e = getenv("MARGS");
    if (e == NULL || *e == '\0')
        e = MARGS;
    d = strdup(e);
    Eargc = str_to_args(Eargv, d);
    if (Eargc > 0) {
        optind = 0;
        o = Eo = getopt(Eargc, Eargv, optstring);
        if (Eo == EOF)
            optind = 1;
    } else
        Eo = EOF;
    if (Eo == EOF)
        o = getopt(argc, argv, optstring);

    while (o != EOF) {
        switch (o) {
        case 'F':
            option->fg_color = get_color_number(optarg);
            if (option->fg_color == -1)
                errflg++;
            break;
        case 'B':
            option->bg_color = get_color_number(optarg);
            if (option->bg_color == -1)
                errflg++;
            break;
        case 'O':
            option->bo_color = get_color_number(optarg);
            if (option->bo_color == -1)
                errflg++;
            break;
        case 's':
            f_stop_on_error = TRUE;
            break;
        case 'w':
            parse_geometry_str(optarg, &lines, &cols, &begx, &begy);
            break;
        default:
            errflg++;
            break;
        }
        if (Eo != EOF) {
            o = Eo = getopt(Eargc, Eargv, optstring);
            if (Eo == EOF)
                optind = 1;
        }
        if (Eo == EOF)
            o = getopt(argc, argv, optstring);
    }

    if (argc < 2 || argc > 4 || errflg) {
        fprintf(stderr,
                "usage: paintfile description-file help-file answer-file\n");
        fprintf(stderr, "                 [-s] (stop on error)\n\n");
        list_colors();
        exit(1);
    }
    build_color_opt_str();
    i = optind;
    j = 1;
    while (i < argc)
        argv[j++] = argv[i++];
    argv[j] = (char *)0;
    argc = j;
    if (argc > 2)
        helpFile = argv[2];
    else
        helpFile = NULL;
    if (argc == 3)
        AnswerFile = NULL;
    else
        AnswerFile = argv[3];
    open_curses();
    if (argc == 2 || argc == 3)
        exit_code = paintfile(argv[1], helpFile, begy, begx);
    else
        exit_code = paintfile(argv[1], AnswerFile, begy, begx);
    close_curses();
    exit(exit_code);
}
