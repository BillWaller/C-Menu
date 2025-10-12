/* fview.c
 * File viewer for MENU
 * Bill Waller
 * billxwaller@gmail.com
 */
#include "view.h"
#include <ctype.h>
#include <fcntl.h>
#include <regex.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/* constants
------------------------------------------------------------------------------*/

#define FileFlag (O_RDONLY)

#define Ctrl(c) ((c) & 0x1f)
#define MAXLINE 1024
#define NMARKS 27

#define NULL_POSITION ((long)(-1))

#define MAXCHAR (unsigned char)0xfa
#define UL_START (unsigned char)0xfb
#define UL_END (unsigned char)0xfc
#define BO_START (unsigned char)0xfd
#define BO_END (unsigned char)0xfe

/*    got     expect       */
/*    -----   ---------    */
#define MO_NORMAL 0                /*                         */
#define MO_UL_EXPECT_CHR 1         /*            next char    */
#define MO_UL_GOT_CHR_EXPECT_BS 2  /*    char    ^H           */
#define MO_UL_GOT_BS_EXPECT_CHR 3  /*    ^H      next char    */
#define MO_BO_EXPECT_CHR 4         /*            next char    */
#define MO_BO_GOT_CHR_EXPECT_BS 5  /*    char    ^H           */
#define MO_BO_GOT_BS_EXPECT_SAME 6 /*    ^H      same char    */

/* macros
------------------------------------------------------------------------------*/

#define get_next_char()                                                        \
    do {                                                                       \
        if (++view->buf_ptr > view->buf_end)                                   \
            c = get_char_next_block();                                         \
        else                                                                   \
            c = *view->buf_ptr;                                                \
    } while (c == 0x0d);

#define GetPrevChar()                                                          \
    {                                                                          \
        view->f_eof = FALSE;                                                   \
        do {                                                                   \
            if (--view->buf_ptr < view->buf_start)                             \
                c = get_char_prev_block();                                     \
            else                                                               \
                c = *view->buf_ptr;                                            \
        } while (c == 0x0d);                                                   \
    }

/* function definitions
------------------------------------------------------------------------------*/

void init_view_struct();
int open_view_stdscr();
int open_view_win();
int view_file();
int initialize_file(char *);
int command_processor();
int get_cmd_char();
int get_cmd_str(char *);
void build_prompt(char, char *);
void cat_file();
void lp(char *, char *);
void go_to_mark(int);
void go_to_eof();
void go_to_line(int);
void go_to_percent(int);
void go_to_position(long);
void search(int, char *, int);
void read_forward_from_current_file_pos(int);
void read_forward_from_file_pos(int, long);
long read_line_forward(long);
long read_line_forward_raw(long);
void read_backward_from_current_file_pos(int);
void read_backward_from_file_pos(int, long);
long read_line_backward(long);
long read_line_backward_raw(long);
int initialize_buffers(int);
int get_char_next_block();
int get_char_prev_block();
int get_char_buffer();
int locate_byte_pipe(long);
void put_line();
void display_error_msg(char *);
void command_line_prompt(char *);
void remove_file();
void view_display_help();

view_ *view;
char err_msg[MAXLEN];

/* code
------------------------------------------------------------------------------*/

void init_view_struct() {
    char *d;

    view = (view_ *)malloc(sizeof(view_));
    if (view == NULL) {
        sprintf(err_msg, "fview.c,line 111,malloc(%d) failed\n",
                (int)sizeof(view_));
        abend(-1, err_msg);
    }
    option->bo_color = 0;
    view->prev_file_pos = NULL_POSITION;
    view->tabstop = 8;
    view->blk_first = NULL;
    view->tabstop = 8;
    view->curx = 0;
    view->cury = 0;
    view->line_mode = 0;
    view->max_col = 0;
    view->next_c = 0;
    view->ptop = 0;
    view->pbot = 0;
    view->f_beg_of_file = TRUE;
    view->f_eof = FALSE;
    view->f_forward = TRUE;
    view->f_is_pipe = FALSE;
    view->f_new_file = TRUE;
    view->f_pipe_processed = FALSE;
    view->f_redraw_screen = TRUE;
    view->curr_argc = 0;
    view->first_column = 0;
    view->last_column = 0;
    view->last_line = 0;
    view->f_displaying_help = FALSE;
    view->f_stdout_is_tty = FALSE;
    view->fd = 0;
    view->cmd_str[0] = '\0';
    view->cur_file_str[0] = '\0';
    view->def_prompt_ptr = NULL;
    view->file_spec_ptr = NULL;
    view->next_file_spec_ptr = NULL;
    view->tmp_file_name_ptr = NULL;
    view->title = NULL;
    view->line_str[0] = '\0';
    view->line_out_str[0] = '\0';
    view->line_start_ptr = NULL;
    view->line_end_ptr = NULL;
    view->buf_start = NULL;
    view->buf_ptr = NULL;
    view->buf_end = NULL;
    view->f_at_end_clear = FALSE;
    view->f_ignore_case = TRUE;
    view->f_squeeze = TRUE;
    view->f_at_end_remove = FALSE;
    view->prompt_type = 'L';
    view->startup_cmd_str[0] = '\0';
    view->startup_cmd_str_all_files[0] = '\0';
    d = getenv("VIEW");
    if (d == NULL || *d == '\0')
        d = VIEWARGS;
    strncpy(view->arg_str, d, MAXLEN);
    view->argc = str_to_args(view->argv, view->arg_str);
}

int open_view_stdscr() {
    open_curses();

    if (view->lines < 0 || view->lines > LINES)
        view->lines = LINES;
    if (view->cols < 0 || view->cols > COLS)
        view->cols = COLS;
    if (view->lines == 0 && view->cols == 0) {
        view->lines = LINES;
        view->cols = COLS;
        view->win = stdscr;
        view->scroll_lines = view->lines - 1;
        view->last_line = view->scroll_lines - 1;
        view->cmd_line = view->lines - 1;
        view->first_column = 0;
        view->last_column = view->cols;
        wsetscrreg(view->win, 0, view->last_line);
        scrollok(view->win, FALSE);
        idcok(view->win, TRUE);
    } else
        open_view_win();
    return (0);
}

int open_view_win() {
    if (win_new(view->lines, view->cols, view->begy, view->begx, NULL)) {
        sprintf(tmp_str, "win_new(%d, %d, %d, %d, %s) failed", view->lines,
                view->cols, view->begy, view->begx, "NULL");
        display_error_message(tmp_str);
        return (1);
    }
    view->win = win_win[win_ptr];
    view->box = win_box[win_ptr];
    view->scroll_lines = view->lines - 1;
    view->last_line = view->scroll_lines - 1;
    view->cmd_line = view->lines - 1;
    view->first_column = 0;
    view->last_column = view->cols;
    wsetscrreg(view->win, 0, view->last_line);
    scrollok(view->win, FALSE);
    keypad(view->win, TRUE);
    idcok(view->win, FALSE);
    return (0);
}

int view_file() {
    if (view->argc < 1) {
        view->curr_argc = -1;
        view->argc = 0;
        view->argv[0] = "-";
    }
    view->next_file_spec_ptr = view->argv[0];
    while (view->curr_argc < view->argc) {
        if (view->next_file_spec_ptr == NULL ||
            *view->next_file_spec_ptr == '\0')
            break;
        view->file_spec_ptr = view->next_file_spec_ptr;
        view->next_file_spec_ptr = NULL;
        if (initialize_file(view->file_spec_ptr)) {
            if (view->fd >= 0)
                command_processor();
        } else {
            view->curr_argc++;
            if (view->curr_argc < view->argc)
                view->next_file_spec_ptr = view->argv[view->curr_argc];
        }
    }
    if (view->fd != 0) {
        close(view->fd);
        remove_file();
    }
    return (0);
}

int initialize_file(char *file_name) {
    struct stat StatStruct;
    int Tmpfd;
    int i;

    if (strcmp(file_name, "-") == 0) {
        if (view->f_pipe_processed) {
            display_error_message("Unable to view pipe twice");
            return (0);
        }
        Tmpfd = 0;
        view->f_is_pipe = TRUE;
    } else if ((Tmpfd = open(file_name, FileFlag)) >= 0) {
        view->f_is_pipe = FALSE;
        view->f_pipe_processed = FALSE;
        view->prev_file_pos = NULL_POSITION;
    } else {
        strncpy(tmp_str, "Unable to open ", MAXLEN);
        strncat(tmp_str, file_name, MAXLEN);
        display_error_message(tmp_str);
        return (0);
    }
    if (isatty(Tmpfd)) {
        display_error_message("No input file and no pipe input");
        return (0);
    }
    view->f_new_file = TRUE;
    if (view->fd >= 0)
        close(view->fd);
    view->fd = Tmpfd;
    if (view->f_is_pipe) {
        view->f_pipe_processed = TRUE;
        view->size_bytes = 0L;
        if (initialize_buffers(PIPEBUFS))
            return (0);
    } else {
        if (stat(file_name, &StatStruct) == -1) {
            strncpy(tmp_str, "Unable to stat ", MAXLEN);
            strncat(tmp_str, file_name, MAXLEN);
            display_error_message(tmp_str);
            return (0);
        }
        view->file_pos = lseek(view->fd, 0L, SEEK_END);
        view->size_bytes = StatStruct.st_size;
        if (view->file_pos != view->size_bytes) {
            sprintf(tmp_str, "Error seeking end of file: pos %ld, size %ld",
                    view->file_pos, view->size_bytes);
            display_error_message(tmp_str);
            return (0);
        }
        if (view->size_bytes == 0L) {
            strncpy(tmp_str, file_name, MAXLEN);
            strncat(tmp_str, " is empty", MAXLEN);
            display_error_message(tmp_str);
            return (0);
        }
        if (initialize_buffers(FILEBUFS))
            return (0);
    }
    view->last_blk_no = -1;
    if (view->startup_cmd_str_all_files[0] != '\0')
        strncpy(view->startup_cmd_str, view->startup_cmd_str_all_files, MAXLEN);
    for (i = 0; i < NMARKS; i++)
        view->mark_tbl[i] = NULL_POSITION;
    strncpy(view->cur_file_str, file_name, MAXLEN);
    if (view->f_stdout_is_tty) {
        for (i = 0; i < NPOS; i++)
            view->pos_tbl[i] = NULL_POSITION;
        view->ptop = 0;
        view->pbot = view->scroll_lines;
        if (view->prev_file_pos != NULL_POSITION) {
            go_to_position(view->prev_file_pos);
            view->prev_file_pos = NULL_POSITION;
        }
    }
    return (1);
}

int command_processor() {
    int fd;
    char tmpnam[] = "/tmp/view-XXXXXX";
    int c = 0;
    int PrevsearchCmd = 0;
    int n;
    char *EditorPtr;
    char PromptStr[MAXLEN];
    char shell_cmd_str[MAXLEN];

    if (view->startup_cmd_str[0] != '\0') {
        view->next_c = view->startup_cmd_str[0];
        strnz_cpy(view->cmd_str, (char *)&view->startup_cmd_str[1], MAXLEN);
    } else
        view->cmd_str[0] = '\0';

    if (setjmp(view->cmd_jmp))
        view->next_c = 0;

    while (1) {
        c = view->next_c;
        view->next_c = 0;
        if (!c) {
            if (view->pos_tbl[view->ptop] == NULL_POSITION)
                go_to_position((long)0);
            else if (view->f_redraw_screen) {
                read_forward_from_file_pos(view->scroll_lines,
                                           view->pos_tbl[view->ptop]);
                view->f_redraw_screen = FALSE;
            }
            build_prompt(view->prompt_type, (char *)&PromptStr);
            if (PromptStr[0] == '\0')
                command_line_prompt("");
            else
                command_line_prompt(PromptStr);
            c = get_cmd_char();
            if (c >= '0' && c <= '9') {
                tmp_str[0] = (char)c;
                tmp_str[1] = '\0';
                c = get_cmd_str(tmp_str);
            }
        }
        switch (c) {

        case KEY_LEFT:
        case KEY_BACKSPACE:
        case Ctrl('H'):
            n = atoi(view->cmd_str);
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
            read_forward_from_file_pos(view->scroll_lines,
                                       view->pos_tbl[view->ptop]);
            break;

        case KEY_RIGHT:
        case 'l':
        case 'L':
            n = atoi(view->cmd_str);
            if (n <= 0)
                n = 1;
            if ((view->first_column + view->cols) < view->max_col)
                view->first_column += n;
            else
                view->first_column = 0;
            view->last_column = view->first_column + view->cols;
            read_forward_from_file_pos(view->scroll_lines,
                                       view->pos_tbl[view->ptop]);
            break;

        case KEY_UP:
        case 'k':
        case 'K':
        case Ctrl('K'):
            n = atoi(view->cmd_str);
            if (n <= 0)
                n = 1;
            read_backward_from_current_file_pos(n);
            break;

        case KEY_DOWN:
        case KEY_ENTER:
        case '\n':
        case '\r':
        case ' ':
        case 'j':
        case 'J':
            n = atoi(view->cmd_str);
            if (n <= 0)
                n = 1;
            read_forward_from_current_file_pos(n);
            /*   idlok(view->win, TRUE); */
            break;

        case KEY_PPAGE:
        case 'b':
        case 'B':
        case Ctrl('B'):
            n = view->scroll_lines;
            read_backward_from_current_file_pos(n);
            break;

        case KEY_NPAGE:
        case 'f':
        case 'F':
        case Ctrl('F'):
            n = view->scroll_lines;
            read_forward_from_current_file_pos(n);
            break;

        case KEY_HOME:
        case 'g':
            view->first_column = 0;
            view->last_column = view->cols;
            go_to_line(1);
            break;

        case KEY_LL:
            go_to_eof();
            break;

        case '!':
            if (view->f_displaying_help)
                break;
            if (get_cmd_str("!") == 0) {
                if (!view->f_is_pipe) {
                    view->prev_file_pos = view->pos_tbl[view->ptop];
                    close(view->fd);
                    view->fd = -1;
                    view->next_file_spec_ptr = view->file_spec_ptr;
                    str_subc(shell_cmd_str, view->cmd_str, '%',
                             view->cur_file_str, MAXLEN);
                } else
                    strcpy(shell_cmd_str, view->cmd_str);
                full_screen_shell(shell_cmd_str);
                if (!view->f_is_pipe) {
                    view->next_file_spec_ptr = view->cur_file_str;
                    return (0);
                }
            }
            break;

        case '+':
            if (get_cmd_str("Startup Command:") == 0)
                strncpy(view->startup_cmd_str_all_files, view->cmd_str, MAXLEN);
            break;

        case '-':
            if (view->f_displaying_help)
                break;
            command_line_prompt("(C, I, P, S, T, or H for Help)->");
            c = get_cmd_char();
            c = tolower(c);
            switch (c) {
            case 'c':
                command_line_prompt("Clear Screen at End (Y or N)->");
                if ((c = get_cmd_char()) == 'y' || c == 'Y')
                    view->f_at_end_clear = TRUE;
                else if (c == 'n' || c == 'N')
                    view->f_at_end_clear = FALSE;
                break;
            case 'i':
                command_line_prompt("Ignore Case in search (Y or N)->");
                if ((c = get_cmd_char()) == 'y' || c == 'Y')
                    view->f_ignore_case = TRUE;
                else if (c == 'n' || c == 'N')
                    view->f_ignore_case = FALSE;
                break;
            case 'p':
                command_line_prompt("(Short Long or No prompt)->");
                c = tolower(get_cmd_char());
                switch (c) {
                case 's':
                    view->prompt_type = 'S';
                    break;
                case 'l':
                    view->prompt_type = 'L';
                    break;
                case 'n':
                    view->prompt_type = 'N';
                    break;
                default:
                    break;
                }
                break;
            case 's':
                command_line_prompt(
                    "view->f_squeeze Multiple Blank lines (Y or N)->");
                if ((c = get_cmd_char()) == 'y' || c == 'Y')
                    view->f_squeeze = TRUE;
                else if (c == 'n' || c == 'N')
                    view->f_squeeze = FALSE;
                break;
            case 't':
                sprintf(tmp_str, "Tabstop Colums Currently %d:", view->tabstop);
                n = 0;
                if (get_cmd_str(tmp_str) == 0)
                    n = atoi(view->cmd_str);
                if (n >= 1 && n <= 12) {
                    view->tabstop = n;
                    view->f_redraw_screen = TRUE;
                } else
                    display_error_message("Tab stops not changed");
                break;
            case 'h':
                if (!view->f_displaying_help)
                    view_display_help();
                view->next_c = '-';
                break;
            default:
                break;
            }
            break;

        case ':':
            view->next_c = get_cmd_str(":");
            break;

        case '/':
        case '?':
            n = atoi(view->cmd_str);
            if (n < 1 || n > 9)
                n = 1;
            tmp_str[0] = (char)c;
            tmp_str[1] = '\0';
            if (get_cmd_str(tmp_str) == 0) {
                search(c, view->cmd_str, n);
                PrevsearchCmd = c;
            }
            break;

        case '=':
        case Ctrl('G'):
            build_prompt(2, (char *)&PromptStr);
            break;

        case '@':
            break;

        case 'd':
        case 'D':
        case Ctrl('D'):
            n = atoi(view->cmd_str);
            if (n < 1)
                n = 10;
            read_forward_from_current_file_pos(n);
            break;

        case 'o':
        case 'O':
        case 'e':
        case 'E':
            if (get_cmd_str("File name:") == 0) {
                strtok(view->cmd_str, " ");
                view->next_file_spec_ptr = strdup(view->cmd_str);
                view->f_redraw_screen = TRUE;
                return (0);
            }
            break;

        case 'G':
            n = atoi(view->cmd_str);
            if (n <= 0)
                go_to_eof();
            else
                go_to_line(n);
            break;

        case 'h':
        case 'H':
            if (!view->f_displaying_help)
                view_display_help();
            break;

        case 'm':
            command_line_prompt("Mark label (A-Z)->");
            c = get_cmd_char();
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
            command_line_prompt("Goto mark (A-Z)->");
            c = get_cmd_char();
            if (c == '@' || c == KEY_F(9) || c == '\033')
                break;
            c = tolower(c);
            if (c < 'a' || c > 'z')
                display_error_message("Not (A-Z)");
            else
                go_to_mark(c);
            break;

        case 'n':
            n = atoi(view->cmd_str);
            if (n < 1 || n > 9)
                n = 1;
            search(PrevsearchCmd, NULL, n);
            break;

        case 'N':
            n = atoi(view->cmd_str);
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
            n = atoi(view->cmd_str);
            if (n < 0)
                go_to_line(1);
            if (n >= 100)
                go_to_eof();
            else
                go_to_percent(n);
            n = 0;
            break;

        case Ctrl('Z'):
            if (view->f_is_pipe) {
                display_error_message("Can't print standard input");
                break;
            }
            get_cmd_str("Enter Notation:");
            fd = mkstemp(tmpnam);
            strcpy(view->tmp_file_name_ptr, tmpnam);
            if (fd == -1) {
                display_error_message("Unable to create temporary file");
                break;
            }
            strncpy(shell_cmd_str, "echo ", MAXLEN - 5);
            strncat(shell_cmd_str, view->cmd_str, MAXLEN - 5);
            strncat(shell_cmd_str, view->tmp_file_name_ptr, MAXLEN - 5);
            shell(shell_cmd_str);
            strncpy(shell_cmd_str, "cat ", MAXLEN - 5);
            strncat(shell_cmd_str, view->cmd_str, MAXLEN - 5);
            strncat(shell_cmd_str, ">>", MAXLEN - 5);
            strncat(shell_cmd_str, view->tmp_file_name_ptr, MAXLEN - 5);
            shell(shell_cmd_str);
            lp(view->cur_file_str, view->cmd_str);
            wrefresh(view->win);
            shell(shell_cmd_str);
            snprintf(shell_cmd_str, (size_t)(MAXLEN - 5), "rm %s",
                     view->tmp_file_name_ptr);
            strncpy(shell_cmd_str, "rm ", MAXLEN - 5);
            strncat(shell_cmd_str, view->tmp_file_name_ptr, MAXLEN - 5);
            shell(shell_cmd_str);
            restore_wins();
            view->f_redraw_screen = TRUE;
            unlink(tmpnam);
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
            n = atoi(view->cmd_str);
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
            n = atoi(view->cmd_str);
            if (n < 1)
                n = 10;
            read_backward_from_current_file_pos(n);
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
            strncpy(shell_cmd_str, EditorPtr, MAXLEN - 5);
            strncat(shell_cmd_str, " ", MAXLEN - 5);
            strncat(shell_cmd_str, view->cur_file_str, MAXLEN - 5);
            full_screen_shell(shell_cmd_str);
            return (0);

        case 'V':
            display_error_message("View: Version 8.0");
            break;

        default:
            break;
        }
        view->cmd_str[0] = '\0';
    }
}

int get_cmd_char() {
    int c;

    wrefresh(view->win);
    /* idlok(view->win, FALSE); */
    c = wgetch(view->win);
    view->cmd_str[0] = '\0';
    return (c);
}

int get_cmd_str(char *MsgPtr) {
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
    if (view->cmd_str[0] != '\0')
        return (0);
    cmd_ptr = view->cmd_str;

    wmove(view->win, view->cmd_line, 0);
    if (view->cols < MAXLEN)
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
            if (cmd_ptr > view->cmd_str) {
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

void build_prompt(char build_prompt_type, char *prompt_ptr) {
    *prompt_ptr = '\0';
    if (build_prompt_type == 'L' || view->f_new_file ||
        view->def_prompt_ptr == NULL || *view->def_prompt_ptr == '\0') {
        if (view->f_is_pipe)
            strncat(prompt_ptr, "stdin", MAXLEN);
        else
            strncat(prompt_ptr, view->cur_file_str, MAXLEN);
    }
    if (view->first_column > 0) {
        sprintf(tmp_str, "Col %d", view->first_column);
        if (*prompt_ptr != '\0')
            strncat(prompt_ptr, " ", MAXLEN);
        strncat(prompt_ptr, tmp_str, MAXLEN);
    }
    if (view->argc > 1 && (view->f_new_file || build_prompt_type == 'L')) {
        sprintf(tmp_str, "File %d of %d", view->curr_argc + 1, view->argc);
        if (*prompt_ptr != '\0')
            strncat(prompt_ptr, " ", MAXLEN);
        strncat(prompt_ptr, tmp_str, MAXLEN); /* File Of      */
    }
    if (build_prompt_type == 'L') { /* Byte of Byte  */
        view->file_pos = view->pos_tbl[view->pbot];
        if (view->file_pos == NULL_POSITION)
            view->file_pos = view->size_bytes;
        if (view->file_pos != NULL_POSITION) {
            sprintf(tmp_str, "Byte %ld", view->file_pos);
            if (*prompt_ptr != '\0')
                strncat(prompt_ptr, " ", MAXLEN);
            strncat(prompt_ptr, tmp_str, MAXLEN);
            if (view->size_bytes > 0) {
                sprintf(tmp_str, " of %ld", view->size_bytes);
                strncat(prompt_ptr, tmp_str, MAXLEN);
            }
        }
    }
    if (!view->f_eof && build_prompt_type != 'N') { /* Percent       */
        view->file_pos = view->pos_tbl[view->pbot];
        if (view->size_bytes > 0L && view->file_pos != NULL_POSITION) {
            sprintf(tmp_str, "(%ld%%)",
                    (100L * view->file_pos) / view->size_bytes);
            if (*prompt_ptr != '\0')
                strncat(prompt_ptr, " ", MAXLEN);
            strncat(prompt_ptr, tmp_str, MAXLEN);
        }
    }
    if (view->f_eof) { /* End           */
        if (*prompt_ptr != '\0')
            strncat(prompt_ptr, " ", MAXLEN);
        strncat(prompt_ptr, "(End)", MAXLEN);
        if (view->curr_argc + 1 < view->argc) {
            sprintf(tmp_str, " Next File: %s", view->argv[view->curr_argc + 1]);
            strncat(prompt_ptr, tmp_str, MAXLEN);
        }
    }
    if (view->def_prompt_ptr != NULL && *view->def_prompt_ptr != '\0') {
        if (*prompt_ptr != '\0')
            strncat(prompt_ptr, " ", MAXLEN);
        strncat(prompt_ptr, view->def_prompt_ptr, MAXLEN);
    }
    view->f_new_file = FALSE;
}

void cat_file() {
    register int c;

    while (1) {
        get_next_char();
        if (view->f_eof)
            break;
        putchar(c);
    }
}

void lp(char *PrintFile, char *Notation) {
    char *print_cmd_ptr;
    char shell_cmd_str[MAXLEN];

    print_cmd_ptr = getenv("PRINTCMD");
    if (print_cmd_ptr == NULL || *print_cmd_ptr == '\0')
        print_cmd_ptr = PRINTCMD;
    sprintf(shell_cmd_str, "%s %s", print_cmd_ptr, PrintFile);
    command_line_prompt(shell_cmd_str);
    wrefresh(view->win);
    shell(shell_cmd_str);
}

void go_to_mark(int c) {
    if (c == '\'')
        view->file_pos = view->mark_tbl[(NMARKS - 1)];
    else
        view->file_pos = view->mark_tbl[c - 'a'];
    if (view->file_pos == NULL_POSITION)
        display_error_message("Mark not set");
    else
        go_to_position(view->file_pos);
}

void go_to_eof() {
    register int c;

    if (view->f_is_pipe) {
        command_line_prompt("Seeking end of pipe");
        view->f_forward = TRUE;
        c = get_char_buffer();
        while (!view->f_eof)
            get_next_char();
        GetPrevChar();
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
    read_backward_from_file_pos(view->scroll_lines, view->file_pos);
}

void go_to_line(int LineNumber) {
    block_struct *block_ptr;
    register int c;
    int LineCnt;

    if (view->f_is_pipe) {
        if (locate_byte_pipe((long)0) != 0) {
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
                go_to_position(view->blk_no * VBUFSIZ);
                display_error_message("Unable to reach beginning of file");
                return;
            }
        }
    } else {
        view->blk_no = (long)0;
        view->blk_offset = 0;
    }
    if (LineNumber <= 1) {
        go_to_position((long)0);
        return;
    }
    view->f_forward = TRUE;
    c = get_char_buffer();
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
    go_to_position(view->blk_no * VBUFSIZ +
                   (long)(view->buf_ptr - view->buf_start));
}

void go_to_percent(int Percent) {
    register int c;

    if (view->size_bytes == NULL_POSITION) {
        display_error_message("Don't know length of file");
        return;
    }
    view->file_pos = ((long)Percent * view->size_bytes) / 100L;
    if (view->f_is_pipe) {
        if (locate_byte_pipe(view->file_pos) != 0) {
            display_error_message("Unable to reach this position of file");
            return;
        }
    } else {
        view->blk_no = view->file_pos / VBUFSIZ;
        view->blk_offset = (int)(view->file_pos % VBUFSIZ);
    }
    view->f_forward = TRUE;
    c = get_char_buffer();
    while (c != '\n') {
        GetPrevChar();
        if (view->f_beg_of_file)
            break;
    }
    get_next_char();
    view->file_pos =
        view->blk_no * VBUFSIZ + (long)(view->buf_ptr - view->buf_start);
    go_to_position(view->file_pos);
}

void go_to_position(long GoToPos) {
    view->mark_tbl[(NMARKS - 1)] = view->pos_tbl[view->ptop];
    read_forward_from_file_pos(view->scroll_lines, GoToPos);
}

void search(int searchCmd, char *regex, int n) {
    static long line_pos;
    char f_wrapped = FALSE;
    regex_t preg;
    int REG_FLAGS = 0;
    int reti;

    if (searchCmd == '/')
        strcpy(tmp_str, "Forward");
    else
        strcpy(tmp_str, "Backward");
    command_line_prompt(tmp_str);
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
        if (searchCmd == '/')
            view->file_pos = (long)0;
        else {
            view->file_pos = view->size_bytes;
            f_wrapped = TRUE;
        }
    } else {
        if (searchCmd == '/') {
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
    if (searchCmd == '/') {
        line_pos = view->file_pos;
        view->file_pos = read_line_forward_raw(view->file_pos);
    } else
        line_pos = view->file_pos = read_line_backward_raw(view->file_pos);
    if (view->file_pos == NULL_POSITION) {
        display_error_message("Nothing to search");
        return;
    }
    reti = regexec(&preg, view->line_start_ptr, 0, NULL, 0);
    if (searchCmd == '/') {
        line_pos = view->file_pos;
        view->file_pos = read_line_forward_raw(view->file_pos);
    } else
        line_pos = view->file_pos = read_line_backward_raw(view->file_pos);
    if (view->file_pos == NULL_POSITION) {
        if (f_wrapped) {
            display_error_message("Pattern not found");
            return;
        } else {
            if (searchCmd == '/') {
                line_pos = view->file_pos = (long)0;
                view->file_pos = read_line_forward_raw(view->file_pos);
            } else {
                view->file_pos = view->size_bytes;
                line_pos = view->file_pos =
                    read_line_backward_raw(view->file_pos);
            }
            f_wrapped = TRUE;
        }
    }
    go_to_position(line_pos);
}

void read_forward_from_current_file_pos(int number_of_lines) {
    view->file_pos = view->pos_tbl[view->pbot];
    if (view->file_pos == NULL_POSITION || view->file_pos == view->size_bytes) {
        view->f_eof = TRUE;
        return;
    }
    read_forward_from_file_pos(number_of_lines, view->file_pos);
}

void read_forward_from_file_pos(int number_of_lines, long read_from_pos) {
    int DisplayLastScreenOnly = 0;

    if (number_of_lines >= view->scroll_lines) {
        view->cury = 0;
        view->max_col = 0;
        DisplayLastScreenOnly = 1;
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
        read_from_pos = read_line_forward(read_from_pos);
        if (view->f_eof) {
            if (number_of_lines >= view->scroll_lines) {
                go_to_eof();
                return;
            }
            view->line_start_ptr = view->line_str + 2;
            *view->line_start_ptr = '\0';
        }
        if (!DisplayLastScreenOnly || number_of_lines < view->scroll_lines) {
            view->pbot = view->ptop;
            if (view->ptop == view->scroll_lines)
                view->ptop = 0;
            else
                view->ptop++;
            view->pos_tbl[view->pbot] = read_from_pos;
            put_line();
        }
    }
    if (!view->f_eof) {
        read_from_pos = view->pos_tbl[view->pbot];
        if (read_from_pos == NULL_POSITION || read_from_pos == view->size_bytes)
            view->f_eof = TRUE;
    }
}

long read_line_forward(long current_file_pos) {
    register int c;
    register char *dst_line_ptr;

    if (current_file_pos == NULL_POSITION)
        return (NULL_POSITION);
    if (view->f_is_pipe) {
        if (locate_byte_pipe(current_file_pos) != 0)
            return (NULL_POSITION);
    } else {
        view->blk_no = current_file_pos / VBUFSIZ;
        view->blk_offset = (int)(current_file_pos % VBUFSIZ);
    }
    view->f_forward = TRUE;
    c = get_char_buffer();
    if (c == '\r')
        get_next_char();
    if (view->f_eof)
        return (NULL_POSITION);

    view->line_start_ptr = dst_line_ptr = view->line_str + 2;
    view->line_end_ptr = view->line_str + MAXLINE - 20;
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
        if (dst_line_ptr >= view->line_end_ptr)
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
        GetPrevChar();
    }
    if (view->f_eof)
        GetPrevChar();
    return (view->blk_no * VBUFSIZ + (long)(view->buf_ptr - view->buf_start) +
            1);
}

long read_line_forward_raw(long current_file_pos) {
    register int c;
    register char *dst_line_ptr;
    char *DstLineEnd;

    if (current_file_pos == NULL_POSITION)
        return (NULL_POSITION);
    if (view->f_is_pipe) {
        if (locate_byte_pipe(current_file_pos) != 0)
            return (NULL_POSITION);
    } else {
        view->blk_no = current_file_pos / VBUFSIZ;
        view->blk_offset = (int)(current_file_pos % VBUFSIZ);
    }
    view->f_forward = TRUE;
    c = get_char_buffer();
    if (c == '\r')
        get_next_char();
    if (view->f_eof)
        return (NULL_POSITION);
    view->line_start_ptr = dst_line_ptr = view->line_str + 2;
    view->line_end_ptr = view->line_str + MAXLINE - 20;
    DstLineEnd = dst_line_ptr + MAXLINE - 1;
    while (1) {
        if (c == '\n' || view->f_eof || dst_line_ptr >= DstLineEnd)
            break;
        *dst_line_ptr++ = (char)c;
        if (dst_line_ptr >= view->line_end_ptr)
            break;
        get_next_char();
    }
    *dst_line_ptr = '\0';
    if (view->f_eof) {
        GetPrevChar();
        return (NULL_POSITION);
    }
    return (view->blk_no * VBUFSIZ + (long)(view->buf_ptr - view->buf_start) +
            1);
}

void read_backward_from_current_file_pos(int number_of_lines) {
    view->file_pos = view->pos_tbl[view->ptop];
    if (view->file_pos == NULL_POSITION || view->file_pos == (long)0)
        return;
    read_backward_from_file_pos(number_of_lines, view->file_pos);
}

void read_backward_from_file_pos(int number_of_lines, long read_from_pos) {
    int DisplayLastScreenOnly = 0;

    if (number_of_lines >= view->scroll_lines) {
        DisplayLastScreenOnly = 1;
        view->max_col = 0;
    }
    while (--number_of_lines >= 0) {
        view->prev_file_pos = read_from_pos;
        read_from_pos = read_line_backward(read_from_pos);
        if (read_from_pos == NULL_POSITION) {
            if (view->f_is_pipe)
                read_forward_from_file_pos(view->scroll_lines,
                                           view->prev_file_pos);
            else
                read_forward_from_file_pos(view->scroll_lines, (long)0);
            return;
        }
        if (number_of_lines < view->scroll_lines) {
            if (!DisplayLastScreenOnly) {
                wmove(view->win, 0, 0);
                view->cury = 0;
                winsertln(view->win);
            } else
                view->cury = number_of_lines;
            put_line();
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

long read_line_backward(long current_file_pos) {
    register int c;
    register char *dst_line_ptr;
    long NewPos, LineStartPos;
    int i;

    if (current_file_pos == NULL_POSITION || current_file_pos <= (long)0)
        return (NULL_POSITION);
    if (view->f_is_pipe) {
        if (locate_byte_pipe(current_file_pos - 1) != 0)
            return (NULL_POSITION);
    } else {
        view->blk_no = (current_file_pos - 1) / VBUFSIZ;
        view->blk_offset = (int)((current_file_pos - 1) % VBUFSIZ);
    }
    view->f_forward = FALSE;
    c = get_char_buffer();
    if (c == '\r')
        GetPrevChar();
    if (view->f_squeeze) {
        if (c == '\n') {
            i = 0;
            while (1) {
                GetPrevChar();
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
        GetPrevChar();
        if (c == '\n' || view->f_beg_of_file)
            break;
    }
    NewPos =
        view->blk_no * VBUFSIZ + (long)(view->buf_ptr - view->buf_start) + 1;
    LineStartPos = NewPos;
    view->line_start_ptr = dst_line_ptr = view->line_str + 2;
    view->line_end_ptr = view->line_str + MAXLINE - 12;
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
        if (dst_line_ptr >= view->line_end_ptr)
            break;
    } while (NewPos < current_file_pos);
    *dst_line_ptr++ = '\0';
    return (LineStartPos);
}

long read_line_backward_raw(long current_file_pos) {
    register int c;
    register char *dst_line_ptr;

    if (current_file_pos == NULL_POSITION || current_file_pos <= (long)0)
        return (NULL_POSITION);
    if (view->f_is_pipe) {
        if (locate_byte_pipe(current_file_pos - 1) != 0)
            return (NULL_POSITION);
    } else {
        view->blk_no = (current_file_pos - 1) / VBUFSIZ;
        view->blk_offset = (int)((current_file_pos - 1) % VBUFSIZ);
    }
    dst_line_ptr = view->line_str + MAXLINE;
    view->line_start_ptr = view->line_str + 2;
    *--dst_line_ptr = '\0';
    view->f_forward = FALSE;
    c = get_char_buffer();
    if (c == '\r')
        get_next_char();
    while (1) {
        GetPrevChar();
        if (c == '\n' || view->f_beg_of_file ||
            dst_line_ptr <= view->line_start_ptr)
            break;
        *--dst_line_ptr = (char)c;
    }
    view->line_start_ptr = dst_line_ptr;
    return (view->blk_no * VBUFSIZ + (long)(view->buf_ptr - view->buf_start) +
            1);
}

int initialize_buffers(int number_of_buffers) {
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
        remove_file();
        return (1);
    }
    block_ptr = view->blk_first;
    for (i = 0; i < number_of_buffers; i++)
        (block_ptr++)->block_no = (long)(-1);
    view->blk_last = (--block_ptr);
    view->blk_curr = view->blk_last;
    return (0);
}

int get_char_next_block() {
    view->blk_offset = 0;
    if (view->blk_no == view->last_blk_no) {
        view->f_eof = TRUE;
        return (EOF);
    }
    view->blk_no++;
    view->f_forward = TRUE;
    return (get_char_buffer());
}

int get_char_prev_block() {
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
    return (get_char_buffer());
}

int get_char_buffer() {
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

int locate_byte_pipe(long locate_file_pos) {
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

void put_line() {
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
                } while ((pl_column % view->tabstop) != 0);
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
    waddstr(view->win, view->line_out_str);
    /* if (pl_column <= view->last_column)
           if ((pl_column - view->first_column) < view->cols) */
    wclrtoeol(view->win);
    if ((unsigned char)*c == '\0') {
        while (c > view->line_start_ptr) {
            if ((unsigned char)*--c != ' ')
                break;
            pl_column--;
        }
        /* if (pl_column > view->max_col)
               view->max_col = pl_column; */
    }
    if (view->cury < view->scroll_lines)
        view->cury++;
    else
        view->cury = view->scroll_lines;
}

void display_error_msg(char *s) {
    char message_str[MAX_COLS + 1];
    char *d, *e;

    d = message_str;
    e = d + view->cols - 4;
    while (*s != '\0' && d < e) {
        *d++ = *s++;
    }
    *d = '\0';
    command_line_prompt(message_str);
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
    view->cmd_str[0] = '\0';
}

void command_line_prompt(char *s) {
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

void remove_file() {
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
                strncpy(tmp_str, "Can't open ", MAXLEN);
                strncat(tmp_str, view->cur_file_str, MAXLEN);
                display_error_message(tmp_str);
            } else
                close(view->fd);
        }
    }
}

void view_display_help() {
    view_ *view_save;
    int begy, begx;
    int eargc;
    char *eargv[MAXARGS];

    if (strcmp(view->file_spec_ptr, HELPFILE) == 0)
        return;
    eargv[0] = VIEWHELPCMD;
    eargv[1] = HELPFILE;
    eargv[2] = NULL;
    eargc = 2;
    begx = view->begx + 4;
    begy = view->begy + 1;
    view_save = view;
    view = (view_ *)0;
    mview(eargc, eargv, 10, 54, begy, begx, NULL, win_attr);
    view = view_save;
}
