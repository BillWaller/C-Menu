/* menu_engine.c
 * Bill Waller
 * billxwaller@gmail.com
 * file system interface
 */

#include "menu.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

// menu_engine is reentrant through m_menu

unsigned int menu_engine(Init *);
unsigned int menu_cmd_processor(Init *);

unsigned int menu_engine(Init *init) {
    int action;
    int i;

    Menu *menu = init->menu;
    if (menu == NULL) {
        Perror("menu_engine: menu is NULL");
        return (1);
    }
    action = MA_INIT;

    /*  ╭───────────────────────────────────────────────────────────────╮
        │ DISPLAY_MENU                                                  │
        ╰───────────────────────────────────────────────────────────────╯ */
    while (action == MA_INIT) {
        action = MA_DISPLAY_MENU;
        if (win_new(menu->lines, menu->cols, menu->begy, menu->begx,
                    menu->title)) {
            snprintf(tmp_str, MAXLEN - 1, "win_new(%d, %d, %d, %d) failed",
                     menu->lines, menu->cols, menu->begy, menu->begx);
            Perror(tmp_str);
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
                    // y = menu->line_idx
                    // x = menu->option_offset
                    // menu->line[menu->line_idx]->option_ptr[menu->line[menu->line_idx]->option_idx]);
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
            while ((action = menu_cmd_processor(init)) == MA_ENTER_OPTION)
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
            menu->title[0] = '\0';
            menu->choice_max_len = 0;
            menu->text_max_len = 0;
            menu->option_offset = 0;
            menu->option_max_len = 0;
            if (parse_menu_description(init))
                return (MA_RETURN);
        }
    }
    return (action);
}
/*  ╭───────────────────────────────────────────────────────────────╮
    │ MENU_CMD_PROCESSOR                                            │
    ╰───────────────────────────────────────────────────────────────╯ */
unsigned int menu_cmd_processor(Init *init) {
    int eargc;
    char *eargv[MAXARGS];
    char earg_str[MAXLEN];
    int i, c, j, rc;
    char *d;
    int in_key;
    int lines, cols;

    keypad(menu->win, TRUE);
    Menu *menu = init->menu;
    mousemask(BUTTON1_CLICKED | BUTTON1_DOUBLE_CLICKED, NULL);
    MEVENT event;
    wattron(menu->win, A_REVERSE);
    mvwaddstr(menu->win, menu->line_idx, 0,
              menu->line[menu->line_idx]->choice_text);
    wattroff(menu->win, A_REVERSE);
    touchwin(win_box[win_ptr]);
    wnoutrefresh(win_box[win_ptr]);
    touchwin(win_win[win_ptr]);
    wnoutrefresh(win_win[win_ptr]);
    event.y = event.x = -1;
    tcflush(2, TCIFLUSH);
    in_key = mvwgetch(menu->win, menu->line_idx, 1);
    mvwaddstr(menu->win, menu->line_idx, 0,
              menu->line[menu->line_idx]->choice_text);

    switch (in_key) {
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
    case KEY_ENTER:
        break;
    case KEY_F(9):
        return (MA_RETURN_MAIN);
    case KEY_BREAK:
    case KEY_DL:
        return (MA_RETURN_MAIN);
        d = getenv("PRTCMD");
        if (d == NULL || *d == '\0')
            strncpy(earg_str, PRINTCMD, MAXLEN - 1);
        else
            strncpy(earg_str, d, MAXLEN - 1);
        strncat(earg_str, " ", MAXLEN - 1);
        d = getenv("PRTFILE");
        if (d == NULL || *d == '\0') {
            d = getenv("HOME");
            if (d == NULL || *d == '\0')
                strncat(earg_str, VIEW_PRT_FILE, MAXLEN - 1);
            else {
                strncat(earg_str, d, MAXLEN - 1);
                strncat(earg_str, "/", MAXLEN - 1);
                strncat(earg_str, VIEW_PRT_FILE, MAXLEN - 1);
            }
        } else
            strncat(earg_str, d, MAXLEN - 1);
        full_screen_shell(earg_str);
        return (MA_DISPLAY_MENU);
    case '\r':
        restore_wins();
        return (MA_DISPLAY_MENU);
        d = getenv("DEFAULTEDITOR");
        if (d == NULL || *d == '\0')
            strncpy(earg_str, DEFAULTEDITOR, MAXLEN - 1);
        else
            strncpy(earg_str, d, MAXLEN - 1);
        str_to_args(eargv, earg_str, MAX_ARGS);
        full_screen_fork_exec(eargv);
        return (MA_INIT);
        /*  ╭───────────────────────────────────────────────────────────────╮
            │ MENU MOUSE FUNCTIONS                                          │
            ╰───────────────────────────────────────────────────────────────╯ */
    case KEY_MOUSE:
        if (getmouse(&event) != OK)
            return (MA_ENTER_OPTION);
        switch (event.bstate) {
        case BUTTON1_PRESSED:
        case BUTTON1_CLICKED:
        case BUTTON1_DOUBLE_CLICKED:
            if (!wenclose(menu->win, event.y, event.x)) {
                return (MA_ENTER_OPTION);
            }
            wmouse_trafo(menu->win, &event.y, &event.x, false);
            if (event.y < 0 || event.y >= menu->item_count) {
                return (MA_ENTER_OPTION);
            }
            menu->line_idx = event.y;
            break;
            ;
        default:
            return (MA_ENTER_OPTION);
            break;
        }
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
        break;
    }
    c = (int)menu->line[menu->line_idx]->command_type;
    switch (c) {

    case CT_RETURNMAIN:
        return (MA_RETURN_MAIN);

    case CT_EXEC:
        strncpy(earg_str, menu->line[menu->line_idx]->command_str, MAXLEN - 1);
        eargc = str_to_args(eargv, earg_str, MAX_ARGS);
        j = 0;
        for (i = 1; i < eargc && eargv[i] != NULL; i++)
            eargv[j++] = eargv[i];
        eargv[j] = NULL;
        full_screen_fork_exec(eargv);
        return (MA_DISPLAY_MENU);

    case CT_HELP:
        strnz__cpy(earg_str, HELP_CMD, MAXLEN - 1);
        strnz__cat(earg_str, " ", MAXLEN - 1);
        strnz__cat(earg_str, menu->help_spec, MAXLEN - 1);
        eargc = str_to_args(eargv, earg_str, MAX_ARGS);
        zero_opt_args(init);
        parse_opt_args(init, eargc, eargv);
        mview(init, eargc, eargv, 0, 0, 0, 0, menu->title);
        return (MA_DISPLAY_MENU);

    case CT_MENU:
        strncpy(earg_str, menu->line[menu->line_idx]->command_str, MAXLEN - 1);
        eargc = str_to_args(eargv, earg_str, MAX_ARGS);
        if (eargc == 0)
            return (MA_DISPLAY_MENU);
        zero_opt_args(init);
        parse_opt_args(init, eargc, eargv);
        if (!init_menu_files(init, eargc, eargv)) {
            Perror("menu_cmd_processor: init_menu_files failed");
            return (MA_DISPLAY_MENU);
        }
        rc = menu_engine(init);
        if (rc == MA_RETURN_MAIN)
            //     if (menu->caller != C_MAIN)  // if not called by first
            //     menu
            //         return (MA_RETURN_MAIN); // go all the way back
            return (MA_DISPLAY_MENU);
        break;

    case CT_PICK:
        strncpy(earg_str, menu->line[menu->line_idx]->command_str, MAXLEN - 1);
        eargc = str_to_args(eargv, earg_str, MAX_ARGS);
        lines = 2 * LINES / 3;
        cols = 2 * COLS / 3;
        begy = (LINES - lines) / 2;
        begx = (COLS - cols) / 2;
        zero_opt_args(init);
        parse_opt_args(init, eargc, eargv);
        init_pick(init, eargc, eargv, menu->begy + 1, menu->begx + 4);
        return (MA_DISPLAY_MENU);

    case CT_FORM:
        strncpy(earg_str, menu->line[menu->line_idx]->command_str, MAXLEN - 1);
        eargc = str_to_args(eargv, earg_str, MAX_ARGS);
        zero_opt_args(init);
        parse_opt_args(init, eargc, eargv);
        init_form(init, eargc, eargv, menu->begy + 1, menu->begx + 4);
        return (MA_DISPLAY_MENU);

    case CT_VIEW:
        strncpy(earg_str, menu->line[menu->line_idx]->command_str, MAXLEN - 1);
        eargc = str_to_args(eargv, earg_str, MAX_ARGS);
        zero_opt_args(init);
        parse_opt_args(init, eargc, eargv);
        if (init->title[0] == '\0')
            strcpy(init->title, init->start_cmd);
        strip_quotes(init->title);
        mview(init, eargc, eargv, init->lines, init->cols, menu->begy + 1,
              menu->begx + 4, init->title);
        return (MA_DISPLAY_MENU);

    case CT_CKEYS:
        display_curses_keys();
        return (MA_DISPLAY_MENU);

    case CT_RETURN:
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
        write_config(init);
        return (MA_ENTER_OPTION);

    default:
        return (MA_ENTER_OPTION);
    }
    return 0;
}
