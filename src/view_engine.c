/* view_engine.c
 * Bill Waller
 * billxwaller@gmail.com
 */
#include "menu.h"
#include <ctype.h>
#include <fcntl.h>
#include <regex.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/* constants
------------------------------------------------------------------------------*/

#define Ctrl(c) ((c) & 0x1f)
#define MAXLINE 1024
#define NMARKS 27

#define NULL_POSITION ((long)(-1))

#define MAXCHAR (unsigned char)0xfa
#define UL_START (unsigned char)0xfb
#define UL_END (unsigned char)0xfc
#define BO_START (unsigned char)0xfd
#define BO_END (unsigned char)0xfe

//    got     expect
//    -----   ---------
#define MO_NORMAL 0                //
#define MO_UL_EXPECT_CHR 1         //            next char
#define MO_UL_GOT_CHR_EXPECT_BS 2  //    char    ^H
#define MO_UL_GOT_BS_EXPECT_CHR 3  //    ^H      next char
#define MO_BO_EXPECT_CHR 4         //            next char
#define MO_BO_GOT_CHR_EXPECT_BS 5  //    char    ^H
#define MO_BO_GOT_BS_EXPECT_SAME 6 //    ^H      same char

/* macros
------------------------------------------------------------------------------*/

#define get_next_char()                                                        \
    do {                                                                       \
        if (++view->buf_ptr > view->buf_end)                                   \
            c = get_char_next_block(view);                                     \
        else                                                                   \
            c = *view->buf_ptr;                                                \
    } while (c == 0x0d);

#define get_prev_char()                                                        \
    {                                                                          \
        view->f_eof = FALSE;                                                   \
        do {                                                                   \
            if (--view->buf_ptr < view->buf_start)                             \
                c = get_char_prev_block(view);                                 \
            else                                                               \
                c = *view->buf_ptr;                                            \
        } while (c == 0x0d);                                                   \
    }

int view_file(View *);
int view_cmd_processor(View *);
int get_cmd_char(View *);
int get_cmd_spec(View *, char *);
void build_prompt(View *, char, char *);
void cat_file(View *);
void lp(char *, char *);
void go_to_mark(View *, int);
void go_to_eof(View *);
void go_to_line(View *, int);
void go_to_percent(View *, int);
void go_to_position(View *, long);
void search(View *, int, char *, int);
void read_forward_from_current_file_pos(View *, int);
void read_forward_from_file_pos(View *, int, long);
long read_line_forward(View *, long);
long read_line_forward_raw(View *, long);
void read_backward_from_current_file_pos(View *, int);
void read_backward_from_file_pos(View *, int, long);
long read_line_backward(View *, long);
long read_line_backward_raw(View *, long);
int initialize_buffers(View *, int);
int get_char_next_block(View *);
int get_char_prev_block(View *);
int get_char_buffer(View *);
int locate_byte_pipe(View *, long);
void put_line(View *);
void display_error_msg(View *, char *);
void cmd_line_prompt(View *, char *);
void remove_file(View *);
void view_display_help(View *);

char err_msg[MAXLEN];

/* ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
   ┃ VIEW_FILE                                             ┃
   ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛ */
int view_file(View *view) {
    if (view->argc < 1) {
        view->curr_argc = -1;
        view->argc = 0;
        view->argv[0][0] = '-';
    }
    view->next_file_spec_ptr = view->argv[0];
    while (view->curr_argc < view->argc) {
        if (view->next_file_spec_ptr == NULL ||
            *view->next_file_spec_ptr == '\0') {
            break;
        }
        view->file_spec_ptr = view->next_file_spec_ptr;
        view->next_file_spec_ptr = NULL;
        if (view_init_input(view, view->file_spec_ptr)) {
            if (view->fd >= 0) {
                view_cmd_processor(view);
            }
        } else {
            view->curr_argc++;
            if (view->curr_argc < view->argc) {
                view->next_file_spec_ptr = view->argv[view->curr_argc];
            }
        }
    }
    if (view->fd != 0) {
        close(view->fd);
        remove_file(view);
    }
    return (0);
}

/* ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
   ┃ VIEW_CMD_PROCESSOR                                    ┃
   ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛ */
int view_cmd_processor(View *view) {
    int tfd;
    char tmp_str[MAXLEN];
    int c;
    int prev_search_cmd = 0;
    int n;
    char *EditorPtr;
    char PromptStr[MAXLEN];
    char shell_cmd_spec[MAXLEN];

    if (view->start_cmd[0]) {
        view->next_c = view->start_cmd[0];
        strnz_cpy(view->cmd_spec, (char *)&view->start_cmd[1], MAXLEN - 1);
    } else
        view->cmd_spec[0] = '\0';

    if (setjmp(view->cmd_jmp))
        view->next_c = 0;

    while (1) {
        c = view->next_c;
        view->next_c = 0;
        if (!c) {
            if (view->pos_tbl[view->ptop] == NULL_POSITION)
                go_to_position(view, (long)0);
            else if (view->f_redraw_screen) {
                read_forward_from_file_pos(view, view->scroll_lines,
                                           view->pos_tbl[view->ptop]);
                view->f_redraw_screen = FALSE;
            }
            build_prompt(view, view->prompt[0], (char *)&PromptStr);
            if (PromptStr[0] == '\0')
                cmd_line_prompt(view, "");
            else
                cmd_line_prompt(view, PromptStr);
            c = get_cmd_char(view);
            if (c >= '0' && c <= '9') {
                tmp_str[0] = (char)c;
                tmp_str[1] = '\0';
                c = get_cmd_spec(view, tmp_str);
            }
        }
        switch (c) {

        case KEY_LEFT:
        case KEY_BACKSPACE:
        case Ctrl('H'):
            n = atoi(view->cmd_spec);
            if (n <= 0)
                n = 1;
            if (view->first_column >= n)
                view->first_column = view->first_column - n;
            else {
                if (view->first_column == 0) {
                    if (view->max_col >= view->cols)
                        view->first_column = view->max_col - view->cols;
                    else
                        view->first_column = 0;
                } else
                    view->first_column = 0;
            }
            view->last_column = view->first_column + view->cols;
            read_forward_from_file_pos(view, view->scroll_lines,
                                       view->pos_tbl[view->ptop]);
            break;

        case KEY_RIGHT:
        case 'l':
        case 'L':
            n = atoi(view->cmd_spec);
            if (n <= 0)
                n = 1;
            if ((view->first_column + view->cols) < view->max_col)
                view->first_column += n;
            else
                view->first_column = 0;
            view->last_column = view->first_column + view->cols;
            read_forward_from_file_pos(view, view->scroll_lines,
                                       view->pos_tbl[view->ptop]);
            break;

        case KEY_UP:
        case 'k':
        case 'K':
        case Ctrl('K'):
            n = atoi(view->cmd_spec);
            if (n <= 0)
                n = 1;
            read_backward_from_current_file_pos(view, n);
            break;

        case KEY_DOWN:
        case KEY_ENTER:
        case '\n':
        case '\r':
        case ' ':
        case 'j':
        case 'J':
            n = atoi(view->cmd_spec);
            if (n <= 0)
                n = 1;
            read_forward_from_current_file_pos(view, n);
            /*   idlok(view->win, TRUE); */
            break;

        case KEY_PPAGE:
        case 'b':
        case 'B':
        case Ctrl('B'):
            n = view->scroll_lines;
            read_backward_from_current_file_pos(view, n);
            break;

        case KEY_NPAGE:
        case 'f':
        case 'F':
        case Ctrl('F'):
            n = view->scroll_lines;
            read_forward_from_current_file_pos(view, n);
            break;

        case KEY_HOME:
        case 'g':
            view->first_column = 0;
            view->last_column = view->cols;
            go_to_line(view, 1);
            break;

        case KEY_LL:
            go_to_eof(view);
            break;

        case '!':
            if (view->f_displaying_help)
                break;
            if (get_cmd_spec(view, "!") == 0) {
                if (!view->f_is_pipe) {
                    view->prev_file_pos = view->pos_tbl[view->ptop];
                    close(view->fd);
                    view->fd = -1;
                    view->next_file_spec_ptr = view->file_spec_ptr;
                    str_subc(shell_cmd_spec, view->cmd_spec, '%',
                             view->cur_file_str, MAXLEN - 1);
                } else
                    strcpy(shell_cmd_spec, view->cmd_spec);
                full_screen_shell(shell_cmd_spec);
                if (!view->f_is_pipe) {
                    view->next_file_spec_ptr = view->cur_file_str;
                    return (0);
                }
            }
            break;

        case '+':
            if (get_cmd_spec(view, "Startup Command:") == 0)
                strncpy(view->start_cmd_all_files, view->cmd_spec, MAXLEN - 1);
            break;

        case '-':
            if (view->f_displaying_help)
                break;
            cmd_line_prompt(view, "(C, I, P, S, T, or H for Help)->");
            c = get_cmd_char(view);
            c = tolower(c);
            switch (c) {
            case 'c':
                cmd_line_prompt(view, "Clear Screen at End (Y or N)->");
                if ((c = get_cmd_char(view)) == 'y' || c == 'Y')
                    view->f_at_end_clear = TRUE;
                else if (c == 'n' || c == 'N')
                    view->f_at_end_clear = FALSE;
                break;
            case 'i':
                cmd_line_prompt(view, "Ignore Case in search (Y or N)->");
                if ((c = get_cmd_char(view)) == 'y' || c == 'Y')
                    view->f_ignore_case = TRUE;
                else if (c == 'n' || c == 'N')
                    view->f_ignore_case = FALSE;
                break;
            case 'p':
                cmd_line_prompt(view, "(Short Long or No prompt)->");
                c = tolower(get_cmd_char(view));
                switch (c) {
                case 's':
                    view->prompt[0] = 'S';
                    view->prompt[1] = '\0';
                    break;
                case 'l':
                    view->prompt[0] = 'L';
                    view->prompt[1] = '\0';
                    break;
                case 'n':
                    view->prompt[0] = 'N';
                    view->prompt[1] = '\0';
                    break;
                default:
                    break;
                }
                break;
            case 's':
                cmd_line_prompt(
                    view, "view->f_squeeze Multiple Blank lines (Y or N)->");
                if ((c = get_cmd_char(view)) == 'y' || c == 'Y')
                    view->f_squeeze = TRUE;
                else if (c == 'n' || c == 'N')
                    view->f_squeeze = FALSE;
                break;
            case 't':
                sprintf(tmp_str,
                        "Tabstop Colums Currently %d:", view->tab_stop);
                n = 0;
                if (get_cmd_spec(view, tmp_str) == 0)
                    n = atoi(view->cmd_spec);
                if (n >= 1 && n <= 12) {
                    view->tab_stop = n;
                    view->f_redraw_screen = TRUE;
                } else
                    display_error_message("Tab stops not changed");
                break;
            case 'h':
                if (!view->f_displaying_help)
                    view_display_help(view);
                view->next_c = '-';
                break;
            default:
                break;
            }
            break;

        case ':':
            view->next_c = get_cmd_spec(view, ":");
            break;

        case '/':
        case '?':
            n = atoi(view->cmd_spec);
            if (n < 1 || n > 9)
                n = 1;
            tmp_str[0] = (char)c;
            tmp_str[1] = '\0';
            if (get_cmd_spec(view, tmp_str) == 0) {
                search(view, c, view->cmd_spec, n);
                prev_search_cmd = c;
            }
            break;

        case '=':
        case Ctrl('G'):
            build_prompt(view, 'L', (char *)&PromptStr);
            break;

        case '@':
            break;

        case 'd':
        case 'D':
        case Ctrl('D'):
            n = atoi(view->cmd_spec);
            if (n < 1)
                n = 10;
            read_forward_from_current_file_pos(view, n);
            break;

        case 'o':
        case 'O':
        case 'e':
        case 'E':
            if (get_cmd_spec(view, "File name:") == 0) {
                strtok(view->cmd_spec, " ");
                view->next_file_spec_ptr = strdup(view->cmd_spec);
                view->f_redraw_screen = TRUE;
                return (0);
            }
            break;

        case 'G':
            n = atoi(view->cmd_spec);
            if (n <= 0)
                go_to_eof(view);
            else
                go_to_line(view, n);
            break;

        case 'h':
        case 'H':
            if (!view->f_displaying_help)
                view_display_help(view);
            break;

        case 'm':
            cmd_line_prompt(view, "Mark label (A-Z)->");
            c = get_cmd_char(view);
            if (c == '@' || c == KEY_F(9) || c == '\033')
                break;
            c = tolower(c);
            if (c < 'a' || c > 'z')
                display_error_message("Not (A-Z)");
            else
                view->mark_tbl[c - 'a'] = view->pos_tbl[view->ptop];
            break;

        case 'M':
        case '\'':
            cmd_line_prompt(view, "Goto mark (A-Z)->");
            c = get_cmd_char(view);
            if (c == '@' || c == KEY_F(9) || c == '\033')
                break;
            c = tolower(c);
            if (c < 'a' || c > 'z')
                display_error_message("Not (A-Z)");
            else
                go_to_mark(view, c);
            break;

        case 'n':
            n = atoi(view->cmd_spec);
            if (n < 1 || n > 9)
                n = 1;
            search(view, prev_search_cmd, NULL, n);
            break;

        case 'N':
            n = atoi(view->cmd_spec);
            if (n <= 0)
                n = 1;
            if (view->curr_argc + n >= view->argc) {
                display_error_message("no more files");
                view->curr_argc = view->argc - 1;
                n = 0;
            } else {
                view->curr_argc++;
                if (view->curr_argc < view->argc)
                    view->next_file_spec_ptr = view->argv[view->curr_argc];
                return (0);
            }
            break;

        case 'p':
        case '%':
            n = atoi(view->cmd_spec);
            if (n < 0)
                go_to_line(view, 1);
            if (n >= 100)
                go_to_eof(view);
            else
                go_to_percent(view, n);
            n = 0;
            break;

        case Ctrl('Z'):
            if (view->f_is_pipe) {
                display_error_message("Can't print standard input");
                break;
            }
            get_cmd_spec(view, "Enter Notation:");
            strncpy(tmp_str, "/tmp/view-XXXXXX", MAXLEN - 1);
            tfd = mkstemp(tmp_str);
            strcpy(view->tmp_file_name_ptr, tmp_str);
            if (tfd == -1) {
                display_error_message("Unable to create temporary file");
                break;
            }
            strncpy(shell_cmd_spec, "echo ", MAXLEN - 5);
            strncat(shell_cmd_spec, view->cmd_spec, MAXLEN - 5);
            strncat(shell_cmd_spec, view->tmp_file_name_ptr, MAXLEN - 5);
            shell(shell_cmd_spec);
            strncpy(shell_cmd_spec, "cat ", MAXLEN - 5);
            strncat(shell_cmd_spec, view->cmd_spec, MAXLEN - 5);
            strncat(shell_cmd_spec, ">>", MAXLEN - 5);
            strncat(shell_cmd_spec, view->tmp_file_name_ptr, MAXLEN - 5);
            shell(shell_cmd_spec);
            lp(view->cur_file_str, view->cmd_spec);
            wrefresh(view->win);
            shell(shell_cmd_spec);
            snprintf(shell_cmd_spec, (size_t)(MAXLEN - 5), "rm %s",
                     view->tmp_file_name_ptr);
            strncpy(shell_cmd_spec, "rm ", MAXLEN - 5);
            strncat(shell_cmd_spec, view->tmp_file_name_ptr, MAXLEN - 5);
            shell(shell_cmd_spec);
            restore_wins();
            view->f_redraw_screen = TRUE;
            unlink(tmp_str);
            break;

        case Ctrl('P'):
        case KEY_CATAB:
        case KEY_PRINT:
            if (view->f_is_pipe) {
                display_error_message("Can't print standard input");
                break;
            }
            lp(view->cur_file_str, NULL);
            view->f_redraw_screen = TRUE;
            break;

        case 'P':
            n = atoi(view->cmd_spec);
            if (n <= 0)
                n = 1;
            if (view->curr_argc - n < 0) {
                display_error_message("No previous file");
                view->curr_argc = 0;
                n = 0;
            } else {
                view->curr_argc--;
                if (view->curr_argc >= 0)
                    view->next_file_spec_ptr = view->argv[view->curr_argc];
                return (0);
            }
            break;

        case 'q':
        case 'Q':
        case KEY_F(9):
        case '\033':
            view->curr_argc = view->argc;
            view->next_file_spec_ptr = NULL;
            return (0);

        case 'r':
        case 'R':
        case Ctrl('R'):
        case Ctrl('L'):
            restore_wins();
            view->f_redraw_screen = TRUE;
            break;

        case 'u':
        case 'U':
        case Ctrl('U'):
            n = atoi(view->cmd_spec);
            if (n < 1)
                n = 10;
            read_backward_from_current_file_pos(view, n);
            break;

        case 'v':
            if (view->f_displaying_help)
                break;
            if (view->f_is_pipe) {
                display_error_message("Can't edit standard input");
                break;
            }
            EditorPtr = getenv("EDITOR");
            if (EditorPtr == NULL || *EditorPtr == '\0')
                EditorPtr = EDITOR;
            if (EditorPtr == NULL || *EditorPtr == '\0') {
                display_error_message("set EDITOR environment variable");
                break;
            }
            view->prev_file_pos = view->pos_tbl[view->ptop];
            close(view->fd);
            view->fd = -1;
            view->next_file_spec_ptr = view->file_spec_ptr;
            strncpy(shell_cmd_spec, EditorPtr, MAXLEN - 5);
            strncat(shell_cmd_spec, " ", MAXLEN - 5);
            strncat(shell_cmd_spec, view->cur_file_str, MAXLEN - 5);
            full_screen_shell(shell_cmd_spec);
            return (0);

        case 'V':
            display_error_message("View: Version 8.0");
            break;

        default:
            break;
        }
        view->cmd_spec[0] = '\0';
    }
}

/* ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
   ┃ GET_CMD_CHAR                                          ┃
   ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛ */
int get_cmd_char(View *view) {
    int c;
    // idlok(view->win, FALSE);
    c = wgetch(view->win);
    view->cmd_spec[0] = '\0';
    return (c);
}

/* ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
   ┃ GET_CMD_SPEC                                          ┃
   ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛ */
int get_cmd_spec(View *view, char *MsgPtr) {
    int c;
    int numeric_arg = FALSE;
    char *cmd_ptr;
    char *cmd_end;
    char message_str[MAX_COLS + 1];
    char *s, *d, *e;
    int l;

    s = MsgPtr;
    d = message_str;
    e = d + view->cols - 4;
    l = 0;
    while (*s != '\0' && d < e) {
        *d++ = *s++;
        l++;
    }
    *d = '\0';
    if (view->cmd_spec[0] != '\0')
        return (0);
    cmd_ptr = view->cmd_spec;

    wmove(view->win, view->cmd_line, 0);
    if (view->cols < MAXLEN - 1)
        cmd_end = cmd_ptr + view->cols - l - 3;
    else
        cmd_end = cmd_ptr + MAXLEN - l - 3;
    if (l == 0)
        numeric_arg = TRUE;
    if (l > 1) {
        wstandout(view->win);
        waddch(view->win, ' ');
        waddstr(view->win, message_str);
        waddch(view->win, ' ');
        wstandend(view->win);
    } else {
        if (*MsgPtr == ':')
            numeric_arg = TRUE;
        else {
            s = MsgPtr;
            if (*s >= '0' && *s <= '9') {
                *cmd_ptr++ = *s;
                *cmd_ptr = '\0';
                numeric_arg = TRUE;
            }
        }
        waddstr(view->win, message_str);
        wmove(view->win, view->cmd_line, l);
    }
    wclrtoeol(view->win);
    wrefresh(view->win);

    while (1) {

        wrefresh(view->win);

        c = wgetch(view->win);

        switch (c) {
        case KEY_LEFT:
        case KEY_BACKSPACE:
        case '\b':
            if (cmd_ptr > view->cmd_spec) {
                cmd_ptr--;
                if (*cmd_ptr < ' ' || *cmd_ptr == 0x7f) {
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
            return (0);

        case '@':
        case KEY_F(9):
        case '\033':
            return (-1);

        default:
            *cmd_ptr++ = (char)c;
            *cmd_ptr = '\0';
            if ((char)c < ' ') {
                waddch(view->win, '^');
                c |= '@';
            } else if ((unsigned char)c == 0x7f)
                c = '?';
            waddch(view->win, (char)c);
            if (cmd_ptr >= cmd_end)
                return (0);
            if (numeric_arg && (c < '0' || c > '9'))
                return (c);
            break;
        }
    }
}

/* ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
   ┃ BUILD PROMPT                                          ┃
   ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛ */
void build_prompt(View *view, char build_prompt_type, char *prompt_ptr) {
    *prompt_ptr = '\0';
    if (build_prompt_type == 'L' || view->f_new_file) {
        if (view->f_is_pipe)
            strncat(prompt_ptr, "stdin", MAXLEN - 1);
        else
            strncat(prompt_ptr, view->cur_file_str, MAXLEN - 1);
    }
    if (view->first_column > 0) {
        sprintf(tmp_str, "Col %d", view->first_column);
        if (*prompt_ptr != '\0')
            strncat(prompt_ptr, " ", MAXLEN - 1);
        strncat(prompt_ptr, tmp_str, MAXLEN - 1);
    }
    if (view->argc > 1 && (view->f_new_file || build_prompt_type == 'L')) {
        sprintf(tmp_str, "File %d of %d", view->curr_argc + 1, view->argc);
        if (*prompt_ptr != '\0')
            strncat(prompt_ptr, " ", MAXLEN - 1);
        strncat(prompt_ptr, tmp_str, MAXLEN - 1); /* File Of      */
    }
    if (build_prompt_type == 'L') { /* Byte of Byte  */
        view->file_pos = view->pos_tbl[view->pbot];
        if (view->file_pos == NULL_POSITION)
            view->file_pos = view->size_bytes;
        if (view->file_pos != NULL_POSITION) {
            sprintf(tmp_str, "Byte %ld", view->file_pos);
            if (*prompt_ptr != '\0')
                strncat(prompt_ptr, " ", MAXLEN - 1);
            strncat(prompt_ptr, tmp_str, MAXLEN - 1);
            if (view->size_bytes > 0) {
                sprintf(tmp_str, " of %ld", view->size_bytes);
                strncat(prompt_ptr, tmp_str, MAXLEN - 1);
            }
        }
    }
    if (!view->f_eof && build_prompt_type != 'N') { /* Percent       */
        view->file_pos = view->pos_tbl[view->pbot];
        if (view->size_bytes > 0L && view->file_pos != NULL_POSITION) {
            sprintf(tmp_str, "(%ld%%)",
                    (100L * view->file_pos) / view->size_bytes);
            if (*prompt_ptr != '\0')
                strncat(prompt_ptr, " ", MAXLEN - 1);
            strncat(prompt_ptr, tmp_str, MAXLEN - 1);
        }
    }
    if (view->f_eof) { /* End           */
        if (*prompt_ptr != '\0')
            strncat(prompt_ptr, " ", MAXLEN - 1);
        strncat(prompt_ptr, "(End)", MAXLEN - 1);
        if (view->curr_argc + 1 < view->argc) {
            sprintf(tmp_str, " Next File: %s", view->argv[view->curr_argc + 1]);
            strncat(prompt_ptr, tmp_str, MAXLEN - 1);
        }
    }
    if (*prompt_ptr != '\0') {
        strncat(prompt_ptr, " ", MAXLEN - 1);
    }
    view->f_new_file = FALSE;
}

/* ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
   ┃ CAT FILE                                              ┃
   ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛ */
void cat_file(View *view) {
    register int c;

    while (1) {
        get_next_char();
        if (view->f_eof)
            break;
        putchar(c);
    }
}

/* ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
   ┃ LP                                                    ┃
   ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛ */
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

/* ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
   ┃ GO TO MARK                                            ┃
   ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛ */
void go_to_mark(View *view, int c) {
    if (c == '\'')
        view->file_pos = view->mark_tbl[(NMARKS - 1)];
    else
        view->file_pos = view->mark_tbl[c - 'a'];
    if (view->file_pos == NULL_POSITION)
        display_error_message("Mark not set");
    else
        go_to_position(view, view->file_pos);
}

/* ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
   ┃ GO TO EOF                                             ┃
   ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛ */
void go_to_eof(View *view) {
    register int c;

    if (view->f_is_pipe) {
        cmd_line_prompt(view, "Seeking end of pipe");
        view->f_forward = TRUE;
        c = get_char_buffer(view);
        while (!view->f_eof)
            get_next_char();
        get_prev_char();
        view->file_pos = view->blk_no * VBUFSIZ +
                         (long)(view->buf_ptr - view->buf_start) + 1;
    } else
        view->file_pos = lseek(view->fd, 0L, SEEK_END);
    if (view->file_pos < 0 || view->file_pos > view->size_bytes) {
        sprintf(tmp_str, "Error seeking end of file: pos %ld, size %ld",
                view->file_pos, view->size_bytes);
        display_error_message(tmp_str);
    }
    view->mark_tbl[(NMARKS - 1)] = view->pos_tbl[view->ptop];
    view->pos_tbl[view->ptop] = view->file_pos;
    read_backward_from_file_pos(view, view->scroll_lines, view->file_pos);
}

/* ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
   ┃ GO TO LINE                                            ┃
   ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛ */
void go_to_line(View *view, int LineNumber) {
    block_struct *block_ptr;
    register int c;
    int LineCnt;

    if (view->f_is_pipe) {
        if (locate_byte_pipe(view, (long)0) != 0) {
            if (LineNumber <= 1) {          /* can't go to beginning of file */
                block_ptr = view->blk_curr; /* so go back as far as possible */
                view->blk_no = block_ptr->block_no;
                do {
                    if (block_ptr > view->blk_first)
                        block_ptr--;
                    else
                        block_ptr = view->blk_last;
                    if (block_ptr->block_no < view->blk_no)
                        view->blk_no = block_ptr->block_no;
                } while (block_ptr != view->blk_curr);
                view->blk_curr = block_ptr;
                go_to_position(view, view->blk_no * VBUFSIZ);
                display_error_message("Unable to reach beginning of file");
                return;
            }
        }
    } else {
        view->blk_no = (long)0;
        view->blk_offset = 0;
    }
    if (LineNumber <= 1) {
        go_to_position(view, (long)0);
        return;
    }
    view->f_forward = TRUE;
    c = get_char_buffer(view);
    if (c == '\r')
        get_next_char();
    for (LineCnt = 1; LineCnt < LineNumber; LineCnt++) {
        while (c != '\n' && !view->f_eof)
            get_next_char();
        if (view->f_eof) {
            sprintf(tmp_str, "File only has %d lines", LineCnt - 1);
            display_error_message(tmp_str);
            return;
        }
        get_next_char();
    }
    go_to_position(view, view->blk_no * VBUFSIZ +
                             (long)(view->buf_ptr - view->buf_start));
}

/* ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
   ┃ GO TO PERCENT                                         ┃
   ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛ */
void go_to_percent(View *view, int Percent) {
    register int c;

    if (view->size_bytes == NULL_POSITION) {
        display_error_message("Don't know length of file");
        return;
    }
    view->file_pos = ((long)Percent * view->size_bytes) / 100L;
    if (view->f_is_pipe) {
        if (locate_byte_pipe(view, view->file_pos) != 0) {
            display_error_message("Unable to reach this position of file");
            return;
        }
    } else {
        view->blk_no = view->file_pos / VBUFSIZ;
        view->blk_offset = (int)(view->file_pos % VBUFSIZ);
    }
    view->f_forward = TRUE;
    c = get_char_buffer(view);
    while (c != '\n') {
        get_prev_char();
        if (view->f_beg_of_file)
            break;
    }
    get_next_char();
    view->file_pos =
        view->blk_no * VBUFSIZ + (long)(view->buf_ptr - view->buf_start);
    go_to_position(view, view->file_pos);
}

void go_to_position(View *view, long GoToPos) {
    view->mark_tbl[(NMARKS - 1)] = view->pos_tbl[view->ptop];
    read_forward_from_file_pos(view, view->scroll_lines, GoToPos);
}

/* ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
   ┃ SEARCH                                                ┃
   ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛ */
void search(View *view, int search_cmd, char *regex, int n) {
    static long line_pos;
    char f_wrapped = FALSE;
    regex_t preg;
    int REG_FLAGS = 0;
    int reti;

    if (search_cmd == '/')
        strcpy(tmp_str, "Forward");
    else
        strcpy(tmp_str, "Backward");
    cmd_line_prompt(view, tmp_str);
    wrefresh(view->win);
    if (regex == NULL || *regex == '\0') {
        display_error_message("No previous regular expression");
        return;
    } else {
        if (view->f_ignore_case)
            REG_FLAGS = REG_ICASE;
        else
            REG_FLAGS = 0;
        reti = regcomp(&preg, regex, REG_FLAGS);
        if (reti) {
            display_error_message("Invalid pattern");
            return;
        }
    }
    if (view->pos_tbl[view->ptop] == NULL_POSITION) {
        if (search_cmd == '/')
            view->file_pos = (long)0;
        else {
            view->file_pos = view->size_bytes;
            f_wrapped = TRUE;
        }
    } else {
        if (search_cmd == '/') {
            if (view->ptop < view->scroll_lines)
                view->file_pos = view->pos_tbl[view->ptop + 1];
            else
                view->file_pos = view->pos_tbl[0];
        } else {
            if (view->pos_tbl[view->ptop] == (long)0) {
                f_wrapped = TRUE;
                view->file_pos = view->size_bytes;
            } else
                view->file_pos = view->pos_tbl[view->ptop];
        }
    }
    if (search_cmd == '/') {
        line_pos = view->file_pos;
        view->file_pos = read_line_forward_raw(view, view->file_pos);
    } else
        line_pos = view->file_pos =
            read_line_backward_raw(view, view->file_pos);
    if (view->file_pos == NULL_POSITION) {
        display_error_message("Nothing to search");
        return;
    }
    reti = regexec(&preg, view->line_start_ptr, 0, NULL, 0);
    if (search_cmd == '/') {
        line_pos = view->file_pos;
        view->file_pos = read_line_forward_raw(view, view->file_pos);
    } else
        line_pos = view->file_pos =
            read_line_backward_raw(view, view->file_pos);
    if (view->file_pos == NULL_POSITION) {
        if (f_wrapped) {
            display_error_message("Pattern not found");
            return;
        } else {
            if (search_cmd == '/') {
                line_pos = view->file_pos = (long)0;
                view->file_pos = read_line_forward_raw(view, view->file_pos);
            } else {
                view->file_pos = view->size_bytes;
                line_pos = view->file_pos =
                    read_line_backward_raw(view, view->file_pos);
            }
            f_wrapped = TRUE;
        }
    }
    go_to_position(view, line_pos);
}

/* ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
   ┃ READ_FORWARD_FROM_CURRENT_FILE_POS                    ┃
   ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛ */
void read_forward_from_current_file_pos(View *view, int number_of_lines) {
    view->file_pos = view->pos_tbl[view->pbot];
    if (view->file_pos == NULL_POSITION || view->file_pos == view->size_bytes) {
        view->f_eof = TRUE;
        return;
    }
    read_forward_from_file_pos(view, number_of_lines, view->file_pos);
}

/* ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
   ┃ READ_FORWARD_FROM_FILE_POS                            ┃
   ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛ */
void read_forward_from_file_pos(View *view, int number_of_lines,
                                long read_from_pos) {
    int f_display_last_screen_only = 0;

    if (number_of_lines >= view->scroll_lines) {
        view->cury = 0;
        view->max_col = 0;
        f_display_last_screen_only = 1;
    } else
        view->cury = view->scroll_lines;
    if (read_from_pos != view->pos_tbl[view->pbot]) {
        view->pbot = view->ptop;
        if (view->ptop == view->scroll_lines)
            view->ptop = 0;
        else
            view->ptop++;
        view->pos_tbl[view->pbot] = read_from_pos;
    }
    while (--number_of_lines >= 0) {
        read_from_pos = read_line_forward(view, read_from_pos);
        if (view->f_eof) {
            if (number_of_lines >= view->scroll_lines) {
                go_to_eof(view);
                return;
            }
            view->line_start_ptr = view->line_str + 2;
            *view->line_start_ptr = '\0';
        }
        if (!f_display_last_screen_only ||
            number_of_lines < view->scroll_lines) {
            view->pbot = view->ptop;
            if (view->ptop == view->scroll_lines)
                view->ptop = 0;
            else
                view->ptop++;
            view->pos_tbl[view->pbot] = read_from_pos;
            put_line(view);
        }
    }
    if (!view->f_eof) {
        read_from_pos = view->pos_tbl[view->pbot];
        if (read_from_pos == NULL_POSITION || read_from_pos == view->size_bytes)
            view->f_eof = TRUE;
    }
}

/* ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
   ┃ READ_LINE_FORWARD                                     ┃
   ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛ */
long read_line_forward(View *view, long current_file_pos) {
    register int c;
    register char *dst_line_ptr;

    if (current_file_pos == NULL_POSITION)
        return (NULL_POSITION);
    if (view->f_is_pipe) {
        if (locate_byte_pipe(view, current_file_pos) != 0)
            return (NULL_POSITION);
    } else {
        view->blk_no = current_file_pos / VBUFSIZ;
        view->blk_offset = (int)(current_file_pos % VBUFSIZ);
    }
    view->f_forward = TRUE;
    c = get_char_buffer(view);
    if (c == '\r')
        get_next_char();
    if (view->f_eof)
        return (NULL_POSITION);

    view->line_start_ptr = dst_line_ptr = view->line_str + 2;
    view->line_ptr = view->line_str + MAXLINE - 20;
    view->line_mode = MO_NORMAL;

    while (1) {
        if (c == '\0' || c == '\n' || view->f_eof) {
            switch (view->line_mode) {
            case MO_NORMAL:
                break;
            case MO_UL_GOT_CHR_EXPECT_BS:
                dst_line_ptr[0] = dst_line_ptr[-1];
                dst_line_ptr[-1] = UL_END;
                dst_line_ptr++;
                break;
            case MO_BO_GOT_CHR_EXPECT_BS:
                dst_line_ptr[0] = dst_line_ptr[-1];
                dst_line_ptr[-1] = BO_END;
                dst_line_ptr++;
                break;
            case MO_UL_GOT_BS_EXPECT_CHR:
            case MO_UL_EXPECT_CHR:
                *dst_line_ptr++ = UL_END;
                break;
            case MO_BO_GOT_BS_EXPECT_SAME:
            case MO_BO_EXPECT_CHR:
                *dst_line_ptr++ = BO_END;
                break;
            }
            break;
        }

        switch (view->line_mode) {
        case MO_NORMAL:
            if (dst_line_ptr[-1] != '\b')
                break;
            if (c == dst_line_ptr[-2]) {
                dst_line_ptr[-1] = dst_line_ptr[-2];
                dst_line_ptr[-2] = BO_START;
                dst_line_ptr--;
                view->line_mode = MO_BO_EXPECT_CHR;
                break;
            }
            if (c == '_' || dst_line_ptr[-2] == '_') {
                dst_line_ptr[-1] = dst_line_ptr[-2];
                dst_line_ptr[-2] = UL_START;
                dst_line_ptr++;
                if (c == '_')
                    c = dst_line_ptr[-2];
                dst_line_ptr -= 2;
                view->line_mode = MO_UL_EXPECT_CHR;
            }
            break;
        case MO_UL_GOT_BS_EXPECT_CHR:
            if (c != '_' && dst_line_ptr[-2] != '_' && c == dst_line_ptr[-2]) {
                dst_line_ptr[0] = dst_line_ptr[-2];
                dst_line_ptr[-2] = UL_END;
                dst_line_ptr[-1] = BO_START;
                view->line_mode = MO_BO_EXPECT_CHR;
                break;
            }
            if (c == '_')
                c = dst_line_ptr[-2];
            dst_line_ptr -= 2;
            view->line_mode = MO_UL_EXPECT_CHR;
            break;
        case MO_BO_GOT_BS_EXPECT_SAME:
            if (c != dst_line_ptr[-2] &&
                (c == '_' || dst_line_ptr[-2] == '_')) {
                dst_line_ptr[0] = dst_line_ptr[-2];
                dst_line_ptr[-2] = BO_END;
                dst_line_ptr[-1] = UL_START;
                if (c == '_')
                    c = dst_line_ptr[-2];
                view->line_mode = MO_UL_EXPECT_CHR;
                break;
            }
            dst_line_ptr -= 2;
            view->line_mode = MO_BO_EXPECT_CHR;
            break;
        case MO_UL_EXPECT_CHR:
            if (c == '\b') {
                view->line_mode = MO_UL_GOT_BS_EXPECT_CHR;
                break;
            }
            view->line_mode = MO_UL_GOT_CHR_EXPECT_BS;
            break;
        case MO_BO_EXPECT_CHR:
            if (c == '\b') {
                view->line_mode = MO_BO_GOT_BS_EXPECT_SAME;
                break;
            }
            view->line_mode = MO_BO_GOT_CHR_EXPECT_BS;
            break;
        case MO_UL_GOT_CHR_EXPECT_BS:
            if (c == '\b')
                view->line_mode = MO_UL_GOT_BS_EXPECT_CHR;
            else {
                dst_line_ptr[0] = dst_line_ptr[-1];
                dst_line_ptr[-1] = UL_END;
                dst_line_ptr++;
                view->line_mode = MO_NORMAL;
            }
            break;
        case MO_BO_GOT_CHR_EXPECT_BS:
            if (c == '\b')
                view->line_mode = MO_BO_GOT_BS_EXPECT_SAME;
            else {
                dst_line_ptr[0] = dst_line_ptr[-1];
                dst_line_ptr[-1] = BO_END;
                dst_line_ptr++;
                view->line_mode = MO_NORMAL;
            }
            break;
        }
        if ((unsigned char)c > MAXCHAR)
            *dst_line_ptr++ = '.';
        else
            *dst_line_ptr++ = (char)c;
        if (dst_line_ptr >= view->line_ptr)
            break;
        get_next_char();
    }
    *dst_line_ptr = '\0';
    if (view->f_squeeze && *view->line_start_ptr == '\0') {
        while (1) {
            get_next_char();
            if (c != '\n')
                break;
        }
        get_prev_char();
    }
    if (view->f_eof)
        get_prev_char();
    return (view->blk_no * VBUFSIZ + (long)(view->buf_ptr - view->buf_start) +
            1);
}

/* ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
   ┃ READ_LINE_FORWARD_RAW                                 ┃
   ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛ */
long read_line_forward_raw(View *view, long current_file_pos) {
    register int c;
    register char *dst_line_ptr;
    char *dst_line_end;

    if (current_file_pos == NULL_POSITION)
        return (NULL_POSITION);
    if (view->f_is_pipe) {
        if (locate_byte_pipe(view, current_file_pos) != 0)
            return (NULL_POSITION);
    } else {
        view->blk_no = current_file_pos / VBUFSIZ;
        view->blk_offset = (int)(current_file_pos % VBUFSIZ);
    }
    view->f_forward = TRUE;
    c = get_char_buffer(view);
    if (c == '\r')
        get_next_char();
    if (view->f_eof)
        return (NULL_POSITION);
    view->line_start_ptr = dst_line_ptr = view->line_str + 2;
    view->line_ptr = view->line_str + MAXLINE - 20;
    dst_line_end = dst_line_ptr + MAXLINE - 1;
    while (1) {
        if (c == '\n' || view->f_eof || dst_line_ptr >= dst_line_end)
            break;
        *dst_line_ptr++ = (char)c;
        if (dst_line_ptr >= view->line_ptr)
            break;
        get_next_char();
    }
    *dst_line_ptr = '\0';
    if (view->f_eof) {
        get_prev_char();
        return (NULL_POSITION);
    }
    return (view->blk_no * VBUFSIZ + (long)(view->buf_ptr - view->buf_start) +
            1);
}

/* ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
   ┃ READ_BACKWARD_FROM_CURRENT_FILE_POS                   ┃
   ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛ */
void read_backward_from_current_file_pos(View *view, int number_of_lines) {
    view->file_pos = view->pos_tbl[view->ptop];
    if (view->file_pos == NULL_POSITION || view->file_pos == (long)0)
        return;
    read_backward_from_file_pos(view, number_of_lines, view->file_pos);
}

/* ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
   ┃ READ_BACKWARD_FROM_FILE_POS                           ┃
   ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛ */
void read_backward_from_file_pos(View *view, int number_of_lines,
                                 long read_from_pos) {
    int f_display_last_screen_only = 0;

    if (number_of_lines >= view->scroll_lines) {
        f_display_last_screen_only = 1;
        view->max_col = 0;
    }
    while (--number_of_lines >= 0) {
        view->prev_file_pos = read_from_pos;
        read_from_pos = read_line_backward(view, read_from_pos);
        if (read_from_pos == NULL_POSITION) {
            if (view->f_is_pipe)
                read_forward_from_file_pos(view, view->scroll_lines,
                                           view->prev_file_pos);
            else
                read_forward_from_file_pos(view, view->scroll_lines, (long)0);
            return;
        }
        if (number_of_lines < view->scroll_lines) {
            if (!f_display_last_screen_only) {
                wmove(view->win, 0, 0);
                view->cury = 0;
                winsertln(view->win);
            } else
                view->cury = number_of_lines;
            put_line(view);
            view->ptop = view->pbot;
            if (view->pbot == 0)
                view->pbot = view->scroll_lines;
            else
                view->pbot--;
            view->pos_tbl[view->ptop] = read_from_pos;
        }
    }
    read_from_pos = view->pos_tbl[view->pbot];
    if (read_from_pos == NULL_POSITION || read_from_pos == view->size_bytes)
        view->f_eof = TRUE;
}

/* ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
   ┃ READ_LINE_BACKWARD                                    ┃
   ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛ */
long read_line_backward(View *view, long current_file_pos) {
    register int c;
    register char *dst_line_ptr;
    long NewPos, LineStartPos;
    int i;

    if (current_file_pos == NULL_POSITION || current_file_pos <= (long)0)
        return (NULL_POSITION);
    if (view->f_is_pipe) {
        if (locate_byte_pipe(view, current_file_pos - 1) != 0)
            return (NULL_POSITION);
    } else {
        view->blk_no = (current_file_pos - 1) / VBUFSIZ;
        view->blk_offset = (int)((current_file_pos - 1) % VBUFSIZ);
    }
    view->f_forward = FALSE;
    c = get_char_buffer(view);
    if (c == '\r')
        get_prev_char();
    if (view->f_squeeze) {
        if (c == '\n') {
            i = 0;
            while (1) {
                get_prev_char();
                i++;
                if (c != '\n')
                    break;
            }
            if (view->f_eof)
                return (NULL_POSITION);
            get_next_char();
            if (i > 1)
                get_next_char();
        }
    }
    while (1) {
        get_prev_char();
        if (c == '\n' || view->f_beg_of_file)
            break;
    }
    NewPos =
        view->blk_no * VBUFSIZ + (long)(view->buf_ptr - view->buf_start) + 1;
    LineStartPos = NewPos;
    view->line_start_ptr = dst_line_ptr = view->line_str + 2;
    view->line_ptr = view->line_str + MAXLINE - 12;
    view->line_mode = MO_NORMAL;
    do {
        get_next_char();
        NewPos = view->blk_no * VBUFSIZ +
                 (long)(view->buf_ptr - view->buf_start) + 1;
        if (c == '\0' || c == '\n' || view->f_eof) {
            switch (view->line_mode) {
            case MO_NORMAL:
                break;
            case MO_UL_GOT_CHR_EXPECT_BS:
                dst_line_ptr[0] = dst_line_ptr[-1];
                dst_line_ptr[-1] = UL_END;
                dst_line_ptr++;
                break;
            case MO_BO_GOT_CHR_EXPECT_BS:
                dst_line_ptr[0] = dst_line_ptr[-1];
                dst_line_ptr[-1] = BO_END;
                dst_line_ptr++;
                break;
            case MO_UL_GOT_BS_EXPECT_CHR:
            case MO_UL_EXPECT_CHR:
                *dst_line_ptr++ = UL_END;
                break;
            case MO_BO_GOT_BS_EXPECT_SAME:
            case MO_BO_EXPECT_CHR:
                *dst_line_ptr++ = BO_END;
                break;
            }
            view->line_mode = MO_NORMAL;
            break;
        }
        switch (view->line_mode) {
        case MO_NORMAL:
            if (dst_line_ptr[-1] != '\b')
                break;
            if (c == dst_line_ptr[-2]) {
                dst_line_ptr[-1] = dst_line_ptr[-2];
                dst_line_ptr[-2] = BO_START;
                dst_line_ptr--;
                view->line_mode = MO_BO_EXPECT_CHR;
                break;
            }
            if (c == '_' || dst_line_ptr[-2] == '_') {
                dst_line_ptr[-1] = dst_line_ptr[-2];
                dst_line_ptr[-2] = UL_START;
                dst_line_ptr++;
                if (c == '_')
                    c = dst_line_ptr[-2];
                dst_line_ptr -= 2;
                view->line_mode = MO_UL_EXPECT_CHR;
            }
            break;
        case MO_UL_GOT_BS_EXPECT_CHR:
            if (c != '_' && dst_line_ptr[-2] != '_' && c == dst_line_ptr[-2]) {
                dst_line_ptr[0] = dst_line_ptr[-2];
                dst_line_ptr[-2] = UL_END;
                dst_line_ptr[-1] = BO_START;
                view->line_mode = MO_BO_EXPECT_CHR;
                break;
            }
            if (c == '_')
                c = dst_line_ptr[-2];
            dst_line_ptr -= 2;
            view->line_mode = MO_UL_EXPECT_CHR;
            break;
        case MO_BO_GOT_BS_EXPECT_SAME:
            if (c != dst_line_ptr[-2] &&
                (c == '_' || dst_line_ptr[-2] == '_')) {
                dst_line_ptr[0] = dst_line_ptr[-2];
                dst_line_ptr[-2] = BO_END;
                dst_line_ptr[-1] = UL_START;
                if (c == '_')
                    c = dst_line_ptr[-2];
                view->line_mode = MO_UL_EXPECT_CHR;
                break;
            }
            dst_line_ptr -= 2;
            view->line_mode = MO_BO_EXPECT_CHR;
            break;
        case MO_UL_EXPECT_CHR:
            if (c == '\b') {
                view->line_mode = MO_UL_GOT_BS_EXPECT_CHR;
                break;
            }
            view->line_mode = MO_UL_GOT_CHR_EXPECT_BS;
            break;
        case MO_BO_EXPECT_CHR:
            if (c == '\b') {
                view->line_mode = MO_BO_GOT_BS_EXPECT_SAME;
                break;
            }
            view->line_mode = MO_BO_GOT_CHR_EXPECT_BS;
            break;
        case MO_UL_GOT_CHR_EXPECT_BS:
            if (c == '\b')
                view->line_mode = MO_UL_GOT_BS_EXPECT_CHR;
            else {
                dst_line_ptr[0] = dst_line_ptr[-1];
                dst_line_ptr[-1] = UL_END;
                dst_line_ptr++;
                view->line_mode = MO_NORMAL;
            }
            break;
        case MO_BO_GOT_CHR_EXPECT_BS:
            if (c == '\b')
                view->line_mode = MO_BO_GOT_BS_EXPECT_SAME;
            else {
                dst_line_ptr[0] = dst_line_ptr[-1];
                dst_line_ptr[-1] = BO_END;
                dst_line_ptr++;
                view->line_mode = MO_NORMAL;
            }
            break;
        } /* switch(view->line_mode) */
        if (c > MAXCHAR)
            *dst_line_ptr++ = '.';
        else
            *dst_line_ptr++ = (char)c;
        if (dst_line_ptr >= view->line_ptr)
            break;
    } while (NewPos < current_file_pos);
    *dst_line_ptr++ = '\0';
    return (LineStartPos);
}

/* ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
   ┃ READ_LINE_BACKWARD_RAW                                ┃
   ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛ */
long read_line_backward_raw(View *view, long current_file_pos) {
    register int c;
    register char *dst_line_ptr;

    if (current_file_pos == NULL_POSITION || current_file_pos <= (long)0)
        return (NULL_POSITION);
    if (view->f_is_pipe) {
        if (locate_byte_pipe(view, current_file_pos - 1) != 0)
            return (NULL_POSITION);
    } else {
        view->blk_no = (current_file_pos - 1) / VBUFSIZ;
        view->blk_offset = (int)((current_file_pos - 1) % VBUFSIZ);
    }
    dst_line_ptr = view->line_str + MAXLINE;
    view->line_start_ptr = view->line_str + 2;
    *--dst_line_ptr = '\0';
    view->f_forward = FALSE;
    c = get_char_buffer(view);
    if (c == '\r')
        get_next_char();
    while (1) {
        get_prev_char();
        if (c == '\n' || view->f_beg_of_file ||
            dst_line_ptr <= view->line_start_ptr)
            break;
        *--dst_line_ptr = (char)c;
    }
    view->line_start_ptr = dst_line_ptr;
    return (view->blk_no * VBUFSIZ + (long)(view->buf_ptr - view->buf_start) +
            1);
}

/* ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
   ┃ INITIALIZE_BUFFERS                                    ┃
   ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛ */
int initialize_buffers(View *view, int number_of_buffers) {
    block_struct *block_ptr;
    int i;

    if (view->blk_first != NULL)
        free((char *)view->blk_first);
    while (number_of_buffers > 0) {
        view->blk_first =
            (block_struct *)calloc(number_of_buffers, sizeof(block_struct));
        if (view->blk_first != NULL)
            break;
        number_of_buffers--;
    }
    if (view->blk_first == NULL) {
        sprintf(tmp_str, "Unable to allocate %d %d byte buffers",
                number_of_buffers, (int)sizeof(block_struct));
        display_error_message(tmp_str);
        remove_file(view);
        return (1);
    }
    block_ptr = view->blk_first;
    for (i = 0; i < number_of_buffers; i++)
        (block_ptr++)->block_no = (long)(-1);
    view->blk_last = (--block_ptr);
    view->blk_curr = view->blk_last;
    return (0);
}

/* ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
   ┃ GET_CHAR_NEXT_BLOCK                                   ┃
   ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛ */
int get_char_next_block(View *view) {
    view->blk_offset = 0;
    if (view->blk_no == view->last_blk_no) {
        view->f_eof = TRUE;
        return (EOF);
    }
    view->blk_no++;
    view->f_forward = TRUE;
    return (get_char_buffer(view));
}

/* ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
   ┃ GET_CHAR_PREV_BLOCK                                   ┃
   ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛ */
int get_char_prev_block(View *view) {
    block_struct *block_ptr;

    if (view->blk_no <= 0) {
        view->blk_no = view->blk_offset = 0;
        view->f_beg_of_file = TRUE;
        return (EOF);
    }
    if (view->f_is_pipe) {
        block_ptr = view->blk_curr;
        do {
            if (block_ptr->block_no == (view->blk_no - 1))
                break;
            if (block_ptr > view->blk_first)
                block_ptr--;
            else
                block_ptr = view->blk_last;
        } while (block_ptr != view->blk_curr);
        view->blk_curr = block_ptr;
        if (view->blk_curr->block_no != (view->blk_no - 1)) {
            view->f_beg_of_file = TRUE;
            return (EOF);
        }
    }
    view->blk_no--;
    view->blk_offset = VBUFSIZ - 1;
    view->f_forward = FALSE;
    return (get_char_buffer(view));
}

/* ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
   ┃ GET_CHAR_BUFFER                                       ┃
   ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛ */
int get_char_buffer(View *view) {
    block_struct *block_ptr;
    register int bytes_read;
    int bytes_in_buffer;
    long file_pos;

    view->f_beg_of_file = FALSE;
    view->f_eof = FALSE;
    block_ptr = view->blk_curr;
    do {
        if (block_ptr->block_no == view->blk_no) {
            view->blk_curr = block_ptr;
            if (view->blk_curr->data + view->blk_offset >=
                view->blk_curr->end_ptr) {
                view->f_eof = TRUE;
                return (EOF);
            }
            view->buf_start = view->blk_curr->data;
            view->buf_ptr = view->blk_curr->data + view->blk_offset;
            view->buf_end = view->blk_curr->end_ptr - 1;
            return (*view->buf_ptr);
        }
        if (view->f_forward) {
            if (block_ptr < view->blk_last)
                block_ptr++;
            else
                block_ptr = view->blk_first;
        } else {
            if (block_ptr > view->blk_first)
                block_ptr--;
            else
                block_ptr = view->blk_last;
        }
    } while (block_ptr != view->blk_curr);
    if (view->f_forward) {
        if (block_ptr < view->blk_last)
            block_ptr++;
        else
            block_ptr = view->blk_first;
    } else {
        if (block_ptr > view->blk_first)
            block_ptr--;
        else
            block_ptr = view->blk_last;
    }
    if (view->f_is_pipe) {
        if (view->blk_no == view->blk_curr->block_no + 1)
            block_ptr->block_no = view->blk_no;
        else {
            sprintf(tmp_str, "Unable to access pipe block %ld", view->blk_no);
            display_error_message(tmp_str);
            return (EOF);
        }
    } else {
        block_ptr->block_no = view->blk_no;
        file_pos = view->file_pos;
        if ((long)(view->blk_no * VBUFSIZ + view->blk_offset) <=
            view->size_bytes)
            view->file_pos =
                lseek(view->fd, (long)(view->blk_no * VBUFSIZ), SEEK_SET);
        else
            view->file_pos = lseek(view->fd, 0L, SEEK_END);
        if (view->file_pos == -1) {
            view->f_eof = TRUE;
            return (EOF);
        }
        if (view->file_pos < 0 || view->file_pos > view->size_bytes) {
            sprintf(tmp_str, "seek error: requested %ld got %ld size %ld",
                    (view->blk_no * VBUFSIZ + view->blk_offset), view->file_pos,
                    view->size_bytes);
            display_error_message(tmp_str);
            view->file_pos = lseek(view->fd, file_pos, SEEK_SET);
            if (view->file_pos == -1) {
                view->f_eof = TRUE;
                return (EOF);
            }
        }
    }
    bytes_in_buffer = 0;
    block_ptr->end_ptr = block_ptr->data;

    while (1) {
        bytes_read = VBUFSIZ - bytes_in_buffer;
        bytes_read =
            read(view->fd, &block_ptr->data[bytes_in_buffer], bytes_read);
        if (bytes_read <= 0) {
            view->last_blk_no = view->blk_no;
            if (view->f_is_pipe)
                view->size_bytes =
                    (long)(view->last_blk_no * VBUFSIZ + bytes_in_buffer);
            break;
        }
        bytes_in_buffer += bytes_read;
        block_ptr->end_ptr = block_ptr->data + bytes_in_buffer;
        if (bytes_in_buffer >= VBUFSIZ)
            break;
    }
    if (bytes_read < 0) {
        display_error_message("Error reading file");
        view->f_eof = TRUE;
        return (EOF);
    }
    view->blk_curr = block_ptr;
    if (view->blk_curr->data + view->blk_offset >= view->blk_curr->end_ptr) {
        view->f_eof = TRUE;
        return (EOF);
    }
    view->buf_start = view->blk_curr->data;
    view->buf_ptr = view->blk_curr->data + view->blk_offset;
    view->buf_end = view->blk_curr->end_ptr - 1;
    return (*view->buf_ptr);
}

/* ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
   ┃ LOCATE_BYTE_PIPE                                      ┃
   ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛ */
int locate_byte_pipe(View *view, long locate_file_pos) {
    block_struct *block_ptr;
    long new_blk_no;

    new_blk_no = locate_file_pos / VBUFSIZ;
    if (new_blk_no != view->blk_curr->block_no + 1) {
        block_ptr = view->blk_curr;
        do {
            if (block_ptr->block_no == new_blk_no) {
                view->blk_no = new_blk_no;
                view->blk_offset = (int)(locate_file_pos % VBUFSIZ);
                view->blk_curr = block_ptr;
                return (0);
            }
            if (block_ptr < view->blk_last)
                block_ptr++;
            else
                block_ptr = view->blk_first;
        } while (block_ptr != view->blk_curr);
        view->blk_curr = block_ptr;
        return (-1);
    }
    view->blk_no = new_blk_no;
    view->blk_offset = (int)(locate_file_pos % VBUFSIZ);
    return (0);
}

/* ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
   ┃ PUT_LINE                                              ┃
   ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛ */
void put_line(View *view) {
    char *c;
    char *line_out_ptr;
    int pl_column;

    line_out_ptr = view->line_out_str;
    if (view->cury > view->last_line) {
        scrollok(view->win, TRUE);
        scroll(view->win);
        scrollok(view->win, FALSE);
        view->cury = view->last_line;
    }
    wmove(view->win, view->cury, 0);

    pl_column = 0;
    line_out_ptr = view->line_out_str;
    for (c = view->line_start_ptr; *c != '\0'; c++) {
        if ((unsigned char)*c > MAXCHAR || (unsigned char)*c < ' ') {
            switch ((unsigned char)*c) {
            case UL_START:
                wattron(view->win, A_UNDERLINE);
                break;
            case UL_END:
                wattroff(view->win, A_UNDERLINE);
                break;
            case BO_START:
                wattron(view->win, A_BOLD);
                break;
            case BO_END:
                wattroff(view->win, A_BOLD);
                break;
            case '\t':
                do {
                    if (pl_column >= view->first_column &&
                        pl_column < view->last_column)
                        *line_out_ptr++ = ' ';
                    pl_column++;
                } while ((pl_column % view->tab_stop) != 0);
                break;
            case '\b':
                if (line_out_ptr > view->line_out_str)
                    line_out_ptr--;
                if (pl_column > 0)
                    pl_column--;
                break;
            case '\n':
                break;
            case '\r':
                break;
            default:
                /* if ((unsigned char)*c < ' ') *c == '^'; */
                /* else */
                if ((unsigned char)*c == 0x7f)
                    *c = '?';
            }
        }
        /* if (pl_column >= view->first_column && pl_column < view->last_column)
         */
        {
            *line_out_ptr++ = *c;
            pl_column++;
        }
    }
    *line_out_ptr = '\0';
    /* if (pl_column <= view->last_column)
           if ((pl_column - view->first_column) < view->cols) */
    if ((unsigned char)*c == '\0') {
        while (c > view->line_start_ptr) {
            if ((unsigned char)*--c != ' ')
                break;
            pl_column--;
        }
        /* if (pl_column > view->max_col)
               view->max_col = pl_column; */
    }
    waddstr(view->win, view->line_out_str);
    wclrtoeol(view->win);
    if (view->cury < view->scroll_lines)
        view->cury++;
    else
        view->cury = view->scroll_lines;
}

/* ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
   ┃ DISPLAY_ERROR_MSG                                     ┃
   ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛ */
void display_error_msg(View *view, char *s) {
    char message_str[MAX_COLS + 1];
    char *d, *e;

    d = message_str;
    e = d + view->cols - 4;
    while (*s != '\0' && d < e) {
        *d++ = *s++;
    }
    *d = '\0';
    cmd_line_prompt(view, message_str);
    wrefresh(view->win);

    view->next_c = wgetch(view->win);
    switch (view->next_c) {
    case '\n':
    case '\r':
    case ' ':
    case 'q':
    case 'Q':
    case KEY_F(9):
    case '\033':
        view->next_c = 0;
    default:
        break;
    }
    view->cmd_spec[0] = '\0';
}

/* ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
   ┃ CMD_LINE_PROMPT                                       ┃
   ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛ */
void cmd_line_prompt(View *view, char *s) {
    char message_str[MAX_COLS + 1];
    char *d, *e;
    int l;

    d = message_str;
    e = d + view->cols - 2;
    l = 0;
    while (*s != '\0' && d < e) {
        *d++ = *s++;
        l++;
    }
    *d = '\0';
    wmove(view->win, view->cmd_line, 0);
    if (l != 0) {
        wattron(view->win, A_REVERSE);
        waddstr(view->win, " ");
        waddstr(view->win, message_str);
        waddstr(view->win, " ");
        wattroff(view->win, A_REVERSE);
        waddstr(view->win, " ");
        wmove(view->win, view->cmd_line, l + 2);
    }
    wclrtoeol(view->win);
}

/* ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
   ┃ REMOVE_FILE                                           ┃
   ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛ */
void remove_file(View *view) {
    char c;

    if (view->f_at_end_remove) {
        wmove(view->win, view->cmd_line, 0);
        waddstr(view->win, "Remove File (Y or N)->");
        wclrtoeol(view->win);
        wrefresh(view->win);
        c = (char)wgetch(view->win);
        waddch(view->win, (char)toupper(c));
        wrefresh(view->win);
        if (c == 'Y' || c == 'y') {
            if ((view->fd = open(view->cur_file_str, O_TRUNC)) < 0) {
                strncpy(tmp_str, "Can't open ", MAXLEN - 1);
                strncat(tmp_str, view->cur_file_str, MAXLEN - 1);
                display_error_message(tmp_str);
            } else
                close(view->fd);
        }
    }
}

/* ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
   ┃ DISPLAY_HELP                                          ┃
   ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛ */
void view_display_help(View *view) {
    View *view_save;
    int begy, begx;
    int eargc;
    char *eargv[MAXARGS];

    eargv[0] = HELP_CMD;
    eargv[1] = VIEW_HELP_FILE;
    eargv[2] = NULL;
    eargc = 2;
    begx = view->begx + 4;
    begy = view->begy + 1;
    view_save = view;
    view = (View *)0;
    mview(init, eargc, eargv, 10, 54, begy, begx);
    view = view_save;
}
