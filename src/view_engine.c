/** @file view_engine.c
    @brief The working part of View
    @author Bill Waller
    Copyright (c) 2025
    MIT License
    billxwaller@gmail.com
    @date 2026-02-09
 */

/**
   @defgroup view_engine View Engine
   @brief File mapping, user input, command processing, and display logic
 */

#include <common.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

/** @brief read the next characater from the virtual file
    @ingroup view_engine
   @note Line numbers are tracked when reading forward and stored in a line
   table for quick access when moving backwards.
   @note When reading forward, the End of Data (EOD) flag is set when the
   position is equal to the file size, and cleared when the position or reading
   direction changes.
   @note Carriage-returns are ignored as they should be.
   @note View uses the kernel's demand paged virtual address space to map files
   directly into memory. This allows for efficient access to file contents
   without the need for explicit buffering or read system calls, as the kernel
   handles loading the necessary pages into memory on demand.
 */
#define get_next_char()                                                        \
    {                                                                          \
        c = 0;                                                                 \
        do {                                                                   \
            if (view->file_pos == view->file_size) {                           \
                view->f_eod = true;                                            \
                break;                                                         \
            } else                                                             \
                view->f_eod = false;                                           \
            c = view->buf[view->file_pos++];                                   \
        } while (c == 0x0d);                                                   \
        if (c == '\n')                                                         \
            increment_ln(view);                                                \
    }
/** @brief read the previous characater from the virtual file
    @ingroup view_engine
    @note There is no need to track line numbers when moving backwards as they
   are stored in the line table and accessed as needed.
    @note When reading in reverse, the Beginning of Data (BOD) flag is set when
   the file position is zero, and cleared when the position or reading direction
   changes.
    @note Carriage-returns are ignored as they should be.
 */
#define get_prev_char()                                                        \
    {                                                                          \
        c = 0;                                                                 \
        do {                                                                   \
            if (view->file_pos == 0) {                                         \
                view->f_bod = true;                                            \
                break;                                                         \
            } else                                                             \
                view->f_bod = false;                                           \
            c = view->buf[--view->file_pos];                                   \
        } while (c == 0x0d);                                                   \
    }

char prev_regex_pattern[MAXLEN];
FILE *dbgfp;
int view_file(Init *);
int view_cmd_processor(Init *);
int get_cmd_char(View *, off_t *);
int get_cmd_arg(View *, char *);
void build_prompt(View *);
void cat_file(View *);
void lp(char *);
void go_to_mark(View *, int);
void go_to_eof(View *);
int go_to_line(View *, off_t);
void go_to_percent(View *, int);
void go_to_position(View *, off_t);
bool search(View *, int *, char *);
void next_page(View *);
void prev_page(View *);
void scroll_down_n_lines(View *, int);
void scroll_up_n_lines(View *, int);
off_t get_next_line(View *, off_t);
off_t get_prev_line(View *, off_t);
off_t get_pos_next_line(View *, off_t);
off_t get_pos_prev_line(View *, off_t);
int fmt_line(View *);
void display_line(View *);
void view_display_page(View *);
void parse_ansi_str(char *, attr_t *, int *);
void view_display_help(Init *);
int display_prompt(View *, char *);
void remove_file(View *);
int write_view_buffer(Init *, bool);
bool enter_file_spec(Init *, char *);
int a_toi(char *, bool *);
void increment_ln(View *);
void initialize_line_table(View *);
int pad_refresh(View *);
void sync_ln(View *);
char err_msg[MAXLEN];
void view_restore_wins();

/** @brief Start view
    @ingroup view_engine
    @param init Pointer to the Init structure containing initialization
   parameters and state for the view application. This structure is used to pass
   necessary information and maintain state across different functions within
   the view application.
    @return Returns 0 on successful completion of the view application, or a
   non-zero value if an error occurs during initialization or execution.
 */
int view_file(Init *init) {
    view = init->view;
    if (view->argc < 1) {
        view->curr_argc = -1;
        view->argc = 0;
        view->next_file_spec_ptr = "-";
    } else
        view->next_file_spec_ptr = view->argv[0];
    while (view->curr_argc < view->argc) {
        if (view->next_file_spec_ptr == nullptr ||
            *view->next_file_spec_ptr == '\0') {
            break;
        }
        view->file_spec_ptr = view->next_file_spec_ptr;
        view->next_file_spec_ptr = nullptr;
        strnz__cpy(view->cur_file_str, view->file_spec_ptr, MAXLEN - 1);
        if (view_init_input(view, view->cur_file_str) == 0) {
            if (view->buf) {
                view->f_eod = 0;
                view->f_bod = 0;
                view->maxcol = 0;
                view->page_top_pos = 0;
                view->page_top_ln = 0;
                view->page_bot_ln = 0;
                view->ln_max_pos = 0;
                view->ln = 0;
                view->page_bot_pos = 0;
                view->file_pos = 0;
                initialize_line_table(view);
                next_page(view);
                view_cmd_processor(init);
                munmap(view->buf, view->file_size);
            }
        } else {
            view->curr_argc++;
            if (view->curr_argc < view->argc) {
                view->next_file_spec_ptr = view->argv[view->curr_argc];
            }
        }
    }
    return 0;
}
/** @brief Main Command Processing Loop for View
    @ingroup view_engine
    @param init Pointer to the Init structure containing initialization
   parameters and state for the view application. This structure is used to pass
   necessary information and maintain state across different functions within
   the view application.
*/
int view_cmd_processor(Init *init) {
    char tmp_str[MAXLEN];
    int tfd;
    int c;
    int shift = 0;
    int search_cmd = 0;
    int prev_search_cmd = 0;
    int rc, i;
    ssize_t bytes_written;
    char *e;
    char shell_cmd_spec[MAXLEN];
    off_t n_cmd = 0L;
    off_t prev_file_pos;
    int swidth;
    int max_pmincol;
    view = init->view;
    view->cmd[0] = '\0';
    while (1) {
        if (view->f_redisplay_page) {
            view_display_page(view);
            view->f_redisplay_page = false;
        }
        c = view->next_cmd_char;
        view->next_cmd_char = 0;
        if (!c) {
            build_prompt(view);
            display_prompt(view, view->prompt_str);
            if (view->f_ln) {
                touchwin(view->ln_win);
                wrefresh(view->ln_win);
            }
            c = get_cmd_char(view, &n_cmd);
            if (c >= '0' && c <= '9') {
                tmp_str[0] = (char)c;
                tmp_str[1] = '\0';
                c = get_cmd_arg(view, tmp_str);
            }
        }
        switch (c) {
        case Ctrl(
            'R'): /**<  Ctrl('R') or KEY_RESIZE - Handle terminal resize */
        case 'x':
        case KEY_RESIZE:
            getmaxyx(stdscr, view->lines, view->cols);
#ifdef DEBUG_RESIZE
            ssnprintf(em0, MAXLEN - 1,
                      "view->page_top_ln=%d, resized to lines: %d, cols: %d\n",
                      view->page_top_ln, view->lines, view->cols);
            write_cmenu_log_nt(em0);
#endif
            if (view->f_full_screen) {
                view_full_screen_resize(init);
            } else {
                view_win_resize(init, view->title);
            }
            view->f_redisplay_page = true;
            continue;
        case KEY_ALTHOME: /**< KEY_ALTHOME - horizontal scroll to the first
                             column */
            view->pmincol = 0;
            break;
        case KEY_ALTEND: /**< KEY_ALTEND  horizontal scroll to the last
                          * column
                          */
            if (view->maxcol > view->cols)
                view->pmincol = view->maxcol - view->cols;
            else
                view->pmincol = 0;
            break;
        case 'h': /**< 'h', Ctrl('H'), KEY_LEFT, KEY_BACKSPACE - Horizontal
                     scroll left by two thirds of the page width */
        case Ctrl('H'):
        case KEY_LEFT:
        case KEY_BACKSPACE:
            if (n_cmd <= 0)
                n_cmd = 1;
            shift = (int)n_cmd;
            // swidth = view->smaxcol - view->smincol;
            if (view->pmincol - shift > 0)
                view->pmincol -= shift;
            else
                view->pmincol = 0;
            break;
        case 'l': /**< 'l', 'L', KEY_RIGHT - Horizontal scroll right by two
                     thirds of the page width */
        case 'L':
        case KEY_RIGHT:
            if (n_cmd <= 0)
                n_cmd = 1;
            shift = (int)n_cmd;
            swidth = view->smaxcol - view->smincol + 1;
            if (view->f_ln)
                max_pmincol = view->maxcol - swidth + 8;
            else
                max_pmincol = view->maxcol - swidth;
            if (view->pmincol + shift < max_pmincol)
                view->pmincol += shift;
            else
                view->pmincol = max_pmincol;
            break;
        case 'k': /** 'k', 'K', KEY_UP, Ctrl('K') - Scroll up one line */
        case 'K':
        case KEY_UP:
        case Ctrl('K'):
            if (n_cmd <= 0)
                n_cmd = 1;
            scroll_up_n_lines(view, n_cmd);
            break;
        /** 'j', 'J', KEY_DOWN, KEY_ENTER, SPACE - scroll down one line */
        case 'j':
        case 'J':
        case '\n':
        case ' ':
        case KEY_DOWN:
        case KEY_ENTER:
            if (n_cmd <= 0)
                n_cmd = 1;
            for (i = 0; i < n_cmd; i++) {
                scroll_down_n_lines(view, n_cmd);
            }
            break;
        /** 'b', 'B', Ctrl('B'), KEY_PPAGE - Previous Page */
        case KEY_PPAGE:
        case 'b':
        case 'B':
        case Ctrl('B'):
            prev_page(view);
            break;
        /**  'f', 'F', Ctrl('F'), KEY_NPAGE Next Page */
        case 'f':
        case 'F':
        case KEY_NPAGE:
        case Ctrl('F'):
            next_page(view);
            break;
        /** 'g', KEY_HOME - Go to the beginning of the document */
        case 'g':
        case KEY_HOME:
            view->pmincol = 0;
            go_to_line(view, 0);
            break;
        /**  KEY_LL - Go to the end of the document */
        case KEY_LL:
            go_to_eof(view);
            break;
        /**  '!', Execute Shell Command from within C-Menu View */
        case '!':
            if (view->f_displaying_help)
                break;
            if (get_cmd_arg(view, "!") == 0) {
                if (!view->f_is_pipe) {
                    view->prev_file_pos = view->page_top_pos;
                    view->next_file_spec_ptr = view->file_spec_ptr;
                    str_subc(shell_cmd_spec, view->cmd_arg, '%',
                             view->cur_file_str, MAXLEN - 1);
                } else
                    strnz__cpy(shell_cmd_spec, view->cmd_arg, MAXLEN - 1);
                full_screen_shell(shell_cmd_spec);
                if (!view->f_is_pipe) {
                    view->next_file_spec_ptr = view->cur_file_str;
                    return 0;
                }
            }
            break;
        /**  '+', Set Startup Command */
        case '+':
            if (get_cmd_arg(view, "Startup Command:") == 0)
                strnz__cpy(view->cmd, view->cmd_arg, MAXLEN - 1);
            break;
        /**  '-', Change View Settings */
        case '-':
            display_prompt(view, "(i, n, s, t, or h)->");
            c = get_cmd_char(view, &n_cmd);
            c = S_TOLOWER(c);
            switch (c) {
            /**   -i   ignore_case in search */
            /**   -n   line numbers */
            /**   -s   squeeze multiple blank lines */
            /**   -t n set tab stop columns */
            /**   -h   display help */
            case 'i':
                display_prompt(view, "Ignore Case in search (Y or N)->");
                if ((c = get_cmd_char(view, &n_cmd)) == 'y' || c == 'Y')
                    view->f_ignore_case = true;
                else if (c == 'n' || c == 'N')
                    view->f_ignore_case = false;
                break;
            /**   -n   line numbers */
            case 'n':
                if (view->f_ln)
                    view->f_ln = false;
                else
                    view->f_ln = true;
                view_win_resize(init, view->title);
                view->ln = view->page_top_ln;
                view->file_pos = view->ln_tbl[view->ln];
                view->maxcol = 0;
                view->cury = 0;
                view->page_top_ln = view->ln;
                mvwaddstr(view->pad, view->cmd_line, 0, "       ");
                pad_refresh(view);
                view_display_page(view);
                break;
            /**  -s  Squeeze Multiple Blank Lines */
            case 's':
                display_prompt(
                    view, "view->f_squeeze Multiple Blank lines (Y or N)->");
                if ((c = get_cmd_char(view, &n_cmd)) == 'y' || c == 'Y')
                    view->f_squeeze = true;
                else if (c == 'n' || c == 'N')
                    view->f_squeeze = false;
                break;
            /**  -t  n Set Tab Stop Columns */
            case 't':
                sprintf(tmp_str,
                        "Tabstop Colums Currently %d:", view->tab_stop);
                i = 0;
                if (get_cmd_arg(view, tmp_str) == 0)
                    i = atoi(view->cmd_arg);
                if (i >= 1 && i <= 12) {
                    view->tab_stop = i;
                    view->f_redisplay_page = true;
                } else
                    Perror("Tab stops not changed");
                break;
            /**  -h  Display Help */
            case 'h':
            case KEY_F(1):
                if (!view->f_displaying_help) {
                    view_display_help(init);
                    /** Pedantic reassignment of view pointer as the
                        init->view pointer was was reassigned during
                       view_display_help */
                    view = init->view;
                }
                view->next_cmd_char = '-';
                break;
            default:
                break;
            }
            break;
        /**  ':' - Set a Prompt String */
        case ':':
            view->next_cmd_char = get_cmd_arg(view, ":");
            break;
        /**  'n' - Repeat Previous Search */
        case 'n':
            if (prev_search_cmd == 0 || view->f_search_complete) {
                Perror("No previous search or search complete");
                break;
            }
            if (prev_search_cmd == '/') {
                view->cury = 0;
                view->srch_curr_pos = view->page_bot_pos;
            } else {
                view->cury = view->scroll_lines + 1;
                view->srch_curr_pos = view->page_top_pos;
            }
            search(view, &prev_search_cmd, prev_regex_pattern);
            break;
        /**  '/' or '?' - Search Forward fromk top of page */
        case '/':
            view->f_search_complete = false;
            strnz__cpy(tmp_str, "(forward)->", MAXLEN - 1);
            search_cmd = c;
            c = get_cmd_arg(view, tmp_str);
            if (c == '\n') {
                view->cury = 0;
                view->f_first_iter = true;
                view->srch_beg_pos = view->page_top_pos;
                view->srch_curr_pos = view->page_top_pos;
                search(view, &search_cmd, view->cmd_arg);
                prev_search_cmd = search_cmd;
                strnz__cpy(prev_regex_pattern, view->cmd_arg, MAXLEN - 1);
            }
            break;
        /**  '?' - Search Backward */
        case '?':
            view->f_search_complete = false;
            strnz__cpy(tmp_str, "(backward)->", MAXLEN - 1);
            search_cmd = c;
            c = get_cmd_arg(view, tmp_str);
            if (c == '\n') {
                view->cury = view->scroll_lines;
                view->f_first_iter = true;
                view->srch_beg_pos = view->page_bot_pos;
                view->srch_curr_pos = view->page_bot_pos;
                search(view, &search_cmd, view->cmd_arg);
                prev_search_cmd = search_cmd;
                strnz__cpy(prev_regex_pattern, view->cmd_arg, MAXLEN - 1);
            }
            break;
        /**  'o' or 'O' - Open a File */
        case 'o':
        case 'O':
        case 'e':
        case 'E':
            if (get_cmd_arg(view, "File name:") == 0) {
                strtok(view->cmd_arg, " ");
                view->next_file_spec_ptr = strdup(view->cmd_arg);
                view->f_redisplay_page = true;
                return 0;
            }
            break;
        /**  'g' or 'G' - Go to the End of the Document */
        case 'G':
        case KEY_END:
            if (n_cmd <= 0)
                go_to_eof(view);
            else
                go_to_line(view, n_cmd);
            break;
        /**  'H' or KEY_F(1) - Display Help Information */
        case 'H':
        case KEY_F(1):
            if (!view->f_displaying_help) {
                view_display_help(init);
                view = init->view;
            }
            break;
        /**  'm' - Set a Mark at the Current Position */
        case 'm':
            display_prompt(view, "Mark label (A-Z)->");
            c = get_cmd_char(view, &n_cmd);
            if (c == '@' || c == KEY_F(9) || c == '\033')
                if (c >= 'A' && c <= 'Z')
                    c += ' ';
            if (c < 'a' || c > 'z')
                Perror("Not (a-z)");
            else
                view->mark_tbl[c - 'a'] = view->page_top_pos;
            break;
        /**  'M' - Go to a Mark */
        case 'M':
            display_prompt(view, "Goto mark (A-Z)->");
            c = get_cmd_char(view, &n_cmd);
            if (c == '@' || c == KEY_F(9) || c == '\033')
                break;
            if (c >= 'A' && c <= 'Z')
                c += ' ';
            if (c < 'a' || c > 'z')
                Perror("Not (A-Z)");
            else
                go_to_mark(view, c);
            break;
        /** 'N' - Close Current File and Open Next File */
        case 'N':
            if (n_cmd <= 0)
                n_cmd = 1;
            if (view->curr_argc + n_cmd >= view->argc) {
                Perror("no more files");
                view->curr_argc = view->argc - 1;
            } else {
                view->curr_argc++;
                if (view->curr_argc < view->argc)
                    view->next_file_spec_ptr = view->argv[view->curr_argc];
                return 0;
            }
            break;
        /** 'p' or '%' - Go to a Percent of the File */
        case 'p':
        case '%':
            if (n_cmd < 0)
                go_to_line(view, 1);
            if (n_cmd >= 100)
                go_to_eof(view);
            else
                go_to_percent(view, n_cmd);
            break;
        /**  Ctrl('Z') - Send File to Print Queue with Notation */
        case Ctrl('Z'):
            get_cmd_arg(view, "Enter Notation:");
            strnz__cpy(tmp_str, "/tmp/view-XXXXXX", MAXLEN - 1);
            tfd = mkstemp(tmp_str);
            strnz__cpy(view->tmp_file_name_ptr, tmp_str, MAXLEN - 1);
            if (tfd == -1) {
                Perror("Unable to create temporary file");
                break;
            }
            strnz__cpy(shell_cmd_spec, "echo ", MAXLEN - 5);
            strnz__cat(shell_cmd_spec, view->cmd_arg, MAXLEN - 5);
            strnz__cat(shell_cmd_spec, view->tmp_file_name_ptr, MAXLEN - 5);
            shell(shell_cmd_spec);
            strnz__cpy(shell_cmd_spec, "cat ", MAXLEN - 5);
            strnz__cat(shell_cmd_spec, view->cmd_arg, MAXLEN - 5);
            strnz__cat(shell_cmd_spec, ">>", MAXLEN - 5);
            strnz__cat(shell_cmd_spec, view->tmp_file_name_ptr, MAXLEN - 5);
            shell(shell_cmd_spec);
            lp(view->cur_file_str);
            shell(shell_cmd_spec);
            ssnprintf(shell_cmd_spec, (size_t)(MAXLEN - 5), "rm %s",
                      view->tmp_file_name_ptr);
            strnz__cpy(shell_cmd_spec, "rm ", MAXLEN - 5);
            strnz__cat(shell_cmd_spec, view->tmp_file_name_ptr, MAXLEN - 5);
            shell(shell_cmd_spec);
            restore_wins();
            view->f_redisplay_page = true;
            unlink(tmp_str);
            break;
        /** 'P' or KEY_CATAB or KEY_PRINT - Print Current File */
        case Ctrl('P'):
        case KEY_CATAB:
        case KEY_PRINT:
            lp(view->cur_file_str);
            view->f_redisplay_page = true;
            break;
        /** 'P' or KEY_F(9) or ESC - Close Current File and Open Next */
        case 'P':
            if (n_cmd <= 0)
                n_cmd = 1;
            if (view->curr_argc - n_cmd < 0) {
                Perror("No previous file");
                view->curr_argc = 0;
            } else {
                view->curr_argc--;
                if (view->curr_argc >= 0)
                    view->next_file_spec_ptr = view->argv[view->curr_argc];
                return 0;
            }
            break;
        /**  'q' or 'Q' or KEY_F(9) or ESC - Quit the Application */
        case 'q':
        case 'Q':
        case KEY_F(9):
        case '\033':
            view->curr_argc = view->argc;
            view->next_file_spec_ptr = nullptr;
            return 0;
        /** 'v' - Open Current File in Editor */
        case 'v':
            if (init->editor[0] == 0) {
                e = getenv("DEFAULTEDITOR");
                if (e == nullptr || *e == '\0')
                    strnz__cpy(init->editor, DEFAULTEDITOR, MAXLEN);
                else
                    strnz__cpy(init->editor, e, MAXLEN);
            }
            strnz__cpy(em0,
                       "View doesn't support editing current buffer directly",
                       MAXLEN - 1);
            strnz__cpy(em1, "Would you like to write the buffer to a file?",
                       MAXLEN - 1);
            strnz__cpy(em2, "Enter Y for yes or any other key to cancel.",
                       MAXLEN - 1);
            rc = display_error(em0, em1, em2, nullptr);
            if (rc != 'y' && rc != 'Y')
                break;
            if (!enter_file_spec(init, view->out_spec)) {
                view->f_redisplay_page = true;
                break;
            }
            prev_file_pos = view->page_top_pos;
            bytes_written = write_view_buffer(init, view->f_strip_ansi);
            if (bytes_written == 0) {
                ssnprintf(em0, MAXLEN - 1, "%s, line: %d", __FILE__,
                          __LINE__ - 2);
                strnz__cpy(em1, "0 bytes written", MAXLEN - 1);
                strerror_r(errno, em1, MAXLEN - 1);
                display_error(em0, em1, nullptr, nullptr);
                break;
            }
            view->next_file_spec_ptr = view->in_spec;
            strnz__cpy(shell_cmd_spec, init->editor, MAXLEN - 5);
            strnz__cat(shell_cmd_spec, " ", MAXLEN - 5);
            strnz__cat(shell_cmd_spec, view->in_spec, MAXLEN - 5);
            full_screen_shell(shell_cmd_spec);
            view->file_pos = view->page_top_pos = view->page_bot_pos =
                prev_file_pos;

            restore_wins();
            view->f_redisplay_page = true;
            return 0;
        /** 'w' - Write the current buffer to file */
        case 'w':
            if (!enter_file_spec(init, view->out_spec)) {
                view->f_redisplay_page = true;
                break;
            }
            // prev_file_pos = view->page_top_pos;
            bytes_written = write_view_buffer(init, view->f_strip_ansi);
            if (bytes_written == 0) {
                ssnprintf(em0, MAXLEN - 1, "%s, line: %d", __FILE__,
                          __LINE__ - 2);
                strnz__cpy(em1, "0 bytes written", MAXLEN - 1);
                strerror_r(errno, em1, MAXLEN - 1);
                display_error(em0, em1, nullptr, nullptr);
                break;
            }
            display_prompt(view, tmp_str);
            view->f_redisplay_page = true;
            break;
        case CT_VIEW:
            break;
        /** 'V' - Display Version Information */
        case 'V':
            ssnprintf(em0, MAXLEN - 1, "C-Menu Version: %s", CM_VERSION);
            display_error(em0, em1, nullptr, nullptr);
            break;
        default:
            break;
        }
        view->cmd_arg[0] = '\0';
    }
}
/** @brief Get Command Character and Numeric Argument
    @ingroup view_engine
    @param view Pointer to the View structure containing the state around
   the view application. This structure is used to access and modify the
   state
    @param n is used to store the numeric argument entered by the user, if
   applicable. The function reads user input and extracts both the command
   character and any numeric argument, allowing for commands that require a
   numeric parameter to be processed effectively.
    @return Returns the command character entered by the user, or a special
   value if a mouse event is detected. The numeric argument is stored in the
   variable pointed to by n if applicable.
 */
int get_cmd_char(View *view, off_t *n) {
    int c = 0, i = 0;
    char cmd_str[33];
    cmd_str[0] = '\0';
    pad_refresh(view);
    wmove(view->cmdln_win, view->cmd_line, view->curx);
    do {
        c = xwgetch(view->cmdln_win, nullptr, 0);
        if ((c >= '0' && c <= '9') && i < 32) {
            cmd_str[i++] = (char)c;
            cmd_str[i] = '\0';
        } else {
            if (c >= 256)
                return c;
            else
                continue;
        }
    } while (c >= '0' && c <= '9');
    *n = atol(cmd_str);
    view->cmd_arg[0] = '\0';
    return (c);
}
/** @brief Get Command Argument from User Input
    @ingroup view_engine
    @param view Pointer to the View structure containing the state and
   parameters of the view application. This structure is used to access and
   modify the state of the application as needed.
    @param prompt A string containing the prompt to be displayed to the user
   when requesting input for the command argument. This prompt is shown on
   the command line to guide the user in providing the necessary input for
   the command being executed.
    @return Returns the command character entered by the user, or a special
   value if a mouse event is detected. The command argument entered by the
   user is stored in the view->cmd_arg buffer for use by the calling
   function. The function handles user input, including editing keys, and
   updates the command argument buffer accordingly. If the user enters a
   numeric argument, it is validated based on the context of the command
   being executed.
*/
int get_cmd_arg(View *view, char *prompt) {
    int c;
    int numeric_arg = false;
    char *cmd_p;
    char *cmd_e;
    char prompt_s[PAD_COLS + 1];
    char *n;
    int prompt_l;
    prompt_l = strnz__cpy(prompt_s, prompt, view->cols - 4);
    if (view->cmd_arg[0] != '\0')
        return 0;
    cmd_p = view->cmd_arg;
    cmd_e = view->cmd_arg + MAXLEN - 2;
    wmove(view->cmdln_win, view->cmd_line, 0);
    if (prompt_l == 0)
        numeric_arg = true;
    if (prompt_l > 1) {
        wstandout(view->cmdln_win);
        waddch(view->cmdln_win, ' ');
        waddstr(view->cmdln_win, prompt_s);
        waddch(view->cmdln_win, ' ');
        wstandend(view->cmdln_win);
    } else {
        if (*prompt == ':')
            numeric_arg = true;
        else {
            n = prompt;
            if (*n >= '0' && *n <= '9') {
                *cmd_p++ = *n;
                *cmd_p = '\0';
                numeric_arg = true;
            }
        }
        waddstr(view->cmdln_win, prompt_s);
        wmove(view->cmdln_win, view->cmd_line, prompt_l);
    }
    wclrtoeol(view->cmdln_win);
    while (1) {
        c = xwgetch(view->cmdln_win, nullptr, -1);
        switch (c) {
        /** Basic Editing Keys for Command Line */
        case KEY_LEFT:
        case KEY_BACKSPACE:
        case '\b':
            if (cmd_p > view->cmd_arg) {
                cmd_p--;
                if (*cmd_p < ' ' || *cmd_p == 0x7f) {
                    getyx(view->cmdln_win, view->cury, view->curx);
                    if (view->curx > 0) {
                        view->curx--;
                        wmove(view->cmdln_win, view->cmd_line, view->curx);
                        waddch(view->cmdln_win, ' ');
                        wmove(view->cmdln_win, view->cmd_line, view->curx);
                    }
                }
                getyx(view->cmdln_win, view->cury, view->curx);
                if (view->curx > 0) {
                    view->curx--;
                    wmove(view->cmdln_win, view->cmd_line, view->curx);
                    waddch(view->cmdln_win, ' ');
                    wmove(view->cmdln_win, view->cmd_line, view->curx);
                }
            }
            break;
        case '\n':
        case KEY_ENTER:
            return c;
        case '\033':
        case KEY_F(9):
            return c;
        case KEY_MOUSE:
            continue;
        default:
            *cmd_p++ = (char)c;
            *cmd_p = '\0';
            if ((char)c < ' ') {
                waddch(view->cmdln_win, '^');
                c |= '@';
            } else if ((uchar)c == 0x7f)
                c = '?';
            waddch(view->cmdln_win, (char)c);
            if (cmd_p >= cmd_e)
                return 0;
            if (numeric_arg && (c < '0' || c > '9'))
                return -1;
            break;
        }
    }
    return c;
}
/** @brief Build Prompt String
    @ingroup view_engine
    @param view Pointer to the View structure containing the state and
   parameters of the view application. This structure is used to access and
   modify the state of the application as needed.
 */
void build_prompt(View *view) {
    char tmp_str[MAXLEN];
    if (view->f_is_pipe)
        strnz__cpy(view->prompt_str, "stdin", MAXLEN - 1);
    else
        strnz__cpy(view->prompt_str, view->file_name, MAXLEN - 1);
    if (view->pmincol > 0) {
        sprintf(tmp_str, "Col %d of %d", view->pmincol, view->maxcol);
        if (view->prompt_str[0] != '\0')
            strnz__cat(view->prompt_str, "|", MAXLEN - 1);
        strnz__cat(view->prompt_str, tmp_str, MAXLEN - 1);
    }
    if (view->argc > 0) {
        sprintf(tmp_str, "File %d of %d", view->curr_argc + 1, view->argc);
        if (view->prompt_str[0] != '\0') {
            strnz__cat(view->prompt_str, "|", MAXLEN - 1);
            strnz__cat(view->prompt_str, tmp_str, MAXLEN - 1);
        }
    }
    if (view->page_top_pos == NULL_POSITION)
        view->page_top_pos = view->file_size;
    sprintf(tmp_str, "Pos %zd-%zd", view->page_top_pos, view->page_bot_pos);
    if (view->prompt_str[0] != '\0') {
        strnz__cat(view->prompt_str, "|", MAXLEN - 1);
        strnz__cat(view->prompt_str, tmp_str, MAXLEN - 1);
    }
    if (!view->f_is_pipe) {
        if (view->file_size > 0) {
            sprintf(tmp_str, " of %zd", view->file_size);
            strnz__cat(view->prompt_str, tmp_str, MAXLEN - 1);
        }
    }
    if (view->f_eod) {
        if (view->prompt_str[0] != '\0')
            strnz__cat(view->prompt_str, " ", MAXLEN - 1);
        strnz__cat(view->prompt_str, "(End)", MAXLEN - 1);
        if (view->curr_argc + 1 < view->argc) {
            base_name(tmp_str, view->argv[view->curr_argc + 1]);
            strnz__cpy(view->prompt_str, " Next File: ", MAXLEN - 1);
            strnz__cat(view->prompt_str, tmp_str, MAXLEN - 1);
        }
    }
}
/** @brief Write buffer contents to files
    @ingroup view_engine
    @param init data structure
    @param f_strip_ansi strip ANSI escape sequences
    @details
   */
int write_view_buffer(Init *init, bool f_strip_ansi) {
    ssize_t bytes_written = 0;
    off_t pos;
    View *view = init->view;
    int rc;
    size_t l;
    char tmp_line_s[PAD_COLS];
    if (!f_strip_ansi) {
        strnz__cpy(em0, "Would you like to strip ansi escape sequences?",
                   MAXLEN - 1);
        strnz__cpy(em1, "Enter Y for yes or any other key to cancel.",
                   MAXLEN - 1);
        rc = answer_yn(nullptr, em0, em1, nullptr);
        if (rc == 'y' || rc == 'Y')
            f_strip_ansi = true;
        else
            f_strip_ansi = false;
    }
    restore_wins();
    /** write the buffer */
    pos = 0;
    view->out_fd = open(view->out_spec, O_CREAT | O_TRUNC | O_WRONLY, 0644);
    if (view->out_fd == -1) {
        ssnprintf(em0, MAXLEN - 1, "%s, line: %d", __FILE__, __LINE__ - 2);
        strnz__cpy(em1, "fwrite ", MAXLEN - 1);
        strnz__cat(em1, view->out_spec, MAXLEN - 1);
        strerror_r(errno, em2, MAXLEN - 1);
        display_error(em0, em1, em2, nullptr);
        return false;
    }
    bytes_written = 0;
    while (!view->f_eod) {
        pos = get_next_line(view, pos);
        if (f_strip_ansi)
            strip_ansi(tmp_line_s, view->line_in_s);
        else
            strnz__cpy(tmp_line_s, view->line_in_s, MAXLEN - 1);
        l = strnlf(tmp_line_s, PAD_COLS - 1);
        bytes_written += write(view->out_fd, tmp_line_s, l);
    }
    close(view->out_fd);
    strnz__cpy(view->in_spec, view->out_spec, MAXLEN - 1);
    return bytes_written;
}
/** @brief Concatenate File to Standard Output
    @ingroup view_engine
 */
void cat_file(View *view) {
    int c;
    while (1) {
        get_next_char();
        if (view->f_eod)
            break;
        putchar(c);
    }
}
/** @brief Send File to Print Queue
    @ingroup view_engine
    @param PrintFile - file to print */
void lp(char *PrintFile) {
    char *print_cmd_ptr;
    char shell_cmd_spec[MAXLEN];
    print_cmd_ptr = getenv("PRINTCMD");
    if (print_cmd_ptr == nullptr || *print_cmd_ptr == '\0')
        print_cmd_ptr = PRINTCMD;
    ssnprintf(shell_cmd_spec, MAXLEN - 1, "%s %s", print_cmd_ptr, PrintFile);
    display_prompt(view, shell_cmd_spec);
    shell(shell_cmd_spec);
}
/** @defgroup view_navigation View Navigation
    @brief Navigation functions for the view application
 */
/** @brief Go to Mark
    @ingroup view_navigation
    @param view Pointer to the View structure containing the state and
   parameters of the view application. This structure is used to access and
   modify the state of the application as needed.
    @param c The character representing the mark to go to. This character is
   typically a lowercase letter (a-z) corresponding to a mark that has been
   set previously in the view application. The function will attempt to
   navigate to the position associated with this mark, allowing the user to
   quickly jump to specific locations in the file based on the marks they
   have set. If the mark is not set, an error message will be displayed to
   the user.
 */
void go_to_mark(View *view, int c) {
    if (c == '\'')
        view->file_pos = view->mark_tbl[(NMARKS - 1)];
    else
        view->file_pos = view->mark_tbl[c - 'a'];
    if (view->file_pos == NULL_POSITION)
        Perror("Mark not set");
    else
        go_to_position(view, view->file_pos);
}
/** @brief Search for Regular Expression Pattern
    @ingroup view_navigation
    @param view Pointer to View Structure
    @param search_cmd Search Command Character ('/' or '?')
    @param regex_pattern Regular Expression Pattern to Search For
    @returns true if a match is found and displayed, false if the search
   completes without finding a match or if an error occurs
    @note The search performs extended regular expression matching, ignoring
   ANSI sequences and Unicode characters. Matches are highlighted on the
   screen, and the search continues until the page is full or the end of the
   file is reached. If the search wraps around the file, a message is
   displayed indicating that the search is complete.
    @note The search state is maintained in the view structure, allowing for
   repeat searches and tracking of the current search position. @note this
   function highlights all matches in the current ncurses pad, including
   those not displayed on the screen, and tracks the first and last match
   columns for prompt display.
    @note ANSI sequences and Unicode characters are stripped before
   matching, so matching corresponds to the visual display */
bool search(View *view, int *search_cmd, char *regex_pattern) {
    char tmp_str[MAXLEN];
    int REG_FLAGS = 0;
    regmatch_t pmatch[1];
    regex_t compiled_regex;
    int reti;
    int line_offset;
    int line_len;
    int match_len;
    off_t prev_ln;
    bool f_page = false;
    if (*regex_pattern == '\0')
        return false;
    if (view->f_ignore_case)
        REG_FLAGS = REG_ICASE | REG_EXTENDED;
    else
        REG_FLAGS = REG_EXTENDED;
    reti = regcomp(&compiled_regex, regex_pattern, REG_FLAGS);
    if (reti) {
        Perror("Invalid pattern");
        return false;
    }
    /**  */
    while (1) {
        /** initialize iteration */
        if (*search_cmd == '/') {
            if (view->srch_curr_pos == view->file_size)
                view->srch_curr_pos = 0;
        } else {
            if (view->srch_curr_pos == 0)
                view->srch_curr_pos = view->file_size;
        }
        if (view->srch_curr_pos == view->srch_beg_pos) {
            if (view->f_first_iter == true) {
                view->f_first_iter = false;
                view->f_search_complete = false;
                if (*search_cmd == '/')
                    view->cury = 0;
                else
                    view->cury = view->scroll_lines + 1;
            } else {
                view->f_search_complete = true;
                return true;
            }
        }
        view->file_pos = view->srch_curr_pos;
        sync_ln(view);
        /** get line to scan */
        if (*search_cmd == '/') {
            if (view->cury == view->scroll_lines)
                return true;
            prev_ln = view->ln; /**< Note placement before get_next_line */
            view->srch_curr_pos = get_next_line(view, view->srch_curr_pos);
            view->page_bot_pos = view->srch_curr_pos;
        } else {
            if (view->cury == 0)
                return true;
            view->srch_curr_pos = get_prev_line(view, view->srch_curr_pos);
            prev_ln = view->ln; /**< Note placement after get_prev_line */
            view->page_top_pos = view->srch_curr_pos;
        }
        fmt_line(view);
        reti = regexec(&compiled_regex, view->stripped_line_out,
                       compiled_regex.re_nsub + 1, pmatch, REG_FLAGS);
        if (reti == REG_NOMATCH) {
            if (f_page) {
                /** non-matching page filler */
                if (*search_cmd == '?')
                    view->cury -= 2;
                display_line(view);
                if ((*search_cmd == '/' && view->cury == view->scroll_lines) ||
                    (*search_cmd == '?' && view->cury == 1)) {
                    break;
                }
            }
            continue;
        }
        if (reti) {
            char err_str[MAXLEN];
            regerror(reti, &compiled_regex, err_str, sizeof(err_str));
            strnz__cpy(tmp_str, "Regex match failed: ", MAXLEN - 1);
            strnz__cat(tmp_str, err_str, MAXLEN - 1);
            Perror(tmp_str);
            regfree(&compiled_regex);
            return false;
        }
        /** Display matching lines */
        if (!f_page) {
            if (*search_cmd == '/') {
                view->page_top_ln = prev_ln;
                wmove(view->pad, view->cury, 0);
            } else {
                view->page_bot_ln = view->ln;
                wmove(view->pad, 0, 0);
            }
            wclrtobot(view->pad);
            f_page = true;
        }
        if (*search_cmd == '?')
            view->cury -= 2;
        display_line(view);
        /** All matches on the current line are highlighted,
           including those not displayed on the screen.
           Track first and last match columns for prompt display. */
        view->first_match_x = -1;
        view->last_match_x = 0;
        line_len = strlen(view->stripped_line_out);

        line_offset = 0;
        while (1) {
            view->curx = line_offset + pmatch[0].rm_so;
            match_len = pmatch[0].rm_eo - pmatch[0].rm_so;
            mvwchgat(view->pad, view->cury - 1, view->curx, match_len,
                     WA_REVERSE, cp_win, nullptr);
            if (view->first_match_x == -1)
                view->first_match_x = pmatch[0].rm_so;
            view->last_match_x = line_offset + pmatch[0].rm_eo;
            line_offset += pmatch[0].rm_eo;
            if (line_offset >= line_len)
                break;
            view->line_out_p = view->stripped_line_out + line_offset;
            reti = regexec(&compiled_regex, view->line_out_p,
                           compiled_regex.re_nsub + 1, pmatch, REG_FLAGS);
            if (reti == REG_NOMATCH)
                break;
            if (reti) {
                char msgbuf[100];
                regerror(reti, &compiled_regex, msgbuf, sizeof(msgbuf));
                sprintf(tmp_str, "Regex match failed: %s", msgbuf);
                Perror(tmp_str);
                regfree(&compiled_regex);
                return false;
            }
            if (*search_cmd == '/') {
                if (view->cury == view->scroll_lines) {
                    regfree(&compiled_regex);
                    return true;
                }
            } else if (view->cury == 1) {
                regfree(&compiled_regex);
                return true;
            }
        }
    }
    view->file_pos = view->srch_curr_pos;
#ifdef DEBUG_SEARCH
    /** Statistics for debugging */
    ssnprintf(view->tmp_prompt_str, MAXLEN - 1,
              "%s|%c%s|Pos %zu-%zu|(%zd) %zu %zu", view->file_name, *search_cmd,
              regex_pattern, view->page_top_pos, view->page_bot_pos,
              view->file_size, view->srch_beg_pos, view->srch_curr_pos);
#else
    if (view->last_match_x > view->maxcol)
        ssnprintf(view->tmp_prompt_str, MAXLEN - 1,
                  "%s|%c%s|Match Cols %d-%d of %d-%d|(%zd%%)", view->file_name,
                  *search_cmd, regex_pattern, view->first_match_x,
                  view->last_match_x, view->pmincol, view->smaxcol - view->begx,
                  (view->page_bot_pos * 100 / view->file_size));
    else
        ssnprintf(view->tmp_prompt_str, MAXLEN - 1,
                  "%s|%c%s|Pos %zu-%zu|(%zd%%)", view->file_name, *search_cmd,
                  regex_pattern, view->page_top_pos, view->page_bot_pos,
                  (view->page_bot_pos * 100 / view->file_size));
#endif
    regfree(&compiled_regex);
    return true;
}

/** @defgroup view_display Manage View Display
    @brief Manage the View Display
 */
/** @brief Refresh Pad and Line Number Window
    @ingroup view_display
    @param view data structure
    @returns OK on success, ERR on failure
*/
int pad_refresh(View *view) {
    int rc;
    if (view->f_ln)
        rc = prefresh(view->pad, view->pminrow, view->pmincol, view->sminrow,
                      view->smincol + 8, view->smaxrow, view->smaxcol);
    else
        rc = prefresh(view->pad, view->pminrow, view->pmincol, view->sminrow,
                      view->smincol, view->smaxrow, view->smaxcol);
    if (rc == ERR)
        Perror("Error refreshing screen");
    wrefresh(view->cmdln_win);
    if (view->f_ln)
        wrefresh(view->ln_win);
    return rc;
}
/*--------------------------------------------------------------
   Navigation
 *--------------------------------------------------------------- */
/** @brief display previous page
    @ingroup view_navigation
    @param view data structure
    @details Displays the previous page starting at (view->page_top_ln -
   view->scroll_lines).
 */
void prev_page(View *view) {
    if (view->page_top_pos == 0)
        return;
    view->cury = 0;
    view->ln = view->page_top_ln;
    if (view->ln - view->scroll_lines >= 0)
        view->ln -= view->scroll_lines;
    else
        view->ln = 0;
    next_page(view);
}
/** @brief Advance to Next Page
    @ingroup view_navigation
    @param view data structure
    @details Advances from view->page_bot_pos to view the next page of
   content. view->page_bot_pos must be set properly when calling this
   function. If the current bottom position of the page is at the end of the
   file, the function returns without making any changes. Otherwise, it
   resets the maximum column and current line position to the top of the
   page, updates the file position to the current bottom position of the
   page, and sets the top position and line number of the page accordingly.
   Finally, it calls the function to display the new page content.
*/
void next_page(View *view) {
    view->file_pos = view->ln_tbl[view->ln];
    if (view->file_pos == view->file_size)
        return;
    view->maxcol = 0;
    view->cury = 0;
    view->page_top_ln = view->ln;
    view_display_page(view);
}
/** @brief Display Current Page
    @ingroup view_display
    @param view data structure
 */
void view_display_page(View *view) {
    int i;
    view->cury = 0;
    wmove(view->pad, 0, 0);
    if (view->ln_win)
        wmove(view->ln_win, 0, 0);
    view->ln = view->page_top_ln;
    view->page_bot_ln = view->ln;
    view->page_top_pos = view->ln_tbl[view->ln];
    view->file_pos = view->page_top_pos;
    view->page_bot_pos = view->file_pos;
    for (i = 0; i < view->scroll_lines; i++) {
        view->page_bot_pos = get_next_line(view, view->page_bot_pos);
        if (view->f_eod)
            break;
        fmt_line(view);
        display_line(view);
    }
    if (view->cury < view->scroll_lines) {
        wmove(view->ln_win, view->cury, 0);
        wclrtobot(view->ln_win);
        wmove(view->pad, view->cury, 0);
        wclrtobot(view->pad);
    }
    view->page_bot_ln = view->ln;
}
/** @brief Scroll N Lines
    @ingroup view_navigation
    @param view data Structure
    @param n number of lines to scroll
 */
void scroll_down_n_lines(View *view, int n) {
    int i = 0;
    view->f_bod = false;
    if (view->page_bot_pos == view->file_size)
        return;
    /** Locate New Top of Page */
    if (view->page_top_ln + n < view->ln) {
        view->page_top_ln += n;
        view->page_top_pos = view->ln_tbl[view->page_top_ln];
    } else
        view->page_top_ln = view->ln;
    /** Scroll */
    if (n > view->scroll_lines) {
        if (view->f_ln) {
            wmove(view->ln_win, 0, 0);
            wclrtobot(view->ln_win);
        }
        wmove(view->pad, 0, 0);
        wclrtobot(view->pad);
    } else {
        if (view->f_ln)
            wscrl(view->ln_win, n);
        wscrl(view->pad, n);
        if (view->cury + n < view->scroll_lines)
            view->cury = view->scroll_lines - n;
    }
    /** Fill in Page Bottom */
    wmove(view->pad, view->cury, 0);
    for (i = 0; i < n; i++) {
        view->page_bot_pos = get_next_line(view, view->page_bot_pos);
        view->page_bot_ln = view->ln;
        if (view->f_eod)
            break;
        fmt_line(view);
        display_line(view);
    }
}
/** @brief Scroll Up N Lines
    @ingroup view_navigation
    @param view data Structure
    @param n number of lines to scroll
 */
void scroll_up_n_lines(View *view, int n) {
    int i;
    view->page_top_pos = view->ln_tbl[view->page_top_ln];
    view->f_eod = false;
    if (view->page_top_pos == 0)
        return;
    if (view->page_top_ln - n >= 0) {
        view->page_top_ln -= n;
    } else
        view->page_top_ln = 1;
    view->ln = view->page_top_ln;
    view->page_top_pos = view->ln_tbl[view->page_top_ln];
    view->file_pos = view->page_top_pos;

    if (view->f_ln) {
        wscrl(view->ln_win, -n);
        wnoutrefresh(view->ln_win);
    }
    wscrl(view->pad, -n);
    view->cury = 0;
    wmove(view->pad, view->cury, 0);
    /** Fill in Page Top */
    for (i = 0; i < n; i++) {
        view->file_pos = get_next_line(view, view->file_pos);
        if (view->f_eod)
            break;
        fmt_line(view);
        display_line(view);
    }
    if (view->page_bot_ln - n >= view->scroll_lines)
        view->page_bot_ln -= n;
    else
        view->page_bot_ln = view->scroll_lines;
    view->ln = view->page_bot_ln;
    view->page_bot_pos = view->ln_tbl[view->page_bot_ln];
    view->file_pos = view->page_bot_pos;
    return;
}
/** @brief Get Next Line from View->buf
    @ingroup view_navigation
    @param view struct
    @param pos buffer offset
    @returns file position of next line
    @note gets view->line_in_s
 */
off_t get_next_line(View *view, off_t pos) {
    char c;
    char *line_in_p;

    view->file_pos = pos;
    view->f_eod = false;
    get_next_char();
    if (view->f_eod)
        return view->file_pos;
    line_in_p = view->line_in_s;
    view->line_in_beg_p = view->line_in_s;
    view->line_in_end_p = view->line_in_s + PAD_COLS;
    while (1) {
        if (c == '\n')
            break;
        if (line_in_p >= view->line_in_end_p)
            break;
        *line_in_p++ = c;
        get_next_char();
        if (view->f_eod)
            return view->file_pos;
    }
    *line_in_p = '\0';
    if (view->f_squeeze) {
        while (1) {
            get_next_char();
            if (c != '\n')
                break;
            if (view->f_eod)
                break;
        }
        get_prev_char();
        if (view->f_eod)
            return view->file_pos;
    }
    return view->file_pos;
}
/** @brief Get Previous Line from View->buf
    @ingroup view_navigation
   @param view data structure
   @param pos buffer offset
   @returns file position of previous line
 */
off_t get_prev_line(View *view, off_t pos) {
    pos = get_pos_prev_line(view, pos);
    view->file_pos = pos;
    get_next_line(view, view->file_pos);
    return pos;
}
/** @brief Get Position of Next Line
    @ingroup view_navigation
   @param view data structure
   @param pos buffer offset
   @returns file position of next line
   */
off_t get_pos_next_line(View *view, off_t pos) {
    char c;
    if (pos == view->file_size) {
        view->f_eod = true;
        return view->file_pos;
    } else
        view->f_eod = false;
    view->file_pos = pos;
    get_next_char();
    if (view->f_eod)
        return view->file_pos;
    if (view->f_squeeze) {
        while (1) {
            if (c != '\n')
                break;
            get_next_char();
            if (view->f_eod)
                return view->file_pos;
        }
        get_prev_char();
    }
    while (!view->f_eod) {
        if (c == '\n') {
            break;
        }
        get_next_char();
    }
    return view->file_pos;
}
/** @brief Get Position of Previous Line
    @ingroup view_navigation
    @param view data structure
    @param pos buffer offset
    @returns file position of previous line
  */
off_t get_pos_prev_line(View *view, off_t pos) {
    view->file_pos = pos;
    if (view->file_pos == 0) {
        view->f_bod = true;
        return view->file_pos;
    }
    while (view->ln_tbl[view->ln] >= view->file_pos) {
        if (view->ln == 0)
            break;
        view->ln--;
    }
    view->file_pos = view->ln_tbl[view->ln];
    return view->file_pos;
}
/** @brief Go to Specific File Position
    @ingroup view_navigation
    @param view data Structure
    @param go_to_pos
*/
void go_to_position(View *view, off_t go_to_pos) {
    view->file_pos = go_to_pos;
    view->page_top_pos = view->file_pos;
    view->page_bot_pos = view->file_pos;
    sync_ln(view);
    next_page(view);
}
/** @brief Go to End of File
    @ingroup view_navigation
    @param view data structure
 */
void go_to_eof(View *view) {
    view->file_pos = view->file_size;
    sync_ln(view);
    if (view->ln > view->scroll_lines)
        view->ln -= view->scroll_lines;
    else
        view->page_top_ln = 0;
    view->page_top_ln = view->ln;
    view->page_top_pos = view->ln_tbl[view->ln];
    view->page_bot_pos = view->page_top_pos;
    view->file_pos = view->page_top_pos;
    view->cury = 0;
    next_page(view);
}
/** @brief Go to Percent of File
    @ingroup view_navigation
    @param view data structure
    @param percent of file
*/
void go_to_percent(View *view, int percent) {
    if (view->file_size < 0) {
        Perror("Cannot determine file length");
        return;
    }
    view->file_pos = (percent * view->file_size) / 100;
    sync_ln(view);
    if (view->ln > view->scroll_lines)
        view->page_top_ln = view->ln - view->scroll_lines;
    else
        view->page_top_ln = 0;
    view->page_top_pos = view->ln_tbl[view->page_top_ln];
    view->page_bot_pos = view->page_top_pos;
    view->file_pos = view->page_top_pos;
    next_page(view);
}
/** @brief Go to Specific Line
    @ingroup view_navigation
    @param view data structure
    @param line_idx line number to go to (1-based index)
    @returns 0 on success, EOF if line index is out of bounds or end of data
   is reached
 */
int go_to_line(View *view, off_t line_idx) {
    if (line_idx <= 1) {
        go_to_position(view, 0);
        return EOF;
    }
    sync_ln(view);
    view->page_top_pos = view->file_pos;
    view->page_bot_pos = view->file_pos;
    view->file_pos = view->page_top_pos;
    next_page(view);
    return 0;
}
/** @brief Initialize Line Table
    @ingroup view_navigation
    @param view data structure
    @details The line table is initialized with a specified increment size
   (LINE_TBL_INCR). Memory is allocated for the line table, and the first
   entry is set to 0, indicating the file position of the first line. The
   line index (view->ln) is initialized to 0.
 */
void initialize_line_table(View *view) {
    view->ln_tbl_size = LINE_TBL_INCR;
    view->ln_tbl = (off_t *)malloc(view->ln_tbl_size * sizeof(off_t));
    if (view->ln_tbl == nullptr) {
        Perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    view->ln_max_pos = 0;
    view->ln_tbl[0] = 0;
    view->ln = 0;
}
/** @brief Increment Line Index and Update Line Table
    @ingroup view_navigation
    @param view data structure
    @details This function is called when a line feed character is
   encountered while reading the file. It increments the line index
   (view->ln) and checks if the current file position exceeds the maximum
   position recorded in the line table. If it does, it updates the line
   table with the new file position. If the line index exceeds the current
   size of the line table, the table is resized by allocating more memory.
 */
void increment_ln(View *view) {
    view->ln++;
    if (view->file_pos <= view->ln_max_pos)
        return;
    if (view->ln > view->ln_tbl_size - 1) {
        view->ln_tbl_size += LINE_TBL_INCR;
        view->ln_tbl =
            (off_t *)realloc(view->ln_tbl, view->ln_tbl_size * sizeof(off_t));
        if (view->ln_tbl == nullptr) {
            Perror("Memory allocation failed");
            exit(EXIT_FAILURE);
        }
    }
    view->ln_tbl_cnt = view->ln;
    view->ln_max_pos = view->file_pos;
    view->ln_tbl[view->ln] = view->file_pos;
}
/** @brief Synchronize Line Table with Current File Position
    @ingroup view_navigation
    @param view data Structure
    @details The line table (view->ln_tbl) is an array that stores the file
   position of each line. The index (view->ln + 1) corresponds to the
   current line number. (the line number table is 0-based, while line
   numbering starts at 1).
    @note If the line or position requested is not in the line table, this
   function reads forward to sycn.
    @note If the line or positione requested is behind the current line
   table index, the line index will be decremented it matches the file
   position.
 */
void sync_ln(View *view) {
    int c = 0;
    off_t idx;
    off_t target_pos;
    if (view->ln_tbl[view->ln] == view->file_pos)
        return;
    target_pos = view->file_pos;
    view->file_pos = view->ln_tbl[view->ln_tbl_cnt];
    if (view->file_pos < target_pos) {
        view->ln = view->ln_tbl_cnt;
        while (view->ln_max_pos < target_pos) {
            get_next_char();
            if (view->f_eod)
                return;
        }
    } else if (view->ln_tbl[view->ln] > target_pos) {
        idx = view->ln - 1;
        while (view->ln_tbl[idx] > target_pos)
            idx--;
        view->ln = idx;
        view->file_pos = view->ln_tbl[view->ln];
    } else {
        view->ln = view->ln_tbl_cnt;
        view->file_pos = view->ln_tbl[view->ln];
    }
}
/*------------------------------------------------------------
        END NAVIGATION
        BEGIN DISPLAY
  ------------------------------------------------------------*/
/** @brief Display Line on Pad
    @ingroup view_display
    param View *view data structure
    @details This function displays a single line of text on the ncurses
   pad.
    @note If line numbering is enabled (view->f_ln), it is formatted and
   displayed at the beginning of the line with the specified attributes and
   color pair.
    @details Because get_next_char calls increment_ln upon encountering a
   line feed and increment_ln advances view->ln after updating the line
   table, the line number displayed is one greater than the index to the
   line table. That means the line counter begins with 1, while the table
   origin is 0.
  */
void display_line(View *view) {
    char ln_s[16];

    if (view->cury < 0)
        view->cury = 0;
    if (view->cury > view->scroll_lines - 1)
        view->cury = view->scroll_lines - 1;
    if (view->f_ln) {
        ssnprintf(ln_s, 8, "%7jd", view->ln);
        wmove(view->ln_win, view->cury, 0);
        wclrtoeol(view->ln_win);
        mvwaddstr(view->ln_win, view->cury, 0, ln_s);
    }
    wmove(view->pad, view->cury, 0);
    wclrtoeol(view->pad);
    wadd_wchstr(view->pad, view->cmplx_buf);
    view->cury++;
}
/** @brief Format Line for Display
    @ingroup view_display
    @param view pointer to View structure containing line input and output
   buffers
    @return length of formatted line in characters
    @details This function processes the input line from view->line_in_s,
   handling ANSI escape sequences for text attributes and colors, as well as
   multi-byte characters. It converts the input line into a formatted line
   suitable for display in the terminal, storing the result in
   view->cmplx_buf and view->stripped_line_out. The function returns the
   length of the formatted line in characters, which may be used for
   tracking the maximum column width of the displayed content.
 */
int fmt_line(View *view) {
    char ansi_tok[MAXLEN];
    int i = 0, j = 0;
    int len = 0;
    const char *s;
    attr_t attr = WA_NORMAL;
    int cpx = cp_win;
    cchar_t cc = {0};
    wchar_t wstr[2] = {L'\0', L'\0'};

    char *in_str = view->line_in_s;
    cchar_t *cmplx_buf = view->cmplx_buf;

    rtrim(view->line_out_s);
    mbstate_t mbstate;
    memset(&mbstate, 0, sizeof(mbstate));
    while (in_str[i] != '\0') {
        if (in_str[i] == '\033' && in_str[i + 1] == '[') {
            len = strcspn(&in_str[i], "mK ") + 1;
            memcpy(ansi_tok, &in_str[i], len + 1);
            ansi_tok[len] = '\0';
            if (ansi_tok[0] == '\0') {
                if (i + 2 < MAXLEN)
                    i += 2;
                continue;
            }
            if (len == 0 || in_str[i + len - 1] == ' ') {
                i += 2;
                continue;
            } else if (in_str[i + len - 1] == 'K') {
                i += len;
                continue;
            }
            parse_ansi_str(ansi_tok, &attr, &cpx);
            i += len;
        } else {
            if (in_str[i] == '\033') {
                i++;
                continue;
            }
            s = &in_str[i];
            if (*s == '\t') {
                do {
                    wstr[1] = L'\0';
                    setcchar(&cc, wstr, attr, cpx, nullptr);
                    view->stripped_line_out[j] = ' ';
                    cmplx_buf[j++] = cc;
                } while ((j < PAD_COLS - 2) && (j % view->tab_stop != 0));
                i++;
            } else {
                wstr[1] = L'\0';
                len = mbrtowc(wstr, s, MB_CUR_MAX, &mbstate);
                if (len <= 0) {
                    wstr[0] = L'?';
                    wstr[1] = L'\0';
                    len = 1;
                }
                if (setcchar(&cc, wstr, attr, cpx, nullptr) != ERR) {
                    if (len > 0 && (j + len) < PAD_COLS - 1) {
                        view->stripped_line_out[j] = *s;
                        cmplx_buf[j++] = cc;
                    }
                }
                i += len;
            }
        }
    }
    if (j > view->maxcol)
        view->maxcol = j;
    wstr[0] = '\0';
    wstr[1] = '\0';
    setcchar(&cc, wstr, WA_NORMAL, cpx, nullptr);
    cmplx_buf[j] = cc;
    view->stripped_line_out[j] = '\0';
    return j;
}
/** @brief Parse ANSI SGR Escape Sequence
    @ingroup view_display
    @param ansi_str is the ANSI escape sequence string to parse
    @param attr is a pointer to an attr_t variable where the parsed
   attributes will be stored
    @param cpx is a pointer to an int variable where the parsed color pair
   index will be stored
    @details This function parses an ANSI SGR (Select Graphic Rendition)
   escape sequence and updates the provided attr_t and color pair index
   based on the attributes specified in the ANSI string.

   @verbatim

    This function converts the following SGR specification types to the
   appropriate curses color pair index for use in the terminal display.

    RGB:

        foreground \033[38;2;r;g;bm
        background \033[48;2;r;g;bm

        Where r, g, b are the red, green, and blue color components
   (0-255)

    XTERM 256-color:

        foreground \033[38;5;xm
        background \033[48;5;xm

        Where x is the 256-color index (0-255)

        uses xterm256_idx_to_rgb() to convert the 256-color index to
   RGB

    8-color:

        foreground \033[3cm
        background \033[4cm

        Where c is the color code (0 for black, 1 for red, 2 for green, 3
   for yellow, 4 for blue, 5 for magenta, 6 for cyan, 7 for white).

    Attributes:

        \033[am

        Where a is the attribute code (1 for bold, 2 for dim, 3 for italic,
   4 for underline, 5 for blink, 7 for reverse, 8 for invis). The function
   also supports resetting attributes and colors to default using \033[0m.

    @sa xterm256_idx_to_rgb(), rgb_to_curses_clr(), extended_pair_content(),
   get_clr_pair()

    @endverbatim
*/
void parse_ansi_str(char *ansi_str, attr_t *attr, int *cpx) {
    char *tok;
    char t0, t1;
    char tstr[3];
    int len, x_idx;
    int fg, bg;
    int fg_clr, bg_clr;
    char *ansi_p = ansi_str + 2;
    extended_pair_content(*cpx, &fg_clr, &bg_clr);
    fg = fg_clr;
    bg = bg_clr;
    RGB rgb;
    tok = strtok((char *)ansi_p, ";m");
    bool a_toi_error = false;
    while (1) {
        if (tok == nullptr || *tok == '\0')
            break;
        len = strlen(tok);
        if (len == 2) {
            t0 = tok[0];
            t1 = tok[1];
            if (t0 == '3' || t0 == '4') {
                if (t1 == '8') {
                    tok = strtok(nullptr, ";m");
                    if (tok != nullptr) {
                        if (*tok == '5') {
                            tok = strtok(nullptr, ";m");
                            if (tok != nullptr) {
                                x_idx = a_toi(tok, &a_toi_error);
                                rgb = xterm256_idx_to_rgb(x_idx);
                            }
                        } else if (*tok == '2') {
                            tok = strtok(nullptr, ";m");
                            rgb.r = a_toi(tok, &a_toi_error);
                            tok = strtok(nullptr, ";m");
                            rgb.g = a_toi(tok, &a_toi_error);
                            tok = strtok(nullptr, ";m");
                            rgb.b = a_toi(tok, &a_toi_error);
                        }
                    }
                    if (t0 == '3')
                        fg_clr = rgb_to_curses_clr(&rgb);
                    else if (t0 == '4')
                        bg_clr = rgb_to_curses_clr(&rgb);
                } else if (t1 == '9') {
                    if (t0 == '3')
                        fg_clr = CLR_FG;
                    else if (t0 == '4')
                        bg_clr = CLR_BG;
                } else if (t1 >= '0' && t1 <= '7') {
                    if (t0 == '3') {
                        tstr[0] = t1;
                        tstr[1] = '\0';
                        x_idx = a_toi(tstr, &a_toi_error);
                        rgb = xterm256_idx_to_rgb(x_idx);
                        fg_clr = rgb_to_curses_clr(&rgb);
                    } else if (t0 == '4') {
                        tstr[0] = t1;
                        tstr[1] = '\0';
                        x_idx = a_toi(tstr, &a_toi_error);
                        rgb = xterm256_idx_to_rgb(x_idx);
                        bg_clr = rgb_to_curses_clr(&rgb);
                    }
                }
            } else if (t0 == '0') {
                *tok = t1;
                len = 1;
            }
        }
        if (len == 1) {
            if (*tok == '0') {
                *attr = WA_NORMAL;
                fg_clr = CLR_FG;
                bg_clr = CLR_BG;
            } else {
                switch (a_toi(tok, &a_toi_error)) {
                case 1:
                    *attr |= WA_BOLD;
                    break;
                case 2:
                    *attr |= WA_DIM;
                    break;
                case 3:
                    *attr |= WA_ITALIC;
                    break;
                case 4:
                    *attr |= WA_UNDERLINE;
                    break;
                case 5:
                    *attr |= WA_BLINK;
                    break;
                case 7:
                    *attr |= WA_REVERSE;
                    break;
                case 8:
                    *attr |= WA_INVIS;
                    break;
                default:
                    break;
                }
            }
        } else if (len == 0) {
            *attr = WA_NORMAL;
            fg_clr = CLR_FG;
            bg_clr = CLR_BG;
        }
        tok = strtok(nullptr, ";m");
    }
    if (!a_toi_error && (fg_clr != fg || bg_clr != bg)) {
        clr_pair_idx = get_clr_pair(fg_clr, bg_clr);
        *cpx = clr_pair_idx;
    }
    return;
}
/** @brief Display Command Line Prompt
    @ingroup view_display
    @param view is the current view data structure
    @param s is the prompt string */
int display_prompt(View *view, char *s) {
    char message_str[PAD_COLS + 1];
    int l;
    l = strnz__cpy(message_str, s, PAD_COLS);
    wmove(view->cmdln_win, view->cmd_line, 0);
    if (l != 0) {
        wclrtoeol(view->cmdln_win);
        wattron(view->cmdln_win, WA_REVERSE);
        waddstr(view->cmdln_win, " ");
        waddstr(view->cmdln_win, message_str);
        waddstr(view->cmdln_win, " ");
        wattroff(view->cmdln_win, WA_REVERSE);
        waddstr(view->cmdln_win, " ");
        view->curx = l + 2;
        wmove(view->cmdln_win, view->cmd_line, view->curx);
    }
    return (view->curx);
}
/** @brief Remove File
    @ingroup view_engine
    @param view is the current view data structure */
void remove_file(View *view) {
    char c;
    if (view->f_at_end_remove) {
        wmove(view->pad, view->cmd_line, 0);
        waddstr(view->pad, "Remove File (Y or N)->");
        wclrtoeol(view->pad);
        c = (char)xwgetch(view->cmdln_win, nullptr, -1);
        waddch(view->pad, (char)toupper(c));
        if (c == 'Y' || c == 'y')
            remove(view->cur_file_str);
    }
}
/** @brief Display View Help File
    @ingroup view_display
    @param init is the current initialization data structure.
    @note The current View context is set aside by assigning the view
   structure to "view_save" while the help file is displayed using a new,
   separate view structure.
    @note The help file is specified by the VIEW_HELP_FILE macro can be set
   to a default help file path or overridden by the user through an
   environment variable.
    @note  After the help file is closed, the original view is restored and
   the page is redisplayed.
    @note It may be necessary to reassign view after calling this function
   because the init->view pointer is temporarily set to nullptr during the
   help file display, and the original view is restored afterward.
    @note The default screen size for help can be set in the code below. If
   set to 0, popup_view will determine reasonable maximal size based on the
   terminal dimensions.
    @note The help file may contain Unicode characters and ANSI escape
   sequences for formatting, which will be properly handled and displayed by
   popup_view. */
void view_display_help(Init *init) {
    char tmp_str[MAXLEN];
    int eargc = 0;
    char *eargv[MAXARGS];
    if (view->f_help_spec && view->help_spec[0] != '\0')
        strnz__cpy(tmp_str, view->help_spec, MAXLEN - 1);
    else {
        strnz__cpy(tmp_str, init->mapp_help, MAXLEN - 1);
        strnz__cat(tmp_str, "/", MAXLEN - 1);
        strnz__cat(tmp_str, VIEW_HELP_FILE, MAXLEN - 1);
    }
    eargv[eargc++] = strdup("view");
    eargv[eargc++] = strdup("-N");
    eargv[eargc++] = strdup("f");
    eargv[eargc++] = strdup(tmp_str);
    eargv[eargc] = nullptr;
    init->lines = 48;
    init->cols = 72;
    init->begy = 0;
    init->begx = 0;
    strnz__cpy(init->title, "View Help", MAXLEN - 1);
    popup_view(init, eargc, eargv, init->lines, init->cols, init->begy,
               init->begx);
    destroy_argv(eargc, eargv);
    init->view->f_redisplay_page = true;
}

void view_restore_wins() {
    wnoutrefresh(view->ln_win);
    wrefresh(view->ln_win);
    wnoutrefresh(view->cmdln_win);
    wrefresh(view->cmdln_win);
}
/*------------------------------------------------------------
      END DISPLAY
  ------------------------------------------------------------*/
/** @brief use form to enter a file specification
    @ingroup view_engine
    @param init data structure
    @param file_spec - pointer to file specification
    the file_spec
    @returns true if successful
    @note the user must provide a character array large enough to
   hold file_spec without overflowing
 */
bool enter_file_spec(Init *init, char *file_spec) {
    char earg_str[MAXLEN];
    char *eargv[MAXARGS];
    int eargc;
    char tmp_dir[MAXLEN];
    char tmp_str[MAXLEN];
    char tmp_spec[MAXLEN];
    int rc = false;
    FILE *tmp_fp;
    view = init->view;
    strnz__cpy(tmp_dir, init->mapp_home, MAXLEN - 1);
    strnz__cat(tmp_dir, "/tmp", MAXLEN - 1);
    expand_tilde(tmp_dir, MAXLEN - 1);
    if (!mk_dir(tmp_dir)) {
        ssnprintf(em0, MAXLEN - 1, "%s, line: %d", __FILE__, __LINE__ - 2);
        strnz__cpy(em1, "Unable to ", MAXLEN - 1);
        strnz__cat(em1, "mkdir", MAXLEN - 1);
        strnz__cat(em1, tmp_dir, MAXLEN - 1);
        strerror_r(errno, em2, MAXLEN - 1);
        display_error(em0, em1, em2, nullptr);
        return false;
    }
    while (rc == false) {
        strnz__cpy(tmp_spec, tmp_dir, MAXLEN - 1);
        strnz__cat(tmp_spec, "/tmp_XXXXXX", MAXLEN - 1);
        view->in_fd = mkstemp(tmp_spec);
        if (view->in_fd == -1) {
            ssnprintf(em0, MAXLEN - 1, "%s, line: %d", __FILE__, __LINE__ - 2);
            strnz__cpy(em1, "unable to ", MAXLEN - 1);
            strnz__cat(em1, "mkstemp ", MAXLEN - 1);
            strnz__cat(em1, tmp_spec, MAXLEN - 1);
            strerror_r(errno, em2, MAXLEN - 1);
            display_error(em0, em1, nullptr, nullptr);
            return false;
        }
        /** call form to get file_name
            write the name to a temporary file */

        strnz__cpy(earg_str, "form -d file_name.f -o ", MAXLEN - 1);
        strnz__cat(earg_str, tmp_spec, MAXLEN - 1);
        eargc = str_to_args(eargv, earg_str, MAX_ARGS);
        rc = popup_form(init, eargc, eargv, view->begy + view->lines - 7, 4);
        destroy_argv(eargc, eargv);
        restore_wins();
        if (rc == P_CANCEL || rc == 'q' || rc == 'Q' || rc == KEY_F(9))
            return false;
        close(view->in_fd);
        tmp_fp = fopen(tmp_spec, "r");
        if (tmp_fp == nullptr) {
            ssnprintf(em0, MAXLEN - 1, "%s, line: %d", __FILE__, __LINE__ - 2);
            strnz__cpy(em1, "unable to ", MAXLEN - 1);
            strnz__cat(em1, "fopen ", MAXLEN - 1);
            strnz__cat(em1, tmp_spec, MAXLEN - 1);
            strerror_r(errno, em2, MAXLEN - 1);
            display_error(em0, em1, em2, nullptr);
            return false;
        }
        fgets(tmp_str, MAXLEN - 1, tmp_fp);
        strnz(tmp_str, MAXLEN - 1);
        fclose(tmp_fp);
        unlink(tmp_spec);
        if (!verify_spec_arg(file_spec, tmp_str, "~/menuapp/tmp", ".",
                             S_WCOK | S_QUIET)) {
            ssnprintf(em0, MAXLEN - 1, "Unable to open %s for writing",
                      tmp_str);
            strnz__cpy(em1, "Try again? y (yes) or n (no) ", MAXLEN - 1);
            rc = display_error(em0, em1, nullptr, nullptr);
            if (rc == 'y' || rc == 'Y')
                continue;

        } else
            return true;
    }
    return true;
}
