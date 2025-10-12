/* menu.c
 * Bill Waller
 * billxwaller@gmail.com
 */

#include "menu.h"
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    int rc;
    menu_ *menu;

    if ((rc = initialization(argc, argv)))
        exit(rc);

    menu = (menu_ *)malloc(sizeof(menu_));
    if (!menu) {
        snprintf(tmp_str, MAXLEN - 1, "malloc(%ld bytes) failed M\n",
                 sizeof(menu));
        abend(-1, tmp_str);
    }
    menu->choice_max_len = 0;
    menu->text_max_len = 0;
    menu->option_offset = 0;
    menu->option_max_len = 0;
    menu->item_count = 0;
    menu->line_idx = 0;
    menu->lines = 0;
    menu->cols = 0;
    menu->title = NULL;
    menu->begy = 2;
    menu->begx = 4;
    menu->argc = 0;
    menu->argv[0] = NULL;
    menu->rargv[0] = NULL;
    menu->rargc = 0;
    menu->argv[0] = strdup(argv[0]);
    menu->argv[1] = strdup(option->mapp_spec);
    menu->argv[2] = (char *)0;
    menu->argc = 3;

    open_curses();
    if (read_menu_file(menu))
        abend(-1, "read_menu_file failed");
    wclear(stdscr);
    menu->caller = C_MAIN;
    disp_menu(menu);
    close_curses();
    free(menu);
    exit(0);
}
