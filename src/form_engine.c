/* form_engine.c
 * form - data entry and editing for menu system
 * Bill Waller
 * billxwaller@gmail.com
 */

#include "menu.h"
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>

#define D_COMMENT '#'
#define D_CMD '!'
#define D_FIELD 'F'
#define D_TEXT 'T'
#define D_HELP '?'
#define D_HEADER 'H'
#define D_CALC 'C'
#define D_QUERY 'Q'

unsigned int form_display_screen(Init *);
void form_display_chyron(Form *);
int form_enter_fields(Form *);
int form_parse_desc(Form *);
int form_process_text(Form *);
int form_read(Form *);
int form_write(Form *);
void form_usage();
int form_desc_error(int, char *, char *);
int form_exec_cmd(Init *);
int form_calculate(Init *);
int init_form(Init *, int, char **, int, int);
unsigned int form_engine(Init *);

/* ╭────────────────────────────────────────────────────────────────╮
   │ INIT_FORM                                                      │
   ╰────────────────────────────────────────────────────────────────╯ */
int init_form(Init *init, int argc, char **argv, int begy, int begx) {
    struct stat sb;
    char tmp_str[MAXLEN];
    int m;

    if (init->form != NULL)
        close_form(init);
    Form *form = new_form(init, argc, argv, begy, begx);
    if (init->form != form)
        abend(-1, "init->form != form\n");

    form->begy = begy + 1;
    form->begx = begx + 4;
    strnz__cpy(form->title, init->title, MAXLEN - 1);
    if (!form->f_in_spec || (form->in_spec[0] == '\0') ||
        (strcmp(form->in_spec, "-") == 0) ||
        strcmp(form->in_spec, "/dev/stdin") == 0) {
        strcpy(form->in_spec, "/dev/stdin");
        /*  ╭───────────────────────────────────────────────────────╮
            │ NO IN SPEC PROVIDED, USE STDIN                        │
            ╰───────────────────────────────────────────────────────╯
         */
        form->f_in_pipe = true;
        form->in_fp = fdopen(STDIN_FILENO, "r");

    } else {
        /*  ╭───────────────────────────────────────────────────────╮
            │ IN SPEC IS A FILE                                     │
            ╰───────────────────────────────────────────────────────╯
         */
        if (lstat(form->in_spec, &sb) == -1) {
            m = MAXLEN - 29;
            strncpy(tmp_str, "Can\'t stat form input file: ", m);
            m -= strlen(form->in_spec);
            strncat(tmp_str, form->in_spec, m);
            Perror(tmp_str);
            return (1);
        }
        if (sb.st_size == 0) {
            m = MAXLEN - 24;
            strncpy(tmp_str, "Form input file empty: ", m);
            m -= strlen(form->in_spec);
            strncat(tmp_str, form->in_spec, m);
            Perror(tmp_str);
            return (1);
        }
        if ((form->in_fp = fopen(form->in_spec, "rb")) == NULL) {
            m = MAXLEN - 29;
            strncpy(tmp_str, "Can't open form input file: ", m);
            m -= strlen(form->in_spec);
            strncat(tmp_str, form->in_spec, m);
            Perror(tmp_str);
            return (1);
        }
    }
    if (form->title[0] == '\0')
        strncpy(form->title, form->in_spec, MAXLEN - 1);
    form_engine(init);
    if (form->win)
        win_del();
    close_form(init);
    return 0;
}
/*  ╭───────────────────────────────────────────────────────────────╮
    │ FORM_ENGINE                                                   │
    ╰───────────────────────────────────────────────────────────────╯ */
unsigned int form_engine(Init *init) {
    int eargc;
    char *eargv[MAXARGS];
    char earg_str[MAXLEN + 1];
    int form_action;

    form = init->form;
    if (form == NULL) {
        Perror("FORM: form data structure is NULL");
    }
    if (form_parse_desc(form)) {
        close_form(init);
        abend(EXIT_FAILURE, "FORM:read form description failed");
    }
    form_read(form);
    form_display_screen(init);
    form->fidx = 0;
    form_action = 0;
    while (1) {
        if (form_action == 0)
            form_action = form_enter_fields(form);
        switch (form_action) {
        case P_ACCEPT:
            wmove(form->win, form->lines - 1, 0);
            wclrtoeol(form->win);
            if (form->f_calculate) {
                form_action = form_calculate(init);
                if (form_action == P_END) {
                    return 0;
                }
                if (form_action == P_HELP || form_action == P_CANCEL)
                    continue;
            }
            if (form->f_out_spec)
                form_write(form);
            if (form->f_cmd_spec)
                form_exec_cmd(init);
            return 0;
        case P_HELP:
            strnz__cpy(earg_str, HELP_CMD, MAXLEN - 1);
            strnz__cat(earg_str, " ", MAXLEN - 1);
            strnz__cat(earg_str, form->help_spec, MAXLEN - 1);
            eargc = str_to_args(eargv, earg_str, MAX_ARGS);
            mview(init, eargc, eargv, 10, 68, form->begy + 1, form->begx + 4,
                  form->title);
            restore_wins();
            form_action = 0;
            break;
        case P_CANCEL:
            return 0;
        default:
            form_action = 0;
            break;
        }
    }
}

/*  ╭───────────────────────────────────────────────────────────────╮
    │ FORM_CALCULATE                                                │
    ╰───────────────────────────────────────────────────────────────╯ */
int form_calculate(Init *init) {
    int i, c, rc;
    char earg_str[MAXLEN + 1];
    bool loop = true;
    form = init->form;
    set_fkey(10, "");
    set_fkey(5, "Calculate");
    form_display_chyron(form);
    mousemask(BUTTON1_CLICKED | BUTTON1_DOUBLE_CLICKED, NULL);
    MEVENT event;
    event.y = event.x = -1;
    tcflush(0, TCIFLUSH);
    c = wgetch(form->win);
    while (loop) {
        switch (c) {
        case KEY_F(1):
            return P_HELP;
        case KEY_F(5):
            if (form->f_out_spec)
                form_write(form);
            if (form->f_cmd_spec) {
                strnz__cpy(earg_str, form->cmd_spec, MAXLEN - 1);
                for (i = 0; i < form->fcnt; i++) {
                    strnz__cat(earg_str, " ", MAXLEN - 1);
                    strnz__cat(earg_str, form->field[i]->accept_s, MAXLEN - 1);
                }
            }
            if (form->f_out_spec) {
                strnz__cat(earg_str, " >", MAXLEN - 1);
                strnz__cat(earg_str, form->out_spec, MAXLEN - 1);
            }
            shell(earg_str);
            form_read(form);
            form_display_screen(init);
            rc = P_CONTINUE;
            loop = false;
            break;
        case KEY_F(9):
            rc = P_CANCEL;
            loop = false;
            break;
        case KEY_F(10):
            rc = P_END;
            loop = false;
            break;
        default:
            break;
        }
    }
    key_cmd[5].text[0] = '\0';
    strnz__cpy(key_cmd[10].text, "Accept", 26);
    form_display_chyron(form);
    return rc;
}
/*  ╭───────────────────────────────────────────────────────────────╮
    │ FORM_ENTER_FIELDS                                             │
    ╰───────────────────────────────────────────────────────────────╯ */
int form_enter_fields(Form *form) {
    if (form->fidx < 0)
        return (-1);
    while (1) {
        cmd_key = form_accept_field(form);
        wrefresh(form->win);

        switch (cmd_key) {

        case KEY_F(10):
            return (P_ACCEPT);

        case KEY_F(1):
            return (P_HELP);

        case KEY_F(5):
            if (form->f_calculate)
                return (P_CALC);
            break;

        case KEY_F(9):
            return (P_CANCEL);

        case KEY_UP:
            if (form->fidx != 0)
                form->fidx--;
            break;

        case '\r':
        case KEY_ENTER:
            if (form->fidx < form->fcnt - 1)
                form->fidx++;
            else if (form->fidx == form->fcnt - 1)
                return (P_ACCEPT);
            break;

        case KEY_DOWN:
            if (form->fidx < form->fcnt - 1)
                form->fidx++;
            else if (form->fidx == form->fcnt - 1)
                return (P_ACCEPT);
            break;

        default:
            break;
        }
    }
}
/*  ╭───────────────────────────────────────────────────────────────╮
    │ FORM_DISPLAY_SCREEN                                           │
    ╰───────────────────────────────────────────────────────────────╯ */
unsigned int form_display_screen(Init *init) {
    int n;

    form = init->form;
    form->lines = 0;
    for (n = 0; n < form->dcnt; n++)
        if (form->text[n]->line > form->lines)
            form->lines = form->text[n]->line;
    for (n = 0; n < form->fcnt; n++)
        if (form->field[n]->line > form->lines)
            form->lines = form->field[n]->line;
    form->lines += 3;
    if (form->lines > (LINES - form->begy))
        form->lines = LINES - form->begy;
    for (n = 0; n < form->fcnt; n++) {
        if (form->field[n]->line >= (form->lines - 2))
            form->fcnt = n;
    }
    form->cols += 2;
    if (form->cols > (COLS - form->begx - 1))
        form->cols = COLS - form->begx - 1;
    if (win_new(form->lines, form->cols, form->begy, form->begx, form->title)) {
        strnz__cpy(tmp_str, "win_new failed: ", MAXLEN - 1);
        strnz__cat(tmp_str, form->title, MAXLEN - 1);
        Perror(tmp_str);
        return (1);
    }
    form->win = win_win[win_ptr];
    form->box = win_box[win_ptr];
    for (n = 0; n < form->dcnt; n++) {
        strnz(form->text[n]->str, form->cols);
        mvwaddstr(form->win, form->text[n]->line, form->text[n]->col,
                  form->text[n]->str);
#ifdef DEBUG
        wrefresh(form->win);
#endif
    }
    for (n = 0; n < form->fcnt; n++) {
        if (form->field[n]->len > form->cols)
            form->field[n]->len = form->cols;
        strnz(form->field[n]->display_s, form->cols);
        form_display_field_n(form, n);
#ifdef DEBUG
        wrefresh(form->win);
#endif
    }
    for (n = 0; key_cmd[n].end_pos != -1; n++)
        key_cmd[n].text[0] = '\0';
    strnz__cpy(key_cmd[1].text, "F1 Help", 32);
    strnz__cpy(key_cmd[9].text, "F9 Cancel", 32);
    strnz__cpy(key_cmd[10].text, "F10 Accept", 32);
    form_display_chyron(form);
    return (0);
}
/*  ╭───────────────────────────────────────────────────────────────╮
    │ FORM_DISPLAY_CHYRON                                           │
    ╰───────────────────────────────────────────────────────────────╯ */
void form_display_chyron(Form *form) {
    int l;

    l = chyron_mk(key_cmd, form->chyron_s);
    wattron(form->win, A_REVERSE);
    mvwaddstr(form->win, form->lines - 1, 0, form->chyron_s);
    wattroff(form->win, A_REVERSE);
    wmove(form->win, form->lines - 1, l);
    wclrtoeol(form->win);
}
/*  ╭───────────────────────────────────────────────────────────────╮
    │ FORM_PARSE_DESCRIPTION                                        │
    ╰───────────────────────────────────────────────────────────────╯ */
int form_parse_desc(Form *form) {
    FILE *form_desc_fp;
    char *token;
    char *s;
    int cols = 0;
    int i, l;
    int in_line_num = 0;
    char in_buf[BUFSIZ];
    char tmp_buf[BUFSIZ];
    char *tmp_buf_p;
    char tmp_str[BUFSIZ];
    char delim[5];
    char directive;

    form_desc_fp = fopen(form->mapp_spec, "r");
    if (form_desc_fp == NULL) {
        ssnprintf(em0, MAXLEN - 65, "%s, line: %d", __FILE__, __LINE__ - 2);
        strnz__cpy(em1, "fopen ", MAXLEN - 65);
        strnz__cat(em1, form->mapp_spec, MAXLEN - 1);
        strerror_r(errno, em2, MAXLEN);
        display_error(em0, em1, em2, NULL);
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
    form->didx = 0;
    form->fidx = 0;
    form->fcnt = 0;
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
    // D_CALC      'Q' Calculate?
    // D_QUERY     'Q' Query?
    //
    /*  ╭───────────────────────────────────────────────────────────╮
        │ MAIN PARSE LOOP                                            │
        ╰───────────────────────────────────────────────────────────╯ */
    while ((fgets(in_buf, MAXLEN, form_desc_fp)) != NULL) {
        s = in_buf;
        in_line_num++;
        l = trim(in_buf);
        if (l == 0)
            continue;
        if (*s == D_COMMENT)
            continue;
        delim[0] = '\n';
        delim[1] = in_buf[1];
        delim[2] = '\0';
        strnz__cpy(tmp_buf, in_buf, MAXLEN - 1);
        tmp_buf_p = tmp_buf;
        if (!(token = strtok(tmp_buf_p, delim))) {
            continue;
        }
        directive = *token;
        switch ((int)directive) {
        case D_COMMENT:
            break;
        /*  ╭───────────────────────────────────────────────────╮
            │ 'C' Calculate                                     │
            ╰───────────────────────────────────────────────────╯ */
        case D_CALC:
            form->f_calculate = true;
            break;
            /*  ╭───────────────────────────────────────────────────╮
                │ 'Q' Query                                         │
                ╰───────────────────────────────────────────────────╯ */
        case D_QUERY:
            form->f_query = true;
            break;
            /*  ╭───────────────────────────────────────────────────╮
                │ '!' CMD                                           │
                ╰───────────────────────────────────────────────────╯ */
        case D_CMD:
            if (!(token = strtok(NULL, delim))) {
                form_desc_error(in_line_num, in_buf,
                                "FORM: cmd_spec delimiter");
                continue;
            }
            strnz__cpy(form->cmd_spec, token, MAXLEN - 1);
            break;
            /*  ╭───────────────────────────────────────────────────╮
                │ '?' HELP_FILE                                     │
                ╰───────────────────────────────────────────────────╯ */
        case D_HELP:
            if (!(token = strtok(NULL, delim))) {
                form_desc_error(in_line_num, in_buf,
                                "FORM: help_spec delimiter");
            }
            strnz__cpy(form->help_spec, token, MAXLEN - 1);
            break;
            /*  ╭───────────────────────────────────────────────────╮
                │ 'F' FIELD F:line:column:length:format:command     │
                ╰───────────────────────────────────────────────────╯ */
        case D_FIELD:
            /*  ╭───────────────────────────────────────────────────╮
                │ FIELD LINE                                        │
                ╰───────────────────────────────────────────────────╯ */
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
            /*  ╭───────────────────────────────────────────────────╮
                │ FIELD COL                                         │
                ╰───────────────────────────────────────────────────╯ */
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
            /*  ╭───────────────────────────────────────────────────╮
                │ FIELD LEN                                         │
                ╰───────────────────────────────────────────────────╯ */
            if (!(token = strtok(NULL, delim))) {
                strnz__cpy(tmp_str, in_buf, MAXLEN - 1);
                form_desc_error(in_line_num, tmp_str, "FORM: length delimiter");
                break;
            }
            form->field[form->fidx]->len = atoi(token);
            if (form->field[form->fidx]->len < 0 ||
                form->field[form->fidx]->len > MAX_COLS) {
                form_desc_error(in_line_num, in_buf, "FORM: invalid length");
                break;
            }
            /*  ╭───────────────────────────────────────────────────╮
                │ FIELD FORMAT                                      │
                ╰───────────────────────────────────────────────────╯ */
            if (!(token = strtok(NULL, delim))) {
                form_desc_error(in_line_num, in_buf,
                                "FORM: validation code delimiter");
                break;
            }
            form->field[form->fidx]->ff = -1;
            for (i = 0; i < FF_INVALID; i++) {
                str_to_lower(token);
                str_to_lower(ff_tbl[i]);
                if (!strcmp(token, ff_tbl[i])) {
                    form->field[form->fidx]->ff = i;
                    break;
                }
            }
            if (form->field[form->fidx]->ff < 0 ||
                form->field[form->fidx]->ff >= FF_INVALID) {
                form_desc_error(in_line_num, in_buf,
                                "FORM: invalid format code");
                break;
            }
            /*  ╭───────────────────────────────────────────────────╮
                │ FORM cols   form->cols                            │
                ╰───────────────────────────────────────────────────╯ */
            cols =
                form->field[form->fidx]->col + form->field[form->fidx]->len + 1;
            if (cols > form->cols)
                form->cols = cols;
            /*  ╭───────────────────────────────────────────────────╮
                │ FORM fidx                                         │
                ╰───────────────────────────────────────────────────╯ */
            form->fidx++;
            form->fcnt = form->fidx;
            break;
            /*  ╭───────────────────────────────────────────────────╮
                │ TEXT                                              │
                ╰───────────────────────────────────────────────────╯ */
        case D_TEXT:
            if (form->text[form->didx] == NULL) {
                sprintf(tmp_str, "FORM: calloc failed for text");
                abend(EXIT_FAILURE, tmp_str);
            }
            /*  ╭───────────────────────────────────────────────────╮
                │ TEXT line                                         │
                ╰───────────────────────────────────────────────────╯ */
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
            /*  ╭───────────────────────────────────────────────────╮
                │ TEXT col                                          │
                ╰───────────────────────────────────────────────────╯ */
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
            /*  ╭───────────────────────────────────────────────────╮
                │ TEXT str                                          │
                ╰───────────────────────────────────────────────────╯ */
            if (!(token = strtok(NULL, delim))) {
                form_desc_error(in_line_num, in_buf, "FORM: text delimiter");
                break;
            }
            strnz__cpy(form->text[form->didx]->str, token, MAXLEN - 1);
            /*  ╭───────────────────────────────────────────────────╮
                │ TEXT len                                          │
                ╰───────────────────────────────────────────────────╯ */
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
            /*  ╭───────────────────────────────────────────────────╮
                │ TEXT didx                                         │
                ╰───────────────────────────────────────────────────╯ */
            form->didx++;
            form->dcnt = form->didx;
            break;
        /*  ╭───────────────────────────────────────────────────────╮
            │ HEADER title                                          │
            ╰───────────────────────────────────────────────────────╯ */
        case D_HEADER:
            if ((token = strtok(NULL, delim))) {
                strnz__cpy(form->title, token, MAXLEN - 1);
            }
            break;
        default:
            form_desc_error(in_line_num, in_buf, "invalid directive");
            break;
        }
    }
    fclose(form_desc_fp);
    if (form->didx < 1 && form->fidx < 1) {
        ssnprintf(em0, MAXLEN - 65, "%s, line: %d", __FILE__, __LINE__);
        ssnprintf(em1, MAXLEN - 65, "%s", "Error in description file:");
        ssnprintf(em2, MAXLEN - 65, "%s", form->mapp_spec);
        display_error(em0, em1, em2, NULL);
        return (1);
    }
    return (0);
}
/*  ╭───────────────────────────────────────────────────────────────╮
    │ READ FORM IN FILE                                             │
    ╰───────────────────────────────────────────────────────────────╯ */
int form_read(Form *form) {
    FILE *in_fp;
    char in_buf[MAXLEN];
    char field[MAXLEN];

    in_fp = fopen(form->in_spec, "r");
    if (in_fp == NULL)
        return (1);
    form->fidx = 0;
    while ((fgets(in_buf, MAXLEN, in_fp)) != NULL) {
        if (form->fidx < MAXFIELDS)
            strnz__cpy(field, in_buf, MAXLEN - 1);
        form_fmt_field(form, field);
        form->fidx++;
    }
    fclose(in_fp);
    return (0);
}
/*  ╭───────────────────────────────────────────────────────────────╮
    │ FORM_EXEC_CMD                                                 │
    ╰───────────────────────────────────────────────────────────────╯ */
int form_exec_cmd(Init *init) {
    char earg_str[MAXLEN + 1];
    int i;
    strnz__cpy(earg_str, form->cmd_spec, MAXLEN - 1);
    for (i = 0; i < form->fcnt; i++) {
        strnz__cat(earg_str, " ", MAXLEN - 1);
        strnz__cat(earg_str, form->field[i]->accept_s, MAXLEN - 1);
    }
    if (form->f_out_spec && form->f_calculate) {
        strnz__cat(earg_str, " >", MAXLEN - 1);
        strnz__cat(earg_str, form->out_spec, MAXLEN - 1);
    }
    shell(earg_str);
    return 0;
}
/*  ╭───────────────────────────────────────────────────────────────╮
    │ WRITE FORM                                                    │
    ╰───────────────────────────────────────────────────────────────╯ */
int form_write(Form *form) {
    int n;
    if (!form->f_out_spec || (form->out_spec[0] == '\0') ||
        (strcmp(form->out_spec, "-") == 0) ||
        strcmp(form->out_spec, "/dev/stdout") == 0) {
        /*  ╭───────────────────────────────────────────────────────╮
            │ NO OUT SPEC PROVIDED, USE STDOUT                      │
            │ BUT DETACHED FROM TTY                                 │
            ╰───────────────────────────────────────────────────────╯ */
        strcpy(form->out_spec, "/dev/stdout");
        close(form->out_fd);
        form->out_fd = open(form->out_spec, O_CREAT | O_RDWR | O_TRUNC, 0644);
        dup2(form->out_fd, STDOUT_FILENO);
        form->out_fp = fdopen(STDOUT_FILENO, "w");
        form->f_out_spec = true;
        form->f_out_pipe = true;
    } else {
        /*  ╭───────────────────────────────────────────────────╮
            │ OUT SPEC IS A FILE                                │
            ╰───────────────────────────────────────────────────╯ */
        if ((form->out_fp = fopen(form->out_spec, "w")) == NULL) {
            ssnprintf(em0, MAXLEN - 65, "%s, line: %d", __FILE__, __LINE__ - 1);
            strerror_r(errno, em2, MAXLEN);
            display_error(em0, em1, em2, NULL);
            return (1);
        }
    }
    for (n = 0; n < form->fcnt; n++)
        fprintf(form->out_fp, "%s\n", form->field[n]->accept_s);
    fclose(form->out_fp);
    return (0);
}
/*  ╭───────────────────────────────────────────────────────────────╮
    │ FORM USAGE                                                    │
    ╰───────────────────────────────────────────────────────────────╯ */
void form_usage() {
    dump_opts_by_use("FORML: usage: ", "..f.");
    (void)fprintf(stderr, "\n");
    Perror("press any key to continue");
}
/*  ╭───────────────────────────────────────────────────────────────╮
    │ FORM_DESC_ERROR                                               │
    ╰───────────────────────────────────────────────────────────────╯ */
int form_desc_error(int in_line_num, char *in_buf, char *em) {
    int cmd_key;

    ssnprintf(em0, MAXLEN - 65, "%s: %s", __FILE__, em);
    ssnprintf(em1, MAXLEN - 65, "Desc file: %s, line: %d", form->mapp_spec,
              in_line_num);
    strnz__cpy(em2, in_buf, MAXLEN - 1);
    cmd_key = display_error(em0, em1, em2, NULL);
    return cmd_key;
}
