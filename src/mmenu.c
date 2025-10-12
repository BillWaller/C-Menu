/* mmenu.c
 * Bill Waller
 * billxwaller@gmail.com
 */

#include "menu.h"
#include <stdlib.h>
#include <string.h>

int m_menu(int, char **, int, int, int, int, int, int);

int m_menu(int argc, char **argv, int caller, int lines, int cols, int begy,
           int begx, int Attr) {
    char errflg = 0;
    int i, rc;
    menu_ *menu;

    menu = (menu_ *)malloc(sizeof(menu_));
    if (menu == (menu_ *)0) {
        sprintf(tmp_str, "malloc(%lu bytes) failed M\n", sizeof(menu_));
        abend(-1, tmp_str);
    }
    menu->choice_max_len = 0;
    menu->text_max_len = 0;
    menu->option_offset = 0;
    menu->option_max_len = 0;
    menu->item_count = 0;
    menu->line_idx = 0;
    menu->lines = lines;
    menu->cols = cols;
    menu->begy = begy;
    menu->begx = begx;
    menu->title = NULL;
    menu->argc = 0;
    menu->argv[0] = NULL;
    menu->rargv[0] = NULL;
    menu->rargc = 0;

    menu->argv[0] = argv[0];
    menu->argv[1] = strdup(option->mapp_spec);
    menu->argv[2] = (char *)'\0';
    menu->argc = 2;

    strncpy(tmp_str, argv[0], 6);

    normalize_file_spec(tmp_str);
    if (strncmp(tmp_str, "option", 6) == 0) {
        if (argc < 2) {
            display_error_message(
                "usage: option description-file [command-file]");
            free(menu);
            return (1);
        }
        if (read_menu_file(menu)) {
            free(menu);
            return (1);
        }
        menu->caller = C_OPTION;
        rc = disp_menu(menu);
        if (menu->rargc > 0) {
            if (rc) {
                rc = full_screen_fork_exec(menu->rargv);
            } else {
                for (i = 0; i < menu->rargc; i++)
                    fprintf(stderr, "%s\n", menu->rargv[i]);
                free(menu);
                return (-1);
            }
            return (rc);
        }
    } else {
        if (errflg != 0) {
            fprintf(stderr, "\nprogram %s ABEND\n", argv[0]);
            return (1);
        }
        if ((rc = read_menu_file(menu)))
            return (rc);
        menu->caller = C_MAIN;
        rc = disp_menu(menu);
        free(menu);
        return (rc);
    }
    return rc;
}
