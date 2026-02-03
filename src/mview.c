//  mview.c
//  Bill Waller Copyright (c) 2025
//  MIT License
//  billxwaller@gmail.com
///  Startup for C-Menu View

#include "menu.h"
#include <unistd.h>

int mview(Init *init, int argc, char **argv) {
    view = init->view;
    if (!view)
        view = new_view(init, argc, argv);
    else
        view = init->view;

    ///  ╭───────────────────────────────────────────────────────────╮
    ///  │ view->lines     2/3                                       │
    ///  ╰───────────────────────────────────────────────────────────╯
    if (init->lines > LINES - 3)
        init->lines = LINES - 3;
    init->lines = LINES * 3 / 4;
    view->lines = init->lines;

    ///  ╭───────────────────────────────────────────────────────────╮
    ///  │ view->cols      2/3                                       │
    ///  ╰───────────────────────────────────────────────────────────╯
    if (init->cols > COLS - 3)
        init->cols = COLS - 3;
    init->cols = COLS * 3 / 4;
    view->cols = init->cols;
    ///  ╭───────────────────────────────────────────────────────────╮
    ///  │ view->cols      1/5      top margin                       │
    ///  ╰───────────────────────────────────────────────────────────╯
    if (init->begy + view->lines > LINES - 4)
        init->begy = LINES - view->lines - 2;
    init->begy = (LINES - view->lines) / 5;
    view->begy = init->begy;

    ///  ╭───────────────────────────────────────────────────────────╮
    ///  │ view->cols      1/5      left margin                      │
    ///  ╰───────────────────────────────────────────────────────────╯
    if (init->begx + view->cols > COLS - 4)
        init->begx = COLS - view->cols - 2;
    init->begx = (COLS - view->cols) / 2;
    view->begx = init->begx;

    view->f_full_screen = false;
    if (!init_view_boxwin(init, init->title)) {
        view_file(init);
        win_del();
    }
    destroy_view(init);
    return 0;
}
