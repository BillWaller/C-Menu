/* paint.c
 * Bill Waller
 * billxwaller@gmail.com
 */

#include "menu.h"
#include <stdlib.h>

int main(int argc, char **argv) {
    char *helpFile = NULL;

    if (initialization(argc, argv))
        abend(-1, "initialization failed");
    open_curses();
    paint_form(argv[1], helpFile, begy, begx);
    close_curses();
    exit(0);
}
