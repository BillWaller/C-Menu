// init_view.c

#include "menu.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
    open_curses(init);
    getmaxyx(stdscr, view->lines, view->cols);
    // Screen    50 lines
    //          - 1 command line
    //          -----------
    //           49 viewable lines
    //
    view->scroll_lines = view->lines - 1;
    view->cmd_line = view->lines - 1;
    view->pminrow = 0;
    view->pmincol = 0;
    view->sminrow = 0;
    view->smincol = 0;
    view->smaxrow = view->lines - 1;
    view->smaxcol = view->cols - 1;

    view->win = newpad(view->lines, MAX_COLS);
    if (view->win == NULL) {
        snprintf(emsg0, MAXLEN - 65, "%s, line: %d", __FILE__, __LINE__);
        snprintf(emsg1, MAXLEN - 65, "newpad(%d, %d) failed", view->lines,
                 MAX_COLS);
        emsg2[0] = '\0';
        display_error(emsg0, emsg1, emsg2);
        abend(-1, "init_view_full_screen: newpad() failed");
    }
    view->box = NULL;

    wcolor_set(view->win, CP_NORM, NULL);
    wbkgd(view->win, COLOR_PAIR(CP_NORM) | ' ');
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
int init_view_boxwin(View *view) {
    char emsg0[MAXLEN];
    char emsg1[MAXLEN];
    char emsg2[MAXLEN];
    if (!f_curses_open) {
        open_curses(init);
    }
    if (win_new(view->lines, view->cols, view->begy, view->begx,
                view->argv[0])) {
        snprintf(emsg0, MAXLEN - 65, "%s, line: %d", __FILE__, __LINE__);
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
        snprintf(emsg0, MAXLEN - 65, "%s, line: %d", __FILE__, __LINE__);
        snprintf(emsg1, MAXLEN - 65, "newpad(%d, %d) failed", view->lines,
                 MAX_COLS);
        emsg2[0] = '\0';
        display_error(emsg0, emsg1, emsg2);
        return -1;
    }
    wbkgd(view->win, COLOR_PAIR(CP_NORM) | ' ');
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
    long bytes_read = 0;
    long pos = 0;
    char emsg0[MAXLEN];
    char emsg1[MAXLEN];
    char emsg2[MAXLEN];

    if (lstat(file_name, &sb) == -1) {
        snprintf(emsg0, MAXLEN - 65, "%s, line: %d", __FILE__, __LINE__);
        snprintf(emsg1, MAXLEN - 65, "lstat %s", file_name);
        strerror_r(errno, emsg2, MAXLEN);
        display_error(emsg0, emsg1, emsg2);
        return false;
    }
    view->buf = (char *)malloc(VBUFSIZ + 1);
    if (view->buf == NULL) {
        snprintf(emsg0, MAXLEN - 65, "%s, line: %d", __FILE__, __LINE__);
        snprintf(emsg1, MAXLEN - 65, "view->buf = malloc(%d): failed\n",
                 VBUFSIZ + 1);
        emsg2[0] = '\0';
        display_error(emsg0, emsg1, emsg2);
        abend(-1, err_msg);
    }
    view->file_size = sb.st_size;
    view->buf_last = view->file_size / VBUFSIZ;
    if (S_ISFIFO(sb.st_mode) || S_ISCHR(sb.st_mode)) {
        if (view->f_pipe_processed) {
            snprintf(emsg0, MAXLEN - 65, "%s, line: %d", __FILE__, __LINE__);
            snprintf(emsg1, MAXLEN - 65, "file status %s", file_name);
            strerror_r(errno, emsg2, MAXLEN);
            emsg2[0] = '\0';
            display_error(emsg0, emsg1, emsg2);
            return false;
        }
        view->fp = fopen(file_name, "r");
        if (view->fp == NULL) {
            snprintf(emsg0, MAXLEN - 65, "%s, line: %d", __FILE__, __LINE__);
            snprintf(emsg1, MAXLEN - 65, "fopen %s", file_name);
            strerror_r(errno, emsg2, MAXLEN);
            display_error(emsg0, emsg1, emsg2);
            return false;
        }
        if (setvbuf(view->fp, view->buf, _IOFBF, VBUFSIZ) != 0) {
            snprintf(emsg0, MAXLEN - 65, "%s, line: %d", __FILE__, __LINE__);
            snprintf(emsg1, MAXLEN - 65, "setvbuf %s", file_name);
            strerror_r(errno, emsg2, MAXLEN);
            display_error(emsg0, emsg1, emsg2);
        }
        view->f_is_pipe = true;
        view->f_pipe_processed = false;
        view->f_new_file = true;
        view->prev_file_pos = NULL_POSITION;
    } else {
        view->fp = fopen(file_name, "r");
        if (view->fp == NULL) {
            snprintf(emsg0, MAXLEN - 65, "%s, line: %d", __FILE__, __LINE__);
            snprintf(emsg1, MAXLEN - 65, "fopen %s", file_name);
            strerror_r(errno, emsg2, MAXLEN);
            display_error(emsg0, emsg1, emsg2);
            return false;
        }
        if (setvbuf(view->fp, view->buf, _IOFBF, VBUFSIZ) != 0) {
            snprintf(emsg0, MAXLEN - 65, "%s, line: %d", __FILE__, __LINE__);
            snprintf(emsg1, MAXLEN - 65, "setvbuf %s", file_name);
            strerror_r(errno, emsg2, MAXLEN);
            display_error(emsg0, emsg1, emsg2);
        }
        view->f_is_pipe = false;
        view->f_pipe_processed = false;
        view->f_new_file = true;
        view->prev_file_pos = NULL_POSITION;
    }
    if (fseek(view->fp, 0L, SEEK_SET) != 0) {
        snprintf(emsg0, MAXLEN - 65, "%s, line: %d", __FILE__, __LINE__);
        snprintf(emsg1, MAXLEN - 65, "seek error: %s", file_name);
        strerror_r(errno, emsg2, MAXLEN);
        display_error(emsg0, emsg1, emsg2);
        return false;
    }
    if (view->file_size == 0L) {
        snprintf(emsg0, MAXLEN - 65, "%s, line: %d", __FILE__, __LINE__);
        snprintf(emsg1, MAXLEN - 65, "empty file: %s", file_name);
        strerror_r(errno, emsg2, MAXLEN);
        display_error(emsg0, emsg1, emsg2);
        return false;
    }
    // warning: The 1st argument to 'fread' is NULL but should not be NULL
    // [unix.S 199 |     bytes_read = fread(view->buf, 1, VBUFSIZ, view->fp);
    // Can't see what ccc analyzer means by that
    if (view->buf == NULL)
        abend(-1, "view->buf is NULL");
    bytes_read = fread(view->buf, 1, VBUFSIZ, view->fp);
    if (bytes_read > 0)
        view->buf_end_ptr = view->buf + bytes_read;
    if (bytes_read < VBUFSIZ) {
        if (!feof(view->fp)) {
            pos = (view->buf_idx * VBUFSIZ) +
                  (long)(view->buf_curr_ptr - view->buf);
            snprintf(emsg0, MAXLEN - 65, "%s, line: %d", __FILE__, __LINE__);
            snprintf(emsg1, MAXLEN - 65, "file: %s, read error at position %ld",
                     file_name, pos);
            strerror_r(errno, emsg2, MAXLEN);
            display_error(emsg0, emsg1, emsg2);
            abend(-1, emsg1);
        }
    }
    view->buf_last = view->file_size / VBUFSIZ;
    view->buf_curr_ptr = view->buf;
    if (view->start_cmd_all_files[0] != '\0')
        strncpy(view->start_cmd, view->start_cmd_all_files, MAXLEN - 1);
    for (idx = 0; idx < NMARKS; idx++)
        view->mark_tbl[idx] = NULL_POSITION;
    strncpy(view->cur_file_str, file_name, MAXLEN - 1);
    if (view->f_stdout_is_tty) {
        view->page_top_pos = 0;
        view->page_bot_pos = 0;
    }
    return true;
}
