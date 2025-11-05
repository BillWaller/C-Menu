/*  menu.c
    Bill Waller
    billxwaller@gmail.com

    This is the main file for mapp, a terminal-based application
    launcher and menu system.

    It operates by reading a menu description file and displaying
    a navigable menu in the terminal. Users can select applications
    to launch or scripts to execute.

    Several ancillary files provide supporting functionality, including
    handling terminal I/O settings, managing the menu structure,
    and rendering the interface using the ncurses library.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 */

#include "menu.h"

int main(int argc, char **argv) {
    int begy, begx;
    capture_shell_tioctl();
    Init *init = new_init(argc, argv);
    mapp_initialization(init, argc, argv);
    open_curses();
    sig_prog_mode();
    capture_curses_tioctl();
    win_init_attrs(init->fg_color, init->bg_color, init->bo_color);
    begy = LINES / 14;
    begx = COLS / 14;
    new_menu(init, init->argc, init->argv, begy, begx);
    menu = init->menu;
    parse_menu_description(init);
    menu_engine(init);
    close_curses();
    sig_dfl_mode();
    restore_shell_tioctl();
    return 0;
}
