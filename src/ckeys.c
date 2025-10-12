// ckeys.c
// Test Curses Keys
// Bill Waller */
// billxwaller@gmail.com

#include "menu.h"

int main(int argc, char **argv) {
    int rc;

    if ((rc = initialization(argc, argv)))
        return (rc);
    open_curses();
    int begy = 3;
    int begx = 5;
    ckeys(begy, begx);
    close_curses();
    sig_shell_mode();
    reset_shell_mode();
    return (0);
}
