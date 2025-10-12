/* mpick.c
 * Bill Waller
 * billxwaller@gmail.com
 */

#include "menu.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

pick_ *pick;

int Mpick(int, char **, int, int, int, int, char *, int);

int Mpick(int argc, char **argv, int lines, int cols, int begy, int begx,
          char *title, int attr) {
    extern int optind;
    extern char *optarg;

    pick = (pick_ *)malloc(sizeof(pick_));
    if (pick == NULL) {
        sprintf(errmsg, "mpick.c, malloc(%ld) failed\n", sizeof(pick_));
        abend(-1, errmsg);
    }
    pick->win = (WINDOW *)0;
    pick->lines = lines;
    pick->cols = cols;
    pick->begy = begy + 1;
    pick->begx = begx + 4;
    pick->f_append_cmd_args = 0;
    pick->argc = 0;
    pick->argv[0] = NULL;
    pick->title = title;
    pick->in_file[0] = '\0';
    pick->out_file[0] = '\0';
    pick->cmd_str[0] = '\0';
    pick->select_cnt = 0;
    opt_process_cmdline(argc, argv);

    if (option->cmd_str[0])
        strcpy(pick->cmd_str, option->cmd_str);
    if (option->in_file[0])
        strcpy(pick->in_file, option->in_file);
    if (option->out_file[0])
        strcpy(pick->out_file, option->out_file);
    else {
        if (option->in_file[0])
            strcpy(pick->out_file, option->in_file);
        else
            strcpy(pick->out_file, "default");
        strcat(pick->out_file, ".out");
    }

    if (pick->in_file[0]) {
        if ((pick->in_fp = fopen(pick->in_file, "rb")) == NULL) {
            strncpy(tmp_str, "Can\'t open input ", MAXLEN - 25);
            strncat(tmp_str, pick->in_file, MAXLEN - 25);
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
    if (pick->out_file[0]) {
        if ((pick->out_fp = fopen(pick->out_file, "w")) == NULL) {
            strncpy(tmp_str, "Can\'t open output ", MAXLEN - 5);
            strncat(tmp_str, pick->out_file, MAXLEN - 5);
            display_error_message(tmp_str);
            return (1);
        }
    } else {
        if (pick->cmd_str[0]) {
            pick->out_fp = stdout;
        }
    }

    pick_obj();
    if (pick->win) {
        win_del();
        pick->win = win_win[win_ptr];
        pick->box = win_box[win_ptr];
    }
    free(pick);
    return 0;
}
