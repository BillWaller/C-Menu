/** @file main.c
    @brief Command line start-up for C-Menu components
    @author Bill Waller
    Copyright (c) 2025
    MIT License
    billxwaller@gmail.com
    @date 2026-02-09
 */

#include <common.h>
#include <string.h>

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
        parse_menu_description(init);
        menu_engine(init);
    } else if (!strcmp(pgm_name, "form")) {
        init_form(init, argc, argv, LINES / 14, COLS / 14);
    } else if (!strcmp(pgm_name, "pick")) {
        init_pick(init, init->argc, init->argv, 0, 0);
    } else if (!strcmp(pgm_name, "view")) {
        view = new_view(init, argc, argv);
        if (view->lines > 0 && view->cols > 0)
            mview(init, view->argc, view->argv);
        else if (!init_view_full_screen(init))
            view_file(init);
    } else if (!strcmp(pgm_name, "ckeys")) {
        display_curses_keys();
    }

    destroy_init(init);
    win_del();
    destroy_curses();
    return 0;
}
