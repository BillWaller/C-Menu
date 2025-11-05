/* fields.c
 * Field support for MENU
 * Bill Waller
 * billxwaller@gmail.com
 */

#include "menu.h"
#include <string.h>

int accept_field(WINDOW *, int, int, int, char *, int, char, int, bool);
void display_field(WINDOW *, int, int, char *, int, char, int);
int format_field(char *, int, char, int);
int validate_field(char *, int);

int accept_field(WINDOW *win, int flin, int fcol, int fpos, char *fptr,
                 int flen, char fprompt, int fval, bool f_erase_remainder) {
    bool f_insert = FALSE;
    int in_key;
    static char buf[MAXLEN + 1];
    char *str_end; /*  End of string */
    char *fstart;
    char *fend; /*  End of field  */
    char *s, *d, *e;
    int rc = 0;

    display_field(win, flin, fcol, fptr, flen, fprompt, fval);
    fstart = fptr;
    s = fptr;
    while (*s++ != '\0')
        ;
    str_end = s;
    fend = fstart + flen;
    if (fpos < str_end - fstart)
        fptr = fstart + fpos; /* place cursor in text */
    else
        fptr = str_end; /* or at end of field   */
    f_insert = FALSE;
    while (1) {
        in_key = mvwgetch(win, flin, (fcol + fptr - fstart));
        switch (in_key) {

        // Accept Field
        case KEY_F(10):
            display_field(win, flin, fcol, fstart, flen, fprompt, fval);
            return (in_key);

        // Abort Operation
        case KEY_CTLC:
        case KEY_CTLD:
        case KEY_ESC:
        case KEY_BREAK:
        case KEY_F(9):
            display_field(win, flin, fcol, fstart, flen, fprompt, fval);
            in_key = KEY_F(9);
            return (in_key);

        case KEY_BTAB:
        case key_up:
        case KEY_UP:
        case KEY_F(7):
            display_field(win, flin, fcol, fstart, flen, fprompt, fval);
            in_key = KEY_UP;
            return (in_key);

        case KEY_TAB:
        case key_down:
        case KEY_DOWN:
            display_field(win, flin, fcol, fstart, flen, fprompt, fval);
            in_key = KEY_DOWN;
            return (in_key);

        case KEY_END:
            str_end = fptr;
            *fptr = '\0';
            display_field(win, flin, fcol, fstart, flen, fprompt, fval);
            return (in_key);

        case KEY_CTLM:
        case KEY_ENTER:
            if (f_erase_remainder) {
                str_end = fptr;
                *fptr = '\0';
            }
            in_key = KEY_ENTER;
            display_field(win, flin, fcol, fstart, flen, fprompt, fval);
            return (in_key);

        case KEY_IC:
            if (f_insert) {
                f_insert = FALSE;
            } else {
                f_insert = TRUE;
            }
            break;

        case KEY_DC:
            s = fptr + 1;
            d = fptr;
            while (*s != '\0')
                *d++ = *s++; // Shift remainder of line left one character
            *d = '\0';
            str_end = d;
            f_insert = FALSE;
            break;

        case KEY_HOME:
            fptr = fstart;
            break;

        case KEY_BACKSPACE:
            if (fptr > fstart)
                fptr--;
            s = fptr + 1;
            d = fptr;
            while (*s != '\0') /* remove one character */
                *d++ = *s++;
            *d = '\0';
            str_end = d; /* new end of string    */
            if (fptr == fstart) {
                display_field(win, flin, fcol, fstart, flen, fprompt, fval);
                return (KEY_STAB);
            }
            break;

        case KEY_CTLH:
        case KEY_LEFT:
            if (fptr > fstart)
                fptr--;
            if (fptr == fstart) {
                display_field(win, flin, fcol, fstart, flen, fprompt, fval);
                return (KEY_STAB);
            }
            break;

        case KEY_CTLL:
        case KEY_RIGHT:
            if (fptr < fend)
                if (fptr <= str_end)
                    fptr++; /* move one to the right */
            if (fptr > str_end) {
                display_field(win, flin, fcol, fstart, flen, fprompt, fval);
                return (KEY_ENTER);
            }
            break;

        default:
            if (in_key >= ' ') {
                if (f_insert) {
                    if (str_end < fend) {
                        s = str_end - 1;
                        d = str_end;
                        *++str_end = '\0';
                        while (s >= fptr)
                            *d-- = *s--;
                        *fptr++ = in_key;
                    }
                } else {
                    if (fptr == str_end) {
                        *fptr++ = in_key;
                        *fptr = '\0';
                        str_end = fptr;
                    } else {
                        *fptr++ = in_key;
                    }
                }
                break;
            }
        }
        s = fstart;
        d = buf;
        e = d + flen;
        while (*s != '\0' && d < e)
            *d++ = *s++;
        *d = '\0';
        if ((rc = format_field(buf, flen, fprompt, fval)) == -1) /* not full */
            mvwaddstr(win, flin, fcol, buf);
        else if (rc == 0) { /* full */
            mvwaddstr(win, flin, fcol, buf);
            if (in_key != KEY_LL)
                if (fptr == str_end)
                    return (KEY_DOWN);
        }
    }
}

void display_field(WINDOW *win, int flin, int fcol, char *fptr, int flen,
                   char fprompt, int fval) {
    char formatted_field[MAXLEN + 1];
    char *s;

    if (flen == 0) {
        s = fptr;
        while (*s++ != '\0')
            flen++;
    }
    wmove(win, flin, fcol - 1);
    waddch(win, '[');
    wmove(win, flin, fcol - 1);
    strcpy(formatted_field, fptr);
    (void)format_field(formatted_field, flen, fprompt, fval);
    mvwaddstr(win, flin, fcol, formatted_field);
    waddch(win, ']');
}

int format_field(char *fptr, int flen, char fprompt, int fval) {
    char *fstart; /* Field Start */
    char buf[MAXLEN + 1];
    char *buf_ptr;
    char *buf_start;
    char *s, *d, *e;
    unsigned int c, l;
    int formatted_len;

    if (flen > MAXLEN)
        display_error_message("format_field() invalid length");
    buf[0] = '\0';
    fstart = fptr;
    buf_ptr = buf_start = buf;
    while (*fptr != '\0') {
        if (fval & F_NOECHO)
            c = '#';
        else {
            c = *fptr;
            if (c < ' ') {
                if (c == '\t')
                    do {
                        *buf_ptr++ = ' ';
                        l = (int)(buf_ptr - buf_start);
                    } while (l % 8);
                else {
                    *buf_ptr++ = '^';
                    *buf_ptr++ = c | '@';
                }
            } else
                *buf_ptr++ = c;
        }
        fptr++;
    }
    formatted_len = (int)(buf_ptr - buf_start);
    d = buf_ptr;
    e = buf_start + flen;
    while (d < e)
        *d++ = fprompt;
    *d = '\0';
    s = buf;
    d = fstart;
    e = d + flen;
    while (*s != '\0' && d < e)
        *d++ = *s++;
    *d = '\0';
    if (formatted_len < flen)
        return (-1);
    if (formatted_len == flen)
        return (0);
    return (1);
}

int validate_field(char *fptr, int fval) {
    char *s;

    if (fval & F_NOTBLANK) {
        s = fptr;
        while (*s++ == ' ')
            ;
        if (*s == '\0') {
            display_error_message("blank field not allowed");
            return (1);
        }
    }
    if (fval & F_NOMETAS) {
        if (strpbrk(fptr, "*?[]") != 0) {
            display_error_message("metacharacters not allowed");
            return (1);
        }
    }
    return (0);
}
