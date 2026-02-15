/** @file view.h
    @brief View data structures, enums, types, end external declarations
    @author Bill Waller
    Copyright (c) 2025
    MIT License
    billxwaller@gmail.com
    @date 2026-02-09
 */

#ifndef _VIEW_H
#define _VIEW_H 1

#define _XOPEN_SOURCE_EXTENDED 1
#define NCURSES_WIDECHAR 1
#include <ncursesw/ncurses.h>
#include <signal.h>
#include <stddef.h>
#include <stdlib.h>
#ifndef _COMMON_H
typedef struct Init Init;
#endif

#define NPOS 256
#define NMARKS 256
#define MAXLEN 256
#define NULSL
#define NULL_POSITION -1
#define VBUFSIZ 65536
#define BUFSIZ 8192
#define LINE_IN_PAD_COLS 2048
#define PAD_COLS 4096

enum PROMPT_TYPE { PT_NONE, PT_SHORT, PT_LONG, PT_STRING };

typedef struct {
    int fg_color;              /**< foreground_color */
    int bg_color;              /**< background_color */
    int bo_color;              /**< bold_color */
    int lines;                 /**< number of lines for window size */
    int cols;                  /**< number of columns for window size */
    int begy;                  /**< screen line upper left corner of window */
    int begx;                  /**< screen column upper left corner of window */
    char provider_cmd[MAXLEN]; /**< command provides input */
    char receiver_cmd[MAXLEN]; /**< command receives output */
    char cmd[MAXLEN]; /**< command to execute in foreground, e.g. an editor */
    char cmd_all[MAXLEN]; /**< View - command to execute at start of program */
    char prompt_str[MAXLEN]; /**< prompt string for chyron */
    int prompt_type; /**< View - prompt type for chyron, e.g. 0=short, 1=long,
                        2=none */
    char title[MAXLEN];   /**< display on top line of box window */
    int argc;             /**< command line arguments count */
    char **argv;          /**< command line arguments vector */
    int optind;           /**< getopt pointer to non-option arguments in argv */
    bool f_ignore_case;   /**< View - ignore case in search */
    bool f_at_end_clear;  /**< obsolete, unneeded */
    bool f_at_end_remove; /**< obsolete, unneeded */
    bool f_squeeze; /**< View - print one line for each group of blank lines */
    bool f_stop_on_error;     /**< obsolete, unneeded */
    bool f_multiple_cmd_args; /**< View - put multiple arguments in a single
                                 string */
    WINDOW *win;              /**< ncurses window used by View */
    WINDOW *box; /**< ncurses window used by View for box around win */
    char tmp_prompt_str[MAXLEN]; /**< temporary prompt string used when building
                                    prompt */
    int curr_argc;        /**< current argument count when processing multiple
                             arguments */
    char cmd_arg[MAXLEN]; /**< argument string */
    int tab_stop;         /**< number of spaces between tab stops */
    int next_cmd_char;    /**< index of next character in cmd string to process
                             when building prompt */
    bool f_bod;           /**< beginning of data */
    bool f_eod;           /**< end of data */
    bool f_forward;       /**< motion flag forward */
    bool f_is_pipe;       /**< input is from a pipe */
    char file_name[MAXLEN]; /**< basename of file being viewed */
    bool f_redisplay_page;  /**< flag indicating page needs to be redisplayed */
    bool f_displaying_help; /**< currently didsplaying help information */
    bool f_line_numbers;    /**< not implemented */
    bool f_wrap; /**< wrap long lines to fit the screen - not implemented */
    bool f_full_screen; /**< default mode if lines and columns not specified */
    bool f_timer;       /**< time commands and display elapsed time in prompt */

    bool f_cmd;                        /**< cmd is verified */
    bool f_cmd_all;                    /**< cmd_all is verified */
    char cur_file_str[MAXLEN];         /**< file currently open for viewing */
    char line_in_s[LINE_IN_PAD_COLS];  /**< raw input line from buffer */
    char line_out_s[LINE_IN_PAD_COLS]; /**< scratch buffer */
    char stripped_line_out[PAD_COLS];  /**< printable characters only */
    cchar_t cmplx_buf[PAD_COLS];       /**< complex character buffer */
    char *line_out_p;         /**< pointer to current position in line_out_s */
    unsigned int line_number; /**< currently not implemented */
    char line_number_s[20];   /**< currently not implemented */
    char *line_in_beg_p;      /**< pointer used in matching search targets */
    char *line_in_end_p;      /**< pointer used in matching search targets */
    WINDOW *pad;              /**< ncurses pad used by View */
    int cury;                 /**< cury is the pad row of the cursor location */
    int curx;         /**< curx is the pad column of the cursor location */
    int scroll_lines; /**< number of lines to scroll */
    int cmd_line;     /**< command line location on pad */
    int maxcol;       /**< length of longest line on pad */
    int pminrow;      /**< first pad row displayed in view window */
    int pmincol;      /**< first pad column displayed in view window */
    int sminrow;      /**< screen position of first row of pad displayed in view
                         window */
    int smincol; /**< screen position of first column of pad displayed in view
                    window */
    int smaxrow; /**< screen position of last row of pad displayed in view
                    windiow */
    int smaxcol; /**< screen position of last column of pad displayed in view
                    window */
    int first_match_x;        /**< first column of current search match in
                                 stripped_line_out */
    int last_match_x;         /**< last column of current search match in
                                 stripped_line_out */
    char in_spec[MAXLEN];     /**< input file spec */
    char out_spec[MAXLEN];    /**< output file spec */
    char help_spec[MAXLEN];   /**< help file spec */
    bool f_in_spec;           /**< input file verified */
    bool f_out_spec;          /**< output file verified */
    char *file_spec_ptr;      /**< pointer to current file spec */
    char *next_file_spec_ptr; /**< pointer to next file spec */
    char *tmp_file_name_ptr;  /**< pointer to temporary file spec */
    off_t file_size;          /**< size of file being viewed */
    off_t file_pos;           /**< current file position */
    off_t prev_file_pos;      /**< previous file position */
    off_t page_top_pos;       /**< file position of top line displayed */
    off_t page_bot_pos;       /**< file position of last line displayed */
    off_t srch_beg_pos;       /**< file position when search started */
    off_t mark_tbl[NMARKS];   /**< not implemented */
    bool f_in_pipe;           /**< input is from a pipe */
    int in_fd;                /**< input file descriptor */
    int out_fd;               /**< output file descriptor */
    FILE *in_fp;              /**< pointer to input stream data structure */
    int stdin_fd;             /**< standard input file descriptor */
    FILE *stdin_fp;  /**< pointer to standard input stream data structure */
    int stdout_fd;   /**< standard output file descriptor */
    FILE *stdout_fp; /**< pointer to standard output stream data structure */
    char *buf;       /**< pointer to first byte of virtual file buffer */
    char *
        buf_curr_ptr; /**< pointer to current position in virtual file buffer */
    char *buf_end_ptr; /**< pointer to first byte after end of data in virtual
                          file buffer */
} View;
extern View *view;

extern int get_cmd_spec(View *, char *);
extern void go_to_position(View *, long);
extern void cat_file(View *);
extern char err_msg[MAXLEN];

#endif
