//  form_exec.c
//  Bill Waller Copyright (c) 2025
//  MIT License
//  billxwaller@gmail.com
/// Command Line Start-up for C-Menu Form

#include "menu.h"

__end_pgm; // Called by atexit

int main(int argc, char **argv) {
    __atexit;
    capture_shell_tioctl();
    Init *init = new_init(argc, argv);
    SIO *sio = init->sio;
    mapp_initialization(init, argc, argv);
    open_curses(sio);
    win_init_attrs(sio->fg_color, sio->bg_color, sio->bo_color);
    int begy = LINES / 14;
    int begx = COLS / 14;
    init_form(init, argc, argv, begy, begx);
    destroy_init(init);
    win_del();
    destroy_curses();
    return 0;
}
