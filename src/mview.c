/* mview.c
 * file viewer
 * Bill Waller
 * billxwaller@gmail.com
 */
#include "menu.h"
#include <stdlib.h>
#include <unistd.h>

int mview(Init *init, int argc, char **argv, int lines, int cols, int begy,
          int begx, char *title) {
    if (!view)
        view = new_view(init, argc, argv, begy, begx);
    else
        view = init->view;

    view->f_stdout_is_tty = isatty(1);
    if (!view->f_stdout_is_tty) {
        if (view->argc < 1) {
            if (view_init_input(view, "-"))
                if (view->buf)
                    cat_file(view);
        } else {
            while (view->curr_argc < view->argc) {
                if (view_init_input(view, view->argv[view->curr_argc]))
                    if (view->buf)
                        cat_file(view);
                view->curr_argc++;
            }
        }
        exit(EXIT_SUCCESS);
    }

    if (init->provider_cmd[0] != '\0')
        strip_quotes(init->provider_cmd);
    if (init->title[0] == '\0')
        strnz__cpy(init->title, init->provider_cmd, MAXLEN - 1);
    else
        strip_quotes(init->title);

    /*  ╭───────────────────────────────────────────────────────────────╮
        │ view->lines     2/3                                           │
        ╰───────────────────────────────────────────────────────────────╯ */
    if (init->lines > LINES - 3)
        init->lines = LINES - 3;
    else if (init->lines == 0)
        init->lines = LINES * 3 / 4;
    view->lines = init->lines;

    /*  ╭───────────────────────────────────────────────────────────────╮
        │ view->cols      2/3                                           │
        ╰───────────────────────────────────────────────────────────────╯ */
    if (init->cols > COLS - 3)
        init->cols = COLS - 3;
    else if (init->cols == 0)
        init->cols = COLS * 3 / 4;
    view->cols = init->cols;
    /*  ╭───────────────────────────────────────────────────────────────╮
        │ view->cols      1/5      top margin                           │
        ╰───────────────────────────────────────────────────────────────╯ */
    if (init->begy + view->lines > LINES - 4)
        init->begy = LINES - view->lines - 2;
    else if (init->begy == 0)
        init->begy = (LINES - view->lines) / 5;
    view->begy = init->begy;

    /*  ╭───────────────────────────────────────────────────────────────╮
        │ view->cols      1/5      left margin                          │
        ╰───────────────────────────────────────────────────────────────╯ */
    if (init->begx + view->cols > COLS - 4)
        init->begx = COLS - view->cols - 2;
    else if (init->begx == 0)
        init->begx = (COLS - view->cols) / 8;
    view->begx = init->begx;

    view->f_full_screen = false;
    if (!init_view_boxwin(init, title)) {
        view_file(init);
        win_del();
    }
    close_view(init);
    return (0);
}
