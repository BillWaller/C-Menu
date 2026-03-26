/** @file menu_engine.c
    @brief The working part of C-Menu Menu
    @author Bill Waller
    Copyright (c) 2025
    MIT License
    billxwaller@gmail.com
    @date 2026-02-09
 */

/** @defgroup menu_engine Menu Engine
    @brief Main loop and command processor for the C-Menu menu system
    @details This module contains the core functionality for displaying menus,
    processing user input, and executing commands associated with menu choices.
    It handles navigation through menu options, responding to special keys,
    and managing the display of submenus, pick lists, forms, and views.
 */

#include <common.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

unsigned int menu_engine(Init *);
unsigned int menu_cmd_processor(Init *);

/** @brief The main loop of the menu system.
   @ingroup menu_engine
   @param init A pointer to an Init structure containing initialization data for
   the menu system.
   @returns an integer indicating the action taken by the user, such as
   returning to the main menu or exiting the menu system.
   @note displays the menu and processes user input until the user exits the
   menu or returns to the main menu.
 */
unsigned int menu_engine(Init *init) {
    int action;
    int i;
    char tmp_str[MAXLEN];

    Menu *menu = init->menu;
    if (menu == nullptr) {
        Perror("menu_engine: menu is nullptr");
        return (1);
    }
    action = MA_INIT;
    while (action == MA_INIT) {
        action = MA_DISPLAY_MENU;
        if (win_new(menu->lines, menu->cols, menu->begy, menu->begx,
                    menu->title, 0)) {
            ssnprintf(tmp_str, MAXLEN - 1,
                      "win_new(%d, %d, %d, %d, %s, %b) failed", menu->lines,
                      menu->cols, menu->begy, menu->begx, menu->title, 0);
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
        if (action == MA_RETURN) {
            win_del();
            menu->win = win_win[win_ptr];
            menu->box = win_box[win_ptr];
            return (MA_RETURN);
        }
        if (action == MA_INIT) {
            menu->lines = 0;
            menu->cols = 0;
            menu->line_idx = 0;
            menu->item_count = 0;
            menu->title[0] = '\0';
            menu->choice_max_len = 0;
            menu->text_max_len = 0;
            if (parse_menu_description(init))
                return (MA_RETURN);
        }
    }
    return (action);
}
/** @brief Processes user input for the menu system.
   @ingroup menu_engine
   @param init A pointer to an Init structure containing initialization data for
   the menu system.
   @returns an integer indicating the action taken by the user, such as
   returning to the main menu, displaying a submenu, or executing a command
   associated with a menu choice.
   @note handles navigation through the menu options, executing commands
   associated with menu choices, and responding to special keys such as function
   keys and mouse clicks.
 */
unsigned int menu_cmd_processor(Init *init) {
    int i, c, j;
    char *d;
    int in_key;
    char tmp_str[MAXLEN];

    keypad(menu->win, TRUE);
    Menu *menu = init->menu;
    mousemask(BUTTON1_CLICKED | BUTTON1_DOUBLE_CLICKED, nullptr);
    MEVENT event;
    wattron(menu->win, A_REVERSE);
    mvwaddstr_fill(menu->win, menu->line_idx, 0,
                   menu->line[menu->line_idx]->choice_text, menu->cols);
    wattroff(menu->win, A_REVERSE);
    event.y = event.x = -1;
    // tcflush(2, TCIFLUSH);
    wmove(menu->win, menu->line_idx, 1);
    in_key = xwgetch(menu->win, nullptr, -1);
    mvwaddstr_fill(menu->win, menu->line_idx, 0,
                   menu->line[menu->line_idx]->choice_text, menu->cols);
    switch (in_key) {
    /** Move up to the previous menu choice */
    case 'k':
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
        /** Move down to the next menu choice */
    case 'j':
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
        /** Select the current menu choice and execute its associated command */
    case '\n':
    case KEY_ENTER:
        break;
        /** @brief Display help information for the menu system */
    case 'H':
    case KEY_F(1):
        eargv[0] = strdup("view");
        if (menu->f_help_spec && menu->help_spec[0] != '\0')
            strnz__cpy(tmp_str, menu->help_spec, MAXLEN - 1);
        else {
            strnz__cpy(tmp_str, init->mapp_help, MAXLEN - 1);
            strnz__cat(tmp_str, "/", MAXLEN - 1);
            strnz__cat(tmp_str, MENU_HELP_FILE, MAXLEN - 1);
        }
        eargv[1] = strdup(tmp_str);
        eargv[2] = nullptr;
        eargc = 2;
        init->lines = 30;
        init->cols = 60;
        init->begy = menu->begy + 1;
        init->begx = menu->begx + 1;
        strnz__cpy(init->title, "Menu Help", MAXLEN - 1);
        popup_view(init, eargc, eargv);
        return (MA_DISPLAY_MENU);
        /** Exit the menu and return to the previous menu or exit if at top */
    case 'q':
    case KEY_F(9):
    case KEY_BREAK:
    case KEY_DL:
        return (MA_RETURN);
        /** @brief send default printer output file to printer */
    case KEY_ALTF(9):
        d = getenv("PRTCMD");
        if (d == nullptr || *d == '\0')
            strnz__cpy(earg_str, PRINTCMD, MAXLEN - 1);
        else
            strnz__cpy(earg_str, d, MAXLEN - 1);
        strnz__cat(earg_str, " ", MAXLEN - 1);
        d = getenv("PRTFILE");
        if (d == nullptr || *d == '\0') {
            d = getenv("HOME");
            if (d == nullptr || *d == '\0')
                strnz__cat(earg_str, VIEW_PRT_FILE, MAXLEN - 1);
            else {
                strnz__cat(earg_str, d, MAXLEN - 1);
                strnz__cat(earg_str, "/", MAXLEN - 1);
                strnz__cat(earg_str, VIEW_PRT_FILE, MAXLEN - 1);
            }
        } else
            strnz__cat(earg_str, d, MAXLEN - 1);
        full_screen_shell(earg_str);
        return (MA_DISPLAY_MENU);
        /** @brief open the default editor */
    case KEY_ALTF(10):
        restore_wins();
        return (MA_DISPLAY_MENU);
        d = getenv("DEFAULTEDITOR");
        if (d == nullptr || *d == '\0')
            strnz__cpy(earg_str, DEFAULTEDITOR, MAXLEN - 1);
        else
            strnz__cpy(earg_str, d, MAXLEN - 1);
        str_to_args(eargv, earg_str, MAX_ARGS);
        full_screen_fork_exec(eargv);
        return (MA_INIT);
        /** @brief process mouse event */
    case KEY_MOUSE:
        if (click_y == -1 || click_x == -1)
            return (MA_ENTER_OPTION);
        if (click_y < 0 || click_y >= menu->item_count)
            return (MA_ENTER_OPTION);
        menu->line_idx = click_y;
        break;
    default:
        /** @brief If the user presses a key that corresponds to a menu
         * choice's letter, select that menu choice */
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
        /** @brief Return to the main menu */
    case CT_RETURNMAIN:
        return (MA_RETURN_MAIN);
        /** @brief Execute the command associated with the selected menu
         * choice
         */
    case CT_EXEC:
        strnz__cpy(earg_str, menu->line[menu->line_idx]->command_str,
                   MAXLEN - 1);
        eargc = str_to_args(eargv, earg_str, MAX_ARGS);
        j = 0;
        free(eargv[0]);
        for (i = 1; i < eargc && eargv[i] != nullptr; i++) {
            if (eargv[i][0] == '~') {
                strnz__cpy(tmp_str, eargv[i], MAXLEN - 1);
                expand_tilde(tmp_str, MAXLEN - 1);
                eargv[j++] = strdup(tmp_str);
            } else
                eargv[j++] = strdup(eargv[i]);
            free(eargv[i]);
        }
        eargv[j] = nullptr;
        full_screen_fork_exec(eargv);
        return (MA_DISPLAY_MENU);
        /** @brief Display help information for the menu system */
    case CT_HELP:
        if (menu->f_help_spec && menu->help_spec[0] != '\0')
            strnz__cpy(tmp_str, menu->help_spec, MAXLEN - 1);
        else {
            strnz__cpy(tmp_str, init->mapp_help, MAXLEN - 1);
            strnz__cat(tmp_str, "/", MAXLEN - 1);
            strnz__cat(tmp_str, MENU_HELP_FILE, MAXLEN - 1);
        }
        eargv[0] = strdup("view");
        eargv[1] = strdup(tmp_str);
        eargv[2] = nullptr;
        eargc = 2;
        init->lines = 30;
        init->cols = 60;
        init->begy = menu->begy + 1;
        init->begx = menu->begx + 1;
        strnz__cpy(init->title, "Menu Help", MAXLEN - 1);
        popup_view(init, eargc, eargv);
        return (MA_DISPLAY_MENU);
        /** @brief Display a submenu or perform an action associated with
         * the selected menu choice */
    case CT_MENU:
        strnz__cpy(earg_str, menu->line[menu->line_idx]->command_str,
                   MAXLEN - 1);
        eargc = str_to_args(eargv, earg_str, MAX_ARGS);
        if (eargc == 0)
            return (MA_DISPLAY_MENU);
        zero_opt_args(init);
        parse_opt_args(init, eargc, eargv);
        if (init->begy == 0)
            init->begy = menu->begy + 1;
        if (init->begx == 0)
            init->begx = menu->begx + 1;
        Menu *save_menu = init->menu;
        init->menu = nullptr;
        init->menu = new_menu(init, eargc, eargv, init->begy, init->begx);
        if (!init->menu)
            abend(-1, "menu_cmd_processor: new_menu() failed");
        menu = init->menu;
        parse_menu_description(init);
        menu_engine(init);
        init->menu = destroy_menu(init);
        init->menu = save_menu;
        menu = init->menu;
        return (MA_INIT);
        /** @brief Display a pick list or form associated with the selected
         * menu choice */
    case CT_PICK:
        strnz__cpy(earg_str, menu->line[menu->line_idx]->command_str,
                   MAXLEN - 1);
        eargc = str_to_args(eargv, earg_str, MAX_ARGS);
        popup_pick(init, eargc, eargv, menu->begy + 1, menu->begx + 1);
        return (MA_DISPLAY_MENU);
        /** @brief Display a form associated with the selected menu choice
         */
    case CT_FORM:
        strnz__cpy(earg_str, menu->line[menu->line_idx]->command_str,
                   MAXLEN - 1);
        eargc = str_to_args(eargv, earg_str, MAX_ARGS);
        popup_form(init, eargc, eargv, menu->begy + 1, menu->begx + 1);
        return (MA_DISPLAY_MENU);
        /** @brief Display a view associated with the selected menu choice
         */
    case CT_VIEW:
        strnz__cpy(earg_str, menu->line[menu->line_idx]->command_str,
                   MAXLEN - 1);
        eargc = str_to_args(eargv, earg_str, MAX_ARGS);

        init->begy = menu->begy + 1;
        init->begx = menu->begx + 1;
        strnz__cpy(init->title, menu->line[menu->line_idx]->raw_text,
                   MAXLEN - 1);
        popup_view(init, eargc, eargv);
        return (MA_DISPLAY_MENU);
        /** @brief open ckeys (test curses keys) */
    case CT_CKEYS:
        popup_ckeys();
        return (MA_DISPLAY_MENU);
        /** @brief return to calling program */
    case CT_RETURN:
        return (MA_RETURN);
        /** @brief write the current menu configuration to a file */
    case CT_WRITE_CONFIG:
        write_config(init);
        return (MA_ENTER_OPTION);
    default:
        return (MA_ENTER_OPTION);
    }
    return 0;
}
