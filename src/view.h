#include "menu.h"
#include <setjmp.h>

#ifndef _VIEW_H
#define _VIEW_H 1

#define HELPFILE "view.help"
#define VIEWARGS "-i -s -P L"
#define VBUFSIZ 4096
#define PIPEBUFS 16
#define FILEBUFS 4
#define NPOS 32
#define NMARKS 27
#define MAXLEN 256

extern int view_file();

typedef struct {
    long block_no;
    char data[VBUFSIZ + 1];
    char *end_ptr;
} block_struct;

typedef struct {
    long blk_no, last_blk_no, size_bytes, prev_file_pos, file_pos,
        pos_tbl[NPOS], mark_tbl[NMARKS];
    int lines, cols, begx, begy, curx, cury, max_col, next_c, scroll_lines,
        cmd_line, first_column, last_column, last_line, ptop, pbot, blk_offset,
        tabstop, argc, curr_argc, attr, line_mode, fd, f_at_end_clear,
        f_ignore_case, f_squeeze, f_beg_of_file, f_eof, f_forward, f_is_pipe,
        f_new_file, f_pipe_processed, f_redraw_screen, f_displaying_help,
        f_stdout_is_tty, f_at_end_remove;
    char cmd_str[MAXLEN + 1], cur_file_str[MAXLEN + 1], line_str[MAX_COLS + 1],
        line_out_str[MAX_COLS + 1], *argv[MAXARGS], arg_str[MAXLEN + 1],
        startup_cmd_str[MAXLEN + 1], startup_cmd_str_all_files[MAXLEN + 1],
        prompt_type, *def_prompt_ptr, *file_spec_ptr, *next_file_spec_ptr,
        *tmp_file_name_ptr, *line_start_ptr, *line_end_ptr, *buf_start,
        *buf_ptr, *buf_end, *title;
    block_struct *blk_first, *blk_last, *blk_curr;
    jmp_buf cmd_jmp;
    WINDOW *win;
    WINDOW *box;
} view_;

extern view_ *view;
#endif
