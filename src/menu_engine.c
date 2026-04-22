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
    menu->lines = 0;
    menu->cols = 0;
    menu->line_idx = 0;
    menu->item_count = 0;
    menu->title[0] = '\0';
    menu->choice_max_len = 0;
    menu->text_max_len = 0;
    parse_menu_description(init);
    if (box_new(menu->lines, menu->cols, menu->begy, menu->begx, menu->title,
                true)) {
        ssnprintf(tmp_str, MAXLEN - 1, "box_new(%d, %d, %d, %d, %s) failed",
                  menu->lines, menu->cols, menu->begy, menu->begx, menu->title);
        Perror(tmp_str);
        return 1;
    }
    menu->win = win_win[win_ptr];
    menu->box = win_box[win_ptr];

    action = MA_DISPLAY_MENU;
    while (action) {
        switch (action) {
        case MA_RETURN:
            win_del();
            if (win_ptr < 0) {
                destroy_menu(init);
                return 0;
            }
            menu->win = win_win[win_ptr];
            menu->box = win_box[win_ptr];
            restore_wins();
            return 0;
        case MA_DISPLAY_MENU:
            for (menu->line_idx = 0; menu->line_idx < menu->item_count;
                 menu->line_idx++) {
                mvwaddstr(menu->win, menu->line_idx, 0,
                          menu->line[menu->line_idx]->choice_text);
            }
            action = MA_RESET_MENU;
            break;
        case MA_RESET_MENU:
            menu->line_idx = 0;
            for (i = 0; i < menu->item_count; i++) {
                if (menu->line[i]->type == MT_CHOICE) {
                    menu->line_idx = i;
                    break;
                }
            }
            action = MA_CONTINUE;
            break;
        case MA_CONTINUE:
            while ((action = menu_cmd_processor(init)) == MA_CONTINUE)
                ;
            break;
        default:
            break;
        }
    }
    destroy_menu(init);
    return 0;
}
/** @brief Processes user input for the menu system.
   @ingroup menu_engine
   @param init A pointer to an Init structure containing initialization data
   for the menu system.
   @returns an integer indicating the action taken by the user, such as
   returning to the main menu, displaying a submenu, or executing a command
   associated with a menu choice.
   @note handles navigation through the menu options, executing commands
   associated with menu choices, and responding to special keys such as
   function keys and mouse clicks.
 */
unsigned int menu_cmd_processor(Init *init) {
    int i, c;
    char *d;
    int in_key;
    char tmp_str[MAXLEN];
    char earg_str[MAXLEN];
    char *eargv[MAXARGS];
    int eargc;

    Menu *menu = init->menu;
    keypad(menu->win, TRUE);
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
    case KEY_UP:
        i = menu->line_idx;
        while (i > 0) {
            i--;
            if (menu->line[i]->type == MT_CHOICE) {
                menu->line_idx = i;
                break;
            }
        }
        return (MA_CONTINUE);
        /** Move down to the next menu choice */
    case KEY_DOWN:
        i = menu->line_idx;
        while (i < menu->item_count - 1) {
            i++;
            if (menu->line[i]->type == MT_CHOICE) {
                menu->line_idx = i;
                break;
            }
        }
        return (MA_CONTINUE);
        /** Select the current menu choice and execute its associated
         * command */
    case '\n':
    case KEY_ENTER:
        break;
        /** @brief Display help information for the menu system */
    case '?':
    case KEY_F(1):
        if (menu->f_help_spec && menu->help_spec[0] != '\0')
            strnz__cpy(tmp_str, menu->help_spec, MAXLEN - 1);
        else {
            strnz__cpy(tmp_str, init->mapp_help, MAXLEN - 1);
            strnz__cat(tmp_str, "/", MAXLEN - 1);
            strnz__cat(tmp_str, MENU_HELP_FILE, MAXLEN - 1);
        }
        eargc = 0;
        eargv[eargc++] = strdup("view");
        eargv[eargc++] = strdup("-N");
        eargv[eargc++] = strdup("f");
        eargv[eargc++] = strdup(tmp_str);
        eargv[eargc] = nullptr;
        init->lines = 30;
        init->cols = 60;
        init->begy = menu->begy + 1;
        init->begx = menu->begx + 1;
        strnz__cpy(init->title, "Menu Help", MAXLEN - 1);
        popup_view(init, eargc, eargv, init->lines, init->cols, init->begy,
                   init->begx);
        eargc = destroy_argv(eargc, eargv);
        return (MA_DISPLAY_MENU);
        /** Exit the menu and return to the previous menu or exit if at top
         */
    case KEY_F(9):
    case KEY_BREAK:
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
        return (MA_RESET_MENU);
        /** @brief open the default editor */
    case KEY_ALTF(10):
        restore_wins();
        return (MA_DISPLAY_MENU);
        /** @brief process mouse event */
    case KEY_MOUSE:
        if (click_y == -1 || click_x == -1)
            return (MA_CONTINUE);
        if (click_y < 0 || click_y >= menu->item_count)
            return (MA_CONTINUE);
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
            return (MA_CONTINUE);
        break;
    }
    c = (int)menu->line[menu->line_idx]->command_type;
    switch (c) {
        /** @brief Execute the command associated with the selected menu
         * choice
         */
    case CT_EXEC:
        char *s;
        s = strpbrk(menu->line[menu->line_idx]->command_str, " \t\f\v");
        strnz__cpy(earg_str, s, MAXLEN - 1);
        trim(earg_str);
        eargc = str_to_args(eargv, s, MAX_ARGS);
        full_screen_fork_exec(eargv);
        eargc = destroy_argv(eargc, eargv);
        return (MA_RESET_MENU);
        /** @brief Display help information for the menu system */
    case CT_HELP:
        if (menu->f_help_spec && menu->help_spec[0] != '\0')
            strnz__cpy(tmp_str, menu->help_spec, MAXLEN - 1);
        else {
            strnz__cpy(tmp_str, init->mapp_help, MAXLEN - 1);
            strnz__cat(tmp_str, "/", MAXLEN - 1);
            strnz__cat(tmp_str, MENU_HELP_FILE, MAXLEN - 1);
        }
        eargc = 0;
        eargv[eargc++] = strdup("view");
        eargv[eargc++] = strdup("-N");
        eargv[eargc++] = strdup("f");
        eargv[eargc++] = strdup(tmp_str);
        eargv[eargc] = nullptr;
        init->lines = 40;
        init->cols = 60;
        init->begy = menu->begy + 1;
        init->begx = menu->begx + 1;
        strnz__cpy(init->title, "Menu Help", MAXLEN - 1);
        popup_view(init, eargc, eargv, init->lines, init->cols, init->begy,
                   init->begx);
        eargc = destroy_argv(eargc, eargv);
        return (MA_DISPLAY_MENU);
        /** @brief Display a submenu or perform an action associated with
         * the selected menu choice */
    case CT_MENU:
        strnz__cpy(earg_str, menu->line[menu->line_idx]->command_str,
                   MAXLEN - 1);
        eargc = str_to_args(eargv, earg_str, MAX_ARGS);
        if (eargc == 0)
            return (MA_DISPLAY_MENU);
        popup_menu(init, eargc, eargv, menu->begy + 1, menu->begx + 1);
        eargc = destroy_argv(eargc, eargv);
        return (MA_RESET_MENU);
        /** @brief Display a pick list or form associated with the selected
         * menu choice */
    case CT_PICK:
        strnz__cpy(earg_str, menu->line[menu->line_idx]->command_str,
                   MAXLEN - 1);
        eargc = str_to_args(eargv, earg_str, MAX_ARGS);
        popup_pick(init, eargc, eargv, menu->begy + 1, menu->begx + 1);
        eargc = destroy_argv(eargc, eargv);
        return (MA_DISPLAY_MENU);
        /** @brief Display a form associated with the selected menu choice
         */
    case CT_FORM:
        strnz__cpy(earg_str, menu->line[menu->line_idx]->command_str,
                   MAXLEN - 1);
        eargc = str_to_args(eargv, earg_str, MAX_ARGS);
        popup_form(init, eargc, eargv, menu->begy + 1, menu->begx + 1);
        eargc = destroy_argv(eargc, eargv);
        return (MA_RESET_MENU);
        /** @brief Display a view associated with the selected menu choice
         */
    case CT_VIEW:
        strnz__cpy(earg_str, menu->line[menu->line_idx]->command_str,
                   MAXLEN - 1);
        eargc = str_to_args(eargv, earg_str, MAX_ARGS);
        init->lines = 66;
        init->cols = 80;
        init->begy = menu->begy + 1;
        init->begx = menu->begx + 1;
        strnz__cpy(init->title, menu->line[menu->line_idx]->raw_text,
                   MAXLEN - 1);
        popup_view(init, eargc, eargv, init->lines, init->cols, init->begy,
                   init->begx);
        eargc = destroy_argv(eargc, eargv);
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
        return (MA_CONTINUE);
    default:
        return (MA_CONTINUE);
    }
    return 0;
}
