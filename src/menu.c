/*  menu.c
    Bill Waller
    billxwaller@gmail.com

    This is the main file for C-Menu, a terminal-based application
    launcher and menu system.

    It operates by reading a menu description file and displaying
    a navigable menu in the terminal. Users can select applications
    to launch or scripts to execute.

    Several ancillary files provide supporting functionality, including
    handling terminal I/O settings, managing the menu structure,
    and rendering the interface using the ncurses library.
 */

#include "menu.h"

int main(int argc, char **argv) {
    int begy, begx;
    capture_shell_tioctl();
    Init *init = new_init(argc, argv);
    mapp_initialization(init, argc, argv);
    open_curses(init);
    sig_prog_mode();
    capture_curses_tioctl();
    win_init_attrs(stdscr, init->fg_color, init->bg_color, init->bo_color);
    begy = LINES / 14;
    begx = COLS / 14;
    new_menu(init, init->argc, init->argv, begy, begx);
    menu = init->menu;
    parse_menu_description(init);
    /* ╭───────────────────────────────────────────────────────────────────╮
       │ MENU_ENGINE                                                       │
       ╰───────────────────────────────────────────────────────────────────╯ */
    menu_engine(init);
    close_curses();
    sig_dfl_mode();
    restore_shell_tioctl();
    return 0;
}
