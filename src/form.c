/** @file form.c
    @brief Command Line Start-up for C-Menu Form
    @author Bill Waller
    Copyright (c) 2025
    MIT License
    billxwaller@gmail.com
    @date 2026-02-09
 */

#include <common.h>

__end_pgm;
int main(int argc, char **argv) {
    __atexit;
    capture_shell_tioctl();
    Init *init = new_init(argc, argv);
    SIO *sio = init->sio;
    mapp_initialization(init, argc, argv);
    open_curses(sio);
    sig_prog_mode();
    capture_curses_tioctl();
    win_init_attrs();

    init_form(init, argc, argv, LINES / 14, COLS / 14);

    destroy_init(init);
    win_del();
    destroy_curses();
    return 0;
}
