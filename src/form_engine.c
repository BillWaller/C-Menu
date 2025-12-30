// form_engine.c
// form - data entry and editing for menu system
// Bill Waller Copyright (c) 2025
// billxwaller@gmail.com

#include "menu.h"
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
#include <wait.h>

#define D_COMMENT '#'
#define D_CMD '!'
#define D_FIELD 'F'
#define D_TEXT 'T'
#define D_HELP '?'
#define D_HEADER 'H'
#define D_CALC 'C'
#define D_QUERY 'Q'

unsigned int form_display_screen(Init *);
void form_display_fields(Form *);
void form_display_chyron(Form *);
int form_enter_fields(Form *);
int form_parse_desc(Form *);
int form_process_text(Form *);
int form_read_data(Form *);
int form_write(Form *);
void form_usage();
int form_desc_error(int, char *, char *);
int form_exec_cmd(Init *);
int form_calculate(Init *);
int init_form(Init *, int, char **, int, int);
unsigned int form_engine(Init *);

// ╭────────────────────────────────────────────────────────────────╮
// │ INIT_FORM                                                      │
// ╰────────────────────────────────────────────────────────────────╯
int init_form(Init *init, int argc, char **argv, int begy, int begx) {
    if (init->form != NULL)
        close_form(init);
    init->mapp_spec[0] = '\0';
    init->form = new_form(init, argc, argv, begy, begx);
    form = init->form;
    if (!form->f_mapp_spec) {
        if (form->mapp_spec[0] == '\0') {
            Perror("Error: No form specification file given");
        } else {
            strcpy(tmp_str, "form->mapp_spec: ");
            strcat(tmp_str, form->mapp_spec);
            strcat(tmp_str, " not found");
            Perror(tmp_str);
        }
        close_curses();
        restore_shell_tioctl();
        return 1;
    }
    form->begy = begy + 1;
    form->begx = begx + 4;
    if ((form->f_in_spec && (form->in_spec[0] == '\0')) ||
        (strcmp(form->in_spec, "-") == 0) ||
        strcmp(form->in_spec, "/dev/stdin") == 0) {
        strcpy(form->in_spec, "/dev/stdin");
        form->f_in_pipe = true;
    }
    if (form->title[0] == '\0')
        strncpy(form->title, form->in_spec, MAXLEN - 1);
    form_engine(init);
    if (form->win)
        win_del();
    close_form(init);
    return 0;
}
//  ╭───────────────────────────────────────────────────────────────╮
//  │ FORM_ENGINE                                                   │
//  ╰───────────────────────────────────────────────────────────────╯
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
        return 0;
    }
    form_read_data(form);
    form_display_screen(init);
    form->fidx = 0;
    form_action = 0;
    while (1) {
        if (form_action == 0 || form_action == P_CONTINUE)
            form_action = form_enter_fields(form);
        switch (form_action) {
        case P_ACCEPT:
            wmove(form->win, form->lines - 1, 0);
            wclrtoeol(form->win);
            wrefresh(form->win);
            if (form->f_calculate) {
                form_action = form_calculate(init);
                if (form_action == P_HELP || form_action == P_CANCEL ||
                    form_action == P_CONTINUE || form_action == P_END)
                    continue;
                if (form_action == P_ACCEPT) {
                    form_action = P_END;
                    continue;
                }
            }
            break;
        case P_END:
            if (form->f_out_spec)
                form_write(form);
            if (form->f_receiver_cmd) {
                form_exec_cmd(init);
                form_action = P_CONTINUE;
                continue;
            }
            return 0;
        case P_HELP:
            strnz__cpy(earg_str, HELP_CMD, MAXLEN - 1);
            strnz__cat(earg_str, " ", MAXLEN - 1);
            strnz__cat(earg_str, form->help_spec, MAXLEN - 1);
            eargc = str_to_args(eargv, earg_str, MAX_ARGS);
            mview(init, eargc, eargv, 10, 68, form->begy + 1, form->begx + 4,
                  form->title);
            restore_wins();
            form_action = P_CONTINUE;
            break;
        case P_CANCEL:
            return 0;
        default:
            form_action = P_CONTINUE;
            break;
        }
    }
    return 0;
}

//  ╭───────────────────────────────────────────────────────────────╮
//  │ FORM_CALCULATE                                                │
//  ╰───────────────────────────────────────────────────────────────╯
int form_calculate(Init *init) {
    int i, c, rc;
    char earg_str[MAXLEN + 1];
    char *eargv[MAXARGS];
    bool loop = true;
    pid_t pid;
    int pipe_fd[2];

    form = init->form;
    set_fkey(5, "Calculate");
    mousemask(BUTTON1_CLICKED | BUTTON1_DOUBLE_CLICKED, NULL);
    MEVENT event;
    while (loop) {
        form_display_chyron(form);
        event.y = event.x = -1;
        tcflush(2, TCIFLUSH);
        c = wgetch(form->win);
        switch (c) {
        case KEY_F(1):
            return P_HELP;
        case KEY_F(5):
            if (form->f_out_spec)
                form_write(form);
            if (form->f_provider_cmd) {
                strnz__cpy(earg_str, form->provider_cmd, MAXLEN - 1);
                for (i = 0; i < form->fcnt; i++) {
                    strnz__cat(earg_str, " ", MAXLEN - 1);
                    strnz__cat(earg_str, form->field[i]->accept_s, MAXLEN - 1);
                }
                str_to_args(eargv, earg_str, MAXARGS);
                //  ╭───────────────────────────────────────────────╮
                //  │ SETUP PIPE FORK EXEC                          │
                //  ╰───────────────────────────────────────────────╯
                if (pipe(pipe_fd) == -1) {
                    Perror("pipe(pipe_fd) failed in init_form");
                    return (1);
                }
                if ((pid = fork()) == -1) {
                    Perror("fork() failed in init_form");
                    return (1);
                }
                if (pid == 0) { // Child
                    close(pipe_fd[P_READ]);
                    dup2(pipe_fd[P_WRITE], STDOUT_FILENO);
                    close(pipe_fd[P_WRITE]);
                    execvp(eargv[0], eargv);
                    strnz__cpy(tmp_str,
                               "Can't exec form start cmd: ", MAXLEN - 1);
                    strnz__cat(tmp_str, eargv[0], MAXLEN - 1);
                    Perror(tmp_str);
                    exit(EXIT_FAILURE);
                }
                // Back to parent
                close(pipe_fd[P_WRITE]);
                form->in_fp = fdopen(pipe_fd[P_READ], "rb");
                form->f_in_pipe = true;
                form_read_data(form);
                close(pipe_fd[P_READ]);
                waitpid(pid, NULL, 0);
                form_display_fields(form);
                set_fkey(8, "Edit");
                continue;
            }
            break;
        case KEY_F(8):
            if (is_set_fkey(8)) {
                loop = false;
                rc = P_CONTINUE;
                break;
            }
            continue;
        case KEY_F(9):
            loop = false;
            rc = P_CANCEL;
            break;
        case KEY_F(10):
            loop = false;
            rc = P_ACCEPT;
            break;
        default:
            break;
        }
    }
    unset_fkey(8);
    unset_fkey(5);
    form_display_chyron(form);
    return rc;
}
//  ╭───────────────────────────────────────────────────────────────╮
//  │ FORM_ENTER_FIELDS                                             │
//  ╰───────────────────────────────────────────────────────────────╯
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
//  ╭───────────────────────────────────────────────────────────────╮
//  │ FORM_DISPLAY_SCREEN                                           │
//  ╰───────────────────────────────────────────────────────────────╯
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
    // immedok(form->win, TRUE);
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
    form_display_fields(form);
    return 0;
}
void form_display_fields(Form *form) {
    int n;
    char fill_char = form->fill_char[0];
    for (n = 0; n < form->fcnt; n++) {
        if (form->field[n]->len > form->cols)
            form->field[n]->len = form->cols;
        strnfill(form->field[n]->filler_s, fill_char, form->field[n]->len);
        strnz(form->field[n]->display_s, form->field[n]->len);
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
    return;
}
//  ╭───────────────────────────────────────────────────────────────╮
//  │ FORM_DISPLAY_CHYRON                                           │
//  ╰───────────────────────────────────────────────────────────────╯
void form_display_chyron(Form *form) {
    int l;

    l = chyron_mk(key_cmd, form->chyron_s);
    wattron(form->win, A_REVERSE);
    mvwaddstr(form->win, form->lines - 1, 0, form->chyron_s);
    wattroff(form->win, A_REVERSE);
    wclrtoeol(form->win);
    wmove(form->win, form->lines - 1, l);
}
//  ╭───────────────────────────────────────────────────────────────╮
//  │ FORM_PARSE_DESCRIPTION                                        │
//  ╰───────────────────────────────────────────────────────────────╯
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
    //  ╭───────────────────────────────────────────────────────────╮
    //  │ MAIN PARSE LOOP                                           │
    //  ╰───────────────────────────────────────────────────────────╯
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
        //  ╭───────────────────────────────────────────────────────╮
        //  │ 'C' Calculate                                         │
        //  ╰───────────────────────────────────────────────────────╯
        case D_CALC:
            form->f_calculate = true;
            break;
            //  ╭───────────────────────────────────────────────────╮
            //  │ 'Q' Query                                         │
            //  ╰───────────────────────────────────────────────────╯
        case D_QUERY:
            form->f_query = true;
            break;
            //  ╭───────────────────────────────────────────────────╮
            //  │ '!' CMD                                           │
            //  ╰───────────────────────────────────────────────────╯
        case D_CMD:
            if (!(token = strtok(NULL, delim))) {
                form_desc_error(in_line_num, in_buf,
                                "FORM: receiver_cmd delimiter");
                continue;
            }
            strnz__cpy(form->receiver_cmd, token, MAXLEN - 1);
            break;
            //  ╭───────────────────────────────────────────────────╮
            //  │ '?' HELP_FILE                                     │
            //  ╰───────────────────────────────────────────────────╯
        case D_HELP:
            if (!(token = strtok(NULL, delim))) {
                form_desc_error(in_line_num, in_buf,
                                "FORM: help_spec delimiter");
            }
            strnz__cpy(form->help_spec, token, MAXLEN - 1);
            break;
            //  ╭───────────────────────────────────────────────────╮
            //  │ 'F' FIELD F:line:column:length:format:command     │
            //  ╰───────────────────────────────────────────────────╯
        case D_FIELD:
            //  ╭───────────────────────────────────────────────────╮
            //  │ FIELD LINE                                        │
            //  ╰───────────────────────────────────────────────────╯
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
            //  ╭───────────────────────────────────────────────────╮
            //  │ FIELD COL                                         │
            //  ╰───────────────────────────────────────────────────╯
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
            //  ╭───────────────────────────────────────────────────╮
            //  │ FIELD LEN                                         │
            //  ╰───────────────────────────────────────────────────╯
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
            //  ╭───────────────────────────────────────────────────╮
            //  │ FIELD FORMAT                                      │
            //  ╰───────────────────────────────────────────────────/
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
            //  ╭───────────────────────────────────────────────────╮
            //  │ FORM cols   form->cols                            │
            //  ╰───────────────────────────────────────────────────╯
            cols =
                form->field[form->fidx]->col + form->field[form->fidx]->len + 1;
            if (cols > form->cols)
                form->cols = cols;
            //  ╭───────────────────────────────────────────────────╮
            //  │ FORM fidx                                         │
            //  ╰───────────────────────────────────────────────────╯
            form->fidx++;
            form->fcnt = form->fidx;
            break;
            //  ╭───────────────────────────────────────────────────╮
            //  │ TEXT                                              │
            //  ╰───────────────────────────────────────────────────╯
        case D_TEXT:
            if (form->text[form->didx] == NULL) {
                sprintf(tmp_str, "FORM: calloc failed for text");
                abend(EXIT_FAILURE, tmp_str);
            }
            //  ╭───────────────────────────────────────────────────╮
            //  │ TEXT line                                         │
            //  ╰───────────────────────────────────────────────────╯
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
            //  ╭───────────────────────────────────────────────────╮
            //  │ TEXT col                                          │
            //  ╰───────────────────────────────────────────────────╯
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
            //  ╭───────────────────────────────────────────────────╮
            //  │ TEXT str                                          │
            //  ╰───────────────────────────────────────────────────╯
            if (!(token = strtok(NULL, delim))) {
                form_desc_error(in_line_num, in_buf, "FORM: text delimiter");
                break;
            }
            strnz__cpy(form->text[form->didx]->str, token, MAXLEN - 1);
            //  ╭───────────────────────────────────────────────────╮
            //  │ TEXT len                                          │
            //  ╰───────────────────────────────────────────────────╯
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
            //  ╭───────────────────────────────────────────────────╮
            //  │ TEXT didx                                         │
            //  ╰───────────────────────────────────────────────────╯
            form->didx++;
            form->dcnt = form->didx;
            break;
        //  ╭───────────────────────────────────────────────────────╮
        //  │ HEADER title                                          │
        //  ╰───────────────────────────────────────────────────────╯
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
//  ╭───────────────────────────────────────────────────────────────╮
//  │ READ FORM IN FILE                                             │
//  ╰───────────────────────────────────────────────────────────────╯
int form_read_data(Form *form) {
    struct stat sb;
    char in_buf[MAXLEN];
    char field[MAXLEN];

    if (!form->f_in_pipe)
        if ((lstat(form->in_spec, &sb) == -1) || (sb.st_size == 0) ||
            ((form->in_fp = fopen(form->in_spec, "rb")) == NULL)) {
            strncat(em0, form->in_spec, MAXLEN - 1);
            // if (errno)
            // strerror_r(errno, em1, MAXLEN - 1);
            // else
            if (sb.st_size == 0)
                strncpy(em1, "File is empty", MAXLEN - 1);
            else
                strncpy(em1, "File does not exist", MAXLEN - 1);
            strncpy(em2, "Fields will be blank or zero", MAXLEN - 1);
            cmd_key = display_error(em0, em1, em2, NULL);
            if (cmd_key == KEY_F(9))
                return (1);
        }
    if (form->in_fp == NULL)
        return (1);
    form->fidx = 0;
    while ((fgets(in_buf, MAXLEN, form->in_fp)) != NULL) {
        if (form->fidx < MAXFIELDS)
            strnz__cpy(field, in_buf, MAXLEN - 1);
        form_fmt_field(form, field);
        form->fidx++;
    }
    fclose(form->in_fp);
    return (0);
}
//  ╭───────────────────────────────────────────────────────────────╮
//  │ FORM_EXEC_CMD                                                 │
//  ╰───────────────────────────────────────────────────────────────╯
int form_exec_cmd(Init *init) {
    char earg_str[MAXLEN + 1];
    int i;
    strnz__cpy(earg_str, form->receiver_cmd, MAXLEN - 1);
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
//  ╭───────────────────────────────────────────────────────────────╮
//  │ WRITE FORM                                                    │
//  ╰───────────────────────────────────────────────────────────────╯
int form_write(Form *form) {
    int n;
    if (!form->f_out_spec || (form->out_spec[0] == '\0') ||
        (strcmp(form->out_spec, "-") == 0) ||
        strcmp(form->out_spec, "/dev/stdout") == 0) {
        //  ╭───────────────────────────────────────────────────────╮
        //  │ NO OUT SPEC PROVIDED, USE STDOUT                      │
        //  │ BUT DETACHED FROM TTY                                 │
        //  ╰───────────────────────────────────────────────────────╯
        strcpy(form->out_spec, "/dev/stdout");
        close(form->out_fd);
        form->out_fd = open(form->out_spec, O_CREAT | O_RDWR | O_TRUNC, 0644);
        if (form->out_fd == -1) {
            ssnprintf(em0, MAXLEN - 65, "%s, line: %d", __FILE__, __LINE__ - 1);
            strnz__cpy(em1, "open ", MAXLEN - 65);
            strnz__cat(em1, form->out_spec, MAXLEN - 1);
            strerror_r(errno, em2, MAXLEN);
            display_error(em0, em1, em2, NULL);
            return (1);
        }
        dup2(form->out_fd, STDOUT_FILENO);
        form->out_fp = fdopen(STDOUT_FILENO, "w");
        form->f_out_spec = true;
        form->f_out_pipe = true;
    } else {
        //  ╭───────────────────────────────────────────────────────╮
        //  │ OUT SPEC IS A FILE                                    │
        //  ╰───────────────────────────────────────────────────────╯
        if ((form->out_fp = fopen(form->out_spec, "w")) == NULL) {
            ssnprintf(em0, MAXLEN - 65, "%s, line: %d", __FILE__, __LINE__ - 1);
            strerror_r(errno, em2, MAXLEN);
            display_error(em0, em1, em2, NULL);
            return (1);
        }
    }
    for (n = 0; n < form->fcnt; n++)
        fprintf(form->out_fp, "%s\n", form->field[n]->accept_s);
    if (form->out_fp != NULL)
        fclose(form->out_fp);
    return (0);
}
//  ╭───────────────────────────────────────────────────────────────╮
//  │ FORM USAGE                                                    │
//  ╰───────────────────────────────────────────────────────────────╯
void form_usage() {
    dump_opts_by_use("FORML: usage: ", "..f.");
    (void)fprintf(stderr, "\n");
    Perror("press any key to continue");
}
//  ╭───────────────────────────────────────────────────────────────╮
//  │ FORM_DESC_ERROR                                               │
//  ╰───────────────────────────────────────────────────────────────╯
int form_desc_error(int in_line_num, char *in_buf, char *em) {
    int cmd_key;

    ssnprintf(em0, MAXLEN - 65, "%s: %s", __FILE__, em);
    ssnprintf(em1, MAXLEN - 65, "Desc file: %s, line: %d", form->mapp_spec,
              in_line_num);
    strnz__cpy(em2, in_buf, MAXLEN - 1);
    cmd_key = display_error(em0, em1, em2, NULL);
    return cmd_key;
}
