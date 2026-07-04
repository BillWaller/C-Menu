/** @file field.c
    @brief Stand-alone Field Edit and Entry for C-Menu
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
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

typedef struct {
    int flin;
    int fcol;
    int flen;
    int ff;
    char fill_char;
    char input_s[FIELD_MAXLEN];
    char accept_s[FIELD_MAXLEN];
    char display_s[FIELD_MAXLEN];
    Chyron *chyron;
    WINDOW *win;
    bool cf_erase_remainder;
} CmField;

bool f_erase_remainder = true;

int cf_accept(WINDOW *win, Chyron *chyron, char *input_s, char *accept_s, char *display_s, int flin, int fcol, int flen, int ff);
int cf_display(WINDOW *win, char *display_s, int flin, int fcol, int flen);
int cf_display_accept(WINDOW *win, char *accept_s, int flin, int fcol, int flen);
int cf_validate(char *accept_s, int ff);
int cf_fmt(char *input_s, char *accept_s, char *display_s, int ff, int flen);

int cf_accept(WINDOW *win, Chyron *chyron, char *input_s, char *accept_s, char *display_s, int flin, int fcol, int flen, int ff) {
    bool f_insert = FALSE;
    int in_key = 0;
    char *s, *d;
    char *fstart;
    int lines;
    cf_fmt(input_s, accept_s, display_s, ff, flen);

    click_x = click_y = -1;
    char *p = fstart = accept_s;
    char *fend = fstart + flen;
    int x = fcol;
    char *str_end = p + strlen(p);
    if (chyron != nullptr) {
        in_key = 0;
        if (f_insert)
            set_chyron_key_cp(chyron, 18, "INS", KEY_IC, cp_nt_hl_rev);
        else
            set_chyron_key_cp(chyron, 18, "INS", KEY_IC, cp_nt_rev);
        compile_chyron(chyron);
        lines = getmaxy(win);
        display_chyron(win, chyron, lines - 1, chyron->l);
    }
    while (1) {
        if (in_key == 0) {
            cf_fmt(input_s, accept_s, display_s, ff, flen);
            cf_display_accept(win, accept_s, flin, fcol, flen);
            tcflush(0, TCIFLUSH);
            wmove(win, flin, x);
            curs_set(1);
            update_panels();
            doupdate();
            in_key = xwgetch(win, chyron, -1);
        }
        curs_set(0);
        switch (in_key) {
        case KEY_F(10):
            cf_fmt(input_s, accept_s, display_s, ff, flen);
            cf_display(win, display_s, flin, fcol, flen);
            if (cf_validate(accept_s, ff) != 0)
                continue;
            return (in_key);
            /** KEY_F(9) Cancels the current operation */
        case KEY_BREAK:
        case KEY_F(9):
            cf_display(win, display_s, flin, fcol, flen);
            in_key = KEY_F(9);
            return (in_key);
            /** KEY_UP, KEY_BTAB moves to the previous field */
        case 'H':
        case KEY_F(1):
            return (in_key);
        case KEY_BTAB:
        case KEY_UP:
            cf_fmt(input_s, accept_s, display_s, ff, flen);
            cf_display(win, display_s, flin, fcol, flen);
            in_key = KEY_UP;
            return (in_key);
            /** KEY_DOWN, TAB moves to the next field */
        case '\t':
        case KEY_DOWN:
            cf_fmt(input_s, accept_s, display_s, ff, flen);
            cf_display(win, display_s, flin, fcol, flen);
            in_key = KEY_DOWN;
            return (in_key);
            /** KEY_END moves cursor to end of field */
        case KEY_END:
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
        case KEY_ENTER:
            if (f_erase_remainder)
                *p = '\0';
            cf_fmt(input_s, accept_s, display_s, ff, flen);
            cf_display(win, display_s, flin, fcol, flen);
            in_key = KEY_ENTER;
            return (in_key);
            /** KEY_IC toggles insert mode */
        case KEY_IC:
            if (chyron != nullptr) {
                if (f_insert) {
                    f_insert = FALSE;
                    set_chyron_key_cp(chyron, 18, "INS", KEY_IC, cp_nt_rev);
                } else {
                    f_insert = TRUE;
                    set_chyron_key_cp(chyron, 18, "INS", KEY_IC,
                                      cp_nt_hl_rev);
                }
                compile_chyron(chyron);
                display_chyron(win, chyron, lines - 1,
                               chyron->l);
            }
            in_key = 0;
            continue;
            /** KEY_DC deletes character at cursor */
        case KEY_DC:
            s = p + 1;
            d = p;
            while (*s != '\0')
                *d++ = *s++;
            *d = '\0';
            str_end = d;
            f_insert = FALSE;
            in_key = 0;
            continue;
            /** KEY_HOME moves cursor to start of field */
        case KEY_HOME:
        case Ctrl('a'):
            p = fstart;
            x = fcol;
            in_key = 0;
            continue;
            /** KEY_BACKSPACE deletes character before cursor */
        case KEY_BACKSPACE:
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
            /** KEY_LEFT moves cursor left one character */
        case KEY_LEFT:
            if (p > fstart) {
                p--;
                x--;
            }
            in_key = 0;
            continue;
            /** KEY_RIGHT moves cursor right one character */
        case KEY_RIGHT:
            if (p < fend && p < str_end) {
                p++;
                x++;
            }
            in_key = 0;
            continue;
            /** Handles mouse events for field editing */
        case KEY_MOUSE:
            x = click_x;
            flin = click_y;
            cf_fmt(input_s, accept_s, display_s, ff, flen);
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
            /** Validates fields based on format */
            switch (ff) {
                /** FF_STRING accepts all printable characters and spaces */
            case FF_STRING:
                break;
                /** FF_DECIMAL_INT accepts digits 0 through 9 and decimal point
                 * ('.') */
            case FF_DECIMAL_INT:
                if ((in_key >= '0' && in_key <= '9') || in_key == '.')
                    break;
                beep();
                in_key = 0;
                continue;
                /** FF_HEX_INT accepts digits 0 through 9 and letters A through
                 * F (case-insensitive) */
            case FF_HEX_INT:
                if ((in_key >= '0' && in_key <= '9') ||
                    (in_key >= 'A' && in_key <= 'F') ||
                    (in_key >= 'a' && in_key <= 'f'))
                    break;
                beep();
                in_key = 0;
                continue;
                /** FF_FLOAT accepts digits 0 through 9 and decimal point ('.')
                 * and negative operator ('-') at the start of the field */
            case FF_FLOAT:
                if ((in_key >= '0' && in_key <= '9') || in_key == '.' ||
                    (in_key == '-' && p == fstart))
                    break;
                beep();
                in_key = 0;
                continue;
                /** FF_DOUBLE accepts digits 0 through 9 and decimal point ('.')
                 * and negative operator ('-') at the start of the field */
            case FF_DOUBLE:
                if ((in_key >= '0' && in_key <= '9') || in_key == '.' ||
                    (in_key == '-' && p == fstart))
                    break;
                beep();
                in_key = 0;
                continue;
                /** FF_CURRENCY accepts digits 0 through 9 and decimal point
                 * ('.') and negative operator ('-') at the start of the field
                 */
            case FF_CURRENCY:
                if ((in_key >= '0' && in_key <= '9') || in_key == '.' ||
                    (in_key == '-' && p == fstart))
                    break;
                beep();
                in_key = 0;
                continue;
                /** FF_YYYYMMDD accepts digits 0 through 9 */
            case FF_YYYYMMDD:
                if (in_key >= '0' && in_key <= '9')
                    break;
                beep();
                in_key = 0;
                continue;
                /** FF_HHMMSS accepts digits 0 through 9 */
            case FF_HHMMSS:
                if (in_key >= '0' && in_key <= '9')
                    break;
                beep();
                in_key = 0;
                continue;
                /** FF_APR accepts digits 0 through 9 and decimal point ('.') */
            case FF_APR:
                if ((in_key >= '0' && in_key <= '9') || in_key == '.')
                    break;
                beep();
                in_key = 0;
                continue;
            default:
                Perror("cf_field_editor() invalid format");
                break;
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

int cf_display(WINDOW *win, char *display_s, int y, int x, int flen) {
    cchar_t display_cc[flen + 1];
    mvwadd_wchnstr(win, y, x, &sp, flen);
    str_to_cc(display_cc, display_s, A_NORMAL, cp_nt, flen);
    mvwadd_wchnstr(win, y, x, display_cc, flen);
    return 0;
}

int cf_display_accept(WINDOW *win, char *accept_s, int y, int x, int flen) {
    cchar_t accept_cc[flen + 1];
    mvwadd_wchnstr(win, y, x, &sp, flen);
    str_to_cc(accept_cc, accept_s, A_NORMAL, cp_nt, flen);
    mvwadd_wchnstr(win, y, x, accept_cc, flen);
    return 0;
}

int cf_fmt(char *input_s, char *accept_s, char *display_s, int ff, int flen) {
    char field_s[FIELD_MAXLEN];
    int decimal_int_n = 0;
    int hex_int_n = 0;
    float float_n = 0.0;
    double double_n = 0.0;
    double currency_n = 0.0;
    Date date;
    date.yyyy = date.mm = date.dd = 0;
    Time time;
    time.hh = time.mm = time.ss = 0;

    strnz(accept_s, flen);
    switch (ff) {
    case FF_STRING:
        left_justify(input_s);
        trim(input_s);
        strnz__cpy(accept_s, input_s, FIELD_MAXLEN - 1);
        strnz__cpy(display_s, input_s, FIELD_MAXLEN - 1);
        break;
    case FF_DECIMAL_INT:
        sscanf(input_s, "%d", &decimal_int_n);
        sprintf(accept_s, "%d", decimal_int_n);
        sprintf(display_s, "%d", decimal_int_n);
        right_justify(display_s, flen);
        break;
    case FF_HEX_INT:
        sscanf(input_s, "%x", &hex_int_n);
        sprintf(accept_s, "%d", hex_int_n);
        sprintf(display_s, "%x", hex_int_n);
        right_justify(display_s, flen);
        break;
    case FF_FLOAT:
        sscanf(input_s, "%f", &float_n);
        sprintf(accept_s, "%f", float_n);
        sprintf(display_s, "%f", float_n);
        right_justify(display_s, flen);
        break;
    case FF_DOUBLE:
        sscanf(input_s, "%lf", &double_n);
        sprintf(accept_s, "%lf", double_n);
        sprintf(display_s, "%lf", double_n);
        right_justify(display_s, flen);
        break;
    case FF_CURRENCY:
        numeric(field_s, input_s);
        sscanf(field_s, "%lf", &currency_n);
        sprintf(accept_s, "%.2lf", currency_n);
        sprintf(display_s, "%'.2lf", currency_n);
        right_justify(display_s, flen);
        break;
    case FF_YYYYMMDD:
        date.yyyy = date.mm = date.dd = 0;
        strnz__cpy(field_s, input_s, FIELD_MAXLEN - 1);
        sscanf(field_s, "%4d%2d%2d", &date.yyyy, &date.mm, &date.dd);
        sprintf(accept_s, "%04d%02d%02d", date.yyyy, date.mm, date.dd);
        if (is_valid_date(date.yyyy, date.mm, date.dd))
            sprintf(display_s, "%04d-%02d-%02d", date.yyyy, date.mm, date.dd);
        break;
    case FF_HHMMSS:
        time.hh = time.mm = time.ss = 0;
        strnz__cpy(field_s, input_s, FIELD_MAXLEN - 1);
        sscanf(field_s, "%2d%2d%2d", &time.hh, &time.mm, &time.ss);
        sprintf(accept_s, "%02d%02d%02d", time.hh, time.mm, time.ss);
        if (is_valid_time(time.hh, time.mm, time.ss))
            sprintf(display_s, "%02d:%02d:%02d", time.hh, time.mm, time.ss);
        break;
    case FF_APR:
        sscanf(input_s, "%lf", &double_n);
        sprintf(accept_s, "%lf", double_n);
        sprintf(display_s, "%0.3lf", double_n);
        right_justify(display_s, flen);
        break;
    default:
        break;
    }
    strnz(accept_s, flen);
    left_justify(accept_s);
    return 0;
}

int cf_validate(char *accept_s, int ff) {
    char *p = accept_s;
    if (ff & F_NOTBLANK) {
        char *s = accept_s;
        while (*s++ == ' ')
            ;
        if (*s == '\0') {
            Perror("blank field not allowed");
            return (1);
        }
    }
    if (ff & F_NOMETAS) {
        if (strpbrk(p, "*?[]") != 0) {
            Perror("metacharacters not allowed");
            return (1);
        }
    }
    return (0);
}
