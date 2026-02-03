//  ckeys.c
//  Bill Waller Copyright (c) 2025
//  MIT License
//  billxwaller@gmail.com
///  Test Curses Keys

#include "cm.h"
#include "menu.h"
#include <sys/types.h>

// __end_pgm; // Called by atexit

int main(int argc, char **argv) {
    // __atexit;
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
