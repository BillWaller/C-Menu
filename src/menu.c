/// menu.c
//  Bill Waller Copyright (c) 2025
//  MIT License
//  billxwaller@gmail.com
//  Command Line Start-up for C-Menu Menu
//
/// This is the main file for C-Menu, a terminal-based application
/// launcher and menu system.
///
/// It operates by reading a menu description file and displaying
/// a navigable menu in the terminal. Users can select applications
/// to launch or scripts to execute.
///
/// Several ancillary files provide supporting functionality, including
/// handling terminal initialization IO settings, managing the menu structure,
/// and rendering the interface using the NCurses library.

#include "menu.h"

__end_pgm; // Called by atexit

int main(int argc, char **argv) {
    __atexit;
    int begy, begx;
    capture_shell_tioctl();
    Init *init = new_init(argc, argv);
    SIO *sio = init->sio;
    mapp_initialization(init, argc, argv);
    open_curses(sio);
    sig_prog_mode();
    capture_curses_tioctl();
    win_init_attrs(sio->fg_color, sio->bg_color, sio->bo_color);
    begy = LINES / 14;
    begx = COLS / 14;
    new_menu(init, init->argc, init->argv, begy, begx);
    menu = init->menu;
    parse_menu_description(init);
    /// ╭───────────────────────────────────────────────────────────────────╮
    /// │ MENU_ENGINE                                                       │
    /// ╰───────────────────────────────────────────────────────────────────╯
    menu_engine(init);
    /// ╭───────────────────────────────────────────────────────────────────╮
    /// │ CLEANUP                                                           │
    /// ╰───────────────────────────────────────────────────────────────────╯
    destroy_init(init);
    win_del();
    destroy_curses();
    return 0;
}
