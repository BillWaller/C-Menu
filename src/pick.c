/* pick.c
 * Bill Waller
 * billxwaller@gmail.com
 */

#include "menu.h"

int main(int argc, char **argv) {
    sig_prog_mode();
    capture_shell_tioctl();
    Init *init = new_init(argc, argv);
    mapp_initialization(init, argc, argv);
    open_curses(init);
    capture_curses_tioctl();
    win_init_attrs(win, init->fg_color, init->bg_color, init->bo_color);
    init_pick(init, init->argc, init->argv, 0, 0);
    close_init(init);
    close_curses();
    return 0;
}
