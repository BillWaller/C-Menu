//  pick.c
//  Bill Waller Copyright (c) 2025
//  MIT License
//  billxwaller@gmail.com
/// pick.c
/// Command Line Start-up for C-Menu Pick
#include "menu.h"

__end_pgm; // Called by atexit

int main(int argc, char **argv) {
    __atexit;
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
