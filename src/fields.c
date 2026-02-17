/** @file fields.c
    @brief Field Edit and Entry for C-Menu Form
    @author Bill Waller
    Copyright (c) 2025
    MIT License
    billxwaller@gmail.com
    @date 2026-02-09
 */

#include "common.h"
#include <errno.h>
#include <monetary.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

bool is_valid_date(int, int, int);
bool is_valid_time(int, int, int);
int form_fmt_field(Form *, char *);
void numeric(char *, char *);
void right_justify(char *, int);
void left_justify(char *);

char ff_tbl[][26] = {"string",   "decimal_int", "hex_int", "float", "double",
                     "currency", "yyyymmdd",    "hhmmss",  "apr",   ""};

int form_accept_field(Form *);
int form_display_field(Form *);
int form_display_field_n(Form *, int);
int form_display_field_brackets(Form *);
int form_validate_field(Form *);
void mk_filler(char *, int);

/** @brief Accept input for a field
     @param form Pointer to Form structure
     @return int Key code of action taken
     @note Handles character input, navigation keys, and mouse events for field
     editing. Validates input based on field format and updates display.
     Returns key code of action taken (e.g., accept field, tab to next field,
     break out of input loop).
 */
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
    mousemask(BUTTON1_CLICKED | BUTTON1_DOUBLE_CLICKED, NULL);
    MEVENT event;
    event.y = event.x = -1;
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
            wmove(win, flin, x);
            in_key = xwgetch(win);
        }
        switch (in_key) {
            /** KEY_F(10) is the default key for accepting the field and moving
                to the next field */
        case KEY_F(10):
            form_fmt_field(form, accept_s);
            form_display_field(form);
            if (form_validate_field(form) != 0)
                continue;
            return (in_key);
            /** KEY_F(9) Cancels the current operation */
        case KEY_BREAK:
        case KEY_F(9):
            form_display_field(form);
            in_key = KEY_F(9);
            return (in_key);
            /** KEY_UP, KEY_BTAB moves to the previous field */
        case 'H':
        case KEY_F(1):
            return (in_key);
        case KEY_BTAB:
        case KEY_UP:
            form_fmt_field(form, accept_s);
            form_display_field(form);
            in_key = KEY_UP;
            return (in_key);
            /** KEY_DOWN, TAB moves to the next field */
        case '\t':
        case KEY_DOWN:
            form_fmt_field(form, accept_s);
            form_display_field(form);
            in_key = KEY_DOWN;
            return (in_key);
            /** KEY_END moves cursor to end of field */
        case KEY_END:
            while (*p != '\0')
                p++;
            x = fcol + (p - fstart);
            in_key = 0;
            continue;
            /**< Enter accepts the field and moves to the next field
                if form->f_erase_remainder is set, it will clear the remaining
                characters above and after the current cursor location */
        case '\n':
        case KEY_ENTER:
            if (form->f_erase_remainder)
                *p = '\0';
            form_fmt_field(form, accept_s);
            form_display_field(form);
            in_key = KEY_ENTER;
            return (in_key);
            /** KEY_IC toggles insert mode */
        case KEY_IC:
            if (f_insert)
                f_insert = FALSE;
            else
                f_insert = TRUE;
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
            if (p < fend)
                if (p <= str_end) {
                    p++; // move one to the right
                    x++;
                }
            if (p > str_end) {
                mvwaddstr(win, flin, fcol, accept_s);
                return ('\n');
            }
            in_key = 0;
            continue;
            /** Handles mouse events for field editing */
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
                /// translate mouse position to field position
                if (event.y == form->lines - 1)
                    in_key = get_chyron_key(key_cmd, event.x);
            }
            continue;

        default:
            /** Validates fields based on format */
            switch (ff) {
                /** FF_STRING accepts all printable characters and spaces */
            case FF_STRING:
                break;
                /** FF_DECIMAL_INT accepts digits 0 through 9 and decimal point
                    ('.') */
            case FF_DECIMAL_INT:
                if ((in_key >= '0' && in_key <= '9') || in_key == '.')
                    break;
                beep();
                in_key = 0;
                continue;
                /** FF_HEX_INT accepts digits 0 through 9 and letters A through
                    F (case-insensitive) */
            case FF_HEX_INT:
                if ((in_key >= '0' && in_key <= '9') ||
                    (in_key >= 'A' && in_key <= 'F') ||
                    (in_key >= 'a' && in_key <= 'f'))
                    break;
                beep();
                in_key = 0;
                continue;
                /** FF_FLOAT accepts digits 0 through 9 and decimal point ('.')
                    and negative operator ('-') at the start of the field */
            case FF_FLOAT:
                if ((in_key >= '0' && in_key <= '9') || in_key == '.' ||
                    (in_key == '-' && p == fstart))
                    break;
                beep();
                in_key = 0;
                continue;
                /** FF_DOUBLE accepts digits 0 through 9 and decimal point ('.')
                    and negative operator ('-') at the start of the field */
            case FF_DOUBLE:
                if ((in_key >= '0' && in_key <= '9') || in_key == '.' ||
                    (in_key == '-' && p == fstart))
                    break;
                beep();
                in_key = 0;
                continue;
                /** FF_CURRENCY accepts digits 0 through 9 and decimal point
                    ('.') and negative operator ('-') at the start of the field
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
/** @brief Display field n
    @param form Pointer to Form structure
    @param n Field index to display
    @return 0 on success, non-zero on error
    @note This function temporarily sets the form's current field index (fidx)
   to n, calls form_display_field() to display that field, and then restores the
   original fidx value. This allows for displaying a specific field without
   permanently changing the form's current field index.
 */
int form_display_field_n(Form *form, int n) {
    /// Display field n
    int fidx = form->fidx;
    form->fidx = n;
    form_display_field(form);
    form->fidx = fidx;
    return 0;
}
/** @brief Display current field
    @param form Pointer to Form structure
    @return 0 on success, non-zero on error
    @note This function displays the current field based on the form's current
   field index (fidx). It retrieves the line and column information for the
   current field, displays any brackets if set, and then displays the field's
   content using the display_s string. The function ensures that the field is
   displayed correctly within the form's window and refreshes the display to
   show the updated field content.
 */
int form_display_field(Form *form) {
    /// Display current field
    WINDOW *win = form->win;
    int flin = form->field[form->fidx]->line;
    int fcol = form->field[form->fidx]->col;
    form_display_field_brackets(form);
    mvwaddstr(win, flin, fcol, form->field[form->fidx]->filler_s);
    mvwaddstr(win, flin, fcol, form->field[form->fidx]->display_s);
    wrefresh(win);
    return 0;
}
/** @brief Display brackets around current field if set
    @param form Pointer to Form structure
    @return 0 on success, non-zero on error
    @note This function checks if the form's brackets array has non-empty values
   for both the left and right brackets. If so, it retrieves the line and column
   information for the current field and uses the form's box window to display
   the left bracket at the start of the field and the right bracket at the end
   of the field. The display is refreshed to show the brackets around the field.
 */
int form_display_field_brackets(Form *form) {
    /// Display field brackets if set
    /// @param form Pointer to Form structure
    /// @return 0 on success, non-zero on error
    int flin, fcol;
    if (form->brackets[0] != '\0' && form->brackets[1] != '\0') {
        WINDOW *box = form->box;
        flin = form->field[form->fidx]->line + 1;
        fcol = form->field[form->fidx]->col;
        wmove(box, flin, fcol);
        waddch(box, form->brackets[0]);
        wmove(box, flin, fcol + form->field[form->fidx]->len + 1);
        waddch(box, form->brackets[1]);
        wrefresh(box);
    }
    return 0;
}
/** @brief Format field according to its format type
    @param form Pointer to Form structure
    @param s Input string to format
    @return 0 on success, non-zero on error
    @note This function takes the input string for the current field and formats
   it according to the field's specified format type (ff). It updates the
   accept_s and display_s strings for the field based on the formatted value.
   The function handles various format types, including strings, decimal
   integers, hexadecimal integers, floating-point numbers, currency, dates, and
   times. It uses helper functions for validation and formatting, such as
   is_valid_date(), is_valid_time(), numeric(), right_justify(), left_justify(),
   and strnzcpy(). The function ensures that the formatted output fits within
   the field's length and creates a filler string for the field as needed. The
   function also handles error cases, such as invalid formats, and provides
   feedback through error messages. It is designed to be extensible, allowing
   for additional format types to be added in the future as needed.
 */
int form_fmt_field(Form *form, char *s) {
    /** @brief Format field according to its format type
        @param form Pointer to Form structure
        @param s Input string to format
        @return 0 on success, non-zero on error

        @note takes the input string for the current field and formats it
       according to the field's specified format type (ff). It updates the
       accept_s and display_s strings for the field based on the formatted
       value.

        @note handles various format types, including strings, decimal integers,
       hexadecimal integers, floating-point numbers, currency, dates, and times.

        @note uses helper functions for validation and formatting, such as
       is_valid_date(), is_valid_time(), numeric(), right_justify(),
       left_justify(), and strnzcpy(). The function ensures that the formatted
       output fits within the field's length and creates a filler string for the
       field as needed. The function also handles error cases, such as invalid
       formats, and provides feedback through error messages. It is designed to
       be extensible, allowing for additional format types to be added in the
       future as needed.

        @ assumes that the input string is well-formed and does not contain
       malicious content. Input validation and sanitization should be performed
       at a higher level in the application to ensure security and robustness.
       @note The function currently does not handle localization or
       internationalization of number formats, such as different decimal
       separators or currency symbols. Future enhancements may include support
       for locale-specific formatting. @note Error handling is basic; future
       versions may include more detailed error reporting and logging
       mechanisms. @note Performance optimizations may be considered for
       handling large volumes of data or high-frequency updates in real-time
       applications. @note The following variables and structures are used in
       this function:
        @code
            char field_s[MAXLEN];
            int decimal_int_n = 0;
            int hex_int_n = 0;
            float float_n = 0.0;
            double double_n = 0.0;
            double currency_n = 0.0;
            struct Date { int yyyy; int mm; int dd; };
            struct Time { int hh; int mm; int ss; };
        @endcode
        @note supports format types: FF_STRING, FF_DECIMAL_INT, FF_HEX_INT,
       FF_FLOAT, FF_DOUBLE, FF_CURRENCY, FF_YYYYMMDD, FF_HHMMSS, FF_APR with
       appropriate formatting and validation for each type. @note Handles field
       length and filler string creation. @note As the roadmap indicates, these
       data types are just a start and more complex types and formats may be
       added in the future. @note These data types are not suitable for
       financial, scientific, or high-precision applications. They are intended
       for basic data entry and display in a text-based user interface.
        @note In the future, we will aqdd support for additional data types and
       formats such as: dates with time zones, timestamps, percentages,
       scientific notation, and custom user-defined formats.
        @note For high precision applications, we will be integrating support
       for 128-bit binary coded decimal (BCD) types and arbitrary precision
       decimal types using libraries such as the GNU MPFR library, IBM's
       decNumber, Rust Decimal, or the Intel Decimal Floating-Point Math
       Library.
        @note C-Menu Form converts each field's input string into internal
       numeric binary based on the field's specified format. The internal
       numberic binary is then formated and displayed, verifying the user's
       input and Form's interpretation of the user's input.
      */
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
        left_justify(s);
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
    left_justify(accept_s);
    mk_filler(filler_s, fl);
    return 0;
}
/** @brief Validate current field based on flags
    @param form Pointer to Form structure
    @return 0 if valid, 1 if invalid
    @note Very underdeveloped - only checks F_NOTBLANK and F_NOMETAS
 */
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
/** @brief Create filler string for field
    @param s Filler string to create
    @param fl Field length
    @note Fills the string s with the fill character specified in the form's
   fill_char array, repeated for the length of the field (fl). The resulting
   string is null-terminated. This filler string can be used to display empty
   fields or to clear the field area before displaying new content.
 */
void mk_filler(char *s, int fl) {
    char *e = s + fl;
    unsigned char c;

    c = form->fill_char[0];
    while (s != e)
        *s++ = c;
    *s = '\0';
}
/** @brief Left justify string by removing leading spaces
    @param s String to left justify
    @note This function takes a string s and removes any leading spaces,
   effectively left-justifying the text. It does this by finding the first
   non-space character and shifting the string to the left, overwriting the
   leading spaces. The resulting string is null-terminated.
 */
void left_justify(char *s) { trim(s); }
/** @brief Right justify string by removing trailing spaces and adding leading
   spaces
    @param s String to right justify
    @param fl Field length
    @note This function takes a string s and right-justifies it within a field
   of length fl. It first removes any trailing spaces from the string, then
   shifts the characters to the right end of the field, filling the left side
   with spaces. The resulting string is null-terminated and fits within the
   specified field length.
 */
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
/** @brief Check if a given date is valid, including leap years
    @param yyyy Year
    @param mm Month
    @param dd Day
    @return true if the date is valid, false otherwise
    @note This function checks if the provided year, month, and day constitute a
   valid date. It accounts for leap years when determining the number of days in
   February. The function returns true if the date is valid and false if it is
   not.
 */
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
/** @brief Check if a given time is valid
    @param hh Hour
    @param mm Minute
    @param ss Second
    @return true if the time is valid, false otherwise
    @note This function checks if the provided hour, minute, and second
   constitute a valid time. It ensures that hours are between 0 and 23, minutes
   and seconds are between 0 and 59. The function returns true if the time is
   valid and false if it is not.
 */
bool is_valid_time(int hh, int mm, int ss) {
    if (hh < 0 || hh > 23 || mm < 0 || mm > 59 || ss < 0 || ss > 59)
        return false;
    return true;
}
/** @brief Extract numeric characters from source string to destination string
    @param d Destination string
    @param s Source string
    @note This function takes a source string s and extracts only the numeric
   characters (digits 0-9), as well as dashes ('-') and periods ('.'), copying
   them into the destination string d. The resulting string in d is
   null-terminated. This is useful for processing input that may contain
   non-numeric characters, allowing the application to focus on the numeric
   content for further processing or validation.
 */
void numeric(char *d, char *s) {
    while (*s != '\0') {
        if (*s == '-' || *s == '.' || (*s >= '0' && *s <= '9'))
            *d++ = *s++;
        else
            s++;
    }
    *d = '\0';
}
