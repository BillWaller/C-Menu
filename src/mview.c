/* mview.c
 * file viewer
 * Bill Waller
 * billxwaller@gmail.com
 */
#include "menu.h"
#include <stdlib.h>
#include <unistd.h>

int mview(Init *init, int argc, char **argv, int lines, int cols, int begy,
          int begx) {
    /*
     * P: DefPrompt        {S-Short, L-Long, N-None}[String]
     * m  Medium Prompt    for reverse compatibility only
     * M  Long Prompt      for reverse compatibility only
     * c  f_at_end_clear   clear screen at end
     * h  errflg           display command line help
     * i  f_ignore_case    ignore case in search
     * r  f_at_end_remove  remove file at end of program
     * s  f_squeeze        squeeze multiple blank lines
     * t: tab_stop         number of spaces in tab
     * w:                  window initialization string
     *                         lllcccLLLCCCC
     * +  start_cmd        Command to execute on start
     */

    if (!view)
        view = new_view(init, argc, argv, begy, begx);
    else
        view = init->view;

    view->f_stdout_is_tty = isatty(1);
    if (!view->f_stdout_is_tty) {
        if (view->argc < 1) {
            if (view_init_input(view, "-"))
                if (view->fp)
                    cat_file(view);
        } else {
            while (view->curr_argc < view->argc) {
                if (view_init_input(view, view->argv[view->curr_argc]))
                    if (view->fp)
                        cat_file(view);
                view->curr_argc++;
            }
        }
        exit(EXIT_SUCCESS);
    }
    if (!lines || !cols) {
        view->lines = LINES * 3 / 4;
        view->cols = COLS * 3 / 4;
    } else {
        view->lines = lines;
        view->cols = cols;
    }
    if (!begy || !begx) {
        begy = (LINES - view->lines) / 2;
        begx = (COLS - view->cols) / 2;
    }
    view->begy = begy;
    view->begx = begx;
    view->f_full_screen = false;
    if (!init_view_boxwin(view)) {
        view_file(view);
        win_del();
    }
    view = close_view(init);
    return (0);
}
