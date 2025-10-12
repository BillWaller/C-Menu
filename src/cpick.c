/* cpick.c
 * pick from a list of choices
 * Bill Waller
 * billxwaller@gmail.com
 */

#include "menu.h"
#include <stdlib.h>
#include <string.h>

void pick_usage(char *);

int main(int argc, char **argv) {

    if (initialization(argc, argv))
        abend(-1, "initialization failed");

    pick = (pick_ *)malloc(sizeof(pick_));
    if (pick == NULL) {
        sprintf(errmsg, "cpick: (pick_ *)P=(opt *)malloc(%ld)\n",
                sizeof(pick_));
        abend(-1, errmsg);
    }
    strcpy(pick->in_file, option->in_file);
    strcpy(pick->out_file, option->out_file);
    pick->win = (WINDOW *)0;
    pick->box = (WINDOW *)0;
    pick->lines = lines;
    pick->cols = cols;
    pick->begy = begy;
    pick->begx = begx;
    pick->argc = argc;
    pick->argv[0] = argv[0];
    pick->title = "pick";

    if (pick->in_file[0]) {
        if ((pick->in_fp = fopen(pick->in_file, "rb")) == NULL) {
            strncpy(tmp_str, "Can't open input ", MAXLEN - 5);
            strncat(tmp_str, pick->in_file, MAXLEN - 5);
            abend(1, tmp_str);
        }
    } else {
        if (argc == 1 || (argv[1][0] == '-' && argv[1][1] == '\0'))
            pick->in_fp = stdin;
        else if ((pick->in_fp = fopen(argv[1], "rb")) == NULL) {
            sprintf(tmp_str, "Can\'t open input %s\n", argv[1]);
            abend(1, tmp_str);
        } else
            pick_usage(argv[0]);
    }
    if (pick->out_file[0]) {
        if ((pick->out_fp = fopen(pick->out_file, "w")) == NULL) {
            strncpy(tmp_str, "Can't open output ", MAXLEN - 5);
            strncat(tmp_str, pick->out_file, MAXLEN - 5);
            abend(1, tmp_str);
        }
    } else {
        if (!pick->cmd_str[0])
            pick->out_fp = stdout;
    }
    /*-------------------------------------------------------------------*/

    open_curses();
    pick_obj();
    pick->win = win_del();
    close_curses();
    free(pick);
    exit(0);
}

void pick_usage(char *pgmid) {
    fprintf(stderr, "usage: %s [-w window initialization string]\n", pgmid);
    fprintf(stderr, "                 lllcccLLLCCC\n");
    fprintf(stderr, "             [-i input file-name]\n");
    fprintf(stderr, "             [-n number of selections]\n");
    fprintf(stderr, "             [-o output file-name]\n\n");
    list_colors();
    abend(1, "Press any key");
}
