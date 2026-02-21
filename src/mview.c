/** @file mview.c
    @brief Callable Startup for C-Menu View
    @author Bill Waller
    Copyright (c) 2025
    MIT License
    billxwaller@gmail.com
    @date 2026-02-09
 */

#include "common.h"
#include <unistd.h>

/** @brief subprogram entry point for C-Menu View
 *  @param init Pointer to Init structure containing initialization parameters
 *  @param argc Argument count from command line
 *  @param argv Argument vector from command line
 *  @return 0 on success, non-zero on failure
 *  @note use this function to start the C-Menu View from another program,
 * passing in the necessary initialization parameters through the Init
 * structure. The function will handle setting up the view based on the provided
 * parameters and will return 0 on success or a non-zero value if an error
 * occurs during initialization.
 */
int mview(Init *init, int argc, char **argv) {
    view = init->view;
    if (!view)
        view = new_view(init, argc, argv);
    else
        view = init->view;
    if (init->lines != 0 && view->lines == 0)
        view->lines = init->lines;
    if (view->lines == 0)
        view->lines = LINES * 3 / 4;
    if (view->lines > LINES - 3)
        view->lines = LINES - 3;
    if (init->cols != 0 && view->lines == 0)
        view->cols = init->cols;
    if (view->cols == 0)
        view->cols = COLS * 3 / 4;
    if (view->cols > COLS - 3)
        view->cols = COLS - 3;
    if (init->begy != 0 && view->begy == 0)
        view->begy = init->begy;
    if (view->begy == 0)
        view->begy = (LINES - view->lines) / 5;
    if (view->begy + view->lines > LINES - 2)
        view->begy = LINES - view->lines - 2;
    if (init->begx != 0 && view->begx == 0)
        view->begx = init->begx;
    if (view->begx == 0)
        view->begx = (COLS - view->cols) / 5;
    if (view->begx + view->cols > COLS - 2)
        view->begx = COLS - view->cols - 2;
    view->f_full_screen = false;
    if (!init_view_boxwin(init, init->title)) {
        view_file(init);
        win_del();
    }
    destroy_view(init);
    return 0;
}
