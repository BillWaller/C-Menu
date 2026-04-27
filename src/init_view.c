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
void view_calc_win_dimensions(Init *, char *title);
void view_win_resize(Init *, char *);

/** @brief Initialize C-Menu View in full screen mode.
    @ingroup init_view
    @note This function sets up the view structure for full screen mode and
   creates a new pad for the view.
   @param init Pointer to the Init structure containing view settings.
   @return 0 on success, -1 on failure.
   @verbatim
   The function creates the following windows:
   1. view->cmdln_win: Status or Command Line
   2. view->ln_win: Line Number Window
   3. view->pad: Main Content Pad
   @endverbatim
 */
int init_view_full_screen(Init *init) {
    view = init->view;

    if (view->tab_stop <= 0)
        view->tab_stop = TABSIZE;
    set_tabsize(view->tab_stop);
    view->f_full_screen = true;
    view_calc_full_screen_dimensions(init);
#ifdef DEBUG_RESIZE
    ssnprintf(em0, MAXLEN - 1,
              "init_view_full_screen(): lines=%d, cols=%d, "
              "ln_win_lines=%d, ln_win_cols=%d, "
              "scroll_lines=%d",
              view->lines, view->cols, view->ln_win_lines, view->ln_win_cols,
              view->scroll_lines);
#endif
    /** view->cmdln_win: status or command line window */
    view->cmdln_win =
        newwin(1, view->cols, view->begy + view->lines - 1, view->begx);
    keypad(view->cmdln_win, true);
    idlok(view->cmdln_win, false);
    idcok(view->cmdln_win, false);
    wbkgrnd(view->cmdln_win, &CCC_WIN);
    wbkgrndset(view->cmdln_win, &CCC_WIN);
    scrollok(view->cmdln_win, false);
#ifdef DEBUG_IMMEDOK
    immedok(view->cmdln_win, true);
#endif
    /** view->ln_win: line number window */
    view->ln_win = newwin(view->ln_win_lines - 1, view->ln_win_cols, 0, 0);
    keypad(view->ln_win, false);
    idlok(view->ln_win, false);
    idcok(view->ln_win, false);
    wbkgrnd(view->ln_win, &CCC_LN);
    wbkgrndset(view->ln_win, &CCC_LN);
    scrollok(view->ln_win, true);
    wsetscrreg(view->ln_win, 0, view->scroll_lines - 1);
#ifdef DEBUG_IMMEDOK
    immedok(view->ln_win, true);
#endif
    /** view->cmdln_win: status or command line window */
    view->pad = newpad(view->lines - 1, PAD_COLS);
    if (view->pad == nullptr) {
        ssnprintf(em0, MAXLEN - 1, "%s, line: %d", __FILE__, __LINE__ - 2);
        ssnprintf(em1, MAXLEN - 1, "newpad(%d, %d) failed", view->lines,
                  PAD_COLS);
        em2[0] = '\0';
        display_error(em0, em1, em2, nullptr);
        abend(-1, "init_view_full_screen: newpad() failed");
    }
    keypad(view->pad, false);
    idlok(view->pad, false);
    idcok(view->pad, false);
    wbkgrnd(view->pad, &CCC_WIN);
    wbkgrndset(view->pad, &CCC_WIN);
    scrollok(view->pad, true);
    wsetscrreg(view->pad, 0, view->scroll_lines - 1);
#ifdef DEBUG_IMMEDOK
    immedok(view->pad, true);
#endif
    return 0;
}
/** @brief Resize the full screen view and its components.
    @ingroup window_support
    @param init Pointer to the Init structure containing view settings.
    @note This function resizes the full screen view and its components,
   including the command line window, line number window, and main content pad.
   It also recalculates the dimensions for the full screen mode and updates the
   scroll regions accordingly.
 */
void view_full_screen_resize(Init *init) {
    erase();
    wnoutrefresh(stdscr);
    wrefresh(stdscr);
    view_calc_full_screen_dimensions(init);
    mvwin(view->cmdln_win, view->lines - 1, 0);
    wresize(view->cmdln_win, 1, view->cols);
    wnoutrefresh(view->cmdln_win);

    // wresize(view->ln_win, view->ln_win_lines, view->ln_win_cols);
    wresize(view->ln_win, view->ln_win_lines - 1, view->ln_win_cols);
    wsetscrreg(view->ln_win, 0, view->scroll_lines - 1);
    wnoutrefresh(view->ln_win);

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
    view = init->view;
    getmaxyx(stdscr, view->lines, view->cols);
    view->ln_win_lines = view->lines;
    view->ln_win_cols = 8;
    view->scroll_lines = view->lines - 1;
#ifdef DEBUG_RESIZE
    ssnprintf(
        em0, MAXLEN - 1,
        "view->lines=%d, view->cols=%d, view->maxrows=%d, view->maxcols=%d",
        view->lines, view->cols, view->smaxrow, view->smaxcol);
    write_cmenu_log_nt(em0);
#endif
    view->cmd_line = 0;
    view->pminrow = 0;
    view->pmincol = 0;
    view->sminrow = 0;
    view->smincol = 0;
    view->smaxrow = view->lines - 1;
    view->smaxcol = view->cols - 1;
    view->ln = view->page_top_ln + view->scroll_lines;
    view->page_bot_ln = view->ln;
}
/** @brief Initialize the C-Menu View in box window mode.
    @ingroup init_view
    @note sets up the view structure for box window mode, adjusts dimensions
   based on screen size, and creates a new pad for the view. It also configures
   various parameters such as scroll lines, command line position, and tab size.
    @param init Pointer to the Init structure containing view settings.
    @param title Title for the box window.
    @return 0 on success, -1 on failure.
 */
int init_view_boxwin(Init *init, char *title) {
    view = init->view;

    if (view->tab_stop <= 0)
        view->tab_stop = TABSIZE;
    set_tabsize(view->tab_stop);
    view->f_full_screen = false;

    view_calc_win_dimensions(init, title);
    if (title != nullptr && title[0] != '\0')
        strnz__cpy(view->title, title, MAXLEN - 1);
    else {
        if (view->argv != nullptr && view->argv[0] != nullptr &&
            view->argv[0][0] != '\0')
            strnz__cpy(view->title, view->argv[0], MAXLEN - 1);
    }
#ifdef DEBUG_RESIZE
    ssnprintf(em0, MAXLEN - 1,
              "init_view_boxwin(): view->box: begy=%d, begx=%d, lines=%d, "
              "cols=%d, title=%s",
              view->begy, view->begx, view->lines + 2, view->cols + 2,
              view->title);
    write_cmenu_log_nt(em0);
#endif
    if (box_new(view->lines, view->cols, view->begy, view->begx, view->title,
                false)) {
        ssnprintf(em0, MAXLEN - 1, "%s, line: %d", __FILE__, __LINE__ - 1);
        ssnprintf(em1, MAXLEN - 1, "win_new(%d, %d, %d, %d, %s) failed",
                  view->lines, view->cols, view->begy, view->begx, view->title);
        em2[0] = '\0';
        display_error(em0, em1, em2, nullptr);
        return (-1);
    }
    view->box = win_box[win_ptr];

    /** view->cmdln_win: status or command line window */
    view->cmdln_win =
        newwin(1, view->cols, view->begy + view->lines, view->begx + 1);
    keypad(view->cmdln_win, true);
    idlok(view->cmdln_win, false);
    idcok(view->cmdln_win, false);
    wbkgrnd(view->cmdln_win, &CCC_WIN);
    wbkgrndset(view->cmdln_win, &CCC_WIN);
    scrollok(view->cmdln_win, false);
#ifdef DEBUG_IMMEDOK
    immedok(view->cmdln_win, true);
#endif

    /** view->ln_win: line number window */
#ifdef DEBUG_RESIZE
    ssnprintf(em0, MAXLEN - 1,
              "init_view_boxwin(): view->ln_win: begy=%d, begx=%d, lines=%d, "
              "cols=%d, scroll_lines=%d",
              view->begy, view->begx, view->ln_win_lines, view->ln_win_cols,
              view->scroll_lines);
    write_cmenu_log_nt(em0);
#endif
    if (view->f_ln) {
        view->ln_win = newwin(view->ln_win_lines, view->ln_win_cols,
                              view->begy + 1, view->begx + 1);
        keypad(view->ln_win, false);
        idlok(view->ln_win, false);
        idcok(view->ln_win, false);
        wbkgrnd(view->ln_win, &CCC_LN);
        wbkgrndset(view->ln_win, &CCC_LN);
        scrollok(view->ln_win, true);
        wsetscrreg(view->ln_win, 0, view->scroll_lines - 1);
    }
#ifdef DEBUG_IMMEDOK
    immedok(view->ln_win, true);
#endif
    /** pad for main content */
    view->pad = newpad(view->lines - 1, PAD_COLS);
    keypad(view->pad, true);
    idlok(view->pad, false);
    idcok(view->pad, false);
    wbkgrnd(view->pad, &CCC_WIN);
    wbkgrndset(view->pad, &CCC_WIN);
    scrollok(view->pad, true);
    wsetscrreg(view->pad, 0, view->scroll_lines - 1);
#ifdef DEBUG_IMMEDOK
    immedok(view->pad, true);
#endif
    return (0);
}
/** @brief Resize the current window and its box, and update the title
    @ingroup window_support
    @param init Pointer to the Init structure containing view settings.
    @param title Window title
    @note This function resizes the current window and its associated box window
   to the specified number of lines and columns. It also updates the title of
   the box window if a title is provided. After resizing, it refreshes the
   windows to apply the changes. */
void view_win_resize(Init *init, char *title) {
    int maxx;
    erase();
    wnoutrefresh(stdscr);
    wrefresh(stdscr);
    view_calc_win_dimensions(init, title);
#ifdef DEBUG_RESIZE
    ssnprintf(em0, MAXLEN - 1, "view->box: begy=%d, begx=%d, lines=%d, cols=%d",
              view->begy, view->begx, view->lines + 2, view->cols + 2);
    write_cmenu_log_nt(em0);
#endif
    mvwin(view->box, view->begy, view->begx);
    wresize(view->box, view->lines + 2, view->cols + 2);
    wbkgrnd(view->box, &CCC_BOX);
    wbkgrndset(view->box, &CCC_BOX);
    cbox(view->box);
    if (title != nullptr && *title != '\0') {
        wmove(view->box, 0, 1);
        waddnstr(view->box, (const char *)&bw_rt, 1);
        wmove(view->box, 0, 2);
        waddnstr(view->box, (const char *)&bw_sp, 1);
        mvwaddnwstr(view->box, 0, 1, &bw_rt, 1);
        mvwaddnwstr(view->box, 0, 2, &bw_sp, 1);
        int len = strlen(title);
        if (len > (view->cols - 4)) {
            len -= (view->cols - 4);
            mvwaddstr(view->box, 0, 3, &title[len]);
        } else
            mvwaddstr(view->box, 0, 3, title);
        maxx = getmaxx(view->box);
        int s = strlen(title);
        if ((s + 3) < maxx)
            mvwaddch(view->box, 0, (s + 3), ' ');
        if ((s + 4) < maxx)
            mvwaddnwstr(view->box, 0, (s + 4), &bw_lt, 1);
    }
    wnoutrefresh(view->box);
#ifdef DEBUG_RESIZE
    ssnprintf(em0, MAXLEN - 1,
              "view->cmdln_win: begy=%d, begx=%d, lines=%d, cols=%d",
              view->begy + view->lines, view->begx + 1, 1, view->cols);
    write_cmenu_log_nt(em0);
#endif
    mvwin(view->cmdln_win, view->begy + view->lines, view->begx + 1);
    wresize(view->cmdln_win, 1, view->cols);
    wnoutrefresh(view->cmdln_win);
#ifdef DEBUG_RESIZE
    ssnprintf(em0, MAXLEN - 1,
              "(285)view->ln_win: begy=%d, begx=%d, lines=%d, cols=%d, "
              "scroll_lines %d",
              view->begy + 1, view->begx + 1, view->ln_win_lines + 2,
              view->ln_win_cols, view->scroll_lines);
    write_cmenu_log_nt(em0);
#endif
    if (view->f_ln) {
        if (view->ln_win == nullptr) {
            view->ln_win = newwin(view->ln_win_lines, view->ln_win_cols,
                                  view->begy + 1, view->begx + 1);
            keypad(view->ln_win, false);
            idlok(view->ln_win, false);
            idcok(view->ln_win, false);
            wbkgrnd(view->ln_win, &CCC_LN);
            wbkgrndset(view->ln_win, &CCC_LN);
            scrollok(view->ln_win, true);
            wsetscrreg(view->ln_win, 0, view->scroll_lines - 1);
#ifdef DEBUG_IMMEDOK
            immedok(view->ln_win, true);
#endif
        } else {
            mvwin(view->ln_win, view->begy + 1, view->begx + 1);
            wresize(view->ln_win, view->ln_win_lines, view->ln_win_cols);
            wsetscrreg(view->ln_win, 0, view->scroll_lines);
            wnoutrefresh(view->ln_win);
        }
    } else if (view->ln_win != nullptr) {
        delwin(view->ln_win);
        view->ln_win = nullptr;
    }
    view->sminrow = view->begy + 1;
    view->smincol = view->begx + 1;
    view->smaxrow = view->begy + view->lines;
    view->smaxcol = view->begx + view->cols;
#ifdef DEBUG_RESIZE
    ssnprintf(em0, MAXLEN - 1,
              "view->pad: sminrow=%d, smincol=%d, smaxrow=%d, smaxcol=%d",
              view->sminrow, view->smincol, view->smaxrow, view->smaxcol);
    write_cmenu_log_nt(em0);
#endif
    wresize(view->pad, view->lines - 1, PAD_COLS);
    wsetscrreg(view->pad, 0, view->scroll_lines);
}
/** @brief Calculate the dimensions and position of the box window for C-Menu
   View.
    @ingroup init_view
    @details This function calculates the dimensions and position of the box
   window for the C-Menu View based on the screen size and any specified
   parameters in the Init structure. It ensures that the box window fits within
   the screen dimensions and adjusts its size and position accordingly.
    @param init Pointer to the Init structure containing view settings.
    @param title Title for the box window, used to ensure minimum width if
   provided.
 */
void view_calc_win_dimensions(Init *init, char *title) {
    int scr_lines, scr_cols;
    view = init->view;
    getmaxyx(stdscr, scr_lines, scr_cols);
    int len = strlen(title);

    /** Use view->lines and view->cols if set, otherwise calculate based on
     * screen size with some padding. Ensure the view fits within the screen
     * dimensions. */

    if (init->lines != 0 && view->lines == 0)
        view->lines = init->lines;
    if (view->lines == 0)
        view->lines = scr_lines * 3 / 4;
    if (view->lines > scr_lines - 3)
        view->lines = scr_lines - 3;

    if (init->cols != 0 && view->cols == 0)
        view->cols = init->cols;
    if (view->cols == 0)
        view->cols = scr_cols * 3 / 4;
    if (view->cols < len + 4)
        view->cols = len + 4;
    if (view->cols > scr_cols - 4)
        view->cols = scr_cols - 4;

    if (init->begy != 0 && view->begy == 0)
        view->begy = init->begy;
    if (view->begy == 0)
        view->begy = (scr_lines - view->lines) / 5;
    if (view->begy + view->lines > scr_lines - 2)
        view->begy = scr_lines - view->lines - 2;

    if (init->begx != 0 && view->begx == 0)
        view->begx = init->begx;
    if (view->begx == 0)
        view->begx = (scr_cols - view->cols) / 5;
    if (view->begx + view->cols > scr_cols - 2)
        view->begx = scr_cols - view->cols - 2;

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
    view->smincol = view->begx + 1;
    view->smaxrow = view->begy + view->lines;
    view->smaxcol = view->begx + view->cols;

    view->ln = view->page_top_ln + view->scroll_lines;
    view->page_bot_ln = view->ln;
}
/** @brief Initialize the input for a C-Menu View.
@ingroup init_view
@details This function initializes the input for view, which can be a file,
standard input, or a provider command to be initiated by view. It handles
different input sources and sets up the necessary file descriptors and memory
mapping for efficient access.
@param view Pointer to the View structure to be initialized.
@param file_name Name of the input file or "-" for standard input.
@return true on success, false on failure.
@note if a provider command is specified, set up a pipe to read its output.
A child process is spawned, and view, the parent process, reads from the
pipe.
@note If input is from a pipe or standard input, clone it to a temporary
file. This allows for memory-mapping the input later. It does not support
real-time updates to the input, but it allows for efficient access to the
data.
*/
int view_init_input(View *view, char *file_name) {
    struct stat sb;
    int idx = 0;
    pid_t pid = -1;
    int pipe_fd[2];
    int s_argc = 0;
    char *s_argv[MAXARGS];
    char tmp_str[MAXLEN];
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
        close(pipe_fd[P_WRITE]);
        dup2(pipe_fd[P_READ], STDIN_FILENO);
        view->in_fd = dup(STDIN_FILENO);
        view->f_in_pipe = true;
    } else {
        if (view->f_in_pipe)
            view->in_fd = dup(STDIN_FILENO);
        else {
            /*----------------------------------------------------------------------*/
            /** Open the input file for reading and get its size. */
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
        char tmp_filename[] = "/tmp/view_XXXXXX";
        char buf[VBUFSIZ];
        ssize_t bytes_read = 0;
        ssize_t bytes_written = 0;
        close(view->in_fd);
        view->in_fd = mkstemp(tmp_filename);
        if (view->in_fd == -1) {
            abend(-1, "failed to mkstemp");
            exit(EXIT_FAILURE);
        }
        unlink(tmp_filename);
        /*-----------------------------------------------------------------*/
        bool f_wait = false;
        int ready;
        fd_set read_fds;
        struct timeval timeout;
        Chyron *wait_chyron = nullptr;
        WINDOW *wait_win = nullptr;
        int remaining;
        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds);
        timeout.tv_sec = 0;
        timeout.tv_usec = 200000; /**< 200ms timeout to check for input */
        ready = select(STDIN_FILENO + 1, &read_fds, nullptr, nullptr, &timeout);
        if (ready == 0) {
            f_wait = true;
            remaining = wait_timeout;
            wait_chyron = wait_mk_chyron();
            wait_win = wait_mk_win(wait_chyron, "WAITING for VIEW INPUT");
        }
        cmd_key = 0;
        while (ready == 0 && remaining > 0 && cmd_key != KEY_F(9)) {
            cmd_key = wait_continue(wait_win, wait_chyron, remaining);
            if (cmd_key == KEY_F(9))
                break;
            FD_ZERO(&read_fds);
            FD_SET(STDIN_FILENO, &read_fds);
            timeout.tv_sec = 0;
            timeout.tv_usec = 0;
            ready =
                select(STDIN_FILENO + 1, &read_fds, nullptr, nullptr, &timeout);
            remaining--;
        }
        if (f_wait) {
            if (wait_chyron != nullptr)
                wait_destroy(wait_chyron);
        }
        if (cmd_key == KEY_F(9)) {
            if (view->f_in_pipe && pid > 0) {
                /** If user cancels while waiting for view input, kill
                 * provider_cmd child process and close pipe */
                kill(pid, SIGKILL);
                waitpid(pid, nullptr, 0);
                close(pipe_fd[P_READ]);
            }
            Perror("No view input available");
            return -1;
        }
        if (ready == -1) {
            Perror("Error waiting for view input");
            if (view->f_in_pipe && pid > 0) {
                /** If error occurs while waiting for view input, kill
                 * provider_cmd child process and close pipe */
                kill(pid, SIGKILL);
                waitpid(pid, nullptr, 0);
                close(pipe_fd[P_READ]);
            }
            return -1;
        }
        if (ready == 0) {
            Perror("Timeout waiting for view input");
            if (view->f_in_pipe && pid > 0) {
                /** If timeout occurs while waiting for view input, kill
                 * provider_cmd child process and close pipe */
                kill(pid, SIGKILL);
                waitpid(pid, nullptr, 0);
                close(pipe_fd[P_READ]);
            }
            return -1;
        }
        if (ready == 1 && !FD_ISSET(STDIN_FILENO, &read_fds)) {
            Perror("Unexpected error waiting for view input");
            if (view->f_in_pipe && pid > 0) {
                /** If unexpected error occurs while waiting for view input,
                 * kill provider_cmd child process and close pipe */
                kill(pid, SIGKILL);
                waitpid(pid, nullptr, 0);
                close(pipe_fd[P_READ]);
            }
            return -1;
        }
        /*-----------------------------------------------------------------*/
        while ((bytes_read = read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
            if (write(view->in_fd, buf, bytes_read) != bytes_read) {
                abend(-1, "unable to write tmp");
                exit(EXIT_FAILURE);
            }
            bytes_written += bytes_read;
        }
        if (bytes_written == 0) {
            abend(-1, "unable to read stdin");
            exit(EXIT_FAILURE);
        }
        if (fstat(view->in_fd, &sb) == -1) {
            abend(-1, "fstat failed");
            exit(EXIT_FAILURE);
        }
        view->file_size = sb.st_size;
        if (view->file_size == 0) {
            close(view->in_fd);
            strnz__cpy(tmp_str, "no standard input", MAXLEN - 1);
            abend(-1, tmp_str);
            exit(EXIT_FAILURE);
        }
        waitpid_with_timeout(pid, wait_timeout);
        // waitpid(-1, nullptr, 0);
    }
    //  Memory-map the input file for efficient access.
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
