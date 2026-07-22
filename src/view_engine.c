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
#include <iso646.h>
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
   @details Line numbers are tracked when reading forward and stored in a line
   table for quick access when moving backwards.
   When reading forward, the End of Data (EOD) flag is set when the
   position is equal to the file size, and cleared when the position or reading
   direction changes.
   @note view_engine.c line numbering
   The macro, get_next_char() reads lines delimited by '\n', advancing the
   line number index at the end of each record. The line number index
   is one record ahead for most operations, including line numbering as lines
   are displayed. Nevertheless, when providing a line number index to
   get_line(), you must use the zero origin index, which is one less than
   the line number index
 */
#define get_next_char()                              \
    {                                                \
        c = 0;                                       \
        do {                                         \
            if (view->file_pos == view->file_size) { \
                view->f_eod = true;                  \
                break;                               \
            } else                                   \
                view->f_eod = false;                 \
            c = view->buf[view->file_pos++];         \
        } while (c == 0x0d);                         \
        if (c == '\n')                               \
            increment_ln(view);                      \
    }
/** @brief read the previous characater from the virtual file
    @ingroup view_engine
    There is no need to track line numbers when moving backwards as they
   are stored in the line table and accessed as needed.
    When reading in reverse, the Beginning of Data (BOD) flag is set when
   the file position is zero, and cleared when the position or reading direction
   changes.
    Carriage-returns are ignored as they should be.
 */
#define get_prev_char()                      \
    {                                        \
        c = 0;                               \
        do {                                 \
            if (view->file_pos == 0) {       \
                view->f_bod = true;          \
                break;                       \
            } else                           \
                view->f_bod = false;         \
            c = view->buf[--view->file_pos]; \
        } while (c == 0x0d);                 \
    }

#define confirm()                                                \
    {                                                            \
        if ((c = get_cmd_char(view, &n_cmd)) == 'y' || c == 'Y') \
            ans = true;                                          \
        else if (c == 'n' || c == 'N')                           \
            ans = false;                                         \
    }

#define _Perror(msg)                                                        \
    {                                                                       \
        ssnprintf(em0, MAXLEN - 1, "%s, line: %d", __FILE__, __LINE__ - 2); \
        strnz__cpy(em1, msg, MAXLEN - 1);                                   \
        display_error(em0, em1, nullptr, nullptr);                          \
    }

#define _Refresh(view)     \
    {                      \
        pad_refresh(view); \
        update_panels();   \
        doupdate();        \
    }

char prev_regex_pattern[MAXLEN];
FILE *dbgfp;
int view_file(Init *);
int view_cmd_processor(Init *);
int get_cmd_char(View *, off_t *);
int get_cmd_arg(View *, char *);
void build_prompt(View *);
void cat_file(View *);
void lp(View *, char *);
void go_to_mark(View *, int);
void go_to_eof(View *);
int go_to_line(View *, off_t);
void go_to_percent(View *, int);
void go_to_position(View *, off_t);
bool search(View *, int, char *);
void next_page(View *);
void prev_page(View *);
void scroll_down(View *, int);
void scroll_up(View *, int);
void get_line(View *, off_t);
int fmt_line(View *);
void log_split_lines(View *);
void log_cc_buf(View *);
void log_stripped_line_out(View *);
void log_strnz(char *, int);
void display_line(View *);
void display_line_eod(View *);
void display_split_line(View *);
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
void destroy_line_table(View *);
int pad_refresh(View *);
void sync_ln(View *);
off_t line_number(View *, off_t);
char err_msg[MAXLEN];

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
    View *view = init->view;
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
        if (view_init_input(init, view->cur_file_str) == 0) {
            if (view->buf) {
                view->f_eod = 0;
                view->f_bod = 0;
                view->maxcol = 0;
                view->page_top_pos = 0;
                view->page_top_ln_no = 0;
                view->page_bot_ln_no = 0;
                view->ln_max_pos = 0;
                view->page_bot_pos = 0;
                view->file_pos = 0;
                strnz__cpy(view->title, view->cur_file_str, MAXLEN - 1);
                border_title(view->box_win, view->title);
                initialize_line_table(view);
                next_page(view);
                view_cmd_processor(init);
                destroy_line_table(view);
                munmap(view->buf, view->file_size);
            }
        } else {
            view->curr_argc++;
            if (view->curr_argc < view->argc) {
                view->next_file_spec_ptr = view->argv[view->curr_argc];
            }
        }
    }
    destroy_view_win(init);
    destroy_view(init);
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
    bool ans;
    ssize_t bytes_written;
    char *e;
    char shell_cmd_spec[MAXLEN];
    off_t n_cmd = 0L;
    off_t prev_file_pos;
    int max_pmincol;
    View *view = init->view;
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
            c = get_cmd_char(view, &n_cmd);
            if (c >= '0' && c <= '9') {
                tmp_str[0] = (char)c;
                tmp_str[1] = '\0';
                c = get_cmd_arg(view, tmp_str);
            }
        }
        switch (c) {

        case 'd':
            break;
        case Ctrl('R'): /**<  Ctrl('R') or KEY_RESIZE - Handle terminal resize */
        case 'x':
        case KEY_RESIZE:
            getmaxyx(stdscr, view->lines, view->cols);
#ifdef DEBUG_RESIZE
            ssnprintf(em0, MAXLEN - 1,
                      "%s:%d view->page_top_ln_no=%d, resized to lines: %d, cols: %d\n",
                      __FILE__, __LINE__, view->page_top_ln_no, view->lines, view->cols);
            write_cmenu_log(em0);
#endif
            if (view->f_full_screen)
                view_full_screen_resize(init);
            else
                view_boxwin_resize(init);
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
        case KEY_LEFT:
        case KEY_BACKSPACE:
            if (n_cmd > 0)
                view->h_shift = n_cmd;
            else if (n_cmd == 0) {
                if (view->h_shift > 0)
                    n_cmd = view->h_shift;
                else
                    n_cmd = 1;
            }
            shift = (int)n_cmd;
            if (view->pmincol - shift > 0)
                view->pmincol -= shift;
            else
                view->pmincol = 0;
            break;
        case 'l': /**< 'l', KEY_RIGHT - Horizontal scroll right by two thirds of the page width */
        case KEY_RIGHT:
            if (n_cmd > 0)
                view->h_shift = n_cmd;
            else if (n_cmd == 0) {
                if (view->h_shift > 0)
                    n_cmd = view->h_shift;
                else
                    n_cmd = 1;
            }
            shift = (int)n_cmd;
            max_pmincol = (view->maxcol > view->cols) ? (view->maxcol - view->cols) : 0;
            if (view->pmincol + shift < max_pmincol)
                view->pmincol += shift;
            else
                view->pmincol = max_pmincol;
            break;
        case 'k': /** 'k', KEY_UP - Scroll up one line */
        case KEY_UP:
            if (n_cmd <= 0)
                n_cmd = 1;
            scroll_up(view, n_cmd);
            break;
        /** 'j', KEY_DOWN, KEY_ENTER - scroll down one line */
        case 'j':
        case '\n':
        case KEY_DOWN:
        case KEY_ENTER:
            if (n_cmd <= 0)
                n_cmd = 1;
            scroll_down(view, n_cmd);
            break;
        /** Ctrl('B'), KEY_PPAGE - Previous Page */
        case KEY_PPAGE:
        case Ctrl('B'):
            prev_page(view);
            break;
        /**  Ctrl('F'), KEY_NPAGE Next Page */
        case KEY_NPAGE:
        case Ctrl('F'):
            view->ln_no++;
            next_page(view);
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
            display_prompt(view, "(i, n, s, t, w, or h)->");
            c = get_cmd_char(view, &n_cmd);
            c = S_TOLOWER(c);
            switch (c) {
            /**   -i   ignore_case in search */
            /**   -n   line numbers */
            /**   -s   squeeze multiple blank lines */
            /**   -t n set tab stop columns */
            /**   -w   wrap long lines */
            /**   -h   display help */
            case 'i':
                display_prompt(view, "Ignore Case in search (Y or N)->");
                confirm();
                if (ans)
                    view->f_ignore_case = true;
                else if (c == 'n' || c == 'N')
                    view->f_ignore_case = false;
                break;
            /**   -n   line numbers */
            case 'n':
                display_prompt(view, "Line Numbering (Y or N)->");
                confirm();
                if (view->f_ln == ans)
                    break;
                view->f_ln = ans;
                view->f_redisplay_page = true;
                (view->f_full_screen) ? view_full_screen_resize(init) : view_boxwin_resize(init);
                break;
            /**  -s  Squeeze Multiple Blank Lines */
            case 's':
                display_prompt(
                    view, "view->f_squeeze Multiple Blank lines (Y or N)->");
                confirm();
                view->f_squeeze = ans;
                break;
            case 'w':
                display_prompt(
                    view, "Line Wrapping (Y or N)->");
                if ((c = get_cmd_char(view, &n_cmd)) == 'y' || c == 'Y')
                    ans = true;
                else if (c == 'n' || c == 'N')
                    ans = false;
                if (view->wrap == ans)
                    break;
                view->wrap = ans;
                view->f_redisplay_page = true;
                (view->f_full_screen) ? view_full_screen_resize(init) : view_boxwin_resize(init);
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
            case KEY_F(1):
            case 'h':
                if (!view->f_displaying_help) {
                    view_display_help(init);
                    view = init->view;
                }
                view->next_cmd_char = '-';
                break;
            default:
                break;
            }
            break;
        /**  'n' - Repeat Previous Search */
        case 'n':
            if (view->f_search_complete) {
                Perror("Search complete, no more matches");
                break;
            }
            if (prev_search_cmd == 0) {
                Perror("No previouis search");
                break;
            }
            if (prev_search_cmd == '/') {
                view->cury = 0;
                view->srch_curr_pos = view->page_bot_pos;
            } else {
                view->cury = view->scroll_lines + 1;
                view->srch_curr_pos = view->page_top_pos;
            }
            rc = search(view, prev_search_cmd, prev_regex_pattern);
            if (rc == false) {
                Perror("No matches found");
                break;
            }
            break;
        /**  '/' or '?' - Search Forward fromk top of page */
        case '/':
            view->f_search_complete = false;
            strnz__cpy(tmp_str, " Forward", MAXLEN - 1);
            search_cmd = c;
            c = get_cmd_arg(view, tmp_str);
            if (c == KEY_F(9) || c == '\033')
                break;
            if (c == KEY_ENTER) {
                view->cury = 0;
                view->f_first_iter = true;
                view->srch_beg_pos = view->page_top_pos;
                view->srch_curr_pos = view->page_top_pos;
                rc = search(view, search_cmd, view->cmd_arg);
                if (rc == false) {
                    Perror("No matches found");
                    break;
                }
                prev_search_cmd = search_cmd;
                strnz__cpy(prev_regex_pattern, view->cmd_arg, MAXLEN - 1);
            }
            break;
        /**  '?' - Search Backward */
        case '?':
            view->f_search_complete = false;
            strnz__cpy(tmp_str, " Backward", MAXLEN - 1);
            search_cmd = c;
            c = get_cmd_arg(view, tmp_str);
            if (c == KEY_F(9) || c == '\033')
                break;
            if (c == '\n') {
                view->cury = view->scroll_lines;
                view->f_first_iter = true;
                view->srch_beg_pos = view->page_bot_pos;
                view->srch_curr_pos = view->page_bot_pos;
                rc = search(view, search_cmd, view->cmd_arg);
                if (rc == false) {
                    Perror("No matches found");
                    break;
                }
                prev_search_cmd = search_cmd;
                strnz__cpy(prev_regex_pattern, view->cmd_arg, MAXLEN - 1);
            }
            break;
        /**  'o' - Open a File */
        case 'o':
            if (get_cmd_arg(view, "File name:") == 0) {
                strtok(view->cmd_arg, " ");
                view->next_file_spec_ptr = strdup(view->cmd_arg);
                view->f_redisplay_page = true;
                return 0;
            }
            break;
        case 'G': /**  'G' - Go to the Beginning of the Document or line */
        case KEY_HOME:
            if (n_cmd > 0) {
                n_cmd -= 1;
                go_to_line(view, n_cmd);
            } else
                go_to_eof(view);
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
            lp(view, view->cur_file_str);
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
            lp(view, view->cur_file_str);
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
            mvwadd_wchnstr(view->cmdln_win, view->cmd_line, view->curx, &sp, 1);
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
/** @brief Get Command Character from User Input
    @ingroup view_engine
    @param view Pointer to the View structure containing the state and
   parameters of the view application. This structure is used to access and
   modify the state of the application as needed.
    @param n Pointer to an off_t variable where the numeric argument entered by
   the user will be stored. If the user enters a numeric argument, it will be
   converted to an off_t value and stored in this variable for use by the
   calling function.
    @return Returns the command character entered by the user, or a special
   value if a mouse event is detected. The function handles user input,
   including editing keys, and updates the command argument buffer accordingly.
   If the user enters a numeric argument, it is validated based on the context
   of the command being executed.
*/
int get_cmd_char(View *view, off_t *n) {
    int c = 0, i = 0;
    char *d, *s;
    int prevx;
    bool once = false;
    char cmd_str[33];
    cmd_str[0] = '\0';
    pad_refresh(view);
    // update_panels();
    curs_set(1);
    getyx(view->cmdln_win, view->cmd_line, view->curx);
    wbkgrndset(view->cmdln_win, &CC_IND);
    wmove(view->cmdln_win, view->cmd_line, view->curx);
    mvwadd_wchnstr(view->cmdln_win, view->cmd_line, view->curx, &ran, 1);
    wmove(view->cmdln_win, view->cmd_line, view->curx + 1);
    // update_panels();
    // doupdate();
    while (1) {
        if (i == 1 && !once) {
            once = true;
            view->curx = 0;
            wmove(view->cmdln_win, view->cmd_line, view->curx);
            wclrtoeol(view->cmdln_win);
            mvwadd_wchnstr(view->cmdln_win, view->cmd_line, view->curx, &ran, 1);
            mvwaddch(view->cmdln_win, view->cmd_line, view->curx + 1, (chtype)c);
        }
        getyx(view->cmdln_win, view->cmd_line, view->curx);
        update_panels();
        wmove(view->cmdln_win, view->cmd_line, view->curx);
        doupdate();
        c = vgetch(view->cmdln_win, 0);
        switch (c) {
        case KEY_MOUSE:
            // if (getmouse(event) == OK) {
            //     if (event.bstate & BUTTON1_CLICKED) {
            //         c = KEY_MOUSE;
            //         break;
            //     }
            // }
            break;
        case KEY_RESIZE:
        case '\n':
        case '\r':
            break;
        case 'q':
        case KEY_F(9):
            build_prompt(view);
            display_prompt(view, view->prompt_str);
            c = KEY_F(9);
            return c;
        case '\b':
        case KEY_BACKSPACE:
            if (i > 0) {
                s = &cmd_str[i];
                d = &cmd_str[--i];
                view->curx--;
                prevx = view->curx;
                while (*s != '\0') {
                    mvwaddch(view->cmdln_win, view->cmd_line, view->curx++, *s);
                    *d++ = *s++;
                }
                *d = '\0';
                wmove(view->cmdln_win, view->cmd_line, view->curx);
                wclrtoeol(view->cmdln_win);
                view->curx = prevx;
                wmove(view->cmdln_win, view->cmd_line, view->curx);
            }
            continue;
        case KEY_DC:
            if (i < 32 && cmd_str[i] != '\0') {
                s = &cmd_str[i + 1];
                d = &cmd_str[i];
                while (*s != '\0') {
                    mvwaddch(view->cmdln_win, view->cmd_line, view->curx++, *s);
                    *d++ = *s++;
                }
                *d = '\0';
                wmove(view->cmdln_win, view->cmd_line, view->curx);
                wclrtoeol(view->cmdln_win);
                wmove(view->cmdln_win, view->cmd_line, view->curx);
            }
            continue;
        case KEY_SRIGHT:
            if (i < 32 && cmd_str[i] != '\0') {
                i++;
                getyx(view->cmdln_win, view->cury, view->curx);
                if (view->curx < view->cols - 1) {
                    view->curx++;
                    wmove(view->cmdln_win, view->cmd_line, view->curx);
                }
            }
            continue;
        case KEY_SLEFT:
            if (i > 0) {
                i--;
                getyx(view->cmdln_win, view->cury, view->curx);
                if (view->curx > 0) {
                    view->curx--;
                    wmove(view->cmdln_win, view->cmd_line, view->curx);
                }
            }
            continue;
        default:
            if (c < '0' || c > '9' || i == 32)
                break;
            cmd_str[i++] = (char)c;
            cmd_str[i] = '\0';
            mvwaddch(view->cmdln_win, view->cmd_line, view->curx, (chtype)c);
            view->curx++;
            continue;
        }
        if (c < '0' || c > '9' || i == 32)
            break;
    }
    if (cmd_str[0] == '\0') {
        *n = 0;
        view->cmd_arg[0] = '\0';
        mvwadd_wchnstr(view->cmdln_win, view->cmd_line, view->curx + 1, &sp, 1);
        wclrtoeol(view->cmdln_win);
        return (c);
    }
    *n = atol(cmd_str);
    view->cmd_arg[0] = '\0';
    getyx(view->cmdln_win, view->cmd_line, view->curx);
    mvwadd_wchnstr(view->cmdln_win, view->cmd_line, view->curx + 1, &sp, 1);
    wclrtoeol(view->cmdln_win);
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
    char prompt_s[MAXLEN];
    int prompt_l = strnz__cpy(prompt_s, prompt, min(MAXLEN - 1, view->cols - 4));
    if (view->cmd_arg[0] != '\0')
        return 0;
    wmove(view->cmdln_win, view->cmd_line, 0);
    if (prompt_l > 1) {
        wbkgrndset(view->cmdln_win, &CC_NT_REV);
        waddstr(view->cmdln_win, prompt_s);
        wbkgrndset(view->cmdln_win, &CC_NT);
    }
    view->curx = prompt_l;
    wclrtoeol(view->cmdln_win);
    pad_refresh(view);
    update_panels();
    curs_set(1);
    mvwadd_wchnstr(view->cmdln_win, view->cmd_line, view->curx, &ran, 1);
    wmove(view->cmdln_win, view->cmd_line, view->curx + 1);
    doupdate();
    int flin = view->cmd_line;
    int fcol = prompt_l + 1;
    int flen = view->cols - prompt_l;
    c = cf_accept(view->cmdln_win, view->cmd_arg, flin, fcol, flen);
    return c;
}
/** @brief Build Prompt String
 *   @ingroup view_engine
    @param view Pointer to the View structure containing the state and
   parameters of the view application. This structure is used to access and
   modify the state of the application as needed.
    @details
    This function constructs a prompt string that provides information about
   the current state of the view application. The prompt includes details such
   as the file name, current column, file number, file position, and whether
   the end of the document has been reached. The constructed prompt string is
   stored in the view->prompt_str buffer for display to the user.
    @note The prompt string is built based on the current state of the view
   application, and segments that are not relevant will be omitted. Less
   relevant segments will be omitted if the length of the prompt string
   exceeds more than half the available space. The length of the prompt
   string has a hard limit of view->cols - 4.
 */
void build_prompt(View *view) {
    char tmp_str[MAXLEN];
    int prompt_maxlen = min(MAXLEN - 1, view->cols - 4);
    int prompt_l = 0;
    // ----------------< File Name >----------------
    if (view->f_is_pipe)
        strnz__cpy(view->prompt_str, "stdin", prompt_maxlen);
    else
        strnz__cpy(view->prompt_str, view->file_name, prompt_maxlen);
    // ----------------< Columns >----------------
    if (view->pmincol > 0) {
        sprintf(tmp_str, "Col %d of %d", view->pmincol, view->maxcol);
        if (view->prompt_str[0] != '\0')
            strnz__cat(view->prompt_str, "|", prompt_maxlen);
        strnz__cat(view->prompt_str, tmp_str, prompt_maxlen);
    }
    // ----------------< File Number >----------------
    if (view->argc > 1) {
        prompt_l = (int)strlen(view->prompt_str);
        if (prompt_l > (view->cols - 4) / 2)
            return;
        if (view->argc > 0) {
            sprintf(tmp_str, "File %d of %d", view->curr_argc + 1, view->argc);
            if (view->prompt_str[0] != '\0') {
                strnz__cat(view->prompt_str, "|", prompt_maxlen);
                strnz__cat(view->prompt_str, tmp_str, prompt_maxlen);
            }
        }
    }
    // ----------------< File Position >----------------
    prompt_l = (int)strlen(view->prompt_str);
    if (prompt_l > (view->cols - 4) / 2)
        return;
    view->page_top_pos = view->ln_tbl[view->page_top_ln_no];
    if (view->page_top_pos == NULL_POSITION)
        view->page_top_pos = view->file_size;
    view->page_bot_pos = view->ln_tbl[view->page_bot_ln_no + 1];
    if (view->page_bot_pos == NULL_POSITION)
        view->page_bot_pos = view->file_size;
    sprintf(tmp_str, "Pos %zd-%zd", view->page_top_pos, view->page_bot_pos);
    if (view->prompt_str[0] != '\0') {
        strnz__cat(view->prompt_str, "|", prompt_maxlen);
        strnz__cat(view->prompt_str, tmp_str, prompt_maxlen);
    }
    if (!view->f_is_pipe) {
        if (view->file_size > 0) {
            sprintf(tmp_str, " of %zd", view->file_size);
            strnz__cat(view->prompt_str, tmp_str, prompt_maxlen);
        }
    }
    // ----------------< (End) >----------------
    prompt_l = (int)strlen(view->prompt_str);
    if (prompt_l > (view->cols - 4) / 2)
        return;
    if (view->f_eod) {
        if (view->prompt_str[0] != '\0')
            strnz__cat(view->prompt_str, " ", prompt_maxlen);
        strnz__cat(view->prompt_str, "(End)", prompt_maxlen);
        if (view->curr_argc + 1 < view->argc) {
            base_name(tmp_str, view->argv[view->curr_argc + 1]);
            strnz__cpy(view->prompt_str, " Next File: ", prompt_maxlen);
            strnz__cat(view->prompt_str, tmp_str, prompt_maxlen);
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
        off_t ln_no = 0;
        get_line(view, ln_no);
        if (f_strip_ansi)
            strip_ansi(tmp_line_s, view->line_in_s);
        else
            strnz__cpy(tmp_line_s, view->line_in_s, MAXLEN - 1);
        l = strnlf(tmp_line_s, PAD_COLS - 1);
        bytes_written += write(view->out_fd, tmp_line_s, l);
        if (ln_no >= view->ln_tbl_size - 1)
            break;
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
    @param view Pointer to the View structure containing the state and
    @param PrintFile - file to print */
void lp(View *view, char *PrintFile) {
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
    @details The search performs extended regular expression matching, ignoring
   ANSI sequences and Unicode characters. Matches are highlighted on the
   screen, and the search continues until the page is full or the end of the
   file is reached. If the search wraps around the file, a message is
   displayed indicating that the search is complete.
    The search state is maintained in the view structure, allowing for
   repeat searches and tracking of the current search line. This
   function highlights all matches in the current ncurses pad, including
   those not displayed on the screen, and tracks the first and last match
   columns for prompt display.
    ANSI sequences and Unicode characters are stripped before
   matching, so matching corresponds to the visual display */
bool search(View *view, int search_cmd, char *regex_pattern) {
    char tmp_str[MAXLEN];
    int REG_FLAGS = 0;
    regmatch_t pmatch[1];
    regex_t compiled_regex;
    int reti;
    int line_offset;
    int line_len;
    int match_idx;
    int match_len;
    mbstate_t mbstate;
    memset(&mbstate, 0, sizeof(mbstate));
    wchar_t wstr[2] = {L'\0', L'\0'};
    attr_t attr;
    short cpx;
    cchar_t cc = {0};
    off_t prev_ln_no;
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
    bool rc = false;
    while (1) {
        /** initialize iteration */
        if (search_cmd == '/') {
            if (view->srch_curr_ln_no == view->file_size)
                view->srch_curr_ln_no = 0;
        } else {
            if (view->srch_curr_ln_no == 0)
                view->srch_curr_ln_no = view->file_size;
        }
        if (view->srch_curr_ln_no == view->srch_beg_ln_no) {
            if (view->f_first_iter == true) {
                view->f_first_iter = false;
                view->f_search_complete = false;
                if (search_cmd == '/')
                    view->cury = 0;
                else
                    view->cury = view->scroll_lines + 1;
            } else {
                view->f_search_complete = true;
                goto cleanup; /* FIX: Was return rc; */
            }
        }
        view->ln_no = view->srch_curr_ln_no;
        sync_ln(view);
        /** get line to scan */
        if (search_cmd == '/') {
            if (view->cury == view->scroll_lines)
                goto cleanup; /* FIX: Was return rc; */
            prev_ln_no = view->srch_curr_ln_no;
            get_line(view, view->srch_curr_ln_no);
            view->page_bot_ln_no = view->srch_curr_ln_no;
        } else {
            if (view->cury == 0)
                goto cleanup; /* FIX: Was return rc; */
            get_line(view, view->srch_curr_ln_no);
            prev_ln_no = view->srch_curr_ln_no;
            view->page_top_ln_no = view->srch_curr_ln_no;
        }
        fmt_line(view);
        reti = regexec(&compiled_regex, view->stripped_line_out,
                       compiled_regex.re_nsub + 1, pmatch, REG_FLAGS);
        if (reti == REG_NOMATCH) {
            if (f_page) {
                /** non-matching page filler */
                if (search_cmd == '?')
                    view->cury -= 2;
                display_line(view);
                if ((search_cmd == '/' && view->cury == view->scroll_lines) ||
                    (search_cmd == '?' && view->cury == 1)) {
                    break;
                }
            }
            view->srch_curr_ln_no += (search_cmd == '/') ? 1 : -1;
            continue;
        }
        if (reti) {
            char err_str[MAXLEN];
            regerror(reti, &compiled_regex, err_str, sizeof(err_str));
            strnz__cpy(tmp_str, "Regex match failed: ", MAXLEN - 1);
            strnz__cat(tmp_str, err_str, MAXLEN - 1);
            Perror(tmp_str);
            rc = false;   /* Set status */
            goto cleanup; /* FIX: Consolidated to cleanup */
        }
        rc = true;
        /** Display matching lines */
        if (!f_page) {
            if (search_cmd == '/') {
                view->page_top_ln_no = prev_ln_no;
                wmove(view->pad, view->cury, 0);
            } else {
                view->page_bot_ln_no = view->ln_no;
                wmove(view->pad, 0, 0);
            }
            wclrtobot(view->pad);
            f_page = true;
        }
        if (search_cmd == '?')
            view->cury -= 2;
        view->first_match_x = -1;
        view->last_match_x = 0;
        line_len = strlen(view->stripped_line_out);
        line_offset = 0;
        while (1) {
            match_idx = line_offset + pmatch[0].rm_so;
            match_len = pmatch[0].rm_eo - pmatch[0].rm_so;
            for (int i = match_idx; i < match_idx + match_len; i++) {
                cc = view->cmplx_buf[i];
                getcchar(&cc, wstr, &attr, &cpx, nullptr);
                cpx = cp_nt_rev;
                setcchar(&cc, wstr, attr, cpx, nullptr);
                view->cmplx_buf[i] = cc;
            }
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
                rc = false;   /* Set status */
                goto cleanup; /* FIX: Consolidated to cleanup */
            }
            if (search_cmd == '/') {
                if (view->cury == view->scroll_lines - 1) {
                    break;
                }
            } else if (view->cury == 1) {
                break;
            }
        }
        display_line(view);
        view->srch_curr_ln_no += (search_cmd == '/') ? 1 : -1;
    }
cleanup:
    regfree(&compiled_regex);
    view->ln_no = view->srch_curr_ln_no;
    return rc;
}

/*--------------------------------------------------------------
   Navigation
 *--------------------------------------------------------------- */
/** @brief display previous page
    @ingroup view_navigation
    @param view data structure
    @details Displays the previous page starting at (view->page_top_ln_no -
   view->scroll_lines).
 */
void prev_page(View *view) {
    off_t ln_no;
    if (view->page_top_ln_no == 0)
        return;
    view->cury = 0;
    view->ln_no = view->page_top_ln_no;
    int scroll, avail, scroll_this_line;
    if (view->wrap) {
        scroll = view->scroll_lines;
        ln_no = view->page_top_ln_no;
        while (scroll > 0) {
            get_line(view, ln_no);
            fmt_line(view);
            view->page_top_sl = (view->cur.sl_cnt > 1);
            if (view->page_top_sl == false) {
                scroll--;
                if (ln_no > 0)
                    ln_no--;
                continue;
            }
            if (ln_no == view->page_top_ln_no && ln_no != view->ln_no_max) {
                view->cur.sl_idx = view->page_top_sl_idx;
                if (view->cur.sl_idx == 0) {
                    if (ln_no > 0)
                        ln_no--;
                    continue;
                }
                avail = view->cur.sl_idx;
            } else
                avail = view->cur.sl_cnt;
            scroll_this_line = min(scroll, avail);
            scroll -= scroll_this_line;
            if (ln_no == view->page_top_ln_no && ln_no != view->ln_no_max)
                view->cur.sl_idx -= scroll_this_line;
            else
                view->cur.sl_idx = view->cur.sl_cnt - scroll_this_line;
            if (ln_no > 0)
                ln_no--;
        }
        ln_no++;
        view->page_top_sl_idx = view->cur.sl_idx;
        view->page_top_sl_cnt = view->cur.sl_cnt;
        view->page_top_ln_no = ln_no;
        view->ln_no = view->page_top_ln_no;
        view->page_top_sl = (view->page_top_sl_cnt > 1);
    } else {
        view->ln_no = view->page_top_ln_no;
        view->cury = 0;
        if (view->ln_no - view->scroll_lines >= 0)
            view->ln_no -= view->scroll_lines;
        else
            view->ln_no = 0;
        view->page_top_ln_no = view->ln_no;
    }
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

    view->file_pos = view->ln_tbl[view->ln_no];
    if (view->file_pos == view->file_size)
        return;
    view->maxcol = 0;
    view->cury = 0;
    view->page_top_ln_no = view->ln_no;
    view_display_page(view);
}
/** @brief Display Current Page
    @ingroup view_display
    @param view data structure
 */
void view_display_page(View *view) {
    view->cury = 0;
    view->page_top_sl = false;
    view->page_bot_sl = false;
    wmove(view->pad, 0, 0);
    if (view->lnno_win)
        wmove(view->lnno_win, 0, 0);
    view->ln_no = view->page_top_ln_no;
    view->page_top_ln_no = 0;
    view->page_bot_ln_no = 0;
    while (view->cury < view->scroll_lines) {
        get_line(view, view->ln_no);
        if (view->f_eod)
            break;
        fmt_line(view);
        if (view->wrap && view->cury == 0)
            view->cur.sl_idx = view->page_top_sl_idx;
        display_line(view);
        _Refresh(view);
        view->ln_no++;
    }
    view->ln_no--;
    if (view->f_eod)
        display_line_eod(view);
    view->page_bot_end_pos = view->file_pos - 1;
    view->page_bot_pos = view->file_pos;
    view->page_bot_ln_no = view->ln_no;
}
/** @brief Display Line on Pad
    @ingroup view_display
    param View *view data structure
    @details This function displays a single line of text on the ncurses
   pad.
    If line numbering is enabled (view->f_ln), it is formatted and
   displayed at the beginning of the line with the specified attributes and
   color pair.
    @details Because get_next_char calls increment_ln upon encountering a
   line feed and increment_ln advances view->ln_no after updating the line
   table, the line number displayed is one greater than the index to the
   line table. That means the line counter begins with 1, while the table
   origin is 0.
  */
void display_line(View *view) {
    char ln_s[16];

    if (view->wrap && view->cur.sl_cnt > 0) {
        if (view->cur.sl_idx < view->cur.sl_cnt) {
            display_split_line(view);
            return;
        }
    }
    if (view->cury < 0)
        view->cury = 0;
    if (view->cury > view->scroll_lines - 1)
        view->cury = view->scroll_lines - 1;
    if (view->f_ln) {
        ssnprintf(ln_s, 8, "%7jd", view->ln_no);
        wmove(view->lnno_win, view->cury, 0);
        wclrtoeol(view->lnno_win);
        mvwaddstr(view->lnno_win, view->cury, 0, ln_s);
    }
    wmove(view->pad, view->cury, 0);
    wclrtoeol(view->pad);
    wadd_wchstr(view->pad, view->cmplx_buf);
    if (view->cury == 0)
        view->page_top_ln_no = view->ln_no;
    if (view->cury == view->scroll_lines - 1)
        view->page_bot_ln_no = view->ln_no;
    view->cury++;
}
/** @brief Display Split Line on Pad
    @ingroup view_display
    @param view data structure
    @details This function is used to display a line that has been split into
   multiple segments due to wrapping. It iterates through the segments of the
   current line (stored in view->cur.sl_cc and view->cur.sl_cells) and
   displays each segment on the pad, along with the corresponding line number
   if line numbering is enabled. The function also updates the top and bottom
   line numbers of the page based on the current position in the split line.
 */
void display_split_line(View *view) {
    char ln_s[16];
    if (view->cury < 0)
        view->cury = 0;
    if (view->cury > view->scroll_lines - 1)
        view->cury = view->scroll_lines - 1;

    for (int i = view->cur.sl_idx; i < view->cur.sl_cnt; i++) {
        if (i == 0) {
            if (view->f_ln) {
                ssnprintf(ln_s, 8, "%7jd", view->ln_no);
                wmove(view->lnno_win, view->cury, 0);
                wclrtoeol(view->lnno_win);
                mvwaddstr(view->lnno_win, view->cury, 0, ln_s);
            }
        } else {
            if (view->f_ln) {
                wmove(view->lnno_win, view->cury, 0);
                wclrtoeol(view->lnno_win);
            }
        }
        wmove(view->pad, view->cury, 0);
        wclrtoeol(view->pad);
        wadd_wchnstr(view->pad, view->cur.sl_cc[i], view->cur.sl_cells[i]);
        if (view->cury == 0) {
            view->page_top_ln_no = view->ln_no;
            view->page_top_sl = true;
            view->page_top_sl_idx = i;
            view->page_top_sl_cnt = view->cur.sl_cnt;
        }
        if (view->cury == view->scroll_lines - 1) {
            view->page_bot_ln_no = view->ln_no;
            view->page_bot_sl = true;
            view->page_bot_sl_idx = i;
            view->page_bot_sl_cnt = view->cur.sl_cnt;
            view->page_bot_ln_no = view->ln_no;
        }
        view->cury++;
        if (view->cury == view->scroll_lines)
            break;
    }
}
/** @brief Display End of Data
    @ingroup view_display
    @param view data structure
    @details This function is called when the end of the file is reached and
   there are no more lines to display. It clears the remaining lines in the
   pad and line number window (if line numbering is enabled) to ensure that
   no residual content from previous lines is displayed.
 */
void display_line_eod(View *view) {
    if (view->f_ln) {
        wmove(view->lnno_win, view->cury, 0);
        wclrtobot(view->lnno_win);
    }
    wmove(view->pad, view->cury, 0);
    wclrtobot(view->pad);
    view->page_bot_ln_no = view->ln_no;
}
/** @brief Scroll Down by n Lines
    @ingroup view_navigation
    @param view Pointer to the View structure containing the state and
   parameters of the view application. This structure is used to access and
   modify the state of the application as needed.
    @param n The number of lines to scroll down. This parameter specifies how
   many lines the view should move down in the file, effectively advancing
   the display by n lines. The function will handle scrolling, updating the
   current line number, and refreshing the display accordingly.
 */
void scroll_down(View *view, int n) {
    int scroll, scroll_this_line, avail;
    off_t ln_no;
    view->f_bod = false;
    if (view->wrap) {
        // Set Top Line State
        scroll = n;
        ln_no = view->page_top_ln_no;
        while (scroll > 0) {
            get_line(view, ln_no);
            if (view->f_eod)
                break;
            fmt_line(view);
            view->page_top_sl = (view->cur.sl_cnt > 1);
            if (view->page_top_sl == false) {
                ln_no++;
                scroll--;
                continue;
            }
            if (ln_no == view->page_top_ln_no)
                view->cur.sl_idx = view->page_top_sl_idx;
            avail = view->cur.sl_cnt - 1 - view->cur.sl_idx;
            scroll_this_line = min(scroll, avail);
            scroll -= scroll_this_line;
            if (ln_no == view->page_top_ln_no)
                view->cur.sl_idx += scroll_this_line;
            else
                view->cur.sl_idx += scroll_this_line - 1;
            if (scroll != 0)
                ln_no++;
        }
        view->page_top_sl_idx = view->cur.sl_idx;
        view->page_top_sl_cnt = view->cur.sl_cnt;
        view->page_top_ln_no = ln_no;
        view->page_top_sl = (view->cur.sl_cnt > 1);
        // Set Bottom Line State
        scroll = n;
        ln_no = view->page_bot_ln_no;
        while (scroll > 0) {
            get_line(view, ln_no);
            if (view->f_eod)
                break;
            fmt_line(view);
            view->page_bot_sl = (view->cur.sl_cnt > 1);
            if (view->page_bot_sl == false) {
                ln_no++;
                scroll--;
                continue;
            }
            if (ln_no == view->page_bot_ln_no)
                view->cur.sl_idx = view->page_bot_sl_idx;
            avail = view->cur.sl_cnt - 1 - view->cur.sl_idx;
            if (avail == 0) {
                ln_no++;
                continue;
            }
            scroll_this_line = min(scroll, avail);
            scroll -= scroll_this_line;
            if (ln_no == view->page_bot_ln_no)
                view->cur.sl_idx += scroll_this_line;
            else
                view->cur.sl_idx += scroll_this_line - 1;
            view->page_bot_sl_idx = view->cur.sl_idx;
            view->page_bot_sl_cnt = view->cur.sl_cnt;
            view->page_bot_ln_no = ln_no;
            view->page_bot_sl = (view->cur.sl_cnt > 1);
            view->ln_no = view->page_bot_ln_no;
            if (view->f_ln)
                wscrl(view->lnno_win, 1);
            wscrl(view->pad, scroll_this_line);
            view->cury = view->scroll_lines - scroll_this_line;
            wmove(view->pad, view->cury, 0);
            display_line(view);
            if (scroll != 0)
                ln_no++;
        }
    } else {
        view->ln_no = view->page_bot_ln_no;
        if (view->ln_no >= view->ln_no_max)
            return;
        if (n > view->scroll_lines) {
            if (view->f_ln) {
                wmove(view->lnno_win, 0, 0);
                wclrtobot(view->lnno_win);
            }
            wmove(view->pad, 0, 0);
            wclrtobot(view->pad);
        } else {
            if (view->f_ln)
                wscrl(view->lnno_win, n);
            wscrl(view->pad, n);
            if (n < view->scroll_lines)
                view->cury = view->scroll_lines - n;
        }
        view->page_top_ln_no += n;
        scroll = n;
        while (scroll > 0) {
            if (view->ln_no >= view->ln_no_max)
                break;
            view->ln_no++;
            get_line(view, view->ln_no);
            if (view->f_eod)
                break;
            fmt_line(view);
            wmove(view->pad, view->cury, 0);
            display_line(view);
            if (view->cury == view->scroll_lines)
                break;
        }
        view->page_bot_ln_no = view->ln_no;
    }
    return;
}
/** @brief Scroll Up by n Lines
    @ingroup view_navigation
    @param view Pointer to the View structure containing the state and
   parameters of the view application. This structure is used to access and
   modify the state of the application as needed.
    @param n The number of lines to scroll up. This parameter specifies how
   many lines the view should move up in the file, effectively moving the
   display up by n lines. The function will handle scrolling, updating the
   current line number, and refreshing the display accordingly.
 */
void scroll_up(View *view, int n) {
    int scroll, avail, scroll_this_line;
    off_t ln_no;
    view->f_eod = false;
    if (view->page_top_ln_no == 0) {
        if (view->wrap) {
            if (view->page_top_sl == false || view->page_top_sl_idx == 0)
                return;
        } else
            return;
    }
    if (n > view->scroll_lines) {
        if (view->f_ln) {
            wmove(view->lnno_win, 0, 0);
            wclrtobot(view->lnno_win);
        }
        wmove(view->pad, 0, 0);
        wclrtobot(view->pad);
    } else {
        if (view->f_ln)
            wscrl(view->lnno_win, -n);
        wscrl(view->pad, -n);
    }
    wmove(view->pad, 0, 0);
    if (view->wrap) {
        scroll = n;
        ln_no = view->page_top_ln_no;
        while (scroll > 0) {
            get_line(view, ln_no);
            if (view->f_bod)
                break;
            fmt_line(view);
            view->page_top_sl = (view->cur.sl_cnt > 1);
            if (view->page_top_sl == false) {
                scroll--;
                if (ln_no > 0)
                    ln_no--;
                continue;
            }
            if (ln_no == view->page_top_ln_no) {
                view->cur.sl_idx = view->page_top_sl_idx;
                if (view->cur.sl_idx == 0) {
                    if (ln_no > 0)
                        ln_no--;
                    continue;
                }
                avail = view->cur.sl_idx;
            } else
                avail = view->cur.sl_cnt;
            scroll_this_line = min(scroll, avail);
            scroll -= scroll_this_line;
            if (ln_no == view->page_top_ln_no)
                view->cur.sl_idx -= scroll_this_line;
            else
                view->cur.sl_idx = view->cur.sl_cnt - scroll_this_line;
            if (scroll == 0)
                break;
        }
        view->page_top_sl_idx = view->cur.sl_idx;
        view->page_top_sl_cnt = view->cur.sl_cnt;
        view->page_top_ln_no = ln_no;
        view->page_top_sl = (view->page_top_sl_cnt > 1);

        view->cury = 0;
        scroll = n;
        while (scroll > 0) {
            if (view->ln_no != view->page_top_ln_no) {
                get_line(view, view->ln_no);
                if (view->f_eod)
                    break;
                fmt_line(view);
            }
            display_line(view);
            scroll--;
            view->ln_no++;
        }
        view->ln_no--;

        // Set Bottom Line State
        scroll = n;
        ln_no = view->page_bot_ln_no;
        while (scroll > 0) {
            get_line(view, ln_no);
            if (view->f_bod)
                break;
            fmt_line(view);
            view->page_bot_sl = (view->cur.sl_cnt > 1);
            if (view->page_bot_sl == false) {
                scroll--;
                if (ln_no > 0)
                    ln_no--;
                continue;
            }
            if (ln_no == view->page_bot_ln_no) {
                view->cur.sl_idx = view->page_bot_sl_idx;
                if (view->cur.sl_idx == 0) {
                    ln_no--;
                    continue;
                }
                avail = view->cur.sl_idx;
            } else
                avail = view->cur.sl_cnt;
            scroll_this_line = min(scroll, avail);
            scroll -= scroll_this_line;
            if (ln_no == view->page_bot_ln_no)
                view->cur.sl_idx -= scroll_this_line;
            else
                view->cur.sl_idx = view->cur.sl_cnt - scroll_this_line;
            if (ln_no > 0)
                ln_no--;
        }
        ln_no++;
        view->page_bot_sl_idx = view->cur.sl_idx;
        view->page_bot_sl_cnt = view->cur.sl_cnt;
        view->page_bot_ln_no = ln_no;
        view->page_bot_sl = (view->cur.sl_cnt > 1);
    } else {
        view->cury = 0;
        scroll = n;
        while (scroll > 0) {
            view->ln_no = view->page_top_ln_no - scroll;
            if (view->ln_no < 0)
                view->ln_no = 0;
            get_line(view, view->ln_no);
            if (view->f_eod)
                break;
            fmt_line(view);
            display_line(view);
            scroll--;
            view->ln_no++;
        }
        view->ln_no--;
    }
    return;
}
void get_line(View *view, off_t line) {
    char c;
    char *line_in_p;

    view->ln_no = line;
    view->file_pos = view->ln_tbl[view->ln_no];
    view->f_eod = false;
    get_next_char();
    if (view->f_eod) {
        view->ln_no = line;
        return;
    }
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
        if (view->f_eod) {
            view->ln_no = line;
            return;
        }
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
        if (view->f_eod) {
            view->ln_no = line;
            return;
        }
    }
    view->ln_no = line;
    return;
}
/** @brief Go to End of File
    @ingroup view_navigation
    @param view data structure
 */
void go_to_eof(View *view) {
    view->file_pos = view->file_size;
    sync_ln(view);
    view->ln_no--;
    view->ln_no_max = view->ln_no;
    if (view->wrap) {
        view->page_top_ln_no = view->ln_no;
        view->f_eod = true;
        prev_page(view);
        return;
    }
    if (view->ln_no > view->scroll_lines)
        view->ln_no -= view->scroll_lines - 1;
    // else
    // view->page_top_ln_no = 0;
    view->page_top_ln_no = view->ln_no;
    // view->page_top_pos = view->ln_tbl[view->ln_no];
    // view->page_bot_pos = view->page_top_pos;
    // view->file_pos = view->page_top_pos;
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
    view->ln_no = line_number(view, view->file_pos);
    view->file_pos = view->ln_tbl[view->ln_no];
    sync_ln(view);
    if (view->ln_no > view->scroll_lines)
        view->page_top_ln_no = view->ln_no - view->scroll_lines;
    else
        view->page_top_ln_no = 0;
    view->page_top_pos = view->ln_tbl[view->page_top_ln_no];
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
    if (line_idx < 0 || line_idx > view->ln_tbl_size - 1) {
        Perror("Line number out of bounds");
        return EOF;
    }
    view->ln_no = line_idx;
    view->file_pos = view->ln_tbl[view->ln_no];
    sync_ln(view);
    view->page_top_pos = view->file_pos;
    view->page_bot_pos = view->file_pos;
    // view->file_pos = view->page_top_pos;
    next_page(view);
    return 0;
}
/** @brief Go to Specific File Position
    @ingroup view_navigation
    @param view data Structure
    @param go_to_pos
*/
void go_to_position(View *view, off_t go_to_pos) {
    view->ln_no = line_number(view, go_to_pos);
    view->file_pos = view->ln_tbl[view->ln_no];
    sync_ln(view);
    next_page(view);
}
/** @brief Get Line Number for a Given File Position
    @ingroup view_navigation
    @param view data structure
    @param target file position
    @returns line number corresponding to the given file position
    @details This function performs a binary search on the line table (view->ln_tbl)
   to find the line number corresponding to the specified file position (target).
   It returns the index of the line in the line table that is closest to the target
   position without exceeding it. If the target position is not found, it returns
   the previous line number.
 */
off_t line_number(View *view, off_t position) {
    off_t low = 0;
    off_t high = view->ln_tbl_size - 1;
    off_t prev_ln_no = 0;
    while (low <= high) {
        off_t guess = low + (high - low) / 2;
        if (view->ln_tbl[guess] == position)
            return guess;
        if (view->ln_tbl[guess] < position) {
            prev_ln_no = guess;
            low = guess + 1;
        } else
            high = guess - 1;
    }
    return prev_ln_no;
}
/** @brief Initialize Line Table
    @ingroup view_navigation
    @param view data structure
    @details The line table is initialized with a specified increment size
   (LINE_TBL_INCR). Memory is allocated for the line table, and the first
   entry is set to 0, indicating the file position of the first line. The
   line index (view->ln_no) is initialized to 0.
 */
void initialize_line_table(View *view) {
    view->ln_tbl_size = LINE_TBL_INCR;
    view->ln_tbl = (off_t *)calloc(view->ln_tbl_size, sizeof(off_t));
    if (view->ln_tbl == nullptr) {
        Perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    view->ln_max_pos = 0;
    view->ln_tbl[0] = 0;
    view->ln_no = 0;
}
/** @brief Destroy Line Table
    @ingroup view_navigation
    @param view data structure
    @details This function frees the memory allocated for the line table and
   resets the relevant fields in the View structure. It ensures that the line
   table is properly cleaned up when it is no longer needed, preventing memory
   leaks.
 */
void destroy_line_table(View *view) {
    if (view->ln_tbl == nullptr)
        return;
    free(view->ln_tbl);
    view->ln_tbl = nullptr;
    view->ln_tbl_size = 0;
    view->ln_max_pos = 0;
    view->ln_no = 0;
}
/** @brief Increment Line Index and Update Line Table
    @ingroup view_navigation
    @param view data structure
    @details This function is called when a line feed character is
   encountered while reading the file. It increments the line index
   (view->ln_no) and checks if the current file position exceeds the maximum
   position recorded in the line table. If it does, it updates the line
   table with the new file position. If the line index exceeds the current
   size of the line table, the table is resized by allocating more memory.
 */
void increment_ln(View *view) {
    // view->ln_tbl[0] is set to 0 in initialize_line_table
    // view->ln_tbl[1] is the second line
    view->ln_no++;
    if (view->file_pos <= view->ln_max_pos)
        return;
    if (view->ln_no > view->ln_tbl_size - 1) {
        view->ln_tbl_size += LINE_TBL_INCR;
        view->ln_tbl =
            (off_t *)realloc(view->ln_tbl, view->ln_tbl_size * sizeof(off_t));
        if (view->ln_tbl == nullptr) {
            Perror("Memory allocation failed");
            exit(EXIT_FAILURE);
        }
    }
    view->ln_tbl_cnt = view->ln_no;
    view->ln_max_pos = view->file_pos;
    view->ln_tbl[view->ln_no] = view->file_pos;
}
/** @brief Synchronize Line Table with Current File Position
    @ingroup view_navigation
    @param view data Structure
    @details The line table (view->ln_tbl) is an array that stores the file
   position of each line. The index (view->ln_no + 1) corresponds to the
   current line number. (the line number table is 0-based, while line
   numbering starts at 1).
    @details If the line or position requested is not in the line table, this
   function reads forward to sycn.
    If the line or positione requested is behind the current line
   table index, the line index will be decremented it matches the file
   position.
 */
void sync_ln(View *view) {
    int c = 0;
    off_t idx;
    off_t target_pos;
    if (view->ln_tbl[view->ln_no] == view->file_pos)
        return;
    target_pos = view->file_pos;
    view->file_pos = view->ln_tbl[view->ln_tbl_cnt];
    if (view->file_pos < target_pos) {
        view->ln_no = view->ln_tbl_cnt;
        while (view->ln_max_pos < target_pos) {
            get_next_char();
            if (view->f_eod)
                return;
        }
    } else if (view->ln_tbl[view->ln_no] > target_pos) {
        idx = view->ln_no - 1;
        while (view->ln_tbl[idx] > target_pos)
            idx--;
        view->ln_no = idx;
        view->file_pos = view->ln_tbl[view->ln_no];
    } else {
        view->ln_no = view->ln_tbl_cnt;
        view->file_pos = view->ln_tbl[view->ln_no];
    }
}
/*------------------------------------------------------------
        END NAVIGATION
        BEGIN DISPLAY
  ------------------------------------------------------------*/
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
    touchwin(view->pad);
    rc = pnoutrefresh(view->pad, view->pminrow, view->pmincol, view->sminrow,
                      view->smincol, view->smaxrow, view->smaxcol);
    rc = prefresh(view->pad_view_win,
                  view->pminrow,
                  view->pmincol,
                  view->sminrow,
                  view->smincol,
                  view->smaxrow,
                  view->smaxcol);
    if (rc == ERR) {
        ssnprintf(em0, MAXLEN - 1, "%s:%d prefresh(view->pad_view_win, pminrow=%d, pmincol=%d, smaxrow=%d, smaxcol=%d) returned %d\n",
                  __FILE__, __LINE__, view->pminrow, view->pmincol, view->smaxrow, view->smaxcol, rc);
        Perror(em0);
    }
    return rc;
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
    int i = 0, j = 0, x = 0;
    int len = 0;
    int tab_spaces = 0;
    int char_width;
    attr_t attr = WA_NORMAL;
    int cpx = cp_nt;
    cchar_t cc = {0};
    wchar_t wstr[2] = {L'\0', L'\0'};
    char *in_str = view->line_in_s;
    int sl_cols = 0;
    int sl_cells = 0;
    int word_cells = 0;
    cchar_t *cmplx_buf = view->cmplx_buf;
    view->cur.sl_idx = 0;
    view->cur.sl_cnt = 0;
    view->cur.sl_cols[0] = 0;
    view->cur.sl_cells[0] = 0;
    view->cur.sl_s[0] = nullptr;
    view->cur.sl_cc[0] = nullptr;
    char *sl_s = view->stripped_line_out;
    cchar_t *sl_cc = view->cmplx_buf;
    rtrim(view->line_out_s);
    mbstate_t mbstate;
    memset(&mbstate, 0, sizeof(mbstate));
    int word_cols = 0;
    int sl_maxlen = PAD_COLS - 1;
    if (view->f_eod)
        return 0;
    if (view->wrap)
        sl_maxlen = view->cols;
    if (view->f_ln)
        sl_maxlen -= view->ln_win_cols;
    memset(view->stripped_line_out, 0, sizeof(view->stripped_line_out));
    while (in_str[i] != '\0') {        // line
        while (1) {                    // ANSI SGR, Character, and Word
            if (in_str[i] == '\033') { // ANSI SGR
                if (in_str[i + 1] == '[') {
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
                    i++;
                    continue;
                }
            } else { // Character and Word
                if (in_str[i] == ' ') {
                    if (view->wrap && (sl_cols + word_cols) > (sl_maxlen - 1))
                        break;
                    wstr[0] = L' ';
                    wstr[1] = L'\0';
                    setcchar(&cc, wstr, attr, cpx, nullptr);
                    view->stripped_line_out[x++] = ' ';
                    cmplx_buf[j++] = cc;
                    if (view->wrap) {
                        sl_cols += word_cols + 1;
                        word_cols = 0;
                        sl_cells += word_cells + 1;
                        word_cells = 0;
                    }
                    i++;
                    continue;
                }
                if (in_str[i] == '\0') {
                    if (view->wrap) {
                        if (sl_cols + word_cols + 1 > sl_maxlen - 1)
                            break;
                        sl_cols += word_cols;
                        word_cols = 0;
                        sl_cells += word_cells;
                        word_cells = 0;
                    }
                    break;
                }
                if (in_str[i] == '\t') {
                    tab_spaces = view->tab_stop - ((sl_cols + word_cols) % view->tab_stop);
                    if (view->wrap && sl_cols + word_cols + tab_spaces > sl_maxlen - 1)
                        break;
                    wstr[0] = L' ';
                    wstr[1] = L'\0';
                    setcchar(&cc, wstr, attr, cpx, nullptr);
                    for (int z = 0; z < tab_spaces; z++) {
                        view->stripped_line_out[x++] = ' ';
                        cmplx_buf[j++] = cc;
                    }
                    if (view->wrap) {
                        sl_cols += word_cols + tab_spaces;
                        word_cols = 0;
                        sl_cells += word_cells + tab_spaces;
                        word_cells = 0;
                    }
                    i++;
                    continue;
                }
                wstr[1] = L'\0';
                len = mbrtowc(wstr, &in_str[i], MB_CUR_MAX, &mbstate);
                if (len <= 0) {
                    wstr[0] = L'?';
                    wstr[1] = L'\0';
                    len = 1;
                }
                char_width = wcwidth(wstr[0]);
                view->stripped_line_out[x++] = in_str[i];
                if (char_width > 1)
                    for (int n = 1; n < char_width; n++)
                        view->stripped_line_out[x++] = ' ';
                setcchar(&cc, wstr, attr, cpx, nullptr);
                cmplx_buf[j++] = cc;
                i += len;
                if (view->wrap) {
                    word_cols += char_width;
                    word_cells++;
                }
                continue;
            } // END Character and Word
        } // END WHILE - ANSI SGR, Character, and Word
        if (view->wrap) {        // handle line wrapping
            if (word_cols > 0) { // break line, add last word to next line
                // if (view->cur.sl_idx == 0)
                //    view->cur.sl_ln_no = view->ln_no;
                if (sl_cols <= sl_maxlen - 1) {
                    view->cur.sl_s[view->cur.sl_idx] = sl_s;
                    view->cur.sl_cc[view->cur.sl_idx] = sl_cc;
                    view->cur.sl_cols[view->cur.sl_idx] = sl_cols;
                    view->cur.sl_cells[view->cur.sl_idx] = sl_cells;
                    view->cur.sl_idx++;
                    sl_s = &sl_s[sl_cols];
                    sl_cc = &sl_cc[sl_cells];
                    sl_cols > view->maxcol ? view->maxcol = sl_cols : 0;
                    sl_cols = word_cols;
                    sl_cells = word_cells;
                    word_cols = 0;
                    word_cells = 0;
                }
            }
            while (sl_cols > sl_maxlen - 1) { // split long lines
                int safe_cells = 0;
                int safe_cols = 0;
                while (safe_cols < sl_maxlen && safe_cells < sl_cells) {
                    wchar_t wstr_chk[CCHARW_MAX];
                    attr_t attr_chk;
                    short cpx_chk;
                    getcchar(&sl_cc[safe_cells], wstr_chk, &attr_chk, &cpx_chk, nullptr);
                    int cw = wcwidth(wstr_chk[0]);
                    if (safe_cols + cw > sl_maxlen)
                        break;
                    safe_cols += cw;
                    safe_cells++;
                }
                if (safe_cells == 0) {
                    safe_cells = 1;
                    safe_cols = sl_maxlen;
                }
                view->cur.sl_s[view->cur.sl_idx] = sl_s;
                view->cur.sl_cc[view->cur.sl_idx] = sl_cc;
                view->cur.sl_cols[view->cur.sl_idx] = safe_cols;
                view->cur.sl_cells[view->cur.sl_idx] = safe_cells;
                view->cur.sl_idx++;
                sl_s = &sl_s[safe_cells];
                sl_cc = &sl_cc[safe_cells];
                sl_cols > view->maxcol ? view->maxcol = sl_cols : 0;
                sl_cols -= safe_cols;
                sl_cells -= safe_cells;
                word_cols = 0;
                word_cells = 0;
            } // END WHILE - split long lines
        } // END IF - handle line wrapping
        j > view->maxcol ? view->maxcol = j : 0;
    } // END WHILE - line
    if (view->wrap) { // finish and commit wrap state
        if (view->cur.sl_idx > 0) {
            view->cur.sl_s[view->cur.sl_idx] = sl_s;
            view->cur.sl_cc[view->cur.sl_idx] = sl_cc;
            view->cur.sl_cols[view->cur.sl_idx] = sl_cols;
            view->cur.sl_cells[view->cur.sl_idx] = sl_cells;
            view->cur.sl_idx++;
            view->cur.sl_cc[view->cur.sl_idx] = nullptr;
            view->cur.sl_s[view->cur.sl_idx] = nullptr;
            view->cur.sl_cnt = view->cur.sl_idx;
            view->cur.sl_idx = 0;
        }
        view->cur.sl_ln_no = view->ln_no;
    }
#ifdef DEBUG_WRAP
    // log_split_lines(view);
    log_cc_buf(view);
    // log_stripped_line_out(view);
#endif
    //-------------------------------------------------------------------------
    wstr[0] = '\0';
    wstr[1] = '\0';
    setcchar(&cc, wstr, WA_NORMAL, cpx, nullptr);
    cmplx_buf[j] = cc;
    view->stripped_line_out[x] = '\0';
    return j;
}
/** @brief Log Stripped Line Output
    @ingroup view_display
    @param view pointer to View structure containing stripped line output
   buffer
    @details This function logs the contents of the stripped line output
   buffer (view->stripped_line_out) for debugging purposes. It iterates
   through the split lines (if any) and logs each line to the cmenu log.
 */
void log_stripped_line_out(View *view) {
    char tmp_str[PAD_COLS];

    write_cmenu_log("");
    write_cmenu_log("stripped_line_out");
    for (int k = 0; k < view->cur.sl_cnt; k++) {
        memset(tmp_str, 0, sizeof(tmp_str));
        for (int c = 0; c < view->cur.sl_cols[k]; c++)
            tmp_str[c] = view->cur.sl_s[k][c];
        write_cmenu_log(tmp_str);
    }
}
/** @brief Log Complex Character Buffer
    @ingroup view_display
    @param view pointer to View structure containing complex character buffer
   (view->cmplx_buf)
    @details This function logs the contents of the complex character buffer
   (view->cmplx_buf) for debugging purposes. It iterates through the split
   lines (if any) and logs each line to the cmenu log, converting wide
   characters to their corresponding single-byte representation for easier
   viewing.
 */
void log_cc_buf(View *view) {
    char tmp_str[PAD_COLS];

    write_cmenu_log("");
    write_cmenu_log("cc_buf");
    for (int k = 0; k < view->cur.sl_cnt; k++) {
        memset(tmp_str, 0, sizeof(tmp_str));
        for (int c = 0; c < view->cur.sl_cells[k]; c++) {
            wchar_t wstr[CCHARW_MAX];
            attr_t attr;
            short cpx;
            getcchar(&view->cur.sl_cc[k][c], wstr, &attr, &cpx, nullptr);
            if (wstr[0] == L'\0')
                tmp_str[c] = ' ';
            else
                tmp_str[c] = (char)wstr[0];
        }
        write_cmenu_log(tmp_str);
    }
}
/** @brief Log String with Specified Length
    @ingroup view_display
    @param str pointer to the string to log
    @param len length of the string to log
    @details This function logs a specified number of characters from a given
   string to the cmenu log. It copies the specified length of the string into
   a temporary buffer and then writes it to the log.
 */
void log_strnz(char *str, int len) {
    char tmp_str[MAXLEN];
    strnz__cpy(tmp_str, str, len);
    write_cmenu_log(tmp_str);
}
/** @brief Log Split Lines
    @ingroup view_display
    @param view pointer to View structure containing split line information
    @details This function logs the contents of the split lines stored in the
   view structure for debugging purposes. It iterates through the split lines
   and logs each line to the cmenu log, converting wide characters to their
   corresponding single-byte representation for easier viewing.
 */
void log_split_lines(View *view) {
    char tmp_str[MAXLEN];

    for (int k = 0; k < view->cur.sl_cnt; k++) {
        if (view->cur.sl_cols[k] > 0) {
            memset(tmp_str, 0, sizeof(tmp_str));
            char *s = view->cur.sl_s[k];
            char *d = tmp_str;
            char *e = view->cur.sl_s[k] + view->cur.sl_cols[k];
            while (s < e)
                *d++ = *s++;
            *d = '\0';
            write_cmenu_log(tmp_str);
        }
    }
}
//----------------------------------------------------------------------------------
/** @brief Parse ANSI SGR Escape Sequence
    @ingroup view_display
    @param ansi_str is the ANSI escape sequence string to parse
    @param attr is a pointer to an attr_t variable where the parsed
   attributes will be stored
    @param cpx is a pointer to an int variable where the parsed color pair
   index will be stored
    @details This function parses an ANSI escape sequence and updates the color
   pair and color tables according to the attributes specified. Despite the
   depth of nested conditionals, with an understanding of the ANSI Select
   Graphics Rendition (SGR) scheme, which is defined in the ECMA-48 standard
   (ISO/IEC 6429), you will find that it is exceptionally simple and
   straightforward.

   @verbatim

   This function converts the following SGR specification types to the
   appropriate curses color pair index for use in the terminal display.

    RGB:

        foreground \033[38;2;r;g;bm
        background \033[48;2;r;g;bm

        Where r, g, b are the red, green, and blue color components (0-255)

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
                        fg_clr = CLR_NT_FG;
                    else if (t0 == '4')
                        bg_clr = CLR_NT_BG;
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
                fg_clr = CLR_NT_FG;
                bg_clr = CLR_NT_BG;
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
            fg_clr = CLR_NT_FG;
            bg_clr = CLR_NT_BG;
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
        // wbkgrndset(view->cmdln_win, &CC_NT_HL_REV);
        // mvwadd_wchnstr(view->cmdln_win, view->cmd_line, 0, &ran, 1);
        // mvwadd_wchnstr(view->cmdln_win, view->cmd_line, 0, &sp, 1);
        wbkgrndset(view->cmdln_win, &CC_NT_REV);
        mvwaddstr(view->cmdln_win, view->cmd_line, 0, " ");
        mvwaddstr(view->cmdln_win, view->cmd_line, 1, message_str);
        waddstr(view->cmdln_win, " ");
        wbkgrndset(view->cmdln_win, &CC_NT);
        getyx(view->cmdln_win, view->cmd_line, view->curx);
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
        update_panels();
        doupdate();
        c = (char)vgetch(view->cmdln_win, -1);
        waddch(view->pad, (char)toupper(c));
        if (c == 'Y' || c == 'y')
            remove(view->cur_file_str);
    }
}
/** @brief Display View Help File
    @ingroup view_display
    @param init is the current initialization data structure.
    @details The current View context is set aside by assigning the view
   structure to "view_save" while the help file is displayed using a new,
   separate view structure.
    The help file is specified by the VIEW_HELP_FILE macro can be set
   to a default help file path or overridden by the user through an
   environment variable.
    After the help file is closed, the original view is restored and
   the page is redisplayed.
    It may be necessary to reassign view after calling this function
   because the init->view pointer is temporarily set to nullptr during the
   help file display, and the original view is restored afterward.
    The default screen size for help can be set in the code below. If
   set to 0, popup_view will determine reasonable maximal size based on the
   terminal dimensions.
    The help file may contain Unicode characters and ANSI escape
   sequences for formatting, which will be properly handled and displayed by
   popup_view. */
void view_display_help(Init *init) {
    char tmp_str[MAXLEN];
    int eargc = 0;
    char *eargv[MAXARGS];
    View *view = init->view;
    if (view->f_help_spec || view->help_spec[0] != '\0')
        strnz__cpy(tmp_str, view->help_spec, MAXLEN - 1);
    else {
        strnz__cpy(tmp_str, init->mapp_help, MAXLEN - 1);
        strnz__cat(tmp_str, "/", MAXLEN - 1);
        strnz__cat(tmp_str, VIEW_HELP_FILE, MAXLEN - 1);
    }
    eargv[eargc++] = strdup("view");
    eargv[eargc++] = strdup("-N");
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
/*------------------------------------------------------------
      END DISPLAY
  ------------------------------------------------------------*/
/** @brief use form to enter a file specification
    @ingroup view_engine
    @param init data structure
    @param file_spec - pointer to file specification
    the file_spec
    @returns true if successful
    @details the user must provide a character array large enough to
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
    View *view = init->view;
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
        if (rc == FA_CANCEL || rc == 'q' || rc == 'Q' || rc == KEY_F(9))
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
