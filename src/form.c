//  form_exec.c
//  Bill Waller Copyright (c) 2025
//  MIT License
//  billxwaller@gmail.com
/// Command Line Start-up for mapp C-Menu Form

#include "menu.h"

int main(int argc, char **argv) {
    capture_shell_tioctl();
    Init *init = new_init(argc, argv);
    mapp_initialization(init, argc, argv);
    open_curses(init);
    win_init_attrs(stdscr, init->fg_color, init->bg_color, init->bo_color);
    int begy = LINES / 14;
    int begx = COLS / 14;
    init_form(init, argc, argv, begy, begx);
    close_init(init);
    win_del();
    close_curses();
    return 0;
}
