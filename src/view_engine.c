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
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>

/*
╭───────────────────────────────────────────────────────────────────╮
│ CONSTANTS                                                         │
│ The following artifacts, circa 1980's, were used to process       │
│ nroff/troff manual pages for the Decision, Inc. Broadcast         │
│ software. TROFF, because it was designed for offset printing,     │
│ used backspace for underlining and overstrike bold characters.    │
│                                                                   │
│ Linux manual pages still use Groff/Nroff/Troff type processing.   │
│ For now, that code has been removed from view, but at some point, │
│ it may be may be resurrected.                                     │
╰───────────────────────────────────────────────────────────────────╯ */
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
/*
╭───────────────────────────────────────────────────────────────────╮
│ MACROS                                                            │
│                                                                   │
│ The following artifacts, circa 1980's, were used to get the best  │
│ performance from such ancient processors such as the Motorola     │
│ 68010/20/30 and the 80486. They still work.                       │ │ │ │ The
80486DX had a math coprocessor with eight 80-bit registers    │ │ and supported
packed BCD integer arithmetic as a data type.       │ │ BCD hardware is fast and
accurate for scientific and financial    │ │ calculatiions where floating-point
errors are unacceptable.       │ │ You can still get BCD hardware today in IBM's
System-Z and        │ │ Power platforms. │
╰───────────────────────────────────────────────────────────────────╯ */
#define Ctrl(c) ((c) & 0x1f)

#define get_next_char()                                                        \
    do {                                                                       \
        if (view->buf_curr_ptr == view->buf_end_ptr)                           \
            c = advance_buffer(view);                                          \
        else                                                                   \
            c = *view->buf_curr_ptr++;                                         \
    } while (c == 0x0d);

#define get_prev_char()                                                        \
    {                                                                          \
        do {                                                                   \
            if (view->buf_curr_ptr == view->buf)                               \
                c = advance_buffer(view);                                      \
            else                                                               \
                c = *--view->buf_curr_ptr;                                     \
        } while (c == 0x0d);                                                   \
    }

char prev_regex_pattern[MAXLEN];
FILE *dbgfp;
int view_file(View *);
int view_cmd_processor(View *);
int get_cmd_char(View *);
int get_cmd_spec(View *, char *);
void build_prompt(View *, int, char *);
void cat_file(View *);
void lp(char *, char *);

void go_to_mark(View *, int);
void go_to_eof(View *);
int go_to_line(View *, int);
void go_to_percent(View *, int);
void go_to_position(View *, long);

bool search(View *, int, char *, bool);

void next_page(View *);
void prev_page(View *);
void scroll_down_n_lines(View *, int);
void scroll_up_n_lines(View *, int);
long get_next_line(View *, long);
long get_prev_line(View *, long);
long get_pos_next_line(View *, long);
long get_pos_prev_line(View *, long);
void format_output_line(View *);
void display_output_line(View *);
int advance_buffer(View *);
bool load_buffer(View *, long);

void cmd_line_prompt(View *, char *);
void remove_file(View *);
void view_display_help(View *);

char err_msg[MAXLEN];

/*
╭───────────────────────────────────────────────────────────────────╮
│ VIEW_FILE                                                         │
╰───────────────────────────────────────────────────────────────────╯ */
int view_file(View *view) {
    if (view->argc < 1) {
        view->curr_argc = -1;
        view->argc = 0;
        view->argv[0][0] = '-';
    }
#ifdef DEBUG
    dbgfp = fopen("./view.log", "w");
#endif
    view->next_file_spec_ptr = view->argv[0];
    while (view->curr_argc < view->argc) {
        if (view->next_file_spec_ptr == NULL ||
            *view->next_file_spec_ptr == '\0') {
            break;
        }
        view->file_spec_ptr = view->next_file_spec_ptr;
        view->next_file_spec_ptr = NULL;
        if (view_init_input(view, view->file_spec_ptr)) {
            if (view->fp) {
                if (view->f_is_pipe) {
                    cat_file(view);
                    fclose(view->fp);
                    view->fp = NULL;
                    remove_file(view);
                } else {
                    view->f_new_file = true;
                    view->maxcol = 0;
                    view->f_forward = true;
                    view->page_top_pos = (long)0;
                    view->page_bot_pos = (long)0;
                    view->file_pos = 0;
                    next_page(view);
                    view_cmd_processor(view);
                    fclose(view->fp);
                }
            }
        } else {
            view->curr_argc++;
            if (view->curr_argc < view->argc) {
                view->next_file_spec_ptr = view->argv[view->curr_argc];
            }
        }
    }
    return (0);
}

/*
╭───────────────────────────────────────────────────────────────────╮
│ VIEW_CMD_PROCESSOR                                                │
╰───────────────────────────────────────────────────────────────────╯ */
int view_cmd_processor(View *view) {
    int tfd;
    char tmp_str[MAXLEN];
    int c;
    int prev_search_cmd = 0;
    int rc, n, i;
    char *editor_ptr;
    char shell_cmd_spec[MAXLEN];

    if (view->start_cmd[0]) {
        view->next_c = view->start_cmd[0];
        strnz__cpy(view->cmd_spec, (char *)&view->start_cmd[1], MAXLEN - 1);
    } else
        view->cmd_spec[0] = '\0';

    while (1) {
        c = view->next_c;
        view->next_c = 0;
        if (!c) {
            if (view->f_redraw_screen) {
                scroll_down_n_lines(view, view->scroll_lines);
                view->f_redraw_screen = false;
            }
            build_prompt(view, view->prompt_type, view->prompt_str);
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
            if (rc == ERR) {
                display_error_message("Error refreshing screen");
            }
            c = get_cmd_char(view);
            if (c >= '0' && c <= '9') {
                tmp_str[0] = (char)c;
                tmp_str[1] = '\0';
                c = get_cmd_spec(view, tmp_str);
            }
        }
        switch (c) {

        case KEY_ALTLEFT:
            n = COLS / 2;
            if (view->pmincol - n < 0)
                view->pmincol = 0;
            else
                view->pmincol -= n;
            break;

        case KEY_ALTRIGHT:
            n = COLS / 2;
            if ((view->pmincol + n) < view->maxcol)
                view->pmincol += n;
            break;

        case KEY_LEFT:
        case KEY_BACKSPACE:
        case Ctrl('H'):
            n = atoi(view->cmd_spec);
            if (n <= 0)
                n = 1;
            if ((view->pmincol - n) < 0)
                view->pmincol = 0;
            else
                view->pmincol -= n;
            break;

        case KEY_RIGHT:
        case 'l':
        case 'L':
            n = atoi(view->cmd_spec);
            if (n <= 0)
                n = 1;
            if ((view->pmincol + n) < view->maxcol)
                view->pmincol += n;
            break;

        case KEY_UP:
        case 'k':
        case 'K':
        case Ctrl('K'):
            n = atoi(view->cmd_spec);
            if (n <= 0)
                n = 1;
            scroll_up_n_lines(view, n);
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
            for (i = 0; i < n; i++) {
                scroll_down_n_lines(view, n);
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
            go_to_line(view, 0);
            break;

        case KEY_LL:
            go_to_eof(view);
            break;

        case '!':
            if (view->f_displaying_help)
                break;
            if (get_cmd_spec(view, "!") == 0) {
                if (!view->f_is_pipe) {
                    view->prev_file_pos = view->page_top_pos;
                    fclose(view->fp);
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
                    view->f_at_end_clear = true;
                else if (c == 'n' || c == 'N')
                    view->f_at_end_clear = false;
                break;
            case 'i':
                cmd_line_prompt(view, "Ignore Case in search (Y or N)->");
                if ((c = get_cmd_char(view)) == 'y' || c == 'Y')
                    view->f_ignore_case = true;
                else if (c == 'n' || c == 'N')
                    view->f_ignore_case = false;
                break;
            case 'p':
                cmd_line_prompt(view, "(Short Long or No prompt)->");
                c = tolower(get_cmd_char(view));
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
                if ((c = get_cmd_char(view)) == 'y' || c == 'Y')
                    view->f_squeeze = true;
                else if (c == 'n' || c == 'N')
                    view->f_squeeze = false;
                break;
            case 't':
                sprintf(tmp_str,
                        "Tabstop Colums Currently %d:", view->tab_stop);
                n = 0;
                if (get_cmd_spec(view, tmp_str) == 0)
                    n = atoi(view->cmd_spec);
                if (n >= 1 && n <= 12) {
                    view->tab_stop = n;
                    view->f_redraw_screen = true;
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
            strcpy(tmp_str, (c == '/') ? "(forward)->" : "(backward)->");
            if (get_cmd_spec(view, tmp_str) == 0) {
                view->f_wrap = false;
                search(view, c, view->cmd_spec, false);
                prev_search_cmd = c;
                strncpy(prev_regex_pattern, view->cmd_spec, MAXLEN - 1);
            }
            view->srch_beg_pos = view->page_top_pos;
            load_buffer(view, view->page_top_pos);
            break;

        case '@':
            break;

        case 'd':
        case 'D':
        case Ctrl('D'):
            n = atoi(view->cmd_spec);
            if (n < 1)
                n = 10;
            scroll_down_n_lines(view, n);
            break;

        case 'o':
        case 'O':
        case 'e':
        case 'E':
            if (get_cmd_spec(view, "File name:") == 0) {
                strtok(view->cmd_spec, " ");
                view->next_file_spec_ptr = strdup(view->cmd_spec);
                view->f_redraw_screen = true;
                return (0);
            }
            break;

        case KEY_END:
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
                view->mark_tbl[c - 'a'] = view->page_top_pos;
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
            search(view, prev_search_cmd, prev_regex_pattern, true);
            break;

        case 'N':
            n = atoi(view->cmd_spec);
            if (n <= 0)
                n = 1;
            if (view->curr_argc + n >= view->argc) {
                display_error_message("no more files");
                view->curr_argc = view->argc - 1;
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
            view->f_redraw_screen = true;
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
            view->f_redraw_screen = true;
            break;

        case 'P':
            n = atoi(view->cmd_spec);
            if (n <= 0)
                n = 1;
            if (view->curr_argc - n < 0) {
                display_error_message("No previous file");
                view->curr_argc = 0;
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
            view->f_redraw_screen = true;
            break;

        case 'u':
        case 'U':
        case Ctrl('U'):
            n = atoi(view->cmd_spec);
            if (n < 1)
                n = 10;
            scroll_up_n_lines(view, n);
            break;

        case 'v':
            if (view->f_displaying_help)
                break;
            if (view->f_is_pipe) {
                display_error_message("Can't edit standard input");
                break;
            }
            editor_ptr = getenv("DEFAULTEDITOR");
            if (editor_ptr == NULL || *editor_ptr == '\0')
                editor_ptr = DEFAULTEDITOR;
            if (editor_ptr == NULL || *editor_ptr == '\0') {
                display_error_message("set DEFAULTEDITOR environment variable");
                break;
            }
            view->prev_file_pos = view->page_top_pos;
            fclose(view->fp);
            view->next_file_spec_ptr = view->file_spec_ptr;
            strncpy(shell_cmd_spec, editor_ptr, MAXLEN - 5);
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

/*
╭───────────────────────────────────────────────────────────────────╮
│ GET_CMD_CHAR                                                      │
╰───────────────────────────────────────────────────────────────────╯ */
int get_cmd_char(View *view) {
    int c;
    tcflush(0, TCIFLUSH);
    c = wgetch(view->win);
    view->cmd_spec[0] = '\0';
    return (c);
}

/*
╭───────────────────────────────────────────────────────────────────╮
│ GET_CMD_SPEC                                                      │
╰───────────────────────────────────────────────────────────────────╯ */
int get_cmd_spec(View *view, char *prompt) {
    int c;
    int numeric_arg = false;
    char *cmd_p;
    char *cmd_e;
    char prompt_s[MAX_COLS + 1];
    char *n;
    int rc, prompt_l;

    prompt_l = strnz__cpy(prompt_s, prompt, view->cols - 4);

    if (view->cmd_spec[0] != '\0')
        return (0);
    cmd_p = view->cmd_spec;
    cmd_e = view->cmd_spec + MAXLEN - 2;

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
            display_error_message("Error refreshing screen");
        }
        if (rc == ERR) {
            display_error_message("Error refreshing screen");
        }
        c = wgetch(view->win);

        switch (c) {
        case KEY_LEFT:
        case KEY_BACKSPACE:
        case '\b':
            if (cmd_p > view->cmd_spec) {
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
            return (0);

        case '@':
        case KEY_F(9):
        case '\033':
            return (-1);

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
                return (0);
            if (numeric_arg && (c < '0' || c > '9'))
                return (c);
            break;
        }
    }
}

/*
╭───────────────────────────────────────────────────────────────────╮
│ BUILD_PROMPT                                                      │
╰───────────────────────────────────────────────────────────────────╯ */
void build_prompt(View *view, int prompt_type, char *prompt_str) {
    prompt_str[0] = '\0';
    strncpy(prompt_str, "", MAXLEN - 1);
    if (prompt_type == PT_LONG || view->f_new_file) {
        if (view->f_is_pipe)
            strncat(prompt_str, "stdin", MAXLEN - 1);
        else
            strncat(prompt_str, view->cur_file_str, MAXLEN - 1);
    }
    if (view->pmincol > 0) {
        sprintf(tmp_str, "Col %d", view->pmincol);
        if (prompt_str[0] != '\0')
            strncat(prompt_str, " ", MAXLEN - 1);
        strncat(prompt_str, tmp_str, MAXLEN - 1);
    }
    if (view->argc > 1 && (view->f_new_file || prompt_type == PT_LONG)) {
        sprintf(tmp_str, "File %d of %d", view->curr_argc + 1, view->argc);
        if (prompt_str[0] != '\0')
            strncat(prompt_str, " ", MAXLEN - 1);
        strncat(prompt_str, tmp_str, MAXLEN - 1); /* File Of      */
    }
    if (prompt_type == PT_LONG) { /* Byte of Byte  */
        if (view->page_top_pos == NULL_POSITION)
            view->page_top_pos = view->file_size;
        sprintf(tmp_str, "Pos %ld-%ld", view->page_top_pos, view->page_bot_pos);
        if (prompt_str[0] != '\0')
            strncat(prompt_str, " ", MAXLEN - 1);
        strncat(prompt_str, tmp_str, MAXLEN - 1);
        if (view->file_size > 0) {
            sprintf(tmp_str, " of %ld", view->file_size);
            strncat(prompt_str, tmp_str, MAXLEN - 1);
        }
    }
    if (!view->f_eod && prompt_type != PT_NONE) { /* Percent       */
        if (view->file_size > 0L && view->page_bot_pos != NULL_POSITION) {
            sprintf(tmp_str, "(%ld%%)",
                    (100L * view->page_bot_pos) / view->file_size);
            if (prompt_str[0] != '\0')
                strncat(prompt_str, " ", MAXLEN - 1);
            strncat(prompt_str, tmp_str, MAXLEN - 1);
        }
    }
    if (view->f_eod) { /* End           */
        if (prompt_str[0] != '\0')
            strncat(prompt_str, " ", MAXLEN - 1);
        strncat(prompt_str, "(End)", MAXLEN - 1);
        if (view->curr_argc + 1 < view->argc) {
            sprintf(tmp_str, " Next File: %s", view->argv[view->curr_argc + 1]);
            strncat(prompt_str, tmp_str, MAXLEN - 1);
        }
    }
    view->f_new_file = false;
}

/*
╭───────────────────────────────────────────────────────────────────╮
│ CAT_FILE                                                          │
╰───────────────────────────────────────────────────────────────────╯ */
void cat_file(View *view) {
    int c;

    while (1) {
        get_next_char();
        if (view->f_eod)
            break;
        putchar(c);
    }
}

/*
╭───────────────────────────────────────────────────────────────────╮
│ LP (PRINT)                                                        │
╰───────────────────────────────────────────────────────────────────╯ */
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

/*
╭───────────────────────────────────────────────────────────────────╮
│ GO_TO_MARK                                                        │
╰───────────────────────────────────────────────────────────────────╯ */
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

/*
╭───────────────────────────────────────────────────────────────────╮
│ GO_TO_EOF                                                         │
╰───────────────────────────────────────────────────────────────────╯ */
void go_to_eof(View *view) {
    int c;
    long pos, buf_idx;
    unsigned int bytes_read;

    if (view->f_is_pipe) {
        cmd_line_prompt(view, "Seeking end of pipe");
        view->f_forward = true;
        get_next_char();
        while (!view->f_eod)
            get_next_char();
        get_prev_char();
        view->file_pos = view->buf_idx * VBUFSIZ +
                         (long)(view->buf_curr_ptr - view->buf) + 1;
    } else {
        pos = ftell(view->fp);
        buf_idx = ((pos - 1) / VBUFSIZ);
        view->buf_idx = ((view->file_size - 1) / VBUFSIZ);
        if (view->buf_idx != buf_idx) {
            fseek(view->fp, (long)(view->buf_idx * VBUFSIZ), SEEK_SET);
            bytes_read = fread(view->buf, 1, VBUFSIZ, view->fp);
            if (bytes_read < VBUFSIZ) {
                if (!feof(view->fp)) {
                    pos = (view->buf_idx * VBUFSIZ) +
                          (long)(view->buf_curr_ptr - view->buf);
                    sprintf(tmp_str, "Read error at position %ld", pos);
                    display_error_message(tmp_str);
                    abend(-1, tmp_str);
                }
            }
        }
        view->buf_curr_ptr = view->buf + bytes_read - 1;
        view->buf_end_ptr = view->buf + bytes_read;
        view->page_top_pos =
            view->buf_idx * VBUFSIZ + (long)(view->buf_end_ptr - view->buf);
    }
    prev_page(view);
}

/*
╭───────────────────────────────────────────────────────────────────╮
│ GO_TO_LINE                                                        │
╰───────────────────────────────────────────────────────────────────╯ */
int go_to_line(View *view, int LineNumber) {
    int c;
    int LineCnt;

    if (LineNumber <= 1) {
        go_to_position(view, (long)0);
        return EOF;
    }
    load_buffer(view, (long)0);
    view->f_forward = true;
    get_next_char();
    if (view->f_eod) {
        display_error_message("End of data");
        return EOF;
    }
    if (c == '\r') {
        get_next_char();
        if (view->f_eod) {
            display_error_message("End of data");
            return EOF;
        }
    }
    for (LineCnt = 1; LineCnt < LineNumber; LineCnt++) {
        while (c != '\n') {
            get_next_char();
            if (view->f_eod) {
                sprintf(tmp_str, "End of data at %d lines", LineCnt - 1);
                display_error_message(tmp_str);
                return EOF;
            }
        }
        get_next_char();
    }
    go_to_position(view, view->buf_idx * VBUFSIZ +
                             (long)(view->buf_curr_ptr - view->buf));
    return 0;
}
/*
╭───────────────────────────────────────────────────────────────────╮
│ GO_TO_PERCENT                                                     │
╰───────────────────────────────────────────────────────────────────╯ */
void go_to_percent(View *view, int Percent) {
    int c;

    if (view->file_size < 0) {
        display_error_message("Cannot determine file length");
        return;
    }
    view->file_pos = ((long)Percent * view->file_size) / 100L;
    load_buffer(view, view->file_pos);
    view->f_forward = true;
    get_next_char();
    while (c != '\n') {
        get_prev_char();
        if (view->f_bod)
            break;
    }
    get_next_char();
    view->file_pos =
        view->buf_idx * VBUFSIZ + (long)(view->buf_curr_ptr - view->buf);
    go_to_position(view, view->file_pos);
}
/*
╭───────────────────────────────────────────────────────────────────╮
│ GO_TO_POSITION                                                    │
╰───────────────────────────────────────────────────────────────────╯ */
void go_to_position(View *view, long go_to_pos) {
    load_buffer(view, go_to_pos);
    view->f_forward = true;
    next_page(view);
}
/*
╭───────────────────────────────────────────────────────────────────╮
│ SEARCH                                                            │
╰───────────────────────────────────────────────────────────────────╯ */
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
    /*
    ╭───────────────────────────────────────────────────────────────────╮
    │ SEARCH - COMPILE REGULAR EXPRESSION                               │
    ╰───────────────────────────────────────────────────────────────────╯ */
    if (view->f_ignore_case)
        REG_FLAGS = REG_ICASE;
    else
        REG_FLAGS = 0;
    reti = regcomp(&compiled_regex, regex_pattern, REG_FLAGS);
    if (reti) {
        display_error_message("Invalid pattern");
        return false;
    }
    /*
    ╭───────────────────────────────────────────────────────────────────╮
    │ SEARCH - READ LINES                                               │
    ╰───────────────────────────────────────────────────────────────────╯ */
    while (1) {
#ifdef DEBUG
        snprintf(tmp_str, MAXLEN - 1, "Pos %ld of %ld, %s for: ", srch_curr_pos,
                 view->file_size, (search_cmd == '/') ? "Forward" : "Backward");
        strncat(tmp_str, regex_pattern, MAXLEN - strlen(tmp_str) - 1);
        cmd_line_prompt(view, tmp_str);
        rc = prefresh(view->win, view->pminrow, view->pmincol, view->sminrow,
                      view->smincol, view->smaxrow, view->smaxcol);
        if (rc == ERR) {
            display_error_message("Error refreshing screen");
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
            load_buffer(view, srch_curr_pos);
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
            load_buffer(view, srch_curr_pos);
            srch_curr_pos = get_prev_line(view, srch_curr_pos);
        }
        format_output_line(view);
        /*
        ╭───────────────────────────────────────────────────────────╮
        │ SEARCH - CURRENT LINE                                     │
        ╰───────────────────────────────────────────────────────────╯ */
        reti = regexec(&compiled_regex, view->line_out_s,
                       compiled_regex.re_nsub + 1, pmatch, REG_FLAGS);
        /*
        ╭─────────────────────────────────────────────────────────╮
        │ SEARCH - NO MATCH - DISPLAY LINE IF PAGING              │
        ╰─────────────────────────────────────────────────────────╯ */
        if (reti == REG_NOMATCH) {
            if (f_page) {
                display_output_line(view);
                if (view->cury == view->scroll_lines)
                    break;
            }
            continue;
        }
        /*
        ╭───────────────────────────────────────────────────────────╮
        │ SEARCH - ERROR                                            │
        ╰───────────────────────────────────────────────────────────╯ */
        if (reti) {
            char err_str[MAXLEN];
            regerror(reti, &compiled_regex, err_str, sizeof(err_str));
            strcpy(tmp_str, "Regex match failed: ");
            strcat(tmp_str, err_str);
            display_error_message(tmp_str);
            regfree(&compiled_regex);
            return false;
        }
        /*
        ╭───────────────────────────────────────────────────────────╮
        │ SEARCH - MATCH - DISPLAY LEADING CONTEXT LINES            │
        ╰───────────────────────────────────────────────────────────╯ */
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
            header_lines = i;
            if (header_lines < 1)
                header_lines = 1;
            for (i = 0; i < header_lines; i++) {
                srch_curr_pos = get_next_line(view, srch_curr_pos);
                format_output_line(view);
                display_output_line(view);
            }
            f_page = true;
        } else {
            /*
            ╭──────────────────────────────────────────────────────────────╮
            │ SEARCH - CONTINUE HIGHLIGHTING MATCHING LINES ON SAME PAGE   │
            ╰──────────────────────────────────────────────────────────────╯ */
            display_output_line(view);
            cury = view->cury;
        }
        /*
        ╭───────────────────────────────────────────────────────────────╮
        │ SEARCH - HIGHLIGHT ALL MATCHES ON CURRENT LINE                │
        ╰───────────────────────────────────────────────────────────────╯*/
        view->first_match_x = -1;
        view->last_match_x = 0;
        line_len = strlen(view->line_out_s);
        line_offset = 0;
        while (1) {
            /*
            ╭───────────────────────────────────────────────────────╮
            │ SEARCH - HIGHLIGHT MATCH                              │
            ╰───────────────────────────────────────────────────────╯*/
            view->curx = line_offset + pmatch[0].rm_so;
            match_len = pmatch[0].rm_eo - pmatch[0].rm_so;
            mvwchgat(view->win, view->cury - 1, view->curx, match_len,
                     A_REVERSE, CP_NORM, NULL);
            rc =
                prefresh(view->win, view->pminrow, view->pmincol, view->sminrow,
                         view->smincol, view->smaxrow, view->smaxcol);
            if (rc == ERR) {
                display_error_message("Error refreshing screen");
            }
            /*
            ╭───────────────────────────────────────────────────────╮
            │ SEARCH - UPDATE LINE MATCH COLUMNS                    │
            ╰───────────────────────────────────────────────────────╯*/
            if (view->first_match_x == -1)
                view->first_match_x = pmatch[0].rm_so;
            view->last_match_x = line_offset + pmatch[0].rm_eo;
            /*
            ╭───────────────────────────────────────────────────────╮
            │ SEARCH - TRACK LINE_OFFSET                            │
            ╰───────────────────────────────────────────────────────╯*/
            line_offset += pmatch[0].rm_eo;
            if (line_offset >= line_len)
                break;
            view->line_out_p = view->line_out_s + line_offset;
            /*
            ╭───────────────────────────────────────────────────────╮
            │ SEARCH - CONTINUE HIGHLIGHTING SAME LINE              │
            ╰───────────────────────────────────────────────────────╯*/
            reti = regexec(&compiled_regex, view->line_out_p,
                           compiled_regex.re_nsub + 1, pmatch, REG_FLAGS);
            if (reti == REG_NOMATCH)
                break;
            if (reti) {
                char msgbuf[100];
                regerror(reti, &compiled_regex, msgbuf, sizeof(msgbuf));
                sprintf(tmp_str, "Regex match failed: %s", msgbuf);
                display_error_message(tmp_str);
                regfree(&compiled_regex);
                return false;
            }
            if (view->cury == view->scroll_lines) {
                regfree(&compiled_regex);
                return true;
            }
        }
    }
    /*
    ╭───────────────────────────────────────────────────────╮
    │ SEARCH - SUCCESS - PREPARE PROMPT                     │
    ╰───────────────────────────────────────────────────────╯*/
    snprintf(view->tmp_prompt_str, MAXLEN - 1,
             "%c%s Match at Cols %d - %d, Cols %d - %d showing", search_cmd,
             regex_pattern, view->first_match_x, view->last_match_x,
             view->pmincol, view->smaxcol - view->begx);
    return true;
}

/*
╭───────────────────────────────────────────────────────╮
│ NEXT_PAGE                                             │
╰───────────────────────────────────────────────────────╯*/
void next_page(View *view) {
    int i;
    view->maxcol = 0;
    view->f_forward = true;
    view->cury = 0;
    wmove(view->win, view->cury, 0);
    view->page_top_pos = view->page_bot_pos;
    for (i = 0; i < view->scroll_lines; i++) {
        view->page_bot_pos = get_next_line(view, view->page_bot_pos);
        if (view->f_eod)
            break;
        format_output_line(view);
        display_output_line(view);
    }
    wclrtobot(view->win);
}

/*
╭───────────────────────────────────────────────────────╮
│ PREV_PAGE                                             │
╰───────────────────────────────────────────────────────╯*/
void prev_page(View *view) {
    int i;
    if (view->page_top_pos == (long)0)
        return;
    view->maxcol = 0;
    view->f_forward = false;
    view->cury = 0;
    wmove(view->win, view->cury, 0);
    view->page_bot_pos = view->page_top_pos;
    for (i = 0; i < view->scroll_lines; i++) {
        view->page_top_pos = get_pos_prev_line(view, view->page_top_pos);
        if (view->page_top_pos == (long)0)
            break;
    }
    view->page_bot_pos = view->page_top_pos;
    next_page(view);
}

/*
╭───────────────────────────────────────────────────────╮
│ SCROLL_FORWARD_N_LINES                                │
╰───────────────────────────────────────────────────────╯*/
void scroll_down_n_lines(View *view, int n) {
    int i = 0;

    if (view->page_bot_pos == view->file_size)
        return;
    view->f_forward = true;

    // Locate New Top of Page
    load_buffer(view, view->page_top_pos);
    for (i = 0; i < n; i++) {
        view->page_top_pos = get_pos_next_line(view, view->page_top_pos);
        if (view->f_eod)
            break;
    }
    n = i;
    // Scroll
    wscrl(view->win, n);

    // Locate New Bottom of Page
    view->cury = view->scroll_lines - n;
    wmove(view->win, view->cury, 0);
    load_buffer(view, view->page_bot_pos);
    for (i = 0; i < n; i++) {
        view->page_bot_pos = get_next_line(view, view->page_bot_pos);
        if (view->f_eod)
            break;
        format_output_line(view);
        display_output_line(view);
    }
}

/*
╭───────────────────────────────────────────────────────╮
│ SCROLL_BACK_N_LINES                                   │
╰───────────────────────────────────────────────────────╯*/

void scroll_up_n_lines(View *view, int n) {
    int i;

    if (view->page_top_pos == (long)0)
        return;
    // Scroll Up "n" Lines
    // Locate New Top of Page
    load_buffer(view, view->page_top_pos);
    for (i = 0; i < n; i++) {
        if (view->page_top_pos == (long)0)
            break;
        view->page_top_pos = get_pos_prev_line(view, view->page_top_pos);
    }
    n = i;
    // Locate New Bottom of Page
    load_buffer(view, view->page_bot_pos);
    for (i = 0; i < n; i++) {
        view->page_bot_pos = get_pos_prev_line(view, view->page_bot_pos);
    }
    // Scroll Up
    if (n < view->scroll_lines)
        wscrl(view->win, -n);
    // Add New Top of Page
    view->cury = 0;
    wmove(view->win, view->cury, 0);
    load_buffer(view, view->page_top_pos);
    for (i = 0; i < n; i++) {
        get_next_line(view, view->page_top_pos);
        format_output_line(view);
        display_output_line(view);
    }
    return;
}

/*
╭───────────────────────────────────────────────────────╮
│ GET_NEXT_LINE                                         │
╰───────────────────────────────────────────────────────╯*/
long get_next_line(View *view, long pos) {
    uchar c;
    char *line_in_p;

    view->f_forward = true;
    // get_next_char();
    do {
        if (view->buf_curr_ptr == view->buf_end_ptr)
            c = advance_buffer(view);
        else
            c = *view->buf_curr_ptr++;
    } while (c == 0x0d);
    if (view->f_eod)
        return pos;
    line_in_p = view->line_in_s;
    view->line_in_beg_p = view->line_in_s;
    view->line_in_end_p = view->line_in_s + MAX_COLS;
    while (1) {
        if (c == (uchar)'\n')
            break;
        if (line_in_p >= view->line_in_end_p)
            break;
        *line_in_p++ = c;
        // get_next_char();
        do {
            if (view->buf_curr_ptr == view->buf_end_ptr)
                c = advance_buffer(view);
            else
                c = *view->buf_curr_ptr++;
        } while (c == 0x0d);
        if (view->f_eod)
            return pos;
    }
    *line_in_p = '\0';
    if (view->f_squeeze) {
        while (1) {
            get_next_char();
            if (view->f_eod)
                return pos;
            if (c != (uchar)'\n')
                break;
        }
        get_prev_char();
    }
    pos = (view->buf_idx * VBUFSIZ) + (long)(view->buf_curr_ptr - view->buf);
    return pos;
}

/*
╭───────────────────────────────────────────────────────╮
│ GET_PREV_LINE                                         │
╰───────────────────────────────────────────────────────╯*/
long get_prev_line(View *view, long pos) {
    uchar c;

    if (pos <= (long)0)
        return (NULL_POSITION);
    view->f_forward = false;
    get_prev_char();
    while ((uchar)c != '\n' && view->buf_curr_ptr > view->buf)
        get_prev_char();
    if (view->f_squeeze) {
        if ((uchar)c == '\n') {
            while (1) {
                if (view->buf_curr_ptr == view->buf)
                    break;
                get_prev_char();
                if ((uchar)c != '\n')
                    break;
            }
            get_next_char();
        }
    }
    while (1) {
        if ((uchar)c == '\n')
            break;
        if (view->buf_curr_ptr == view->buf)
            break;
        get_prev_char();
    }
    pos = view->buf_idx * VBUFSIZ + (long)(view->buf_curr_ptr - view->buf);
    if (pos < view->file_size)
        view->f_eod = false;
    return pos;
}

/*
╭───────────────────────────────────────────────────────╮
│ DISPLAY_OUTPUT_LINE                                   │
╰───────────────────────────────────────────────────────╯*/
void display_output_line(View *view) {
    int len;
    int rc;
    len = strlen(view->line_out_s);
    if (len > view->maxcol)
        view->maxcol = len;
    if (view->cury < 0)
        view->cury = 0;
    if (view->cury > view->scroll_lines)
        view->cury = view->scroll_lines;
    wmove(view->win, view->cury, 0);
    waddstr(view->win, view->line_out_s);
    wclrtoeol(view->win);
    view->cury++;
    rc = prefresh(view->win, view->pminrow, view->pmincol, view->sminrow,
                  view->smincol, view->smaxrow, view->smaxcol);
    if (rc == ERR) {
        display_error_message("Error refreshing screen");
    }
}

void prepend_top_line(View *view) {
    view->cury = 0;
    wscrl(view->win, -1);
    display_output_line(view);
    view->page_bot_pos = get_pos_prev_line(view, view->page_bot_pos);
}

void append_bot_line(View *view) {
    view->cury = view->scroll_lines;
    wscrl(view->win, 1);
    display_output_line(view);
    view->page_top_pos = get_pos_next_line(view, view->page_top_pos);
}
/*
╭───────────────────────────────────────────────────────╮
│ GET_POS_NEXT_LINE                                     │
╰───────────────────────────────────────────────────────╯*/
long get_pos_next_line(View *view, long pos) {
    uchar c;

    if (pos == view->file_size)
        return pos;
    view->f_forward = true;
    // get_next_char();
    do {
        if (view->buf_curr_ptr == view->buf_end_ptr)
            c = advance_buffer(view);
        else
            c = *view->buf_curr_ptr++;
    } while (c == 0x0d);
    if (view->f_eod)
        return pos;
    if (view->f_squeeze) {
        while (1) {
            if (c != '\n')
                break;
            get_next_char();
        }
        get_prev_char();
    }
    while (1) {
        if (c == '\n')
            break;
        // get_next_char();
        do {
            if (view->buf_curr_ptr == view->buf_end_ptr)
                c = advance_buffer(view);
            else
                c = *view->buf_curr_ptr++;
        } while (c == 0x0d);
        if (view->f_eod)
            return pos;
    }
    pos = (view->buf_idx * VBUFSIZ) + (long)(view->buf_curr_ptr - view->buf);
    return pos;
}

/*
╭───────────────────────────────────────────────────────╮
│ GET_POS_PREV_LINE                                     │
╰───────────────────────────────────────────────────────╯*/
long get_pos_prev_line(View *view, long pos) {
    uchar c;

    if (pos == (long)0)
        return pos;
    view->f_forward = false;
    get_prev_char();
    if (c == '\n')
        get_prev_char();
    while (1) {
        if (c == '\n') {
            get_next_char();
            break;
        }
        if (view->buf_curr_ptr == view->buf && view->buf_idx == 0)
            break;
        get_prev_char();
    }
    pos = (view->buf_idx * VBUFSIZ) + (long)(view->buf_curr_ptr - view->buf);
    if (pos < view->file_size)
        view->f_eod = false;
    return pos;
}

/*
╭───────────────────────────────────────────────────────╮
│ FORMAT_OUTPUT_LINE                                    │
╰───────────────────────────────────────────────────────╯*/
void format_output_line(View *view) {
    uchar *c;
    char *line_out_p;
    char *line_out_e = view->line_out_s + MAX_COLS - 1;
    int scr_column;
    wmove(view->win, view->cury, 0);
    scr_column = 0;
    line_out_p = view->line_out_s;
    for (c = (uchar *)view->line_in_s; *c != (uchar)'\0'; c++) {
        if (line_out_p >= line_out_e - 1)
            break;
        if (*c == (uchar)'\t') {
            do {
                *line_out_p++ = ' ';
                scr_column++;
            } while (scr_column % view->tab_stop != 0 &&
                     line_out_p < line_out_e - 1);
            continue;
        }
        *line_out_p++ = *c;
        scr_column++;
    }
    *line_out_p = '\0';
    c = (uchar *)line_out_p--;
    while (c > (uchar *)line_out_p) {
        if (--c != (uchar *)' ')
            break;
        scr_column--;
    }
    view->line_out_s[scr_column] = '\0';
    if (scr_column > view->maxcol)
        view->maxcol = scr_column;
}

/*
╭───────────────────────────────────────────────────────╮
│ ADVANCE_BUFFER                                        │
╰───────────────────────────────────────────────────────╯*/
int advance_buffer(View *view) {
    size_t bytes_read = 0;
    int c;
    long pos;

    view->f_bod = false;
    view->f_eod = false;
    if (view->f_forward) {
        if (view->buf_idx + 1 > view->buf_last) {
            view->f_eod = true;
            return (EOF);
        }
        view->buf_idx++;
        fseek(view->fp, (long)(view->buf_idx * VBUFSIZ), SEEK_SET);
        bytes_read = fread(view->buf, 1, VBUFSIZ, view->fp);
        if (bytes_read > 0)
            view->buf_end_ptr = view->buf + bytes_read;
        if (bytes_read < VBUFSIZ) {
            if (!feof(view->fp)) {
                pos = (view->buf_idx * VBUFSIZ) +
                      (long)(view->buf_curr_ptr - view->buf);
                sprintf(tmp_str, "Read error at position %ld", pos);
                display_error_message(tmp_str);
                abend(-1, tmp_str);
            }
        }
        view->buf_curr_ptr = view->buf;
        c = *view->buf_curr_ptr++;
    } else {
        if (view->buf_idx == 0)
            return (EOF);
        view->buf_idx--;
        fseek(view->fp, (long)(view->buf_idx * VBUFSIZ), SEEK_SET);
        bytes_read = fread(view->buf, 1, VBUFSIZ, view->fp);
        if (bytes_read > 0) {
            view->buf_end_ptr = view->buf + bytes_read;
            view->buf_curr_ptr = view->buf + bytes_read;
        }
        if (bytes_read < VBUFSIZ) {
            if (!feof(view->fp)) {
                pos = (view->buf_idx * VBUFSIZ) +
                      (long)(view->buf_curr_ptr - view->buf);
                sprintf(tmp_str, "Read error at position %ld", pos);
                display_error_message(tmp_str);
                abend(-1, tmp_str);
            }
        }
        view->buf_curr_ptr = view->buf_end_ptr;
        c = *--view->buf_curr_ptr;
    }
    return (c);
}

/*
╭───────────────────────────────────────────────────────╮
│ LOAD_BUFFER                                           │
╰───────────────────────────────────────────────────────╯*/
bool load_buffer(View *view, long pos) {
    view->f_eod = false;
    long bytes_read;
    long fp_pos = ftell(view->fp);
    long fp_idx = (fp_pos / VBUFSIZ) - 1;
    view->buf_curr_ptr = view->buf + (pos % VBUFSIZ);
    view->buf_idx = pos / VBUFSIZ;

    if (fp_idx != view->buf_idx) {
        fseek(view->fp, view->buf_idx * VBUFSIZ, SEEK_SET);
        bytes_read = fread(view->buf, sizeof(uchar), VBUFSIZ, view->fp);
        if (bytes_read > 0)
            view->buf_end_ptr = view->buf + bytes_read;
        if (bytes_read < VBUFSIZ) {
            if (!feof(view->fp)) {
                pos = (view->buf_idx * VBUFSIZ) +
                      (long)(view->buf_curr_ptr - view->buf);
                sprintf(tmp_str, "Read error at position %ld", pos);
                display_error_message(tmp_str);
                abend(-1, tmp_str);
            }
        }
    }
    return true;
}

/*
╭───────────────────────────────────────────────────────╮
│ CMD_LINE_PROMPT                                       │
╰───────────────────────────────────────────────────────╯*/
void cmd_line_prompt(View *view, char *s) {
    char message_str[MAX_COLS + 1];
    int l;

    l = strnz__cpy(message_str, s, MAX_COLS);
    wmove(view->win, view->cmd_line, 0);
    if (l != 0) {
        wattron(view->win, A_REVERSE);
        waddstr(view->win, " ");
        waddstr(view->win, message_str);
        waddstr(view->win, " ");
        wattroff(view->win, A_REVERSE);
        waddstr(view->win, " ");
        wclrtoeol(view->win);
        wmove(view->win, view->cmd_line, l + 2);
    }
    wclrtoeol(view->win);
}

/*
╭───────────────────────────────────────────────────────╮
│ REMOVE_FILE                                           │
╰───────────────────────────────────────────────────────╯*/
void remove_file(View *view) {
    char c;

    if (view->f_at_end_remove) {
        wmove(view->win, view->cmd_line, 0);
        waddstr(view->win, "Remove File (Y or N)->");
        wclrtoeol(view->win);
        c = (char)wgetch(view->win);
        waddch(view->win, (char)toupper(c));
        if (c == 'Y' || c == 'y') {
            fclose(view->fp);
            remove(view->cur_file_str);
        }
    }
}

/*
╭───────────────────────────────────────────────────────╮
│ VIEW_DISPLAY_HELP                                     │
╰───────────────────────────────────────────────────────╯ */
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
    mview(init, eargc, eargv, 10, 54, begy, begx);
}
