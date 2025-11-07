/* pick.c
 * Bill Waller
 * billxwaller@gmail.com
 */

#include "menu.h"

int main(int argc, char **argv) {
    int begy, begx;
    sig_prog_mode();
    capture_shell_tioctl();
    Init *init = new_init(argc, argv);
    mapp_initialization(init, argc, argv);
    open_curses();
    capture_curses_tioctl();
    win_init_attrs(init->fg_color, init->bg_color, init->bo_color);
    begy = 0;
    begx = 0;
    new_pick(init, init->argc, init->argv, begy, begx);
    pick = init->pick;
    init_pick(init, init->argc, init->argv, begy, begx);
    close_curses();
    restore_shell_tioctl();
    return 0;
}
