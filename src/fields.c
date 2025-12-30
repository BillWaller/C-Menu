// fields.c
// Bill Waller Copyright (c) 2025
// Field support for MENU
// billxwaller@gmail.com
//
// This program was once unwieldy. It has been tamed.
//
// The original author was Bill Waller. He wrote the original code sometime
// in the late 1980s. That was the bad Bill. As witnesses will testify, he has
// been known to have written code in COBOL, Assembler, and once he even dabbled
// with BASIC. What shame he must now live with?
//
// The current author is also Bill Waller, the good Bill. He has rewritten
// and modified the original code and attempted to cover-up the sins of his
// predecessor. This is the good Bill. He writes code in C, Rust, QML, and other
// modern languages.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the MIT License. See the LICENSE file for details.
//
//

#include "menu.h"
#include <monetary.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

bool is_valid_date(int, int, int);
bool is_valid_time(int, int, int);
double form_fmt_currency(Form *form, char *currency_s);
int form_fmt_decimal_int(Form *form, char *decimal_int_s);
int form_fmt_hex_int(Form *form, char *hex_int_s);
float form_fmt_float(Form *form, char *float_s);
double form_fmt_double(Form *form, char *double_s);
int form_fmt_field(Form *, char *s);
void numeric(char *, char *);
void right_justify(char *s, int fl);
void left_justify(char *s, int fl);

char ff_tbl[][26] = {"string",   "decimal_int", "hex_int", "float", "double",
                     "currency", "yyyymmdd",    "hhmmss",  "apr",   ""};

int form_accept_field(Form *);
int form_display_field(Form *);
int form_display_field_n(Form *, int);
int form_display_field_brackets(Form *);
int form_validate_field(Form *);
void mk_filler(char *, int);

//  ╭───────────────────────────────────────────────────────────────╮
//  │ ACCEPT_FIELD                                                  │
//  ╰───────────────────────────────────────────────────────────────╯
int form_accept_field(Form *form) {
    bool f_insert = FALSE;
    int in_key;
    char *s, *d;
    WINDOW *win = form->win;

    int flin = form->field[form->fidx]->line;
    int fcol = form->field[form->fidx]->col;
    int flen = form->field[form->fidx]->len;
    int ff = form->field[form->fidx]->ff;
    char *accept_s = form->field[form->fidx]->accept_s;
    char *filler_s = form->field[form->fidx]->filler_s;

    form_fmt_field(form, accept_s);
    // mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION |
    // NCURSES_BUTTON_CLICKED,
    //          NULL);
    mousemask(NCURSES_BUTTON_CLICKED, NULL);
    MEVENT event;
    event.y = event.x = -1;

    //  ╭───────────────────────────────────────────────────────────────╮
    //  │ CURSOR AND FIELD POSITIONING                                  │
    //  ╰───────────────────────────────────────────────────────────────╯

    char *fstart = accept_s;
    char *fend = fstart + flen;
    int x = fcol;
    char *p = fstart = accept_s;
    char *str_end = p;
    in_key = 0;

    while (1) {
        if (in_key == 0) {
            mvwaddstr(win, flin, fcol, filler_s);
            mvwaddstr(win, flin, fcol, accept_s);
            wmove(win, flin, x);
            tcflush(0, TCIFLUSH);
            in_key = mvwgetch(win, flin, x);
        }
        switch (in_key) {
        // Accept Field
        case KEY_F(10):
            if (form_validate_field(form) != 0)
                continue;
            form_display_field(form);
            return (in_key);

        // Abort Operation
        case KEY_BREAK:
        case KEY_F(9):
            form_display_field(form);
            in_key = KEY_F(9);
            return (in_key);

        case KEY_BTAB:
        case KEY_UP:
            form_fmt_field(form, accept_s);
            form_display_field(form);
            in_key = KEY_UP;
            return (in_key);

        case '\t':
        case KEY_DOWN:
            form_fmt_field(form, accept_s);
            form_display_field(form);
            in_key = KEY_DOWN;
            return (in_key);

        case KEY_END:
            while (*p != '\0')
                p++;
            x = fcol + (p - fstart);
            in_key = 0;
            continue;

        case KEY_ENTER:
            if (form->f_erase_remainder)
                *p = '\0';
            form_fmt_field(form, accept_s);
            form_display_field(form);
            in_key = KEY_ENTER;
            return (in_key);

        case KEY_IC:
            if (f_insert)
                f_insert = FALSE;
            else
                f_insert = TRUE;
            in_key = 0;
            continue;

        case KEY_DC:
            s = p + 1;
            d = p;
            while (*s != '\0')
                *d++ = *s++; // Shift remainder of line left one character
            *d = '\0';
            str_end = d;
            f_insert = FALSE;
            in_key = 0;
            continue;

        case KEY_HOME:
            p = fstart;
            x = fcol;
            in_key = 0;
            continue;

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
            str_end = d; // new end of string
            in_key = 0;
            continue;

        case KEY_LEFT:
            if (p > fstart) {
                p--;
                x--;
            }
            in_key = 0;
            continue;

        case KEY_RIGHT:
            if (p < fend)
                if (p <= str_end) {
                    p++; // move one to the right
                    x++;
                }
            if (p > str_end) {
                mvwaddstr(win, flin, fcol, accept_s);
                return (KEY_ENTER);
            }
            in_key = 0;
            continue;
            //  ╭───────────────────────────────────────────────────────╮
            //  │ FORM MOUSE EVENT                                      │
            //  ╰───────────────────────────────────────────────────────╯
        case KEY_MOUSE:
            in_key = 0;
            if (getmouse(&event) != OK)
                break;
            if (event.bstate == BUTTON1_PRESSED ||
                event.bstate == BUTTON1_CLICKED ||
                event.bstate == BUTTON1_DOUBLE_CLICKED) {
                if (!wenclose(form->win, event.y, event.x))
                    continue;
                wmouse_trafo(form->win, &event.y, &event.x, false);
                if (event.y == form->lines - 1)
                    in_key = get_chyron_key(key_cmd, event.x);
            }
            continue;

        default:
            //  ╭───────────────────────────────────────────────────────╮
            //  │ FIELD CHARACTER FILTER                                │
            //  ╰───────────────────────────────────────────────────────╯
            switch (ff) {
            case FF_STRING:
                break;
            case FF_DECIMAL_INT:
                if ((in_key >= '0' || in_key <= '9') || in_key == '.')
                    break;
                beep();
                in_key = 0;
                continue;
            case FF_HEX_INT:
                if ((in_key >= '0' && in_key <= '9') ||
                    (in_key >= 'A' && in_key <= 'F') ||
                    (in_key >= 'a' && in_key <= 'f'))
                    break;
                beep();
                in_key = 0;
                continue;
            case FF_FLOAT:
                if ((in_key >= '0' && in_key <= '9') || in_key == '.' ||
                    (in_key == '-' && p == fstart))
                    break;
                beep();
                in_key = 0;
                continue;
            case FF_DOUBLE:
                if ((in_key >= '0' && in_key <= '9') || in_key == '.' ||
                    (in_key == '-' && p == fstart))
                    break;
                beep();
                in_key = 0;
                continue;
            case FF_CURRENCY:
                if ((in_key >= '0' && in_key <= '9') || in_key == '.' ||
                    (in_key == '-' && p == fstart))
                    break;
                beep();
                in_key = 0;
                continue;
            case FF_YYYYMMDD:
                if (in_key >= '0' && in_key <= '9')
                    break;
                beep();
                in_key = 0;
                continue;
            case FF_HHMMSS:
                if (in_key >= '0' && in_key <= '9')
                    break;
                beep();
                in_key = 0;
                continue;
            case FF_APR:
                if ((in_key >= '0' && in_key <= '9') || in_key == '.')
                    break;
                beep();
                in_key = 0;
                continue;
            default:
                Perror("form_accept_field() invalid format");
                break;
            }
            if (in_key >= ' ') {
                if (f_insert) {
                    if (str_end < fend) {
                        s = str_end - 1;
                        d = str_end;
                        *++str_end = '\0';
                        while (s >= p)
                            *d-- = *s--;
                        *p++ = in_key;
                    }
                } else {
                    if (p == str_end) {
                        *p++ = in_key;
                        *p = '\0';
                        str_end = p;
                        x++;
                    } else {
                        *p++ = in_key;
                        x++;
                    }
                }
                in_key = 0;
                continue;
            }
        }
    }
}

//  ╭───────────────────────────────────────────────────────────────╮
//  │ DISPLAY_FIELD_N                                               │
//  ╰───────────────────────────────────────────────────────────────╯
int form_display_field_n(Form *form, int n) {
    int fidx = form->fidx;
    form->fidx = n;
    form_display_field(form);
    form->fidx = fidx;
    return 0;
}
//  ╭───────────────────────────────────────────────────────────────╮
//  │ DISPLAY_FIELD                                                 │
//  ╰───────────────────────────────────────────────────────────────╯
int form_display_field(Form *form) {
    WINDOW *win = form->win;
    int flin = form->field[form->fidx]->line;
    int fcol = form->field[form->fidx]->col;
    form_display_field_brackets(form);
    mvwaddstr(win, flin, fcol, form->field[form->fidx]->filler_s);
    mvwaddstr(win, flin, fcol, form->field[form->fidx]->display_s);
    wrefresh(win);
    return 0;
}
//  ╭───────────────────────────────────────────────────────────────╮
//  │ DISPLAY_FIELD_BRACKETS                                        │
//  ╰───────────────────────────────────────────────────────────────╯
int form_display_field_brackets(Form *form) {
    int flin, fcol;
    if (form->f_brackets) {
        WINDOW *box = form->box;
        flin = form->field[form->fidx]->line + 1;
        fcol = form->field[form->fidx]->col;
        wmove(box, flin, fcol);
        waddch(box, '[');
        wmove(box, flin, fcol + form->field[form->fidx]->len + 1);
        waddch(box, ']');
        wrefresh(box);
    }
    return 0;
}
//  ╭───────────────────────────────────────────────────────────────╮
//  │ FORM_FMT_FIELD                                                │
//  ╰───────────────────────────────────────────────────────────────╯
int form_fmt_field(Form *form, char *s) {
    strnz__cpy(form->field[form->fidx]->input_s, s, MAXLEN - 1);
    char *input_s = form->field[form->fidx]->input_s;
    char *accept_s = form->field[form->fidx]->accept_s;
    char *display_s = form->field[form->fidx]->display_s;
    char *filler_s = form->field[form->fidx]->filler_s;
    int ff = form->field[form->fidx]->ff;
    int fl = form->field[form->fidx]->len;

    char field_s[MAXLEN];
    int decimal_int_n = 0;
    int hex_int_n = 0;
    float float_n = 0.0;
    double double_n = 0.0;
    double currency_n = 0.0;

    struct {
        int yyyy;
        int mm;
        int dd;
    } Date;
    Date.yyyy = Date.mm = Date.dd = 0;
    struct {
        int hh;
        int mm;
        int ss;
    } Time;
    Time.hh = Time.mm = Time.ss = 0;
    strnz(accept_s, fl);
    switch (ff) {
    case FF_STRING:
        left_justify(s, fl);
        trim(s);
        strnz__cpy(input_s, s, MAXLEN - 1);
        strnz__cpy(accept_s, s, MAXLEN - 1);
        strnz__cpy(display_s, s, MAXLEN - 1);
        break;
    case FF_DECIMAL_INT:
        sscanf(input_s, "%d", &decimal_int_n);
        sprintf(accept_s, "%d", decimal_int_n);
        sprintf(display_s, "%d", decimal_int_n);
        right_justify(display_s, fl);
        break;
    case FF_HEX_INT:
        sscanf(input_s, "%x", &hex_int_n);
        sprintf(accept_s, "%d", hex_int_n);
        sprintf(display_s, "%x", hex_int_n);
        right_justify(display_s, fl);
        break;
    case FF_FLOAT:
        sscanf(input_s, "%f", &float_n);
        sprintf(accept_s, "%f", float_n);
        sprintf(display_s, "%f", float_n);
        right_justify(display_s, fl);
        break;
    case FF_DOUBLE:
        sscanf(input_s, "%lf", &double_n);
        sprintf(accept_s, "%lf", double_n);
        sprintf(display_s, "%lf", double_n);
        right_justify(display_s, fl);
        break;
    case FF_CURRENCY:
        numeric(field_s, input_s);
        sscanf(field_s, "%lf", &currency_n);
        sprintf(accept_s, "%.2lf", currency_n);
        sprintf(display_s, "%'.2lf", currency_n);
        right_justify(display_s, fl);
        break;
    case FF_YYYYMMDD:
        Date.yyyy = Date.mm = Date.dd = 0;
        strnz__cpy(field_s, input_s, MAXLEN - 1);
        sscanf(field_s, "%4d%2d%2d", &Date.yyyy, &Date.mm, &Date.dd);
        sprintf(accept_s, "%04d%02d%02d", Date.yyyy, Date.mm, Date.dd);
        if (is_valid_date(Date.yyyy, Date.mm, Date.dd))
            sprintf(display_s, "%04d-%02d-%02d", Date.yyyy, Date.mm, Date.dd);
        break;
    case FF_HHMMSS:
        Time.hh = Time.mm = Time.ss = 0;
        strnz__cpy(field_s, input_s, MAXLEN - 1);
        sscanf(field_s, "%2d%2d%2d", &Time.hh, &Time.mm, &Time.ss);
        sprintf(accept_s, "%02d%02d%02d", Time.hh, Time.mm, Time.ss);
        if (is_valid_time(Time.hh, Time.mm, Time.ss))
            sprintf(display_s, "%02d:%02d:%02d", Time.hh, Time.mm, Time.ss);
        break;
    case FF_APR:
        sscanf(input_s, "%lf", &double_n);
        sprintf(accept_s, "%lf", double_n);
        sprintf(display_s, "%0.3lf", double_n);
        right_justify(display_s, fl);
        break;
    default:
        Perror("form_fmt_field() invalid format");
        break;
    }
    strnz(accept_s, fl);
    left_justify(accept_s, fl);
    mk_filler(filler_s, fl);
    return 0;
}
//  ╭───────────────────────────────────────────────────────────────╮
//  │ VALIDATE_FIELD                                                │
//  ╰───────────────────────────────────────────────────────────────╯
int form_validate_field(Form *form) {
    int n = form->fidx;
    char *p = form->field[n]->accept_s;
    if (form->field[n]->ff & F_NOTBLANK) {
        char *s = form->field[n]->accept_s;
        while (*s++ == ' ')
            ;
        if (*s == '\0') {
            Perror("blank field not allowed");
            return (1);
        }
    }
    if (form->field[n]->ff & F_NOMETAS) {
        if (strpbrk(p, "*?[]") != 0) {
            Perror("metacharacters not allowed");
            return (1);
        }
    }
    return (0);
}

//  ╭───────────────────────────────────────────────────────────────╮
//  │ MK_FILLER_S                                                   │
//  ╰───────────────────────────────────────────────────────────────╯
void mk_filler(char *s, int fl) {
    char *e = s + fl;
    unsigned char c;

    c = form->fill_char[0];
    while (s != e)
        *s++ = c;
    *s = '\0';
}

void left_justify(char *s, int fl) { trim(s); }

void right_justify(char *s, int fl) {
    char *p = s;
    char *d = s + fl;
    trim(s);
    *d = '\0';
    while (*s != '\0') {
        s++;
    }
    while (s != p) {
        *(--d) = *(--s);
    }
    while (d != p) {
        *(--d) = ' ';
    }
}
//  ╭───────────────────────────────────────────────────────────────╮
//  │ IS_VALID_DATE                                                 │
//  ╰───────────────────────────────────────────────────────────────╯
bool is_valid_date(int yyyy, int mm, int dd) {
    if (yyyy < 1 || mm < 1 || mm > 12 || dd < 1)
        return false;
    int days_in_month[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if ((yyyy % 4 == 0 && yyyy % 100 != 0) || (yyyy % 400 == 0))
        days_in_month[2] = 29;
    if (dd > days_in_month[mm])
        return false;
    return true;
}
//  ╭───────────────────────────────────────────────────────────────╮
//  │ IS_VALID_TIME                                                 │
//  ╰───────────────────────────────────────────────────────────────╯
bool is_valid_time(int hh, int mm, int ss) {
    if (hh < 0 || hh > 23 || mm < 0 || mm > 59 || ss < 0 || ss > 59)
        return false;
    return true;
}
//  ╭───────────────────────────────────────────────────────────────╮
//  │ NUMERIC                                                       │
//  ╰───────────────────────────────────────────────────────────────╯
void numeric(char *d, char *s) {
    while (*s != '\0') {
        if (*s == '-' || *s == '.' || (*s >= '0' && *s <= '9'))
            *d++ = *s++;
        else
            s++;
    }
    *d = '\0';
}
