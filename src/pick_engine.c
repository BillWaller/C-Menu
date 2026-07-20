/** @file pick_engine.c
    @brief pick from a list of choices
    @author Bill Waller
    Copyright (c) 2025
    MIT License
    billxwaller@gmail.com
    @date 2026-02-09
 */

/** @defgroup pick_engine Object Selection
    @brief Navigate, Select, and Perform Action on Objects
 */

#include "include/common.h"
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>
int tbl_col, tbl_line, tbl_page, tbl_cols, pg_lines, tbl_pages;
int obj_idx, calculated_idx;
int pick_engine(Init *);
void save_object(Pick *, char *);
int picker(Init *, char *field);
void display_pick_page(Pick *);
void reverse_object(Pick *);
void unreverse_object(Pick *);
void toggle_object(Pick *);
int output_objects(Pick *);
int exec_objects(Init *);
int open_pick_win(Init *);
void display_pick_help(Init *);
int read_pick_input(Init *);
void deselect_object(Pick *);
int read_theme(Init *);
int new_pick_view(Init *);
void new_view_file(Init *, char *);
void end_pick_view(Init *);
void destroy_pick_view(Init *);
int match_objects(Pick *pick, char *s);
void remove_right_angle(Pick *);
void pick_std_chyron(Pick *);
int pipe_fd[2];

char const pagers_editors[12][10] = {"view", "view", "less", "more",
                                     "vi", "vim", "nano", "nvim",
                                     "pico", "emacs", "edit", ""};

/** @brief Initializes pick structure and opens pick input file or pipe
 *  @ingroup pick_engine
    @param init Pointer to Init structure
    @param argc Argument count
    @param argv Argument vector
    @param by Beginning y coordinate for pick window
    @param bx Beginning x coordinate for pick window
    @details If provider_cmd is specified, it takes precedence over in_spec and
    input file arguments.
    provider_cmd is executed and its output is read as pick input
    If provider_cmd is not specified, in_spec is used to read pick
    input from a file or stdin
    If provider_cmd is specified, it is executed and its output is
    read as pick input */
int init_pick(Init *init, int argc, char **argv, int by, int bx) {
    struct stat sb;
    char *s_argv[MAXARGS];
    int s_argc;
    char tmp_str[MAXLEN];
    pid_t pid = 0;

    Pick *pick = new_pick(init, argc, argv, by, bx);
    if (init->pick != pick)
        abend(-1, "init->pick != pick\n");
    // SIO *sio = init->sio;
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
            /** Prevent child process from writing to terminal */
            int dev_null = open("/dev/null", O_WRONLY);
            if (dev_null == -1) {
                Perror("open(/dev/null) failed in init_pick child process");
                exit(EXIT_FAILURE);
            }
            dup2(dev_null, STDERR_FILENO);
            close(dev_null);
            /** Close read end of pipe as Child only needs to write to pipe */
            close(pipe_fd[P_READ]);
            /** Connect CHILD STDOUT to write end of pipe */
            dup2(pipe_fd[P_WRITE], STDOUT_FILENO);
            /** STDOUT attached to write end of pipe, so close pipe fd */
            close(pipe_fd[P_WRITE]);
            execvp(s_argv[0], s_argv);
            strnz__cpy(tmp_str, "Can't exec pick start cmd: ", MAXLEN - 1);
            strnz__cat(tmp_str, s_argv[0], MAXLEN - 1);
            Perror(tmp_str);
            exit(EXIT_FAILURE);
        }
        /** Return to Parent
            Close write end of pipe as Parent only needs to read from pipe */
        close(pipe_fd[P_WRITE]);
        /** Open a file pointer on read end of pipe */
        pick->in_fp = fdopen(pipe_fd[P_READ], "rb");
        pick->f_in_pipe = true;
        destroy_argv(s_argc, s_argv);
    } else {
        if ((pick->in_spec[0] == '\0') || strcmp(pick->in_spec, "-") == 0 || strcmp(pick->in_spec, "/dev/stdin") == 0) {
            strnz__cpy(pick->in_spec, "/dev/stdin", MAXLEN - 1);
            pick->in_fp = fdopen(STDIN_FILENO, "rb");
            pick->f_in_pipe = true;
        }
    }
    if (!pick->f_in_pipe) {
        /** No provider_cmd specified, so read pick input from file or stdin */
        if (lstat(pick->in_spec, &sb) == -1) {
            strnz__cpy(tmp_str, "Can\'t stat pick input file: ", MAXLEN - 1);
            strnz__cat(tmp_str, pick->in_spec, MAXLEN - 1);
            Perror(tmp_str);
            return (1);
        }
        if (sb.st_size == 0) {
            strnz__cpy(tmp_str, "Pick input file empty: ", MAXLEN - 1);
            strnz__cat(tmp_str, pick->in_spec, MAXLEN - 1);
            Perror(tmp_str);
            return (1);
        }
        if ((pick->in_fp = fopen(pick->in_spec, "rb")) == nullptr) {
            strnz__cpy(tmp_str, "Can't open pick input file: ", MAXLEN - 1);
            strnz__cat(tmp_str, pick->in_spec, MAXLEN - 1);
            Perror(tmp_str);
            return (1);
        }
    }
    /*------------------------------------------------------------*/
    if (pick->in_fp == nullptr) {
        Perror("No pick input available");
        return (1);
    }
    /*------------------------------------------------------------*/
    read_pick_input(init);
    if (pick->f_in_pipe && pid > 0) {
        //
        // Wait for provider_cmd child process to finish before proceeding bool
        // rc = waitpid_with_timeout(pid, wait_timeout);
        //
        //  if (rc == false) {
        //      Perror("Timeout waiting for provider_cmd child process to finish");
        //      kill(pid, SIGKILL);
        //      waitpid(pid, nullptr, 0);
        //  }
        waitpid(pid, nullptr, 0);
        close(pipe_fd[P_READ]);
        // dup2(sio->stdin_fd, STDIN_FILENO);
        restore_curses_tioctl();
        sig_prog_mode();
        keypad(pick->win, true);
    }
    if (pick->m_cnt == 0) {
        Perror("No pick objects available");
        return (1);
    }
    /** Enter pick_engine */
    pick->m_idx = 0;
    pick->d_idx = 0;
    while (pick->m_idx < pick->m_cnt)
        pick->d_object[pick->d_idx++] = pick->m_object[pick->m_idx++];
    pick->d_cnt = pick->d_idx;
    pick->chyron = new_chyron();
    set_chyron_key(pick->chyron, 1, "F1 Help", KEY_F(1));
    set_chyron_key(pick->chyron, 2, "F9 Cancel", KEY_F(9));
    set_chyron_key(pick->chyron, 3, "F10 Accept", KEY_F(10));
    set_chyron_key(pick->chyron, 4, "<v> View", 'v');
    set_chyron_key(pick->chyron, 5, "<q> Quit View", 'q');
    set_chyron_key(pick->chyron, 6, "<Sp> Process", ' ');
    set_chyron_key(pick->chyron, 7, "<Sp> Edit", ' ');
    set_chyron_key(pick->chyron, 9, "<Tab> Search", '\t');
    set_chyron_key(pick->chyron, 10, "<Tab> Select", '\t');
    set_chyron_key(pick->chyron, 11, "PgUp", KEY_PPAGE);
    set_chyron_key(pick->chyron, 12, "PgDn", KEY_NPAGE);
    set_chyron_key(pick->chyron, 13, "INS", KEY_IC);
    pick_std_chyron(pick);
    compile_chyron(pick->chyron);
    pick_engine(init);
    if (pick->p_view_files)
        destroy_pick_view(init);
    win_del();
    return 0;
}

/** @brief Destroys pick view, unmaps file buffer, and cleans up resources
 *  @ingroup pick_engine
    @param init Pointer to Init structure containing pick information
    @details If pick->p_view_files is true and view->buf is not nullptr, destroys
   line table, unmaps the file buffer, and sets view->buf to nullptr. Destroys
   view window and view structure to free associated resources.
 */
void destroy_pick_view(Init *init) {
    stdio_fdnames(stdio_names_str, "pick_engine.c 279");
    Pick *pick = init->pick;
    View *view = init->view;
    if (pick->p_view_files) {
        if (view->buf != nullptr) {
            destroy_line_table(view);
            munmap(view->buf, view->file_size);
            view->buf = nullptr;
        }
    }
    destroy_view_win(init);
    destroy_view(init);
}
/** @brief Reads pick input from file pointer and saves objects into pick
 structure
 *  @ingroup pick_engine
    @param init Pointer to Init structure containing pick information
    @return 0 on success, -1 if no objects were read
    @details Reads lines from pick->in_fp and saves them as objects in the pick
   structure using save_object function. If no objects are read, returns -1.
   Otherwise, sets obj_cnt to the number of objects read and resets obj_idx to 0
   before returning 0. */
int read_pick_input(Init *init) {
    int i;

    Pick *pick = init->pick;
    pick->select_cnt = 0;
    pick->tbl_pages = 1;

    if (pick->in_fp) {
        while (fgets(pick->in_buf, sizeof(pick->in_buf), pick->in_fp) != nullptr)
            save_object(pick, pick->in_buf);

    } else
        for (i = 1; i < pick->argc; i++)
            save_object(pick, pick->argv[i]);
    if (pick->in_fp != nullptr)
        fclose(pick->in_fp);
    if (!pick->m_idx)
        return (-1);
    pick->m_cnt = pick->m_idx;
    return 0;
}
/** @brief Initializes pick interface, calculates window size and position, and
   enters picker loop
 *  @ingroup pick_engine
    @param init Pointer to Init structure containing pick information
    @return Count of selected objects on success, -1 if user cancels
    @details Initializes key command strings for chyron display and calculates
 pick window size and position based on terminal size and pick parameters. Opens
 pick window and displays first page of objects. Enters picker loop to handle
 user input and interactions. If user cancels selection, returns -1. If user
 accepts selection, returns count of selected objects. */
int pick_engine(Init *init) {
    int rc;
    int maxy, maxx;
    int whitespace_ratio = 15;
    int usable_lines;
    int pick_ratio = 50;
    int tbl_max_cols, pg_max_objs;
    bool f_processed = false;

    Pick *pick = init->pick;
    getmaxyx(stdscr, maxy, maxx);
    // Screen Geometry
    // Calculate pick window size and position based on terminal size and pick
    // parameters
    //
    // Calculate line usage:
    if (pick->begy == 0)
        pick->begy = (maxy * whitespace_ratio) / 200;
    usable_lines = maxy - (maxy * whitespace_ratio) / 100;
    usable_lines -= pick->begy;
    usable_lines -= 5; // Pick overhead lines
    if (pick->p_view_files) {
        if (pick->lines == 0) {
            pick->lines = usable_lines * pick_ratio / 100;
            usable_lines -= pick->lines;
        }
        init->begy = pick->begy + 5 + pick->lines;
        init->begx = 0;
        usable_lines -= 3; // View overhead lines
        init->lines = usable_lines;
        init->cols = maxx - 2;
    } else if (pick->lines == 0)
        pick->lines = usable_lines;

    pick->tbl_col_width = max(pick->tbl_col_width, 4);
    pick->tbl_col_width = min(pick->tbl_col_width, (maxx - (2 + pick->begx)));
    if (pick->d_cnt <= pick->lines) {
        pick->tbl_lines = pick->d_cnt;
        pick->lines = pick->d_cnt;
        pick->tbl_cols = 1;
    } else {
        tbl_max_cols = ((maxx - (2 + pick->begx)) / (pick->tbl_col_width + 1));
        pg_max_objs = pick->lines * tbl_max_cols;
        if (pick->d_cnt > pg_max_objs)
            pick->tbl_cols = tbl_max_cols;
        else
            pick->tbl_cols = pick->d_cnt / pick->lines;
        pick->tbl_lines = pick->d_cnt / tbl_max_cols;
    }
    pick->tbl_pages = (pick->tbl_lines / (pick->lines - 1)) + 1;
    // pick->lines = (pick->tbl_lines + pick->tbl_pages - 1) / pick->tbl_pages;
    pick->tbl_page = 0;
    // if (pick->begy == 0)
    //     pick->begy = (LINES - pick->lines) / 5;
    // else if (pick->begy + pick->lines > LINES - 4)
    //     pick->begy = LINES - pick->lines - 2;
    pick->width = (pick->tbl_col_width + 1) * pick->tbl_cols;

    // The following lines will accomodate the longest chyron line
    pick->chyron->key[11]->active = true;
    pick->chyron->key[12]->active = true;
    compile_chyron(pick->chyron);
    pick->width = max(pick->width, pick->chyron->l);
    pick_std_chyron(pick);
    rc = open_pick_win(init);
    if (rc) {
        Perror("Failed to open pick window");
        exit(EXIT_FAILURE);
        // return (rc);
    }
    /** Enter picker loop to handle user input and interactions */
    pick->d_idx = 0;
    pick->x = 1;
    char field[MAXLEN]; /**< Buffer for user input in the field */
    field[0] = '\0';
    display_pick_page(pick);

    do {
        rc = picker(init, field);
        if (rc == KEY_F(9))
            break;
        if (rc == -1)
            break;
        else {
            if (pick->select_cnt > 0) {
                if (pick->f_out_spec && pick->out_spec[0]) {
                    output_objects(pick);
                    f_processed = true;
                }
                if (pick->f_cmd && pick->cmd[0]) {
                    exec_objects(init);
                    f_processed = true;
                }
                if (pick->f_read_theme) {
                    read_theme(init);
                    f_processed = true;
                }
                if (f_processed) {
                    mvwaddstr(pick->win2, 0, 0, "Selection Processed");
                    wclrtoeol(pick->win2);
                }
            }
        }
        deselect_object(pick);
    } while (1);
    destroy_chyron(pick->chyron);
    return (rc);
}
/** @brief Sets standard chyron key states based on pick structure
 *  @ingroup pick_engine
    @param pick Pointer to Pick structure containing chyron and selection
   information
    @details Updates the active state of chyron keys based on the current state
   of the pick structure. Enables or disables keys for help, cancel, accept,
   view, quit view, process, toggle, search, select, page up, page down, and
   insert based on selection count, view mode, and table pages.
 */
void pick_std_chyron(Pick *pick) {
    // *   1  F1 Help         KEY_F(1));
    // *   2  F9 Cancel       KEY_F(9));
    // *   3  F10 Accept      KEY_F(10));
    // ?   4  <v> View        <v>
    // ?   5  <q> Quit View   <q>
    // ?   6  <Sp> Toggle     <Sp>
    // ?   7  <Sp> Edit       <Sp>
    // ?   9  <Tab> Search    <Tab>
    // ?  10  <Tab> Select"   <Tab>
    // ?  11  PgUp            KEY_PPAGE
    // ?  12  PgDn            KEY_NPAGE
    // ?  13  INS             KEY_IC
    pick->chyron->key[1]->active = true;                                                 // F1 Help
    pick->chyron->key[2]->active = true;                                                 // F9 Cancel
    pick->chyron->key[3]->active = pick->select_max != 1 ? true : false;                 // F10 Accept
    pick->chyron->key[4]->active = pick->p_view_files;                                   // <v> View
    pick->chyron->key[5]->active = false;                                                // <q> Quit View
    pick->chyron->key[6]->active = pick->select_max == 1 ? false : true;                 // <Sp> Toggle
    pick->chyron->key[7]->active = pick->select_max == 1 ? true : false;                 // <Sp> Process
    pick->chyron->key[9]->active = true;                                                 // <Tab> Search;
    pick->chyron->key[10]->active = false;                                               // <Tab> Select;
    pick->chyron->key[11]->active = pick->tbl_page > 0 ? true : false;                   // PgUp
    pick->chyron->key[12]->active = pick->tbl_page < pick->tbl_pages - 1 ? true : false; // PgDn
    pick->chyron->key[13]->active = false;                                               // INS
}
/** @brief Saves a string as an object in the pick structure
 *  @ingroup pick_engine
    @param pick Pointer to Pick structure
    @param s String to save as an object
    @details If the current object index is less than the maximum allowed, saves
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
    @details Clears the pick window and displays the current page of objects
 based on the current table page, line, and column. Marks selected objects with
 an asterisk. Updates the chyron with page information at the bottom of the pick
   window. */
void display_pick_page(Pick *pick) {
    int col;
    for (pick->y = 0; pick->y < pick->lines; pick->y++) {
        wmove(pick->win, pick->y, 0);
        wclrtoeol(pick->win);
    }
    pick->d_idx = pick->tbl_page * pick->lines * pick->tbl_cols;
    for (col = 0; col < pick->tbl_cols; col++) {
        pick->x = col * (pick->tbl_col_width + 1) + 1;
        pick->y = 0;
        while (pick->d_idx < pick->d_cnt && pick->y < pick->lines) {
            if (pick->f_selected[pick->d_idx])
                mvwaddstr(pick->win, pick->y, pick->x - 1, "*");
            mvwaddstr_fill(pick->win, pick->y++, pick->x,
                           pick->d_object[pick->d_idx++], pick->tbl_col_width - 1);
        }
    }
    pick->d_idx -= 1;
    pick->tbl_lines = pick->d_cnt;
    pick->tbl_pages = ((pick->tbl_lines + pick->lines - 1) / pick->lines);
    if (pick->y < pick->lines) {
        pick->y_offset = pick->lines - pick->y;
        wscrl(pick->win, -pick->y_offset);
    } else
        pick->y_offset = 0;
}
/** @brief Displays current page of objects in pick window
    @ingroup pick_engine
    @param pick Pointer to Pick structure containing objects and display
   information
    @param s String to filter objects by
    @details Clears the pick window and displays the current page of objects
   based on the current table page, line, and column. Marks selected objects
   with an asterisk. Updates the chyron with page information at the bottom of
   the pick window. */
int match_objects(Pick *pick, char *s) {
    /** pick->m_idx  Master  (as read from input) */
    /** pick->d_idx  Display (to display) */
    pick->m_idx = 0;
    pick->d_idx = 0;
    while (pick->m_idx < pick->m_cnt) {
        if (s == nullptr || s[0] == '\0' || strstr(pick->m_object[pick->m_idx], s) != nullptr) {
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
    @details Calculates the x coordinate for the currently selected object based
 on the current table column and column width. Moves the cursor to the object's
   position in the pick window, turns on reverse video attribute, and displays
   the object's text. Turns off reverse video attribute and refreshes the pick
   window to show the updated display. Moves the cursor back to the position
   before the object text for potential further interactions. */
void reverse_object(Pick *pick) {
    if (pick->d_idx >= pick->d_cnt)
        pick->d_idx = pick->d_cnt - 1;
    pick->x = pick->tbl_col * (pick->tbl_col_width + 1) + 1;
    pick->tbl_line = (pick->d_idx / pick->tbl_cols) % pick->lines;
    pick->y = pick->tbl_line + pick->y_offset;
    pick->d_idx = pick->tbl_page * pick->lines * pick->tbl_cols + pick->tbl_col * pick->lines + pick->tbl_line;
    wbkgrndset(pick->win, &CC_NT_REV);
    wmove(pick->win, pick->y, pick->x);
    mvwaddstr_fill(pick->win, pick->y, pick->x, pick->d_object[pick->d_idx],
                   pick->tbl_col_width - 1);
    wmove(pick->win, pick->y, pick->x - 1);
    if (pick->f_selected[pick->d_idx])
        mvwadd_wchnstr(pick->win, pick->y, 0, &chk, 1);
    else
        mvwadd_wchnstr(pick->win, pick->y, 0, &ran, 1); // space
    wbkgrndset(pick->win, &CC_NT);
}
/** @brief Unreverses the display of the currently selected object in pick
   window
   @ingroup pick_engine
    @param pick Pointer to Pick structure containing object and display
   information
    @details Calculates the x coordinate for the currently selected object based
   on the current table column and column width. Moves the cursor to the
   object's position in the pick window and displays the object's text without
   reverse video attribute. Refreshes the pick window to show the updated
   display. Moves the cursor back to the position before the object text for
   potential further interactions. */
void unreverse_object(Pick *pick) {
    if (pick->d_idx >= pick->d_cnt)
        pick->d_idx = pick->d_cnt - 1;
    pick->x = pick->tbl_col * (pick->tbl_col_width + 1) + 1;
    pick->tbl_line = (pick->d_idx / pick->tbl_cols) % pick->lines;
    pick->y = pick->tbl_line + pick->y_offset;
    pick->d_idx = pick->tbl_page * pick->lines * pick->tbl_cols + pick->tbl_col * pick->lines + pick->tbl_line;
    wbkgrndset(pick->win, &CC_NT);
    mvwaddstr_fill(pick->win, pick->y, pick->x, pick->d_object[pick->d_idx],
                   pick->tbl_col_width - 1);
    if (pick->f_selected[pick->d_idx])
        mvwadd_wchnstr(pick->win, pick->y, 0, &chk, 1);
    else
        mvwadd_wchnstr(pick->win, pick->y, 0, &sp, 1); // space
    wmove(pick->win, pick->y, 0);
}
void remove_right_angle(Pick *pick) {
    if (pick->d_idx >= pick->d_cnt)
        pick->d_idx = pick->d_cnt - 1;
    pick->tbl_line = (pick->d_idx / pick->tbl_cols) % pick->lines;
    pick->y = pick->tbl_line + pick->y_offset;
    pick->d_idx = pick->tbl_page * pick->lines * pick->tbl_cols + pick->tbl_col * pick->lines + pick->tbl_line;
    if (pick->f_selected[pick->d_idx])
        mvwadd_wchnstr(pick->win, pick->y, 0, &chk, 1);
    else
        mvwadd_wchnstr(pick->win, pick->y, 0, &sp, 1); // space
    wmove(pick->win, pick->y, 0);
}
/** @brief Toggles the selection state of the currently selected object in pick
   window
   @ingroup pick_engine
    @param pick Pointer to Pick structure containing object and selection
   information
    @details Calculates the x coordinate for the currently selected object based
   on the current table column and column width. If the object is currently
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
        mvwadd_wchnstr(pick->win, pick->y, 0, &sp, 1); // space
    } else {
        pick->select_cnt++;
        pick->f_selected[pick->d_idx] = true;
        mvwadd_wchnstr(pick->win, pick->y, 0, &chk, 1);
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
        mvwadd_wchnstr(pick->win, pick->y, 0, &sp, 1); // space
    }
}
int read_theme(Init *init) {
    int rc;
    expand_tilde(init->mapp_theme, MAXLEN - 1);
    rc = process_config_file(init->mapp_theme, init);
    if (rc)
        return rc;
    SIO *sio = init->sio;
    initialize_local_colors(sio);
    update_panels();
    doupdate();
    return 0;
}

/** @brief Outputs selected objects to specified output file
   @ingroup pick_engine
    @param pick Pointer to Pick structure containing selected objects and
   output file information
    @return 0 on success, 1 on failure
    @details If output file cannot be opened, an error message is printed and
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
   @details Parses command string and appends selected objects as arguments to
   the command. If command contains "%%", it is replaced with a space- separated
   list of selected objects. Executes the command using execvp in a child
   process and waits for it to finish. If the command is a pager or editor, it
   is executed within the pick interface using popup_view instead of execvp.
   If f_append_objects is true, the argument containing %% is replaced with
   the concatenated selected objects. If f_append_objects is false, selected
   objects are added as separate arguments and the original command arguments
   remain unchanged.
   eargv should be null-terminated to indicate the end of arguments for
   execvp
   Memory allocated for arguments is freed after execution to prevent
   memory leaks.
   If execvp fails, an error message is printed and the child process
   exits with failure status
   The parent process waits for the child process to finish before
   proceeding and restores terminal settings
   If the command is a pager or editor, it is executed within the pick
   interface using popup_view instead of execvp
   The base name of the command is extracted to check if it is a pager or
   editor
   If the command is a pager or editor, the pick interface is used to
   display the command output instead of executing it in a separate terminal
   This allows the user to view the command output without leaving the pick
   interface and provides a more seamless user experience.
   If the command is not a pager or editor, it is executed in a separate
   terminal and the pick interface is restored after execution
   If the command to be executed is view, an external command is not
   needed, instead the popup_view function can be used to display the output
   within the pick interface */
int exec_objects(Init *init) {
    int rc = -1;
    int eargc;
    char *eargv[MAXARGS];
    char tmp_str[MAXLEN] = {'\0'};
    char title[MAXLEN];
    char sav_arg[MAXLEN];
    char *out_s;
    int eargx = 0;
    int i = 0;
    pid_t pid = 0;
    bool f_append_objects = false;

    Pick *pick = init->pick;
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
        f_append_objects = false;
        i = 0;
        while (i < eargc) {
            /** This is the line that gets the selected objects */
            if (strstr(eargv[i], "%%") != nullptr) {
                tmp_str[0] = '\0';
                f_append_objects = true;
                strnz__cpy(sav_arg, eargv[i], MAXLEN - 1);
                eargx = i;
                break;
            }
            i++;
        }
        for (i = 0; i < pick->d_cnt; i++) {
            /** append arguments onto tmp_str */
            if (pick->f_selected[i] && eargc < MAXARGS - 1) {
                if (f_append_objects == true) {
                    if (tmp_str[0] != '\0')
                        strnz__cat(tmp_str, " ", MAXLEN - 1);
                    strnz__cat(tmp_str, pick->d_object[i], MAXLEN - 1);
                    continue;
                }
                eargv[eargc++] = strdup(pick->d_object[i]);
            }
        }
        if (f_append_objects == true) {
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
    char *sav_ptr;
    char *tok;
    tok = strtok_r(tmp_str, " ", &sav_ptr);
    strnz__cpy(sav_arg, tok, MAXLEN - 1);
    base_name(tmp_str, sav_arg);
    if (tmp_str[0] != '\0' && (strcmp(tmp_str, "view") == 0 || strcmp(tmp_str, "view") == 0)) {
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
        werase(stdscr);
        endwin();
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
        } else if (pid == 0) {
            /** Prevent child process from writing to terminal */
            // int dev_null = open("/dev/null", O_WRONLY);
            // if (dev_null == -1) {
            //     Perror("open(/dev/null) failed in init_pick child process");
            //     exit(EXIT_FAILURE);
            // }
            // dup2(dev_null, STDERR_FILENO);
            // close(dev_null);
            execvp(eargv[0], eargv);
            exit(EXIT_FAILURE);
        }
    }
    waitpid(pid, nullptr, 0);
    destroy_argv(eargc, eargv);
    reset_prog_mode();
    restore_wins();
    return rc;
}
/** @brief Initializes the pick window based on the parameters specified in the
Pick structure
   @ingroup pick_engine
   @param init Pointer to Init structure containing pick information
   @return 0 on success, 1 on failure
   @details Creates a new window for the pick interface using win_new function
with the specified parameters from the Pick structure. If window creation fails,
an error message is printed and the function returns 1. Otherwise, initializes
the window and box pointers in the Pick structure, sets scrollok and keypad
options for the window, and returns 0 on success. */
int open_pick_win(Init *init) {
    char tmp_str[MAXLEN];
    Pick *pick = init->pick;
    pick = init->pick;
    int split_win_lines = 2; // 1 text, 1 chyron
    if (box_hsplit_new(pick->lines, split_win_lines, pick->width, pick->begy, pick->begx,
                       pick->title)) {
        ssnprintf(tmp_str, MAXLEN - 1, "box_hsplit_new(%d, %d, %d, %d, %d, %s) failed",
                  pick->lines, split_win_lines, pick->width, pick->begy, pick->begx,
                  pick->title);
        Perror(tmp_str);
        return (1);
    }
    pick->separator_line = pick->lines + 1;
    pick->win = win_win[win_ptr];
    pick->win2 = win_win2[win_ptr];
    pick->box = win_box[win_ptr];
    keypad(pick->win, true);
    if (pick->p_view_files)
        new_pick_view(init);
    return 0;
}
/** @brief Displays the help screen for the pick interface using view
   @ingroup pick_engine
    @param init Pointer to Init structure containing pick information
    @details Initializes the help_spec field in the Pick structure with the
   path to the pick help file. Then, constructs the argument list for
   executing popup_view with the help file as an argument. Finally, calls
   popup_view function to display the help screen within the pick interface. */
void display_pick_help(Init *init) {
    int eargc;
    char *eargv[MAXARGS];
    char tmp_str[MAXLEN];
    Pick *pick = init->pick;
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
    eargv[eargc] = nullptr;
    init->lines = 30;
    init->cols = 76;
    init->begy = pick->begy + 1;
    init->begx = pick->begx + 1;
    strnz__cpy(init->title, "Pick Help", MAXLEN - 1);
    popup_view(init, eargc, eargv, init->lines, init->cols, init->begy,
               init->begx);
    destroy_argv(eargc, eargv);
    return;
}
/** @brief Main loop for handling user input and interactions in the pick
   interface
   @ingroup pick_engine
    @param init Pointer to Init structure containing pick information
    @param field Buffer for user input in the field
    @return Count of selected objects on success, -1 if user cancels
    @details The first loop handles navigation through the pick table.
    The second loop handles user input for selecting/deselecting objects,
   accepting the selection, or canceling the selection. Depending on the key
   pressed, the appropriate action is taken, such as toggling selection, moving
   to the next/previous object, or displaying the help screen. If the user
   accepts the selection, the count of selected objects is returned. If the user
   cancels the selection, -1 is returned.
    */
int picker(Init *init, char *field) {
    bool f_insert = false; /* Flag to indicate if insert mode is active */
    Pick *pick = init->pick;
    int col = 0;
    int flen = pick->width - 4;
    char *accept_s = field; /* pointer to start of field buffer */
    char *ptr = field;      /* pointer to current cursor position in field buffer */
    char *s = field;        /* source pointer for editing operations */
    char *d = field;        /* destination pointer for editing operations */
    char *fend = field + flen;
    char *str_end = field + strlen(field); /* End of field content */
    int pos = 0;
    int prev_pos = 0;
    char prev_field[MAXLEN];
    char *prev_ptr = prev_field;
    char view_file[MAXLEN] = {'\0'};
    pick = init->pick;

    ptr = accept_s;
    ptr = str_end;
    click_x = -1;
    click_y = click_x = -1;
    char tmp_str[MAXLEN];

    mousemask(BUTTON1_CLICKED | BUTTON1_DOUBLE_CLICKED, nullptr);

    f_insert = false;

    keypad(pick->win, true);

    int in_key = 0;
    while (1) {
        while (1) {
            /** ===========================================================
                Pick Objects Loop
                =========================================================== */
            if (in_key == 0) {
                reverse_object(pick);
                pick->tbl_line = (pick->d_idx / pick->tbl_cols) % pick->lines;
                pick->y = pick->tbl_line + pick->y_offset;
                /** box display_pick_page_info */
                ssnprintf(tmp_str, MAXLEN - 1, "Line %d, Page %d/%d",
                          pick->tbl_line + 1, pick->tbl_page + 1,
                          pick->tbl_pages);
                border_hsplit_text(pick->box, tmp_str, pick->separator_line);
                if (pick->p_view_files)
                    if (strcmp(pick->d_object[pick->d_idx], view_file) != 0) {
                        strnz__cpy(view_file, pick->d_object[pick->d_idx], MAXLEN - 1);
                        new_view_file(init, view_file);
                    }
                mouse_win = nullptr;
                // 1
                // top_panel(panel_win[win_ptr]);
                pick_std_chyron(pick);
                compile_chyron(pick->chyron);
                display_chyron(pick->win2, pick->chyron, 1, pick->chyron->l);
                reverse_object(pick);
                update_panels();
                wmove(pick->win, pick->y, pick->x);
                doupdate();
                in_key = dxwgetch(pick->win, pick->win, pick->win2, nullptr, pick->win2, pick->chyron, -1);
                if (pick->f_selected[pick->d_idx])
                    mvwadd_wchnstr(pick->win, pick->y, 0, &chk, 1);
                else
                    mvwadd_wchnstr(pick->win, pick->y, 0, &sp, 1); // space
                if (mouse_win == pick->win2 && click_y == 0)
                    break;
            }
            switch (in_key) {
            case KEY_F(1):
                display_pick_help(init);
                display_pick_page(pick);
                f_insert = false;

                cchar_t cc = {0};
                wchar_t wstr[2] = {BW_RAN, L'\0'};
                setcchar(&cc, wstr, WA_NORMAL, cp_box, nullptr);
                mvwadd_wch(pick->win2, 0, 0, &cc);
                in_key = 0;
                continue;

            case 'v':
                if (!pick->p_view_files) {
                    in_key = 0;
                    continue;
                }
                // *   1  F1 Help         KEY_F(1));
                // *   2  F9 Cancel       KEY_F(9));
                // *   3  F10 Accept      KEY_F(10));
                // ?   4  <v> View        <v>
                // ?   5  <q> Quit View   <q>
                // ?   6  <Sp> Process    <Sp>
                // ?   7  <Sp> Toggle     <Sp>
                // ?   9  <Tab> Search    <Tab>
                // ?  10  <Tab> Select"   <Tab>
                // ?  11  PgUp            KEY_PPAGE
                // ?  12  PgDn            KEY_NPAGE
                // ?  13  INS             KEY_IC
                pick->chyron->key[1]->active = true;   // F1 Help
                pick->chyron->key[2]->active = true;   // F9 Cancel
                pick->chyron->key[4]->active = false;  // <v> View
                pick->chyron->key[5]->active = true;   // <q> Quit View
                pick->chyron->key[6]->active = false;  // <Sp> Toggle
                pick->chyron->key[7]->active = false;  // <Sp> Edit
                pick->chyron->key[9]->active = false;  // <Tab> Search;
                pick->chyron->key[10]->active = false; // <Tab> Select;
                pick->chyron->key[11]->active = true;  // PgUp
                pick->chyron->key[12]->active = true;  // PgDn
                pick->chyron->key[13]->active = false; // INS
                remove_right_angle(pick);
                compile_chyron(pick->chyron);
                display_chyron(pick->win2, pick->chyron, 1, pick->chyron->l);
                update_panels();
                doupdate();
                if (pick->p_view_files)
                    view_cmd_processor(init);
                pick_std_chyron(pick);
                compile_chyron(pick->chyron);
                display_chyron(pick->win2, pick->chyron, 1, pick->chyron->l);
                in_key = 0;
                continue;

            case 't':
            case ' ':
                reverse_object(pick);
                toggle_object(pick);
                if (pick->select_max > 0 && pick->select_cnt == pick->select_max)
                    return pick->select_cnt;
                in_key = 0;
                continue;

            /** 'q', or KEY_F(9) cancel selection and exit picker */
            case 'q':
            case KEY_F(9):
                deselect_object(pick);
                return -1;

            /** Enter or KEY_F(10) Accepts current selection and exits
               picker, returning count of selected objects */
            case KEY_F(10):
            case '\n':
            case KEY_ENTER:
                reverse_object(pick);
                return pick->select_cnt;

            /** KEY_END Moves selection to last object in list */
            case KEY_END:
                mvwaddstr_fill(pick->win, pick->y, pick->x,
                               pick->d_object[pick->d_idx],
                               pick->tbl_col_width - 1);
                int display_tbl_page = pick->tbl_page;
                pick->d_idx = pick->d_cnt - 1;
                pick->tbl_page = pick->d_idx / (pick->lines * pick->tbl_cols);
                pick->tbl_line = (pick->d_idx / pick->tbl_cols) % pick->lines;
                pick->tbl_col = pick->d_idx % pick->tbl_cols;
                pick->y = pick->tbl_line;
                if (display_tbl_page != pick->tbl_page)
                    display_pick_page(pick);
                in_key = 0;
                continue;
            case '\t':
                remove_right_angle(pick);
                wnoutrefresh(pick->win);
                in_key = 0;
                break;

            /** 'h' or KEY_LEFT or Backspace Moves selection to previous
               object in list */
            case 'h':
            case KEY_LEFT:
            case KEY_BACKSPACE:
                if (pick->tbl_col == 0) {
                    in_key = 0;
                    continue;
                }
                unreverse_object(pick);
                mvwaddstr_fill(pick->win, pick->y, pick->x,
                               pick->d_object[pick->d_idx],
                               pick->tbl_col_width - 1);
                if (pick->tbl_col > 0)
                    pick->tbl_col--;
                pick->d_idx = pick->tbl_page * pick->lines * pick->tbl_cols + pick->tbl_col * pick->lines + pick->tbl_line;
                in_key = 0;
                reverse_object(pick);
                continue;

            /** 'j' or KEY_DOWN Moves selection to next object in list */
            case 'j':
            case KEY_DOWN:
                if (pick->tbl_line == pick->tbl_lines - 1)
                    break;
                mvwaddstr_fill(pick->win, pick->y, pick->x,
                               pick->d_object[pick->d_idx],
                               pick->tbl_col_width - 1);
                unreverse_object(pick);
                if (pick->tbl_page * pick->lines * pick->tbl_cols + pick->tbl_col * pick->lines + pick->tbl_line < pick->d_cnt - 1 && pick->tbl_line < pick->lines - 1)
                    pick->tbl_line++;
                pick->d_idx = pick->tbl_page * pick->lines * pick->tbl_cols + pick->tbl_col * pick->lines + pick->tbl_line;
                in_key = 0;
                reverse_object(pick);
                continue;

            /** 'k' or KEY_UP Moves selection to previous object in list */
            case 'k':
            case KEY_UP:
                mvwaddstr_fill(pick->win, pick->y, pick->x,
                               pick->d_object[pick->d_idx],
                               pick->tbl_col_width - 1);
                unreverse_object(pick);
                if (pick->tbl_line > 0)
                    pick->tbl_line--;
                pick->d_idx = pick->tbl_page * pick->lines * pick->tbl_cols + pick->tbl_col * pick->lines + pick->tbl_line;
                in_key = 0;
                reverse_object(pick);
                continue;

            /** 'l' or KEY_RIGHT Moves selection to next object in list */
            case 'l':
            case KEY_RIGHT:
                if (pick->tbl_col == pick->tbl_cols - 1) {
                    in_key = 0;
                    continue;
                }
                mvwaddstr_fill(pick->win, pick->y, pick->x,
                               pick->d_object[pick->d_idx],
                               pick->tbl_col_width - 1);
                unreverse_object(pick);
                /** pick->obj_idx += pick->tbl_lines -> next column */
                if (pick->tbl_page * pick->lines * pick->tbl_cols + (pick->tbl_col + 1) * pick->lines + pick->tbl_line < pick->d_cnt - 1 && pick->tbl_col < pick->tbl_cols - 1)
                    pick->tbl_col++;
                pick->d_idx = pick->tbl_page * pick->lines * pick->tbl_cols + pick->tbl_col * pick->lines + pick->tbl_line;
                in_key = 0;
                reverse_object(pick);
                continue;

            /** KEY_NPAGE or 'Ctrl+f' Moves selection to next page of
               objects */
            case KEY_NPAGE:
            case '\06':
                if (pick->tbl_pages == 1) {
                    in_key = 0;
                    continue;
                }
                if (pick->tbl_page < pick->tbl_pages - 1) {
                    pick->tbl_page++;
                    pick->pg_line = 0;
                    pick->tbl_col = 0;
                }
                pick->d_idx = pick->tbl_page * pick->lines * pick->tbl_cols + pick->tbl_cols * pick->pg_line + pick->tbl_col;
                display_pick_page(pick);
                in_key = 0;
                continue;

            /**   KEY_PPAGE or 'Ctrl+b' Moves selection to previous page of
               objects */
            case KEY_PPAGE:
            case '\02':
                if (pick->tbl_pages == 1) {
                    in_key = 0;
                    continue;
                }
                if (pick->tbl_page > 0)
                    pick->tbl_page--;
                pick->d_idx = pick->tbl_page * pick->lines * pick->tbl_cols + pick->tbl_cols * pick->pg_line + pick->tbl_col;
                display_pick_page(pick);
                in_key = 0;
                continue;

            /** KEY_HOME Moves selection to first object in list */
            case KEY_HOME:
                pick->tbl_page = 0;
                pick->tbl_line = 0;
                pick->tbl_col = 0;
                pick->d_idx = pick->tbl_page * pick->lines * pick->tbl_cols + pick->tbl_cols * pick->pg_line + pick->tbl_col;
                display_pick_page(pick);
                in_key = 0;
                continue;

            /** KEY_LL (lower left of numeric pad) Moves selection to last
                object in list */
            case KEY_LL:
                pick->tbl_page = pick->tbl_pages - 1;
                pick->d_idx = pick->tbl_page * pick->lines * pick->tbl_cols + pick->tbl_cols * pick->pg_line + pick->tbl_col;
                display_pick_page(pick);
                in_key = 0;
                continue;

                /** KEY_MOUSE Handles mouse events for selection and chyron
                 * key activation */

            case KEY_MOUSE:
                if (click_y == -1 || click_x == -1)
                    continue;
                if (click_y < pick->y_offset) {
                    in_key = 0;
                    continue;
                }
                unreverse_object(pick);
                pick->y = click_y;
                pick->tbl_col = (click_x - 1) / (pick->tbl_col_width + 1);
                if (pick->tbl_col < 0 || pick->tbl_col >= pick->tbl_cols)
                    continue;
                pick->d_idx = pick->tbl_page * pick->lines * pick->tbl_cols + pick->tbl_col * pick->lines + pick->y;
                in_key = KEY_F(13); /** toggle selection on mouse click */
                click_y = click_x = -1;
                continue;

            default:
                in_key = 0;
                continue;
            }
            in_key = 0;
            break;
        }
        /** ===============================================================
            Line editor loop
            =============================================================== */
        pick->chyron->key[1]->active = true;   // F1 Help
        pick->chyron->key[2]->active = true;   // F9 Cancel
        pick->chyron->key[3]->active = false;  // F10 Accept
        pick->chyron->key[4]->active = false;  // <v> View
        pick->chyron->key[5]->active = false;  // <q> Quit View
        pick->chyron->key[6]->active = false;  // <Sp> Toggle
        pick->chyron->key[7]->active = false;  // <Sp> Edit
        pick->chyron->key[9]->active = false;  // <Tab> Search
        pick->chyron->key[10]->active = true;  // <Tab> Select
        pick->chyron->key[11]->active = false; // PgDn
        pick->chyron->key[12]->active = false; // PgUp
        pick->chyron->key[13]->active = true;  // INS

        while (1) {

            if (in_key == 0) {
                mouse_win = nullptr;
                if (accept_s != nullptr && accept_s[0] != '\0') {
                    if (match_objects(pick, accept_s) == 0) {
                        strnz__cpy(field, prev_field, MAXLEN - 1);
                        pos = prev_pos;
                        ptr = prev_ptr;
                    } else {
                        display_pick_page(pick);
                        ssnprintf(tmp_str, MAXLEN - 1, "Line %d, Page %d/%d",
                                  pick->tbl_line + 1, pick->tbl_page + 1,
                                  pick->tbl_pages);
                        strnz__cat(tmp_str, "     ", MAXLEN - 1);
                        tmp_str[21] = '\0';
                        mvwaddstr(pick->box, pick->separator_line, 3, tmp_str);
                    }
                }
                compile_chyron(pick->chyron);
                display_chyron(pick->win2, pick->chyron, 1, pick->chyron->l);
                wmove(pick->win2, 0, 1);
                /** display_field_content */
                rtrim(accept_s);
                col = 1;
                mvwaddstr(pick->win2, 0, col, accept_s);
                wclrtoeol(pick->win2);
                if (pick->p_view_files)
                    if (strcmp(pick->d_object[pick->d_idx], view_file) != 0) {
                        strnz__cpy(view_file, pick->d_object[pick->d_idx], MAXLEN - 1);
                        new_view_file(init, view_file);
                    }
                mouse_win = nullptr;
                pos = col + strlen(accept_s);
                // 2
                // top_panel(panel_win2[win_ptr]);
                mvwadd_wchnstr(pick->win2, 0, 0, &ran, 1);
                update_panels();
                wmove(pick->win2, 0, pos);
                doupdate();
                in_key = dxwgetch(pick->win2, pick->win, pick->win2, nullptr, pick->win2, pick->chyron, -1);
                if (mouse_win == pick->win)
                    break;
                if (in_key == KEY_F(13)) {
                    in_key = 0;
                    continue;
                }
            }
            strnz__cpy(prev_field, accept_s, MAXLEN - 1);
            prev_pos = pos;
            prev_ptr = ptr;
            switch (in_key) {
            case '\n':
            case KEY_ENTER:
                in_key = 0;
                break;

            case KEY_BTAB:
            case KEY_UP:
            case '\t':
                in_key = 0;
                break;

            case KEY_F(1):
                return (in_key);

            case KEY_F(2):
                if (pick->p_view_files)
                    view_cmd_processor(init);
                in_key = 0;
                continue;

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
                pos = col + (ptr - accept_s);
                in_key = 0;
                continue;

            case KEY_IC:
                if (f_insert) {
                    f_insert = FALSE;
                    set_chyron_key_cp(pick->chyron, 12, "INS", KEY_IC,
                                      cp_nt_rev);
                } else {
                    f_insert = TRUE;
                    set_chyron_key_cp(pick->chyron, 12, "INS", KEY_IC,
                                      cp_nt_hl_rev);
                }
                compile_chyron(pick->chyron);
                display_chyron(pick->win2, pick->chyron, 1, pick->chyron->l);
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
                ptr = accept_s;
                pos = col;
                in_key = 0;
                continue;

            /** KEY_BACKSPACE deletes character before cursor */
            case KEY_BACKSPACE:
                if (ptr > accept_s) {
                    ptr--;
                    pos--;
                } else {
                    in_key = 0;
                    continue;
                }
                s = ptr + 1;
                d = ptr;
                while (*s != '\0')
                    *d++ = *s++;
                *d = '\0';
                str_end = d;
                if (ptr == accept_s) {
                    match_objects(pick, accept_s);
                    display_pick_page(pick);
                    ssnprintf(tmp_str, MAXLEN - 1, "Line %d, Page %d/%d",
                              pick->tbl_line + 1, pick->tbl_page + 1,
                              pick->tbl_pages);
                    strnz__cat(tmp_str, "     ", MAXLEN - 1);
                    tmp_str[21] = '\0';
                    mvwaddstr(pick->box, pick->separator_line, 3, tmp_str);
                }
                in_key = 0;
                continue;

            /** KEY_LEFT moves cursor left one character */
            case KEY_LEFT:
                if (ptr > accept_s) {
                    ptr--;
                    pos--;
                }
                in_key = 0;
                continue;

            /** KEY_RIGHT moves cursor right one character */
            case KEY_RIGHT:
                if (ptr < fend && ptr <= str_end) {
                    ptr++;
                    pos++;
                }
                in_key = 0;
                continue;

            /** Handles mouse events for field editing */
            case KEY_MOUSE:
                if (click_x < col || click_x >= col + flen) {
                    in_key = 0;
                    continue;
                }
                pos = click_x;
                fend = accept_s + flen;
                str_end = accept_s + strlen(accept_s);
                ptr = accept_s + (pos - col);
                ptr = min(ptr, str_end);
                pos = col + (ptr - accept_s);
                click_x = -1;
                in_key = 0;
                continue;

            default:
                if (in_key < ' ' || in_key > '~') {
                    in_key = 0;
                    continue;
                }
                if (ptr >= fend) {
                    in_key = 0;
                    continue;
                }
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
            break;
        }
        mvwaddnwstr(pick->box, pick->separator_line + 1, 1, &bw_sp, 1);
    }
}
/** @brief Initializes a new view for displaying file contents in the pick
   interface
   @ingroup pick_engine
    @param init Pointer to Init structure containing pick information
    @return 0 on success, 1 on failure
    @details Allocates memory for a new View structure and initializes its
   fields based on the parameters specified in the Init structure. Sets up the
   help_spec field for the view, and calls init_view_boxwin to create the view's
   window. Returns 0 on success, or 1 if window creation fails.
    */
int new_pick_view(Init *init) {
    char *e;
    // if (init->view != nullptr)
    // view_stack_push(&view_stack, *init->view);
    init->view = nullptr;
    destroy_argv(init->argc, init->argv);
    View *view = init->view;
    view = new_view(init); // View struct allocation
    view->lines = init->lines;
    view->cols = init->cols;
    view->begy = init->begy;
    view->begx = init->begx;
    view->receiver_cmd[0] = '\0';
    view->f_ignore_case = init->f_ignore_case;
    view->f_at_end_remove = init->f_at_end_remove;
    view->f_squeeze = init->f_squeeze;
    view->tab_stop = init->tab_stop;
    view->f_ln = init->f_ln;
    view->h_shift = init->h_shift;
    e = getenv("VIEW_HELP_FILE");
    if (e && e[0] != '\0') {
        strnz__cpy(view->help_spec, e, MAXLEN - 1);
    }
    view->f_help_spec =
        verify_spec_arg(view->help_spec, view->help_spec, init->mapp_help,
                        "~/menuapp/help", R_OK);
    if (!view->f_help_spec) {
        strnz__cpy(view->help_spec, init->mapp_home, MAXLEN - 1);
        strnz__cat(view->help_spec, "/help/", MAXLEN - 1);
        strnz__cat(view->help_spec, VIEW_HELP_FILE, MAXLEN - 1);
    }
    int rc = init_view_boxwin(init);
    return rc;
}
/** @brief Initializes a new view for displaying the contents of a specified
   file in the pick interface
   @ingroup pick_engine
    @param init Pointer to Init structure containing pick information
    @param file Name of the file to be displayed in the view
    @details If a view is already initialized, it destroys the existing line
   table and unmaps the buffer. Then, it sets up the provider command for
   highlighting the specified file, initializes the view input, and if successful,
   resets various view parameters, initializes the line table, and displays the
   first page of the file contents along with the prompt.
    */
void new_view_file(Init *init, char *file) {
    View *view = init->view;
    if (view->buf != nullptr) {
        destroy_line_table(view);
        munmap(view->buf, view->file_size);
        view->buf = nullptr;
    }
    strnz__cpy(view->provider_cmd, "tree-sitter highlight ", MAXLEN - 1);
    strnz__cat(view->provider_cmd, file, MAXLEN - 1);
    strnz__cpy(view->title, file, MAXLEN - 1);
    if (view_init_input(init, file) == 0) {
        if (view->buf) {
            view->f_eod = 0;
            view->f_bod = 0;
            view->maxcol = 0;
            view->page_top_pos = 0;
            view->page_top_ln_no = 0;
            view->page_bot_ln_no = 0;
            view->ln_max_pos = 0;
            view->ln_no = 0;
            view->page_bot_pos = 0;
            view->file_pos = 0;
            border_title(view->box_win, view->title);
            initialize_line_table(view);
            next_page(view);
            build_prompt(view);
            display_prompt(view, view->prompt_str);
            pad_refresh(view);
        }
    }
}
