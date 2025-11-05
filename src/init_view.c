// init_view.c

#include "menu.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <unistd.h>

long const NULL_POSITION = (-1);

// void init_view_struct(View *);
// int view_init_input(View *view, char *);

// View *init_view_struct(View *view) {
//     view = (View *)malloc(sizeof(View));
//     if (view == NULL) {
//         (void)sprintf(err_msg, "fview.c,line 111,malloc(%d) failed\n",
//                       (int)sizeof(View));
//         abend(-1, err_msg);
//     }
//     view->prev_file_pos = NULL_POSITION;
//     view->tab_stop = EIGHT;
//     view->blk_first = NULL;
//     view->prompt_type[0] = 'L';
//     view->start_cmd[0] = '\0';
//     view->start_cmd_all_files[0] = '\0';
//     return (view);
// }

int init_view_stdscr(View *view) {
    open_curses();
    win_init_attrs(view->fg_color, view->bg_color, view->bo_color);
    getmaxyx(stdscr, view->lines, view->cols);
    view->last_column = view->cols;
    view->scroll_lines = view->lines - 1;
    view->last_line = view->scroll_lines - 1;
    view->cmd_line = view->lines - 1;
    view->first_column = 0;
    view->last_column = view->cols;
    view->win = stdscr;
    view->box = NULL;
    wsetscrreg(view->win, 0, view->last_line);
    scrollok(view->win, FALSE);
    keypad(view->win, TRUE);
    idcok(view->win, TRUE);
    return 0;
}

int init_view_boxwin(View *view) {
    if (!f_curses_open) {
        open_curses();
    }
    win_init_attrs(view->fg_color, view->bg_color, view->bo_color);
    if (win_new(view->lines, view->cols, view->begy, view->begx, NULL)) {
        (void)sprintf(tmp_str, "win_new(%d, %d, %d, %d, %s) failed",
                      view->lines, view->cols, view->begy, view->begx, "NULL");
        display_error_message(tmp_str);
        return (-1);
    }
    view->win = win_win[win_ptr];
    view->box = win_box[win_ptr];
    view->last_column = view->cols;
    view->scroll_lines = view->lines - 1;
    view->last_line = view->scroll_lines - 1;
    view->cmd_line = view->lines - 1;
    view->first_column = 0;
    view->last_column = view->cols;
    wsetscrreg(view->win, 0, view->last_line);
    scrollok(view->win, FALSE);
    keypad(view->win, TRUE);
    idcok(view->win, TRUE);
    return (0);
}

int view_init_input(View *view, char *file_name) {
    struct stat statbuf;
    int idx = 0;

    // if (view->fd >= 0) {
    //     close(view->fd);
    // }
    // view->fd = Tmpfd;
    if (strcmp(file_name, "-") == 0) {
        if (view->f_pipe_processed) {
            display_error_message("Unable to view pipe twice");
            return (0);
        }
        // Tmpfd = 0;
        view->f_is_pipe = TRUE;
    } else if ((view->fd = open(file_name, O_RDONLY)) >= 0) {
        view->f_is_pipe = FALSE;
        view->f_pipe_processed = FALSE;
        view->prev_file_pos = NULL_POSITION;
    } else {
        strncpy(tmp_str, "Unable to open ", MAXLEN - 1);
        strncat(tmp_str, file_name, MAXLEN - 1);
        display_error_message(tmp_str);
        return (0);
    }
    if (isatty(view->fd)) {
        display_error_message("No input file and no pipe input");
        return (0);
    }
    view->f_new_file = TRUE;

    if (view->f_is_pipe) {
        view->f_pipe_processed = TRUE;
        view->size_bytes = 0L;
        if (initialize_buffers(view, PIPEBUFS)) {
            return (0);
        }
    } else {
        if (lstat(file_name, &statbuf) == -1) {
            strncpy(tmp_str, "Unable to lstat ", MAXLEN - 1);
            strncat(tmp_str, file_name, MAXLEN - 1);
            display_error_message(tmp_str);
            return (0);
        }
        view->file_pos = lseek(view->fd, 0L, SEEK_END);
        view->size_bytes = statbuf.st_size;
        if (view->file_pos != view->size_bytes) {
            (void)snprintf(tmp_str, MAXLEN - 1,
                           "Error seeking end of file: pos %ld, size %ld",
                           view->file_pos, view->size_bytes);
            display_error_message(tmp_str);
            return (0);
        }
        if (view->size_bytes == 0L) {
            strncpy(tmp_str, file_name, MAXLEN - 1);
            strncat(tmp_str, " is empty", MAXLEN - 1);
            display_error_message(tmp_str);
            return (0);
        }
        if (initialize_buffers(view, FILEBUFS)) {
            return (0);
        }
    }
    view->last_blk_no = -1;
    if (view->start_cmd_all_files[0] != '\0') {
        strncpy(view->start_cmd, view->start_cmd_all_files, MAXLEN - 1);
    }
    for (idx = 0; idx < NMARKS; idx++) {
        view->mark_tbl[idx] = NULL_POSITION;
    }
    strncpy(view->cur_file_str, file_name, MAXLEN - 1);
    if (view->f_stdout_is_tty) {
        for (idx = 0; idx < NPOS; idx++) {
            view->pos_tbl[idx] = NULL_POSITION;
        }
        view->ptop = 0;
        view->pbot = view->scroll_lines;
        if (view->prev_file_pos != NULL_POSITION) {
            go_to_position(view, view->prev_file_pos);
            view->prev_file_pos = NULL_POSITION;
        }
    }
    return (1);
}
