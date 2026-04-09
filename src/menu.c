/** @file menu.c
    @brief Command line start-up for C-Menu components
    @author Bill Waller
    Copyright (c) 2025
    MIT License
    billxwaller@gmail.com
    @date 2026-02-09
 */

#include <common.h>
#include <string.h>

int mview(Init *);

__end_pgm;
int main(int argc, char **argv) {
    __atexit;
    char pgm_name[MAXLEN];
    capture_shell_tioctl();
    Init *init = new_init(argc, argv);
    SIO *sio = init->sio;
    mapp_initialization(init, argc, argv);
    open_curses(sio);
    sig_prog_mode();
    capture_curses_tioctl();
    win_init_attrs();

    base_name(pgm_name, argv[0]);

    if (!strcmp(pgm_name, "menu")) {
        new_menu(init, init->argc, init->argv, LINES / 14, COLS / 14);
        menu = init->menu;
        menu_engine(init);
    } else if (!strcmp(pgm_name, "form")) {
        init_form(init, init->argc, init->argv, LINES / 14, COLS / 14);
    } else if (!strcmp(pgm_name, "pick")) {
        init_pick(init, init->argc, init->argv, 0, 0);
    } else if (!strcmp(pgm_name, "view")) {
        view = new_view(init);
        if (init->lines > 0 || init->cols > 0)
            mview(init);
        else if (!init_view_full_screen(init))
            view_file(init);
    } else if (!strcmp(pgm_name, "ckeys")) {
        popup_ckeys();
    }
    destroy_init(init);
    win_del();
    destroy_curses();
    return 0;
}

int mview(Init *init) {
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
    if (view->begx + view->cols > COLS - 1)
        view->begx = COLS - view->cols - 1;
    view->f_full_screen = false;
    if (!init_view_boxwin(init, init->title)) {
        view_file(init);
        win_del();
    }
    destroy_view(init);
    return 0;
}
