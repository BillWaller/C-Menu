/** @file pick_engine.c
    @brief pick from a list of choices
    @author Bill Waller
    Copyright (c) 2025
    MIT License
    billxwaller@gmail.com
    @date 2026-02-09
 */

#include <common.h>
#include <fcntl.h>
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
void pick_display_chyron(Pick *);
int read_pick_input(Init *);

int pipe_fd[2];

char const pagers_editors[12][10] = {"view", "mview", "less", "more",
                                     "vi",   "vim",   "nano", "nvim",
                                     "pico", "emacs", "edit", ""};

/** @brief Initializes pick structure and opens pick input file or pipe
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
    char tmp_str[MAXLEN];
    int m;
    pid_t pid = 0;

    if (init->pick != NULL)
        destroy_pick(init);
    Pick *pick = new_pick(init, argc, argv, begy, begx);
    if (init->pick != pick)
        abend(-1, "init->pick != pick\n");
    SIO *sio = init->sio;
    if (pick->provider_cmd[0] != '\0') {
        str_to_args(s_argv, pick->provider_cmd, MAXARGS - 1);
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
        if ((pick->in_fp = fopen(pick->in_spec, "rb")) == NULL) {
            m = MAXLEN - 29;
            strnz__cpy(tmp_str, "Can't open pick input file: ", m);
            m -= strlen(pick->in_spec);
            strnz__cat(tmp_str, pick->in_spec, m);
            Perror(tmp_str);
            return (1);
        }
    }
    read_pick_input(init);
    if (pick->f_in_pipe && pid > 0) {
        /** Wait for provider_cmd child process to finish before proceeding */
        waitpid(pid, NULL, 0);
        close(pipe_fd[P_READ]);
        dup2(sio->stdin_fd, STDIN_FILENO);
        dup2(sio->stdout_fd, STDOUT_FILENO);
        restore_curses_tioctl();
        sig_prog_mode();
        keypad(pick->win, true);
    }
    if (pick->obj_cnt == 0) {
        Perror("No pick objects available");
        destroy_pick(init);
        return (1);
    }
    /** Enter pick_engine */
    pick_engine(init);
    win_del();
    destroy_pick(init);
    return 0;
}
/** @brief Reads pick input from file pointer and saves objects into pick
   structure
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
    pick->obj_cnt = pick->pg_lines = pick->tbl_cols = 0;
    pick->obj_idx = pick->tbl_page = pick->y = pick->tbl_col = pick->x = 0;
    pick->tbl_pages = 1;

    if (pick->in_fp) {
        while (fgets(pick->in_buf, sizeof(pick->in_buf), pick->in_fp) != NULL)
            save_object(pick, pick->in_buf);
    } else
        for (i = 1; i < pick->argc; i++)
            save_object(pick, pick->argv[i]);
    if (pick->in_fp != NULL)
        fclose(pick->in_fp);
    if (!pick->obj_idx)
        return (-1);
    pick->obj_cnt = pick->obj_idx;
    pick->obj_idx = 0;
    return 0;
}
/** @brief Initializes pick interface, calculates window size and position, and
   enters picker loop
    @param init Pointer to Init structure containing pick information
    @return Count of selected objects on success, -1 if user cancels
    @note Initializes key command strings for chyron display and calculates pick
   window size and position based on terminal size and pick parameters.
    @note Opens pick window and displays first page of objects. Enters picker
   loop to handle user input and interactions. If user cancels selection,
   returns -1.
   @note If user accepts selection, returns count of selected objects. */
int pick_engine(Init *init) {
    /** Initialize window and data structures */
    int n, chyron_l, rc;
    int maxy, maxx, win_maxy, win_maxx;

    for (n = 0; key_cmd[n].end_pos != -1; n++)
        key_cmd[n].text[0] = '\0';
    strnz__cpy(key_cmd[1].text, "F1 Help", 32);
    strnz__cpy(key_cmd[9].text, "F9 Cancel", 32);
    strnz__cpy(key_cmd[10].text, "F10 Accept", 32);
    strnz__cpy(key_cmd[11].text, "PgUp", 32);
    strnz__cpy(key_cmd[12].text, "PgDn", 32);
    strnz__cpy(key_cmd[13].text, "Space", 32);
    strnz__cpy(key_cmd[14].text, "Enter", 32);
    chyron_l = chyron_mk(key_cmd, pick->chyron_s);
    getmaxyx(stdscr, maxy, maxx);
    /** Calculate pick window size and position based on terminal size and pick
        parameters */
    win_maxy = (maxy * 8) / 10;
    if (win_maxy > (maxy - pick->begy) - 2)
        win_maxy = (maxy - pick->begy) - 2;
    win_maxx = (maxx * 9) / 10;
    if (win_maxx > (maxx - pick->begx) - 2)
        win_maxx = (maxx - pick->begx) - 2;
    if (chyron_l > win_maxx)
        chyron_l = strnz(pick->chyron_s, win_maxx);
    if (pick->tbl_col_width < 4)
        pick->tbl_col_width = 4;
    if (pick->tbl_col_width > win_maxx - 2)
        pick->tbl_col_width = win_maxx - 2;
    pick->tbl_cols = (win_maxx / (pick->tbl_col_width + 1));
    pick->win_width = (pick->tbl_col_width + 1) * pick->tbl_cols;
    if (pick->win_width < chyron_l)
        pick->win_width = chyron_l;
    pick->tbl_lines = ((pick->obj_cnt - 1) / pick->tbl_cols) + 1;
    pick->tbl_pages = (pick->tbl_lines / (win_maxy - 1)) + 1;
    pick->pg_lines = (pick->tbl_lines / pick->tbl_pages) + 1;
    pick->win_lines = pick->pg_lines + 1;
    pick->tbl_page = 0;
    if (pick->begy == 0)
        pick->begy = (LINES - pick->win_lines) / 5;
    else if (pick->begy + pick->win_lines > LINES - 4)
        pick->begy = LINES - pick->win_lines - 2;
    if (pick->begx + pick->win_width > COLS - 4)
        pick->begx = COLS - pick->win_width - 2;
    else if (pick->begx == 0)
        pick->begx = (COLS - pick->win_width) / 2;

    rc = open_pick_win(init);
    if (rc)
        return (rc);
    display_page(pick);
    reverse_object(pick);
    pick->obj_idx = 0;
    pick->x = 1;
    mousemask(BUTTON1_CLICKED | BUTTON1_DOUBLE_CLICKED, NULL);
    /** Enter picker loop to handle user input and interactions */
    picker(init);
    if (pick->select_cnt > 0) {
        if (pick->f_out_spec && pick->out_spec[0])
            rc = output_objects(pick);
        if (pick->f_cmd && pick->cmd[0])
            rc = exec_objects(init);
    }
    return (rc);
}
/** @brief Saves a string as an object in the pick structure
    @param pick Pointer to Pick structure
    @param s String to save as an object
    @note If the current object index is less than the maximum allowed, saves
   the string as an object in the pick structure. Updates the column width if
   necessary and marks the object as not selected. Increments the object index
   for the next object to be saved. */
void save_object(Pick *pick, char *s) {
    int l;

    if (pick->obj_idx < OBJ_MAXCNT - 1) {
        l = strlen(s);
        if (l > OBJ_MAXLEN - 1)
            s[OBJ_MAXLEN - 1] = '\0';
        if (l > pick->tbl_col_width)
            pick->tbl_col_width = l;
        if (l < 1)
            l = 1;
        pick->object[pick->obj_idx] = (char *)calloc(l + 1, sizeof(char));
        strnz__cpy(pick->object[pick->obj_idx], s, l);
        pick->f_selected[pick->obj_idx] = FALSE;
        pick->obj_idx++;
    }
}

/** @brief Main loop to handle user input and interactions for pick interface
    @param init Pointer to Init structure
    @return Number of selected objects or -1 if user cancels
    @note Handles user input for navigating and selecting objects in the pick
   interface.
    @note Supports various key commands for navigation, selection, and
   accepting/canceling the selection.
    @note Updates the display accordingly based on user interactions.
    @note If the user cancels the selection, returns -1.
    @note If the user accepts the selection, returns the count of selected
   objects. */
int picker(Init *init) {
    int cmd_key;
    int display_tbl_page;

    pick = init->pick;
    MEVENT event;
    event.y = event.x = -1;
    cmd_key = 0;
    while (1) {
        tcflush(tty_fd, TCIFLUSH);
        if (cmd_key == 0)
            cmd_key = xwgetch(pick->win);
        switch (
            cmd_key) { /** 'q', or KEY_F(9) cancel selection and exit picker */
        case 'q':
        case 'Q':
        case KEY_F(9):
            return -1;
            /** KEY_F(1) or 'H' Displays help screen for pick interface */
        case KEY_F(1):
        case 'H':
            display_pick_help(init);
            display_page(pick);
            reverse_object(pick);
            cmd_key = 0;
            break;
            /** 't' or Space Toggles selection of current object */
        case ' ':
        case 't':
        case 'T':
            toggle_object(pick);
            if (pick->select_cnt == pick->select_max)
                return pick->select_cnt;
            cmd_key = 0;
            break;
            /** Enter or KEY_F(10) Accepts current selection and exits picker,
             * returning count of selected objects */
        case KEY_F(10):
        case '\n':
        case KEY_ENTER:
            return pick->select_cnt;
            /** KEY_END Moves selection to last object in list */
        case KEY_END:
            mvwaddstr_fill(pick->win, pick->y, pick->x,
                           pick->object[pick->obj_idx], pick->tbl_col_width);
            display_tbl_page = pick->tbl_page;
            pick->obj_idx = pick->obj_cnt - 1;
            pick->tbl_page = pick->obj_idx / (pick->pg_lines * pick->tbl_cols);
            pick->tbl_line = (pick->obj_idx / pick->tbl_cols) % pick->pg_lines;
            pick->tbl_col = pick->obj_idx % pick->tbl_cols;
            pick->y = pick->tbl_line;
            if (display_tbl_page != pick->tbl_page) {
                display_page(pick);
            }
            reverse_object(pick);
            cmd_key = 0;
            break;
        /** 'l' or KEY_RIGHT Moves selection to next object in list */
        case 'l':
        case KEY_RIGHT:
            mvwaddstr_fill(pick->win, pick->y, pick->x,
                           pick->object[pick->obj_idx], pick->tbl_col_width);
            /** pick->obj_idx += pick->tbl_lines -> next column */
            if (pick->tbl_page * pick->pg_lines * pick->tbl_cols +
                        (pick->tbl_col + 1) * pick->pg_lines + pick->tbl_line <
                    pick->obj_cnt - 1 &&
                pick->tbl_col < pick->tbl_cols - 1)
                pick->tbl_col++;
            pick->obj_idx = pick->tbl_page * pick->pg_lines * pick->tbl_cols +
                            pick->tbl_col * pick->pg_lines + pick->tbl_line;
            reverse_object(pick);
            cmd_key = 0;
            break;
            /** 'h' or KEY_LEFT or Backspace Moves selection to previous object
             * in list */
        case 'h':
        case KEY_LEFT:
        case KEY_BACKSPACE:
            mvwaddstr_fill(pick->win, pick->y, pick->x,
                           pick->object[pick->obj_idx], pick->tbl_col_width);
            if (pick->tbl_col > 0)
                pick->tbl_col--;
            pick->obj_idx = pick->tbl_page * pick->pg_lines * pick->tbl_cols +
                            pick->tbl_col * pick->pg_lines + pick->tbl_line;
            cmd_key = 0;
            reverse_object(pick);
            break;
            /** 'j' or KEY_DOWN Moves selection to next object in list, 'k' or
             * KEY_UP Moves selection to previous object in list */
        case 'j':
        case KEY_DOWN:
            mvwaddstr_fill(pick->win, pick->y, pick->x,
                           pick->object[pick->obj_idx], pick->tbl_col_width);
            /** pick->obj_idx++ column down */
            if (pick->tbl_page * pick->pg_lines * pick->tbl_cols +
                        pick->tbl_col * pick->pg_lines + pick->tbl_line <
                    pick->obj_cnt - 1 &&
                pick->tbl_line < pick->pg_lines - 1)
                pick->tbl_line++;
            pick->obj_idx = pick->tbl_page * pick->pg_lines * pick->tbl_cols +
                            pick->tbl_col * pick->pg_lines + pick->tbl_line;
            reverse_object(pick);
            cmd_key = 0;
            break;
            /** 'k' or KEY_UP Moves selection to previous object in list */
        case 'k':
        case KEY_UP:
            /** KEY_UP or 'k' Moves selection to previous object in list
                pick->obj_idx-- column up */
            mvwaddstr_fill(pick->win, pick->y, pick->x,
                           pick->object[pick->obj_idx], pick->tbl_col_width);
            if (pick->tbl_line > 0)
                pick->tbl_line--;
            pick->obj_idx = pick->tbl_page * pick->pg_lines * pick->tbl_cols +
                            pick->tbl_col * pick->pg_lines + pick->tbl_line;
            reverse_object(pick);
            cmd_key = 0;
            break;
        /** KEY_NPAGE or 'Ctrl+f' Moves selection to next page of objects, */
        case KEY_NPAGE:
        case '\06':
            /** KEY_NPAGE or 'Ctrl+f' Moves selection to next page of objects */
            if (pick->tbl_page < pick->tbl_pages - 1) {
                pick->tbl_page++;
                pick->pg_line = 0;
                pick->tbl_col = 0;
            }
            pick->obj_idx = pick->tbl_page * pick->pg_lines * pick->tbl_cols +
                            pick->tbl_cols * pick->pg_line + pick->tbl_col;
            display_page(pick);
            reverse_object(pick);
            cmd_key = 0;
            break;
            /**   KEY_PPAGE or 'Ctrl+b' Moves selection to previous page of
             * objects */
        case KEY_PPAGE:
        case '\02':
            /** KEY_PPAGE or 'Ctrl+b' Moves selection to previous page of
                objects */
            if (pick->tbl_page > 0)
                pick->tbl_page--;
            pick->obj_idx = pick->tbl_page * pick->pg_lines * pick->tbl_cols +
                            pick->tbl_cols * pick->pg_line + pick->tbl_col;
            display_page(pick);
            reverse_object(pick);
            cmd_key = 0;
            break;
            /** KEY_HOME Moves selection to first object in list */
        case KEY_HOME:
            pick->tbl_page = 0;
            pick->tbl_line = 0;
            pick->tbl_col = 0;
            pick->obj_idx = pick->tbl_page * pick->pg_lines * pick->tbl_cols +
                            pick->tbl_cols * pick->pg_line + pick->tbl_col;
            display_page(pick);
            reverse_object(pick);
            cmd_key = 0;
            break;
            /** KEY_LL (lower left of numeric pad) Moves selection to last
                object in list */
        case KEY_LL:
            pick->tbl_page = pick->tbl_pages - 1;
            pick->obj_idx = pick->tbl_page * pick->pg_lines * pick->tbl_cols +
                            pick->tbl_cols * pick->pg_line + pick->tbl_col;
            display_page(pick);
            reverse_object(pick);
            cmd_key = 0;
            break;
            /** KEY_MOUSE Handles mouse events for selection and chyron key
             * activation */
        case KEY_MOUSE:
            if (getmouse(&event) != OK) {
                cmd_key = 0;
                break;
            }
            /** BUTTON1 CLICK or DOUBLE_CLICK Toggles Selection
                or Activates Chyron Keys */
            if (event.bstate == BUTTON1_CLICKED ||
                event.bstate == BUTTON1_DOUBLE_CLICKED) {
                if (!wenclose(pick->win, event.y, event.x)) {
                    cmd_key = 0;
                    break;
                }
                wmouse_trafo(pick->win, &event.y, &event.x, false);
                if (event.y < 0 ||
                    event.x >= (pick->tbl_cols * (pick->tbl_col_width + 1))) {
                    cmd_key = 0;
                    break;
                }
                unreverse_object(pick);
                pick->y = event.y;
                if (pick->y == pick->pg_lines) {
                    cmd_key = get_chyron_key(key_cmd, event.x);
                    continue;
                }
                pick->tbl_col = (event.x - 1) / (pick->tbl_col_width + 1);
                if (pick->tbl_col < 0 || pick->tbl_col >= pick->tbl_cols) {
                    cmd_key = 0;
                    continue;
                }
                pick->obj_idx =
                    pick->tbl_page * pick->pg_lines * pick->tbl_cols +
                    pick->tbl_col * pick->pg_lines + pick->y;
                toggle_object(pick);
                reverse_object(pick);
                if (pick->select_cnt == pick->select_max)
                    return pick->select_cnt;
                wrefresh(pick->win);
                cmd_key = 0;
                break;
            }
            cmd_key = 0;
            break;
        default:
            cmd_key = 0;
            break;
        }
    }
    return 0;
}
/** @brief Displays current page of objects in pick window
    @param pick Pointer to Pick structure containing objects and display
   information
    @note Clears the pick window and displays the current page of objects based
   on the current table page, line, and column. Marks selected objects with an
   asterisk. Updates the chyron with page information at the bottom of the pick
   window. */
void display_page(Pick *pick) {
    int y, col, pidx;
    for (y = 0; y < pick->pg_lines; y++) {
        wmove(pick->win, y, 0);
        wclrtoeol(pick->win);
    }
    pidx = pick->tbl_page * pick->pg_lines * pick->tbl_cols;
    for (col = 0; col < pick->tbl_cols; col++) {
        pick->x = col * (pick->tbl_col_width + 1) + 1;
        for (y = 0; y < pick->pg_lines; y++, pidx++) {
            if (pidx < pick->obj_cnt) {
                if (pick->f_selected[pidx])
                    mvwaddstr(pick->win, y, pick->x - 1, "*");
                mvwaddstr_fill(pick->win, y, pick->x, pick->object[pidx],
                               pick->tbl_col_width);
            }
        }
    }
    pick_display_chyron(pick);
}
/** @brief Displays chyron with page information at bottom of pick window
    @param pick Pointer to Pick structure containing chyron information
    @note Constructs a string for the chyron that includes the current page
   number and total pages. Displays the chyron at the bottom of the pick window
   with reverse video attribute. Clears any remaining space on the line after
   the chyron text. */
void pick_display_chyron(Pick *pick) {
    int l;
    char tmp_str[MAXLEN];
    ssnprintf(tmp_str, MAXLEN - 1, "%s| Page %d of %d ", pick->chyron_s,
              pick->tbl_page + 1, pick->tbl_pages);
    l = strlen(tmp_str);
    wattron(pick->win, WA_REVERSE);
    mvwaddstr(pick->win, pick->pg_lines, 0, tmp_str);
    wattroff(pick->win, WA_REVERSE);
    wmove(pick->win, pick->pg_lines, l);
    wclrtoeol(pick->win);
}
/** @brief Reverses the display of the currently selected object in pick window
    @param pick Pointer to Pick structure containing object and display
   information
    @note Calculates the x coordinate for the currently selected object based on
   the current table column and column width. Moves the cursor to the object's
   position in the pick window, turns on reverse video attribute, and displays
   the object's text. Turns off reverse video attribute and refreshes the pick
   window to show the updated display. Moves the cursor back to the position
   before the object text for potential further interactions. */
void reverse_object(Pick *pick) {
    pick->x = pick->tbl_col * (pick->tbl_col_width + 1) + 1;
    pick->y = pick->tbl_line;
    wmove(pick->win, pick->y, pick->x);
    wattron(pick->win, WA_REVERSE);
    mvwaddstr_fill(pick->win, pick->y, pick->x, pick->object[pick->obj_idx],
                   pick->tbl_col_width);
    wattroff(pick->win, WA_REVERSE);
    wrefresh(pick->win);
    wmove(pick->win, pick->y, pick->x - 1);
}
/** @brief Unreverses the display of the currently selected object in pick
   window
    @param pick Pointer to Pick structure containing object and display
   information
    @note Calculates the x coordinate for the currently selected object based on
   the current table column and column width. Moves the cursor to the object's
   position in the pick window and displays the object's text without reverse
   video attribute. Refreshes the pick window to show the updated display. Moves
   the cursor back to the position before the object text for potential further
   interactions. */
void unreverse_object(Pick *pick) {
    pick->x = pick->tbl_col * (pick->tbl_col_width + 1) + 1;
    wmove(pick->win, pick->y, pick->x);
    mvwaddstr_fill(pick->win, pick->y, pick->x, pick->object[pick->obj_idx],
                   pick->tbl_col_width);
    wrefresh(pick->win);
    wmove(pick->win, pick->y, pick->x - 1);
}
/** @brief Toggles the selection state of the currently selected object in pick
   window
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
    if (pick->f_selected[pick->obj_idx]) {
        pick->select_cnt--;
        pick->f_selected[pick->obj_idx] = FALSE;
        mvwaddstr(pick->win, pick->y, pick->x - 1, " ");
    } else {
        pick->select_cnt++;
        pick->f_selected[pick->obj_idx] = true;
        mvwaddstr(pick->win, pick->y, pick->x - 1, "*");
    }
}
/** @brief Outputs selected objects to specified output file
    @param pick Pointer to Pick structure containing selected objects and output
    file information
    @return 0 on success, 1 on failure
    @note If output file cannot be opened, an error message is printed and the
   function returns 1. Otherwise, selected objects are written to the output
   file, one per line, and the file is closed before returning 0.
*/
int output_objects(Pick *pick) {
    int m;
    if ((pick->out_fp = fopen(pick->out_spec, "w")) == NULL) {
        m = MAXLEN - 30;
        strnz__cpy(tmp_str, "Can't open pick output file: ", m);
        m -= strlen(pick->in_spec);
        strnz__cat(tmp_str, pick->out_spec, m);
    }
    for (pick->obj_idx = 0; pick->obj_idx < pick->obj_cnt; pick->obj_idx++) {
        if (pick->f_selected[pick->obj_idx])
            fprintf(stdout, "%s\n", pick->object[pick->obj_idx]);
    }
    fflush(stdout);
    if (pick->out_fp != NULL)
        fclose(pick->out_fp);
    return (0);
}
/** @brief Executes specified command with selected objects as arguments
   @param init Pointer to Init structure
   @return 0 on success, 1 on failure
   @note Parses command string and appends selected objects as arguments to the
   command. If command contains "%%", it is replaced with a space- separated
   list of selected objects. Executes the command using execvp in a child
   process and waits for it to finish. If the command is a pager or editor, it
   is executed within the pick interface using mview instead of execvp.
   @note If f_append_args is true, the argument containing %% is replaced with
   the concatenated selected objects. If f_append_args is false, selected
   objects are added as separate arguments and the original command arguments
   remain unchanged.
   @note margv should be null-terminated to indicate the end of arguments for
   execvp
    @note Memory allocated for arguments is freed after execution to prevent
   memory leaks.
    @note If execvp fails, an error message is printed and the child process
   exits with failure status
    @note The parent process waits for the child process to finish before
   proceeding and restores terminal settings
    @note If the command is a pager or editor, it is executed within the pick
   interface using mview instead of execvp
    @note The base name of the command is extracted to check if it is a pager or
   editor
    @note If the command is a pager or editor, the pick interface is used to
   display the command output instead of executing it in a separate terminal
   This allows the user to view the command output without leaving the pick
   interface and provides a more seamless user experience.
    @note If the command is not a pager or editor, it is executed in a separate
   terminal and the pick interface is restored after execution
    @note If the command to be executed is view, an external command is not
   needed, instead the mview function can be used to display the output within
   the pick interface */
int exec_objects(Init *init) {
    int rc = -1;
    int margc;
    char *margv[MAXARGS];
    char tmp_str[MAXLEN];
    char title[MAXLEN];
    char sav_arg[MAXLEN];
    char *out_s;
    int margx = 0;
    int i = 0;
    pid_t pid = 0;
    bool f_append_args = false;
    char *s1;

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
    margc = str_to_args(margv, pick->cmd, MAXARGS - 1);
    tmp_str[0] = '\0';
    if (pick->f_multiple_cmd_args) {
        for (i = 0; i < pick->obj_cnt; i++) {
            if (pick->f_selected[i] && margc < MAXARGS) {
                if (tmp_str[0] != '\0')
                    strnz__cat(tmp_str, " ", MAXLEN - 1);
                strnz__cat(tmp_str, pick->object[i], MAXLEN - 1);
            }
        }
        margv[margc++] = strdup(tmp_str);
    } else {
        f_append_args = false;
        i = 0;
        while (i < margc) {
            if (strstr(margv[i], "%%") != NULL) {
                tmp_str[0] = '\0';
                f_append_args = true;
                strnz__cpy(sav_arg, margv[i], MAXLEN - 1);
                margx = i;
                break;
            }
            i++;
        }
        for (i = 0; i < pick->obj_cnt - 1; i++) {
            if (pick->f_selected[i] && margc < MAXARGS - 1) {
                if (f_append_args == true) {
                    if (tmp_str[0] != '\0')
                        strnz__cat(tmp_str, " ", MAXLEN - 1);
                    strnz__cat(tmp_str, pick->object[i], MAXLEN - 1);
                    continue;
                }
                margv[margc++] = strdup(pick->object[i]);
            }
        }
        if (f_append_args == true) {
            if (margv[margx] != NULL) {
                free(margv[margx]);
                margv[margx] = NULL;
            }
            out_s = rep_substring(sav_arg, "%%", tmp_str);
            if (out_s == NULL || out_s[0] == '\0') {
                i = 0;
                while (i < margc) {
                    if (margv[i] != NULL)
                        free(margv[i]);
                    i++;
                }
                Perror("rep_substring() failed in exec_objects");
                return 1;
            }
            strnz__cpy(title, out_s, MAXLEN - 1);
            margv[margx] = strdup(out_s);
            if (out_s != NULL) {
                free(out_s);
                out_s = NULL;
            }
        }
    }
    strnz__cpy(tmp_str, margv[0], MAXLEN - 1);
    margv[margc] = NULL;
    s1 = tmp_str;
    char *sp;
    char *tok;
    tok = strtok_r(s1, " ", &sp);
    strnz__cpy(sav_arg, tok, MAXLEN - 1);
    base_name(tmp_str, sav_arg);
    if (tmp_str[0] != '\0' &&
        (strcmp(tmp_str, "view") == 0 || strcmp(tmp_str, "mview") == 0)) {
        /** initialize mview arguments and execute mview to display command
            output within pick interface */
        zero_opt_args(init);
        parse_opt_args(init, margc, margv);
        init->lines = 60;
        init->cols = 80;
        init->begy = pick->begy + 2;
        init->begx = pick->begx + 1;
        if (title[0] != '\0')
            strnz__cpy(init->title, title, MAXLEN - 1);
        else
            strnz__cpy(init->title, margv[margc], MAXLEN - 1);
        mview(init, margc, margv);
        i = 0;
        while (i < margc) {
            if (margv[i] != NULL)
                free(margv[i]);
            i++;
        }
        return 0;
    } else {
        if ((pid = fork()) == -1) {
            /** fork failed, free margv and return error */
            i = 0;
            while (i < margc) {
                if (margv[i] != NULL)
                    free(margv[i]);
                i++;
            }
            Perror("fork() failed in exec_objects");
            return (1);
        }
        if (pid == 0) {
            /** Child process to execute command */
            execvp(margv[0], margv);
            /** If execvp returns, it means execution failed, so free margv and
                print error message before exiting */
            strnz__cpy(tmp_str, "Can't exec pick cmd: ", MAXLEN - 1);
            strnz__cat(tmp_str, margv[0], MAXLEN - 1);
            Perror(tmp_str);
            exit(EXIT_FAILURE);
        }
    }
    waitpid(pid, NULL, 0);
    i = 0;
    while (i < margc) {
        if (margv[i] != NULL)
            free(margv[i]);
        i++;
    }
    restore_curses_tioctl();
    sig_prog_mode();
    keypad(pick->win, true);
    restore_wins();
    return rc;
}
/** @brief Initializes the pick window based on the parameters specified in the
Pick structure
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
    if (win_new(pick->win_lines, pick->win_width, pick->begy, pick->begx,
                pick->title, 0)) {
        ssnprintf(tmp_str, MAXLEN - 1, "win_new(%d, %d, %d, %d, %s, %b) failed",
                  pick->win_lines, pick->win_width, pick->begy, pick->begx,
                  pick->title, 0);
        Perror(tmp_str);

        return (1);
    }
    pick->win = win_win[win_ptr];
    pick->box = win_box[win_ptr];
    scrollok(pick->win, FALSE);
    keypad(pick->win, true);
    return 0;
}
/** @brief Displays the help screen for the pick interface using mview
    @param init Pointer to Init structure containing pick information
    @note Initializes the help_spec field in the Pick structure with the path to
   the pick help file. Then, constructs the argument list for executing mview
   with the help file as an argument. Finally, calls mview function to display
   the help screen within the pick interface. */
void display_pick_help(Init *init) {
    eargv[0] = strdup("view");
    eargv[1] = strdup("~/menuapp/help/pick.help");
    eargv[2] = NULL;
    eargc = 2;
    zero_opt_args(init);
    parse_opt_args(init, eargc, eargv);
    init->lines = 30;
    init->cols = 60;
    init->begy = pick->begy + 1;
    init->begx = pick->begx + 1;
    strnz__cpy(init->title, "Pick Help", MAXLEN - 1);
    mview(init, eargc, eargv);
    free(eargv[0]);
    free(eargv[1]);
    return;
}
