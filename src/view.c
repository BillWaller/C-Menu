/** @file view.c
    @brief Command Line Start-up for C-Menu View
    @author Bill Waller
    Copyright (c) 2025
    MIT License
    billxwaller@gmail.com
    @date 2026-02-09
 */

#include "common.h"

__end_pgm;
/** @brief Main entry point for C-Menu View
 *  @param argc Argument count
 *  @param argv Argument vector
 *  @return Exit status
 */
int main(int argc, char **argv) {
    __atexit;
    capture_shell_tioctl();
    Init *init = new_init(argc, argv);
    SIO *sio = init->sio;
    mapp_initialization(init, argc, argv);
    open_curses(sio);
    capture_curses_tioctl();
    sig_prog_mode();

    view = new_view(init, argc, argv);
    if (view->lines > 0 && view->cols > 0) {
        mview(init, view->argc, view->argv);
    } else if (!init_view_full_screen(init))
        view_file(init);
    destroy_init(init);
    win_del();
    destroy_curses();
    exit(EXIT_SUCCESS);
}
