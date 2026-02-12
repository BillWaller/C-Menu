/** @file pick.c
    @brief Command Line Entry Point for C-Menu Pick
    @author Bill Waller
    Copyright (c) 2025
    MIT License
    billxwaller@gmail.com
    @date 2026-02-09
 */

#include "common.h"

__end_pgm;
/**   @brief Main entry point for C-Menu Pick
    @param argc Argument count
    @param argv Argument vector
    @return Exit status
 */
int main(int argc, char **argv) {
    __atexit;
    capture_shell_tioctl();
    Init *init = new_init(argc, argv);
    SIO *sio = init->sio;
    mapp_initialization(init, argc, argv);
    open_curses(sio);
    sig_prog_mode();
    capture_curses_tioctl();
    win_init_attrs(sio->fg_color, sio->bg_color, sio->bo_color);
    init_pick(init, init->argc, init->argv, 0, 0);
    destroy_init(init);
    destroy_curses();
    return 0;
}
