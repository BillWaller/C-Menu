/* init_pick.c
 * Bill Waller
 * billxwaller@gmail.com
 */

#include "menu.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int init_pick(Init *init, int argc, char **argv, int lines, int cols, int begy,
              int begx) {
    Pick *pick = new_pick(init, argc, argv, begy, begx);
    pick->lines = lines;
    pick->cols = cols;
    pick->begy = begy + 1;
    pick->begx = begx + 4;
    strncpy(pick->title, init->title, MAXLEN - 1);
    if (pick->f_in_spec) {
        if ((pick->in_fp = fopen(pick->in_spec, "rb")) == NULL) {
            strncpy(tmp_str, "Can\'t open input ", MAXLEN - 25);
            strncat(tmp_str, pick->in_spec, MAXLEN - 25);
            display_error_message(tmp_str);
            return (1);
        }
    } else if (argc == 1 || (argv[1][0] == '-' && argv[1][1] == '\0')) {
        if (isatty(0)) {
            display_error_message("pick: Can't take input from tty");
            return (1);
        }
        pick->in_fp = stdin;
    }
    if (pick->out_spec[0]) {
        if ((pick->out_fp = fopen(pick->out_spec, "w")) == NULL) {
            strncpy(tmp_str, "Can\'t open output ", MAXLEN - 5);
            strncat(tmp_str, pick->out_spec, MAXLEN - 5);
            display_error_message(tmp_str);
            return (1);
        }
    } else {
        if (pick->cmd_spec[0]) {
            pick->out_fp = stdout;
        }
    }
    pick_engine(init);
    if (pick->win) {
        win_del();
        pick->win = win_win[win_ptr];
        pick->box = win_box[win_ptr];
    }
    free(pick);
    return 0;
}
