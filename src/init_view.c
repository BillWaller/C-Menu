// init_view.c

#include "menu.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <unistd.h>

// void init_view_struct(View *);
// int view_init_input(View *view, char *);

// View *init_view_struct(View *view) {
//     view = (View *)malloc(sizeof(View));
//     if (view == NULL) {
//         (void)sprintf(err_msg, "fview.c,line 111,malloc(%d) failed\n",
//                       (int)sizeof(View));
//         abend(-1, err_msg);
//     }
//     view->fp_prev = NULL_POSITION;
//     view->tab_stop = EIGHT;
//     view->blk_first = NULL;
//     view->prompt_type[0] = 'L';
//     view->start_cmd[0] = '\0';
//     view->start_cmd_all_files[0] = '\0';
//     return (view);
// }

int init_view_stdscr(Init *init) {
    open_curses(init);
    getmaxyx(stdscr, view->lines, view->cols);
    view->last_column = view->cols;
    view->scroll_lines = view->lines - 1;
    view->cmd_line = view->lines - 1;
    view->first_column = 0;
    view->last_column = view->cols;
    view->win = stdscr;
    view->box = NULL;
    wsetscrreg(view->win, 0, view->scroll_lines - 1);
    scrollok(view->win, true);
    immedok(view->win, true);
    keypad(view->win, true);
    idlok(view->win, false);
    idcok(view->win, false);
    return 0;
}

int init_view_boxwin(View *view) {
    if (!f_curses_open) {
        open_curses(init);
    }
    // win_init_attrs(view->fg_color, view->bg_color, view->bo_color);
    if (win_new(view->lines, view->cols, view->begy, view->begx, NULL)) {
        (void)sprintf(tmp_str, "win_new(%d, %d, %d, %d, %s) failed",
                      view->lines, view->cols, view->begy, view->begx, "NULL");
        display_error_message(tmp_str);
        return (-1);
    }
    view->win = win_win[win_ptr];
    view->box = win_box[win_ptr];
    getmaxyx(view->win, view->lines, view->cols);
    view->last_column = view->cols;
    view->scroll_lines = view->lines - 1;
    view->cmd_line = view->lines - 1;
    view->first_column = 0;
    view->last_column = view->cols;
    wsetscrreg(view->win, 0, view->scroll_lines - 1);
    scrollok(view->win, true);
    immedok(view->win, true);
    keypad(view->win, true);
    idlok(view->win, false);
    idcok(view->win, false);
    return (0);
}

bool view_init_input(View *view, char *file_name) {
    struct stat sb;
    int idx = 0;

    if (lstat(file_name, &sb) == -1)
        abend(-1, "lstat failed");
    view->buf = (char *)malloc(VBUFSIZ + 1);
    if (view->buf == NULL) {
        (void)sprintf(err_msg, "view_init_input: malloc(%d) failed\n", VBUFSIZ);
        abend(-1, err_msg);
    }
    view->file_size = sb.st_size;
    view->buf_last = view->file_size / VBUFSIZ;
    if (S_ISFIFO(sb.st_mode) || S_ISCHR(sb.st_mode)) {
        if (view->f_pipe_processed) {
            display_error_message("Unable to view pipe twice");
            return false;
        }
        view->fp = fopen(file_name, "r");
        if (view->fp == NULL) {
            strncpy(tmp_str, "Unable to open pipe ", MAXLEN - 1);
            strncat(tmp_str, file_name, MAXLEN - 1);
            display_error_message(tmp_str);
            return false;
        }
        if (setvbuf(view->fp, view->buf, _IOFBF, VBUFSIZ) != 0) {
            strncpy(tmp_str, "Unable to setvbuf ", MAXLEN - 1);
            strncat(tmp_str, file_name, MAXLEN - 1);
            display_error_message(tmp_str);
        }
        view->f_is_pipe = true;
        view->f_pipe_processed = false;
        view->f_new_file = true;
        view->prev_file_pos = NULL_POSITION;
    } else {
        view->fp = fopen(file_name, "r");
        if (view->fp == NULL) {
            strncpy(tmp_str, "Unable to open file: ", MAXLEN - 1);
            strncat(tmp_str, file_name, MAXLEN - 1);
            display_error_message(tmp_str);
            return false;
        }
        if (setvbuf(view->fp, view->buf, _IOFBF, VBUFSIZ) != 0) {
            strncpy(tmp_str, "Unable to setvbuf ", MAXLEN - 1);
            strncat(tmp_str, file_name, MAXLEN - 1);
            display_error_message(tmp_str);
        }
        view->f_is_pipe = false;
        view->f_pipe_processed = false;
        view->f_new_file = true;
        view->prev_file_pos = NULL_POSITION;
    }
    if (fseek(view->fp, 0L, SEEK_SET) != 0) {
        strncpy(tmp_str, "Error seeking start of file: ", MAXLEN - 1);
        strncat(tmp_str, file_name, MAXLEN - 1);
        display_error_message(tmp_str);
        return false;
    }
    if (view->file_size == 0L) {
        strncpy(tmp_str, file_name, MAXLEN - 1);
        strncat(tmp_str, " is empty", MAXLEN - 1);
        display_error_message(tmp_str);
        return false;
    }
    if (view->start_cmd_all_files[0] != '\0')
        strncpy(view->start_cmd, view->start_cmd_all_files, MAXLEN - 1);
    for (idx = 0; idx < NMARKS; idx++)
        view->mark_tbl[idx] = NULL_POSITION;
    view->buf_idx = NULL_POSITION;
    view->buf_curr_ptr = view->buf;
    view->buf_end_ptr = view->buf + VBUFSIZ;
    strncpy(view->cur_file_str, file_name, MAXLEN - 1);
    if (view->f_stdout_is_tty) {
        view->page_top_pos = 0;
        view->page_bot_pos = 0;
        if (view->prev_file_pos != NULL_POSITION) {
            go_to_position(view, view->prev_file_pos);
            view->prev_file_pos = NULL_POSITION;
        }
    }
    return true;
}
