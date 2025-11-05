/* fform.c
 * form - data entry and editing for menu system
 * Bill Waller
 * billxwaller@gmail.com
 */

#include "menu.h"
#include <stdlib.h>
#include <string.h>

#define D_COMMENT '#'
#define D_CMD '!'
#define D_FIELD 'F'
#define D_TEXT 'T'
#define D_HELP '?'
#define D_HEADER 'H'

int display_form_screen(Form *);
int form_enter_fields(Form *);
int read_form_description(Form *);
int process_form_text(Form *);
int read_form_answer(Form *);
int write_form_answer(Form *);
void form_usage();
int form_desc_error(int, char *, char *);

int form_process(Init *init) {
    char *d;
    int eargc;
    char *eargv[MAXARGS];
    char earg_str[MAXLEN + 1];
    int i;

    form = init->form;
    if (form == NULL) {
        display_error_message("FORM: form data structure is NULL");
    }

    while (1) {
        if (read_form_description(form)) {
            close_form(init);
            abend(EXIT_FAILURE, "FORM:read form description failed");
        }
        display_form_screen(form);
        form->field_pos = 0;
        switch (form_enter_fields(form)) {
        case P_ACCEPT:
            if (form->f_answer_spec)
                write_form_answer(form);
            if (form->f_cmd_spec) {
                strncpy(earg_str, form->cmd_spec, MAXLEN - 1);
                i = str_to_args(eargv, earg_str);
                while (i++ < form->fidx) {
                    strncpy(tmp_str, "\"", MAX_COLS - 1);
                    strncat(tmp_str, form->field[i]->str, MAX_COLS - 1);
                    strncat(tmp_str, "\"", MAX_COLS - 1);
                    eargv[i] = strdup(tmp_str);
                }
                eargv[i] = (char *)0;
                full_screen_fork_exec(eargv);
                while (--i > 0)
                    free(eargv[i]);
            }
            win_del();
            form->win = win_win[win_ptr];
            form->box = win_box[win_ptr];
            close_form(init);
            return (0);

        case P_HELP:
            strncpy(earg_str, HELP_CMD, MAXLEN - 1);
            strncat(earg_str, " ", MAXLEN - 1);
            strncat(earg_str, form->help_spec, MAXLEN - 1);
            eargc = str_to_args(eargv, earg_str);
            mview(init, eargc, eargv, 10, 68, form->begy + 1, form->begx + 4);
            restore_wins();
            break;
        case P_ERROR:
        case P_CANCEL:
            win_del();
            form->win = win_win[win_ptr];
            form->box = win_box[win_ptr];
            close_form(init);
            return (P_CANCEL);
        case P_REFUSE:
            break;
        case KEY_CTLE:
            d = getenv("EDITOR");
            if (d == NULL || *d == '\0')
                strncpy(earg_str, DEFAULTEDITOR, MAXLEN - 1);
            else
                strncpy(earg_str, d, MAXLEN - 1);
            eargv[0] = earg_str;
            eargv[1] = form->mapp_spec;
            eargv[2] = NULL;
            eargc = 2;
            full_screen_fork_exec(eargv);
            close_form(init);
            break;
        case KEY_CTLR:
            restore_wins();
        default:
            break;
        }
    }
}

int display_form_screen(Form *form) {
    int n;

    form->lines = 0;

    for (n = 0; n < form->didx; n++)
        if (form->text[n]->line > form->lines)
            form->lines = form->text[n]->line;

    for (n = 0; n < form->fidx; n++)
        if (form->field[n]->line > form->lines)
            form->lines = form->field[n]->line;

    form->lines += 3;
    if (form->lines > (LINES - form->begy))
        form->lines = LINES - form->begy;

    for (n = 0; n < form->fidx; n++) {
        if (form->field[n]->line >= (form->lines - 2))
            form->fidx = n;
    }

    if (form->cols > (COLS - form->begx - 1))
        form->cols = COLS - form->begx - 1;

    if (win_new(form->lines, form->cols, form->begy, form->begx, form->title)) {
        strncpy(tmp_str, "win_new failed: ", MAXLEN - 1);
        strncat(tmp_str, form->title, MAXLEN - 1);
        display_error_message(tmp_str);
        return (-1);
    }
    form->win = win_win[win_ptr];
    form->box = win_box[win_ptr];

    for (n = 0; n < form->didx; n++) {
        strnz(form->text[n]->str, form->cols);
        mvwaddstr(form->win, form->text[n]->line, form->text[n]->col,
                  form->text[n]->str);
    }

    for (n = 0; n < form->fidx; n++) {
        if (form->field[n]->len > form->cols)
            form->field[n]->len = form->cols;
        strnz(form->field[n]->str, form->cols);
        // mvwaddstr(form->win, form->field[n]->line, form->field[n]->col,
        // form->field[n]->str);
        // mvwaddstr(form->win, form->field[n]->line, form->field[n]->col - 1,
        //         "[");
        display_field(form->win, form->field[n]->line, form->field[n]->col,
                      form->field[n]->str, form->field[n]->len,
                      ACCEPT_PROMPT_CHAR, form->field[n]->val);
        // mvwaddstr(form->win, form->field[n]->line,
        //         form->field[n]->col + form->field[n]->len, "]");
    }
    // DISPLAY STATUS LINE
    wattron(form->win, A_REVERSE);
    mvwaddstr(form->win, form->lines - 1, 0,
              " F1 help   F9 Cancel   F10 Accept ");
    wattroff(form->win, A_REVERSE);
    return (0);
}

int form_enter_fields(Form *form) {
    int n, i;

    if (form->fidx <= 0)
        return (-1);
    n = 0;
    while (1) {
        cmd_key = accept_field(
            form->win, form->field[n]->line, form->field[n]->col,
            form->field_pos, form->field[n]->str, form->field[n]->len,
            ACCEPT_PROMPT_CHAR, form->field[n]->val, form->f_erase_remainder);
        if (mg_action == 1) {
            cmd_key = 0;
            if (mg_line == (form->begy + form->lines)) {
                mg_col -= form->begx + 1;
                if (mg_col >= 1 && mg_col <= 7)
                    cmd_key = KEY_F(1);
                else if (mg_col >= 11 && mg_col <= 19)
                    cmd_key = KEY_F(9);
                else if (mg_col >= 23 && mg_col <= 32)
                    cmd_key = KEY_F(10);
            } else {
                mg_line -= form->begy + 1;
                mg_col -= form->begx + 1;
                for (i = 0; i < form->fidx; i++) {
                    if (mg_line == form->field[i]->line &&
                        mg_col >= form->field[i]->col &&
                        mg_col < form->field[i]->col + form->field[i]->len) {
                        form->field_pos = mg_col - form->field[i]->col;
                        n = i;
                        break;
                    }
                }
            }
        }
        wrefresh(form->win);
        switch (cmd_key) {
        case KEY_F(0):
        case KEY_F(10):
            if (validate_field(form->field[n]->str, form->field[n]->val))
                return (P_REFUSE);
            return (P_ACCEPT);
        case KEY_F(1):
            return (P_HELP);
        case KEY_F(9):
            return (P_CANCEL);
        case KEY_F(7):
        case KEY_UP:
            if (validate_field(form->field[n]->str, form->field[n]->val))
                return (P_REFUSE);
            if (n == 0)
                n = form->fidx - 1;
            else
                n--;
            form->field_pos = 0;
            break;
        case '\r':
        case KEY_ENTER:
            if (validate_field(form->field[n]->str, form->field[n]->val))
                return (P_REFUSE);
            if (n == form->fidx - 1)
                return (P_ACCEPT);
            n++;
            form->field_pos = 0;
            break;
        case KEY_F(8):
        case KEY_DOWN:
            if (validate_field(form->field[n]->str, form->field[n]->val))
                return (P_REFUSE);
            if (n == form->fidx - 1)
                n = 0;
            else
                n++;
            form->field_pos = 0;
            break;
        case KEY_CTLO: /* Insert line */
            for (i = (form->fidx - 1); i > n; i--) {
                strncpy(form->field[i]->str, form->field[i - 1]->str, MAX_COLS);
                display_field(form->win, form->field[i]->line,
                              form->field[i]->col, form->field[i]->str,
                              form->field[i]->len, ACCEPT_PROMPT_CHAR,
                              form->field[i]->val);
            }
            form->field[i]->str[0] = '\0';
            n--;
            break;
        case KEY_CTLR:
            return (cmd_key);

        case KEY_CTLX: /* Delete line */
            for (i = n; i < form->fidx; i++) {
                if (i == (form->fidx - 1))
                    form->field[i]->str[0] = '\0';
                else
                    strncpy(form->field[i]->str, form->field[i + 1]->str,
                            MAX_COLS);
                display_field(form->win, form->field[i]->line,
                              form->field[i]->col, form->field[i]->str,
                              form->field[i]->len, ACCEPT_PROMPT_CHAR,
                              form->field[i]->val);
            }
            n--;
            break;
        default:
            break;
        }
    }
}

int read_form_description(Form *form) {
    FILE *form_desc_fp;
    char *token;
    char *s;
    int cols = 0;
    int i;
    int in_line_num = 0;
    char in_buf[BUFSIZ];
    char tmp_buf[BUFSIZ];
    char *tmp_buf_ptr;
    char tmp_str[BUFSIZ];
    char delim[5];
    char directive;

    form_desc_fp = fopen(form->mapp_spec, "r");
    if (form_desc_fp == NULL) {
        form_desc_error(in_line_num, in_buf,
                        "FORM: cannot open description file");
        return (1);
    }
    for (i = 0; i < MAXFIELDS; i++) {
        form->field[i] = (Field *)calloc(1, sizeof(Field));
        if (!form->field[i]) {
            sprintf(tmp_str, "FORM: calloc failed for fields");
            abend(EXIT_FAILURE, tmp_str);
        }
    }
    for (i = 0; i < MAXFIELDS; i++) {
        form->text[i] = (Text *)calloc(1, sizeof(Text));
        if (!form->field[i]) {
            sprintf(tmp_str, "FORM: calloc failed for text");
            abend(EXIT_FAILURE, tmp_str);
        }
    }
    // -----------------------------------------------------------
    // TEXT line form->text[form->didx]->line
    form->didx = 0;
    form->fidx = 0;
    form->cols = 34;
    form->title_line = 0;

    // D_COMMENT   '#' Comment line
    // D_CMD       '!' Command string - cmd_spec
    // D_IN_FILE   '<' In file
    // D_OUT_FILE  '>' Help file
    // D_HELP_FILE '?' Help file
    // D_HEADER    'H' Header line
    // D_FIELD     'F' Field
    // D_TEXT      'T' Text
    //
    while ((fgets(in_buf, MAXLEN, form_desc_fp)) != NULL) {
        s = in_buf;
        in_line_num++;
        while (*s == ' ' || *s == '\t')
            s++;
        if (strlen(s) < 4)
            continue;
        if (*s == D_COMMENT)
            continue;

        delim[0] = '\n';
        delim[1] = in_buf[1];
        delim[2] = '\0';
        strncpy(tmp_buf, in_buf, MAXLEN - 1);
        tmp_buf_ptr = tmp_buf;
        if (!(token = strtok(tmp_buf_ptr, delim))) {
            continue;
        }
        directive = *token;
        switch ((int)directive) {
        case D_COMMENT:
            break;

        // CMD   !:
        // -----------------------------------------------------------
        case D_CMD:
            // CMD         !:cmd_spec
            if (!(token = strtok(NULL, delim))) {
                form_desc_error(in_line_num, in_buf,
                                "FORM: cmd_spec delimiter");
                continue;
            }
            strncpy(form->cmd_spec, token, MAXLEN - 1);
            break;

        // HELP  ?:
        // -----------------------------------------------------------
        case D_HELP:
            // HELP        ?:help file
            if (!(token = strtok(NULL, delim))) {
                form_desc_error(in_line_num, in_buf,
                                "FORM: help_spec delimiter");
            }
            strncpy(form->help_spec, token, MAXLEN - 1);
            break;

        // FIELD F:line:column:length:validation:command
        // -----------------------------------------------------------
        case D_FIELD:
            // FIELD line form->field[form->fidx]->line
            // -----------------------------------------------------------
            if (form->field[form->fidx] == NULL) {
                sprintf(tmp_str, "FORM: calloc failed for fields");
                abend(EXIT_FAILURE, tmp_str);
            }
            if (!(token = strtok(NULL, delim))) {
                form_desc_error(in_line_num, in_buf,
                                "FORM: line number delimiter");
                return 1;
            }
            form->field[form->fidx]->line = atoi(token);
            if (form->field[form->fidx]->line < 0 ||
                form->field[form->fidx]->line >= MAXFIELDS) {
                form_desc_error(in_line_num, in_buf,
                                "FORM: invalid line number");
                return 1;
            }
            // FIELD column form->field[form->fidx]->col
            // -----------------------------------------------------------
            if (!(token = strtok(NULL, delim))) {
                form_desc_error(in_line_num, in_buf,
                                "FORM: column number delimiter");
                return 1;
            }
            form->field[form->fidx]->col = atoi(token);
            if (form->field[form->fidx]->col < 0 ||
                form->field[form->fidx]->col >= MAX_COLS) {
                form_desc_error(in_line_num, in_buf,
                                "FORM: invalid column number");
                break;
            }
            // FIELD length form->field[form->fidx]->len
            // -----------------------------------------------------------
            if (!(token = strtok(NULL, delim))) {
                strncpy(tmp_str, in_buf, MAXLEN - 1);
                form_desc_error(in_line_num, tmp_str, "FORM: length delimiter");
                break;
            }
            form->field[form->fidx]->len = atoi(token);
            if (form->field[form->fidx]->len < 0 ||
                form->field[form->fidx]->len > MAX_COLS) {
                form_desc_error(in_line_num, in_buf, "FORM: invalid length");
                break;
            }
            // FIELD validation form->field[form->fidx]->val
            // -----------------------------------------------------------
            if (!(token = strtok(NULL, delim))) {
                form_desc_error(in_line_num, in_buf,
                                "FORM: validation code delimiter");
                break;
            }
            form->field[form->fidx]->val = atoi(token);
            if (form->field[form->fidx]->val < 0 ||
                form->field[form->fidx]->val > 8) {
                form_desc_error(in_line_num, in_buf,
                                "FORM: invalid validation code");
                break;
            }
            // FORM form->cols
            // -----------------------------------------------------------
            cols =
                form->field[form->fidx]->col + form->field[form->fidx]->len + 1;
            if (cols > form->cols)
                form->cols = cols;
            // FORM fidx
            // -----------------------------------------------------------
            form->fidx++;
            break;

        // TEXT         !line!column!text
        // -----------------------------------------------------------
        case D_TEXT:
            if (form->text[form->didx] == NULL) {
                sprintf(tmp_str, "FORM: calloc failed for text");
                abend(EXIT_FAILURE, tmp_str);
            }
            // TEXT line form->text[form->didx]->line
            if (!(token = strtok(NULL, delim))) {
                form_desc_error(in_line_num, in_buf,
                                "FORM: line number delimiter");
                break;
            }
            form->text[form->didx]->line = atoi(token);
            if (form->text[form->didx]->line < 0 ||
                form->text[form->didx]->line >= MAXFIELDS) {
                form_desc_error(in_line_num, in_buf,
                                "FORM: invalid line number");
                break;
            }
            // TEXT column form->text[form->didx]->col
            // -----------------------------------------------------------
            if (!(token = strtok(NULL, delim))) {
                form_desc_error(in_line_num, in_buf,
                                "FORM: column number delimiter");
                break;
            }
            form->text[form->didx]->col = atoi(token);
            if (form->text[form->didx]->col < 0 ||
                form->text[form->didx]->col >= MAX_COLS) {
                form_desc_error(in_line_num, in_buf,
                                "FORM: invalid column number");
                break;
            }
            // TEXT str form->text[form->didx]->str
            // -----------------------------------------------------------
            if (!(token = strtok(NULL, delim))) {
                form_desc_error(in_line_num, in_buf, "FORM: text delimiter");
                break;
            }
            strncpy(form->text[form->didx]->str, token, MAX_COLS - 1);
            // TEXT len form->text[form->didx]->len
            // -----------------------------------------------------------
            form->text[form->didx]->len = strlen(form->text[form->didx]->str);
            if (form->text[form->didx]->len < 0 ||
                form->text[form->didx]->len > MAX_COLS) {
                form_desc_error(in_line_num, in_buf, "FORM: invalid length");
                break;
            }
            cols =
                form->text[form->didx]->col + form->text[form->didx]->len + 1;
            if (cols > form->cols)
                form->cols = cols;
            // TEXT form->didx
            // -----------------------------------------------------------
            form->didx++;
            break;

        // HEADER form->title
        // -----------------------------------------------------------
        case D_HEADER:
            if ((token = strtok(NULL, delim))) {
                strncpy(form->title, token, MAX_COLS - 1);
            }
            break;
        default:
            // Invalid directive
            // -----------------------------------------------------------
            form_desc_error(in_line_num, in_buf, "FORM: invalid directive");
            break;
        }
    }
    fclose(form_desc_fp);
    if (form->didx < 1 && form->fidx < 1) {
        i = 0;
        eargv[i++] = strdup("FORM: description file invalid");
        eargv[i++] = strdup(form->mapp_spec);
        eargv[i] = (char *)0;
        error_message(eargv);
        return (1);
    }
    return (0);
}

int read_form_answer(Form *form) {
    FILE *answer_fp;
    char in_buf[BUFSIZ];
    char *s, *e, *d;
    int n;

    answer_fp = fopen(form->answer_spec, "r");
    if (answer_fp == NULL)
        return (1);
    n = 0;
    while ((fgets(in_buf, MAXLEN, answer_fp)) != NULL) {
        if (n < form->fidx) {
            s = in_buf;
            e = s + MAX_COLS;
            while (*s != '\0' && *s != '\n' && *s != '\r' && s < e) {
                s++;
            }
            *s = '\0';
            s = in_buf;
            d = form->field[n]->str;
            while (*s != '\0')
                *d++ = *s++;
            *d = '\0';
        }
    }
    fclose(answer_fp);
    return (0);
}

int write_form_answer(Form *form) {
    int n;
    FILE *answer_fp;

    if ((answer_fp = fopen(form->answer_spec, "w")) == NULL) {
        strncpy(tmp_str, "FORM: Cannot open write ", MAXLEN - 1);
        strncat(tmp_str, form->answer_spec, MAXLEN - 1);
        display_error_message(tmp_str);
        return (1);
    }
    for (n = 0; n < form->fidx; n++)
        fprintf(answer_fp, "%s\n", form->field[n]->str);
    fclose(answer_fp);
    return (0);
}

void form_usage() {
    dump_opts_by_use("FORML: usage: ", "..f.");
    (void)fprintf(stderr, "\n");
    display_error_message("press any key to continue");
}

int form_desc_error(int in_line_num, char *in_buf, char *emsg) {
    char t0[MAXLEN];
    char t1[MAXLEN];
    int cmd_key;

    char **eargv = (char **)calloc(MAXARGS, sizeof(char *));
    int i = 0;
    eargv[i++] = strdup("FORM: error processing description file\n");
    eargv[i++] = strdup(form->mapp_spec);
    sprintf(t0, "LINE: %d:", in_line_num);
    strncpy(t1, t0, MAXLEN - 1);
    strncat(t1, " ", MAXLEN - 1);
    strncat(t1, in_buf, MAXLEN - 1);
    eargv[i++] = strdup(t1);
    eargv[i++] = strdup(emsg);
    eargv[i++] = (char *)0;
    cmd_key = error_message(eargv);
    for (--i; i >= 0; i--)
        free(eargv[i]);
    free(eargv);
    return cmd_key;
}
