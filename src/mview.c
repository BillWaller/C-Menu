// mview.c
// file viewer
// Bill Waller Copyright (c) 2025
// billxwaller@gmail.com

#include "menu.h"
#include <unistd.h>

int mview(Init *init, int argc, char **argv, int lines, int cols, int begy,
          int begx, char *title) {
    if (!view)
        view = new_view(init, argc, argv, begy, begx);
    else
        view = init->view;

    //  ╭───────────────────────────────────────────────────────────╮
    //  │ view->lines     2/3                                       │
    //  ╰───────────────────────────────────────────────────────────╯
    if (init->lines > LINES - 3)
        init->lines = LINES - 3;
    else if (init->lines == 0)
        init->lines = LINES * 3 / 4;
    view->lines = init->lines;

    //  ╭───────────────────────────────────────────────────────────╮
    //  │ view->cols      2/3                                       │
    //  ╰───────────────────────────────────────────────────────────╯
    if (init->cols > COLS - 3)
        init->cols = COLS - 3;
    else if (init->cols == 0)
        init->cols = COLS * 3 / 4;
    view->cols = init->cols;
    //  ╭───────────────────────────────────────────────────────────╮
    //  │ view->cols      1/5      top margin                       │
    //  ╰───────────────────────────────────────────────────────────╯
    if (init->begy + view->lines > LINES - 4)
        init->begy = LINES - view->lines - 2;
    else if (init->begy == 0)
        init->begy = (LINES - view->lines) / 5;
    view->begy = init->begy;

    //  ╭───────────────────────────────────────────────────────────╮
    //  │ view->cols      1/5      left margin                      │
    //  ╰───────────────────────────────────────────────────────────╯
    if (init->begx + view->cols > COLS - 4)
        init->begx = COLS - view->cols - 2;
    else if (init->begx == 0)
        init->begx = (COLS - view->cols) / 2;
    view->begx = init->begx;

    view->f_full_screen = false;
    if (!init_view_boxwin(init, title)) {
        view_file(init);
        win_del();
    }
    close_view(init);
    return 0;
}
