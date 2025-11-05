/*  ckeys.c
    Test Curses Keys
    Bill Waller
    billxwaller @gmail.com
    Ckeys

 */

#include "menu.h"
#include <sys/types.h>

int main(int argc, char **argv) {
    capture_shell_tioctl();
    init = new_init(argc, argv);
    if (!init) {
        abend(-1, "malloc failed init (Init)");
    }
    mapp_initialization(init, argc, argv);
    sig_prog_mode();
    open_curses();
    win_init_attrs(init->fg_color, init->bg_color, init->bo_color);
    display_curses_keys();
    close_curses();
    sig_dfl_mode();
    restore_shell_tioctl();
    return (0);
}
