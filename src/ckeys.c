//  ckeys.c
//  Bill Waller Copyright (c) 2025
//  MIT License
//  billxwaller @gmail.com
///  Test Curses Keys

#include "menu.h"
#include <sys/types.h>

__end_pgm; // Called by atexit

int main(int argc, char **argv) {
    __atexit;
    capture_shell_tioctl();
    init = new_init(argc, argv);
    if (!init) {
        abend(-1, "malloc failed init (Init)");
    }
    mapp_initialization(init, argc, argv);
    sig_prog_mode();
    open_curses(init);
    win_init_attrs(stdscr, init->fg_color, init->bg_color, init->bo_color);
    display_curses_keys();
    win_del();
    destroy_curses();
    return (0);
}
