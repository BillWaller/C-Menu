/** @file init_view.c
    @brief Initialize C-Menu View Screen IO and Input
    @author Bill Waller
    Copyright (c) 2025
    MIT License
    billxwaller@gmail.com
    @date 2026-02-09
 */

#include "common.h"
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <unistd.h>
#include <wait.h>

/** @brief Initialize the C-Menu View in full screen mode.
   This function sets up the view structure for full screen mode and creates
   a new pad for the view. It also configures various parameters such as
   scroll lines, command line position, and tab size.
   @param init Pointer to the Init structure containing view settings.
   @return 0 on success, -1 on failure.
 */
int init_view_full_screen(Init *init) {
    view = init->view;
    view->f_full_screen = true;
    getmaxyx(stdscr, view->lines, view->cols);
    view->pminrow = 0;
    view->pmincol = 0;
    view->sminrow = 0;
    view->smincol = 0;
    view->scroll_lines = view->lines - 1;
    view->cmd_line = view->lines - 1;
    view->smaxrow = view->lines - 1;
    view->smaxcol = view->cols - 1;
    view->win = newpad(view->lines, PAD_COLS);
    if (view->win == NULL) {
        ssnprintf(em0, MAXLEN - 65, "%s, line: %d", __FILE__, __LINE__ - 2);
        ssnprintf(em1, MAXLEN - 65, "newpad(%d, %d) failed", view->lines,
                  PAD_COLS);
        em2[0] = '\0';
        display_error(em0, em1, em2, NULL);
        abend(-1, "init_view_full_screen: newpad() failed");
    }
    view->box = NULL;
    wbkgrnd(view->box, &CCC_BOX);
    if (view->tab_stop <= 0)
        view->tab_stop = TABSIZE;
    set_tabsize(view->tab_stop);
    wsetscrreg(view->win, 0, view->scroll_lines - 1);
    scrollok(view->win, true);
    keypad(view->win, true);
    idlok(view->win, false);
    idcok(view->win, false);
    return 0;
}
/** @brief Initialize the C-Menu View in box window mode.
   @note sets up the view structure for box window mode, adjusts dimensions
   based on screen size, and creates a new pad for the view. It also configures
   various parameters such as scroll lines, command line position, and tab size.
   @param init Pointer to the Init structure containing view settings.
   @param title Title for the box window.
   @return 0 on success, -1 on failure.
 */
int init_view_boxwin(Init *init, char *title) {
    int scr_lines, scr_cols;
    view = init->view;
    view->f_full_screen = false;
    // scr_lines = LINES;
    // scr_cols = COLS;
    getmaxyx(stdscr, scr_lines, scr_cols);
    if (view->lines > scr_lines)
        view->lines = scr_lines;
    if (view->cols > scr_cols)
        view->cols = scr_cols;
    if (view->begy + view->lines > scr_lines)
        view->begy = scr_lines - view->lines - 2;
    if (view->begx + view->cols > scr_cols) {
        view->begx = scr_cols - view->cols - 2;
    }
    if (title != NULL && title[0] != '\0')
        strnz__cpy(view->title, title, MAXLEN - 1);
    else {
        if (view->argv != NULL && view->argv[0] != NULL &&
            view->argv[0][0] != '\0')
            strnz__cpy(view->title, view->argv[0], MAXLEN - 1);
    }
    if (win_new(view->lines, view->cols, view->begy, view->begx, view->title,
                F_VIEW)) {
        ssnprintf(em0, MAXLEN - 65, "%s, line: %d", __FILE__, __LINE__ - 1);
        ssnprintf(em1, MAXLEN - 65, "win_new(%d, %d, %d, %d, %s, %b) failed",
                  view->lines, view->cols, view->begy, view->begx, "NULL",
                  F_VIEW);
        em2[0] = '\0';
        display_error(em0, em1, em2, NULL);
        return (-1);
    }
    view->scroll_lines = view->lines - 1;
    view->cmd_line = view->lines - 1;
    view->pminrow = 0;
    view->pmincol = 0;
    view->sminrow = view->begy + 1;
    view->smincol = view->begx + 1;
    view->smaxrow = view->begy + view->lines;
    view->smaxcol = view->begx + view->cols;
    win_win[win_ptr] = newpad(view->lines, PAD_COLS);
    view->win = win_win[win_ptr];
    if (win_win[win_ptr] == NULL) {
        ssnprintf(em0, MAXLEN - 65, "%s, line: %d", __FILE__, __LINE__ - 2);
        ssnprintf(em1, MAXLEN - 65, "newpad(%d, %d) failed", view->lines,
                  PAD_COLS);
        em2[0] = '\0';
        display_error(em0, em1, em2, NULL);
        return -1;
    }
    wbkgrnd(view->win, &CCC_NORM);
    set_tabsize(view->tab_stop);
    wsetscrreg(view->win, 0, view->scroll_lines - 1);
    scrollok(view->win, true);
    // immedok(view->win, true);
    keypad(view->win, true);
    idlok(view->win, false);
    idcok(view->win, false);
    return (0);
}
/** @brief Initialize the input for a C-Menu View.
   This function initializes the input for a view, which can be a file,
   standard input, or a command to be initiated by view. It handles different
   input sources and sets up the necessary file descriptors and memory mapping
   for efficient access.
   @param view Pointer to the View structure to be initialized.
   @param file_name Name of the input file or "-" for standard input.
   @return true on success, false on failure.
   @note if a provider command is specified, set up a pipe to read its output.
   @note The child process executes the command, and the parent process reads
   from the pipe.
   @note If input is from a pipe or standard input, clone it to a temporary
   file. This allows for memory-mapping the input later. Read from stdin and
   write to a temporary file.
 */
bool view_init_input(View *view, char *file_name) {
    struct stat sb;
    int idx = 0;
    pid_t pid;
    int pipe_fd[2];
    char *s_argv[MAXARGS];

    if (strcmp(file_name, "-") == 0) {
        file_name = "/dev/stdin";
        view->f_in_pipe = true;
    }
    if (view->provider_cmd[0] != '\0') {
        str_to_args(s_argv, view->provider_cmd, MAXARGS - 1);
        if (pipe(pipe_fd) == -1) {
            Perror("pipe(pipe_fd) failed in init_view");
            return (1);
        }
        if ((pid = fork()) == -1) {
            Perror("fork() failed in init_view");
            return (1);
        }
        if (pid == 0) { // Child
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
        close(pipe_fd[P_WRITE]);
        dup2(pipe_fd[P_READ], STDIN_FILENO);
        view->in_fd = dup(STDIN_FILENO);
        view->f_in_pipe = true;
    } else {
        if (view->f_in_pipe)
            view->in_fd = dup(STDIN_FILENO);
        else {
            /// Open the input file for reading and get its size.
            expand_tilde(file_name, MAXLEN - 1);
            view->in_fd = open(file_name, O_RDONLY);
            if (view->in_fd == -1) {
                ssnprintf(em0, MAXLEN - 65, "%s, line: %d", __FILE__,
                          __LINE__ - 2);
                ssnprintf(em1, MAXLEN - 65, "open %s", file_name);
                strerror_r(errno, em2, MAXLEN);
                display_error(em0, em1, em2, NULL);
                return false;
            }
            if (fstat(view->in_fd, &sb) == -1) {
                ssnprintf(em0, MAXLEN - 65, "%s, line: %d", __FILE__,
                          __LINE__ - 1);
                ssnprintf(em1, MAXLEN - 65, "fstat %s", file_name);
                strerror_r(errno, em2, MAXLEN);
                display_error(em0, em1, em2, NULL);
                close(view->in_fd);
                return (EXIT_FAILURE);
            }
            view->file_size = sb.st_size;
            if (view->file_size == 0) {
                close(view->in_fd);
                ssnprintf(em0, MAXLEN - 65, "%s, line: %d", __FILE__,
                          __LINE__ - 1);
                ssnprintf(em1, MAXLEN - 65, "file %s is empty", file_name);
                strerror_r(errno, em2, MAXLEN);
                display_error(em0, em1, em2, NULL);
                return (EXIT_FAILURE);
            }
            if (!S_ISREG(sb.st_mode))
                view->f_in_pipe = true;
        }
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
        waitpid(-1, NULL, 0);
    }
    //  Memory-map the input file for efficient access.
    view->buf =
        mmap(NULL, view->file_size, PROT_READ, MAP_PRIVATE, view->in_fd, 0);
    if (view->buf == MAP_FAILED) {
        ssnprintf(em0, MAXLEN - 65, "%s, line: %d", __FILE__, __LINE__ - 2);
        ssnprintf(em1, MAXLEN - 65, "mmap %s", file_name);
        strerror_r(errno, em2, MAXLEN);
        display_error(em0, em1, em2, NULL);
        close(view->in_fd);
        return (EXIT_FAILURE);
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
    return true;
}
