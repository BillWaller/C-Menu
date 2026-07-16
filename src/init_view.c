/** @file init_view.c
    @brief Initialize C-Menu View Screen IO and Input
    @ingroup init_view
    @author Bill Waller
    Copyright (c) 2025
    MIT License
    billxwaller@gmail.com
    @date 2026-02-09
 */

/**
   @defgroup init_view Initializing View I/O
   @brief Populate the C-Menu View Struct and Connect Input
 */
#include <common.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <unistd.h>
#include <wait.h>

void view_calc_full_screen_dimensions(Init *);
void view_full_screen_resize(Init *);
void view_calc_boxwin_dimensions(Init *);
void view_boxwin_resize(Init *);
void destroy_view_win(Init *);
void view_win_del(PANEL *, WINDOW *);

ViewStack view_stack;

/** @brief Initialize C-Menu View in full screen mode.
    @ingroup init_view
   @param init Pointer to the Init structure containing view settings.
   @return 0 on success, -1 on failure.
   @details This function sets up the view structure for full screen mode and
   creates a new pad for the view.
   @verbatim
   The function creates the following windows:
   1. view->cmdln_win: Status or Command Line
   2. view->lnno_win: Line Number Window
   3. view->pad: Main Content Pad
   @endverbatim
 */
int init_view_full_screen(Init *init) {
    View *view = init->view;

    if (view->tab_stop <= 0)
        view->tab_stop = TABSIZE;
    set_tabsize(view->tab_stop);
    view->f_full_screen = true;

    // -------------------> 1. WIN <-------------------
    view_calc_full_screen_dimensions(init);
    view->win_win = newwin(LINES, COLS, 0, 0);
    if (view->win_win == nullptr) {
        ssnprintf(em0, MAXLEN - 1, "newwin(LINES, COLS, 0, 0) failed in init_view_full_screen");
        Perror(em0);
        return -1;
    }
    view->win_pan = new_panel(view->win_win);
    wbkgrnd(view->win_win, &CC_NT);

    // -------------------> 2. LNNO <-------------------
    view->lnno_win = derwin(view->win_win, LINES - 1, COLS, 0, 0);
    if (view->lnno_win == nullptr) {
        ssnprintf(em0, MAXLEN - 1, "derwin(view->win_win, LINES - 1, COLS, 0, 0) failed in init_view_full_screen");
        Perror(em0);
        return -1;
    }
    view->lnno_pan = new_panel(view->lnno_win);
    wbkgrnd(view->lnno_win, &CC_LN);
    keypad(view->lnno_win, false);
    idlok(view->lnno_win, false);
    idcok(view->lnno_win, false);
    scrollok(view->lnno_win, true);
    wsetscrreg(view->lnno_win, 0, view->scroll_lines - 1);

    // -------------------> 3. CMDLN <-------------------
    view->cmdln_win = derwin(view->win_win, 1, COLS, LINES - 1, 0);
    if (view->cmdln_win == nullptr) {
        ssnprintf(em0, MAXLEN - 1, "derwin(view->win_win, 1, COLS, LINES - 1, 0) failed in init_view_full_screen");
        Perror(em0);
        return -1;
    }
    view->cmdln_pan = new_panel(view->cmdln_win);
    wbkgrnd(view->cmdln_win, &CC_NT);
    keypad(view->cmdln_win, true);
    idlok(view->cmdln_win, false);
    idcok(view->cmdln_win, false);
    scrollok(view->cmdln_win, false);

    // view->pad_container_win = derwin(view->win_win, LINES - 1, COLS -
    // view->ln_win_cols, 0, view->ln_win_cols);
    // if (view->pad_container_win == nullptr) {
    //     ssnprintf(em0, MAXLEN - 1,
    //               "derwin(view->win_win, LINES - 1, COLS - view->ln_win_cols,
    //               0, view->ln_win_cols) failed in init_view_full_screen");
    //     Perror(em0);
    //     return -1;
    // }
    // view->pad_container_pan = new_panel(view->pad_container_win);
    // -------------------> 4. PAD <-------------------
    view->pad = newpad(LINES - 1, PAD_COLS - 1);
    if (view->pad == nullptr) {
        ssnprintf(em0, MAXLEN - 1, "newpad(LINES - 1, PAD_COLS - 1) failed in init_view_full_screen");
        Perror(em0);
        return -1;
    }
    view->pad_view_win = subpad(view->pad, LINES - 1, PAD_COLS - 1, 0, 0);
    immedok(view->pad, true);
    if (view->pad_view_win == nullptr) {
        ssnprintf(em0, MAXLEN - 1,
                  "subpad(view->pad, LINES - 1, COLS - view->ln_win_cols, 0, 0) failed in init_view_full_screen");
        Perror(em0);
        return -1;
    }
    view->pad_view_pan = new_panel(view->pad_view_win);
    wbkgrnd(view->pad, &CC_NT);
    keypad(view->pad, true);
    keypad(view->pad, true);
    idlok(view->pad, false);
    idcok(view->pad, false);
    scrollok(view->pad, true);
    wsetscrreg(view->pad, 0, view->scroll_lines - 1);
    return 0;
}
/** @brief Resize the full screen view and its components.
    @ingroup window_support
    @param init Pointer to the Init structure containing view settings.
    @details This function resizes the full screen view and its components,
   including the command line window, line number window, and main content pad.
   It also recalculates the dimensions for the full screen mode and updates the
   scroll regions accordingly.
 */
void view_full_screen_resize(Init *init) {
    erase();
    View *view = init->view;
    view_calc_full_screen_dimensions(init);
    mvwin(view->cmdln_win, view->lines - 1, 0);
    wresize(view->cmdln_win, 1, view->cols);
    wresize(view->lnno_win, view->ln_win_lines - 1, view->ln_win_cols);
    wsetscrreg(view->lnno_win, 0, view->scroll_lines - 1);
    wresize(view->pad, view->lines - 1, PAD_COLS);
    wsetscrreg(view->pad, 0, view->lines - 1);
}
/** @brief Calculate the dimensions for full screen mode.
    @ingroup init_view
    @details This function calculates the dimensions for the full screen mode of
   the C-Menu View. It retrieves the maximum dimensions of the standard screen
   and sets the view parameters accordingly. It also resizes the line number
   window and command line window based on the new dimensions.
    @param init Pointer to the Init structure containing view settings.
 */
void view_calc_full_screen_dimensions(Init *init) {
    View *view = init->view;
    getmaxyx(stdscr, view->lines, view->cols);
    view->ln_win_lines = view->lines;
    view->ln_win_cols = 8;
    view->scroll_lines = view->lines - 1;
    view->h_shift = view->cols / 3;
#ifdef DEBUG_RESIZE
    char file[MAXLEN];
    ssnprintf(
        em0, MAXLEN - 1,
        "%s:%d view->lines=%d, view->cols=%d, view->maxrows=%d, view->maxcols=%d",
        __FILE__, __LINE__,
        view->lines, view->cols, view->smaxrow, view->smaxcol);
    write_cmenu_log(em0);
#endif
    view->cmd_line = 0;
    view->pminrow = 0;
    view->pmincol = 0;
    view->sminrow = 0;
    view->smincol = view->ln_win_cols;
    view->smaxrow = view->lines - 1;
    view->smaxcol = view->cols - 1;
    view->ln_no = view->page_top_ln_no + view->scroll_lines;
    view->page_bot_ln_no = view->ln_no;
}

/** @brief Initialize the C-Menu View in box window mode.
    @ingroup init_view
    @param init Pointer to the Init structure containing view settings.
    @return 0 on success, -1 on failure.
    @details sets up the view structure for box window mode, adjusts dimensions
   based on screen size, and creates a new pad for the view. It also configures
   various parameters such as scroll lines, command line position, and tab size.
 */
int init_view_boxwin(Init *init) {
    View *view = init->view;
    if (view->tab_stop <= 0)
        view->tab_stop = TABSIZE;
    set_tabsize(view->tab_stop);
    view->f_full_screen = false;
    view_calc_boxwin_dimensions(init);
    // -------------------> 1. BOX <-------------------
    view->box_win = newwin(view->lines + 2, view->cols + 2, view->begy, view->begx);
    if (view->box_win == nullptr) {
        ssnprintf(em0, MAXLEN - 1, "view->box_win: lines=%d, cols=%d, begy=%d, begx=%d",
                  view->lines + 2, view->cols + 2, view->begy, view->begx);
        Perror(em0);
        return -1;
    }
    view->box_pan = new_panel(view->box_win);
    wbkgrnd(view->box_win, &CC_BOX);
    wbkgrndset(view->box_win, &CC_BOX);
#ifdef DEBUG_RESIZE
    ssnprintf(em0, MAXLEN - 1,
              "%s:%d init BOX: lines=%d, cols=%d, begy=%d, begx=%d",
              __FILE__, __LINE__, view->lines + 2, view->cols + 2, view->begy, view->begx);
    write_cmenu_log(em0);
#endif
    wborder_set(view->box_win, &ls, &rs, &ts, &bs, &tl, &tr, &bl, &br);
    // -------------------> 2. WIN <-------------------
    view->win_win = derwin(view->box_win, view->lines, view->cols, 1, 1);
    if (view->win_win == nullptr) {
        ssnprintf(em0, MAXLEN - 1, "%s:%d WIN: lines=%d, cols=%d, begy=%d, begx=%d",
                  __FILE__, __LINE__, view->lines, view->cols, 1, 1);
        Perror(em0);
        return -1;
    }
    view->win_pan = new_panel(view->win_win);
    wbkgrnd(view->win_win, &CC_NT);
    wbkgrndset(view->win_win, &CC_NT);
    // -------------------> 3. CMDLN <-------------------
    view->cmdln_win = derwin(view->win_win, 1, view->cols, view->lines - 1, 0);
    if (view->cmdln_win == nullptr) {
        ssnprintf(em0, MAXLEN - 1, "%s:%d CMDLN: lines=%d, cols=%d, begy=%d, begx=%d",
                  __FILE__, __LINE__, 1, view->cols, view->lines - 1, 0);
        Perror(em0);
        return -1;
    }
    view->cmdln_pan = new_panel(view->cmdln_win);
    wbkgrnd(view->cmdln_win, &CC_NT);
    wbkgrndset(view->cmdln_win, &CC_NT);
    keypad(view->cmdln_win, true);
    idlok(view->cmdln_win, false);
    idcok(view->cmdln_win, false);
    scrollok(view->cmdln_win, false);

    // -------------------> 4. LNNO <-------------------
    view->lnno_win = derwin(view->win_win, view->lines - 1, view->ln_win_cols, 0, 0);
    if (view->lnno_win == nullptr) {
        ssnprintf(em0, MAXLEN - 1, "%s:%d LNNO: lines=%d, cols=%d, begy=%d, begx=%d",
                  __FILE__, __LINE__, view->lines - 1, view->ln_win_cols, 0, 0);
        Perror(em0);
        return -1;
    }
    view->lnno_pan = new_panel(view->lnno_win);
    wbkgrnd(view->lnno_win, &CC_LN);
    wbkgrndset(view->lnno_win, &CC_LN);
    keypad(view->lnno_win, false);
    idlok(view->lnno_win, false);
    idcok(view->lnno_win, false);
    scrollok(view->lnno_win, true);
    wsetscrreg(view->lnno_win, 0, view->scroll_lines);

    // -------------------> 5. PAD CONTAINER <---------
    // view->pad_container_win = derwin(view->win_win, view->lines - 1,
    // view->cols - view->ln_win_cols, 0, view->ln_win_cols);
    // if (view->pad_container_win == nullptr) {
    //     ssnprintf(em0, MAXLEN - 1,
    //               "derwin(view->win_win, view->lines - 1, view->cols -
    //               view->ln_win_cols, 0, view->ln_win_cols) failed in
    //               init_view_full_screen");
    //     Perror(em0);
    //     return -1;
    // }
    // view->pad_container_pan = new_panel(view->pad_container_win);

    // -------------------> 5. PAD <-------------------
    view->pad = newpad(view->lines - 1, PAD_COLS - 1);
    if (view->pad == nullptr) {
        ssnprintf(em0, MAXLEN - 1, "newpad(view->lines - 1, PAD_COLS - 1) failed in init_view_full_screen");
        Perror(em0);
        return -1;
    }
    // view->pad = newpad(view->lines - 1, PAD_COLS - 1);

    // -------------------> 5. PAD_VIEW <--------------
    view->pad_view_win = subpad(view->pad,
                                view->lines - 1,
                                PAD_COLS - 1,
                                0,
                                0);
    if (view->pad_view_win == nullptr) {
        ssnprintf(em0, MAXLEN - 1,
                  "%s:%d PAD: lines=%d, cols=%d, begy=%d, begx=%d",
                  __FILE__, __LINE__,
                  view->lines - 1,
                  view->cols - view->ln_win_cols,
                  0,
                  0);
        Perror(em0);
        return -1;
    }
    view->pad_view_pan = new_panel(view->pad_view_win);

    // -----------------------------
#ifdef DEBUG_LAYOUT
    wbkgrnd(view->pad, &CC_GREEN);
    wbkgrndset(view->pad, &CC_GREEN);
#else
    wbkgrnd(view->pad, &CC_NT);
    wbkgrndset(view->pad, &CC_NT);
#endif
    scrollok(view->pad, true);
    wsetscrreg(view->pad, 0, view->scroll_lines - 1);
    return (0);
}

void destroy_view_win(Init *init) {
    View *view = init->view;
    if (!view)
        return;
    view_win_del(view->pad_view_pan, view->pad_view_win);
    delwin(view->pad);
    view_win_del(view->cmdln_pan, view->cmdln_win);
    view_win_del(view->lnno_pan, view->lnno_win);
    view_win_del(view->win_pan, view->win_win);
    view_win_del(view->box_pan, view->box_win);
    wnoutrefresh(stdscr);
    update_panels();
    doupdate();
}
void view_win_del(PANEL *pan, WINDOW *win) {
    if (pan) {
        del_panel(pan);
        pan = nullptr;
        delwin(win);
        win = nullptr;
    }
}
//------------------------------------------------------------------------------
/** @brief Resize the current window and its box
    @ingroup window_support
    @param init Pointer to the Init structure containing view settings.
    @details This function resizes the current window and its associated box
   window to the specified number of lines and columns. */
void view_boxwin_resize(Init *init) {
    destroy_view_win(init);
    init_view_boxwin(init);
    border_title(init->view->box_win, init->view->title);
    // initialize_line_table(init->view);
}

/** @brief Calculate the dimensions and position of the box window for C-Menu
   View.
    @ingroup init_view
    @details This function calculates the dimensions and position of the box
   window for the C-Menu View based on the screen size and any specified
   parameters in the Init structure. It ensures that the box window fits within
   the screen dimensions and adjusts its size and position accordingly.
    @param init Pointer to the Init structure containing view settings.
 */
void view_calc_boxwin_dimensions(Init *init) {
    int scr_lines, scr_cols;
    View *view = init->view;

    /** Use view->lines and view->cols if set, otherwise calculate based on
     * screen size with some padding. Ensure the view fits within the screen
     * dimensions. */

    getmaxyx(stdscr, scr_lines, scr_cols);
#ifdef DEBUG_RESIZE
    ssnprintf(em0, MAXLEN - 1,
              "%s:%d=%d calc lines=%d, cols=%d, begy=%d, begx=%d",
              __FILE__, __LINE__, 526, view->lines, view->cols, view->begy, view->begx);
    write_cmenu_log(em0);
#endif
    if (view->lines == 0 && view->cols == 0 && view->begy == 0 && view->begx == 0) {
        view->lines = scr_lines - 3;
        view->cols = scr_cols - 2;
        view->begy = 0;
        view->begx = 0;
    }
    int lines = max(scr_lines / 10, 8);
    view->lines = max(view->lines, lines);
    if (view->lines > scr_lines - 3)
        view->lines = scr_lines - 3;
    if (view->lines + view->begy > scr_lines - 3)
        view->begy = scr_lines - (view->lines + 3);

    int cols = max(scr_cols / 3, 40);
    view->cols = max(view->cols, cols);
    if (view->cols > scr_cols - 2)
        view->cols = scr_cols - 2;
    if (view->cols + view->begx > scr_cols - 2)
        view->begx = scr_cols - (view->cols + 2);

    view->h_shift = view->cols / 3;
    view->ln_win_lines = view->lines - 1;
    if (view->f_ln)
        view->ln_win_cols = 8;
    else
        view->ln_win_cols = 0;
    view->scroll_lines = view->lines - 1;
    view->cmd_line = 0;
    view->pminrow = 0;
    view->pmincol = 0;
    view->sminrow = view->begy + 1;
    view->smincol = view->begx + view->ln_win_cols + 1;
    view->smaxrow = view->begy + view->lines;
    view->smaxcol = view->begx + view->cols;
    view->ln_no = view->page_top_ln_no + view->scroll_lines;
    view->page_bot_ln_no = view->ln_no;
#ifdef DEBUG_RESIZE
    ssnprintf(em0, MAXLEN - 1,
              "%s:%d calc BOX lines=%d, cols=%d, begy=%d, begx=%d",
              __FILE__, __LINE__, view->lines + 2, view->cols + 2, view->begy, view->begx);
    write_cmenu_log(em0);
    ssnprintf(em0, MAXLEN - 1,
              "%s:%d calc WIN lines=%d, cols=%d, begy=%d, begx=%d",
              __FILE__, __LINE__, view->lines, view->cols, 1, 1);
    write_cmenu_log(em0);
    ssnprintf(em0, MAXLEN - 1,
              "%s:%d calc CMDLN lines=%d, cols=%d, begy=%d, begx=%d",
              __FILE__, __LINE__, 1, view->cols, view->lines - 1, 1);
    write_cmenu_log(em0);
    ssnprintf(em0, MAXLEN - 1,
              "%s:%d calc LNNO lines=%d, cols=%d, begy=%d, begx=%d",
              __FILE__, __LINE__, view->lines - 1, view->ln_win_cols, 0, 0);
    write_cmenu_log(em0);
    ssnprintf(em0, MAXLEN - 1,
              "%s:%d calc PAD lines=%d, cols=%d, begy=%d, begx=%d",
              __FILE__, __LINE__, view->lines - 1, view->cols - view->ln_win_cols, 0, view->ln_win_cols);
    write_cmenu_log(em0);

#endif
}
/** @brief Initialize the input for the C-Menu View.
    @ingroup init_view
    @param init Pointer to the Init structure containing view settings.
    @param file_name Name of the input file or "-" for standard input.
    @return 0 on success, -1 on failure.
    @details This function initializes the input for the C-Menu View. It handles
   both regular files and standard input, setting up pipes if necessary. It also
   memory-maps the input file for efficient access and sets up the view structure
   accordingly.
 */
int view_init_input(Init *init, char *file_name) {
    struct stat sb;
    int idx = 0;
    pid_t pid = -1;
    int pipe_fd[2];
    int s_argc = 0;
    char *s_argv[MAXARGS];
    char tmp_str[MAXLEN];
    View *view = init->view;
    view->f_in_pipe = false;
    if (strcmp(file_name, "-") == 0) {
        file_name = "/dev/stdin";
        view->f_in_pipe = true;
    }
    if (view->provider_cmd[0] != '\0') {
        s_argc = str_to_args(s_argv, view->provider_cmd, MAXARGS - 1);
        if (pipe(pipe_fd) == -1) {
            Perror("pipe(pipe_fd) failed in init_view");
            return -1;
        }
        // endwin();
        if ((pid = fork()) == -1) {
            Perror("fork() failed in init_view");
            return -1;
        }
        if (pid == 0) { // Child
            /** Prevent child process from writing to terminal */
            int dev_null = open("/dev/null", O_WRONLY);
            if (dev_null == -1) {
                Perror("open(/dev/null) failed in init_pick child process");
                exit(EXIT_FAILURE);
            }
            dup2(dev_null, STDERR_FILENO);
            close(dev_null);
            close(pipe_fd[P_READ]);
            dup2(pipe_fd[P_WRITE], STDOUT_FILENO);
            close(pipe_fd[P_WRITE]);
            execvp(s_argv[0], s_argv);
            strnz__cpy(tmp_str, "Can't exec view start cmd: ", MAXLEN - 1);
            strnz__cat(tmp_str, s_argv[0], MAXLEN - 1);
            Perror(tmp_str);
            exit(EXIT_FAILURE);
        }
        // Back to parent
        destroy_argv(s_argc, s_argv);
        reset_prog_mode();
        restore_wins();
        close(pipe_fd[P_WRITE]);
        dup2(pipe_fd[P_READ], STDIN_FILENO);
        view->in_fd = dup(STDIN_FILENO);
        view->f_in_pipe = true;
    } else {
        if (view->f_in_pipe)
            view->in_fd = dup(STDIN_FILENO);
        else {
            /*----------------------------------------------------------------------*/
            /* Open the input file for reading and get its size. */
            expand_tilde(file_name, MAXLEN - 1);
            view->in_fd = open(file_name, O_RDONLY);
            if (view->in_fd == -1) {
                ssnprintf(em0, MAXLEN - 1, "%s, line: %d", __FILE__,
                          __LINE__ - 3);
                ssnprintf(em1, MAXLEN - 1, "open %s", file_name);
                strerror_r(errno, em2, MAXLEN);
                display_error(em0, em1, em2, nullptr);
                return -1;
            }
            if (fstat(view->in_fd, &sb) == -1) {
                ssnprintf(em0, MAXLEN - 1, "%s, line: %d", __FILE__,
                          __LINE__ - 1);
                ssnprintf(em1, MAXLEN - 1, "fstat %s", file_name);
                strerror_r(errno, em2, MAXLEN);
                display_error(em0, em1, em2, nullptr);
                close(view->in_fd);
                return -1;
            }
            view->file_size = sb.st_size;
            if (view->file_size == 0) {
                close(view->in_fd);
                ssnprintf(em0, MAXLEN - 1, "%s, line: %d", __FILE__,
                          __LINE__ - 1);
                ssnprintf(em1, MAXLEN - 1, "file %s is empty", file_name);
                strerror_r(errno, em2, MAXLEN);
                display_error(em0, em1, em2, nullptr);
                return -1;
            }
            if (!S_ISREG(sb.st_mode))
                view->f_in_pipe = true;
        }
        /*----------------------------------------------------------------------*/
    }
    if (view->f_in_pipe) {
        close(view->in_fd);
        errno = 0;
        view->in_fd = memfd_create("view_input", MFD_CLOEXEC);
        if (view->in_fd < 0 || errno != 0) {
            ssnprintf(em0, MAXLEN - 1, "memfd_create failed\n");
            ssnprintf(em1, MAXLEN - 1, "%s", strerror(errno));
            ssnprintf(em2, MAXLEN - 1, "%s, line: %d, errno: %d", __FILE__,
                      __LINE__ - 4, errno);
            display_error(em0, em1, em2, nullptr);
            exit(EXIT_FAILURE);
        }
        char buf[VBUFSIZ];
        ssize_t bytes_read = 0;
        ssize_t bytes_written = 0;
        while ((bytes_read = read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
            if ((bytes_written = write(view->in_fd, buf, bytes_read)) != bytes_read) {
                abend(-1, "unable to write view->in_fd");
                exit(EXIT_FAILURE);
            }
        }
        if (fstat(view->in_fd, &sb) == -1) {
            ssnprintf(em0, MAXLEN - 1, "fstat(view->in_fd) failed\n");
            ssnprintf(em1, MAXLEN - 1, "%s", strerror(errno));
            ssnprintf(em2, MAXLEN - 1, "%s, line: %d, errno: %d", __FILE__,
                      __LINE__ - 4, errno);
            display_error(em0, em1, em2, nullptr);
            exit(EXIT_FAILURE);
        }
        view->file_size = sb.st_size;
        if (view->file_size == 0) {
            close(view->in_fd);
            strnz__cpy(tmp_str, "no standard input", MAXLEN - 1);
            abend(-1, tmp_str);
            exit(EXIT_FAILURE);
        }
        waitpid(-1, nullptr, 0);
    }
    view->buf =
        mmap(nullptr, view->file_size, PROT_READ, MAP_PRIVATE, view->in_fd, 0);
    if (view->buf == MAP_FAILED) {
        ssnprintf(em0, MAXLEN - 1, "%s, line: %d", __FILE__, __LINE__ - 2);
        ssnprintf(em1, MAXLEN - 1, "mmap %s", file_name);
        strerror_r(errno, em2, MAXLEN);
        display_error(em0, em1, em2, nullptr);
        close(view->in_fd);
        return -1;
    }
    close(view->in_fd);
    SIO *sio = init->sio;
    stdio_names(stdio_names_str, "init_view.c 673");
    stdio_fdnames(stdio_names_str, "init_view.c 674");
    dup2(sio->stdin_fd, STDIN_FILENO);
    stdio_names(stdio_names_str, "init_view.c 673");
    stdio_fdnames(stdio_names_str, "init_view.c 674");
    view->file_size = sb.st_size;
    view->prev_file_pos = NULL_POSITION;
    view->buf_curr_ptr = view->buf;
    if (view->cmd_all[0] != '\0')
        strnz__cpy(view->cmd, view->cmd_all, MAXLEN - 1);
    for (idx = 0; idx < NMARKS; idx++)
        view->mark_tbl[idx] = NULL_POSITION;
    strnz__cpy(view->cur_file_str, file_name, MAXLEN - 1);
    base_name(view->file_name, view->cur_file_str);
    return 0;
}
