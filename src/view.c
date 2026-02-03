//  view.c
//  Bill Waller Copyright (c) 2025
//  MIT License
//  Command Line Start-up for C-Menu Menu
//  billxwaller@gmail.com
/// view.c
/// Command Line Start-up for C-Menu View
#include "menu.h"
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

__end_pgm; // Called by atexit

int main(int argc, char **argv) {
    __atexit;
    capture_shell_tioctl();
    Init *init = new_init(argc, argv);
    SIO *sio = init->sio;
    mapp_initialization(init, argc, argv);
    open_curses(sio);
    view = new_view(init, argc, argv);
    if (view->lines > 0 && view->cols > 0) {
        mview(init, view->argc, view->argv);
    } else if (!init_view_full_screen(init))
        view_file(init);
    destroy_init(init);
    win_del();
    destroy_curses();
    exit(EXIT_SUCCESS);
}
