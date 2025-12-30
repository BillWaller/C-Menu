// view.c
// Bill Waller Copyright (c) 2025
// billxwaller@gmail.com

#include "menu.h"
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

int main(int argc, char **argv) {

    capture_shell_tioctl();
    Init *init = new_init(argc, argv);
    mapp_initialization(init, argc, argv);
    view = new_view(init, argc, argv, init->lines, init->cols);
    open_curses(init);
    if (view->lines > 0 && view->cols > 0) {
        mview(init, view->argc, view->argv, view->lines, view->cols, view->begy,
              view->begx, view->title);
    } else if (!init_view_full_screen(init))
        view_file(init);
    close_init(init);
    win_del();
    close_curses();
    exit(EXIT_SUCCESS);
}
