/// mview.c
//  Bill Waller Copyright (c) 2025
//  MIT License
//  billxwaller@gmail.com
/// Callable Startup for C-Menu View

#include "menu.h"
#include <unistd.h>

int mview(Init *init, int argc, char **argv) {
    view = init->view;
    if (!view)
        view = new_view(init, argc, argv);
    else
        view = init->view;
    ///  ╭───────────────────────────────────────────────────────────╮
    ///  │ view->lines     3/4                                       │
    ///  ╰───────────────────────────────────────────────────────────╯
    if (init->lines == 0)
        view->lines = LINES * 3 / 4;
    else
        view->lines = init->lines;
    if (view->lines > LINES - 3)
        view->lines = LINES - 3;

    ///  ╭───────────────────────────────────────────────────────────╮
    ///  │ view->cols      3/4                                       │
    ///  ╰───────────────────────────────────────────────────────────╯
    if (init->cols == 0)
        view->cols = COLS * 3 / 4;
    else
        view->cols = init->cols;
    if (view->cols > COLS - 3)
        view->cols = COLS - 3;
    ///  ╭───────────────────────────────────────────────────────────╮
    ///  │ view->begy      1/5      top margin                       │
    ///  ╰───────────────────────────────────────────────────────────╯
    if (init->begy == 0)
        view->begy = (LINES - view->lines) / 5;
    else
        view->begy = init->begy;
    if (view->begy + view->lines > LINES - 4)
        view->begy = LINES - view->lines - 2;

    ///  ╭───────────────────────────────────────────────────────────╮
    ///  │ view->begx      1/5      left margin                      │
    ///  ╰───────────────────────────────────────────────────────────╯
    if (init->begx == 0)
        view->begx = (COLS - view->cols) / 5;
    else
        view->begx = init->begx;
    if (view->begx + view->cols > COLS - 4)
        view->begx = COLS - view->cols - 2;
    view->f_full_screen = false;
    if (!init_view_boxwin(init, init->title)) {
        view_file(init);
        win_del();
    }
    destroy_view(init);
    return 0;
}
