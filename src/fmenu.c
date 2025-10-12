/* fmenu.c
 * menu structures for MENU
 * Bill Waller
 * billxwaller@gmail.com
 * file system interface
 */

#include "menu.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

int disp_menu(menu_ *);
int menu_loop(menu_ *);

int disp_menu(menu_ *menu) {
    int action;
    int i;
    action = MA_INIT;
    while (action == MA_INIT) {
        action = MA_DISPLAY_MENU;
        if (win_new(menu->lines, menu->cols, menu->begy, menu->begx,
                    menu->title)) {
            sprintf(tmp_str, "win_new(%d, %d, %d, %d, %s) failed", menu->lines,
                    menu->cols, menu->begy, menu->begx, menu->title);
            display_error_message(tmp_str);
            return (1);
        }
        menu->win = win_win[win_ptr];
        menu->box = win_box[win_ptr];
        while (action == MA_DISPLAY_MENU) {
            for (menu->line_idx = 0; menu->line_idx < menu->item_count;
                 menu->line_idx++) {
                mvwaddstr(menu->win, menu->line_idx, 0,
                          menu->line[menu->line_idx]->choice_text);
                if (menu->line[menu->line_idx]->option_cnt != 0)
                    mvwaddstr(menu->win, menu->line_idx, menu->option_offset,
                              menu->line[menu->line_idx]->option_ptr
                                  [menu->line[menu->line_idx]->option_idx]);
            }
            menu->line_idx = 0;
            for (i = 0; i < menu->item_count; i++)
                if (menu->line[i]->type == MT_CHOICE) {
                    menu->line_idx = i;
                    break;
                }
            while ((action = menu_loop(menu)) == MA_ENTER_OPTION)
                ;
        }
        win_del();
        menu->win = win_win[win_ptr];
        menu->box = win_box[win_ptr];
        if (action == MA_INIT) {
            for (i = 0; i < menu->item_count; i++)
                free_menu_line(menu->line[i]);
            menu->lines = 0;
            menu->cols = 0;
            menu->line_idx = 0;
            menu->item_count = 0;
            free(menu->title);
            menu->title = NULL;
            menu->choice_max_len = 0;
            menu->text_max_len = 0;
            menu->option_offset = 0;
            menu->option_max_len = 0;
            if (read_menu_file(menu))
                return (MA_RETURN);
        }
    }
    return (action);
}

int menu_loop(menu_ *menu) {
    int eargc;
    char *eargv[MAXARGS];
    char earg_str[MAXLEN];
    int i, j, rc;
    char *d;
    int begy, begx;
    int in_key;
    int winy, winx;

    wattron(menu->win, A_REVERSE);
    mvwaddstr(menu->win, menu->line_idx, 0,
              menu->line[menu->line_idx]->choice_text);
    wattroff(menu->win, A_REVERSE);
    touchwin(win_box[win_ptr]);
    wnoutrefresh(win_box[win_ptr]);
    touchwin(win_win[win_ptr]);
    wnoutrefresh(win_win[win_ptr]);
    mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION | NCURSES_BUTTON_CLICKED,
              NULL);
    MEVENT event;
    in_key = mvwgetch(menu->win, menu->line_idx, 1);
    mvwaddstr(menu->win, menu->line_idx, 0,
              menu->line[menu->line_idx]->choice_text);
    switch (in_key) {
    case key_up:
    case KEY_UP:
        i = menu->line_idx;
        while (i > 0) {
            i--;
            if (menu->line[i]->type == MT_CHOICE) {
                menu->line_idx = i;
                break;
            }
        }
        return (MA_ENTER_OPTION);
    case key_down:
    case KEY_DOWN:
        i = menu->line_idx;
        while (i < menu->item_count - 1) {
            i++;
            if (menu->line[i]->type == MT_CHOICE) {
                menu->line_idx = i;
                break;
            }
        }
        return (MA_ENTER_OPTION);
    case key_cr:
    case KEY_ENTER:
        break;
    case key_ctld:
    case KEY_BREAK:
    case KEY_DL:
        return (MA_RETURN_MAIN);
    case key_ctlp:
        d = getenv("PRTCMD");
        if (d == NULL || *d == '\0')
            strncpy(earg_str, VIEWPRTCMD, MAXLEN - 1);
        else
            strncpy(earg_str, d, MAXLEN - 1);
        strncat(earg_str, " ", MAXLEN - 1);
        d = getenv("PRTFILE");
        if (d == NULL || *d == '\0') {
            d = getenv("HOME");
            if (d == NULL || *d == '\0')
                strncat(earg_str, VIEWPRTFILE, MAXLEN - 1);
            else {
                strncat(earg_str, d, MAXLEN - 1);
                strncat(earg_str, "/", MAXLEN - 1);
                strncat(earg_str, VIEWPRTFILE, MAXLEN - 1);
            }
        } else
            strncat(earg_str, d, MAXLEN - 1);
        full_screen_shell(earg_str);
        return (MA_DISPLAY_MENU);
    case key_ctlr:
        restore_wins();
        return (MA_DISPLAY_MENU);
    case key_ctle:
        d = getenv("EDITOR");
        if (d == NULL || *d == '\0')
            strncpy(earg_str, DEFAULTEDITOR, MAXLEN - 1);
        else
            strncpy(earg_str, d, MAXLEN - 1);
        eargv[0] = earg_str;
        eargv[1] = menu->argv[1];
        eargv[2] = NULL;
        eargc = 2;
        full_screen_fork_exec(eargv);
        return (MA_INIT);
    case KEY_MOUSE:
        if (getmouse(&event) == OK) {
            if (event.bstate == BUTTON1_CLICKED) {
                if (wenclose(menu->win, event.y, event.x)) {
                    winy = event.y;
                    winx = event.x;
                    if (wmouse_trafo(menu->win, &winy, &winx, FALSE)) {
                        menu->line_idx = winy;
                    }
                }
            } else
                return (MA_ENTER_OPTION);
        }
        in_key = 0;
        break;
    default:
        for (i = 0; i < menu->item_count; i++) {
            if (menu->line[i]->raw_text[0] == '_') {
                if (menu->line[i]->choice_letter == (char)in_key) {
                    menu->line_idx = i;
                    break;
                }
            } else {
                if (menu->line[i]->choice_letter == toupper((char)in_key)) {
                    menu->line_idx = i;
                    break;
                }
            }
        }
        if (i >= menu->item_count)
            return (MA_ENTER_OPTION);
    }

    switch ((int)menu->line[menu->line_idx]->command_type) {

    case CT_RETURNMAIN:
        return (MA_RETURN_MAIN);

    case CT_EXEC:
        strncpy(earg_str, menu->line[menu->line_idx]->command_str, MAXLEN - 1);
        eargc = str_to_args(eargv, earg_str);
        j = 0;
        for (i = 1; i < eargc && eargv[i] != NULL; i++)
            eargv[j++] = eargv[i];
        eargv[j] = NULL;
        eargc = j;
        full_screen_fork_exec(eargv);
        return (MA_DISPLAY_MENU);

    case CT_HELP:
        strncpy(earg_str, menu->line[menu->line_idx]->command_str, MAXLEN - 1);
        eargc = str_to_args(eargv, earg_str);
        eargv[0] = VIEWHELPCMD;
        begy = menu->begy + 1;
        begx = menu->begx + 4;
        if (begy > 10)
            begy = 10;
        if (begx > 10)
            begx = 10;
        mview(eargc, eargv, 10, 68, begy, begx,
              menu->line[menu->line_idx]->raw_text, win_attr);
        return (MA_DISPLAY_MENU);

    case CT_MENU:
        strncpy(earg_str, menu->line[menu->line_idx]->command_str, MAXLEN - 1);
        eargc = str_to_args(eargv, earg_str);
        if (eargc == 0)
            return (MA_DISPLAY_MENU);
        rc = m_menu(eargc, eargv, C_MENU, 10, 40, menu->begy + 1,
                    menu->begx + 4, win_attr);
        if (rc == MA_RETURN_MAIN)
            if (menu->caller != C_MAIN)  /* if not called by first menu */
                return (MA_RETURN_MAIN); /* go all the way back         */
        return (MA_DISPLAY_MENU);

    case CT_PICK:
        strncpy(earg_str, menu->line[menu->line_idx]->command_str, MAXLEN - 1);
        eargc = str_to_args(eargv, earg_str);
        Mpick(eargc, eargv, 10, 40, menu->begy + 1, menu->begx + 4,
              menu->line[menu->line_idx]->raw_text, win_attr);
        return (MA_DISPLAY_MENU);

    case CT_PAINT:
        strncpy(earg_str, menu->line[menu->line_idx]->command_str, MAXLEN - 1);
        eargc = str_to_args(eargv, earg_str);
        paint_form(eargv[1], eargv[2], menu->begy + 1, menu->begx + 4);
        return (MA_DISPLAY_MENU);

    case CT_PAINTFILE:
        strncpy(earg_str, menu->line[menu->line_idx]->command_str, MAXLEN - 1);
        eargc = str_to_args(eargv, earg_str);
        paintfile(eargv[1], eargv[2], menu->begy + 1, menu->begx + 4);
        return (MA_DISPLAY_MENU);

    case CT_VIEW:
        strncpy(earg_str, menu->line[menu->line_idx]->command_str, MAXLEN - 1);
        eargc = str_to_args(eargv, earg_str);
        begy = menu->begy + 1;
        begx = menu->begx + 4;
        if (begy > 10)
            begy = 10;
        if (begx > 10)
            begx = 10;
        mview(eargc, eargv, 10, 68, begy, begx,
              menu->line[menu->line_idx]->raw_text, win_attr);
        return (MA_DISPLAY_MENU);

    case CT_CKEYS:
        ckeys(menu->begy + 1, menu->begx + 4);
        return (MA_DISPLAY_MENU);

    case CT_RETURN:
        menu->argc = 0;
        return (MA_RETURN);

    case CT_TOGGLE:
        if (menu->line[menu->line_idx]->option_idx <
            menu->line[menu->line_idx]->option_cnt)
            menu->line[menu->line_idx]->option_idx++;
        else
            menu->line[menu->line_idx]->option_idx = 0;
        mvwaddstr(menu->win, menu->line_idx, menu->option_offset,
                  menu->line[menu->line_idx]
                      ->option_ptr[menu->line[menu->line_idx]->option_idx]);
        wclrtoeol(menu->win);
        return (MA_ENTER_OPTION);

    case CT_WRITE_CONFIG:
        write_config();
        return (MA_ENTER_OPTION);

    default:
        strncpy(earg_str, menu->line[menu->line_idx]->command_str, MAXLEN - 1);
        for (i = 0; i < menu->item_count; i++) {
            if (menu->line[i]->option_idx != 0) {
                strncat(earg_str, " ", MAXLEN - 1);
                strncat(earg_str,
                        menu->line[i]->option_ptr[menu->line[i]->option_idx],
                        MAXLEN - 1);
            }
        }
        full_screen_shell(earg_str);
        return (MA_DISPLAY_MENU);
    }
    return 0;
}
