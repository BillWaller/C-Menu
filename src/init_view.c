// init_view.c
// Bill Waller 2025

#include "menu.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <unistd.h>

/*  ╭───────────────────────────────────────────────────────────────╮
    │ INIT_VIEW_FULL_SCREEN                                         │
    ╰───────────────────────────────────────────────────────────────╯ */
int init_view_full_screen(Init *init) {
    char emsg0[MAXLEN];
    char emsg1[MAXLEN];
    char emsg2[MAXLEN];
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
    view->win = newpad(view->lines, MAX_COLS);
    if (view->win == NULL) {
        snprintf(emsg0, MAXLEN - 65, "%s, line: %d", __FILE__, __LINE__ - 2);
        snprintf(emsg1, MAXLEN - 65, "newpad(%d, %d) failed", view->lines,
                 MAX_COLS);
        emsg2[0] = '\0';
        display_error(emsg0, emsg1, emsg2);
        abend(-1, "init_view_full_screen: newpad() failed");
    }
    view->box = NULL;
    wbkgd(view->win, COLOR_PAIR(cp_norm) | ' ');
    if (view->tab_stop <= 0)
        view->tab_stop = TABSIZE;
    set_tabsize(view->tab_stop);
    wsetscrreg(view->win, 0, view->scroll_lines - 1);
    scrollok(view->win, true);
    immedok(view->win, true);
    keypad(view->win, true);
    idlok(view->win, false);
    idcok(view->win, false);
    return 0;
}

/*  ╭───────────────────────────────────────────────────────────────╮
    │ INIT_VIEW_BOXWIN                                              │
    ╰───────────────────────────────────────────────────────────────╯ */
int init_view_boxwin(Init *init, char *title) {
    char emsg0[MAXLEN];
    char emsg1[MAXLEN];
    char emsg2[MAXLEN];
    int scr_lines, scr_cols;
    view = init->view;
    view->f_full_screen = false;
    scr_lines = LINES;
    scr_cols = COLS;
    getmaxyx(stdscr, scr_lines, scr_cols);
    if (view->begy + view->lines > scr_lines)
        view->lines = scr_lines;
    if (view->begx + view->cols > scr_cols)
        view->cols = scr_cols;
    if (title != NULL && title[0] != '\0')
        strnz__cpy(view->title, title, MAXLEN - 1);
    else if (view->argv[0] != NULL && view->argv[0][0] != '\0')
        strnz__cpy(view->title, "no title", MAXLEN - 1);
    if (win_new(view->lines, view->cols, view->begy, view->begx, view->title)) {
        snprintf(emsg0, MAXLEN - 65, "%s, line: %d", __FILE__, __LINE__ - 1);
        snprintf(emsg1, MAXLEN - 65, "win_new(%d, %d, %d, %d, %s) failed",
                 view->lines, view->cols, view->begy, view->begx, "NULL");
        emsg2[0] = '\0';
        display_error(emsg0, emsg1, emsg2);
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
    view->win = newpad(view->lines, MAX_COLS);
    if (view->win == NULL) {
        snprintf(emsg0, MAXLEN - 65, "%s, line: %d", __FILE__, __LINE__ - 2);
        snprintf(emsg1, MAXLEN - 65, "newpad(%d, %d) failed", view->lines,
                 MAX_COLS);
        emsg2[0] = '\0';
        display_error(emsg0, emsg1, emsg2);
        return -1;
    }
    wbkgd(view->win, COLOR_PAIR(cp_norm) | ' ');
    set_tabsize(view->tab_stop);
    wsetscrreg(view->win, 0, view->scroll_lines - 1);
    scrollok(view->win, true);
    immedok(view->win, true);
    keypad(view->win, true);
    idlok(view->win, false);
    idcok(view->win, false);
    return (0);
}

/*  ╭───────────────────────────────────────────────────────────────╮
    │ VIEW_INIT_INPUT                                               │
    ╰───────────────────────────────────────────────────────────────╯ */
bool view_init_input(View *view, char *file_name) {
    struct stat sb;
    int idx = 0;
    char emsg0[MAXLEN];
    char emsg1[MAXLEN];
    char emsg2[MAXLEN];
    if (strcmp(file_name, "-") == 0)
        file_name = "/dev/stdin";

    int in_fd = open(file_name, O_RDONLY);
    if (in_fd == -1) {
        snprintf(emsg0, MAXLEN - 65, "%s, line: %d", __FILE__, __LINE__ - 2);
        snprintf(emsg1, MAXLEN - 65, "open %s", file_name);
        strerror_r(errno, emsg2, MAXLEN);
        display_error(emsg0, emsg1, emsg2);
        return false;
    }
    if (fstat(in_fd, &sb) == -1) {
        snprintf(emsg0, MAXLEN - 65, "%s, line: %d", __FILE__, __LINE__ - 1);
        snprintf(emsg1, MAXLEN - 65, "fstat %s", file_name);
        strerror_r(errno, emsg2, MAXLEN);
        display_error(emsg0, emsg1, emsg2);
        close(in_fd);
        return EXIT_FAILURE;
    }
    view->file_size = sb.st_size;
    if (!S_ISREG(sb.st_mode)) {
        char tmp_filename[] = "/tmp/view_XXXXXX";
        char buf[VBUFSIZ];
        ssize_t bytes_read;

        close(in_fd);
        in_fd = mkstemp(tmp_filename);
        if (in_fd == -1)
            abend(-1, "failed to mkstemp");
        unlink(tmp_filename);
        while ((bytes_read = read(STDIN_FILENO, buf, sizeof(buf))) > 0)
            if (write(in_fd, buf, bytes_read) != bytes_read)
                abend(-1, "unable to write tmp");
        if (bytes_read == -1)
            abend(-1, "unable to read stdin");
        if (fstat(in_fd, &sb) == -1)
            abend(-1, "fstat failed");
        view->file_size = sb.st_size;
        if (view->file_size == 0) {
            close(in_fd);
            strnz__cpy(tmp_str, "no standard input", MAXLEN - 1);
            abend(-1, tmp_str);
        }
    }
    view->buf = mmap(NULL, view->file_size, PROT_READ, MAP_PRIVATE, in_fd, 0);
    if (view->buf == MAP_FAILED) {
        snprintf(emsg0, MAXLEN - 65, "%s, line: %d", __FILE__, __LINE__ - 2);
        snprintf(emsg1, MAXLEN - 65, "mmap %s", file_name);
        strerror_r(errno, emsg2, MAXLEN);
        display_error(emsg0, emsg1, emsg2);
        close(in_fd);
        return EXIT_FAILURE;
    }
    close(in_fd);
    view->file_size = sb.st_size;
    view->f_new_file = true;
    view->prev_file_pos = NULL_POSITION;
    view->buf_curr_ptr = view->buf;
    if (view->start_cmd_all_files[0] != '\0')
        strnz__cpy(view->start_cmd, view->start_cmd_all_files, MAXLEN - 1);
    for (idx = 0; idx < NMARKS; idx++)
        view->mark_tbl[idx] = NULL_POSITION;
    strnz__cpy(view->cur_file_str, file_name, MAXLEN - 1);
    if (view->f_stdout_is_tty) {
        view->page_top_pos = 0;
        view->page_bot_pos = 0;
    }
    return true;
}
