/* view.c
 * Bill Waller
 * billxwaller@gmail.com
 */

#include "menu.h"
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

int main(int argc, char **argv) {

    capture_shell_tioctl();
    Init *init = new_init(argc, argv);
    mapp_initialization(init, argc, argv);
    int begy = 0;
    int begx = 0;
    view = new_view(init, argc, argv, begy, begx);

    view->f_stdout_is_tty = isatty(1);
    if (!view->f_stdout_is_tty) {
        if (view->argc < 1) {
            if (view_init_input(view, "-"))
                if (view->fp)
                    cat_file(view);
        } else {
            while (view->curr_argc < view->argc) {
                if (view_init_input(view, view->argv[view->curr_argc]))
                    if (view->fp)
                        cat_file(view);
                view->curr_argc++;
            }
        }
        exit(EXIT_SUCCESS);
    }
    open_curses(init);
    if (!init_view_full_screen(init)) {
        view_file(init);
    }
    if (f_curses_open) {
        if (view->f_at_end_clear) {
            wclear(stdscr);
            wrefresh(stdscr);
        }
    }
    close_init(init);
    win_del();
    close_curses();
    return 0;
}
