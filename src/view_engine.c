//  view_engine.c
//  Bill Waller Copyright (c) 2025
//  MIT License
/// Command Line Start-up for C-Menu Menu

#include "menu.h"
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

/// Macros to get next and previous character skipping over carriage returns
/// leaving the file position cursor at the next character to be read.
/// @note When reading forward, if the file position cursor (FPC) reaches
///       the end of data (EOD), the EOD flag is set, preventing further
///       forward movement of the FPC.
/// @note When reading backward, if the file position cursor (FPC) reaches
///       the beginning of data (BOD), the BOD flag is set, preventing
///       further backward movement of the FPC.
/// @note Once either flag, EOD OR BOD, is set, no more characters will be
///       read until the FPC moves or the direction of reading changes.
/// @note C-Menu View mapps input to the Kernel's demand paged virtual
///       address space, so the entire file is accessible via a memory
///       pointer. One of the reasons C-Menu View is so responsive, is
///       that it doesn't make you wait while it loads data you will
///       never use.
/// @note This is called lazy loading, and for a pager like C-Menu View,
///       it is lean and performant.
/// The character read is returned in variable 'c'.

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
bool search(View *, int, char *, bool);
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
bool ansi_to_cmplx();
void parse_ansi_str(char *, attr_t *, int *);
void view_display_help(Init *);
void cmd_line_prompt(View *, char *);
void remove_file(View *);

int a_toi(char *s, bool a_toi_error);
bool a_toi_error;

char err_msg[MAXLEN];

///  ╭──────────────────────────────────────────────────────────────╮
///  │ VIEW_FILE                                                    │
///  ╰──────────────────────────────────────────────────────────────╯
/// Main entry point for C-Menu View
/// @param init Pointer to Init structure
/// @return 0 on success
/// @note This function processes each file specified in the command
///       line arguments. If no files are specified, it defaults to
///       reading from standard input ("-").
/// @note For each file, it initializes the viewing context, displays
///       the first page, and enters the command processing loop.
/// @note After processing each file, it cleans up resources before
///       moving to the next file.
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
                view->page_top_pos = 0;
                view->page_bot_pos = 0;
                view->file_pos = 0;
                next_page(view);
                view_cmd_processor(init);
                munmap(view->buf, view->file_size);
                // close(view->in_fd);
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
/// ╭───────────────────────────────────────────────────────────────╮
/// │ VIEW_CMD_PROCESSOR                                            │
/// ╰───────────────────────────────────────────────────────────────╯
/// Command processing loop for C-Menu View
/// @param init Pointer to Init structure
/// @return 0 on exit command
/// @note This function handles user commands for navigating and
/// manipulating the viewed file. It processes commands such as
/// scrolling, searching, jumping to specific lines, and executing
/// shell commands.
/// @note The function maintains the state of the view and updates
/// the display as needed based on user input.
int view_cmd_processor(Init *init) {
    int tfd;
    char tmp_str[MAXLEN];
    char earg_str[MAXLEN];
    char *eargv[MAX_ARGS];
    int eargc;
    int begy, begx;
    int c;
    int max_n;
    int shift = 0;
    int search_cmd = 0;
    int prev_search_cmd = 0;
    int rc, i;
    int n = 0;
    int l = 0;
    ssize_t bytes_written;
    char *editor_ptr;
    char shell_cmd_spec[MAXLEN];
    struct timespec start, end;
    double elapsed = 0;
    bool f_clock_started = false;
    off_t n_cmd = 0L;
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
            build_prompt(view, view->prompt_type, view->prompt_str, elapsed);
            if (view->prompt_str[0] == '\0')
                cmd_line_prompt(view, "");
            else if (view->tmp_prompt_str[0] == '\0')
                cmd_line_prompt(view, view->prompt_str);
            else
                cmd_line_prompt(view, view->tmp_prompt_str);
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
        /// Horizontal Scrolling Commands
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
            /// Resize Command
            /// This command handles terminal resize events, which would
            /// otherwise disrupt the display of the viewed file.
        case Ctrl('R'):
        case KEY_RESIZE:
            resize_page(init);
            break;
            /// Cursor Movement Commands
            /// We have tried to leave these commands consistent with
            /// other popular pagers, including h, j, k, and l.
            /// @note Vertical scrolling can also be accomplished with the
            ///       mouse wheel if the terminal supports it.
            /// 'h' or Left Arrow or Backspace or Ctrl('H') to scroll left
        case Ctrl('H'):
        case 'h':
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
        case KEY_RIGHT:
        case 'l':
        case 'L':
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
            /// Up Arrow, k, K, Ctrl('K')
            ///
        case KEY_UP:
        case 'k':
        case 'K':
        case Ctrl('K'):
            if (n_cmd <= 0)
                n_cmd = 1;
            scroll_up_n_lines(view, n_cmd);
            break;
            /// Down Arrow, j, J, Enter, Space, KEY_DOWN, KEY_ENTER
            ///
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
            /// Page Scrolling Commands
            /// Previous Page, b, B, Ctrl('B'), KEY_PPAGE
        case KEY_PPAGE:
        case 'b':
        case 'B':
        case Ctrl('B'):
            scroll_up_n_lines(view, view->scroll_lines);
            break;
            /// Page Scrolling Commands
            /// Next Page, f, F, Ctrl('F'), KEY_NPAGE
        case KEY_NPAGE:
        case 'f':
        case 'F':
        case Ctrl('F'):
            next_page(view);
            break;
            /// Go to Beginning of Document
        case KEY_HOME:
        case 'g':
            view->pmincol = 0;
            go_to_line(view, 0L);
            break;
            /// KEY_LL lower left key (home down)
        case KEY_LL:
            go_to_eof(view);
            break;
            /// Execute Shell Command from within C-Menu View
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
            /// The "+" command allows executing a startup command
            /// -c Clear Screen at End
            /// -i Ignore Case in Search
            /// -p Prompt Type (Short, Long, None)
            /// -s Squeeze Multiple Blank Linesee
            /// -t Tab Stop Columns
            /// -h Help Display
        case '+':
            if (get_cmd_arg(view, "Startup Command:") == 0)
                strnz__cpy(view->cmd, view->cmd_arg, MAXLEN - 1);
            break;
            /// Settings Commands
        case '-':
            if (view->f_displaying_help)
                break;
            cmd_line_prompt(view, "(C, I, P, S, T, or H for Help)->");
            c = get_cmd_char(view, &n_cmd);
            c = tolower(c);
            if (c >= 'A' && c <= 'Z')
                c += ' ';
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
                /// Help Command 'H'
                /// Displays help information about the settings commands.
                /// Lower case 'h' is reserved for horizontal scrolling.'
            case 'h':
            case 'H':
                if (!view->f_displaying_help)
                    view_display_help(init);
                view->next_cmd_char = '-';
                break;
            default:
                break;
            }
            break;
            /// Other Commands
            /// Set a off_t prompt string
        case ':':
            view->next_cmd_char = get_cmd_arg(view, ":");
            break;
            /// Search Commands
            /// Search forward '/' or backward '?'
        case '/':
        case '?':
            strnz__cpy(tmp_str, (c == '/') ? "(forward)->" : "(backward)->",
                       MAXLEN - 1);
            search_cmd = c;
            c = get_cmd_arg(view, tmp_str);
            if (c == '\n') {
                view->f_wrap = false;
                search(view, search_cmd, view->cmd_arg, false);
                prev_search_cmd = search_cmd;
                strnz__cpy(prev_regex_pattern, view->cmd_arg, MAXLEN - 1);
                view->srch_beg_pos = view->page_top_pos;
            }
            break;
            /// Open File Command
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
            /// Go to End of Document or Specific Line
            /// @param n_cmd Line number to go to, if 0 go to EOF
        case KEY_END:
        case 'G':
            if (n_cmd <= 0)
                go_to_eof(view);
            else
                go_to_line(view, n_cmd);
            break;
            /// Help Command
        case KEY_F(1):
            if (!view->f_displaying_help)
                view_display_help(init);
            break;
            /// Set Mark (feature may be removed in the future)
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
            /// Go to mark (feature may be removed in the future)
        case 'M':
        case '\'':
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
            /// Repeat Previous Search
        case 'n':
            if (prev_search_cmd == 0) {
                Perror("No previous search");
                break;
            }
            if (view->page_bot_pos == view->file_size) {
                view->page_top_pos = 0;
                view->page_bot_pos = 0;
            }
            search(view, prev_search_cmd, prev_regex_pattern, true);
            break;
            /// Close Current and Open Next File
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
            /// Go to Percent of File
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
            /// Send File to Print Queue, with Notation
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
            /// Print Current File
        case Ctrl('P'):
        case KEY_CATAB:
        case KEY_PRINT:
            lp(view->cur_file_str);
            view->f_redisplay_page = true;
            break;
            /// Close Current and Open Previous File
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
            /// Quit Command
        case 'q':
        case 'Q':
        case KEY_F(9):
        case '\033':
            view->curr_argc = view->argc;
            view->next_file_spec_ptr = NULL;
            return 0;
            /// Open Current File in Editor
        case 'v':
            if (view->f_displaying_help)
                break;
            if (view->f_is_pipe) {
                strnz__cpy(em0,
                           "View doesn't support editing of standard input",
                           MAXLEN - 1);
                strnz__cpy(
                    em1, "You may write the data to a file and edit that file",
                    MAXLEN - 1);
                strnz__cpy(em2, "use the w command to initiate the write",
                           MAXLEN - 1);
                strnz__cpy(em3, "and try again.", MAXLEN - 1);
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
        case 'w':
            /// Write to File
            /// @note This feature writes the file being viewed to a
            /// specified output file.
            /// @note The user is presented with a Form to enter the
            /// filename.
            /// @note View then duplicates the contents of the file or pipe
            /// being viewed into the specified output file.
            /// @note This command is useful for saving the contents of the
            /// viewed file to a new location, especially when the user
            /// wants to load piped input into an editor.
            /// @note Error handling is included to manage issues with file
            /// operations, such as opening or writing to the output file.
            /// @note Upon successful writing, a confirmation message is
            /// displayed to the user.
            /// @note After writing, the view is refreshed to reflect any
            /// changes.
            strnz__cpy(earg_str,
                       "form -d filename.f -o \"~/menuapp/data/form-out\"",
                       MAXLEN - 1);
            eargc = str_to_args(eargv, earg_str, MAX_ARGS);
            zero_opt_args(init);
            parse_opt_args(init, eargc, eargv);
            begy = view->begy + view->lines - 7;
            begx = 4;
            rc = init_form(init, eargc, eargv, begy, begx);
            if (rc == P_CANCEL) {
                destroy_form(init);
                view->f_redisplay_page = true;
                break;
            }
            strnz__cpy(tmp_str, "~/menuapp/data/form-out", MAXLEN - 1);
            verify_spec_arg(view->in_spec, tmp_str, "~/menuapp/data", ".",
                            R_OK);
            view->in_fp = fopen(view->in_spec, "r");
            if (view->in_fp == NULL) {
                ssnprintf(em0, MAXLEN - 1, "%s, line: %d", __FILE__,
                          __LINE__ - 2);
                strnz__cpy(em1, "fopen ", MAXLEN - 1);
                strnz__cat(em1, view->in_spec, MAXLEN - 1);
                strerror_r(errno, em2, MAXLEN - 1);
                display_error(em0, em1, em2, NULL);
                return (1);
            }
            fgets(tmp_str, MAXLEN - 1, view->in_fp);
            fclose(view->in_fp);
            l = strlen(tmp_str);
            if (l > 0 && tmp_str[l - 1] == '\n')
                tmp_str[l - 1] = '\0';
            view->f_out_spec = verify_spec_arg(
                view->out_spec, tmp_str, "~/menuapp/data", ".", W_OK | S_QUIET);
            view->out_fd =
                open(view->out_spec, O_CREAT | O_TRUNC | O_WRONLY, 0644);
            bytes_written = write(view->out_fd, view->buf, view->file_size);
            if (bytes_written != (ssize_t)view->file_size) {
                ssnprintf(em0, MAXLEN - 1, "%s, line: %d", __FILE__,
                          __LINE__ - 2);
                strnz__cpy(em1, "fwrite ", MAXLEN - 1);
                strnz__cat(em1, view->out_spec, MAXLEN - 1);
                strerror_r(errno, em2, MAXLEN - 1);
                strnz__cpy(em3, "Not all bytes written", MAXLEN - 1);
                display_error(em0, em1, em2, em3);
                return (1);
            }
            close(view->out_fd);
            ssnprintf(tmp_str, MAXLEN - 1, "Wrote %jd bytes to %s",
                      bytes_written, view->out_spec);
            cmd_line_prompt(view, tmp_str);
            view->f_redisplay_page = true;
            break;
        case CT_VIEW:
            break;
            /// Version Information
        case 'V':
            Perror("View: Version 8.0");
            break;
        default:
            break;
        }
        view->cmd_arg[0] = '\0';
    }
}
///  ╭──────────────────────────────────────────────────────────────╮
///  │ GET_CMD_CHAR                                                 │
///  ╰──────────────────────────────────────────────────────────────╯
///  Get Command Character
///  Returns command character and numeric argument in n
///  Returns MA_ENTER_OPTION on mouse event
///  note Handles Mouse Wheel Up and Down as KEY_UP and KEY_DOWN
///  @param view Pointer to View structure
///  @param n Pointer to off_t to store numeric argument
int get_cmd_char(View *view, off_t *n) {
    int c = 0, i = 0;
    char cmd_str[33];
    cmd_str[0] = '\0';
    MEVENT event;
    /// BUTTON4 and BUTTON5 are typically used for mouse wheel up and down
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
///  ╭──────────────────────────────────────────────────────────────╮
///  │ GET_CMD_ARG                                                  │
///  ╰──────────────────────────────────────────────────────────────╯
///  Get Argument String from View's Command Line
///  Returns 0 on Enter, @, F9, ESC, or Mouse Event
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
///  ╭──────────────────────────────────────────────────────────────╮
///  │ BUILD_PROMPT                                                 │
///  ╰──────────────────────────────────────────────────────────────╯
///  Build Prompt String
///  @param view Pointer to View structure
///  @param prompt_type Prompt Type (PT_SHORT, PT_LONG, PT_NONE)
///  @param prompt_str Pointer to character array to store prompt
///  @param elapsed Elapsed time in seconds for timer display
void build_prompt(View *view, int prompt_type, char *prompt_str,
                  double elapsed) {
    prompt_type = PT_LONG;
    prompt_str[0] = '\0';
    strnz__cpy(prompt_str, "", MAXLEN - 1);
    if (prompt_type == PT_LONG || view->f_new_file) {
        if (view->f_is_pipe)
            strnz__cat(prompt_str, "stdin", MAXLEN - 1);
        else
            strnz__cat(prompt_str, view->cur_file_str, MAXLEN - 1);
    }
    if (view->pmincol > 0) {
        sprintf(tmp_str, "Col %d of %d", view->pmincol, view->maxcol);
        if (prompt_str[0] != '\0')
            strnz__cat(prompt_str, "|", MAXLEN - 1);
        strnz__cat(prompt_str, tmp_str, MAXLEN - 1);
    }
    if (view->argc > 1 && (view->f_new_file || prompt_type == PT_LONG)) {
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
///  ╭──────────────────────────────────────────────────────────────╮
///  │ CAT_FILE                                                     │
///  ╰──────────────────────────────────────────────────────────────╯
///  Concatenate File to Standard Output
void cat_file(View *view) {
    int c;
    while (1) {
        get_next_char();
        if (view->f_eod)
            break;
        putchar(c);
    }
}
///  ╭──────────────────────────────────────────────────────────────╮
///  │ LP (PRINT)                                                   │
///  ╰──────────────────────────────────────────────────────────────╯
///  Send File to Print Queue
void lp(char *PrintFile) {
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
///  ╭──────────────────────────────────────────────────────────────╮
///  │ GO_TO_MARK                                                   │
///  ╰──────────────────────────────────────────────────────────────╯
///  Go to Mark
///  @param c Mark Character ('a' to 'z' or '\'')
///  Marks have been disabled and may be removed in future versions
///  of View.
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
///  ╭──────────────────────────────────────────────────────────────╮
///  │ GO_TO_EOF                                                    │
///  ╰──────────────────────────────────────────────────────────────╯
/// Go to End of File
void go_to_eof(View *view) {
    int c;
    view->file_pos = view->file_size;
    view->page_top_pos = view->file_pos;
    get_prev_char();
    prev_page(view);
}
///  ╭──────────────────────────────────────────────────────────────╮
///  │ GO_TO_LINE                                                   │
///  ╰──────────────────────────────────────────────────────────────╯
///  Go to Specific Line
///  @param line_idx Line Number to Go To (1 based)
///  Returns EOF on error
///  Unlike less, C-Menu View does not automatically keep track of
///  line numbers in the file being viewed. Therefore, to go to a
///  specific line, View must read through the file from the beginning
///  up to the specified line number. This approach can be less
///  efficient for large files, but it simplifies the implementation
///  and avoids the need for maintaining a line index.
int go_to_line(View *view, off_t line_idx) {
    int c = 0;
    off_t line_cnt = 0;
    if (line_idx <= 1) {
        go_to_position(view, 0);
        return EOF;
    }
    view->f_forward = true;
    view->file_pos = 0;
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
///  ╭──────────────────────────────────────────────────────────────╮
///  │ GO_TO_PERCENT                                                │
///  ╰──────────────────────────────────────────────────────────────╯
///  Go to Percent of File
void go_to_percent(View *view, int Percent) {
    int c = 0;
    if (view->file_size < 0) {
        Perror("Cannot determine file length");
        return;
    }
    view->file_pos = (Percent * view->file_size) / 100;
    view->f_forward = true;
    get_next_char();
    while (c != '\n') {
        get_prev_char();
        if (view->f_bod)
            break;
    }
    get_next_char();
    next_page(view);
}
///  ╭──────────────────────────────────────────────────────────────╮
///  │ GO_TO_POSITION                                               │
///  ╰──────────────────────────────────────────────────────────────╯
///  Go to Specific File Position
///  @param go_to_pos File Position to Go To
///  Locates the nearest line starting at or after the specified
///  file position and displays the page starting from that line.
void go_to_position(View *view, off_t go_to_pos) {
    view->f_forward = true;
    view->file_pos = go_to_pos;
    view->page_bot_pos = view->file_pos;
    next_page(view);
}
///  ╭──────────────────────────────────────────────────────────────╮
///  │ SEARCH                                                       │
///  ╰──────────────────────────────────────────────────────────────╯
///  Search for Regular Expression Pattern
///  Supports extended regular expressions.
///  @param view Pointer to View structure
///  @param search_cmd Search Command ('/' for forward, '?' for backward)
///  @param regex_pattern Regular Expression Pattern to Search For
///  @param repeat true to repeat previous search from current position
///  Returns true if match found, false if no match or error
///  Each time a match is found, the matching line along with
///  some leading context lines are displayed. The search continues
///  until the entire file has been searched. If the search reaches
///  the end (or beginning) of the file without finding a new match,
///  searching continues until it wraps around to the starting position.
///  If no new matches are found after wrapping, a message
///  is displayed indicating that the search is complete.
///  All matches are highlighted, including those that are beyond
///  the right margin of the display. An indication of the starting
///  and ending colums of the match is shown in the command line prompt,
///  so that if a match is not visible on the screen, the user can
///  still determine where the match occurred and scroll horizontally to view
///  it.
bool search(View *view, int search_cmd, char *regex_pattern, bool repeat) {
    int REG_FLAGS = 0;
    regmatch_t pmatch[1];
    regex_t compiled_regex;
    int reti;
    int line_offset;
    int line_len;
    int match_len;
    int cury = 0;
    off_t srch_curr_pos;
    bool f_page = false;
    srch_curr_pos = view->page_top_pos;
    view->srch_beg_pos = -1;
    if (repeat) {
        if (search_cmd == '/')
            srch_curr_pos = view->page_bot_pos;
    }
    /// ╭───────────────────────────────────────────────────────────╮
    /// │ SEARCH - COMPILE REGULAR EXPRESSION                       │
    /// ╰───────────────────────────────────────────────────────────╯
    if (view->f_ignore_case)
        REG_FLAGS = REG_ICASE | REG_EXTENDED;
    else
        REG_FLAGS = REG_EXTENDED;
    reti = regcomp(&compiled_regex, regex_pattern, REG_FLAGS);
    if (reti) {
        Perror("Invalid pattern");
        return false;
    }
    /// ╭───────────────────────────────────────────────────────────╮
    /// │ SEARCH - READ LINES                                       │
    /// ╰───────────────────────────────────────────────────────────╯
    while (1) {
        if (search_cmd == '/') {
            if (srch_curr_pos == view->file_size && view->srch_beg_pos == 0)
                srch_curr_pos = 0;
            if (srch_curr_pos == view->srch_beg_pos) {
                if (!view->f_wrap) {
                    view->f_wrap = true;
                } else {
                    ssnprintf(tmp_str, MAXLEN - 1,
                              "Search complete: %zd bytes for %s",
                              view->file_size, regex_pattern);
                    cmd_line_prompt(view, tmp_str);
                    regfree(&compiled_regex);
                    return false;
                }
            }
            if (cury == view->scroll_lines)
                return true;
            if (!view->f_wrap)
                view->srch_beg_pos = srch_curr_pos;
            srch_curr_pos = get_next_line(view, srch_curr_pos);
            view->page_bot_pos = srch_curr_pos;
        } else {
            if (srch_curr_pos == 0 && view->srch_beg_pos == view->file_size)
                srch_curr_pos = view->file_size;
            if (srch_curr_pos == view->srch_beg_pos) {
                if (!view->f_wrap) {
                    view->f_wrap = true;
                } else {
                    ssnprintf(tmp_str, MAXLEN - 1,
                              "Search complete: %ld bytes for %s",
                              view->file_size, regex_pattern);
                    cmd_line_prompt(view, tmp_str);
                    regfree(&compiled_regex);
                    return false;
                }
            }
            srch_curr_pos = get_prev_line(view, srch_curr_pos);
            view->page_top_pos = srch_curr_pos;
        }
        /// ╭───────────────────────────────────────────────────────╮
        /// │ PROCESS LINES                                         │
        /// ╰───────────────────────────────────────────────────────╯
        fmt_line(view);
        /// ╭───────────────────────────────────────────────────────╮
        /// │ SEARCH - CURRENT LINE                                 │
        /// ╰───────────────────────────────────────────────────────╯
        /// Perform extended regular expression matching. ANSI
        /// sequences and Unicode characters are stripped before
        /// matching, so matching corresponds to the visual
        /// display of the line.
        reti = regexec(&compiled_regex, view->stripped_line_out,
                       compiled_regex.re_nsub + 1, pmatch, REG_FLAGS);
        /// ╭───────────────────────────────────────────────────────╮
        /// │ SEARCH - NO MATCH - DISPLAY LINE IF PAGING            │
        /// ╰───────────────────────────────────────────────────────╯
        /// Once a match is found, leading context lines are
        /// displayed at the top of the page. After that, all
        /// subsequent lines without matches are displayed until
        /// the page is full.
        if (reti == REG_NOMATCH) {
            if (f_page) {
                display_line(view);
                if (view->cury == view->scroll_lines)
                    break;
            }
            continue;
        }
        /// ╭───────────────────────────────────────────────────────╮
        /// │ SEARCH - ERROR                                        │
        /// ╰───────────────────────────────────────────────────────╯
        if (reti) {
            char err_str[MAXLEN];
            regerror(reti, &compiled_regex, err_str, sizeof(err_str));
            strnz__cpy(tmp_str, "Regex match failed: ", MAXLEN - 1);
            strnz__cat(tmp_str, err_str, MAXLEN - 1);
            Perror(tmp_str);
            regfree(&compiled_regex);
            return false;
        }
        if (!f_page) {
            view->f_forward = true;
            view->cury = 0;
            wmove(view->win, view->cury, 0);
            wclrtobot(view->win);
            view->page_top_pos = srch_curr_pos;
            f_page = true;
        }
        /// ╭───────────────────────────────────────────────────╮
        /// │ SEARCH - CONTINUE HIGHLIGHTING MATCHED LINES      │
        /// ╰───────────────────────────────────────────────────╯
        /// Search continues displaying matches until the page
        /// is full.
        display_line(view);
        //----------------------------------------------
        cury = view->cury;
        /// ╭───────────────────────────────────────────────────────╮
        /// │ SEARCH - HIGHLIGHT ALL MATCHES ON CURRENT LINE        │
        /// ╰───────────────────────────────────────────────────────╯
        /// All matches on the current line are highlighted,
        /// including those not displayed on the screen. Track
        /// first and last match columns for prompt display.
        //
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
                     WA_REVERSE, cp_norm, NULL);
            // rc =
            // prefresh(view->win, view->pminrow, view->pmincol,
            // view->sminrow, view->smincol, view->smaxrow,
            // view->smaxcol);
            // if (rc == ERR) {
            //     Perror("Error refreshing screen");
            // }
            /// ╭───────────────────────────────────────────────────╮
            /// │ SEARCH - UPDATE LINE MATCH COLUMNS                │
            /// ╰───────────────────────────────────────────────────╯
            if (view->first_match_x == -1)
                view->first_match_x = pmatch[0].rm_so;
            view->last_match_x = line_offset + pmatch[0].rm_eo;
            /// ╭───────────────────────────────────────────────────╮
            /// │ SEARCH - TRACK LINE_OFFSET                        │
            /// ╰───────────────────────────────────────────────────╯
            line_offset += pmatch[0].rm_eo;
            if (line_offset >= line_len)
                break;
            view->line_out_p = view->stripped_line_out + line_offset;
            /// ╭───────────────────────────────────────────────────╮
            /// │ SEARCH - CONTINUE HIGHLIGHTING SAME LINE          │
            /// ╰───────────────────────────────────────────────────╯
            reti = regexec(&compiled_regex, view->line_out_p,
                           compiled_regex.re_nsub + 1, pmatch, REG_FLAGS);
            /// @note lines may be much longer than the screen width, so
            /// continue searching even if the line is not displayed
            /// completely. The pad's complex characters (cchar_t) will
            /// handle the display for horizantal scrolling.
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
    /// ╭───────────────────────────────────────────────────────────╮
    /// │ SEARCH - SUCCESS - PREPARE PROMPT                         │
    /// ╰───────────────────────────────────────────────────────────╯
    /// Update view positions and prepare prompt string
    /// for match information
    view->file_pos = srch_curr_pos;
    view->page_bot_pos = srch_curr_pos;
    if (view->last_match_x > view->maxcol)
        ssnprintf(view->tmp_prompt_str, MAXLEN - 1,
                  "%c%s|Match Cols %d-%d of %d-%d|(%zd%%)", search_cmd,
                  regex_pattern, view->first_match_x, view->last_match_x,
                  view->pmincol, view->smaxcol - view->begx,
                  (view->page_bot_pos * 100 / view->file_size));
    else
        ssnprintf(view->tmp_prompt_str, MAXLEN - 1, "%c%s|Pos %zu-%zu|(%zd%%)",
                  search_cmd, regex_pattern, view->page_top_pos,
                  view->page_bot_pos,
                  (view->page_bot_pos * 100 / view->file_size));
    regfree(&compiled_regex);
    return true;
}
///  ╭──────────────────────────────────────────────────────────────╮
///  │ REDISPLAY_PAGE                                               │
///  ╰──────────────────────────────────────────────────────────────╯
/// Resize Page
/// This function adjusts the size of the viewing window based
/// on the current terminal dimensions and the view's settings.
/// If the view is set to full screen, it resizes to occupy
/// the entire terminal. Otherwise, it checks if the view's
/// dimensions exceed the terminal size and adjusts them accordingly.
/// If a resize occurs, it sets a flag to indicate that the page
/// needs to be redisplayed.
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
///  ╭──────────────────────────────────────────────────────────────╮
///  │ REDISPLAY_PAGE                                               │
///  ╰──────────────────────────────────────────────────────────────╯
///  Redisplay Current Page
///  @param view->page_top_pos is the top of page pointer
///  @param view->page_bot_pos is the bottom of page pointer
///  @param view->cury is the current line on the screen
///  @param view->maxcol is the off_test line on the page
///  @return void
///  @note Clears the screen and displays the current page of lines
///  starting from view->page_top_pos
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
///  ╭──────────────────────────────────────────────────────────────╮
///  │ NEXT_PAGE                                                    │
///  ╰──────────────────────────────────────────────────────────────╯
/// Advance to Next Page
/// @param view->file_pos is the file position pointer
/// @param view->page_top_pos is the top of page pointer
/// @param view->page_top_bot is the bottom of page pointer
/// @param view->cury is the current line on the screen
/// @param view->maxcol is the off_test line on the page
/// @param view->f_forward is the direction flag
/// @return void
/// @note Advances the view to the next page by reading lines from
/// view->page_bot_pos forward
/// @note Keeps track of view->page_top_pos, which will be needed for
/// KEY_UP and KEY_PPAGE
/// @note Clears the screen and displays the next page of lines

void next_page(View *view) {
    int i;
    int line_len;
    curs_set(0);
    if (view->page_bot_pos == view->file_size)
        return;
    view->maxcol = 0;
    view->f_forward = true;
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
///  ╭──────────────────────────────────────────────────────────────╮
///  │ PREV_PAGE                                                    │
///  ╰──────────────────────────────────────────────────────────────╯
/// Go to Previous Page
/// @param view->file_pos is the file position pointer
/// @param view->page_top_pos is the top of page pointer
/// @param view->page_top_bot is the bottom of page pointer
/// @param view->cury is the current line on the screen
/// @param view->maxcol is the off_test line on the page
/// @param view->f_forward is the direction flag
/// @return void
/// @note Moves the view to the previous page by reading lines backward
/// from view->page_top_pos
void prev_page(View *view) {
    int i;
    curs_set(0);
    if (view->page_top_pos == 0)
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
///  ╭──────────────────────────────────────────────────────────────╮
///  │ SCROLL_FORWARD_N_LINES                                       │
///  ╰──────────────────────────────────────────────────────────────╯
///  Scroll Forward N Lines
///  May be from 1 to scroll_lines - 1
///  Adjust Page Top and Bottom Pointers
///  Scroll Screen Up by N Lines
///  Fill in Page Bottom with N New Lines
///  @return void
void scroll_down_n_lines(View *view, int n) {
    int i = 0;
    int line_len;
    curs_set(0);
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
        line_len = fmt_line(view);
        if (line_len > view->maxcol)
            view->maxcol = line_len;
        display_line(view);
    }
    view->page_bot_pos = view->file_pos;
    curs_set(1);
}
///  ╭──────────────────────────────────────────────────────────────╮
///  │ SCROLL_BACK_N_LINES                                          │
///  ╰──────────────────────────────────────────────────────────────╯
///  Scroll Back N Lines
///  May be from 1 to scroll_lines - 1
///  Adjust Page Top and Bottom Pointers
///  Scroll Screen Down by N Lines
///  Fill in Page Top with N New Lines
///  @return void
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
///  ╭──────────────────────────────────────────────────────────────╮
///  │ GET_NEXT_LINE -> line_in_s                                   │
///  ╰──────────────────────────────────────────────────────────────╯
/// Get Next Line from File into line_in_s
/// @note the input may contain ANSI SGR sequences or Unicode,
/// which will be handled later during formatting for display
/// @note Carriage Return (0x0d) characters are skipped
/// @note Newline (0x0a) characters terminate the line
/// @note If view->f_squeeze is set, multiple blank lines are
/// compressed to a single blank line
/// @param view->file_pos is the file position pointer
/// @param view->f_forward is the direction flag
/// @return updated file position pointer
off_t get_next_line(View *view, off_t pos) {
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
///  ╭──────────────────────────────────────────────────────────────╮
///  │ GET_PREV_LINE                                                │
///  ╰──────────────────────────────────────────────────────────────╯
/// Get Previous Line from File into line_in_s
/// @note the input may contain ANSI SGR sequences and Unicode,
/// which will be handled later during formatting for display
/// @note Carriage Return (0x0d) characters are skipped
/// @note Newline (0x0a) characters terminate the line
/// @note If view->f_squeeze is set, multiple blank lines are
/// compressed to a single blank line
/// @param view->file_pos is the file position pointer
/// @param view->f_forward is the direction flag
/// @return updated file position pointer
off_t get_prev_line(View *view, off_t pos) {
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
///  ╭──────────────────────────────────────────────────────────────╮
///  │ GET_POS_NEXT_LINE                                            │
///  ╰──────────────────────────────────────────────────────────────╯
/// Get Position of Next Line
/// @param view->file_pos is the file position pointer
/// @param view->f_forward is the direction flag
/// @return updated file position pointer
off_t get_pos_next_line(View *view, off_t pos) {
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
///  ╭──────────────────────────────────────────────────────────────╮
///  │ GET_POS_PREV_LINE                                            │
///  ╰──────────────────────────────────────────────────────────────╯
/// Get Position of Previous Line
/// @param view->file_pos is the file position pointer
/// @param view->f_forward is the direction flag
/// @return updated file position pointer
off_t get_pos_prev_line(View *view, off_t pos) {
    uchar c;
    view->file_pos = pos;
    if (view->file_pos == 0) {
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
///  ╭──────────────────────────────────────────────────────────────╮
///  │ DISPLAY_LINE   view->cmplx_buf -> view->win                  │
///  ╰──────────────────────────────────────────────────────────────╯
///  Display Line on Pad
///  @return void
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
///  ╭──────────────────────────────────────────────────────────────╮
///  │ FMT_LINE_OUT      line_out_s -> view->cmplx_buf              │
///  │                                 view->stripped_line_out      │
///  ╰──────────────────────────────────────────────────────────────╯
///  Format Line for Display
///  Handle ANSI SGR Sequences and Unicode Multi-byte Characters
///  @return length of line
int fmt_line(View *view) {
    /// @note Convert line_in_s to line_out_s with complex characters
    /// @note Handle ANSI SGR sequences for text attributes
    /// @note Handle Unicode multi-byte characters
    /// @note Handle Tab characters
    /// Resulting lines must match in length and position
    /// @param view->line_in_s is the input line with ANSI and Unicode
    /// @param view->stripped_line_out is the output line without ANSI
    ///         sequences, used for searching
    /// @param view->cmplx_buf is the output complex character buffer
    /// @param view->tab_stop is the tab stop setting
    /// @param view->maxcol is the maximum column on the page
    /// @return length of line
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
    /// Initialize multibyte to wide char conversion
    /// mbtowc, setcchar, and getcchar can sometimes behave badly,
    /// depending on what you feed them. Make sure your locale is set
    /// properly before calling them.
    mbtowc(NULL, NULL, 0);
    while (in_str[i] != '\0') {
        if (in_str[i] == '\033' && in_str[i + 1] == '[') {
            //  ╭───────────────────────────────────────────────────╮
            //  │ ANSI ESCAPE SEQUENCE ENCOUNTERED                  │
            //  ╰───────────────────────────────────────────────────╯
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
            // else if (len == 3 && in_str[i + 3] == 'm') {
            //     ansi_tok[0] = '\033';
            //     ansi_tok[1] = '[';
            //     ansi_tok[2] = '0';
            //     ansi_tok[3] = 'm';
            //     ansi_tok[4] = '\0';
            // }
            parse_ansi_str(ansi_tok, &attr, &cpx);
            i += len;
        } else {
            //  ╭───────────────────────────────────────────────────╮
            //  │ DISPLAYABLE CHARACTER                             │
            //  ╰───────────────────────────────────────────────────╯
            if (in_str[i] == '\033') {
                i++;
                continue;
            }
            s = &in_str[i];
            if (*s == '\t') {
                //  Handle Tab Character
                wc = L' ';
                do {
                    setcchar(&cc, &wc, attr, cpx, NULL);
                    view->stripped_line_out[j] = ' ';
                    cmplx_buf[j++] = cc;
                } while ((j < MAX_COLS - 2) && (j % view->tab_stop != 0));
                i++;
            } else {
                // Handle Multi-byte Character
                // Use mbtowc to get the wide character and its
                // length in bytes from the multibyte string
                len = mbtowc(&wc, s, MB_CUR_MAX);
                if (len <= 0) {
                    // Invalid multibyte sequence, replace with '?'
                    wc = L'?';
                    len = 1;
                }
                // Convert wide character + attributes to complex
                // character
                if (setcchar(&cc, &wc, attr, cpx, NULL) != ERR) {
                    if (len > 0 && (j + len) < MAX_COLS - 1) {
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
///  ╭──────────────────────────────────────────────────────────────╮
///  │ PARSE_ANSI_STR                                               │
///  ╰──────────────────────────────────────────────────────────────╯
/// Parse ANSI SGR Escape Sequences
/// @note Supports SGR sequences for Xterm 256-color and
/// 256 x 256 x 256 = 16,777,216 RGB colors
/// @note Updates attr and cpx for text attributes and color pair index
/// @note The color pairs are updated dynamically as needed
/// @param ansi_str is the ANSI SGR escape sequence string
/// @param attr is the text attribute output
/// @param cpx is the color pair index output
/// @return void
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
    a_toi_error = false;
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
                        a_toi(tok, a_toi_error);
                        if (*tok == '5') {
                            ///  ╭──────────────────────────────────╮
                            ///  │ [34]8;5;<xterm256>               │
                            ///  ╰──────────────────────────────────╯
                            tok = strtok(NULL, ";m");
                            if (tok != NULL) {
                                x_idx = a_toi(tok, a_toi_error);
                                rgb = xterm256_idx_to_rgb(x_idx);
                            }
                        } else if (*tok == '2') {
                            ///  ╭──────────────────────────────────╮
                            ///  │ [34]8;2;<r>,<g>,<b>              │
                            ///  ╰──────────────────────────────────╯
                            tok = strtok(NULL, ";m");
                            rgb.r = a_toi(tok, a_toi_error);
                            tok = strtok(NULL, ";m");
                            rgb.g = a_toi(tok, a_toi_error);
                            tok = strtok(NULL, ";m");
                            rgb.b = a_toi(tok, a_toi_error);
                        }
                    }
                    if (t0 == '3')
                        fg_clr = rgb_to_curses_clr(rgb);
                    else if (t0 == '4')
                        bg_clr = rgb_to_curses_clr(rgb);
                } else if (t1 == '9') {
                    ///  ╭──────────────────────────────────────────╮
                    ///  │ [34]9 DEFAULT COLORS                     │
                    ///  ╰──────────────────────────────────────────╯
                    if (t0 == '3')
                        fg_clr = COLOR_WHITE;
                    else if (t0 == '4')
                        bg_clr = COLOR_BLACK;
                } else if (t1 >= '0' && t1 <= '7') {
                    ///  ╭──────────────────────────────────────────╮
                    ///  │ [34]<pc8>                                │
                    ///  ╰──────────────────────────────────────────╯
                    if (t0 == '3') {
                        x_idx = a_toi(&t1, a_toi_error);
                        rgb = xterm256_idx_to_rgb(x_idx);
                        fg_clr = rgb_to_curses_clr(rgb);
                    } else if (t0 == '4') {
                        x_idx = a_toi(&t1, a_toi_error);
                        rgb = xterm256_idx_to_rgb(x_idx);
                        bg_clr = rgb_to_curses_clr(rgb);
                    }
                }
            } else if (t0 == '0') {
                *tok = t1;
                len = 1;
            }
        }
        if (len == 1) {
            if (*tok == '0') {
                ///  ╭──────────────────────────────────────────────╮
                ///  │ 0m DEFAULT COLORS                            │
                ///  ╰──────────────────────────────────────────────╯
                *attr = WA_NORMAL;
                fg_clr = COLOR_WHITE;
                bg_clr = COLOR_BLACK;
            } else {
                switch (a_toi(tok, a_toi_error)) {
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
            ///  ╭──────────────────────────────────────────────────╮
            ///  │ m DEFAULT COLORS                                 │
            ///  ╰──────────────────────────────────────────────────╯
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
///  ╭──────────────────────────────────────────────────────────────╮
///  │ CMD_LINE_PROMPT                                              │
///  ╰──────────────────────────────────────────────────────────────╯
/// Display Command Line Prompt
/// @param s is the prompt string
/// @return void
void cmd_line_prompt(View *view, char *s) {
    char message_str[MAX_COLS + 1];
    int l;
    l = strnz__cpy(message_str, s, MAX_COLS);
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
///  ╭──────────────────────────────────────────────────────────────╮
///  │ REMOVE_FILE                                                  │
///  ╰──────────────────────────────────────────────────────────────╯
/// Remove File
/// @param view is the current view data structure
/// @return void
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
///  ╭──────────────────────────────────────────────────────────╮
///  │ VIEW_DISPLAY_HELP                                        │
///  ╰──────────────────────────────────────────────────────────╯
/// Display View Help File
/// @param view is the current view data structure
/// @return void
void view_display_help(Init *init) {
    int eargc;
    char *eargv[MAXARGS];
    View *view_save = init->view;
    init->view = NULL;
    eargv[0] = HELP_CMD;
    eargv[1] = VIEW_HELP_FILE;
    eargv[2] = NULL;
    eargc = 2;
    init->lines = 10;
    init->cols = 54;
    init->begy = view->begy + 1;
    init->begx = view->begx + 4;
    strnz__cpy(init->title, "View Help", MAXLEN - 1);
    mview(init, eargc, eargv);
    view = init->view = view_save;
    view->f_redisplay_page = true;
}
