/// fpick.c
/// Bill Waller Copyright (c) 2025
/// MIT License
/// billxwaller@gmail.com
/// pick from a list of choices for MENU

#include "menu.h"
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

int tbl_col, tbl_line, tbl_page, tbl_cols, pg_lines, tbl_pages;
int obj_idx, calculated_idx;
int calc_obj_idx(int, int, int, int, int, int);
void calc_tbl_coord(int, int, int, int *, int *, int *);
int calc_tbl_page(int, int, int);
int calc_tbl_line(int, int);
int calc_tbl_col(int);
int pick_engine(Init *);
void save_object(Pick *, char *);
int picker(Init *);
void display_page(Pick *);
void reverse_object(Pick *);
void unreverse_object(Pick *);
void toggle_object(Pick *);
int output_objects(Pick *);
int exec_objects(Init *);
int open_pick_win(Init *);
void display_pick_help(Init *);
void pick_display_chyron(Pick *);
int read_pick_input(Init *);
char *replace_substring(char *, const char *, const char *, int);

int pipe_fd[2];

char const pagers_editors[12][10] = {"view", "mview", "less", "more",
                                     "vi",   "vim",   "nano", "nvim",
                                     "pico", "emacs", "edit", ""};

/// ╭────────────────────────────────────────────────────────────────╮
/// │ INIT_PICK                                                      │
/// ╰────────────────────────────────────────────────────────────────╯
int init_pick(Init *init, int argc, char **argv, int begy, int begx) {
    struct stat sb;
    char *s_argv[MAXARGS];
    char tmp_str[MAXLEN];
    int m;
    pid_t pid = 0;

    if (init->pick != NULL)
        destroy_pick(init);
    Pick *pick = new_pick(init, argc, argv, begy, begx);
    if (init->pick != pick)
        abend(-1, "init->pick != pick\n");
    SIO *sio = init->sio;
    /// ╭────────────────────────────────────────────────────────────╮
    /// │ START PROVIDER_CMD, attach pipe to its STDOUT              │
    /// ╰────────────────────────────────────────────────────────────╯
    if (pick->provider_cmd[0] != '\0') {
        str_to_args(s_argv, pick->provider_cmd, MAXARGS - 1);
        if (pipe(pipe_fd) == -1) {
            Perror("pipe(pipe_fd) failed in init_pick");
            return (1);
        }
        if ((pid = fork()) == -1) {
            Perror("fork() failed in init_pick");
            return (1);
        }
        if (pid == 0) { /// Child
            /// Child doesn't need read end of pipe
            close(pipe_fd[P_READ]);
            /// Connect CHILD STDOUT to write end of pipe
            dup2(pipe_fd[P_WRITE], STDOUT_FILENO);
            dup2(pipe_fd[P_WRITE], STDERR_FILENO);
            /// STDOUT attached to write end of pipe, so close pipe fd
            close(pipe_fd[P_WRITE]);
            execvp(s_argv[0], s_argv);
            m = MAXLEN - 24;
            strnz__cpy(tmp_str, "Can't exec pick start cmd: ", m);
            m -= strlen(s_argv[0]);
            strnz__cat(tmp_str, s_argv[0], m);
            Perror(tmp_str);
            exit(EXIT_FAILURE);
        }
        /// ╭───────────────────────────────────────────────────────╮
        /// │ BACK TO PARENT                                        │
        /// ╰───────────────────────────────────────────────────────╯
        /// @note Parent doesn't need write end of pipe
        close(pipe_fd[P_WRITE]);
        /// Open a file pointer on read end of pipe
        pick->in_fp = fdopen(pipe_fd[P_READ], "rb");
        pick->f_in_pipe = true;
    } else {
        /// ╭────────────────────────────────────────────────────────╮
        /// │ PREPARE STDIN AS INPUT                                 │
        /// ╰────────────────────────────────────────────────────────╯
        if ((pick->in_spec[0] == '\0') || strcmp(pick->in_spec, "-") == 0 ||
            strcmp(pick->in_spec, "/dev/stdin") == 0) {
            strnz__cpy(pick->in_spec, "/dev/stdin", MAXLEN - 1);
            pick->in_fp = fdopen(STDIN_FILENO, "rb");
            pick->f_in_pipe = true;
        }
    }
    if (!pick->f_in_pipe) {
        ///  ╭───────────────────────────────────────────────────────╮
        ///  │ IN SPEC IS A FILE                                     │
        ///  ╰───────────────────────────────────────────────────────╯
        if (lstat(pick->in_spec, &sb) == -1) {
            m = MAXLEN - 29;
            strnz__cpy(tmp_str, "Can\'t stat pick input file: ", m);
            m -= strlen(pick->in_spec);
            strnz__cat(tmp_str, pick->in_spec, m);
            Perror(tmp_str);
            return (1);
        }
        if (sb.st_size == 0) {
            m = MAXLEN - 24;
            strnz__cpy(tmp_str, "Pick input file empty: ", m);
            m -= strlen(pick->in_spec);
            strnz__cat(tmp_str, pick->in_spec, m);
            Perror(tmp_str);
            return (1);
        }
        if ((pick->in_fp = fopen(pick->in_spec, "rb")) == NULL) {
            m = MAXLEN - 29;
            strnz__cpy(tmp_str, "Can't open pick input file: ", m);
            m -= strlen(pick->in_spec);
            strnz__cat(tmp_str, pick->in_spec, m);
            Perror(tmp_str);
            return (1);
        }
    }
    read_pick_input(init);
    if (pick->f_in_pipe && pid > 0) {
        waitpid(pid, NULL, 0);
        close(pipe_fd[P_READ]);
        dup2(sio->stdin_fd, STDIN_FILENO);
        dup2(sio->stdout_fd, STDOUT_FILENO);
        restore_curses_tioctl();
        sig_prog_mode();
        keypad(pick->win, true);
    }
    if (pick->obj_cnt == 0) {
        Perror("No pick objects available");
        destroy_pick(init);
        return (1);
    }
    pick_engine(init);
    win_del();
    destroy_pick(init);
    return 0;
}
/// ╭────────────────────────────────────────────────────────────────╮
/// │ READ_PICK_INPUT                                                │
/// ╰────────────────────────────────────────────────────────────────╯
int read_pick_input(Init *init) {
    int i;

    Pick *pick = init->pick;
    pick->select_cnt = 0;
    pick->obj_cnt = pick->pg_lines = pick->tbl_cols = 0;
    pick->obj_idx = pick->tbl_page = pick->y = pick->tbl_col = pick->x = 0;
    pick->tbl_pages = 1;

    if (pick->in_fp) {
        while (fgets(pick->in_buf, sizeof(pick->in_buf), pick->in_fp) != NULL)
            save_object(pick, pick->in_buf);
    } else
        for (i = 1; i < pick->argc; i++)
            save_object(pick, pick->argv[i]);
    if (pick->in_fp != NULL)
        fclose(pick->in_fp);
    if (!pick->obj_idx)
        return (-1);
    pick->obj_cnt = pick->obj_idx;
    pick->obj_idx = 0;
    return 0;
}
/// ╭────────────────────────────────────────────────────────────────╮
/// │ PICK_ENGINE                                                    │
/// ╰────────────────────────────────────────────────────────────────╯
int pick_engine(Init *init) {
    int n, chyron_l, rc;
    int maxy, maxx, win_maxy, win_maxx;

    /// ╭────────────────────────────────────────────────────────────╮
    /// │ PICK TABLE LAYOUT                                          │
    /// ╰────────────────────────────────────────────────────────────╯
    for (n = 0; key_cmd[n].end_pos != -1; n++)
        key_cmd[n].text[0] = '\0';
    strnz__cpy(key_cmd[1].text, "F1 Help", 32);
    strnz__cpy(key_cmd[9].text, "F9 Cancel", 32);
    strnz__cpy(key_cmd[10].text, "F10 Accept", 32);
    strnz__cpy(key_cmd[11].text, "PgUp", 32);
    strnz__cpy(key_cmd[12].text, "PgDn", 32);
    strnz__cpy(key_cmd[13].text, "Space", 32);
    strnz__cpy(key_cmd[14].text, "Enter", 32);
    chyron_l = chyron_mk(key_cmd, pick->chyron_s);
    getmaxyx(stdscr, maxy, maxx);

    win_maxy = (maxy * 8) / 10;
    /// ╭────────────────────────────────────────────────────────────╮
    /// │ Window can't be larger than physical display - 2           │
    /// ╰────────────────────────────────────────────────────────────╯
    if (win_maxy > (maxy - pick->begy) - 2)
        win_maxy = (maxy - pick->begy) - 2;
    win_maxx = (maxx * 9) / 10;
    if (win_maxx > (maxx - pick->begx) - 2)
        win_maxx = (maxx - pick->begx) - 2;
    if (chyron_l > win_maxx)
        chyron_l = strnz(pick->chyron_s, win_maxx);
    if (pick->tbl_col_width < 4)
        pick->tbl_col_width = 4;
    /// ╭────────────────────────────────────────────────────────────╮
    /// │ Column can't be larger than physical display - 2           │
    /// ╰────────────────────────────────────────────────────────────╯
    if (pick->tbl_col_width > win_maxx - 2)
        pick->tbl_col_width = win_maxx - 2;
    pick->tbl_cols = (win_maxx / (pick->tbl_col_width + 1));
    pick->win_width = (pick->tbl_col_width + 1) * pick->tbl_cols;
    if (pick->win_width < chyron_l)
        pick->win_width = chyron_l;
    /// ╭────────────────────────────────────────────────────────────╮
    /// │ Calculate final dimensions                                 │
    /// ╰────────────────────────────────────────────────────────────╯
    pick->tbl_lines = ((pick->obj_cnt - 1) / pick->tbl_cols) + 1;
    pick->tbl_pages = (pick->tbl_lines / (win_maxy - 1)) + 1;
    pick->pg_lines = (pick->tbl_lines / pick->tbl_pages) + 1;
    pick->win_lines = pick->pg_lines + 1;
    pick->tbl_page = 0;
    ///  ╭───────────────────────────────────────────────────────────╮
    ///  │ pick->win_lines 1/5      top margin                       │
    ///  ╰───────────────────────────────────────────────────────────╯
    if (pick->begy == 0)
        pick->begy = (LINES - pick->win_lines) / 5;
    else if (pick->begy + pick->win_lines > LINES - 4)
        pick->begy = LINES - pick->win_lines - 2;
    ///  ╭───────────────────────────────────────────────────────────╮
    ///  │ pick->win_width               left margin                 │
    ///  ╰───────────────────────────────────────────────────────────╯
    if (pick->begx + pick->win_width > COLS - 4)
        pick->begx = COLS - pick->win_width - 2;
    else if (pick->begx == 0)
        pick->begx = (COLS - pick->win_width) / 2;

    rc = open_pick_win(init);
    if (rc)
        return (rc);
    display_page(pick);
    reverse_object(pick);
    /// ╭────────────────────────────────────────────────────────────╮
    /// │ PICK MAIN LOOP                                             │
    /// ╰────────────────────────────────────────────────────────────╯
    pick->obj_idx = 0;
    pick->x = 1;
    mousemask(BUTTON1_CLICKED | BUTTON1_DOUBLE_CLICKED, NULL);
    picker(init);
    /// ╭────────────────────────────────────────────────────────────╮
    /// │ PICK FINISHED - Perform Output                             │
    /// ╰────────────────────────────────────────────────────────────╯
    if (pick->select_cnt > 0) {
        if (pick->f_out_spec && pick->out_spec[0])
            rc = output_objects(pick);
        if (pick->f_cmd && pick->cmd[0])
            rc = exec_objects(init);
    }
    return (rc);
}
/// ╭────────────────────────────────────────────────────────────╮
/// │ SAVE_OBJECT                                                │
/// ╰────────────────────────────────────────────────────────────╯
/// Save object string into pick structure
/// @param pick Pointer to Pick structure
/// @param s String to save
/// @return void
void save_object(Pick *pick, char *s) {
    int l;

    if (pick->obj_idx < OBJ_MAXCNT - 1) {
        l = strlen(s);
        if (l > OBJ_MAXLEN - 1)
            s[OBJ_MAXLEN - 1] = '\0';
        if (l > pick->tbl_col_width)
            pick->tbl_col_width = l;
        if (l < 1)
            l = 1;
        pick->object[pick->obj_idx] = (char *)calloc(l + 1, sizeof(char));
        strnz__cpy(pick->object[pick->obj_idx], s, l);
        pick->f_selected[pick->obj_idx] = FALSE;
        pick->obj_idx++;
    }
}

/// ╭────────────────────────────────────────────────────────────────╮
/// │ PICKER                                                         │
/// ╰────────────────────────────────────────────────────────────────╯
int picker(Init *init) {
    int display_tbl_page;

    pick = init->pick;
    MEVENT event;
    event.y = event.x = -1;
    cmd_key = 0;
    while (1) {
        tcflush(tty_fd, TCIFLUSH);
        if (cmd_key == 0)
            cmd_key = xwgetch(pick->win);
        switch (cmd_key) {
            /// ╭───────────────────────────────────────────────────────────╮
            /// │ KEY_F(9), 'Q', 'q' - Cancel                               │
            /// ╰───────────────────────────────────────────────────────────╯
        case 'q':
        case 'Q':
        case KEY_F(9):
            return -1;
            /// ╭───────────────────────────────────────────────────────────╮
            /// │ 'H' - Help                                                │
            /// ╰───────────────────────────────────────────────────────────╯
        case 'H': /// Help
            display_pick_help(init);
            display_page(pick);
            reverse_object(pick);
            cmd_key = 0;
            break;
            /// ╭───────────────────────────────────────────────────────────╮
            /// │ 't', 'T', ' ' - Toggle item                               │
            /// ╰───────────────────────────────────────────────────────────╯
        case ' ':
        case 't':
        case 'T': /// Toggle
            toggle_object(pick);
            if (pick->select_cnt == pick->select_max)
                return pick->select_cnt;
            cmd_key = 0;
            break;
            /// ╭───────────────────────────────────────────────────────────╮
            /// │ KEY_F(10), KEY_ENTER - Accept Selections                  │
            /// ╰───────────────────────────────────────────────────────────╯
        case KEY_F(10):
        case '\n':
        case KEY_ENTER:
            return pick->select_cnt;
            /// ╭───────────────────────────────────────────────────────────╮
            /// │ KEY_END - Display last page                               │
            /// ╰───────────────────────────────────────────────────────────╯
        case KEY_END:
            mvwaddstr_fill(pick->win, pick->y, pick->x,
                           pick->object[pick->obj_idx], pick->tbl_col_width);
            display_tbl_page = pick->tbl_page;
            pick->obj_idx = pick->obj_cnt - 1;
            pick->tbl_page = pick->obj_idx / (pick->pg_lines * pick->tbl_cols);
            pick->tbl_line = (pick->obj_idx / pick->tbl_cols) % pick->pg_lines;
            pick->tbl_col = pick->obj_idx % pick->tbl_cols;
            pick->y = pick->tbl_line;
            if (display_tbl_page != pick->tbl_page) {
                display_page(pick);
            }
            reverse_object(pick);
            cmd_key = 0;
            break;
            /// ╭───────────────────────────────────────────────────────────╮
            /// │ KEY_RIGHT, 'l' - Move right one column                    │
            /// ╰───────────────────────────────────────────────────────────╯
        case 'l':
        case KEY_RIGHT:
            mvwaddstr_fill(pick->win, pick->y, pick->x,
                           pick->object[pick->obj_idx], pick->tbl_col_width);
            /// pick->obj_idx += pick->tbl_lines -> next column
            if (pick->tbl_page * pick->pg_lines * pick->tbl_cols +
                        (pick->tbl_col + 1) * pick->pg_lines + pick->tbl_line <
                    pick->obj_cnt - 1 &&
                pick->tbl_col < pick->tbl_cols - 1)
                pick->tbl_col++;
            pick->obj_idx = pick->tbl_page * pick->pg_lines * pick->tbl_cols +
                            pick->tbl_col * pick->pg_lines + pick->tbl_line;
            reverse_object(pick);
            cmd_key = 0;
            break;
            /// ╭───────────────────────────────────────────────────────────╮
            /// │ KEY_LEFT, 'h' - Move left one column                      │
            /// ╰───────────────────────────────────────────────────────────╯
        case 'h':
        case KEY_LEFT:
        case KEY_BACKSPACE:
            mvwaddstr_fill(pick->win, pick->y, pick->x,
                           pick->object[pick->obj_idx], pick->tbl_col_width);
            if (pick->tbl_col > 0)
                pick->tbl_col--;
            pick->obj_idx = pick->tbl_page * pick->pg_lines * pick->tbl_cols +
                            pick->tbl_col * pick->pg_lines + pick->tbl_line;
            cmd_key = 0;
            reverse_object(pick);
            break;
            /// ╭───────────────────────────────────────────────────────────╮
            /// │ KEY_DOWN, 'j' - Move down one row                         │
            /// ╰───────────────────────────────────────────────────────────╯
        case KEY_DOWN:
        case 'j':
            mvwaddstr_fill(pick->win, pick->y, pick->x,
                           pick->object[pick->obj_idx], pick->tbl_col_width);
            /// pick->obj_idx++ column down
            if (pick->tbl_page * pick->pg_lines * pick->tbl_cols +
                        pick->tbl_col * pick->pg_lines + pick->tbl_line <
                    pick->obj_cnt - 1 &&
                pick->tbl_line < pick->pg_lines - 1)
                pick->tbl_line++;
            pick->obj_idx = pick->tbl_page * pick->pg_lines * pick->tbl_cols +
                            pick->tbl_col * pick->pg_lines + pick->tbl_line;
            reverse_object(pick);
            cmd_key = 0;
            break;
            /// ╭───────────────────────────────────────────────────────────╮
            /// │ KEY_UP, 'k' - Move up one row                             │
            /// ╰───────────────────────────────────────────────────────────╯
        case KEY_UP:
        case 'k':
            mvwaddstr_fill(pick->win, pick->y, pick->x,
                           pick->object[pick->obj_idx], pick->tbl_col_width);
            if (pick->tbl_line > 0)
                pick->tbl_line--;
            pick->obj_idx = pick->tbl_page * pick->pg_lines * pick->tbl_cols +
                            pick->tbl_col * pick->pg_lines + pick->tbl_line;
            reverse_object(pick);
            cmd_key = 0;
            break;
            /// ╭───────────────────────────────────────────────────────────╮
            /// │ KEY_NPAGE, CTRL(F) - Display next page                    │
            /// ╰───────────────────────────────────────────────────────────╯
        case KEY_NPAGE:
        case '\06':
            if (pick->tbl_page < pick->tbl_pages - 1) {
                pick->tbl_page++;
                pick->pg_line = 0;
                pick->tbl_col = 0;
            }
            pick->obj_idx = pick->tbl_page * pick->pg_lines * pick->tbl_cols +
                            pick->tbl_cols * pick->pg_line + pick->tbl_col;
            display_page(pick);
            reverse_object(pick);
            cmd_key = 0;
            break;
            /// ╭───────────────────────────────────────────────────────────╮
            /// │ KEY_PPAGE, CTRL(B) - Display previous page                │
            /// ╰───────────────────────────────────────────────────────────╯
        case KEY_PPAGE:
        case '\02':
            if (pick->tbl_page > 0)
                pick->tbl_page--;
            pick->obj_idx = pick->tbl_page * pick->pg_lines * pick->tbl_cols +
                            pick->tbl_cols * pick->pg_line + pick->tbl_col;
            display_page(pick);
            reverse_object(pick);
            cmd_key = 0;
            break;
            /// ╭───────────────────────────────────────────────────────────╮
            /// │ KEY_HOME - Display First Page                             │
            /// ╰───────────────────────────────────────────────────────────╯
        case KEY_HOME:
            pick->tbl_page = 0;
            pick->tbl_line = 0;
            pick->tbl_col = 0;
            pick->obj_idx = pick->tbl_page * pick->pg_lines * pick->tbl_cols +
                            pick->tbl_cols * pick->pg_line + pick->tbl_col;
            display_page(pick);
            reverse_object(pick);
            cmd_key = 0;
            break;
            /// ╭───────────────────────────────────────────────────────────╮
            /// │ KEY_LL (END) - Display Last Page                          │
            /// ╰───────────────────────────────────────────────────────────╯
        case KEY_LL:
            pick->tbl_page = pick->tbl_pages - 1;
            pick->obj_idx = pick->tbl_page * pick->pg_lines * pick->tbl_cols +
                            pick->tbl_cols * pick->pg_line + pick->tbl_col;
            display_page(pick);
            reverse_object(pick);
            cmd_key = 0;
            break;
        ///  ╭───────────────────────────────────────────────────────╮
        ///  │ PICK MOUSE EVENT                                      │
        ///  ╰───────────────────────────────────────────────────────╯
        /// BUTTON1 CLICK or DOUBLE_CLICK Toggles Selection
        /// or Activates Chyron Keys
        case KEY_MOUSE:
            if (getmouse(&event) != OK) {
                cmd_key = 0;
                break;
            }
            if (event.bstate == BUTTON1_CLICKED ||
                event.bstate == BUTTON1_DOUBLE_CLICKED) {
                if (!wenclose(pick->win, event.y, event.x)) {
                    cmd_key = 0;
                    break;
                }
                wmouse_trafo(pick->win, &event.y, &event.x, false);
                if (event.y < 0 ||
                    event.x >= (pick->tbl_cols * (pick->tbl_col_width + 1))) {
                    cmd_key = 0;
                    break;
                }
                unreverse_object(pick);
                pick->y = event.y;
                if (pick->y == pick->pg_lines) {
                    cmd_key = get_chyron_key(key_cmd, event.x);
                    continue;
                }
                pick->tbl_col = (event.x - 1) / (pick->tbl_col_width + 1);
                if (pick->tbl_col < 0 || pick->tbl_col >= pick->tbl_cols) {
                    cmd_key = 0;
                    continue;
                }
                pick->obj_idx =
                    pick->tbl_page * pick->pg_lines * pick->tbl_cols +
                    pick->tbl_col * pick->pg_lines + pick->y;
                toggle_object(pick);
                reverse_object(pick);
                if (pick->select_cnt == pick->select_max)
                    return pick->select_cnt;
                wrefresh(pick->win);
                cmd_key = 0;
                break;
            }
            cmd_key = 0;
            break;
        default:
            cmd_key = 0;
            break;
        }
    }
    return 0;
}
/// ╭────────────────────────────────────────────────────────────────╮
/// │ DISPLAY_PAGE                                                   │
/// ╰────────────────────────────────────────────────────────────────╯
void display_page(Pick *pick) {
    int y, col, pidx;
    for (y = 0; y < pick->pg_lines; y++) {
        wmove(pick->win, y, 0);
        wclrtoeol(pick->win);
    }
    pidx = pick->tbl_page * pick->pg_lines * pick->tbl_cols;
    for (col = 0; col < pick->tbl_cols; col++) {
        pick->x = col * (pick->tbl_col_width + 1) + 1;
        for (y = 0; y < pick->pg_lines; y++, pidx++) {
            if (pidx < pick->obj_cnt) {
                if (pick->f_selected[pidx])
                    mvwaddstr(pick->win, y, pick->x - 1, "*");
                mvwaddstr_fill(pick->win, y, pick->x, pick->object[pidx],
                               pick->tbl_col_width);
            }
        }
    }
    pick_display_chyron(pick);
}
///  ╭───────────────────────────────────────────────────────────────╮
///  │ PICK_DISPLAY_CHYRON                                           │
///  ╰───────────────────────────────────────────────────────────────╯
void pick_display_chyron(Pick *pick) {
    int l;
    char tmp_str[MAXLEN];
    ssnprintf(tmp_str, MAXLEN - 65, "%s| Page %d of %d ", pick->chyron_s,
              pick->tbl_page + 1, pick->tbl_pages);
    l = strlen(tmp_str);
    wattron(pick->win, WA_REVERSE);
    mvwaddstr(pick->win, pick->pg_lines, 0, tmp_str);
    wattroff(pick->win, WA_REVERSE);
    wmove(pick->win, pick->pg_lines, l);
    wclrtoeol(pick->win);
}
/// ╭────────────────────────────────────────────────────────────────╮
/// │ REVERSE_OBJECT                                                 │
/// ╰────────────────────────────────────────────────────────────────╯
void reverse_object(Pick *pick) {
    pick->x = pick->tbl_col * (pick->tbl_col_width + 1) + 1;
    pick->y = pick->tbl_line;
    wmove(pick->win, pick->y, pick->x);
    wattron(pick->win, WA_REVERSE);
    mvwaddstr_fill(pick->win, pick->y, pick->x, pick->object[pick->obj_idx],
                   pick->tbl_col_width);
    wattroff(pick->win, WA_REVERSE);
    wrefresh(pick->win);
    wmove(pick->win, pick->y, pick->x - 1);
}
/// ╭────────────────────────────────────────────────────────────────╮
/// │ UNREVERSE_OBJECT                                               │
/// ╰────────────────────────────────────────────────────────────────╯
void unreverse_object(Pick *pick) {
    pick->x = pick->tbl_col * (pick->tbl_col_width + 1) + 1;
    wmove(pick->win, pick->y, pick->x);
    mvwaddstr_fill(pick->win, pick->y, pick->x, pick->object[pick->obj_idx],
                   pick->tbl_col_width);
    wrefresh(pick->win);
    wmove(pick->win, pick->y, pick->x - 1);
}
/// ╭────────────────────────────────────────────────────────────────╮
/// │ TOGGLE_OBJECTS                                                 │
/// ╰────────────────────────────────────────────────────────────────╯
void toggle_object(Pick *pick) {
    pick->x = pick->tbl_col * (pick->tbl_col_width + 1) + 1;
    if (pick->f_selected[pick->obj_idx]) {
        pick->select_cnt--;
        pick->f_selected[pick->obj_idx] = FALSE;
        mvwaddstr(pick->win, pick->y, pick->x - 1, " ");
    } else {
        pick->select_cnt++;
        pick->f_selected[pick->obj_idx] = true;
        mvwaddstr(pick->win, pick->y, pick->x - 1, "*");
    }
}
/// ╭────────────────────────────────────────────────────────────────╮
/// │ OUTPUT_OBJECTS                                                 │
/// ╰────────────────────────────────────────────────────────────────╯
int output_objects(Pick *pick) {
    int m;
    if ((pick->out_fp = fopen(pick->out_spec, "w")) == NULL) {
        ///  ╭───────────────────────────────────────────────────────╮
        ///  │ OUT SPEC IS A FILE                                    │
        ///  ╰───────────────────────────────────────────────────────╯
        m = MAXLEN - 30;
        strnz__cpy(tmp_str, "Can't open pick output file: ", m);
        m -= strlen(pick->in_spec);
        strnz__cat(tmp_str, pick->out_spec, m);
    }
    for (pick->obj_idx = 0; pick->obj_idx < pick->obj_cnt; pick->obj_idx++) {
        if (pick->f_selected[pick->obj_idx])
            fprintf(stdout, "%s\n", pick->object[pick->obj_idx]);
    }
    fflush(stdout);
    if (pick->out_fp != NULL)
        fclose(pick->out_fp);
    return (0);
}
/// ╭────────────────────────────────────────────────────────────────╮
/// │ EXEC_OBJECTS                                                   │
/// ╰────────────────────────────────────────────────────────────────╯
int exec_objects(Init *init) {
    int rc = -1;
    int margc;
    char *margv[MAXARGS];
    char tmp_str[MAXLEN];
    char title[MAXLEN];
    char sav_arg[MAXLEN];
    char *out_s;
    int margx = 0;
    int i = 0;
    pid_t pid = 0;
    bool f_append_args = false;
    char *s1;

    title[0] = '\0';
    if (pick->cmd[0] == '\0')
        return -1;
    if (pick->cmd[0] == '\\' || pick->cmd[0] == '\"') {
        /// Remove surrounding quotes if present
        size_t len = strlen(pick->cmd);
        if (len > 1 && pick->cmd[len - 1] == '\"') {
            memmove(pick->cmd, pick->cmd + 1, len - 2);
            pick->cmd[len - 2] = '\0';
        }
    }
    margc = str_to_args(margv, pick->cmd, MAXARGS - 1);
    tmp_str[0] = '\0';
    if (pick->f_multiple_cmd_args) {
        ///  ╭───────────────────────────────────────────────────────╮
        ///  │ COMBINE MULTIPLE OBJECTS IN ONE MARGV[n]              │
        ///  ╰───────────────────────────────────────────────────────╯
        for (i = 0; i < pick->obj_cnt; i++) {
            if (pick->f_selected[i] && margc < MAXARGS) {
                if (tmp_str[0] != '\0')
                    strnz__cat(tmp_str, " ", MAXLEN - 1);
                strnz__cat(tmp_str, pick->object[i], MAXLEN - 1);
            }
        }
        margv[margc++] = strdup(tmp_str);
    } else {
        ///  ╭───────────────────────────────────────────────────────╮
        ///  │ EACH OBJECT IN A SEPARATE MARGV[n]                    │
        ///  │ EXCEPT IF OBJECT CONTAINS %%                          │
        ///  │ WHICH WE REPLACE WITH SELECTED OBJECTS                │
        ///  ╰───────────────────────────────────────────────────────╯
        ///  CHECK FOR %% IN ARGS
        ///  @note Only process first occurrence of %%
        ///  @param %% is replaced with selected objects
        ///         as a parameter list separated by spaces
        f_append_args = false;
        i = 0;
        while (i < margc) {
            if (strstr(margv[i], "%%") != NULL) {
                tmp_str[0] = '\0';
                f_append_args = true;
                strnz__cpy(sav_arg, margv[i], MAXLEN - 1);
                margx = i;
                break;
            }
            i++;
        }
        /// Add selected objects
        for (i = 0; i < pick->obj_cnt - 1; i++) {
            if (pick->f_selected[i] && margc < MAXARGS - 1) {
                if (f_append_args == true) {
                    if (tmp_str[0] != '\0')
                        strnz__cat(tmp_str, " ", MAXLEN - 1);
                    strnz__cat(tmp_str, pick->object[i], MAXLEN - 1);
                    continue;
                }
                margv[margc++] = strdup(pick->object[i]);
            }
        }
        if (f_append_args == true) {
            if (margv[margx] != NULL) {
                free(margv[margx]);
                margv[margx] = NULL;
            }
            out_s = rep_substring(sav_arg, "%%", tmp_str);
            if (out_s == NULL || out_s[0] == '\0') {
                i = 0;
                while (i < margc) {
                    if (margv[i] != NULL)
                        free(margv[i]);
                    i++;
                }
                Perror("rep_substring() failed in exec_objects");
                return 1;
            }
            strnz__cpy(title, out_s, MAXLEN - 1);
            margv[margx] = strdup(out_s);
            if (out_s != NULL) {
                free(out_s);
                out_s = NULL;
            }
        }
    }
    strnz__cpy(tmp_str, margv[0], MAXLEN - 1);
    margv[margc] = NULL;
    // if (margc == 0) {
    //     Perror("No pick exec command available");
    //     return 1;
    // }
    s1 = tmp_str;
    //                Why Memory leak here?
    char *sp;
    char *tok;
    tok = strtok_r(s1, " ", &sp);
    strnz__cpy(sav_arg, tok, MAXLEN - 1);
    base_name(tmp_str, sav_arg);
    ///  ╭──────────────────────────────────────────────────────────╮
    ///  │ CHECK FOR VIEW / MVIEW COMMAND                           │
    ///  ╰──────────────────────────────────────────────────────────╯
    if (tmp_str[0] != '\0' &&
        (strcmp(tmp_str, "view") == 0 || strcmp(tmp_str, "mview") == 0)) {
        ///  ╭──────────────────────────────────────────────────────╮
        ///  │ SPECIAL CASE FOR VIEW / MVIEW                        │
        ///  ╰──────────────────────────────────────────────────────╯
        zero_opt_args(init);
        parse_opt_args(init, margc, margv);
        if (init->begy == 0)
            init->begy = pick->begy + 1;
        if (init->begx == 0)
            init->begx = pick->begx + 1;
        if (title[0] != '\0')
            strnz__cpy(init->title, title, MAXLEN - 1);
        else
            strnz__cpy(init->title, margv[margc], MAXLEN - 1);
        mview(init, margc, margv);
        i = 0;
        while (i < margc) {
            if (margv[i] != NULL)
                free(margv[i]);
            i++;
        }
        return 0;
    }
    ///  ╭──────────────────────────────────────────────────────────╮
    ///  │ FORK/EXEC PICK COMMAND                                   │
    ///  ╰──────────────────────────────────────────────────────────╯
    else {
        if ((pid = fork()) == -1) {
            // fork failed, free margv and return error
            i = 0;
            while (i < margc) {
                if (margv[i] != NULL)
                    free(margv[i]);
                i++;
            }
            Perror("fork() failed in exec_objects");
            return (1);
        }
        if (pid == 0) { /// Child
            execvp(margv[0], margv);
            // exec failed, print error and exit
            strnz__cpy(tmp_str, "Can't exec pick cmd: ", MAXLEN - 1);
            strnz__cat(tmp_str, margv[0], MAXLEN - 1);
            Perror(tmp_str);
            exit(EXIT_FAILURE);
        }
    }
    /// Parent
    waitpid(pid, NULL, 0);
    i = 0;
    while (i < margc) {
        if (margv[i] != NULL)
            free(margv[i]);
        i++;
    }
    restore_curses_tioctl();
    sig_prog_mode();
    keypad(pick->win, true);
    restore_wins();
    return rc;
}
/// ╭───────────────────────────────────────────────────────────────╮
/// │ OPEN_PICK_WIN                                                 │
/// ╰───────────────────────────────────────────────────────────────╯
int open_pick_win(Init *init) {
    char tmp_str[MAXLEN];
    pick = init->pick;
    if (win_new(pick->win_lines, pick->win_width, pick->begy, pick->begx,
                pick->title, 0)) {
        ssnprintf(tmp_str, MAXLEN - 65,
                  "win_new(%d, %d, %d, %d, %s, %b) failed", pick->win_lines,
                  pick->win_width, pick->begy, pick->begx, pick->title, 0);
        Perror(tmp_str);

        return (1);
    }
    pick->win = win_win[win_ptr];
    pick->box = win_box[win_ptr];
    scrollok(pick->win, FALSE);
    keypad(pick->win, true);
    /// idcok(pick->win, true);
    return 0;
}
/// ╭───────────────────────────────────────────────────────────────╮
/// │ DISPLAY_PICK_HELP                                             │
/// ╰───────────────────────────────────────────────────────────────╯
void display_pick_help(Init *init) {
    pick = init->pick;
    char tmp_spec[MAXLEN];
    int margc = 0;
    char *margv[MAXARGS];
    strnz__cpy(tmp_spec, "~/menuapp/help/pick.help", MAXLEN - 1);
    pick->f_help_spec = verify_spec_arg(
        pick->help_spec, tmp_spec, "~/menuapp/help", init->mapp_help, R_OK);
    margv[margc++] = strdup("mview");
    margv[margc++] = pick->help_spec;
    margv[margc] = NULL;
    init->lines = 20;
    init->cols = 60;
    init->begy = pick->begy + 1;
    init->begx = pick->begx + 4;
    strnz__cpy(init->title, margv[1], MAXLEN - 1);
    mview(init, margc, margv);
}
