/* form_exec.c
 * Bill Waller
 * billxwaller@gmail.com
 */

#include "menu.h"

int main(int argc, char **argv) {
    capture_shell_tioctl();
    Init *init = new_init(argc, argv);
    mapp_initialization(init, argc, argv);
    open_curses(init);
    win_init_attrs(init->fg_color, init->bg_color, init->bo_color);
    int begy = LINES / 14;
    int begx = COLS / 14;
    init->form = new_form(init, argc, argv, begy, begx);
    form = init->form;
    form_process(init);
    close_curses();
    restore_shell_tioctl();
    return 0;
}
