/* mview.c
 * file viewer
 * Bill Waller
 * billxwaller@gmail.com
 */
#include "view.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int mview(int, char **, int, int, int, int, char *, int);

int mview(int argc, char **argv, int lines, int cols, int begy, int begx,
          char *title, int attr) {
    extern int optind;
    extern char *optarg;
    /*
     * P: DefPrompt        {S-Short, L-Long, N-None}[String]
     * m  Medium Prompt    for reverse compatibility only
     * M  Long Prompt      for reverse compatibility only
     * c  f_at_end_clear   clear screen at end
     * h  errflg           display command line help
     * i  f_ignore_case    ignore case in search
     * r  f_at_end_remove  remove file at end of program
     * s  f_squeeze        squeeze multiple blank lines
     * t: tabstop          number of spaces in tab
     * w:                  window initialization string
     *                         lllcccLLLCCCC
     * +  startup_cmd_str    Command to execute on startup
     */

    char *optstring = "B:F:O:MP:chimrst:w:+";
    int o;
    int vo;
    char errflg = 0;

    optind = 0;
    init_view_struct();
    view->lines = lines;
    view->cols = cols;
    view->begy = begy;
    view->begx = begx;
    if (view->argc > 0) {
        optind = 0;
        o = vo = getopt(view->argc, view->argv, optstring);
        if (vo == EOF)
            optind = 1;
    } else
        vo = EOF;
    if (vo == EOF)
        o = getopt(argc, argv, optstring);

    while (o != EOF) {
        switch (o) {
        case 'F':
            break;
        case 'B':
            break;
        case 'O':
            break;
        case 'M':
            view->prompt_type = 'L';
            break;
        case 'P':
            switch (*optarg++) {
            case 's':
            case 'S':
                view->prompt_type = 'S';
                break;
            case 'l':
            case 'L':
                view->prompt_type = 'L';
                break;
            case 'n':
            case 'N':
                view->prompt_type = 'N';
                break;
            }
            if (*optarg != '\0')
                strncpy(view->def_prompt_ptr, optarg, MAX_COLS);
            break;

        case 'w':
            parse_geometry_str(optarg, &(view->lines), &(view->cols),
                               &(view->begy), &(view->begx));
            break;

        case '+':
            if (*optarg == '+') {
                optarg++;
                strnz_cpy(view->startup_cmd_str_all_files, optarg, MAXLEN);
                strncat(view->startup_cmd_str_all_files, "\n", MAXLEN);
            }
            strnz_cpy(view->startup_cmd_str, optarg, MAXLEN);
            strncat(view->startup_cmd_str, "\n", MAXLEN);
            break;
        case 'c':
            view->f_at_end_clear = TRUE;
            break;
        case 'i':
            view->f_ignore_case = TRUE;
            break;
        case 'r':
            view->f_at_end_remove = TRUE;
            break;
        case 's':
            view->f_squeeze = TRUE;
            break;
        case 't':
            view->tabstop = atoi(optarg);
            if (view->tabstop < 0)
                view->tabstop = 1;
            if (view->tabstop > 12)
                view->tabstop = 12;
            break;
        case 'h':
        case '?':
        default:
            errflg++;
            break;
        }
        if (vo != EOF) {
            o = vo = getopt(view->argc, view->argv, optstring);
            if (vo == EOF)
                optind = 1;
        }
        if (vo == EOF)
            o = getopt(argc, argv, optstring);
    }
    if (errflg) {
        display_error_message("View failed");
        return (1);
    }
    view->title = title;
    view->argc = 0;
    while (optind < argc) {
        if (view->argc >= MAXARGS) {
            display_error_message("Too many arguments");
            return (1);
        }
        view->argv[view->argc++] = strdup(argv[optind++]);
    }
    view->argv[view->argc] = NULL;
    view->fd = -1;
    view->f_stdout_is_tty = isatty(1);
    view->curr_argc = 0;
    if (!view->f_stdout_is_tty) {
        if (view->argc < 1) {
            if (initialize_file("-") > 0)
                if (view->fd >= 0)
                    cat_file();
        } else {
            while (view->curr_argc < view->argc) {
                if (initialize_file(view->argv[view->curr_argc]) > 0)
                    if (view->fd >= 0)
                        cat_file();
                view->curr_argc++;
            }
        }
        return (1);
    }
    if (open_view_win() == 0) {
        view_file();
        win_del();
    }
    free(view->blk_first);
    free(view);
    return (0);
}
