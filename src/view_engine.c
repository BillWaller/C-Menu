/* view_engine.c_
 * Bill Waller
 * billxwaller@gmail.com
 */
#include "menu.h"
#include <ctype.h>
#include <fcntl.h>
#include <regex.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#define MAXCHAR (uchar)0xfa
#define UL_START (uchar)0xfb
#define UL_END (uchar)0xfc
#define BO_START (uchar)0xfd
#define BO_END (uchar)0xfe

#define MO_NORMAL 0                //
#define MO_UL_EXPECT_CHR 1         //            next char
#define MO_UL_GOT_CHR_EXPECT_BS 2  //    char    ^H
#define MO_UL_GOT_BS_EXPECT_CHR 3  //    ^H      next char
#define MO_BO_EXPECT_CHR 4         //            next char
#define MO_BO_GOT_CHR_EXPECT_BS 5  //    char    ^H
#define MO_BO_GOT_BS_EXPECT_SAME 6 //    ^H      same char
#define Ctrl(c) ((c) & 0x1f)

#define get_next_char()                                                        \
    do {                                                                       \
        if (view->file_pos == view->file_size) {                               \
            view->f_eod = true;                                                \
            break;                                                             \
        } else                                                                 \
            view->f_eod = false;                                               \
        c = view->buf[view->file_pos++];                                       \
    } while (c == 0x0d);

#define get_prev_char()                                                        \
    do {                                                                       \
        if (view->file_pos == (long)0) {                                       \
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
int get_cmd_char(View *, long *);
int get_cmd_arg(View *, char *);
void build_prompt(View *, int, char *, double elapsed);
void cat_file(View *);
void lp(char *, char *);

void go_to_mark(View *, int);
void go_to_eof(View *);
int go_to_line(View *, long);
void go_to_percent(View *, int);
void go_to_position(View *, long);
bool search(View *, int, char *, bool);
void next_page(View *);
void prev_page(View *);
void resize_page(Init *);
void redisplay_page(Init *);
void scroll_down_n_lines(View *, int);
void scroll_up_n_lines(View *, int);
long get_next_line(View *, long);
long get_prev_line(View *, long);
long get_pos_next_line(View *, long);
long get_pos_prev_line(View *, long);
void fmt_line(View *);
void display_line(View *);
bool ansi_to_cmplx();
void parse_ansi_str(WINDOW *, char *, attr_t *, int *);
int fmt_cmplx_buf(View *);
void view_display_help(View *);
void cmd_line_prompt(View *, char *);
void remove_file(View *);

char err_msg[MAXLEN];

//  ╭───────────────────────────────────────────────────────────────╮
//  │ VIEW_FILE                                                     │
//  ╰───────────────────────────────────────────────────────────────╯
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
                view->f_new_file = true;
                view->maxcol = 0;
                view->f_forward = true;
                view->page_top_pos = (long)0;
                view->page_bot_pos = (long)0;
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
//  ╭───────────────────────────────────────────────────────────────╮
//  │ VIEW_CMD_PROCESSOR                                            │
//  ╰───────────────────────────────────────────────────────────────╯
int view_cmd_processor(Init *init) {
    int tfd;
    char tmp_str[MAXLEN];
    int c;
    int prev_search_cmd = 0;
    int rc, i;
    int n = 0;
    char *editor_ptr;
    char shell_cmd_spec[MAXLEN];
    struct timespec start, end;
    double elapsed = 0;
    bool f_clock_started = false;
    long n_cmd = 0L;
    view = init->view;
    view->f_timer = false;
    //     if (view->view_cmd[0]) {
    //        view->next_cmd_char = view->view_cmd[0];
    // strnz__cpy(view->cmd_arg, (char *)&view->view->view_cmd_all[1],
    // MAXLEN
    // - 1);
    //  } else
    view->view_cmd[0] = '\0';
    while (1) {
        c = view->next_cmd_char;
        view->next_cmd_char = 0;
        if (!c) {
            if (view->f_redisplay_page)
                redisplay_page(init);
            view->f_redisplay_page = false;
            if (view->f_timer && f_clock_started) {
                clock_gettime(CLOCK_MONOTONIC, &end);
                elapsed = (end.tv_sec - start.tv_sec) +
                          (end.tv_nsec - start.tv_nsec) / 1e9;
                f_clock_started = false;
            }
            build_prompt(view, view->prompt_type, view->prompt_str, elapsed);
            if (view->prompt_str[0] == '\0')
                cmd_line_prompt(view, "");
            else if (view->tmp_prompt_str[0] == '\0')
                cmd_line_prompt(view, view->prompt_str);
            else
                cmd_line_prompt(view, view->tmp_prompt_str);
            if (view->box)
                wrefresh(view->box);
            rc =
                prefresh(view->win, view->pminrow, view->pmincol, view->sminrow,
                         view->smincol, view->smaxrow, view->smaxcol);
            if (rc == ERR)
                Perror("Error refreshing screen");
            c = get_cmd_char(view, &n_cmd);
            if (view->f_timer) {
                clock_gettime(CLOCK_MONOTONIC, &start);
                f_clock_started = true;
            }
            view->tmp_prompt_str[0] = '\0';
            if (c >= '0' && c <= '9') {
                tmp_str[0] = (char)c;
                tmp_str[1] = '\0';
                c = get_cmd_arg(view, tmp_str);
            }
        }
        switch (c) {
        case KEY_ALTLEFT:
            if (n_cmd == 0)
                n_cmd = COLS / 2;
            if (view->pmincol - n < 0)
                view->pmincol = 0;
            else
                view->pmincol -= n_cmd;
            break;
        case KEY_ALTRIGHT:
            if (n_cmd == 0)
                n_cmd = COLS / 2;
            if ((view->pmincol + n_cmd) < view->maxcol)
                view->pmincol += n_cmd;
            break;
        case Ctrl('R'):
        case KEY_RESIZE:
            resize_page(init);
            break;
        case KEY_LEFT:
        case KEY_BACKSPACE:
        case Ctrl('H'):
        case 'h':
        case 'H':
            if (n_cmd <= 0)
                n_cmd = 1;
            if ((view->pmincol - n_cmd) < 0)
                view->pmincol = 0;
            else
                view->pmincol -= n_cmd;
            break;
        case KEY_RIGHT:
        case 'l':
        case 'L':
            if (n_cmd <= 0)
                n_cmd = 1;
            if ((view->pmincol + n_cmd) < view->maxcol)
                view->pmincol += n_cmd;
            break;
        case KEY_UP:
        case 'k':
        case 'K':
        case Ctrl('K'):
            if (n_cmd <= 0)
                n_cmd = 1;
            scroll_up_n_lines(view, n_cmd);
            break;
        case KEY_DOWN:
        case KEY_ENTER:
        case '\n':
        case '\r':
        case ' ':
        case 'j':
        case 'J':
            if (n_cmd <= 0)
                n_cmd = 1;
            for (i = 0; i < n_cmd; i++) {
                scroll_down_n_lines(view, n_cmd);
            }
            break;
        case KEY_PPAGE:
        case 'b':
        case 'B':
        case Ctrl('B'):
            scroll_up_n_lines(view, view->scroll_lines);
            break;
        case KEY_NPAGE:
        case 'f':
        case 'F':
        case Ctrl('F'):
            next_page(view);
            break;
        case KEY_HOME:
        case 'g':
            view->pmincol = 0;
            go_to_line(view, 0L);
            break;
        case KEY_LL:
            go_to_eof(view);
            break;
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
                    strcpy(shell_cmd_spec, view->cmd_arg);
                full_screen_shell(shell_cmd_spec);
                if (!view->f_is_pipe) {
                    view->next_file_spec_ptr = view->cur_file_str;
                    return 0;
                }
            }
            break;
        case '+':
            if (get_cmd_arg(view, "Startup Command:") == 0)
                strnz__cpy(view->view_cmd, view->cmd_arg, MAXLEN - 1);
            break;
        case '-':
            if (view->f_displaying_help)
                break;
            cmd_line_prompt(view, "(C, I, P, S, T, or H for Help)->");
            c = get_cmd_char(view, &n_cmd);
            c = tolower(c);
            switch (c) {
            case 'c':
                cmd_line_prompt(view, "Clear Screen at End (Y or N)->");
                if ((c = get_cmd_char(view, &n_cmd)) == 'y' || c == 'Y')
                    view->f_at_end_clear = true;
                else if (c == 'n' || c == 'N')
                    view->f_at_end_clear = false;
                break;
            case 'i':
                cmd_line_prompt(view, "Ignore Case in search (Y or N)->");
                if ((c = get_cmd_char(view, &n_cmd)) == 'y' || c == 'Y')
                    view->f_ignore_case = true;
                else if (c == 'n' || c == 'N')
                    view->f_ignore_case = false;
                break;
            case 'p':
                cmd_line_prompt(view, "(Short Long or No prompt)->");
                c = tolower(get_cmd_char(view, &n_cmd));
                switch (c) {
                case 's':
                    view->prompt_type = PT_SHORT;
                    break;
                case 'l':
                    view->prompt_type = PT_LONG;
                    break;
                case 'n':
                    view->prompt_type = PT_NONE;
                    break;
                default:
                    break;
                }
                break;
            case 's':
                cmd_line_prompt(
                    view, "view->f_squeeze Multiple Blank lines (Y or N)->");
                if ((c = get_cmd_char(view, &n_cmd)) == 'y' || c == 'Y')
                    view->f_squeeze = true;
                else if (c == 'n' || c == 'N')
                    view->f_squeeze = false;
                break;
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
            case 'h':
                if (!view->f_displaying_help)
                    view_display_help(view);
                view->next_cmd_char = '-';
                break;
            default:
                break;
            }
            break;
        case ':':
            view->next_cmd_char = get_cmd_arg(view, ":");
            break;
        case '/':
        case '?':
            strcpy(tmp_str, (c == '/') ? "(forward)->" : "(backward)->");
            if (get_cmd_arg(view, tmp_str) == 0) {
                view->f_wrap = false;
                search(view, c, view->cmd_arg, false);
                prev_search_cmd = c;
                strnz__cpy(prev_regex_pattern, view->cmd_arg, MAXLEN - 1);
            }
            view->srch_beg_pos = view->page_top_pos;
            break;
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
        case KEY_END:
        case 'G':
            if (n_cmd <= 0)
                go_to_eof(view);
            else
                go_to_line(view, n_cmd);
            break;
        case KEY_F(1):
            if (!view->f_displaying_help)
                view_display_help(view);
            break;
        case 'm':
            cmd_line_prompt(view, "Mark label (A-Z)->");
            c = get_cmd_char(view, &n_cmd);
            if (c == '@' || c == KEY_F(9) || c == '\033')
                break;
            c = tolower(c);
            if (c < 'a' || c > 'z')
                Perror("Not (A-Z)");
            else
                view->mark_tbl[c - 'a'] = view->page_top_pos;
            break;
        case 'M':
        case '\'':
            cmd_line_prompt(view, "Goto mark (A-Z)->");
            c = get_cmd_char(view, &n_cmd);
            if (c == '@' || c == KEY_F(9) || c == '\033')
                break;
            c = tolower(c);
            if (c < 'a' || c > 'z')
                Perror("Not (A-Z)");
            else
                go_to_mark(view, c);
            break;
        case 'n':
            if (prev_search_cmd == 0) {
                Perror("No previous search");
                break;
            }
            search(view, prev_search_cmd, prev_regex_pattern, true);
            break;
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
        case 'p':
        case '%':
            if (n_cmd < 0)
                go_to_line(view, 1);
            if (n_cmd >= 100)
                go_to_eof(view);
            else
                go_to_percent(view, n_cmd);
            break;
        case Ctrl('Z'):
            if (view->f_is_pipe) {
                Perror("Can't print standard input");
                break;
            }
            get_cmd_arg(view, "Enter Notation:");
            strnz__cpy(tmp_str, "/tmp/view-XXXXXX", MAXLEN - 1);
            tfd = mkstemp(tmp_str);
            strcpy(view->tmp_file_name_ptr, tmp_str);
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
            lp(view->cur_file_str, view->cmd_arg);
            wrefresh(view->win);
            shell(shell_cmd_spec);
            snprintf(shell_cmd_spec, (size_t)(MAXLEN - 5), "rm %s",
                     view->tmp_file_name_ptr);
            strnz__cpy(shell_cmd_spec, "rm ", MAXLEN - 5);
            strnz__cat(shell_cmd_spec, view->tmp_file_name_ptr, MAXLEN - 5);
            shell(shell_cmd_spec);
            restore_wins();
            view->f_redisplay_page = true;
            unlink(tmp_str);
            break;
        case Ctrl('P'):
        case KEY_CATAB:
        case KEY_PRINT:
            lp(view->cur_file_str, NULL);
            view->f_redisplay_page = true;
            break;
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
        case 'q':
        case 'Q':
        case KEY_F(9):
        case '\033':
            view->curr_argc = view->argc;
            view->next_file_spec_ptr = NULL;
            return 0;
        case 'v':
            if (view->f_displaying_help)
                break;
            if (view->f_is_pipe) {
                Perror("Can't edit standard input");
                break;
            }
            editor_ptr = getenv("DEFAULTEDITOR");
            if (editor_ptr == NULL || *editor_ptr == '\0')
                editor_ptr = DEFAULTEDITOR;
            if (editor_ptr == NULL || *editor_ptr == '\0') {
                Perror("set DEFAULTEDITOR environment variable");
                break;
            }
            view->prev_file_pos = view->page_top_pos;
            munmap(view->buf, view->file_size);
            view->next_file_spec_ptr = view->file_spec_ptr;
            strnz__cpy(shell_cmd_spec, editor_ptr, MAXLEN - 5);
            strnz__cat(shell_cmd_spec, " ", MAXLEN - 5);
            strnz__cat(shell_cmd_spec, view->cur_file_str, MAXLEN - 5);
            full_screen_shell(shell_cmd_spec);
            return 0;
        case 'V':
            Perror("View: Version 8.0");
            break;
        default:
            break;
        }
        view->cmd_arg[0] = '\0';
    }
}
//  ╭───────────────────────────────────────────────────────────────╮
//  │ GET_CMD_CHAR                                                  │
//  ╰───────────────────────────────────────────────────────────────╯
int get_cmd_char(View *view, long *n) {
    int c = 0, i = 0;
    char cmd_str[33];
    cmd_str[0] = '\0';
    MEVENT event;
    mousemask(BUTTON4_PRESSED | BUTTON5_PRESSED, NULL);
    tcflush(2, TCIFLUSH);
    do {
        c = wgetch(view->win);
        if (c == KEY_MOUSE) {
            if (getmouse(&event) != OK)
                return (MA_ENTER_OPTION);
            if (event.bstate & BUTTON4_PRESSED)
                return (KEY_UP);
            else if (event.bstate & BUTTON5_PRESSED) {
                return (KEY_DOWN);
            }
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
//  ╭───────────────────────────────────────────────────────────────╮
//  │ GET_CMD_SPEC                                                  │
//  ╰───────────────────────────────────────────────────────────────╯
int get_cmd_arg(View *view, char *prompt) {
    int c;
    int numeric_arg = false;
    char *cmd_p;
    char *cmd_e;
    char prompt_s[MAX_COLS + 1];
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
        if (rc == ERR) {
            Perror("Error refreshing screen");
        }
        c = wgetch(view->win);
        switch (c) {
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
        case KEY_ENTER:
        case '\n':
        case '\r':
            return 0;
        case '@':
        case KEY_F(9):
        case '\033':
            return 0;
        case KEY_MOUSE:
            return 0;
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
                return (c);
            break;
        }
    }
}
//  ╭───────────────────────────────────────────────────────────────╮
//  │ BUILD_PROMPT                                                  │
//  ╰───────────────────────────────────────────────────────────────╯
void build_prompt(View *view, int prompt_type, char *prompt_str,
                  double elapsed) {
    prompt_str[0] = '\0';
    strnz__cpy(prompt_str, "", MAXLEN - 1);
    if (prompt_type == PT_LONG || view->f_new_file) {
        if (view->f_is_pipe)
            strnz__cat(prompt_str, "stdin", MAXLEN - 1);
        else
            strnz__cat(prompt_str, view->cur_file_str, MAXLEN - 1);
    }
    if (view->pmincol > 0) {
        sprintf(tmp_str, "Col %d", view->pmincol);
        if (prompt_str[0] != '\0')
            strnz__cat(prompt_str, " ", MAXLEN - 1);
        strnz__cat(prompt_str, tmp_str, MAXLEN - 1);
    }
    if (view->argc > 1 && (view->f_new_file || prompt_type == PT_LONG)) {
        sprintf(tmp_str, "File %d of %d", view->curr_argc + 1, view->argc);
        if (prompt_str[0] != '\0') {
            strnz__cat(prompt_str, " ", MAXLEN - 1);
            strnz__cat(prompt_str, tmp_str, MAXLEN - 1); /* File Of      */
        }
    }
    if (prompt_type == PT_LONG) { /* Byte of Byte  */
        if (view->page_top_pos == NULL_POSITION)
            view->page_top_pos = view->file_size;
        sprintf(tmp_str, "Pos %ld-%ld", view->page_top_pos, view->page_bot_pos);
        if (prompt_str[0] != '\0') {
            strnz__cat(prompt_str, " ", MAXLEN - 1);
            strnz__cat(prompt_str, tmp_str, MAXLEN - 1);
        }
        if (!view->f_is_pipe) {
            if (view->file_size > 0) {
                sprintf(tmp_str, " of %ld", view->file_size);
                strnz__cat(prompt_str, tmp_str, MAXLEN - 1);
            }
        }
    }
    if (!view->f_eod && prompt_type != PT_NONE) { /* Percent       */
        if (view->file_size > 0L && view->page_bot_pos != NULL_POSITION) {
            sprintf(tmp_str, "(%ld%%)",
                    (100L * view->page_bot_pos) / view->file_size);
            if (prompt_str[0] != '\0')
                strnz__cat(prompt_str, " ", MAXLEN - 1);
            strnz__cat(prompt_str, tmp_str, MAXLEN - 1);
        }
    }
    if (view->f_eod) { /* End           */
        if (prompt_str[0] != '\0')
            strnz__cat(prompt_str, " ", MAXLEN - 1);
        strnz__cat(prompt_str, "(End)", MAXLEN - 1);
        if (view->curr_argc + 1 < view->argc) {
            sprintf(tmp_str, " Next File: %s", view->argv[view->curr_argc + 1]);
            strnz__cat(prompt_str, tmp_str, MAXLEN - 1);
        }
    }
    if (view->f_timer) {
        sprintf(tmp_str, " secs. %.6f\n", elapsed);
        strnz__cat(prompt_str, tmp_str, MAXLEN - 1);
    }
    view->f_new_file = false;
}
//  ╭───────────────────────────────────────────────────────────────╮
//  │ CAT_FILE                                                      │
//  ╰───────────────────────────────────────────────────────────────╯
void cat_file(View *view) {
    int c;
    while (1) {
        get_next_char();
        if (view->f_eod)
            break;
        putchar(c);
    }
}
//  ╭───────────────────────────────────────────────────────────────╮
//  │ LP (PRINT)                                                    │
//  ╰───────────────────────────────────────────────────────────────╯
void lp(char *PrintFile, char *Notation) {
    char *print_cmd_ptr;
    char shell_cmd_spec[MAXLEN];
    print_cmd_ptr = getenv("PRINTCMD");
    if (print_cmd_ptr == NULL || *print_cmd_ptr == '\0')
        print_cmd_ptr = PRINTCMD;
    sprintf(shell_cmd_spec, "%s %s", print_cmd_ptr, PrintFile);
    cmd_line_prompt(view, shell_cmd_spec);
    wrefresh(view->win);
    shell(shell_cmd_spec);
}
//  ╭───────────────────────────────────────────────────────────────╮
//  │ GO_TO_MARK                                                    │
//  ╰───────────────────────────────────────────────────────────────╯
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
//  ╭───────────────────────────────────────────────────────────────╮
//  │ GO_TO_EOF                                                     │
//  ╰───────────────────────────────────────────────────────────────╯
void go_to_eof(View *view) {
    int c;
    if (view->f_is_pipe) {
        view->f_forward = true;
        get_next_char();
        while (!view->f_eod)
            get_next_char();
        if (view->f_eod)
            view->file_pos--;
    } else
        view->file_pos = view->file_size;
    view->page_top_pos = view->file_pos;
    get_prev_char();
    prev_page(view);
}
//  ╭───────────────────────────────────────────────────────────────╮
//  │ GO_TO_LINE                                                    │
//  ╰───────────────────────────────────────────────────────────────╯
int go_to_line(View *view, long line_idx) {
    int c = 0;
    long line_cnt = 0;
    if (line_idx <= 1) {
        go_to_position(view, (long)0);
        return EOF;
    }
    view->f_forward = true;
    view->file_pos = (long)0;
    view->page_top_pos = view->file_pos;
    line_idx = 0;
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
//  ╭───────────────────────────────────────────────────────────────╮
//  │ GO_TO_PERCENT                                                 │
//  ╰───────────────────────────────────────────────────────────────╯
void go_to_percent(View *view, int Percent) {
    int c;
    if (view->file_size < 0) {
        Perror("Cannot determine file length");
        return;
    }
    view->file_pos = ((long)Percent * view->file_size) / 100L;
    view->f_forward = true;
    get_next_char();
    while (c != '\n') {
        get_prev_char();
        if (view->f_bod)
            break;
    }
    get_next_char();
}
//  ╭───────────────────────────────────────────────────────────────╮
//  │ GO_TO_POSITION                                                │
//  ╰───────────────────────────────────────────────────────────────╯
void go_to_position(View *view, long go_to_pos) {
    view->f_forward = true;
    view->file_pos = go_to_pos;
    view->page_bot_pos = view->file_pos;
    next_page(view);
}
//  ╭───────────────────────────────────────────────────────────────╮
//  │ SEARCH                                                        │
//  ╰───────────────────────────────────────────────────────────────╯
bool search(View *view, int search_cmd, char *regex_pattern, bool repeat) {
    int REG_FLAGS = 0;
    regmatch_t pmatch[1];
    regex_t compiled_regex;
    int reti;
    int header_lines = 0;
    int line_offset, line_len, match_len;
    int cury = 0;
    long srch_curr_pos;
    bool f_page = false;
    int i;
    int rc;
    srch_curr_pos = view->page_top_pos;
    if (repeat) {
        if (search_cmd == '/')
            srch_curr_pos = view->page_bot_pos;
    }
    //  ╭───────────────────────────────────────────────────────────────╮
    //  │ SEARCH - COMPILE REGULAR EXPRESSION                           │
    //  ╰───────────────────────────────────────────────────────────────╯
    if (view->f_ignore_case)
        REG_FLAGS = REG_ICASE | REG_EXTENDED;
    else
        REG_FLAGS = REG_EXTENDED;
    reti = regcomp(&compiled_regex, regex_pattern, REG_FLAGS);
    if (reti) {
        Perror("Invalid pattern");
        return false;
    }
    //  ╭───────────────────────────────────────────────────────────────╮
    //  │ SEARCH - READ LINES                                           │
    //  ╰───────────────────────────────────────────────────────────────╯
    while (1) {
#ifdef DEBUG
        snprintf(tmp_str, MAXLEN - 1, "Pos %ld of %ld, %s for: ", srch_curr_pos,
                 view->file_size, (search_cmd == '/') ? "Forward" : "Backward");
        strnz__cat(tmp_str, regex_pattern, MAXLEN - strlen(tmp_str) - 1);
        cmd_line_prompt(view, tmp_str);
        rc = prefresh(view->win, view->pminrow, view->pmincol, view->sminrow,
                      view->smincol, view->smaxrow, view->smaxcol);
        if (rc == ERR) {
            Perror("Error refreshing screen");
        }
#endif
        if (search_cmd == '/') {
            if (srch_curr_pos == view->file_size &&
                view->srch_beg_pos == (long)0)
                srch_curr_pos = (long)0;
            if (srch_curr_pos == view->srch_beg_pos) {
                if (!view->f_wrap) {
                    view->f_wrap = true;
                } else {
                    snprintf(tmp_str, MAXLEN - 1,
                             "Search complete: %ld bytes for %s",
                             view->file_size, regex_pattern);
                    cmd_line_prompt(view, tmp_str);
                    regfree(&compiled_regex);
                    return false;
                }
            }
            if (cury == view->scroll_lines)
                return true;
            srch_curr_pos = get_next_line(view, srch_curr_pos);
        } else {
            if (srch_curr_pos == (long)0 &&
                view->srch_beg_pos == view->file_size)
                srch_curr_pos = view->file_size;
            if (srch_curr_pos == view->srch_beg_pos) {
                if (!view->f_wrap) {
                    view->f_wrap = true;
                } else {
                    snprintf(tmp_str, MAXLEN - 1,
                             "Search complete: %ld bytes for %s",
                             view->file_size, regex_pattern);
                    cmd_line_prompt(view, tmp_str);
                    regfree(&compiled_regex);
                    return false;
                }
            }
            srch_curr_pos = get_prev_line(view, srch_curr_pos);
        }
        fmt_line(view);
        //  ╭───────────────────────────────────────────────────────────╮
        //  │ SEARCH - CURRENT LINE                                     │
        //  ╰───────────────────────────────────────────────────────────╯
        reti = regexec(&compiled_regex, view->stripped_line_out,
                       compiled_regex.re_nsub + 1, pmatch, REG_FLAGS);
        //  ╭───────────────────────────────────────────────────────────╮
        //  │ SEARCH - NO MATCH - DISPLAY LINE IF PAGING                │
        //  ╰───────────────────────────────────────────────────────────╯
        if (reti == REG_NOMATCH) {
            if (f_page) {
                display_line(view);
                if (view->cury == view->scroll_lines)
                    break;
            }
            continue;
        }
        //  ╭───────────────────────────────────────────────────────────╮
        //  │ SEARCH - ERROR                                            │
        //  ╰───────────────────────────────────────────────────────────╯
        if (reti) {
            char err_str[MAXLEN];
            regerror(reti, &compiled_regex, err_str, sizeof(err_str));
            strcpy(tmp_str, "Regex match failed: ");
            strcat(tmp_str, err_str);
            Perror(tmp_str);
            regfree(&compiled_regex);
            return false;
        }
        //  ╭───────────────────────────────────────────────────────────╮
        //  │ SEARCH - MATCH - DISPLAY LEADING CONTEXT LINES            │
        //  ╰───────────────────────────────────────────────────────────╯
        if (!f_page) {
            view->f_forward = true;
            view->cury = 0;
            wmove(view->win, view->cury, 0);
            wclrtobot(view->win);
            for (i = 0; i < 4; i++) {
                if (srch_curr_pos == 0)
                    break;
                srch_curr_pos = get_pos_prev_line(view, srch_curr_pos);
            }
            view->page_top_pos = srch_curr_pos;
            header_lines = i;
            if (header_lines < 1)
                header_lines = 1;
            for (i = 0; i < header_lines; i++) {
                srch_curr_pos = get_next_line(view, srch_curr_pos);
                fmt_line(view);
                display_line(view);
            }
            f_page = true;
        } else {
            //  ╭───────────────────────────────────────────────────────╮
            //  │ SEARCH - CONTINUE HIGHLIGHTING MATCHED LINES          │
            //  ╰───────────────────────────────────────────────────────╯
            display_line(view);
            cury = view->cury;
        }
        //  ╭───────────────────────────────────────────────────────╮
        //  │ SEARCH - HIGHLIGHT ALL MATCHES ON CURRENT LINE        │
        //  ╰───────────────────────────────────────────────────────╯
        view->first_match_x = -1;
        view->last_match_x = 0;
        line_len = strlen(view->stripped_line_out);
        line_offset = 0;
        while (1) {
            //  ╭───────────────────────────────────────────────────╮
            //  │ SEARCH - HIGHLIGHT MATCH                          │
            //  ╰───────────────────────────────────────────────────╯
            view->curx = line_offset + pmatch[0].rm_so;
            match_len = pmatch[0].rm_eo - pmatch[0].rm_so;
            mvwchgat(view->win, view->cury - 1, view->curx, match_len,
                     A_REVERSE, cp_norm, NULL);
            rc =
                prefresh(view->win, view->pminrow, view->pmincol, view->sminrow,
                         view->smincol, view->smaxrow, view->smaxcol);
            if (rc == ERR) {
                Perror("Error refreshing screen");
            }
            //  ╭───────────────────────────────────────────────────╮
            //  │ SEARCH - UPDATE LINE MATCH COLUMNS                │
            //  ╰───────────────────────────────────────────────────╯
            if (view->first_match_x == -1)
                view->first_match_x = pmatch[0].rm_so;
            view->last_match_x = line_offset + pmatch[0].rm_eo;
            //  ╭───────────────────────────────────────────────────╮
            //  │ SEARCH - TRACK LINE_OFFSET                        │
            //  ╰───────────────────────────────────────────────────╯
            line_offset += pmatch[0].rm_eo;
            if (line_offset >= line_len)
                break;
            view->line_out_p = view->stripped_line_out + line_offset;
            //  ╭───────────────────────────────────────────────────╮
            //  │ SEARCH - CONTINUE HIGHLIGHTING SAME LINE          │
            //  ╰───────────────────────────────────────────────────╯
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
            if (view->cury == view->scroll_lines) {
                regfree(&compiled_regex);
                return true;
            }
        }
    }
    //  ╭───────────────────────────────────────────────────────────╮
    //  │ SEARCH - SUCCESS - PREPARE PROMPT                         │
    //  ╰───────────────────────────────────────────────────────────╯
    view->file_pos = srch_curr_pos;
    view->page_bot_pos = srch_curr_pos;
    if (view->last_match_x > view->maxcol)
        snprintf(view->tmp_prompt_str, MAXLEN - 1,
                 "%c%s Match Cols %d-%d of %d-%d (%d%%)", search_cmd,
                 regex_pattern, view->first_match_x, view->last_match_x,
                 view->pmincol, view->smaxcol - view->begx,
                 (int)(view->page_bot_pos * 100 / view->file_size));
    else
        snprintf(view->tmp_prompt_str, MAXLEN - 1, "%c%s lines %ld-%ld (%d%%)",
                 search_cmd, regex_pattern, view->page_top_pos,
                 view->page_bot_pos,
                 (int)(view->page_bot_pos * 100 / view->file_size));
    regfree(&compiled_regex);
    return true;
}
//  ╭───────────────────────────────────────────────────────────────╮
//  │ REDISPLAY_PAGE                                                │
//  ╰───────────────────────────────────────────────────────────────╯
void resize_page(Init *init) {
    int scr_lines, scr_cols;
    bool f_resize;
    view = init->view;
    if (view->f_full_screen) {
        // clear();
        // refresh();
        // delwin(view->win);
        // touchwin(stdscr);
        // wnoutrefresh(stdscr);
        // init_view_full_screen(init);
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
//  ╭───────────────────────────────────────────────────────────────╮
//  │ REDISPLAY_PAGE                                                │
//  ╰───────────────────────────────────────────────────────────────╯
void redisplay_page(Init *init) {
    int i;
    view->cury = 0;
    wmove(view->win, view->cury, 0);
    view->page_bot_pos = view->page_top_pos;
    for (i = 0; i < view->scroll_lines; i++) {
        view->page_bot_pos = get_next_line(view, view->page_bot_pos);
        if (view->f_eod)
            break;
        fmt_line(view);
        display_line(view);
    }
}
//  ╭───────────────────────────────────────────────────────────────╮
//  │ NEXT_PAGE                                                     │
//  ╰───────────────────────────────────────────────────────────────╯
void next_page(View *view) {
    int i;
    view->maxcol = 0;
    view->f_forward = true;
    view->cury = 0;
    wmove(view->win, view->cury, 0);
    view->file_pos = view->page_bot_pos;
    view->page_top_pos = view->file_pos;
    wclrtobot(view->win);
    for (i = 0; i < view->scroll_lines; i++) {
        get_next_line(view, view->file_pos);
        if (view->f_eod)
            break;
        fmt_line(view);
        display_line(view);
    }
    view->page_bot_pos = view->file_pos;
}
//  ╭───────────────────────────────────────────────────────────────╮
//  │ PREV_PAGE                                                     │
//  ╰───────────────────────────────────────────────────────────────╯
void prev_page(View *view) {
    int i;
    if (view->page_top_pos == (long)0)
        return;
    view->maxcol = 0;
    view->f_forward = false;
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
//  ╭───────────────────────────────────────────────────────────────╮
//  │ SCROLL_FORWARD_N_LINES                                        │
//  ╰───────────────────────────────────────────────────────────────╯
void scroll_down_n_lines(View *view, int n) {
    int i = 0;
    if (view->page_bot_pos == view->file_size)
        return;
    view->f_forward = true;
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
        fmt_line(view);
        display_line(view);
    }
    view->page_bot_pos = view->file_pos;
}
//  ╭───────────────────────────────────────────────────────────────╮
//  │ SCROLL_BACK_N_LINES                                           │
//  ╰───────────────────────────────────────────────────────────────╯
void scroll_up_n_lines(View *view, int n) {
    int i;
    if (view->page_top_pos == (long)0)
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
        fmt_line(view);
        display_line(view);
    }
    return;
}
//  ╭───────────────────────────────────────────────────────────────╮
//  │ GET_NEXT_LINE -> line_in_s                                    │
//  ╰───────────────────────────────────────────────────────────────╯
long get_next_line(View *view, long pos) {
    uchar c;
    char *line_in_p;
    view->file_pos = pos;
    view->f_forward = true;
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
    view->line_in_end_p = view->line_in_s + LINE_IN_MAX_COLS;
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
//  ╭───────────────────────────────────────────────────────────────╮
//  │ GET_PREV_LINE                                                 │
//  ╰───────────────────────────────────────────────────────────────╯
long get_prev_line(View *view, long pos) {
    uchar c;
    view->file_pos = pos;
    view->f_forward = false;
    get_prev_char();
    if (view->f_bod)
        return view->file_pos;
    while ((uchar)c != '\n')
        get_prev_char();
    if (view->f_bod)
        return view->file_pos;
    if (view->f_squeeze) {
        if ((uchar)c == '\n') {
            while (1) {
                get_prev_char();
                if (view->f_bod)
                    return view->file_pos;
                if ((uchar)c != '\n')
                    break;
            }
            get_next_char();
        }
    }
    while (1) {
        if ((uchar)c == '\n')
            break;
        get_prev_char();
        if (view->f_bod)
            break;
    }
    if (view->file_pos < view->file_size)
        view->f_eod = false;
    return view->file_pos;
}
//  ╭───────────────────────────────────────────────────────────────╮
//  │ GET_POS_NEXT_LINE                                             │
//  ╰───────────────────────────────────────────────────────────────╯
long get_pos_next_line(View *view, long pos) {
    uchar c;
    if (pos == view->file_size) {
        view->f_eod = true;
        return view->file_pos;
    }
    view->file_pos = pos;
    view->f_forward = true;
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
//  ╭───────────────────────────────────────────────────────────────╮
//  │ GET_POS_PREV_LINE                                             │
//  ╰───────────────────────────────────────────────────────────────╯
long get_pos_prev_line(View *view, long pos) {
    uchar c;
    view->file_pos = pos;
    if (view->file_pos == (long)0) {
        view->f_bod = true;
        return view->file_pos;
    }
    view->f_forward = false;
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
//  ╭───────────────────────────────────────────────────────────╮
//  │ DISPLAY_LINE   view->cmplx_buf -> view->win               │
//  ╰───────────────────────────────────────────────────────────╯
void display_line(View *view) {
    int rc;
    if (view->cury < 0)
        view->cury = 0;
    if (view->cury > view->scroll_lines)
        view->cury = view->scroll_lines;
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
//  ╭───────────────────────────────────────────────────────────────╮
//  │ FMT_LINE_OUT      line_out_s -> view->cmplx_buf               │
//  │                                 view->stripped_line_out       │
//  ╰───────────────────────────────────────────────────────────────╯
void fmt_line(View *view) {
    attr_t attr = A_NORMAL;
    char ansi_tok[64];
    int cp = cp_default;
    int i = 0, j = 0;
    size_t len;
    const char *s;
    wchar_t wc;
    cchar_t cc;
    char *in_str = view->line_in_s;
    cchar_t *cmplx_buf = view->cmplx_buf;
    rtrim(view->line_out_s);
    mbtowc(NULL, NULL, 0);
    while (in_str[i] != '\0') {
        if (in_str[i] == '\033' && in_str[i + 1] == '[') {
            char *start_seq = (char *)&in_str[i];
            char *end_seq = strchr(start_seq, 'm');
            if (end_seq) {
                strnz__cpy(ansi_tok, start_seq, end_seq - start_seq + 1);
                ansi_tok[end_seq - start_seq + 1] = '\0';
                parse_ansi_str(stdscr, ansi_tok, &attr, &cp);
                i = (end_seq - in_str) + 1;
            } else {
                i++;
            }
        } else {
            s = &in_str[i];
            if (*s == '\t') {
                wc = L' ';
                setcchar(&cc, &wc, attr, cp, NULL);
                while ((j < MAX_COLS - 2) && (j % view->tab_stop != 0)) {
                    view->stripped_line_out[j] = ' ';
                    cmplx_buf[j++] = cc;
                }
                i++;
            }
            len = mbtowc(&wc, s, MB_CUR_MAX);
            if (setcchar(&cc, &wc, attr, cp, NULL) != ERR) {
                if (len > 0 && (j + len) < MAX_COLS - 1) {
                    view->stripped_line_out[j] = *s;
                    cmplx_buf[j++] = cc;
                    i += len;
                }
            }
        }
    }
    if (j > view->maxcol)
        view->maxcol = j;
    wc = L'\0';
    setcchar(&cc, &wc, A_NORMAL, cp_default, NULL);
    cmplx_buf[j] = cc;
    view->stripped_line_out[j] = '\0';
    return;
}
//  ╭───────────────────────────────────────────────────────────────╮
//  │ PARSE_ANSI_STR                                                │
//  ╰───────────────────────────────────────────────────────────────╯
void parse_ansi_str(WINDOW *win, char *ansi_str, attr_t *attr, int *cp) {
    char *tok;
    int i, len, x_idx;
    bool f_fg = false, f_bg = false;
    int fg, bg;
    char *ansi_p = ansi_str + 2;
    extended_pair_content(*cp, &fg, &bg);
    RGB rgb;
    for (i = 1; i < 5; i++) {
        tok = strtok((char *)ansi_p, ";m");
        if (tok == NULL)
            break;
        ansi_p = NULL;
        len = strlen(tok);
        if (len == 2) {
            if (tok[0] == '3' && tok[1] == '8') {
                //  ╭───────────────────────────────────────────────╮
                //  │ 38 - FOREGROUND                               │
                //  ╰───────────────────────────────────────────────╯
                tok = strtok((char *)ansi_p, ";m");
                ansi_p = 0;
                if (tok != NULL && strcmp(tok, "5") == 0) {
                    //  ╭───────────────────────────────────────────╮
                    //  │ 5 - 256 CLR PALETTE                       │
                    //  ╰───────────────────────────────────────────╯
                    tok = strtok((char *)ansi_p, ";m");
                    ansi_p = NULL;
                    if (tok != NULL) {
                        //  ╭───────────────────────────────────────╮
                        //  │ XTERM_IDX_TO_RGB                      │
                        //  ╰───────────────────────────────────────╯
                        x_idx = atoi(tok);
                        if (fg != x_idx) {
                            fg = x_idx;
                            f_fg = true;
                        }
                    }
                } else {
                    if (tok != NULL && strcmp(tok, "2") == 0) {
                        //  ╭───────────────────────────────────────────╮
                        //  │ 2 - RGB                                   │
                        //  ╰───────────────────────────────────────────╯
                        tok = strtok((char *)ansi_p, ";m");
                        ansi_p = NULL;
                        rgb.r = atoi(tok);
                        tok = strtok((char *)ansi_p, ";m");
                        ansi_p = NULL;
                        rgb.g = atoi(tok);
                        tok = strtok((char *)ansi_p, ";m");
                        ansi_p = NULL;
                        rgb.b = atoi(tok);
                        x_idx = rgb_to_xterm256_idx(rgb);
                        if (fg != x_idx) {
                            fg = x_idx;
                            f_fg = true;
                        }
                    }
                }
            } else if (tok[0] == '4' && tok[1] == '8') {
                //  ╭───────────────────────────────────────────────╮
                //  │ 48 - BACKGROUND                               │
                //  ╰───────────────────────────────────────────────╯
                tok = strtok((char *)ansi_p, ";m");
                ansi_p = NULL;
                if (tok != NULL && strcmp(tok, "5") == 0) {
                    //  ╭───────────────────────────────────────────╮
                    //  │ 5 - 256 CLR PALETTE                       │
                    //  ╰───────────────────────────────────────────╯
                    tok = strtok((char *)ansi_p, ";m");
                    ansi_p = NULL;
                    if (tok != NULL) {
                        //  ╭───────────────────────────────────────╮
                        //  │ XTERM_IDX_TO_RGB                      │
                        //  ╰───────────────────────────────────────╯
                        int x_idx = atoi(tok);
                        if (bg != x_idx) {
                            bg = x_idx;
                            f_bg = true;
                        }
                    }
                } else {
                    if (tok != NULL && strcmp(tok, "2") == 0) {
                        //  ╭───────────────────────────────────────────╮
                        //  │ 2 - RGB COLOR                             │
                        //  ╰───────────────────────────────────────────╯
                        tok = strtok((char *)ansi_p, ";m");
                        ansi_p = NULL;
                        rgb.r = atoi(tok);
                        tok = strtok((char *)ansi_p, ";m");
                        ansi_p = NULL;
                        rgb.g = atoi(tok);
                        tok = strtok((char *)ansi_p, ";m");
                        ansi_p = NULL;
                        rgb.b = atoi(tok);
                        x_idx = rgb_to_xterm256_idx(rgb);
                        if (bg != x_idx) {
                            bg = x_idx;
                            f_bg = true;
                        }
                    }
                }
            } else if (tok[0] == '3' && tok[1] == '9') {
                if (fg != COLOR_WHITE) {
                    fg = COLOR_WHITE;
                    f_fg = true;
                }
            } else if (tok[0] == '4' && tok[1] == '9') {
                if (bg != COLOR_BLACK) {
                    bg = COLOR_BLACK;
                    f_bg = true;
                }
            } else if ((tok[0] == '3' || tok[0] == '4') && tok[1] >= '0' &&
                       tok[1] <= '7') {
                if (tok[0] == '3') {
                    i = atoi(&tok[1]);
                    if (fg != i) {
                        fg = i;
                        f_fg = true;
                    }
                } else if (tok[0] == '4') {
                    i = atoi(&tok[1]);
                    if (bg != i) {
                        bg = i;
                        f_bg = true;
                    }
                }
            } else if (tok[0] == '0' && tok[1] == '1') {
                *attr = A_NORMAL;
                if (fg != COLOR_WHITE) {
                    fg = COLOR_WHITE;
                    f_fg = true;
                }
                if (bg != COLOR_BLACK) {
                    bg = COLOR_BLACK;
                    f_fg = true;
                }
            } else if (tok[0] == '0' && tok[1] == '1')
                *attr = A_BOLD;
            else if (tok[0] == '0' && tok[1] == '2')
                *attr = A_REVERSE;
            else if (tok[0] == '0' && tok[1] == '3')
                *attr = A_ITALIC;
        } else if (len == 1) {
            if (tok[0] == '0') {
                *attr = A_NORMAL;
                if (fg != COLOR_WHITE) {
                    fg = COLOR_WHITE;
                    f_fg = true;
                }
                if (bg != COLOR_BLACK) {
                    bg = COLOR_BLACK;
                    f_fg = true;
                }
            } else if (tok[0] == '1')
                *attr = A_BOLD;
            else if (tok[0] == '2')
                *attr = A_REVERSE;
            else if (tok[0] == '3')
                *attr = A_ITALIC;
        } else if (len == 0) {
            *attr = A_NORMAL;
            if (fg != COLOR_WHITE) {
                fg = COLOR_WHITE;
                f_fg = true;
            }
            if (bg != COLOR_BLACK) {
                bg = COLOR_BLACK;
                f_fg = true;
            }
        }
    }
    if (f_fg == true || f_bg == true) {
        rgb = xterm256_idx_to_rgb(fg);
        apply_gamma(&rgb);
        fg = rgb_to_xterm256_idx(rgb);
        clr_pair_idx = get_clr_pair(fg, bg);
        *cp = clr_pair_idx;
    }
    return;
}
//  ╭───────────────────────────────────────────────────────────────╮
//  │ CMD_LINE_PROMPT                                               │
//  ╰───────────────────────────────────────────────────────────────╯
void cmd_line_prompt(View *view, char *s) {
    char message_str[MAX_COLS + 1];
    int l;
    l = strnz__cpy(message_str, s, MAX_COLS);
    wmove(view->win, view->cmd_line, 0);
    if (l != 0) {
        wclrtoeol(view->win);
        wattron(view->win, A_REVERSE);
        waddstr(view->win, " ");
        waddstr(view->win, message_str);
        waddstr(view->win, " ");
        wattroff(view->win, A_REVERSE);
        waddstr(view->win, " ");
        wmove(view->win, view->cmd_line, l + 2);
    }
}
//  ╭───────────────────────────────────────────────────────────────╮
//  │ REMOVE_FILE                                                   │
//  ╰───────────────────────────────────────────────────────────────╯
void remove_file(View *view) {
    char c;
    if (view->f_at_end_remove) {
        wmove(view->win, view->cmd_line, 0);
        waddstr(view->win, "Remove File (Y or N)->");
        wclrtoeol(view->win);
        c = (char)wgetch(view->win);
        waddch(view->win, (char)toupper(c));
        if (c == 'Y' || c == 'y')
            remove(view->cur_file_str);
    }
}
//  ╭───────────────────────────────────────────────────────────────╮
//  │ VIEW_DISPLAY_HELP                                             │
//  ╰───────────────────────────────────────────────────────────────╯
void view_display_help(View *view) {
    int begy, begx;
    int eargc;
    char *eargv[MAXARGS];
    eargv[0] = HELP_CMD;
    eargv[1] = VIEW_HELP_FILE;
    eargv[2] = NULL;
    eargc = 2;
    begx = view->begx + 4;
    begy = view->begy + 1;
    mview(init, eargc, eargv, 10, 54, begy, begx, "View Help");
}
