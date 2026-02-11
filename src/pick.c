/** @file pick.c
 *  @brief Command Line Entry Point for C-Menu Pick
 *  @author Bill Waller
 *  Copyright (c) 2025
 *  MIT License
 *  billxwaller@gmail.com
 *  @date 2026-02-09
 */

#include "common.h"

__end_pgm;

int main(int argc, char **argv) {
    __atexit;
    sig_prog_mode();
    capture_shell_tioctl();
    Init *init = new_init(argc, argv);
    SIO *sio = init->sio;
    mapp_initialization(init, argc, argv);
    open_curses(sio);
    capture_curses_tioctl();
    win_init_attrs(sio->fg_color, sio->bg_color, sio->bo_color);
    init_pick(init, init->argc, init->argv, 0, 0);
    destroy_init(init);
    destroy_curses();
    return 0;
}
