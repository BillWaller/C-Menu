/** @file pick_engine.c
    @brief pick from a list of choices
    @author Bill Waller
    Copyright (c) 2025
    MIT License
    billxwaller@gmail.com
    @date 2026-02-09
 */

/** @defgroup pick_engine Object Selection
    @brief Functions to Navigate, Select, and Perform Action on Objects
 */

#include <common.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

int tbl_col, tbl_line, tbl_page, tbl_cols, pg_lines, tbl_pages;
int obj_idx, calculated_idx;
int pick_engine(Init *);
void save_object(Pick *, char *);
int picker(Init *);
void display_page(Pick *);
void reverse_object(Pick *);
void unreverse_object(Pick *);
void toggle_object(Pick *);
int output_objects(Pick *);
int exec_objects(Init *);
int open_pick_win(Init *);
void display_pick_help(Init *);
int read_pick_input(Init *);
void deselect_object(Pick *);

int pipe_fd[2];

char const pagers_editors[12][10] = {"view", "view",  "less", "more",
                                     "vi",   "vim",   "nano", "nvim",
                                     "pico", "emacs", "edit", ""};

/** @brief Initializes pick structure and opens pick input file or pipe
 *  @ingroup pick_engine
    @param init Pointer to Init structure
    @param argc Argument count
    @param argv Argument vector
    @param begy Beginning y coordinate for pick window
    @param begx Beginning x coordinate for pick window
    @note If provider_cmd is specified, it takes precedence over in_spec and
    input file arguments
    @note provider_cmd is executed and its output is read as pick input
    @note If provider_cmd is not specified, in_spec is used to read pick
    input from a file or stdin
    @note If provider_cmd is specified, it is executed and its output is
    read as pick input */
int init_pick(Init *init, int argc, char **argv, int begy, int begx) {
    struct stat sb;
    char *s_argv[MAXARGS];
    int s_argc;
    char tmp_str[MAXLEN];
    int m;
    pid_t pid = 0;

    // if (init->pick != nullptr)
    // destroy_pick(init);
    Pick *pick = new_pick(init, argc, argv, begy, begx);
    if (init->pick != pick)
        abend(-1, "init->pick != pick\n");
    SIO *sio = init->sio;
    if (pick->provider_cmd[0] != '\0') {
        s_argc = str_to_args(s_argv, pick->provider_cmd, MAXARGS - 1);
        if (pipe(pipe_fd) == -1) {
            Perror("pipe(pipe_fd) failed in init_pick");
            return (1);
        }
        if ((pid = fork()) == -1) {
            Perror("fork() failed in init_pick");
            return (1);
        }
        if (pid == 0) {
            /** Spawn Child to execute provider_cmd
                Close read end of pipe as Child only needs to write to pipe */
            close(pipe_fd[P_READ]);
            /** Connect CHILD STDOUT to write end of pipe */
            dup2(pipe_fd[P_WRITE], STDOUT_FILENO);
            dup2(pipe_fd[P_WRITE], STDERR_FILENO);
            /** STDOUT attached to write end of pipe, so close pipe fd */
            close(pipe_fd[P_WRITE]);
            execvp(s_argv[0], s_argv);
            m = MAXLEN - 24;
            strnz__cpy(tmp_str, "Can't exec pick start cmd: ", m);
            m -= strlen(s_argv[0]);
            strnz__cat(tmp_str, s_argv[0], m);
            Perror(tmp_str);
            exit(EXIT_FAILURE);
        }
        /** Return to Parent
            Close write end of pipe as Parent only needs to read from pipe */
        close(pipe_fd[P_WRITE]);
        /** Open a file pointer on read end of pipe */
        pick->in_fp = fdopen(pipe_fd[P_READ], "rb");
        pick->f_in_pipe = true;
        s_argc = destroy_argv(s_argc, s_argv);
    } else {
        if ((pick->in_spec[0] == '\0') || strcmp(pick->in_spec, "-") == 0 ||
            strcmp(pick->in_spec, "/dev/stdin") == 0) {
            strnz__cpy(pick->in_spec, "/dev/stdin", MAXLEN - 1);
            pick->in_fp = fdopen(STDIN_FILENO, "rb");
            pick->f_in_pipe = true;
        }
    }
    if (!pick->f_in_pipe) {
        /** No provider_cmd specified, so read pick input from file or stdin */
        if (lstat(pick->in_spec, &sb) == -1) {
            m = MAXLEN - 29;
            strnz__cpy(tmp_str, "Can\'t stat pick input file: ", m);
            m -= strlen(pick->in_spec);
            strnz__cat(tmp_str, pick->in_spec, m);
            Perror(tmp_str);
            return (1);
        }
        if (sb.st_size == 0) {
            m = MAXLEN - 24;
            strnz__cpy(tmp_str, "Pick input file empty: ", m);
            m -= strlen(pick->in_spec);
            strnz__cat(tmp_str, pick->in_spec, m);
            Perror(tmp_str);
            return (1);
        }
        if ((pick->in_fp = fopen(pick->in_spec, "rb")) == nullptr) {
            m = MAXLEN - 29;
            strnz__cpy(tmp_str, "Can't open pick input file: ", m);
            m -= strlen(pick->in_spec);
            strnz__cat(tmp_str, pick->in_spec, m);
            Perror(tmp_str);
            return (1);
        }
    }
    /*------------------------------------------------------------*/
    bool f_wait = false;
    int ready;
    fd_set read_fds;
    struct timeval timeout;
    Chyron *wait_chyron;
    WINDOW *wait_win;
    int remaining;
    if (pick->in_fp == nullptr) {
        Perror("No pick input available");
        // destroy_pick(init);
        return (1);
    }
    int in_fd = fileno(pick->in_fp);
    FD_ZERO(&read_fds);
    FD_SET(in_fd, &read_fds);
    timeout.tv_sec = 0;
    timeout.tv_usec =
        200000; /**< Initial timeout of 200ms to check for pick input */
    ready = select(in_fd + 1, &read_fds, nullptr, nullptr, &timeout);
    if (ready == 0) {
        f_wait = true;
        remaining = wait_timeout;
        wait_chyron = wait_mk_chyron();
        wait_win = wait_mk_win(wait_chyron, "WAITING for PICK INPUT");
    }
    int in_key = 0;
    while (ready == 0 && remaining > 0 && in_key != KEY_F(9)) {
        in_key = wait_continue(wait_win, wait_chyron, remaining);
        if (in_key == KEY_F(9))
            break;
        FD_ZERO(&read_fds);
        FD_SET(in_fd, &read_fds);
        timeout.tv_sec = 0;
        timeout.tv_usec = 0;
        ready = select(in_fd + 1, &read_fds, nullptr, nullptr, &timeout);
        remaining--;
    }
    if (f_wait) {
        if (wait_chyron != nullptr)
            wait_destroy(wait_chyron);
    }
    if (in_key == KEY_F(9)) {
        if (pick->f_in_pipe && pid > 0) {
            /** If user cancels while waiting for pick input, kill provider_cmd
             * child process and close pipe */
            kill(pid, SIGKILL);
            waitpid(pid, nullptr, 0);
            close(pipe_fd[P_READ]);
        }
        Perror("No pick input available");
        // destroy_pick(init);
        return (1);
    }
    if (ready == -1) {
        Perror("Error waiting for pick input");
        if (pick->f_in_pipe && pid > 0) {
            /** If error occurs while waiting for pick input, kill provider_cmd
             * child process and close pipe */
            kill(pid, SIGKILL);
            waitpid(pid, nullptr, 0);
            close(pipe_fd[P_READ]);
        }
        // destroy_pick(init);
        return (1);
    }
    if (ready == 0) {
        Perror("Timeout waiting for pick input");
        if (pick->f_in_pipe && pid > 0) {
            /** If timeout occurs while waiting for pick input, kill
             * provider_cmd child process and close pipe */
            kill(pid, SIGKILL);
            waitpid(pid, nullptr, 0);
            close(pipe_fd[P_READ]);
        }
        // destroy_pick(init);
        return (1);
    }
    if (ready == 1 && !FD_ISSET(in_fd, &read_fds)) {
        Perror("Unexpected error waiting for pick input");
        if (pick->f_in_pipe && pid > 0) {
            /** If unexpected error occurs while waiting for pick input, kill
             * provider_cmd child process and close pipe */
            kill(pid, SIGKILL);
            waitpid(pid, nullptr, 0);
            close(pipe_fd[P_READ]);
        }
        // destroy_pick(init);
        return (1);
    }
    /*------------------------------------------------------------*/
    read_pick_input(init);
    if (pick->f_in_pipe && pid > 0) {
        /** Wait for provider_cmd child process to finish before proceeding */
        waitpid_with_timeout(pid, wait_timeout);
        // waitpid(pid, nullptr, 0);
        close(pipe_fd[P_READ]);
        dup2(sio->stdin_fd, STDIN_FILENO);
        dup2(sio->stdout_fd, STDOUT_FILENO);
        restore_curses_tioctl();
        sig_prog_mode();
        keypad(pick->win, true);
    }
    if (pick->m_cnt == 0) {
        Perror("No pick objects available");
        // destroy_pick(init);
        return (1);
    }
    /** Enter pick_engine */
    pick->m_idx = 0;
    pick->d_idx = 0;
    while (pick->m_idx < pick->m_cnt)
        pick->d_object[pick->d_idx++] = pick->m_object[pick->m_idx++];
    pick->d_cnt = pick->d_idx;
    pick_engine(init);
    win_del();
    // destroy_pick(init);
    return 0;
}
/** @brief Reads pick input from file pointer and saves objects into pick
 structure
 *  @ingroup pick_engine
    @param init Pointer to Init structure containing pick information
    @return 0 on success, -1 if no objects were read
    @note Reads lines from pick->in_fp and saves them as objects in the pick
   structure using save_object function. If no objects are read, returns -1.
   Otherwise, sets obj_cnt to the number of objects read and resets obj_idx to 0
   before returning 0. */
int read_pick_input(Init *init) {
    int i;

    Pick *pick = init->pick;
    pick->select_cnt = 0;
    pick->m_cnt = pick->pg_lines = pick->tbl_cols = 0;
    pick->m_idx = pick->tbl_page = pick->y = pick->tbl_col = pick->x = 0;
    pick->tbl_pages = 1;

    if (pick->in_fp) {
        while (fgets(pick->in_buf, sizeof(pick->in_buf), pick->in_fp) !=
               nullptr)
            save_object(pick, pick->in_buf);
    } else
        for (i = 1; i < pick->argc; i++)
            save_object(pick, pick->argv[i]);
    if (pick->in_fp != nullptr)
        fclose(pick->in_fp);
    if (!pick->m_idx)
        return (-1);
    pick->m_cnt = pick->m_idx;
    pick->m_idx = 0;
    return 0;
}
/** @brief Initializes pick interface, calculates window size and position, and
   enters picker loop
 *  @ingroup pick_engine
    @param init Pointer to Init structure containing pick information
    @return Count of selected objects on success, -1 if user cancels
    @note Initializes key command strings for chyron display and calculates pick
   window size and position based on terminal size and pick parameters.
    @note Opens pick window and displays first page of objects. Enters picker
   loop to handle user input and interactions. If user cancels selection,
   returns -1.
    @note If user accepts selection, returns count of selected objects. */
int pick_engine(Init *init) {
    int rc;
    int maxy, maxx, win_maxy, win_maxx;
    int tbl_max_cols, pg_max_objs;

    getmaxyx(stdscr, maxy, maxx);
    /** Calculate pick window size and position based on terminal size and pick
        parameters */
    win_maxy = min(((maxy * 8) / 10), (maxy - pick->begy - 1));
    win_maxx = min(((maxx * 9) / 10), (maxx - pick->begx - 1));
    pick->tbl_col_width = max(pick->tbl_col_width, 4);
    pick->tbl_col_width = min(pick->tbl_col_width, win_maxx - 2);
    if (pick->d_cnt <= win_maxy) {
        pick->tbl_lines = pick->d_cnt;
        pick->tbl_cols = 1;
    } else {
        tbl_max_cols = (win_maxx / (pick->tbl_col_width + 1));
        pg_max_objs = win_maxy * tbl_max_cols;
        if (pick->d_cnt > pg_max_objs)
            pick->tbl_cols = tbl_max_cols;
        else
            pick->tbl_cols = pick->d_cnt / win_maxy;
        pick->tbl_lines = pick->d_cnt / tbl_max_cols;
    }
    pick->tbl_pages = (pick->tbl_lines / (win_maxy - 1)) + 1;
    pick->pg_lines = (pick->tbl_lines / pick->tbl_pages);
    pick->win_lines = pick->pg_lines + 1;
    pick->tbl_page = 0;
    if (pick->begy == 0)
        pick->begy = (LINES - pick->win_lines) / 5;
    else if (pick->begy + pick->win_lines > LINES - 4)
        pick->begy = LINES - pick->win_lines - 2;
    pick->win_width = (pick->tbl_col_width + 1) * pick->tbl_cols;

    pick->chyron = new_chyron();
    set_chyron_key(pick->chyron, 1, "F1 Help", KEY_F(1));
    set_chyron_key(pick->chyron, 2, "F9 Cancel", KEY_F(9));
    set_chyron_key(pick->chyron, 3, "F10 Continue", KEY_F(10));
    set_chyron_key(pick->chyron, 4, "Sp Toggle", 't');
    if (pick->tbl_pages > 1) {
        set_chyron_key(pick->chyron, 11, "PgUp", KEY_PPAGE);
        set_chyron_key(pick->chyron, 12, "PgDn", KEY_NPAGE);
    }
    compile_chyron(pick->chyron);
    if (pick->chyron->l > win_maxx)
        pick->chyron->l = strnz(pick->chyron->s, win_maxx);
    if (pick->win_width < pick->chyron->l)
        pick->win_width = pick->chyron->l;
    pick->win_width = max(pick->win_width, pick->chyron->l) + 1;
    if (pick->begx + pick->win_width > COLS - 2)
        pick->begx = COLS - pick->win_width - 2;
    else if (pick->begx == 0)
        pick->begx = (COLS - pick->win_width) / 2;

    rc = open_pick_win(init);
    if (rc)
        return (rc);

    /** Enter picker loop to handle user input and interactions */
    pick->d_idx = 0;
    pick->x = 1;
    do {
        rc = picker(init);
        if (rc == KEY_F(9))
            break;
        if (rc == -1)
            break;
        else {
            if (pick->select_cnt > 0) {
                if (pick->f_out_spec && pick->out_spec[0])
                    output_objects(pick);
                if (pick->f_cmd && pick->cmd[0])
                    exec_objects(init);
            }
        }
        deselect_object(pick);
    } while (1);
    destroy_chyron(pick->chyron);
    return (rc);
}
/** @brief Saves a string as an object in the pick structure
 *  @ingroup pick_engine
    @param pick Pointer to Pick structure
    @param s String to save as an object
    @note If the current object index is less than the maximum allowed, saves
   the string as an object in the pick structure. Updates the column width if
   necessary and marks the object as not selected. Increments the object index
   for the next object to be saved. */
void save_object(Pick *pick, char *s) {
    int l;

    if (pick->m_idx < OBJ_MAXCNT - 1) {
        l = strlen(s);
        if (l > OBJ_MAXLEN - 1)
            s[OBJ_MAXLEN - 1] = '\0';
        pick->tbl_col_width = max(pick->tbl_col_width, l);
        l = max(l, 1);
        pick->m_object[pick->m_idx] = (char *)calloc(l + 1, sizeof(char));
        strnz__cpy(pick->m_object[pick->m_idx], s, l);
        pick->f_selected[pick->m_idx] = false;
        pick->m_idx++;
    }
}

/** @brief Displays current page of objects in pick window
 *  @ingroup pick_engine
    @param pick Pointer to Pick structure containing objects and display
   information
    @note Clears the pick window and displays the current page of objects based
   on the current table page, line, and column. Marks selected objects with an
   asterisk. Updates the chyron with page information at the bottom of the pick
   window. */
void display_page(Pick *pick) {
    int col;
    for (pick->y = 0; pick->y < pick->pg_lines; pick->y++) {
        wmove(pick->win, pick->y, 0);
        wclrtoeol(pick->win);
    }
    pick->d_idx = pick->tbl_page * pick->pg_lines * pick->tbl_cols;
    for (col = 0; col < pick->tbl_cols; col++) {
        pick->x = col * (pick->tbl_col_width + 1) + 1;
        pick->y = 0;
        while (pick->d_idx < pick->d_cnt && pick->y < pick->pg_lines) {
            if (pick->f_selected[pick->d_idx])
                mvwaddstr(pick->win, pick->y, pick->x - 1, "*");
            mvwaddstr_fill(pick->win, pick->y++, pick->x,
                           pick->d_object[pick->d_idx++], pick->tbl_col_width);
        }
    }
    if (pick->y < pick->pg_lines) {
        pick->y_offset = pick->pg_lines - pick->y;
        wscrl(pick->win, -pick->y_offset);
    }
    // if (pick->tbl_pages > 1) {
    //     char page_info[20];
    //     snprintf(page_info, sizeof(page_info), "Page %d/%d", pick->tbl_page +
    //     1,
    //              pick->tbl_pages);
    //     display_chyron(pick->win2, pick->chyron, 11, page_info);
    // }
    // Add page information to chyron later, after we add a function to
    // accomodate it
    wrefresh(pick->win);
}
/** @brief Displays current page of objects in pick window
    @ingroup pick_engine
    @param pick Pointer to Pick structure containing objects and display
   information
    @param s String to filter objects by
    @note Clears the pick window and displays the current page of objects based
   on the current table page, line, and column. Marks selected objects with an
   asterisk. Updates the chyron with page information at the bottom of the pick
   window. */
int match_objects(Pick *pick, char *s) {
    pick->m_idx = 0;
    pick->d_idx = 0;
    while (pick->m_idx < pick->m_cnt) {
        if (s == nullptr || s[0] == '\0' ||
            strstr(pick->m_object[pick->m_idx], s) != nullptr) {
            pick->d_object[pick->d_idx++] = pick->m_object[pick->m_idx];
        }
        pick->m_idx++;
    }
    pick->d_cnt = pick->d_idx;
    return pick->d_cnt;
}
/** @brief Reverses the display of the currently selected object in pick window
 *  @ingroup pick_engine
    @param pick Pointer to Pick structure containing object and display
   information
    @note Calculates the x coordinate for the currently selected object based on
   the current table column and column width. Moves the cursor to the object's
   position in the pick window, turns on reverse video attribute, and displays
   the object's text. Turns off reverse video attribute and refreshes the pick
   window to show the updated display. Moves the cursor back to the position
   before the object text for potential further interactions. */
void reverse_object(Pick *pick) {
    if (pick->d_idx >= pick->d_cnt)
        pick->d_idx = pick->d_cnt - 1;
    pick->x = pick->tbl_col * (pick->tbl_col_width + 1) + 1;

    pick->tbl_line = (pick->d_idx / pick->tbl_cols) % pick->pg_lines;
    pick->y = pick->tbl_line + pick->y_offset;
    pick->d_idx = pick->tbl_page * pick->pg_lines * pick->tbl_cols +
                  pick->tbl_col * pick->pg_lines + pick->tbl_line;

    wmove(pick->win, pick->y, pick->x);
    wattron(pick->win, WA_REVERSE);
    mvwaddstr_fill(pick->win, pick->y, pick->x, pick->d_object[pick->d_idx],
                   pick->tbl_col_width);
    wattroff(pick->win, WA_REVERSE);
    wmove(pick->win, pick->y, pick->x - 1);
}
/** @brief Unreverses the display of the currently selected object in pick
   window
   @ingroup pick_engine
    @param pick Pointer to Pick structure containing object and display
   information
    @note Calculates the x coordinate for the currently selected object based on
   the current table column and column width. Moves the cursor to the object's
   position in the pick window and displays the object's text without reverse
   video attribute. Refreshes the pick window to show the updated display. Moves
   the cursor back to the position before the object text for potential further
   interactions. */
void unreverse_object(Pick *pick) {
    if (pick->d_idx >= pick->d_cnt)
        pick->d_idx = pick->d_cnt - 1;
    pick->x = pick->tbl_col * (pick->tbl_col_width + 1) + 1;

    pick->tbl_line = (pick->d_idx / pick->tbl_cols) % pick->pg_lines;
    pick->y = pick->tbl_line + pick->y_offset;
    pick->d_idx = pick->tbl_page * pick->pg_lines * pick->tbl_cols +
                  pick->tbl_col * pick->pg_lines + pick->tbl_line;

    wmove(pick->win, pick->y, pick->x);
    mvwaddstr_fill(pick->win, pick->y, pick->x, pick->d_object[pick->d_idx],
                   pick->tbl_col_width);
    wmove(pick->win, pick->y, pick->x - 1);
}
/** @brief Toggles the selection state of the currently selected object in pick
   window
   @ingroup pick_engine
    @param pick Pointer to Pick structure containing object and selection
   information
    @note Calculates the x coordinate for the currently selected object based on
   the current table column and column width. If the object is currently
   selected, it is deselected by updating the selection count, marking it as not
   selected, and displaying a space before the object text. If the object is not
   currently selected, it is selected by updating the selection count, marking
   it as selected, and displaying an asterisk before the object text. Refreshes
   the pick window to show the updated display. Moves the cursor back to the
   position before the object text for potential further interactions. */
void toggle_object(Pick *pick) {
    pick->x = pick->tbl_col * (pick->tbl_col_width + 1) + 1;
    if (pick->f_selected[pick->d_idx]) {
        pick->select_cnt--;
        pick->f_selected[pick->d_idx] = false;
        mvwaddstr(pick->win, pick->y, pick->x - 1, " ");
    } else {
        pick->select_cnt++;
        pick->f_selected[pick->d_idx] = true;
        mvwaddstr(pick->win, pick->y, pick->x - 1, "*");
    }
}
/** @brief Deselects the currently selected object in pick window
   @ingroup pick_engine
    @details like toggle, but only deselects object */
void deselect_object(Pick *pick) {
    pick->x = pick->tbl_col * (pick->tbl_col_width + 1) + 1;
    if (pick->f_selected[pick->d_idx]) {
        pick->select_cnt--;
        pick->f_selected[pick->d_idx] = false;
        mvwaddstr(pick->win, pick->y, pick->x - 1, " ");
    }
}
/** @brief Outputs selected objects to specified output file
   @ingroup pick_engine
    @param pick Pointer to Pick structure containing selected objects and
   output file information
    @return 0 on success, 1 on failure
    @note If output file cannot be opened, an error message is printed and
   the function returns 1. Otherwise, selected objects are written to the
   output file, one per line, and the file is closed before returning 0.
*/
int output_objects(Pick *pick) {
    char tmp_str[MAXLEN];
    int m;
    if ((pick->out_fp = fopen(pick->out_spec, "w")) == nullptr) {
        m = MAXLEN - 30;
        strnz__cpy(tmp_str, "Can't open pick output file: ", m);
        m -= strlen(pick->in_spec);
        strnz__cat(tmp_str, pick->out_spec, m);
    }
    for (pick->d_idx = 0; pick->d_idx < pick->d_cnt; pick->d_idx++) {
        if (pick->f_selected[pick->d_idx])
            fprintf(stdout, "%s\n", pick->d_object[pick->d_idx]);
    }
    fflush(stdout);
    if (pick->out_fp != nullptr)
        fclose(pick->out_fp);
    return (0);
}
/** @brief Executes specified command with selected objects as arguments
   @ingroup pick_engine
   @param init Pointer to Init structure
   @return 0 on success, 1 on failure
   @note Parses command string and appends selected objects as arguments to the
   command. If command contains "%%", it is replaced with a space- separated
   list of selected objects. Executes the command using execvp in a child
   process and waits for it to finish. If the command is a pager or editor, it
   is executed within the pick interface using popup_view instead of execvp.
   @note If f_append_args is true, the argument containing %% is replaced with
   the concatenated selected objects. If f_append_args is false, selected
   objects are added as separate arguments and the original command arguments
   remain unchanged.
   @note eargv should be null-terminated to indicate the end of arguments for
   execvp
   @note Memory allocated for arguments is freed after execution to prevent
   memory leaks.
   @note If execvp fails, an error message is printed and the child process
   exits with failure status
   @note The parent process waits for the child process to finish before
   proceeding and restores terminal settings
   @note If the command is a pager or editor, it is executed within the pick
   interface using popup_view instead of execvp
   @note The base name of the command is extracted to check if it is a pager or
   editor
   @note If the command is a pager or editor, the pick interface is used to
   display the command output instead of executing it in a separate terminal
   This allows the user to view the command output without leaving the pick
   interface and provides a more seamless user experience.
   @note If the command is not a pager or editor, it is executed in a separate
   terminal and the pick interface is restored after execution
   @note If the command to be executed is view, an external command is not
   needed, instead the popup_view function can be used to display the output
   within the pick interface */
int exec_objects(Init *init) {
    int rc = -1;
    int eargc;
    char *eargv[MAXARGS];
    char tmp_str[MAXLEN];
    char title[MAXLEN];
    char sav_arg[MAXLEN];
    char *out_s;
    int eargx = 0;
    int i = 0;
    pid_t pid = 0;
    bool f_append_args = false;
    // char *s1;

    title[0] = '\0';
    if (pick->cmd[0] == '\0')
        return -1;
    if (pick->cmd[0] == '\\' || pick->cmd[0] == '\"') {
        size_t len = strlen(pick->cmd);
        if (len > 1 && pick->cmd[len - 1] == '\"') {
            memmove(pick->cmd, pick->cmd + 1, len - 2);
            pick->cmd[len - 2] = '\0';
        }
    }
    eargc = str_to_args(eargv, pick->cmd, MAXARGS - 1);
    tmp_str[0] = '\0';
    if (pick->f_multiple_cmd_args) {
        for (i = 0; i < pick->d_cnt; i++) {
            if (pick->f_selected[i] && eargc < MAXARGS) {
                if (tmp_str[0] != '\0')
                    strnz__cat(tmp_str, " ", MAXLEN - 1);
                strnz__cat(tmp_str, pick->d_object[i], MAXLEN - 1);
            }
        }
        eargv[eargc++] = strdup(tmp_str);
    } else {
        f_append_args = false;
        i = 0;
        while (i < eargc) {
            /** This is the line that gets the selected objects */
            if (strstr(eargv[i], "%%") != nullptr) {
                tmp_str[0] = '\0';
                f_append_args = true;
                strnz__cpy(sav_arg, eargv[i], MAXLEN - 1);
                eargx = i;
                break;
            }
            i++;
        }
        for (i = 0; i < pick->d_cnt; i++) {
            /** append arguments onto tmp_str */
            if (pick->f_selected[i] && eargc < MAXARGS - 1) {
                if (f_append_args == true) {
                    if (tmp_str[0] != '\0')
                        strnz__cat(tmp_str, " ", MAXLEN - 1);
                    strnz__cat(tmp_str, pick->d_object[i], MAXLEN - 1);
                    continue;
                }
                eargv[eargc++] = strdup(pick->d_object[i]);
            }
        }
        if (f_append_args == true) {
            if (eargv[eargx] != nullptr) {
                free(eargv[eargx]);
                eargv[eargx] = nullptr;
            }
            out_s = rep_substring(sav_arg, "%%", tmp_str);
            if (out_s == nullptr || out_s[0] == '\0') {
                i = 0;
                while (i < eargc) {
                    if (eargv[i] != nullptr)
                        free(eargv[i]);
                    i++;
                }
                Perror("rep_substring() failed in exec_objects");
                return 1;
            }
            strnz__cpy(title, out_s, MAXLEN - 1);
            eargv[eargx] = strdup(out_s);
            if (out_s != nullptr) {
                free(out_s);
                out_s = nullptr;
            }
        }
    }
    strnz__cpy(tmp_str, eargv[0], MAXLEN - 1);
    eargv[eargc] = nullptr;
    // s1 = tmp_str;
    char *sp;
    // Scan-build false positive: strtok_r modifies the input string, but
    // tmp_str is a local array and is not used after this point, so it is safe
    // to modify it with strtok_r
    char *tok;
    tok = strtok_r(tmp_str, " ", &sp);
    strnz__cpy(sav_arg, tok, MAXLEN - 1);
    base_name(tmp_str, sav_arg);
    if (tmp_str[0] != '\0' &&
        (strcmp(tmp_str, "view") == 0 || strcmp(tmp_str, "view") == 0)) {
        /** initialize popup_view arguments and execute popup_view to display
           command output within pick interface */
        init->lines = 60;
        init->cols = 80;
        init->begy = pick->begy + 1;
        init->begx = pick->begx + 1;
        if (title[0] != '\0')
            strnz__cpy(init->title, title, MAXLEN - 1);
        else
            strnz__cpy(init->title, eargv[eargc], MAXLEN - 1);
        popup_view(init, eargc, eargv, init->lines, init->cols, init->begy,
                   init->begx);
        i = 0;
        while (i < eargc) {
            if (eargv[i] != nullptr)
                free(eargv[i]);
            i++;
        }
        return 0;
    } else {
        if ((pid = fork()) == -1) {
            /** fork failed, free eargv and return error */
            i = 0;
            while (i < eargc) {
                if (eargv[i] != nullptr)
                    free(eargv[i]);
                i++;
            }
            Perror("fork() failed in exec_objects");
            return (1);
        }
        if (pid == 0) {
            /** Child process to execute command */
            execvp(eargv[0], eargv);
            /** If execvp returns, it means execution failed, so free eargv
               and print error message before exiting */
            strnz__cpy(tmp_str, "Can't exec pick cmd: ", MAXLEN - 1);
            strnz__cat(tmp_str, eargv[0], MAXLEN - 1);
            Perror(tmp_str);
            exit(EXIT_FAILURE);
        }
    }
    // waitpid_with_timeout(pid, wait_timeout);
    // action_disposition("Notification", "Command executed");
    waitpid(pid, nullptr, 0);
    // win_del();
    eargc = destroy_argv(eargc, eargv);
    restore_curses_tioctl();
    sig_prog_mode();
    werase(stdscr);
    wrefresh(stdscr);
    restore_wins();
    return rc;
}
/** @brief Initializes the pick window based on the parameters specified in the
Pick structure
   @ingroup pick_engine
   @param init Pointer to Init structure containing pick information
   @return 0 on success, 1 on failure
   @note Creates a new window for the pick interface using win_new function
with the specified parameters from the Pick structure. If window creation fails,
an error message is printed and the function returns 1. Otherwise, initializes
the window and box pointers in the Pick structure, sets scrollok and keypad
options for the window, and returns 0 on success. */
int open_pick_win(Init *init) {
    char tmp_str[MAXLEN];
    pick = init->pick;
    if (box2_new(pick->win_lines, pick->win_width, pick->begy, pick->begx,
                 pick->title, true)) {
        ssnprintf(tmp_str, MAXLEN - 1, "box2_new(%d, %d, %d, %d, %s) failed",
                  pick->win_lines, pick->win_width, pick->begy, pick->begx,
                  pick->title);
        Perror(tmp_str);
        return (1);
    }
    pick->win = win_win[win_ptr];
    pick->win2 = win_win2[win_ptr];
    pick->box = win_box[win_ptr];
    keypad(pick->win, true);
    return 0;
}
/** @brief Displays the help screen for the pick interface using view
   @ingroup pick_engine
    @param init Pointer to Init structure containing pick information
    @note Initializes the help_spec field in the Pick structure with the
   path to the pick help file. Then, constructs the argument list for
   executing popup_view with the help file as an argument. Finally, calls
   popup_view function to display the help screen within the pick interface. */
void display_pick_help(Init *init) {
    int eargc;
    char *eargv[MAXARGS];
    char tmp_str[MAXLEN];
    if (pick->f_help_spec && pick->help_spec[0] != '\0')
        strnz__cpy(tmp_str, pick->help_spec, MAXLEN - 1);
    else {
        strnz__cpy(tmp_str, init->mapp_help, MAXLEN - 1);
        strnz__cat(tmp_str, "/", MAXLEN - 1);
        strnz__cat(tmp_str, PICK_HELP_FILE, MAXLEN - 1);
    }
    eargc = 0;
    eargv[eargc++] = strdup("view");
    eargv[eargc++] = strdup("-Nf");
    eargv[eargc++] = strdup(tmp_str);
    eargv[eargc] = NULL;
    init->lines = 30;
    init->cols = 76;
    init->begy = pick->begy + 1;
    init->begx = pick->begx + 1;
    strnz__cpy(init->title, "Pick Help", MAXLEN - 1);
    popup_view(init, eargc, eargv, init->lines, init->cols, init->begy,
               init->begx);
    eargc = destroy_argv(eargc, eargv);
    return;
}
/** @brief Main loop for handling user input and interactions in the pick
   interface
   @ingroup pick_engine
    @param init Pointer to Init structure containing pick information
    @return Count of selected objects on success, -1 if user cancels
    @note The first loop handles navigation through the pick table.
    The second loop handles user input for selecting/deselecting objects,
   accepting the selection, or canceling the selection. Depending on the key
   pressed, the appropriate action is taken, such as toggling selection, moving
   to the next/previous object, or displaying the help screen. If the user
   accepts the selection, the count of selected objects is returned. If the user
   cancels the selection, -1 is returned.
    */
int picker(Init *init) {
    bool f_insert = false; /**< Flag to indicate if insert mode is active */
    char field[MAXLEN];    /**< Buffer for user input in the field */
    char filler_s[MAXLEN]; /**< buffer for filling the field with spaces */
    int line = 0;          /**< Starting line for field input */
    int col = 1;    /**< Starting column for field input leaving space for > */
    int pos = col;  /** Current cursor position within the field */
    char *s;        /**< source pointer for editing operations */
    char *d;        /**< destination pointer for editing operations */
    char *e;        /**< end pointer for editing operations */
    char *accept_s; /**< pointer to field buffer */
    char *fstart;   /** pointer to start of field buffer */
    char *ptr;  /**< pointer to current cursor position within field buffer */
    char *fend; /**< pointer to end of field buffer (start + flen) */
    char *str_end; /**< pointer to end of content in field buffer */
    accept_s = field;
    fstart = accept_s;
    field[0] = '\0';
    int flen = pick->win_width - 4;

    pick = init->pick;
    win = pick->win;
    WINDOW *win2 = pick->win2;
    fend = fstart + flen;
    ptr = accept_s;
    str_end = fstart + strlen(fstart); /**< End of field content */
    click_x = -1;
    click_y = click_x = -1;

    mousemask(BUTTON1_CLICKED | BUTTON1_DOUBLE_CLICKED, nullptr);

    display_page(pick);
    f_insert = false;
    set_chyron_key_cp(pick->chyron, 18, "INS", KEY_IC, cp_reverse);
    compile_chyron(pick->chyron);
    display_chyron(pick->win2, pick->chyron, 1, pick->chyron->l);
    cchar_t cc = {0};
    wchar_t wstr[2] = {BW_RAN, L'\0'};
    setcchar(&cc, wstr, WA_NORMAL, cp_box, nullptr);
    mvwadd_wch(pick->win2, 0, 0, &cc);
    wrefresh(pick->win2);

    int in_key = 0;
    while (1) {
        while (1) {
            /** ===============================================================
                Pick Objects Loop
                ===============================================================
             */
            reverse_object(pick);
            tcflush(tty_fd, TCIFLUSH);
            pick->tbl_line = (pick->d_idx / pick->tbl_cols) % pick->pg_lines;
            pick->y = pick->tbl_line + pick->y_offset;
            if (in_key == 0)
                in_key = dxwgetch(pick->win, pick->win2, pick->chyron, -1);

            switch (in_key) {

            /** KEY_F(1) or 'H' Displays help screen for pick interface */
            case KEY_F(1):
                display_pick_help(init);
                display_page(pick);
                f_insert = false;
                set_chyron_key_cp(pick->chyron, 18, "INS", KEY_IC, cp_reverse);
                compile_chyron(pick->chyron);
                display_chyron(pick->win2, pick->chyron, 1, pick->chyron->l);
                cchar_t cc = {0};
                wchar_t wstr[2] = {BW_RAN, L'\0'};
                setcchar(&cc, wstr, WA_NORMAL, cp_box, nullptr);
                mvwadd_wch(pick->win2, 0, 0, &cc);
                wrefresh(pick->win2);
                in_key = 0;
                continue;

            case KEY_F(7):
            case 't':
            case ' ':
                reverse_object(pick);
                toggle_object(pick);
                if (pick->select_max > 0 &&
                    pick->select_cnt == pick->select_max)
                    return pick->select_cnt;
                in_key = 0;
                continue;

            /** 'q', or KEY_F(9) cancel selection and exit picker */
            case 'q':
            case KEY_F(9):
                deselect_object(pick);
                return -1;

            /** Enter or KEY_F(10) Accepts current selection and exits
             * picker, returning count of selected objects */
            case KEY_F(10):
            case '\n':
            case KEY_ENTER:
                return pick->select_cnt;
                /** KEY_END Moves selection to last object in list */
            case KEY_END:
                mvwaddstr_fill(pick->win, pick->y, pick->x,
                               pick->d_object[pick->d_idx],
                               pick->tbl_col_width);
                int display_tbl_page = pick->tbl_page;
                pick->d_idx = pick->d_cnt - 1;
                pick->tbl_page =
                    pick->d_idx / (pick->pg_lines * pick->tbl_cols);
                pick->tbl_line =
                    (pick->d_idx / pick->tbl_cols) % pick->pg_lines;
                pick->tbl_col = pick->d_idx % pick->tbl_cols;
                pick->y = pick->tbl_line;
                if (display_tbl_page != pick->tbl_page)
                    display_page(pick);
                in_key = 0;
                continue;

            case '\t':
                in_key = 0;
                break;

                /** 'h' or KEY_LEFT or Backspace Moves selection to previous
                 * object in list */
            case 'h':
            case KEY_LEFT:
            case KEY_BACKSPACE:
                mvwaddstr_fill(pick->win, pick->y, pick->x,
                               pick->d_object[pick->d_idx],
                               pick->tbl_col_width);
                if (pick->tbl_col > 0)
                    pick->tbl_col--;
                pick->d_idx = pick->tbl_page * pick->pg_lines * pick->tbl_cols +
                              pick->tbl_col * pick->pg_lines + pick->tbl_line;
                in_key = 0;
                continue;

            /** 'j' or KEY_DOWN Moves selection to next object in list, 'k'
                or KEY_UP Moves selection to previous object in list */
            case 'j':
            case KEY_DOWN:
                mvwaddstr_fill(pick->win, pick->y, pick->x,
                               pick->d_object[pick->d_idx],
                               pick->tbl_col_width);
                if (pick->tbl_page * pick->pg_lines * pick->tbl_cols +
                            pick->tbl_col * pick->pg_lines + pick->tbl_line <
                        pick->d_cnt - 1 &&
                    pick->tbl_line < pick->pg_lines - 1)
                    pick->tbl_line++;
                pick->d_idx = pick->tbl_page * pick->pg_lines * pick->tbl_cols +
                              pick->tbl_col * pick->pg_lines + pick->tbl_line;
                in_key = 0;
                continue;

            /** 'k' or KEY_UP Moves selection to previous object in list */
            case 'k':
            case KEY_UP:
                mvwaddstr_fill(pick->win, pick->y, pick->x,
                               pick->d_object[pick->d_idx],
                               pick->tbl_col_width);
                if (pick->tbl_line > 0)
                    pick->tbl_line--;
                pick->d_idx = pick->tbl_page * pick->pg_lines * pick->tbl_cols +
                              pick->tbl_col * pick->pg_lines + pick->tbl_line;
                in_key = 0;
                continue;

            /** 'l' or KEY_RIGHT Moves selection to next object in list */
            case 'l':
            case KEY_RIGHT:
                mvwaddstr_fill(pick->win, pick->y, pick->x,
                               pick->d_object[pick->d_idx],
                               pick->tbl_col_width);
                /** pick->obj_idx += pick->tbl_lines -> next column */
                if (pick->tbl_page * pick->pg_lines * pick->tbl_cols +
                            (pick->tbl_col + 1) * pick->pg_lines +
                            pick->tbl_line <
                        pick->d_cnt - 1 &&
                    pick->tbl_col < pick->tbl_cols - 1)
                    pick->tbl_col++;
                pick->d_idx = pick->tbl_page * pick->pg_lines * pick->tbl_cols +
                              pick->tbl_col * pick->pg_lines + pick->tbl_line;
                in_key = 0;
                continue;

            /** KEY_NPAGE or 'Ctrl+f' Moves selection to next page of
               objects */
            case KEY_NPAGE:
            case '\06':
                if (pick->tbl_pages == 1)
                    continue;
                if (pick->tbl_page < pick->tbl_pages - 1) {
                    pick->tbl_page++;
                    pick->pg_line = 0;
                    pick->tbl_col = 0;
                }
                pick->d_idx = pick->tbl_page * pick->pg_lines * pick->tbl_cols +
                              pick->tbl_cols * pick->pg_line + pick->tbl_col;
                display_page(pick);
                in_key = 0;
                continue;

            /**   KEY_PPAGE or 'Ctrl+b' Moves selection to previous page of
             * objects */
            case KEY_PPAGE:
            case '\02':
                if (pick->tbl_pages == 1)
                    continue;
                if (pick->tbl_page > 0)
                    pick->tbl_page--;
                pick->d_idx = pick->tbl_page * pick->pg_lines * pick->tbl_cols +
                              pick->tbl_cols * pick->pg_line + pick->tbl_col;
                display_page(pick);
                in_key = 0;
                continue;

            /** KEY_HOME Moves selection to first object in list */
            case KEY_HOME:
                pick->tbl_page = 0;
                pick->tbl_line = 0;
                pick->tbl_col = 0;
                pick->d_idx = pick->tbl_page * pick->pg_lines * pick->tbl_cols +
                              pick->tbl_cols * pick->pg_line + pick->tbl_col;
                display_page(pick);
                in_key = 0;
                continue;

            /** KEY_LL (lower left of numeric pad) Moves selection to last
                object in list */
            case KEY_LL:
                pick->tbl_page = pick->tbl_pages - 1;
                pick->d_idx = pick->tbl_page * pick->pg_lines * pick->tbl_cols +
                              pick->tbl_cols * pick->pg_line + pick->tbl_col;
                display_page(pick);
                in_key = 0;
                continue;

                /** KEY_MOUSE Handles mouse events for selection and chyron key
                 * activation */

            case KEY_MOUSE:
                if (click_y == -1 || click_x == -1)
                    continue;
                unreverse_object(pick);
                pick->y = click_y;
                pick->tbl_col = (click_x - 1) / (pick->tbl_col_width + 1);
                if (pick->tbl_col < 0 || pick->tbl_col >= pick->tbl_cols)
                    continue;
                pick->d_idx = pick->tbl_page * pick->pg_lines * pick->tbl_cols +
                              pick->tbl_col * pick->pg_lines + pick->y;
                in_key = 't'; /** Set cmdkey to 't' to toggle selection on mouse
                                 click */
                click_y = click_x = -1;
                in_key = 0;
                continue;

            default:
                in_key = 0;
                continue;
            }
            unreverse_object(pick);
            in_key = 0;
            break;
        }
        /** ===============================================================
            ===============================================================

            Field editing loop

            ===============================================================
            =============================================================== */
        while (1) {
            if (in_key == 0) {
                if (accept_s != nullptr && accept_s[0] != '\0') { // Error
                    if (match_objects(pick, accept_s) == 0) {
                        in_key = KEY_BACKSPACE;
                        continue;
                    }
                    display_page(pick);
                }
                // reverse_object(pick);
                wrefresh(win);
                rtrim(accept_s);
                s = &filler_s[0];
                e = s + flen;
                while (s != e)
                    *s++ = ' ';
                *s = '\0';
                mvwaddstr(win2, line, col, filler_s);
                mvwaddstr(win2, line, col, accept_s);
                tcflush(0, TCIFLUSH);
                wmove(win2, line, pos);
                in_key = dxwgetch(win, win2, pick->chyron, -1);
            }

            switch (in_key) {
            case '\n':
            case KEY_ENTER:
                return (in_key);

                /** KEY_IC toggles insert mode */
                /** Tab toggles between windows */
            case KEY_BTAB:
            case KEY_UP:
            case '\t':
                in_key = 0;
                break;

            case KEY_F(1):
                return (in_key);

            /** KEY_F(9) Cancels the current operation */
            case KEY_BREAK:
            case KEY_F(9):
                in_key = KEY_F(9);
                return (in_key);

            /** KEY_F(10) is the default key for accepting the field */
            case KEY_F(10):
                return (in_key);

            case KEY_END:
            case Ctrl('e'):
                while (*ptr != '\0')
                    ptr++;
                pos = col + (ptr - fstart);
                in_key = 0;
                continue;

            case KEY_IC:
                if (f_insert) {
                    f_insert = FALSE;
                    set_chyron_key_cp(pick->chyron, 18, "INS", KEY_IC,
                                      cp_reverse);
                } else {
                    f_insert = TRUE;
                    set_chyron_key_cp(pick->chyron, 18, "INS", KEY_IC,
                                      cp_reverse_highlight);
                }
                compile_chyron(pick->chyron);
                display_chyron(win2, pick->chyron, 1, pick->chyron->l);
                in_key = 0;
                continue;

            /** KEY_DC deletes character at cursor */
            case KEY_DC:
                s = ptr + 1;
                d = ptr;
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
                ptr = fstart;
                pos = col;
                in_key = 0;
                continue;

            /** KEY_BACKSPACE deletes character before cursor */
            case KEY_BACKSPACE:
                if (ptr > fstart) {
                    ptr--;
                    pos--;
                }
                s = ptr + 1;
                d = ptr;
                while (*s != '\0')
                    *d++ = *s++;
                *d = '\0';
                str_end = d;
                in_key = 0;
                continue;

            /** KEY_LEFT moves cursor left one character */
            case KEY_LEFT:
                if (ptr > fstart) {
                    ptr--;
                    pos--;
                }
                in_key = 0;
                continue;

            /** KEY_RIGHT moves cursor right one character */
            case KEY_RIGHT:
                if (ptr < fend && ptr < str_end) {
                    ptr++;
                    pos++;
                }
                in_key = 0;
                continue;

            /** Handles mouse events for field editing */
            case KEY_MOUSE:
                pos = click_x;
                fstart = accept_s;
                fend = fstart + flen;
                str_end = fstart + strlen(fstart);
                ptr = fstart + (pos - 3);
                if (ptr > str_end)
                    pos -= ptr - str_end;
                ptr = min(ptr, str_end);
                in_key = 0;
                continue;

            default:
                if (ptr >= fend) {
                    in_key = 0;
                    continue;
                }
                if (in_key >= ' ') {
                    if (f_insert) {
                        if (str_end < fend) {
                            s = str_end - 1;
                            d = str_end;
                            while (s >= ptr)
                                *d-- = *s--;
                            *ptr++ = in_key;
                            str_end++;
                            pos++;
                        }
                    } else {
                        if (ptr < fend) {
                            if (ptr < str_end) {
                                *ptr++ = in_key;
                                pos++;
                            } else if (ptr == str_end) {
                                *ptr++ = in_key;
                                *ptr = '\0';
                                str_end = ptr;
                                pos++;
                            }
                        }
                    }
                    in_key = 0;
                    continue;
                }
            }
            break;
        }
    }
}
