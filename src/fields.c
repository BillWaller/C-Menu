/* fields.c
   Field support for MENU
   Bill Waller
   billxwaller@gmail.com

   This program was once unwieldy. It has been tamed.

   The original author was Bill Waller. He wrote the original code sometime
   in the late 1980s. That was the bad Bill. As witnesses will testify, he has
   been known to have written code in COBOL, Assembler, and once he even dabbled
   with BASIC. What shame he must now live with?

   The current author is also Bill Waller, the good Bill. He has rewritten
   and modified the original code and attempted to cover-up the sins of his
   predecessor. This is the good Bill. He writes code in C, Rust, QML, and other
   modern languages.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the MIT License. See the LICENSE file for details.

 */

#include "menu.h"
#include <monetary.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

double field_fmt_currency(Form *form, char *currency_s);
int field_fmt_decimal_int(Form *form, char *decimal_int_s);
int field_fmt_hex_int(Form *form, char *hex_int_s);
float field_fmt_float(Form *form, char *float_s);
double field_fmt_double(Form *form, char *double_s);
void scrub(char *, char *);
void right_justify(char *s, int fl);
void left_justify(char *s, int fl);
void trim(char *);

enum FieldFormat {
    FF_STRING,
    FF_DECIMAL_INT,
    FF_HEX_INT,
    FF_FLOAT,
    FF_DOUBLE,
    FF_CURRENCY,
    FF_INVALID
};

int accept_field(Form *);
int display_field(Form *);
int display_field_n(Form *, int);
int display_field_brackets(Form *);
int field_fmt(Form *, char *s);
int validate_field(Form *);
void mk_filler_s(char *, int);
void mk_blank_s(char *, int);

int accept_field(Form *form) {
    bool f_insert = FALSE;
    int in_key;
    static char buf[MAXLEN + 1];
    char *s, *d;
    int rc = 0;
    WINDOW *win = form->win;
    int flin = form->field[form->fidx]->line;
    int fcol = form->field[form->fidx]->col;
    int flen = form->field[form->fidx]->len;
    int fval = form->field[form->fidx]->val;
    char *accept_s = form->field[form->fidx]->accept_s;
    char *blank_s = form->field[form->fidx]->blank_s;
    char *fstart = accept_s;
    char *fend = fstart + flen;
    int x = fcol;
    char *p = fstart = accept_s;
    while (*p != '\0') {
        p++;
        x++;
    }
    char *str_end = p;

    idlok(win, TRUE);
    idcok(win, TRUE);
    immedok(win, TRUE);

    while (1) {
        mvwaddstr(win, flin, fcol, blank_s);
        mvwaddstr(win, flin, fcol, accept_s);
        tcflush(0, TCIFLUSH);
        in_key = mvwgetch(win, flin, x);
        switch (in_key) {

        // Accept Field
        case KEY_F(10):
            display_field(form);
            return (in_key);

        // Abort Operation
        case KEY_CTLC:
        case KEY_CTLD:
        case KEY_ESC:
        case KEY_BREAK:
        case KEY_F(9):
            display_field(form);
            in_key = KEY_F(9);
            return (in_key);

        case KEY_BTAB:
        case key_up:
        case KEY_UP:
        case KEY_F(7):
            display_field(form);
            in_key = KEY_UP;
            return (in_key);

        case KEY_TAB:
        case key_down:
        case KEY_DOWN:
            display_field(form);
            in_key = KEY_DOWN;
            return (in_key);

        case KEY_END:
            while (*p != '\0')
                p++;
            x = fcol + (p - fstart);
            continue;

        case KEY_CTLM:
        case KEY_ENTER:
            if (form->f_erase_remainder)
                *p = '\0';
            field_fmt(form, accept_s);
            display_field(form);
            in_key = KEY_ENTER;
            return (in_key);

        case KEY_IC:
            if (f_insert) {
                f_insert = FALSE;
            } else {
                f_insert = TRUE;
            }
            continue;

        case KEY_DC:
            s = p + 1;
            d = p;
            while (*s != '\0')
                *d++ = *s++; // Shift remainder of line left one character
            *d = '\0';
            str_end = d;
            f_insert = FALSE;
            continue;

        case KEY_HOME:
            p = fstart;
            break;

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
            str_end = d; /* new end of string    */
            if (p == fstart) {
                mvwaddstr(win, flin, fcol, blank_s);
                mvwaddstr(win, flin, fcol, accept_s);
                return (KEY_STAB);
            }
            continue;

        case KEY_CTLH:
        case KEY_LEFT:
            if (p > fstart) {
                p--;
                x--;
            }
            continue;

        case KEY_CTLL:
        case KEY_RIGHT:
            if (p < fend)
                if (p <= str_end) {
                    p++; /* move one to the right */
                    x++;
                }
            if (p > str_end) {
                mvwaddstr(win, flin, fcol, accept_s);
                return (KEY_ENTER);
            }
            continue;

        default:
            switch (fval) {
            case FF_STRING:
                continue;
            case FF_DECIMAL_INT:
                if ((in_key < '0' || in_key > '9') && in_key != '.') {
                    beep();
                    continue;
                }
                break;
            case FF_HEX_INT:
                if ((in_key < '0' || in_key > '9') &&
                    (in_key < 'A' || in_key > 'F') &&
                    (in_key < 'a' || in_key > 'f')) {
                    beep();
                    continue;
                }
                break;
            case FF_FLOAT:
                if ((in_key < '0' || in_key > '9') && in_key != '.' &&
                    in_key != '-') {
                    beep();
                    continue;
                }
                break;
            case FF_DOUBLE:
                if ((in_key < '0' || in_key > '9') && in_key != '.' &&
                    in_key != '-') {
                    beep();
                    continue;
                }
                break;
            case FF_CURRENCY:
                if ((in_key < '0' || in_key > '9') && in_key != '.' &&
                    in_key != '-') {
                    beep();
                    continue;
                }
                break;
            default:
                display_error_message("accept_field() invalid format");
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
                continue;
            }
        }
        strcpy(buf, fstart);
        if ((rc = field_fmt(form, buf)) == -1)
            mvwaddstr(win, flin, fcol, form->field[form->fidx]->display_s);
        else if (rc == 0) {
            mvwaddstr(win, flin, fcol, form->field[form->fidx]->display_s);
            if (in_key != KEY_LL)
                if (p == str_end)
                    return (KEY_DOWN);
        }
    }
}

int display_field_n(Form *form, int n) {
    int fidx = form->fidx;
    form->fidx = n;
    display_field(form);
    form->fidx = fidx;
    return 0;
}

int display_field(Form *form) {
    WINDOW *win = form->win;
    int flin = form->field[form->fidx]->line;
    int fcol = form->field[form->fidx]->col;
    display_field_brackets(form);
    wmove(win, flin, fcol);
    mvwaddstr(win, flin, fcol, form->field[form->fidx]->display_s);
    wrefresh(win);
    return 0;
}

int display_field_brackets(Form *form) {
    WINDOW *box = form->box;
    int flin = form->field[form->fidx]->line + 1;
    int fcol = form->field[form->fidx]->col;
    wmove(box, flin, fcol);
    waddch(box, '[');
    wmove(box, flin, fcol + form->field[form->fidx]->len + 1);
    waddch(box, ']');
    wrefresh(box);
    return 0;
}

int field_fmt(Form *form, char *s) {
    strncpy(form->field[form->fidx]->input_s, s, MAXLEN - 1);
    char *input_s = form->field[form->fidx]->input_s;
    char *accept_s = form->field[form->fidx]->accept_s;
    char *display_s = form->field[form->fidx]->display_s;
    char *blank_s = form->field[form->fidx]->blank_s;
    int fval = form->field[form->fidx]->val;
    int fl = form->field[form->fidx]->len;
    char field_s[MAXLEN];

    switch (fval) {
    case FF_STRING:
        left_justify(s, fl);
        trim(s);
        strncpy(input_s, s, MAXLEN - 1);
        strncpy(accept_s, s, MAXLEN - 1);
        strncpy(display_s, s, MAXLEN - 1);
        break;
    case FF_DECIMAL_INT:
        sscanf(input_s, "%d", &form->decimal_int_n);
        sprintf(accept_s, "%d", form->decimal_int_n);
        sprintf(display_s, "%d", form->decimal_int_n);
        break;
    case FF_HEX_INT:
        sscanf(input_s, "%x", &form->hex_int_n);
        sprintf(accept_s, "%x", form->hex_int_n);
        sprintf(display_s, "%x", form->hex_int_n);
        break;
    case FF_FLOAT:
        sscanf(input_s, "%f", &form->float_n);
        sprintf(accept_s, "%f", form->float_n);
        sprintf(display_s, "%f", form->float_n);
        break;
    case FF_DOUBLE:
        sscanf(input_s, "%lf", &form->double_n);
        sprintf(accept_s, "%lf", form->double_n);
        sprintf(display_s, "%lf", form->double_n);
        break;
    case FF_CURRENCY:
        scrub(field_s, input_s);
        sscanf(field_s, "%lf", &form->currency_n);
        sprintf(accept_s, "%'.2lf", form->currency_n);
        sprintf(display_s, "%'.2lf", form->currency_n);
        break;
    default:
        display_error_message("field_fmt() invalid format");
        break;
    }
    left_justify(accept_s, fl);
    mk_blank_s(blank_s, fl);
    right_justify(display_s, fl);
    return 0;
}

int validate_field(Form *form) {
    int n = form->fidx;
    char *p = form->field[n]->accept_s;
    if (form->field[n]->val & F_NOTBLANK) {
        char *s = form->field[n]->accept_s;
        while (*s++ == ' ')
            ;
        if (*s == '\0') {
            display_error_message("blank field not allowed");
            return (1);
        }
    }
    if (form->field[n]->val & F_NOMETAS) {
        if (strpbrk(p, "*?[]") != 0) {
            display_error_message("metacharacters not allowed");
            return (1);
        }
    }
    return (0);
}

void mk_filler_s(char *s, int fl) {
    char *e = s + fl;
    while (s != e)
        *s++ = '_';
    *s = '\0';
}

void mk_blank_s(char *s, int fl) {
    char *e = s + fl;
    while (s != e)
        *s++ = ' ';
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

void scrub(char *d, char *s) {
    while (*s != '\0') {
        if (*s == '-' || *s == '.') {
            *d++ = *s++;
        } else {
            if (*s < '0' || *s > '9') {
                s++;
                continue;
            }
        }
        *d++ = *s++;
    }
    *d = '\0';
}

void trim(char *s) {
    char *p = s;
    char *d = s;
    while (*p == ' ')
        p++;
    while (*p != '\0')
        *d++ = *p++;
    while (*(d - 1) == ' ' && d > s)
        d--;
    *d = '\0';
}
