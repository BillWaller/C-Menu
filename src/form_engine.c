/** @file form_engine.c
    @brief The working part of C-Menu Form
    @author Bill Waller
    Copyright (c) 2025
    MIT License
    billxwaller@gmail.com
    @date 2026-02-09
 */

/** @defgroup form_engine Form Engine
    @brief Parses Form Descriptions, Handles User Input, and Integrates with
   External Commands for Calculations and Data Processing.
 */

#include <common.h>
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
int form_read_data(Form *);
int form_write(Form *);
void form_usage();
int form_desc_error(int, char *, char *);
int form_exec_cmd(Form *);
int form_calculate(Init *);
int form_end_fields(Init *);
int init_form(Init *, int, char **, int, int);
int form_engine(Init *);
int form_yx_to_fidx(Form *, int, int);

/** @brief Initialize form data structure and parse description file
    @ingroup form_engine
    @param init A pointer to the Init structure containing form data and state.
    @param argc The number of command line arguments passed to the form.
    @param argv The array of command line arguments passed to the form.
    @param begy The y-coordinate for the top-left corner of the form window.
    @param begx The x-coordinate for the top-left corner of the form window.
    @return 0 on success, or a non-zero value if an error occurs during
*/
int init_form(Init *init, int argc, char **argv, int begy, int begx) {
    int rc;
    char tmp_str[MAXLEN];
    if (init->form != nullptr)
        destroy_form(init);
    init->form = new_form(init, argc, argv, begy, begx);
    form = init->form;
    if (!form->f_mapp_spec) {
        if (form->mapp_spec[0] == '\0') {
            rc = Perror("Error: No form specification file given");
        } else {
            strnz__cpy(tmp_str, "form->mapp_spec: ", MAXLEN - 1);
            strnz__cat(tmp_str, form->mapp_spec, MAXLEN - 1);
            strnz__cat(tmp_str, " not found", MAXLEN - 1);
            rc = Perror(tmp_str);
        }
        return rc;
    }
    if (begy != 0)
        form->begy = begy;
    if (begx != 0)
        form->begx = begx;
    if ((form->f_in_spec && (form->in_spec[0] == '\0')) ||
        (strcmp(form->in_spec, "-") == 0) ||
        strcmp(form->in_spec, "/dev/stdin") == 0) {
        strnz__cpy(form->in_spec, "/dev/stdin", MAXLEN - 1);
        form->f_in_pipe = true;
    }
    if (form->title[0] == '\0')
        strnz__cpy(form->title, form->in_spec, MAXLEN - 1);
    rc = form_engine(init);
    if (form->win)
        win_del();
    destroy_form(init);
    return rc;
}
/** @brief Form main processing loop
    @ingroup form_engine
    @param init A pointer to the Init structure containing form data and state.
    @return 0 on successful completion, or a non-zero value if the user cancels
    the form or if an error occurs during processing.
        1. Parse the form description file to populate the form data structure.
        2. Read any initial data for the form fields from a specified input
        source.
        3. Display the form on the screen with the initial field values.
        4. Enter a loop to handle user input for field entry, calculation, help,
        and cancellation:
          a. If the user selects the accept action, perform any necessary
          calculations or post-processing, and then either return to field entry
          or exit the loop if the form is accepted.
          b. If the user selects the help action, display the help screen and
          return to the form after the user exits the help screen.
          c. If the user selects the cancel action, exit the loop and return a
          cancel status. */
int form_engine(Init *init) {
    char tmp_str[MAXLEN];
    int form_action;

    form = init->form;
    if (form == nullptr) {
        Perror("FORM: form data structure is nullptr");
    }
    if (form_parse_desc(form)) {
        destroy_form(init);
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
            if (form->f_calculate)
                form_action = form_calculate(init);
            else
                form_action = form_end_fields(init);
            if (form_action == P_HELP || form_action == P_CANCEL ||
                form_action == P_CONTINUE || form_action == P_END)
                continue;
            if (form_action == P_ACCEPT) {
                form_action = P_END;
                continue;
            }
            break;
        case P_END:
            if (form->f_out_spec || form->out_spec[0] != '\0')
                form_write(form);
            if (form->f_receiver_cmd) {
                form_exec_cmd(form);
                form_action = P_CONTINUE;
                continue;
            }
            return 0;
        case P_HELP:
            eargv[0] = strdup("view");

            if (pick->f_help_spec && pick->help_spec[0] != '\0')
                strnz__cpy(tmp_str, pick->help_spec, MAXLEN - 1);
            else {
                strnz__cpy(tmp_str, init->mapp_help, MAXLEN - 1);
                strnz__cat(tmp_str, "/", MAXLEN - 1);
                strnz__cat(tmp_str, PICK_HELP_FILE, MAXLEN - 1);
            }
            eargv[1] = strdup(tmp_str);
            eargv[2] = nullptr;
            eargc = 2;
            init->lines = 30;
            init->cols = 60;
            init->begy = form->begy + 1;
            init->begx = form->begx + 1;
            strnz__cpy(init->title, "Form Help", MAXLEN - 1);
            popup_view(init, eargc, eargv);
            form_action = P_CONTINUE;
            break;
        case P_CANCEL:
            return P_CANCEL;
        default:
            form_action = P_CONTINUE;
            break;
        }
    }
    return 0;
}
/** @brief Handle post-processing after field entry, allowing user to edit data,
    execute a provider command, or write data to an output file.
    @ingroup form_engine
    @param init A pointer to the Init structure containing form data and state.
    @return An integer status code indicating the next action for the form
    processing loop (e.g., P_CONTINUE, P_CANCEL, P_ACCEPT, P_HELP). */
int form_end_fields(Init *init) {
    bool loop = true;
    int c, rc;
    click_y = click_x = -1;
    form = init->form;
    wmove(form->win, form->lines - 1, 0);
    wclrtoeol(form->win);
    unset_chyron_key(form->chyron, 18);
    set_chyron_key(form->chyron, 8, "F8 Edit", KEY_F(8));
    set_chyron_key(form->chyron, 10, "F10 Commit", KEY_F(10));
    rc = -1;
    while (loop) {
        if (rc == -1) {
            form_display_chyron(form);
            tcflush(2, TCIFLUSH);
            c = xwgetch_s(form->win, form->chyron, -1);
        }
        switch (c) {
        case KEY_F(1):
            return P_HELP;
        case KEY_F(8):
            if (is_set_chyron_key(form->chyron, 8)) {
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
        case KEY_MOUSE:
            continue;
        default:
            break;
        }
    }
    unset_chyron_key(form->chyron, 8);
    form_display_chyron(form);
    return rc;
}

/** @brief Handle integration with an external program for calculation, allowing
    the user to execute a provider command and read results back into the form
    fields.
    @ingroup form_engine
    @param init A pointer to the Init structure containing form data and state.
    @return An integer status code indicating the next action for the form
    processing loop (e.g., P_CONTINUE, P_CANCEL, P_ACCEPT, P_HELP).
    @details This function provides integration with external programs.
        The requirements are:
        1. The form description file must have a line starting with 'C' to
        indicate that the form supports calculation.
        2. The form description file must specify the provider command using a
        line starting with '!' followed by the command and its arguments.
        3. The external program must be able to accept field data from a file,
        from standard input, or as command line parameters.
        4. The external program must output the calculated field values in a
        format that can be read by the form (e.g., one value per line), either
        to a file or to standard output.

        The sequence of operations is as follows:

        1. The 'C' option causes Form to pause and display an KEY_F(5) Calculate
        option on the chyron.
        2. The user can then cancel the operation by pressing KEY_F(9) or
        activate the calculate option by pressing KEY_F(5).
        3. Form outputs its data to file, standard output, or as command line
        parameters.
        4. Form executes the external program.
        5. The external program processes the data and outputs the results.
        6. Form reads the results and populates the appropriate form fields.
        7. Form presents the user with an option to edit the data, and the
        sequence restarts at 1.

        This function forks and executes the provider executable as a child
        process, creates a pipe to read the output from the provider command,
        reads the output, and updates the Form fields.
            */
int form_calculate(Init *init) {
    int i, c, rc;
    char earg_str[MAXLEN + 1];
    char *eargv[MAXARGS];
    char file_spec[MAXLEN];
    bool loop = true;
    pid_t pid;
    int pipe_fd[2];

    form = init->form;
    wmove(form->win, form->lines - 1, 0);
    wclrtoeol(form->win);
    unset_chyron_key(form->chyron, 18);
    unset_chyron_key(form->chyron, 10);
    set_chyron_key(form->chyron, 5, "F5 Calculate", KEY_F(5));

    while (loop) {
        form_display_chyron(form);
        click_y = click_x = -1;
        tcflush(2, TCIFLUSH);
        c = xwgetch_s(form->win, form->chyron, -1);
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
                strnz__cpy(file_spec, eargv[0], MAXLEN - 1);
                base_name(eargv[0], file_spec);
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
                    ssnprintf(em0, MAXLEN, "%s, line: %d", __FILE__,
                              __LINE__ - 2);
                    strnz__cpy(em1, "execvp(", MAXLEN - 1);
                    strnz__cat(em1, eargv[0], MAXLEN - 1);
                    strnz__cat(em1, ", ", MAXLEN - 1);
                    strnz__cat(em1, earg_str, MAXLEN - 1);
                    strnz__cat(em1, ")", MAXLEN - 1);
                    strerror_r(errno, em2, MAXLEN);
                    display_error(em0, em1, em2, nullptr);
                    exit(EXIT_FAILURE);
                } // Back to parent
                close(pipe_fd[P_WRITE]);
                form->in_fp = fdopen(pipe_fd[P_READ], "rb");
                form->f_in_pipe = true;
                form_read_data(form);
                close(pipe_fd[P_READ]);
                waitpid(pid, nullptr, 0);
                form_display_fields(form);
                set_chyron_key(form->chyron, 8, "F5 Edit", KEY_F(5));
                set_chyron_key(form->chyron, 10, "F10 Commit", KEY_F(10));
                continue;
            }
            break;
        case KEY_F(8):
            if (is_set_chyron_key(form->chyron, 8)) {
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
        case KEY_MOUSE:
            break;
        default:
            break;
        }
    }
    unset_chyron_key(form->chyron, 8);
    unset_chyron_key(form->chyron, 5);
    form_display_chyron(form);
    return rc;
}
/** @brief Handle user input for field entry, allowing navigation between fields
    and looping until an exit action is selected.
    @ingroup form_engine
    @param form A pointer to the Form structure containing form data and state.
    @return An integer status code indicating the next action for the form
    processing loop (e.g., P_ACCEPT, P_HELP, P_CALC, P_CANCEL).
    @note This function manages user input for field entry, including navigation
   between fields and handling of special keys for accepting, canceling,
   requesting help, or performing calculations. The function loops until the
   user selects an exit action (e.g., accept or cancel). */
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
        case KEY_MOUSE:
            break;
        default:
            break;
        }
    }
}
int form_yx_to_fidx(Form *form, int y, int x) {
    for (int i = 0; i < form->fcnt; i++) {
        if (y == form->field[i]->line && x >= form->field[i]->col &&
            x < form->field[i]->col + form->field[i]->len) {
            return i;
        }
    }
    return -1; // No field found at the given coordinates
}

/** @brief Display the form on the screen, including text elements and fields,
    and set up the form window based on the form configuration.
    @ingroup form_engine
    @param init A pointer to the Init structure containing form data and state.
    @return 0 on success, or a non-zero value if an error occurs while
    creating the form window or rendering the form elements. */
unsigned int form_display_screen(Init *init) {
    int n;
    char tmp_str[MAXLEN];

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
    if (form->cols > (COLS - form->begx - 3))
        form->cols = COLS - form->begx - 3;
    if (win_new(form->lines, form->cols, form->begy, form->begx, form->title,
                0)) {
        strnz__cpy(tmp_str, "kwin_new failed: ", MAXLEN - 1);
        strnz__cat(tmp_str, form->title, MAXLEN - 1);
        Perror(tmp_str);
        return (1);
    }
#ifdef DEBUG
    immedok(form->win, TRUE);
#endif
    form->win = win_win[win_ptr];
    form->box = win_box[win_ptr];
    for (n = 0; n < form->dcnt; n++) {
        strnz(form->text[n]->str, form->cols - 3);
        mvwaddstr(form->win, form->text[n]->line, form->text[n]->col,
                  form->text[n]->str);
#ifdef DEBUG
        wrefresh(form->win);
#endif
    }
    form_display_fields(form);
    return 0;
}
/** @brief Display form fields on the screen, populating field values and
    formatting them according to the form configuration.
    @ingroup form_engine
    @param form A pointer to the Form structure containing form data and state.
    @note This function iterates through the defined form fields, formats their
   display values based on the specified fill character and field length, and
   renders them on the form window. It also updates the chyron with available
   commands for user interaction. */
void form_display_fields(Form *form) {
    int n;
    char fill_char = form->fill_char[0];
    for (n = 0; n < form->fcnt; n++) {
        if (form->field[n]->col + form->field[n]->len + 2 > form->cols)
            form->field[n]->len = form->cols - (form->field[n]->col + 2);
        strnfill(form->field[n]->filler_s, fill_char, form->field[n]->len);
        strnz(form->field[n]->display_s, form->field[n]->len);
        form_display_field_n(form, n);
#ifdef DEBUG
        wrefresh(form->win);
#endif
    }
    form->chyron = new_chyron();
    set_chyron_key(form->chyron, 1, "F1 Help", KEY_F(1));
    set_chyron_key(form->chyron, 9, "F9 Cancel", KEY_F(9));
    set_chyron_key(form->chyron, 10, "F10 Continue", KEY_F(10));
    compile_chyron(form->chyron);
    form_display_chyron(form);
    return;
}
/** @brief Display the chyron (status line) at the bottom of the form window,
   showing available commands and their corresponding function keys.
    @ingroup form_engine
    @param form A pointer to the Form structure containing form data and state.
 */
void form_display_chyron(Form *form) {
    compile_chyron(form->chyron);
    wattron(form->win, WA_REVERSE);
    mvwaddstr(form->win, form->lines - 1, 0, form->chyron->s);
    wattroff(form->win, WA_REVERSE);
    wclrtoeol(form->win);
    wmove(form->win, form->lines - 1, form->chyron->l);
}
/** @brief Parse the form description file to populate the Form data structure
   with field definitions, text elements, and other configuration specified in
   the description file.
    @ingroup form_engine
    @param form A pointer to the Form structure containing form data and state.
    @return 0 on success, or a non-zero value if an error occurs while parsing
   the description file (e.g., file not found, invalid format, missing
   directives). */
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
    if (form_desc_fp == nullptr) {
        ssnprintf(em0, MAXLEN - 1, "%s, line: %d", __FILE__, __LINE__ - 2);
        strnz__cpy(em1, "fopen ", MAXLEN - 1);
        strnz__cat(em1, form->mapp_spec, MAXLEN - 1);
        strerror_r(errno, em2, MAXLEN);
        display_error(em0, em1, em2, nullptr);
        return (1);
    }
    for (i = 0; i < FIELD_MAXCNT; i++) {
        form->field[i] = (Field *)calloc(1, sizeof(Field));
        if (!form->field[i]) {
            sprintf(tmp_str, "FORM: calloc failed for fields");
            abend(EXIT_FAILURE, tmp_str);
        }
    }
    for (i = 0; i < FIELD_MAXCNT; i++) {
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
    while ((fgets(in_buf, MAXLEN, form_desc_fp)) != nullptr) {
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
        case D_CALC:
            form->f_calculate = true;
            break;
        case D_QUERY:
            form->f_query = true;
            break;
        case D_CMD:
            if (!(token = strtok(nullptr, delim))) {
                form_desc_error(in_line_num, in_buf,
                                "FORM: receiver_cmd delimiter");
                continue;
            }
            strnz__cpy(form->receiver_cmd, token, MAXLEN - 1);
            break;
        case D_HELP:
            if (!(token = strtok(nullptr, delim))) {
                form_desc_error(in_line_num, in_buf,
                                "FORM: help_spec delimiter");
            }
            strnz__cpy(form->help_spec, token, MAXLEN - 1);
            break;
        case D_FIELD:
            if (form->field[form->fidx] == nullptr) {
                sprintf(tmp_str, "FORM: calloc failed for fields");
                abend(EXIT_FAILURE, tmp_str);
            }
            if (!(token = strtok(nullptr, delim))) {
                form_desc_error(in_line_num, in_buf,
                                "FORM: line number delimiter");
                return 1;
            }
            form->field[form->fidx]->line = atoi(token);
            if (form->field[form->fidx]->line < 0 ||
                form->field[form->fidx]->line >= FIELD_MAXCNT) {
                form_desc_error(in_line_num, in_buf,
                                "FORM: invalid line number");
                return 1;
            }
            if (!(token = strtok(nullptr, delim))) {
                form_desc_error(in_line_num, in_buf,
                                "FORM: column number delimiter");
                return 1;
            }
            form->field[form->fidx]->col = atoi(token);
            if (form->field[form->fidx]->col < 0 ||
                form->field[form->fidx]->col >= FIELD_MAXLEN) {
                form_desc_error(in_line_num, in_buf,
                                "FORM: invalid column number");
                break;
            }
            if (!(token = strtok(nullptr, delim))) {
                strnz__cpy(tmp_str, in_buf, MAXLEN - 1);
                form_desc_error(in_line_num, tmp_str, "FORM: length delimiter");
                break;
            }
            form->field[form->fidx]->len = atoi(token);
            if (form->field[form->fidx]->len < 0 ||
                form->field[form->fidx]->len > FIELD_MAXLEN) {
                form_desc_error(in_line_num, in_buf, "FORM: invalid length");
                break;
            }
            if (!(token = strtok(nullptr, delim))) {
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
            cols =
                form->field[form->fidx]->col + form->field[form->fidx]->len + 1;
            if (cols > form->cols)
                form->cols = cols;
            form->fidx++;
            form->fcnt = form->fidx;
            break;
        case D_TEXT:
            if (form->text[form->didx] == nullptr) {
                sprintf(tmp_str, "FORM: calloc failed for text");
                abend(EXIT_FAILURE, tmp_str);
            }
            if (!(token = strtok(nullptr, delim))) {
                form_desc_error(in_line_num, in_buf,
                                "FORM: line number delimiter");
                break;
            }
            form->text[form->didx]->line = atoi(token);
            if (form->text[form->didx]->line < 0 ||
                form->text[form->didx]->line >= FIELD_MAXCNT) {
                form_desc_error(in_line_num, in_buf,
                                "FORM: invalid line number");
                break;
            }
            if (!(token = strtok(nullptr, delim))) {
                form_desc_error(in_line_num, in_buf,
                                "FORM: column number delimiter");
                break;
            }
            form->text[form->didx]->col = atoi(token);
            if (form->text[form->didx]->col < 0 ||
                form->text[form->didx]->col >= FIELD_MAXLEN) {
                form_desc_error(in_line_num, in_buf,
                                "FORM: invalid column number");
                break;
            }
            if (!(token = strtok(nullptr, delim))) {
                form_desc_error(in_line_num, in_buf, "FORM: text delimiter");
                break;
            }
            strnz__cpy(form->text[form->didx]->str, token, MAXLEN - 1);
            form->text[form->didx]->len = strlen(form->text[form->didx]->str);
            if (form->text[form->didx]->len < 0 ||
                form->text[form->didx]->len > FIELD_MAXLEN) {
                form_desc_error(in_line_num, in_buf, "FORM: invalid length");
                break;
            }
            cols =
                form->text[form->didx]->col + form->text[form->didx]->len + 1;
            if (cols > form->cols)
                form->cols = cols;
            form->didx++;
            form->dcnt = form->didx;
            break;
        case D_HEADER:
            if ((token = strtok(nullptr, delim))) {
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
        ssnprintf(em0, MAXLEN - 1, "%s, line: %d", __FILE__, __LINE__);
        ssnprintf(em1, MAXLEN - 1, "%s", "Error in description file:");
        ssnprintf(em2, MAXLEN - 1, "%s", form->mapp_spec);
        display_error(em0, em1, em2, nullptr);
        return (1);
    }
    return (0);
}
/** @brief Read initial data for form fields from a specified input source, such
    as a file or standard input, and populate the form fields with the data.
    @ingroup form_engine
    @param form A pointer to the Form structure containing form data and state.
    @return 0 on success, or a non-zero value if an error occurs while reading
    the data or if the specified input source is invalid or empty. */
int form_read_data(Form *form) {
    struct stat sb;
    char in_buf[MAXLEN];
    char field[MAXLEN];

    if (!form->f_in_pipe) {
        if (form->f_in_spec && form->in_spec[0] != '\0') {
            if ((lstat(form->in_spec, &sb) == -1) || (sb.st_size == 0) ||
                ((form->in_fp = fopen(form->in_spec, "rb")) == nullptr)) {
                strnz__cat(em0, form->in_spec, MAXLEN - 1);
                // if (errno)
                // strerror_r(errno, em1, MAXLEN - 1);
                // else
                if (sb.st_size == 0)
                    strnz__cpy(em1, "File is empty", MAXLEN - 1);
                else
                    strnz__cpy(em1, "File does not exist", MAXLEN - 1);
                strnz__cpy(em2, "Fields will be blank or zero", MAXLEN - 1);
                cmd_key = display_error(em0, em1, em2, nullptr);
                if (cmd_key == KEY_F(9))
                    return (1);
            }
            if (form->in_fp == nullptr)
                return (1);
        } else
            return (0);
    }
    form->fidx = 0;
    if (form->in_fp != nullptr) {
        while ((fgets(in_buf, MAXLEN, form->in_fp)) != nullptr) {
            if (form->fidx < FIELD_MAXCNT)
                strnz__cpy(field, in_buf, MAXLEN - 1);
            form_fmt_field(form, field);
            form->fidx++;
        }
        fclose(form->in_fp);
    }
    return (0);
}
/** @brief Execute a provider command specified in the form description file,
    passing form field values as arguments, and optionally redirecting output to
    a file.
    @ingroup form_engine
    @param form A pointer to the Form structure containing form data and state.
    @return 0 on success, or a non-zero value if an error occurs while
    constructing or executing the command. */
int form_exec_cmd(Form *form) {
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
/** @brief Write form field values to a specified output destination, such as a
    file or standard output, based on the form configuration and user input.
    @ingroup form_engine
    @param form A pointer to the Form structure containing form data and state.
    @return 0 on success, or a non-zero value if an error occurs while writing
    the data or if the specified output destination is invalid. */
int form_write(Form *form) {
    int n;
    if (form->out_spec[0] == '\0' || strcmp(form->out_spec, "-") == 0 ||
        strcmp(form->out_spec, "/dev/stdout") == 0) {
        strnz__cpy(form->out_spec, "/dev/stdout", MAXLEN - 1);
        close(form->out_fd);
        form->out_fd = open(form->out_spec, O_CREAT | O_RDWR | O_TRUNC, 0644);
        if (form->out_fd == -1) {
            ssnprintf(em0, MAXLEN - 1, "%s, line: %d", __FILE__, __LINE__ - 1);
            strnz__cpy(em1, "open ", MAXLEN - 1);
            strnz__cat(em1, form->out_spec, MAXLEN - 1);
            strerror_r(errno, em2, MAXLEN);
            display_error(em0, em1, em2, nullptr);
            return (1);
        }
        dup2(form->out_fd, STDOUT_FILENO);
        form->out_fp = fdopen(STDOUT_FILENO, "w");
        form->f_out_spec = true;
        form->f_out_pipe = true;
    } else {
        if ((form->out_fp = fopen(form->out_spec, "w")) == nullptr) {
            ssnprintf(em0, MAXLEN - 1, "%s, line: %d", __FILE__, __LINE__ - 1);
            strerror_r(errno, em2, MAXLEN);
            display_error(em0, em1, em2, nullptr);
            return (1);
        }
    }
    if (form->out_fp == nullptr) {
        ssnprintf(em0, MAXLEN - 1, "%s, line: %d", __FILE__, __LINE__ - 1);
        strnz__cpy(em1, "fopen ", MAXLEN - 1);
        strnz__cat(em1, form->out_spec, MAXLEN - 1);
        strerror_r(errno, em2, MAXLEN);
        display_error(em0, em1, em2, nullptr);
        return (1);
    }
    for (n = 0; n < form->fcnt; n++)
        fprintf(form->out_fp, "%s\n", form->field[n]->accept_s);
    if (form->out_fp != nullptr)
        fclose(form->out_fp);
    return (0);
}
/** @brief Display usage information for the form, including available options
   and commands, to assist users in understanding how to interact with the form.
    @ingroup form_engine
    @note This function generates a usage message based on the options defined
   for the form and displays it to the user, typically when they request help or
   when an error occurs. The usage information includes details about the form's
   configuration, available commands, and how to navigate and interact with the
   form fields. */
void form_usage() {
    dump_opts_by_use("Form: usage: ", "..f.");
    (void)fprintf(stderr, "\n");
    Perror("press any key to continue");
}
/** @brief Handle errors encountered while parsing the form description file,
   providing detailed error messages that include the file name, line number,
   and the specific error encountered.
   @ingroup form_engine
    @param in_line_num The line number in the description file where the error
   occurred.
    @param in_buf The content of the line that caused the error, for context.
    @param em A specific error message describing the nature of the error.
    @return An integer status code indicating how the user responded to the
   error message (e.g., which key they pressed to acknowledge the error). */
int form_desc_error(int in_line_num, char *in_buf, char *em) {
    int cmd_key;

    ssnprintf(em0, MAXLEN - 1, "%s: %s", __FILE__, em);
    ssnprintf(em1, MAXLEN - 1, "Desc file: %s, line: %d", form->mapp_spec,
              in_line_num);
    strnz__cpy(em2, in_buf, MAXLEN - 1);
    cmd_key = display_error(em0, em1, em2, nullptr);
    return cmd_key;
}
