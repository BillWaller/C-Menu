/* fpaint.c
 * Data entry and editing for MENU
 * Bill Waller
 * billxwaller@gmail.com
 */

#include "menu.h"
#include <stdlib.h>
#include <string.h>

paint_ *paint = NULL;

int paint_form(char *, char *, int, int);
int paintfile(char *, char *, int, int);
void init_paint_struct();
void free_paint_struct();
int display_paint_screen();
int paint_enter_fields();
int read_description_file();
int get_disp_str(char *);
int read_answer_file();
int write_answer_file();
void help(char *);

int paint_form(char *desc_file, char *help_file, int begy, int begx) {
    char *d;
    int eargc;
    char *eargv[MAXARGS];
    char earg_str[MAXLEN + 1];
    int i;

    while (1) {
        if (paint == NULL) {
            init_paint_struct();
            paint->begy = begy;
            paint->begx = begx;
            strcpy(paint->desc_file_name, desc_file);
            if (help_file)
                strcpy(paint->help_file_name, help_file);
            if (read_description_file()) {
                free_paint_struct();
                return (1);
            }
            display_paint_screen();
            paint->field_pos = 0;
        }
        switch (paint_enter_fields()) {
        case P_ACCEPT:
            eargc = str_to_args(eargv, paint->cmd_str);
            for (i = 0; i < paint->fidx; i++) {
                if (strchr(paint->field[i]->str, ' ')) {
                    strncpy(tmp_str, "\"", MAX_COLS);
                    strncat(tmp_str, paint->field[i]->str, MAX_COLS);
                    strncat(tmp_str, "\"", MAX_COLS);
                    strncpy(paint->field[i]->str, tmp_str, MAX_COLS);
                }
                eargv[eargc++] = paint->field[i]->str;
            }
            eargv[eargc] = (char *)0;
            full_screen_fork_exec(eargv);
            win_del();
            paint->win = win_win[win_ptr];
            paint->box = win_box[win_ptr];
            free_paint_struct();
            return (0);
        case P_HELP:
            help(paint->help_file_name);
            restore_wins();
            break;
        case P_ERROR:
        case P_CANCEL:
            win_del();
            paint->win = win_win[win_ptr];
            paint->box = win_box[win_ptr];
            free_paint_struct();
            return (P_CANCEL);
        case P_REFUSE:
            break;
        case key_ctle:
            d = getenv("EDITOR");
            if (d == NULL || *d == '\0')
                strncpy(earg_str, DEFAULTEDITOR, MAXLEN);
            else
                strncpy(earg_str, d, MAXLEN);
            eargv[0] = earg_str;
            eargv[1] = paint->desc_file_name;
            eargv[2] = NULL;
            eargc = 2;
            full_screen_fork_exec(eargv);
            free_paint_struct();
            break;
        case key_ctlr:
            restore_wins();
        default:
            break;
        }
    }
}

int paintfile(char *desc_file, char *help_file, int begy, int begx) {
    char *d;
    char *eargv[MAXARGS];
    char earg_str[MAXLEN + 1];

    while (1) {
        if (paint == (paint_ *)0) {
            init_paint_struct();
            paint->begy = begy;
            paint->begx = begx;
            strcpy(paint->desc_file_name, desc_file);
            strcpy(paint->help_file_name, help_file);
            if (read_description_file() != 0) {
                free_paint_struct();
                return (1);
            }
            display_paint_screen();
            paint->field_pos = 0;
        }
        switch (paint_enter_fields()) {
        case P_ACCEPT:
            write_answer_file();
            win_del();
            paint->win = win_win[win_ptr];
            paint->box = win_box[win_ptr];
            free_paint_struct();
            return (0);
        case P_HELP:
            help(paint->help_file_name);
            restore_wins();
            break;
        case P_ERROR:
        case P_CANCEL:
            win_del();
            paint->win = win_win[win_ptr];
            paint->box = win_box[win_ptr];
            free_paint_struct();
            return (P_CANCEL);
        case P_REFUSE:
            break;
        case key_ctle:
            d = getenv("EDITOR");
            if (d == NULL || *d == '\0')
                strncpy(earg_str, DEFAULTEDITOR, MAXLEN);
            else
                strncpy(earg_str, d, MAXLEN);
            eargv[0] = earg_str;
            eargv[1] = paint->desc_file_name;
            eargv[2] = NULL;
            full_screen_fork_exec(eargv);
            free_paint_struct();
            break;
        case key_ctlr:
            restore_wins();
        default:
            break;
        }
    }
}

void init_paint_struct() {
    paint = (paint_ *)malloc(sizeof(paint_));
    if (paint == NULL) {
        sprintf(tmp_str, "malloc %d bytes paint_ failed\n",
                (int)sizeof(paint_));
        abend(-1, tmp_str);
    }
    paint->lines = 0;
    paint->cols = 0;
    paint->begy = 0;
    paint->begx = 0;
    paint->didx = 0;
    paint->title_line = 0;
    paint->fidx = 0;
    paint->field_pos = 0;
    paint->title[0] = '\0';
    paint->cmd_str[0] = '\0';
    paint->desc_file_name[0] = '\0';
    paint->help_file_name[0] = '\0';
    paint->answer_file_name[0] = '\0';
}

void free_paint_struct() {
    int i;

    for (i = 0; i < paint->fidx; i++)
        free(paint->field[i]);
    for (i = 0; i < paint->didx; i++)
        free(paint->disp[i]);
    free(paint);
    paint = (paint_ *)0;
}

int display_paint_screen() {
    int n;

    paint->lines = 0;
    for (n = 0; n < paint->didx; n++)
        if (paint->disp[n]->line > paint->lines)
            paint->lines = paint->disp[n]->line;
    for (n = 0; n < paint->fidx; n++)
        if (paint->field[n]->line > paint->lines)
            paint->lines = paint->field[n]->line;
    paint->lines += 3;
    if (paint->lines > (LINES - paint->begy))
        paint->lines = LINES - paint->begy;
    for (n = 0; n < paint->fidx; n++) {
        if (paint->field[n]->line >= (paint->lines - 2))
            paint->fidx = n;
    }
    if (paint->cols > (COLS - paint->begx - 1))
        paint->cols = COLS - paint->begx - 1;
    if (win_new(paint->lines, paint->cols, paint->begy, paint->begx,
                paint->title)) {
        strncpy(tmp_str, "win_new failed: ", MAXLEN);
        strncat(tmp_str, paint->title, MAXLEN);
        display_error_message(tmp_str);
        return (-1);
    }
    paint->win = win_win[win_ptr];
    paint->box = win_box[win_ptr];
    for (n = 0; n < paint->didx; n++) {
        strnz(paint->disp[n]->str, paint->cols);
        mvwaddstr(paint->win, paint->disp[n]->line, paint->disp[n]->col,
                  paint->disp[n]->str);
    }
    for (n = 0; n < paint->fidx; n++) {
        if (paint->field[n]->len > paint->cols)
            paint->field[n]->len = paint->cols;
        strnz(paint->field[n]->str, paint->cols);
        // mvwaddstr(paint->win, paint->field[n]->line, paint->field[n]->col,
        // paint->field[n]->str);
        display_field(paint->win, paint->field[n]->line, paint->field[n]->col,
                      paint->field[n]->str, paint->field[n]->len,
                      ACCEPT_PROMPT_CHAR, paint->field[n]->val);
    }
    wattron(paint->win, A_REVERSE);
    mvwaddstr(paint->win, paint->lines - 1, 0,
              " F1 help   F9 Cancel   F10 Accept ");
    wattroff(paint->win, A_REVERSE);
    return (0);
}

int paint_enter_fields() {
    int n, i;

    if (paint->fidx <= 0)
        return (-1);
    n = 0;
    while (1) {
        cmd_key = accept_field(paint->win, paint->field[n]->line,
                               paint->field[n]->col, paint->field_pos,
                               paint->field[n]->str, paint->field[n]->len,
                               ACCEPT_PROMPT_CHAR, paint->field[n]->val);
        if (mg_action == 1) {
            cmd_key = 0;
            if (mg_line == (paint->begy + paint->lines)) {
                mg_col -= paint->begx + 1;
                if (mg_col >= 1 && mg_col <= 7)
                    cmd_key = KEY_F(1);
                else if (mg_col >= 11 && mg_col <= 19)
                    cmd_key = KEY_F(9);
                else if (mg_col >= 23 && mg_col <= 32)
                    cmd_key = KEY_F(10);
            } else {
                mg_line -= paint->begy + 1;
                mg_col -= paint->begx + 1;
                for (i = 0; i < paint->fidx; i++) {
                    if (mg_line == paint->field[i]->line &&
                        mg_col >= paint->field[i]->col &&
                        mg_col < paint->field[i]->col + paint->field[i]->len) {
                        paint->field_pos = mg_col - paint->field[i]->col;
                        n = i;
                        break;
                    }
                }
            }
        }
        wrefresh(paint->win);
        switch (cmd_key) {
        case KEY_F(0):
        case KEY_F(10):
            if (validate_field(paint->field[n]->str, paint->field[n]->val))
                return (P_REFUSE);
            return (P_ACCEPT);
        case KEY_F(1):
            return (P_HELP);
        case KEY_F(9):
            return (P_CANCEL);
        case KEY_F(7):
        case KEY_UP:
            if (validate_field(paint->field[n]->str, paint->field[n]->val))
                return (P_REFUSE);
            if (n == 0)
                n = paint->fidx - 1;
            else
                n--;
            paint->field_pos = 0;
            break;
        case '\r':
        case KEY_ENTER:
            if (validate_field(paint->field[n]->str, paint->field[n]->val))
                return (P_REFUSE);
            if (n == paint->fidx - 1)
                return (P_ACCEPT);
            n++;
            paint->field_pos = 0;
            break;
        case KEY_F(8):
        case KEY_DOWN:
            if (validate_field(paint->field[n]->str, paint->field[n]->val))
                return (P_REFUSE);
            if (n == paint->fidx - 1)
                n = 0;
            else
                n++;
            paint->field_pos = 0;
            break;
        case key_ctlo: /* Insert line */
            for (i = (paint->fidx - 1); i > n; i--) {
                strncpy(paint->field[i]->str, paint->field[i - 1]->str,
                        MAX_COLS);
                display_field(paint->win, paint->field[i]->line,
                              paint->field[i]->col, paint->field[i]->str,
                              paint->field[i]->len, ACCEPT_PROMPT_CHAR,
                              paint->field[i]->val);
            }
            paint->field[i]->str[0] = '\0';
            n--;
            break;
        case key_ctlr:
            return (cmd_key);

        case key_ctlx: /* Delete line */
            for (i = n; i < paint->fidx; i++) {
                if (i == (paint->fidx - 1))
                    paint->field[i]->str[0] = '\0';
                else
                    strncpy(paint->field[i]->str, paint->field[i + 1]->str,
                            MAX_COLS);
                display_field(paint->win, paint->field[i]->line,
                              paint->field[i]->col, paint->field[i]->str,
                              paint->field[i]->len, ACCEPT_PROMPT_CHAR,
                              paint->field[i]->val);
            }
            n--;
            break;
        default:
            break;
        }
    }
}

int read_description_file() {
    FILE *Fp;
    char *tokptr, *s, *d, *e;
    int cols;
    char InBuf[BUFSIZ + 1];

    if ((Fp = fopen(paint->desc_file_name, "r")) == NULL) {
        strncpy(tmp_str, "description file ", MAXLEN);
        strncat(tmp_str, paint->desc_file_name, MAXLEN);
        strncat(tmp_str, " not found", MAXLEN);
        display_error_message(tmp_str);
        return (1);
    }
    paint->didx = 0;
    paint->fidx = 0;
    paint->cols = 34;
    paint->title_line = 0;
    while ((fgets(InBuf, MAXLEN, Fp)) != NULL) {
        switch ((int)InBuf[0]) {
        case '#': //        Comment line
            break;
        case '?': //        '?' should be help
            s = InBuf + 1;
            d = paint->cmd_str;
            e = d + MAXLEN;
            while (*s != '\n' && *s != '\0' && d < e)
                *d++ = *s++;
            *d = '\0';
            break;
        case '!': //        process executive lines
            //              !line!column!length!validation!command
            paint->field[paint->fidx] = (field_ *)malloc(sizeof(field_));
            if (paint->field[paint->fidx] == NULL) {
                sprintf(tmp_str,
                        "malloc %d bytes struct paint->field[%d] failed",
                        (int)sizeof(field_), paint->fidx);
                abend(-1, tmp_str);
            }
            s = InBuf;
            //
            //          paint->field[paint->fidx]->line
            //
            if ((tokptr = strtok(s, " \t!\n")) == (char *)0)
                break;
            s = (char *)0;
            paint->field[paint->fidx]->line = atoi(tokptr) - paint->title_line;
            if (paint->field[paint->fidx]->line < 0 ||
                paint->field[paint->fidx]->line >= MAXFIELDS) {
                display_error_message("invalid line for field");
                break;
            }
            //          paint->field[paint->fidx]->col
            if ((tokptr = strtok(s, " \t!\n")) == (char *)0)
                break;
            s = (char *)0;
            paint->field[paint->fidx]->col = atoi(tokptr);
            if (paint->field[paint->fidx]->col < 0 ||
                paint->field[paint->fidx]->col >= MAX_COLS) {
                display_error_message("invalid column for field");
                break;
            }
            //          paint->field[paint->fidx]->len
            if ((tokptr = strtok(s, " \t!\n")) == (char *)0)
                break;
            s = (char *)0;
            paint->field[paint->fidx]->len = atoi(tokptr);
            if (paint->field[paint->fidx]->len < 0 ||
                paint->field[paint->fidx]->len > MAX_COLS) {
                display_error_message("invalid length for field");
                break;
            }
            //          paint->field[paint->fidx]->val (Validation)
            if ((tokptr = strtok(s, " \t!\n")) == (char *)0)
                break;
            s = (char *)0;
            paint->field[paint->fidx]->val = atoi(tokptr);
            if (paint->field[paint->fidx]->val < 0 ||
                paint->field[paint->fidx]->val > 8) {
                display_error_message("invalid validation for field");
                break;
            }
            //          paint->cmd_str
            if ((tokptr = strtok(s, "\t\n\r")) == (char *)0)
                break;
            s = tokptr;
            d = paint->cmd_str;
            e = d + MAXLEN;
            while (*s != '\n' && *s != '\0' && d < e)
                *d++ = *s++;
            *d = '\0';
            cols = paint->field[paint->fidx]->col +
                   paint->field[paint->fidx]->len + 1;
            if (cols > paint->cols)
                paint->cols = cols;
            paint->field[paint->fidx]->str[0] = '\0';
            paint->fidx++;
            break;
        case ':':
            get_disp_str((char *)&InBuf[1]);
            break;
        default:
            break;
        }
    }
    fclose(Fp);
    if (paint->fidx < 1) {
        display_error_message("invalid paint description file");
        return (1);
    }
    return (0);
}

int get_disp_str(char *s) {
    char *d;
    int cols;

    d = tmp_str;
    while (*s != '\n' && *s != '\0' && *s != ':')
        *d++ = *s++;
    *d = '\0';

    paint->disp[paint->didx] = (disp_ *)malloc(sizeof(disp_));
    if (paint->disp[paint->didx] == NULL) {
        sprintf(tmp_str, "malloc %d bytes struct paint->disp[%d] failed",
                (int)sizeof(disp_), paint->didx);
        display_error_message(tmp_str);
        return (-1);
    }
    paint->disp[paint->didx]->line = atoi(tmp_str);
    if (paint->disp[paint->didx]->line < 0 ||
        paint->disp[paint->didx]->line > MAXFIELDS) {
        display_error_message("invalid line for text");
        return (1);
    }
    paint->disp[paint->didx]->col = 0;
    if (*s == ':') {
        d = tmp_str;
        s++;
        while (*s != '\n' && *s != '\0' && *s != ':')
            *d++ = *s++;
        *d = '\0';
        paint->disp[paint->didx]->col = atoi(tmp_str);
    }
    if (paint->disp[paint->didx]->col < 0 ||
        paint->disp[paint->didx]->col >= MAX_COLS) {
        display_error_message("invalid column for text");
        return (1);
    }
    while (*++s == ' ')
        ;
    d = paint->disp[paint->didx]->str;
    while (*s != '\n' && *s != '\0')
        *d++ = *s++;
    *d = '\0';
    cols = paint->disp[paint->didx]->col +
           strlen(paint->disp[paint->didx]->str) + 1;
    if (paint->didx == 0) {
        if (strlen(paint->disp[paint->didx]->str) != 0) {
            strncpy(paint->title, paint->disp[paint->didx]->str, MAX_COLS);
            paint->disp[paint->didx]->str[0] = '\0';
            paint->title_line = paint->disp[paint->didx]->line + 1;
            paint->disp[paint->didx]->line = 0;
            paint->didx++;
            cols += 3;
        }
    } else {
        paint->disp[paint->didx]->line -= paint->title_line;
        paint->didx++;
    }
    if (cols > paint->cols)
        paint->cols = cols;
    return (0);
}

int read_answer_file() {
    FILE *Fp;
    char InBuf[BUFSIZ];
    char *s, *e, *d;
    int n;

    if ((Fp = fopen(paint->answer_file_name, "r")) == NULL)
        return (1);
    n = 0;
    while ((fgets(InBuf, MAXLEN, Fp)) != NULL) {
        if (n < paint->fidx) {
            s = InBuf;
            e = s + MAX_COLS;
            while (*s != '\0' && *s != '\n' && *s != '\r' && s < e) {
                s++;
            }
            *s = '\0';
            s = InBuf;
            d = paint->field[n]->str;
            while (*s != '\0')
                *d++ = *s++;
            *d = '\0';
        }
    }
    fclose(Fp);
    return (0);
}

int write_answer_file() {
    int n;
    FILE *Fp;

    if ((Fp = fopen(paint->answer_file_name, "w")) == NULL) {
        display_error_message("unable to open answer file for write");
        return (1);
    }
    for (n = 0; n < paint->fidx; n++)
        fprintf(Fp, "%s\n", paint->field[n]->str);
    fclose(Fp);
    return (0);
}

void help(char *fs) {
    char *eargv[MAXARGS];
    int eargc;
    if (!*fs) {
        display_error_message("No help file specified in configuration");
        return;
    }
    eargv[0] = strdup("view");
    eargv[1] = fs;
    eargv[2] = (char *)0;
    eargc = 3;
    mview(eargc, eargv, 10, 40, paint->begy + 1, paint->begx + 4, "help",
          win_attr);
}
