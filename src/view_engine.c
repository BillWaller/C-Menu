/** @file view_engine.c
    @brief The working part of View
    @author Bill Waller
    Copyright (c) 2025
    MIT License
    billxwaller@gmail.com
    @date 2026-02-09
 */

#include "common.h"
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <regex.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#define Ctrl(c) ((c) & 0x1f)

/** @brief read the next characater from the virtual file */
#define get_next_char()                                                        \
    do {                                                                       \
        if (view->file_pos == view->file_size) {                               \
            view->f_eod = true;                                                \
            break;                                                             \
        } else                                                                 \
            view->f_eod = false;                                               \
        c = view->buf[view->file_pos++];                                       \
    } while (c == 0x0d);

/** @brief read the previous characater from the virtual file */
#define get_prev_char()                                                        \
    do {                                                                       \
        if (view->file_pos == 0) {                                             \
            view->f_bod = true;                                                \
            break;                                                             \
        } else                                                                 \
            view->f_bod = false;                                               \
        c = view->buf[--view->file_pos];                                       \
    } while (c == 0x0d);

char prev_regex_pattern[MAXLEN];
FILE *dbgfp;
int view_file(Init *);
int view_cmd_processor(Init *);
int get_cmd_char(View *, off_t *);
int get_cmd_arg(View *, char *);
void build_prompt(View *, int, char *, double elapsed);
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
void resize_page(Init *);
void redisplay_page(View *);
void scroll_down_n_lines(View *, int);
void scroll_up_n_lines(View *, int);
off_t get_next_line(View *, off_t);
off_t get_prev_line(View *, off_t);
off_t get_pos_next_line(View *, off_t);
off_t get_pos_prev_line(View *, off_t);
int fmt_line(View *);
void display_line(View *);
void parse_ansi_str(char *, attr_t *, int *);
void view_display_help(Init *);
void cmd_line_prompt(View *, char *);
void remove_file(View *);
int write_view_buffer(Init *, bool);
bool enter_file_spec(Init *, char *);
int a_toi(char *s, bool *a_toi_error);

char err_msg[MAXLEN];
/** @brief Start view */
int view_file(Init *init) {
    view = init->view;
    if (view->argc < 1) {
        view->curr_argc = -1;
        view->argc = 0;
        view->next_file_spec_ptr = "-";
    } else
        view->next_file_spec_ptr = view->argv[0];
    while (view->curr_argc < view->argc) {
        if (view->next_file_spec_ptr == NULL ||
            *view->next_file_spec_ptr == '\0') {
            break;
        }
        view->file_spec_ptr = view->next_file_spec_ptr;
        view->next_file_spec_ptr = NULL;
        if (view_init_input(view, view->file_spec_ptr)) {
            if (view->buf) {
                view->f_eod = 0;
                view->f_bod = 0;
                view->maxcol = 0;
                view->page_top_pos = 0;
                view->page_bot_pos = 0;
                view->file_pos = 0;
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
/** @brief Main Command Processing Loop for View */
int view_cmd_processor(Init *init) {
    int tfd;
    int c;
    int max_n;
    int shift = 0;
    int search_cmd = 0;
    int prev_search_cmd = 0;
    int rc, i;
    int n = 0;
    ssize_t bytes_written;
    char *e;
    char shell_cmd_spec[MAXLEN];
    struct timespec start, end;
    double elapsed = 0;
    bool f_clock_started = false;
    off_t n_cmd = 0L;
    off_t prev_file_pos;
    view = init->view;
    view->f_timer = false;
    view->cmd[0] = '\0';
    while (1) {
        c = view->next_cmd_char;
        view->next_cmd_char = 0;
        if (!c) {
            if (view->f_redisplay_page)
                redisplay_page(view);
            view->f_redisplay_page = false;
            if (view->f_timer && f_clock_started) {
                clock_gettime(CLOCK_MONOTONIC, &end);
                elapsed = (end.tv_sec - start.tv_sec) +
                          (end.tv_nsec - start.tv_nsec) / 1e9;
                f_clock_started = false;
            }

            if (view->tmp_prompt_str[0] != '\0') {
                cmd_line_prompt(view, view->tmp_prompt_str);
                view->tmp_prompt_str[0] = '\0';
            } else {
                build_prompt(view, view->prompt_type, view->prompt_str,
                             elapsed);
                if (view->prompt_str[0] == '\0')
                    cmd_line_prompt(view, ">");
                else
                    cmd_line_prompt(view, view->prompt_str);
            }
            rc =
                prefresh(view->win, view->pminrow, view->pmincol, view->sminrow,
                         view->smincol, view->smaxrow, view->smaxcol);
            if (rc == ERR)
                Perror("Error refreshing pad");
            c = get_cmd_char(view, &n_cmd);
            if (view->f_timer) {
                clock_gettime(CLOCK_MONOTONIC, &start);
                f_clock_started = true;
            }
            if (c >= '0' && c <= '9') {
                tmp_str[0] = (char)c;
                tmp_str[1] = '\0';
                c = get_cmd_arg(view, tmp_str);
            }
        }
        switch (c) {
        case Ctrl('R'): /**<  Ctrl('R') or KEY_RESIZE - Handle terminal resize
                           events */
        case KEY_RESIZE:
            resize_page(init);
            break;
        case KEY_ALTHOME: /**< KEY_ALTHOME - horizontal scroll to the first
                             column */
            view->pmincol = 0;
            break;
        case KEY_ALTEND: /**< KEY_ALTEND  horizontal scroll to the last column
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
            shift = (view->cols / 3) * 2;
            max_n = view->pmincol / shift;
            n = (int)n_cmd;
            if (n > max_n)
                n = max_n;
            shift *= n;
            if (view->pmincol - shift < 0)
                view->pmincol = 0;
            else
                view->pmincol -= shift;
            view->f_redisplay_page = true;
            break;
        case 'l': /**< 'l', 'L', KEY_RIGHT - Horizontal scroll right by two
                     thirds of the page width */
        case 'L':
        case KEY_RIGHT:
            if (n_cmd <= 0)
                n_cmd = 1;
            shift = (view->cols / 3) * 2;
            max_n = (view->maxcol - view->pmincol) / shift;
            n = (int)n_cmd;
            if (n > max_n)
                n = max_n;
            shift *= n;
            if (view->pmincol + shift <= view->maxcol)
                view->pmincol += shift;
            else
                view->pmincol = (view->maxcol - shift) > 0
                                    ? (view->maxcol - view->cols)
                                    : 0;
            view->f_redisplay_page = true;
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
        /** 'b', 'B', Ctrl('B'), KEY_PPAGE - Scroll up one page */
        case KEY_PPAGE:
        case 'b':
        case 'B':
        case Ctrl('B'):
            scroll_up_n_lines(view, view->scroll_lines);
            break;
        /**  'f', 'F', Ctrl('F'), KEY_NPAGE - scroll down one page */
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
            go_to_line(view, 0L);
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
                    munmap(view->buf, view->file_size);
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
            if (view->f_displaying_help)
                break;
            cmd_line_prompt(view, "(c, i, p, s, t, or h)->");
            c = get_cmd_char(view, &n_cmd);
            c = tolower(c);
            if (c >= 'A' && c <= 'Z')
                c += ' ';
            switch (c) {
            /**  'c' - Clear Screen at End of File */
            case 'c':
                cmd_line_prompt(view, "Clear Screen at End (Y or N)->");
                if ((c = get_cmd_char(view, &n_cmd)) == 'y' || c == 'Y')
                    view->f_at_end_clear = true;
                else if (c == 'n' || c == 'N')
                    view->f_at_end_clear = false;
                break;
            /**  'i' - Ignore Case in Search */
            case 'i':
                cmd_line_prompt(view, "Ignore Case in search (Y or N)->");
                if ((c = get_cmd_char(view, &n_cmd)) == 'y' || c == 'Y')
                    view->f_ignore_case = true;
                else if (c == 'n' || c == 'N')
                    view->f_ignore_case = false;
                break;
            /** 'p' - Set Prompt Type */
            case 'p':
                cmd_line_prompt(view, "(Short Long or No prompt)->");
                c = tolower(get_cmd_char(view, &n_cmd));
                switch (c) {
                case 's':
                    view->prompt_type = PT_SHORT;
                    break;
                /** 'l' - Long Prompt */
                case 'l':
                    view->prompt_type = PT_LONG;
                    break;
                /**  'n' - No Prompt */
                case 'n':
                    view->prompt_type = PT_NONE;
                    break;
                default:
                    break;
                }
                break;
            /** 's' - Squeeze Multiple Blank Lines */
            case 's':
                cmd_line_prompt(
                    view, "view->f_squeeze Multiple Blank lines (Y or N)->");
                if ((c = get_cmd_char(view, &n_cmd)) == 'y' || c == 'Y')
                    view->f_squeeze = true;
                else if (c == 'n' || c == 'N')
                    view->f_squeeze = false;
                break;
            /** 't' - Set Tab Stop Columns */
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
            /** KEY_F(1), 'H' - Display Help */
            case 'H':
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
                view->cury = view->scroll_lines;
                view->srch_curr_pos = view->page_top_pos;
            }
            view->f_first_iter = true;
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
                view->srch_beg_pos = view->page_top_pos;
                view->srch_curr_pos = view->page_top_pos;
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
                /** Pedantic reassignment of view pointer as the
                    init->view pointer was was reassigned during
                   view_display_help */
                view = init->view;
                view = init->view;
            }
            break;
        /**  'm' - Set a Mark at the Current Position */
        case 'm':
            cmd_line_prompt(view, "Mark label (A-Z)->");
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
            cmd_line_prompt(view, "Goto mark (A-Z)->");
            c = get_cmd_char(view, &n_cmd);
            if (c == '@' || c == KEY_F(9) || c == '\033')
                break;
            // c = tolower(c);
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
            prefresh(view->win, view->pminrow, view->pmincol, view->sminrow,
                     view->smincol, view->smaxrow, view->smaxcol);
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
            view->next_file_spec_ptr = NULL;
            return 0;
        /** 'v' - Open Current File in Editor */
        case 'v':
            if (init->editor[0] == 0) {
                e = getenv("DEFAULTEDITOR");
                if (e == NULL || *e == '\0')
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
            rc = display_error(em0, em1, em2, NULL);
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
                display_error(em0, em1, NULL, NULL);
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
            prev_file_pos = view->page_top_pos;
            bytes_written = write_view_buffer(init, view->f_strip_ansi);
            cmd_line_prompt(view, tmp_str);
            view->f_redisplay_page = true;
            break;
        case CT_VIEW:
            break;
        /** 'V' - Display Version Information */
        case 'V':
            ssnprintf(em0, MAXLEN - 1, "C-Menu Version: %s", CM_VERSION);
            display_error(em0, em1, NULL, NULL);
            break;
        default:
            break;
        }
        view->cmd_arg[0] = '\0';
    }
}
/** @brief Get Command Character and Numeric Argument */
int get_cmd_char(View *view, off_t *n) {
    int c = 0, i = 0;
    char cmd_str[33];
    cmd_str[0] = '\0';
    MEVENT event;
    mousemask(BUTTON4_PRESSED | BUTTON5_PRESSED, NULL);
    tcflush(2, TCIFLUSH);
    do {
        c = xwgetch(view->win);
        if (c == KEY_MOUSE) {
            if (getmouse(&event) != OK)
                return (MA_ENTER_OPTION);
            if (event.bstate & BUTTON4_PRESSED)
                return (KEY_UP);
            else if (event.bstate & BUTTON5_PRESSED) {
                return (KEY_DOWN);
            }
        } else {
            if (c >= '0' && c <= '9' && i < 32) {
                cmd_str[i++] = (char)c;
                cmd_str[i] = '\0';
            }
        }
    } while (c >= '0' && c <= '9');
    *n = atol(cmd_str);
    view->cmd_arg[0] = '\0';
    return (c);
}
/** @brief Get Command Argument from User Input */
int get_cmd_arg(View *view, char *prompt) {
    int c;
    int numeric_arg = false;
    char *cmd_p;
    char *cmd_e;
    char prompt_s[PAD_COLS + 1];
    char *n;
    int rc, prompt_l;
    prompt_l = strnz__cpy(prompt_s, prompt, view->cols - 4);
    if (view->cmd_arg[0] != '\0')
        return 0;
    cmd_p = view->cmd_arg;
    cmd_e = view->cmd_arg + MAXLEN - 2;
    wmove(view->win, view->cmd_line, 0);
    if (prompt_l == 0)
        numeric_arg = true;
    if (prompt_l > 1) {
        wstandout(view->win);
        waddch(view->win, ' ');
        waddstr(view->win, prompt_s);
        waddch(view->win, ' ');
        wstandend(view->win);
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
        waddstr(view->win, prompt_s);
        wmove(view->win, view->cmd_line, prompt_l);
    }
    wclrtoeol(view->win);
    while (1) {
        rc = prefresh(view->win, view->pminrow, view->pmincol, view->sminrow,
                      view->smincol, view->smaxrow, view->smaxcol);
        if (rc == ERR)
            Perror("Error refreshing screen");
        c = xwgetch(view->win);
        switch (c) {
        /// Basic Editing Keys for Command Line
        case KEY_LEFT:
        case KEY_BACKSPACE:
        case '\b':
            if (cmd_p > view->cmd_arg) {
                cmd_p--;
                if (*cmd_p < ' ' || *cmd_p == 0x7f) {
                    getyx(view->win, view->cury, view->curx);
                    if (view->curx > 0) {
                        view->curx--;
                        wmove(view->win, view->cmd_line, view->curx);
                        waddch(view->win, ' ');
                        wmove(view->win, view->cmd_line, view->curx);
                    }
                }
                getyx(view->win, view->cury, view->curx);
                if (view->curx > 0) {
                    view->curx--;
                    wmove(view->win, view->cmd_line, view->curx);
                    waddch(view->win, ' ');
                    wmove(view->win, view->cmd_line, view->curx);
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
                waddch(view->win, '^');
                c |= '@';
            } else if ((uchar)c == 0x7f)
                c = '?';
            waddch(view->win, (char)c);
            if (cmd_p >= cmd_e)
                return 0;
            if (numeric_arg && (c < '0' || c > '9'))
                return -1;
            break;
        }
    }
    return c;
}
/** @brief Build Prompt String */
void build_prompt(View *view, int prompt_type, char *prompt_str,
                  double elapsed) {
    prompt_type = PT_LONG;
    *prompt_str = '\0';
    if (prompt_type == PT_LONG) {
        if (view->f_is_pipe)
            strnz__cat(prompt_str, "stdin", MAXLEN - 1);
        else
            strnz__cat(prompt_str, view->file_name, MAXLEN - 1);
    }
    if (view->pmincol > 0) {
        sprintf(tmp_str, "Col %d of %d", view->pmincol, view->maxcol);
        if (prompt_str[0] != '\0')
            strnz__cat(prompt_str, "|", MAXLEN - 1);
        strnz__cat(prompt_str, tmp_str, MAXLEN - 1);
    }
    if (view->argc > 1 && prompt_type == PT_LONG) {
        sprintf(tmp_str, "File %d of %d", view->curr_argc + 1, view->argc);
        if (prompt_str[0] != '\0') {
            strnz__cat(prompt_str, "|", MAXLEN - 1);
            strnz__cat(prompt_str, tmp_str, MAXLEN - 1); // File Of
        }
    }
    if (prompt_type == PT_LONG) { // Byte of Byte
        if (view->page_top_pos == NULL_POSITION)
            view->page_top_pos = view->file_size;
        sprintf(tmp_str, "Pos %zd-%zd", view->page_top_pos, view->page_bot_pos);
        if (prompt_str[0] != '\0') {
            strnz__cat(prompt_str, "|", MAXLEN - 1);
            strnz__cat(prompt_str, tmp_str, MAXLEN - 1);
        }
        if (!view->f_is_pipe) {
            if (view->file_size > 0) {
                sprintf(tmp_str, " of %zd", view->file_size);
                strnz__cat(prompt_str, tmp_str, MAXLEN - 1);
            }
        }
    }
    if (!view->f_eod && prompt_type != PT_NONE) { // Percent
        if (view->file_size > 0 && view->page_bot_pos != 0) {
            sprintf(tmp_str, "(%zd%%)",
                    (100 * view->page_bot_pos) / view->file_size);
            if (prompt_str[0] != '\0')
                strnz__cat(prompt_str, "|", MAXLEN - 1);
            strnz__cat(prompt_str, tmp_str, MAXLEN - 1);
        }
    }
    if (view->f_eod) { // End
        if (prompt_str[0] != '\0')
            strnz__cat(prompt_str, " ", MAXLEN - 1);
        strnz__cat(prompt_str, "(End)", MAXLEN - 1);
        if (view->curr_argc + 1 < view->argc) {
            base_name(tmp_str, view->argv[view->curr_argc + 1]);
            strnz__cpy(prompt_str, " Next File: ", MAXLEN - 1);
            strnz__cat(prompt_str, tmp_str, MAXLEN - 1);
        }
    }
    if (view->f_timer) {
        sprintf(tmp_str, " secs. %.6f\n", elapsed);
        strnz__cat(prompt_str, tmp_str, MAXLEN - 1);
    }
}
/** @brief Write buffer contents to files
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
        rc = display_error(em0, em1, NULL, NULL);
        if (rc == 'y' || rc == 'Y')
            f_strip_ansi = true;
        else
            f_strip_ansi = false;
    }
    /** write the buffer */
    pos = 0;
    view->out_fd = open(view->out_spec, O_CREAT | O_TRUNC | O_WRONLY, 0644);
    if (view->out_fd == -1) {
        ssnprintf(em0, MAXLEN - 1, "%s, line: %d", __FILE__, __LINE__ - 2);
        strnz__cpy(em1, "fwrite ", MAXLEN - 1);
        strnz__cat(em1, view->out_spec, MAXLEN - 1);
        strerror_r(errno, em2, MAXLEN - 1);
        display_error(em0, em1, em2, NULL);
        return (false);
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
/** @brief Concatenate File to Standard Output */
void cat_file(View *view) {
    int c;
    while (1) {
        get_next_char();
        if (view->f_eod)
            break;
        putchar(c);
    }
}
/** @brief Send File to Print Queue */
void lp(char *PrintFile) {
    /// Send File to Print Queue
    char *print_cmd_ptr;
    char shell_cmd_spec[MAXLEN];
    print_cmd_ptr = getenv("PRINTCMD");
    if (print_cmd_ptr == NULL || *print_cmd_ptr == '\0')
        print_cmd_ptr = PRINTCMD;
    sprintf(shell_cmd_spec, "%s %s", print_cmd_ptr, PrintFile);
    cmd_line_prompt(view, shell_cmd_spec);
    //  wrefresh(view->win);
    prefresh(view->win, view->pminrow, view->pmincol, view->sminrow,
             view->smincol, view->smaxrow, view->smaxcol);
    shell(shell_cmd_spec);
}
/** @brief Go to Mark */
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
/** @brief Go to End of File */
void go_to_eof(View *view) {
    int c;
    view->file_pos = view->file_size;
    view->page_top_pos = view->file_pos;
    get_prev_char();
    prev_page(view);
}
/** @brief Go to Specific Line */
int go_to_line(View *view, off_t line_idx) {
    int c = 0;
    off_t line_cnt = 0;
    if (line_idx <= 1) {
        go_to_position(view, 0);
        return EOF;
    }
    view->file_pos = 0;
    view->page_top_pos = view->file_pos;
    line_idx = 0;
    view->f_eod = false;
    do {
        while (c != '\n') {
            get_next_char();
            if (view->f_eod) {
                sprintf(tmp_str, "End of data at %ld lines", line_cnt - 1);
                Perror(tmp_str);
                return EOF;
            }
        }
        get_next_char();
    } while (line_cnt < line_idx - 1);
    view->page_top_pos = view->file_pos;
    prev_page(view);
    return 0;
}
/** @brief Go to Percent of File */
void go_to_percent(View *view, int Percent) {
    /// Go to Percent of File
    int c = 0;
    if (view->file_size < 0) {
        Perror("Cannot determine file length");
        return;
    }
    view->file_pos = (Percent * view->file_size) / 100;
    get_next_char();
    while (c != '\n') {
        get_prev_char();
        if (view->f_bod)
            break;
    }
    get_next_char();
    next_page(view);
}
/** @brief Go to Specific File Position */
void go_to_position(View *view, off_t go_to_pos) {
    view->file_pos = go_to_pos;
    view->page_bot_pos = view->file_pos;
    next_page(view);
}
/** @brief Search for Regular Expression Pattern
    @param view Pointer to View Structure
    @param search_cmd Search Command Character ('/' or '?')
    @param regex_pattern Regular Expression Pattern to Search For
    @returns true if a match is found and displayed, false if the search
   completes without finding a match or if an error occurs
    @note The search performs extended regular expression matching,
   ignoring ANSI sequences and Unicode characters. Matches are
   highlighted on the screen, and the search continues until the page is
   full or the end of the file is reached. If the search wraps around
   the file, a message is displayed indicating that the search is
   complete.
    @note The search state is maintained in the view structure, allowing
   for repeat searches and tracking of the current search position.
    @note this function highlights all matches in the current ncurses
   pad, including those not displayed on the screen, and tracks the
   first and last match columns for prompt display.
    @note ANSI sequences and Unicode characters are stripped before
   matching, so matching corresponds to the visual display */
bool search(View *view, int *search_cmd, char *regex_pattern) {
    int REG_FLAGS = 0;
    regmatch_t pmatch[1];
    regex_t compiled_regex;
    int reti;
    int line_offset;
    int line_len;
    int match_len;
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
            if (view->srch_curr_pos == view->file_size) {
                view->srch_curr_pos = 0;
                if (view->srch_beg_pos == view->file_size)
                    view->srch_beg_pos = 0;
            }
        } else {
            if (view->srch_curr_pos == 0) {
                view->srch_curr_pos = view->file_size;
                if (view->srch_beg_pos == 0)
                    view->srch_beg_pos = view->file_size;
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
        }
        /** get line to scan */
        if (*search_cmd == '/') {
            if (view->cury == view->scroll_lines)
                return true;
            view->srch_curr_pos = get_next_line(view, view->srch_curr_pos);
            view->page_bot_pos = view->srch_curr_pos;
        } else {
            if (view->cury == 0)
                return true;
            view->srch_curr_pos = get_prev_line(view, view->srch_curr_pos);
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
            wmove(view->win, view->cury, 0);
            wclrtobot(view->win);
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
            mvwchgat(view->win, view->cury - 1, view->curx, match_len,
                     WA_REVERSE, cp_norm, NULL);
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
    /** Update view positions and prepare prompt with match info */
    view->file_pos = view->srch_curr_pos;
    view->page_bot_pos = view->srch_curr_pos;
#ifdef DEBUG
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
/** @brief Resize Viewing Page
    @param init data structure */
void resize_page(Init *init) {
    int scr_lines, scr_cols;
    bool f_resize = false;
    view = init->view;
    if (view->f_full_screen) {
        getmaxyx(stdscr, view->lines, view->cols);
        view->scroll_lines = view->lines - 1;
        view->cmd_line = view->lines - 1;
        view->smaxrow = view->lines - 1;
        view->smaxcol = view->cols - 1;
        wresize(view->win, view->lines, view->cols);
        wrefresh(view->win);
        wsetscrreg(view->win, 0, view->scroll_lines);
        f_resize = true;
    } else {
        getmaxyx(stdscr, scr_lines, scr_cols);
        if (view->begy + view->lines + 2 > scr_lines) {
            view->lines = (scr_lines - view->begy) - 2;
            f_resize = true;
        }
        if (view->begx + view->cols + 2 > scr_cols) {
            view->cols = (scr_cols - view->begx) - 2;
            f_resize = true;
        }
        if (f_resize) {
            view->scroll_lines = view->lines - 1;
            view->cmd_line = view->lines - 1;
            view->smaxrow = view->lines - 1;
            view->smaxcol = view->cols - 1;
            win_resize(view->lines + 2, view->cols + 2, view->title);
            restore_wins();
            wsetscrreg(view->win, 0, view->scroll_lines);
        }
    }
    if (f_resize)
        view->f_redisplay_page = true;
    else
        view->f_redisplay_page = false;
}
/** @brief Redisplay Current Page */
void redisplay_page(View *view) {
    int i;
    int line_len;
    view->cury = 0;
    wmove(view->win, view->cury, 0);
    view->page_bot_pos = view->page_top_pos;
    for (i = 0; i < view->scroll_lines; i++) {
        view->page_bot_pos = get_next_line(view, view->page_bot_pos);
        if (view->f_eod)
            break;
        line_len = fmt_line(view);
        if (line_len > view->maxcol)
            view->maxcol = line_len;
        display_line(view);
    }
}
/** @brief Advance to Next Page */
void next_page(View *view) {
    int i;
    int line_len;
    curs_set(0);
    if (view->page_bot_pos == view->file_size)
        return;
    view->maxcol = 0;
    view->cury = 0;
    view->file_pos = view->page_bot_pos;
    view->page_top_pos = view->file_pos;
    wmove(view->win, view->cury, 0);
    wclrtobot(view->win);
    for (i = 0; i < view->scroll_lines; i++) {
        get_next_line(view, view->file_pos);
        if (view->f_eod)
            break;
        line_len = fmt_line(view);
        if (line_len > view->maxcol)
            view->maxcol = line_len;
        display_line(view);
    }
    view->page_bot_pos = view->file_pos;
    curs_set(1);
}
/** @brief display previous page */
void prev_page(View *view) {
    int i;
    curs_set(0);
    if (view->page_top_pos == 0)
        return;
    view->maxcol = 0;
    view->cury = 0;
    wmove(view->win, view->cury, 0);
    view->file_pos = view->page_top_pos;
    view->page_bot_pos = view->file_pos;
    for (i = 0; i < view->scroll_lines; i++) {
        get_pos_prev_line(view, view->file_pos);
        if (view->f_bod)
            break;
    }
    view->page_bot_pos = view->file_pos;
    next_page(view);
}
/** @brief Scroll N Lines */
void scroll_down_n_lines(View *view, int n) {
    int i = 0;
    int line_len;
    curs_set(0);
    if (view->page_bot_pos == view->file_size)
        return;
    // Locate New Top of Page
    view->file_pos = view->page_top_pos;
    for (i = 0; i < n; i++) {
        view->page_top_pos = get_pos_next_line(view, view->file_pos);
        if (view->f_eod)
            break;
    }
    n = i;
    // Scroll
    wscrl(view->win, n);
    // Fill in Page Bottom
    view->cury = view->scroll_lines - n;
    wmove(view->win, view->cury, 0);
    view->file_pos = view->page_bot_pos;
    for (i = 0; i < n; i++) {
        get_next_line(view, view->file_pos);
        if (view->f_eod)
            break;
        line_len = fmt_line(view);
        if (line_len > view->maxcol)
            view->maxcol = line_len;
        display_line(view);
    }
    view->page_bot_pos = view->file_pos;
    curs_set(1);
}
/** @brief Scroll Up N Lines */
void scroll_up_n_lines(View *view, int n) {
    int i;
    int line_len;
    curs_set(0);
    if (view->page_top_pos == 0)
        return;
    // Locate New Top of Page
    for (i = 0; i < n; i++) {
        if (view->f_bod)
            break;
        view->page_top_pos = get_pos_prev_line(view, view->page_top_pos);
    }
    n = i;
    // Locate New Bottom of Page
    view->f_bod = false;
    for (i = 0; i < n; i++) {
        if (view->f_bod)
            break;
        view->page_bot_pos = get_pos_prev_line(view, view->page_bot_pos);
    }
    // Scroll Up
    if (n < view->scroll_lines)
        wscrl(view->win, -n);
    // Fill in Page Top
    view->cury = 0;
    wmove(view->win, view->cury, 0);
    view->file_pos = view->page_top_pos;
    for (i = 0; i < n; i++) {
        if (view->f_eod)
            break;
        get_next_line(view, view->file_pos);
        line_len = fmt_line(view);
        if (line_len > view->maxcol)
            view->maxcol = line_len;
        display_line(view);
    }
    curs_set(1);
    return;
}
/** @brief Get Next Line from File
    @param view struct
    @param pos buffer offset
    @returns file position of next line
    @note gets view->line_in_s
 */
off_t get_next_line(View *view, off_t pos) {
    uchar c;
    char *line_in_p;
    view->file_pos = pos;
    view->f_eod = false;
    do {
        if (view->file_pos == view->file_size) {
            view->f_eod = true;
            break;
        }
        c = view->buf[view->file_pos++];
    } while (c == 0x0d);
    if (view->f_eod)
        return view->file_pos;
    line_in_p = view->line_in_s;
    view->line_in_beg_p = view->line_in_s;
    view->line_in_end_p = view->line_in_s + PAD_COLS;
    while (1) {
        if (c == (uchar)'\n')
            break;
        if (line_in_p >= view->line_in_end_p)
            break;
        *line_in_p++ = c;
        do {
            if (view->file_pos == view->file_size) {
                view->f_eod = true;
                break;
            }
            c = view->buf[view->file_pos++];
        } while (c == 0x0d);
        if (view->f_eod)
            return view->file_pos;
    }
    *line_in_p = '\0';
    if (view->f_squeeze) {
        while (1) {
            get_next_char();
            if (view->f_eod)
                break;
            if (c != (uchar)'\n')
                break;
        }
        get_prev_char();
    }
    return view->file_pos;
}
/** @brief Get Previous Line from File */
off_t get_prev_line(View *view, off_t pos) {
    pos = get_pos_prev_line(view, pos);
    view->file_pos = pos;
    get_next_line(view, view->file_pos);
    return pos;
}
/** @brief Get Position of Next Line */
off_t get_pos_next_line(View *view, off_t pos) {
    uchar c;
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
        if (view->f_eod)
            return view->file_pos;
    }
    while (!view->f_eod) {
        if (c == '\n')
            break;
        get_next_char();
    }
    return view->file_pos;
}
/** @brief Get Position of Previous Line */
off_t get_pos_prev_line(View *view, off_t pos) {
    uchar c;
    view->file_pos = pos;
    if (view->file_pos == 0) {
        view->f_bod = true;
        return view->file_pos;
    }
    get_prev_char();
    if (view->f_bod)
        return view->file_pos;
    if (c == '\n') {
        get_prev_char();
        if (view->f_bod)
            return view->file_pos;
    }
    while (!view->f_bod) {
        if (c == '\n') {
            get_next_char();
            break;
        }
        get_prev_char();
    }
    return view->file_pos;
}
/** @brief Display Line on Pad */
void display_line(View *view) {
    int rc;
    if (view->cury < 0)
        view->cury = 0;
    if (view->cury > view->scroll_lines - 1)
        view->cury = view->scroll_lines - 1;
    wmove(view->win, view->cury, 0);
    wclrtoeol(view->win);
    wadd_wchstr(view->win, view->cmplx_buf);
    view->cury++;
    refresh();
    rc = prefresh(view->win, view->pminrow, view->pmincol, view->sminrow,
                  view->smincol, view->smaxrow, view->smaxcol);
    if (rc == ERR)
        Perror("Error refreshing screen");
}
/** @brief Format Line for Display
    @param view pointer to View structure containing line input and
   output buffers
    @return length of formatted line in characters
    @details This function processes the input line from
   view->line_in_s, handling ANSI escape sequences for text
   attributes and colors, as well as multi-byte characters. It
   converts the input line into a formatted line suitable for
   display in the terminal, storing the result in view->cmplx_buf
   and view->stripped_line_out. The function returns the length of
   the formatted line in characters, which may be used for tracking
   the maximum column width of the displayed content.
 */
int fmt_line(View *view) {
    attr_t attr = WA_NORMAL;
    char ansi_tok[MAXLEN];
    int cpx = cp_norm;
    int i = 0, j = 0;
    int len = 0;
    const char *s;
    wchar_t wc = L'\0';
    cchar_t cc = {0};
    char *in_str = view->line_in_s;
    cchar_t *cmplx_buf = view->cmplx_buf;

    rtrim(view->line_out_s);
    /** Initialize multibyte to wide char conversion */
    mbstate_t mbstate;
    memset(&mbstate, 0, sizeof(mbstate));
    while (in_str[i] != '\0') {
        if (in_str[i] == '\033' && in_str[i + 1] == '[') {
            /** Handle ANSI escape sequences */
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
            /** Wide and complex characters */
            if (in_str[i] == '\033') {
                i++;
                continue;
            }
            s = &in_str[i];
            if (*s == '\t') {
                wc = L' ';
                do {
                    setcchar(&cc, &wc, attr, cpx, NULL);
                    view->stripped_line_out[j] = ' ';
                    cmplx_buf[j++] = cc;
                } while ((j < PAD_COLS - 2) && (j % view->tab_stop != 0));
                i++;
            } else {
                len = mbrtowc(&wc, s, MB_CUR_MAX, &mbstate);
                if (len <= 0) {
                    wc = L'?';
                    len = 1;
                }
                if (setcchar(&cc, &wc, attr, cpx, NULL) != ERR) {
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
    wc = L'\0';
    setcchar(&cc, &wc, WA_NORMAL, cpx, NULL);
    cmplx_buf[j] = cc;
    view->stripped_line_out[j] = '\0';
    return j;
}
/** @brief Parse ANSI SGR Escape Sequence
    @param ansi_str is the ANSI escape sequence string to parse
    @param attr is a pointer to an attr_t variable where the parsed
   attributes will be stored
    @param cpx is a pointer to an int variable where the parsed
   color pair index will be stored
    @details This function parses an ANSI SGR (Select Graphic
   Rendition) escape sequence and updates the provided attr_t and
   color pair index based on the attributes specified in the ANSI
   string.

    @code

    This function converts the following SGR specification types to
   the appropriate curses color pair index for use in the terminal
   display.

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

        Where c is the color code (0 for black, 1 for red, 2 for
   green, 3 for yellow, 4 for blue, 5 for magenta, 6 for cyan, 7 for
   white).

    Attributes:

        \033[am

        Where a is the attribute code (1 for bold, 2 for dim, 3 for
   italic, 4 for underline, 5 for blink, 7 for reverse, 8 for
   invis). The function also supports resetting attributes and
   colors to default using \033[0m.

    @sa xterm256_idx_to_rgb(), rgb_to_curses_clr(),
   extended_pair_content(), get_clr_pair()

    @endcode
*/
void parse_ansi_str(char *ansi_str, attr_t *attr, int *cpx) {
    char *tok;
    char t0, t1;
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
        if (tok == NULL || *tok == '\0')
            break;
        len = strlen(tok);
        if (len == 2) {
            t0 = tok[0];
            t1 = tok[1];
            if (t0 == '3' || t0 == '4') {
                if (t1 == '8') {
                    tok = strtok(NULL, ";m");
                    if (tok != NULL) {
                        if (*tok == '5') {
                            tok = strtok(NULL, ";m");
                            if (tok != NULL) {
                                x_idx = a_toi(tok, &a_toi_error);
                                rgb = xterm256_idx_to_rgb(x_idx);
                            }
                        } else if (*tok == '2') {
                            tok = strtok(NULL, ";m");
                            rgb.r = a_toi(tok, &a_toi_error);
                            tok = strtok(NULL, ";m");
                            rgb.g = a_toi(tok, &a_toi_error);
                            tok = strtok(NULL, ";m");
                            rgb.b = a_toi(tok, &a_toi_error);
                        }
                    }
                    if (t0 == '3')
                        fg_clr = rgb_to_curses_clr(&rgb);
                    else if (t0 == '4')
                        bg_clr = rgb_to_curses_clr(&rgb);
                } else if (t1 == '9') {
                    if (t0 == '3')
                        fg_clr = COLOR_WHITE;
                    else if (t0 == '4')
                        bg_clr = COLOR_BLACK;
                } else if (t1 >= '0' && t1 <= '7') {
                    if (t0 == '3') {
                        x_idx = a_toi(&t1, &a_toi_error);
                        rgb = xterm256_idx_to_rgb(x_idx);
                        fg_clr = rgb_to_curses_clr(&rgb);
                    } else if (t0 == '4') {
                        x_idx = a_toi(&t1, &a_toi_error);
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
                fg_clr = COLOR_WHITE;
                bg_clr = COLOR_BLACK;
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
            fg_clr = COLOR_WHITE;
            bg_clr = COLOR_BLACK;
        }
        tok = strtok(NULL, ";m");
    }
    if (!a_toi_error && (fg_clr != fg || bg_clr != bg)) {
        clr_pair_idx = get_clr_pair(fg_clr, bg_clr);
        *cpx = clr_pair_idx;
    }
    return;
}
/** @brief Display Command Line Prompt
    @param view is the current view data structure
    @param s is the prompt string */
void cmd_line_prompt(View *view, char *s) {
    char message_str[PAD_COLS + 1];
    int l;
    l = strnz__cpy(message_str, s, PAD_COLS);
    wmove(view->win, view->cmd_line, view->pmincol);
    if (l != 0) {
        wclrtoeol(view->win);
        wattron(view->win, WA_REVERSE);
        waddstr(view->win, " ");
        waddstr(view->win, message_str);
        waddstr(view->win, " ");
        wattroff(view->win, WA_REVERSE);
        waddstr(view->win, " ");
        wmove(view->win, view->cmd_line, view->pmincol + l + 2);
    }
    wrefresh(view->win);
}
/** @brief Remove File
    @param view is the current view data structure */
void remove_file(View *view) {
    char c;
    if (view->f_at_end_remove) {
        wmove(view->win, view->cmd_line, 0);
        waddstr(view->win, "Remove File (Y or N)->");
        wclrtoeol(view->win);
        c = (char)xwgetch(view->win);
        waddch(view->win, (char)toupper(c));
        if (c == 'Y' || c == 'y')
            remove(view->cur_file_str);
    }
}
/** @brief Display View Help File
    @param init is the current initialization data structure.
    @note The current View context is set aside by assigning the
   view structure to "view_save" while the help file is displayed
   using a new, separate view structure.
    @note The help file is specified by the VIEW_HELP_FILE macro can
   be set to a default help file path or overridden by the user
   through an environment variable.
    @note  After the help file is closed, the original view is
   restored and the page is redisplayed.
    @note It may be necessary to reassign view after calling this
   function because the init->view pointer is temporarily set to
   NULL during the help file display, and the original view is
   restored afterward.
    @note The default screen size for help can be set in the code
   below. If set to 0, mview will determine reasonable maximal size
   based on the terminal dimensions.
   @note The help file may contain Unicode characters and ANSI
   escape sequences for formatting, which will be properly handled
   and displayed by mview. */
void view_display_help(Init *init) {
    int eargc;
    char *eargv[MAXARGS];
    View *view_save = init->view;
    init->view = NULL;
    zero_opt_args(init);
    eargv[0] = strdup("mview");
    strnz__cpy(tmp_str, VIEW_HELP_FILE, MAXLEN - 1);
    expand_tilde(tmp_str, MAXLEN - 1);
    eargv[1] = strdup(tmp_str);
    eargv[2] = NULL;
    eargc = 2;
    parse_opt_args(init, eargc, eargv);
    init->lines = 48;
    init->cols = 60;
    init->begy = 0;
    init->begx = 0;
    strnz__cpy(init->title, "View Help", MAXLEN - 1);
    mview(init, eargc, eargv);
    init->view = view_save;
    init->view->f_redisplay_page = true;
}
/** @brief use form to enter a file specification
    @param init data structure
    @param file_spec - pointer to file specification
    the file_spec
    @returns true if successful
    @note the user must provide a character array large enough to
   hold file_spec without overflowing
 */
bool enter_file_spec(Init *init, char *file_spec) {
    char tmp_dir[MAXLEN];
    char tmp_spec[MAXLEN];
    int rc = false;
    FILE *tmp_fp;
    view = init->view;
    strnz__cpy(tmp_dir, init->mapp_home, MAXLEN - 1);
    strnz__cat(tmp_dir, "/tmp", MAXLEN - 1);
    expand_tilde(tmp_dir, MAXLEN - 1);
    if (!mk_dir(tmp_dir)) {
        strnz__cpy(em0, "Unable to create ", MAXLEN - 1);
        strnz__cat(em0, tmp_dir, MAXLEN - 1);
        return false;
    }
    while (rc == false) {
        strnz__cpy(tmp_spec, tmp_dir, MAXLEN - 1);
        strnz__cat(tmp_spec, "/tmp_XXXXXX", MAXLEN - 1);
        view->in_fd = mkstemp(tmp_spec);
        if (view->in_fd == -1) {
            ssnprintf(em0, MAXLEN - 1, "%s, line: %d", __FILE__, __LINE__ - 2);
            strnz__cpy(em1, "mkstemp ", MAXLEN - 1);
            strnz__cat(em1, tmp_spec, MAXLEN - 1);
            strerror_r(errno, em2, MAXLEN - 1);
            display_error(em0, em1, NULL, NULL);
            return (false);
        }
        /** call form to get file_name
            write the name to a temporary file */
        strnz__cpy(earg_str, "form -d file_name.f -o ", MAXLEN - 1);
        strnz__cat(earg_str, tmp_spec, MAXLEN - 1);
        eargc = str_to_args(eargv, earg_str, MAX_ARGS);
        zero_opt_args(init);
        parse_opt_args(init, eargc, eargv);
        rc = init_form(init, eargc, eargv, view->begy + view->lines - 7, 4);
        if (rc == P_CANCEL || rc == 'q' || rc == 'Q' || rc == KEY_F(9)) {
            destroy_form(init);
            view->f_redisplay_page = true;
            return (false);
        }
        close(view->in_fd);
        /** read file_name from tmp_spec */
        tmp_fp = fopen(tmp_spec, "r");
        if (tmp_fp == NULL) {
            ssnprintf(em0, MAXLEN - 1, "%s, line: %d", __FILE__, __LINE__ - 2);
            strnz__cpy(em1, "fopen ", MAXLEN - 1);
            strnz__cat(em1, tmp_spec, MAXLEN - 1);
            strerror_r(errno, em2, MAXLEN - 1);
            display_error(em0, em1, em2, NULL);
            return (false);
        }
        fgets(tmp_str, MAXLEN - 1, tmp_fp);
        strnz(tmp_str, MAXLEN - 1);
        fclose(tmp_fp);
        unlink(tmp_spec);
        if (!verify_spec_arg(file_spec, tmp_str, "~/menuapp/tmp", ".",
                             S_WCOK | S_QUIET)) {
            ssnprintf(em0, MAXLEN - 1, "Unable to open %s for writing",
                      tmp_str);
            strnz__cpy(em1, "Try again", MAXLEN - 1);
            rc = display_error(em0, em1, NULL, NULL);
            if (rc == 'y' || rc == 'Y')
                continue;
        } else {
            return true;
        }
    }
    return true;
}
