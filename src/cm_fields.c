/** @file cm_fields.c
    @brief Stand-alone Field Edit and Entry for C-Menu
    @details Currently, this modules contains a single function, cf_accept(),
    a lightweight yet fully functional field editor. For text fields, it is
    equivalent to the field editor in fields.c. Instead of the built-in
    formatting and validation in fields.c, this module provides a clean slate
    for a modular plug-in architecture to accomodate formatting, validation, and
    high-precision numeric data. It will eventually replace the field editor in
    fields.c. It is a work in progress.
    @author Bill Waller
    Copyright (c) 2025, 2026
    MIT License
    billxwaller@gmail.com
    @date 2026-02-09
 */

#include <common.h>
#include <monetary.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

/** @brief Structure to hold field information for C-Menu
    @note Not currently used in the code, but may be used in
    future versions of C-Menu
 */

bool f_erase_remainder = true;

/** @brief Accepts input for a field in a C-Menu window
    @param UiSurface *sfc The surface to accept input from
    @param win The window to accept input from
    @param accept_s The string to accept input into
    @param flin The line number of the field
    @param fcol The column number of the field
    @param flen The length of the field
    @return The key pressed to exit the field (UIKEY_ENTER, UIKEY_F09, etc.)
 */
int cf_accept(UiSurface *sfc, uint w, char *accept_s, uint flin, uint fcol, uint flen) {
    bool f_insert = false;
    int in_key = 0;
    char *s, *d;
    char *fstart;
    UiEvent event;

    click_x = click_y = -1;
    char *p = fstart = accept_s;
    char *fend = fstart + flen;
    int x = fcol;
    char *str_end = p + strlen(p);
    while (1) {
        if (in_key == 0) {
            ui_mvwaddstr(sfc, w, flin, fcol, accept_s);
            ui_wclrtoeol(sfc, w);
            ui_cursor_move(sfc, w, flin, x);
            ui_render();
            in_key = ui_get_event(sfc, WIN, NULL, &event, -1);
        }
        ui_curs_set(0);
        switch (in_key) {
        case UIKEY_F09:
            in_key = UIKEY_F09;
            return (in_key);
        case '\t':
        case UIKEY_END:
        case Ctrl('e'):
            while (*p != '\0')
                p++;
            x = fcol + (p - fstart);
            in_key = 0;
            continue;
            /**< Enter accepts the field and moves to the next field if
             * f_erase_remainder is set, it will clear the remaining
             * characters above and after the current cursor location */
        case '\n':
        case UIKEY_ENTER:
            if (f_erase_remainder)
                *p = '\0';
            in_key = UIKEY_ENTER;
            return (in_key);
            /** UIKEY_IC toggles insert mode */
        case UIKEY_IC:
            f_insert = !f_insert;
            in_key = 0;
            continue;
            /** UIKEY_DC deletes character at cursor */
        case UIKEY_DC:
            s = p + 1;
            d = p;
            while (*s != '\0')
                *d++ = *s++;
            *d = '\0';
            str_end = d;
            f_insert = false;
            in_key = 0;
            continue;
            /** UIKEY_HOME moves cursor to start of field */
        case UIKEY_HOME:
        case Ctrl('a'):
            p = fstart;
            x = fcol;
            in_key = 0;
            continue;
            /** UIKEY_BACKSPACE deletes character before cursor */
        case UIKEY_BACKSPACE:
            if (p > fstart) {
                p--;
                x--;
            }
            s = p + 1;
            d = p;
            while (*s != '\0')
                *d++ = *s++;
            *d = '\0';
            str_end = d;
            in_key = 0;
            continue;
            /** UIKEY_LEFT moves cursor left one character */
        case UIKEY_LEFT:
            if (p > fstart) {
                p--;
                x--;
            }
            in_key = 0;
            continue;
            /** UIKEY_RIGHT moves cursor right one character */
        case UIKEY_RIGHT:
            if (p < fend && p < str_end) {
                p++;
                x++;
            }
            in_key = 0;
            continue;
            /** Handles mouse events for field editing */
        case UIKEY_BUTTON1:
            x = click_x;
            flin = click_y;
            fstart = accept_s;
            fend = fstart + flen;
            str_end = fstart + strlen(fstart);
            p = fstart + (x - fcol);
            if (p > str_end)
                x -= p - str_end;
            p = min(p, str_end);
            in_key = 0;
            continue;
        default:
            if (p >= fend) {
                in_key = 0;
                continue;
            }
            if (in_key < ' ' || in_key > '~') {
                in_key = 0;
                continue;
            }
            if (f_insert) {
                if (str_end < fend) {
                    s = str_end - 1;
                    d = str_end;
                    while (s >= p)
                        *d-- = *s--;
                    *p++ = in_key;
                    str_end++;
                    x++;
                }
            } else {
                if (p < fend) {
                    if (p < str_end) {
                        if (p == accept_s && in_key == ' ') {
                            d = accept_s;
                            s = accept_s + 1;
                            while (*s != '\0')
                                *d++ = *s++;
                            *d = '\0';
                            in_key = 0;
                            continue;
                        }
                        *p++ = in_key;
                        x++;
                    } else if (p == str_end) {
                        *p++ = in_key;
                        *p = '\0';
                        str_end = p;
                        x++;
                    }
                }
            }
            in_key = 0;
            continue;
        }
    }
}
