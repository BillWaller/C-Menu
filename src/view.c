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

static void end_pgm(void) {
    destroy_init(init);
    win_del();
    destroy_curses();
    restore_shell_tioctl();
}

int main(int argc, char **argv) {
    int rc;
    rc = atexit(end_pgm);
    if (rc != 0) {
        fprintf(stderr, "\nCannot set exit function\n");
        exit(EXIT_FAILURE);
    }
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
    destroy_init(init);
    win_del();
    destroy_curses();
    exit(EXIT_SUCCESS);
}
