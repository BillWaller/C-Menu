// pick.c
// Bill Waller Copyright (c) 2025
// billxwaller@gmail.com
/// pick.c
/// Command Line Start-up for C-Menu Pick
#include "menu.h"
#include <stdlib.h>

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
    sig_prog_mode();
    capture_shell_tioctl();
    Init *init = new_init(argc, argv);
    mapp_initialization(init, argc, argv);
    open_curses(init);
    capture_curses_tioctl();
    win_init_attrs(win, init->fg_color, init->bg_color, init->bo_color);
    init_pick(init, init->argc, init->argv, 0, 0);
    destroy_init(init);
    destroy_curses();
    return 0;
}
