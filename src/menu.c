/** @file menu.c
    @brief Command line start-up for C-Menu
    @author Bill Waller
    Copyright (c) 2025
    MIT License
    billxwaller@gmail.com
    @date 2026-02-09
 */

#include <common.h>

/** @brief This is the main file for C-Menu, a terminal-based application
   launcher and menu system.
    @note It operates by reading a menu description file and displaying a
   navigable menu in the terminal. Users can select applications to launch or
   scripts to execute.
    @note Several ancillary files provide supporting functionality, including
   handling terminal initialization IO settings, managing the menu structure,
   and rendering the interface using the NCurses library. */
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
    win_init_attrs(sio->fg_color, sio->bg_color, sio->bo_color);

    new_menu(init, init->argc, init->argv, LINES / 14, COLS / 14);
    menu = init->menu;
    parse_menu_description(init);
    menu_engine(init);

    destroy_init(init);
    win_del();
    destroy_curses();
    return 0;
}
