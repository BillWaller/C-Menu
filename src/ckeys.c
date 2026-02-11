/** @file menu.c
 *  @brief Test Curses Keys
 *  @author Bill Waller
 *  Copyright (c) 2025
 *  MIT License
 *  billxwaller@gmail.com
 *  @date 2026-02-09
 */

#include "common.h"
#include <sys/types.h>

__end_pgm;

int main(int argc, char **argv) {
    /// Entry point for Curses Keys test
    __atexit;
    int rc;
    capture_shell_tioctl();
    init = new_init(argc, argv);
    if (!init) {
        abend(-1, "malloc failed init (Init)");
    }
    SIO *sio = init->sio;
    mapp_initialization(init, argc, argv);
    open_curses(sio);
    sig_prog_mode();
    win_init_attrs(sio->fg_color, sio->bg_color, sio->bo_color);
    rc = display_curses_keys();
    win_del();
    destroy_curses();
    return (rc);
}
