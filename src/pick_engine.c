/* fpick.c
 * pick from a list of choices for MENU
 * Bill Waller
 * billxwaller@gmail.com
 */

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

int pipe_fd[2];

char const pagers_editors[12][10] = {"view", "mview", "less", "more",
                                     "vi",   "vim",   "nano", "nvim",
                                     "pico", "emacs", "edit", ""};

/* ╭────────────────────────────────────────────────────────────────╮
   │ INIT_PICK                                                      │
   ╰────────────────────────────────────────────────────────────────╯ */
int init_pick(Init *init, int argc, char **argv, int begy, int begx) {
    struct stat sb;
    char *s_argv[MAXARGS];
    char tmp_str[MAXLEN];
    int m;
    pid_t pid = 0;

    if (init->pick != NULL)
        close_pick(init);
    Pick *pick = new_pick(init, argc, argv, begy, begx);
    if (init->pick != pick)
        abend(-1, "init->pick != pick\n");

    /* ╭────────────────────────────────────────────────────────────╮
       │ INPUT IS START_CMD                                         │
       ╰────────────────────────────────────────────────────────────╯ */
    if (pick->start_cmd[0] != '\0') {
        str_to_args(s_argv, pick->start_cmd, MAXARGS - 1);
        if (pipe(pipe_fd) == -1) {
            Perror("pipe(pipe_fd) failed in init_pick");
            return (1);
        }
        if ((pid = fork()) == -1) {
            Perror("fork() failed in init_pick");
            return (1);
        }
        if (pid == 0) { // Child
            close(pipe_fd[P_READ]);
            dup2(pipe_fd[P_WRITE], STDOUT_FILENO);
            close(pipe_fd[P_WRITE]);
            execvp(s_argv[0], s_argv);
            m = MAXLEN - 24;
            strncpy(tmp_str, "Can't exec pick start cmd: ", m);
            m -= strlen(s_argv[0]);
            strncat(tmp_str, s_argv[0], m);
            Perror(tmp_str);
            exit(EXIT_FAILURE);
        }
        // Back to parent
        close(pipe_fd[P_WRITE]);
        pick->in_fp = fdopen(pipe_fd[P_READ], "rb");
        pick->f_in_pipe = true;
    } else {
        /* ╭────────────────────────────────────────────────────────╮
           │ INPUT IS STDIN                                         │
           ╰────────────────────────────────────────────────────────╯ */
        if ((pick->in_spec[0] == '\0') || strcmp(pick->in_spec, "-") == 0 ||
            strcmp(pick->in_spec, "/dev/stdin") == 0) {
            strcpy(pick->in_spec, "/dev/stdin");
            pick->in_fp = fdopen(STDIN_FILENO, "rb");
            pick->f_in_pipe = true;
        }
    }
    if (!pick->f_in_pipe) {
        /*  ╭───────────────────────────────────────────────────────╮
            │ IN SPEC IS A FILE                                     │
            ╰───────────────────────────────────────────────────────╯ */
        if (lstat(pick->in_spec, &sb) == -1) {
            m = MAXLEN - 29;
            strncpy(tmp_str, "Can\'t stat pick input file: ", m);
            m -= strlen(pick->in_spec);
            strncat(tmp_str, pick->in_spec, m);
            Perror(tmp_str);
            return (1);
        }
        if (sb.st_size == 0) {
            m = MAXLEN - 24;
            strncpy(tmp_str, "Pick input file empty: ", m);
            m -= strlen(pick->in_spec);
            strncat(tmp_str, pick->in_spec, m);
            Perror(tmp_str);
            return (1);
        }
        if ((pick->in_fp = fopen(pick->in_spec, "rb")) == NULL) {
            m = MAXLEN - 29;
            strncpy(tmp_str, "Can't open pick input file: ", m);
            m -= strlen(pick->in_spec);
            strncat(tmp_str, pick->in_spec, m);
            Perror(tmp_str);
            return (1);
        }
    }
    read_pick_input(init);
    if (pick->f_in_pipe && pid > 0) {
        waitpid(pid, NULL, 0);
        dup2(init->stdin_fd, STDIN_FILENO);
        dup2(init->stdout_fd, STDOUT_FILENO);
        restore_curses_tioctl();
        sig_prog_mode();
        keypad(pick->win, true);
    }
    if (pick->obj_cnt == 0) {
        Perror("No pick objects available");
        close_pick(init);
        return (1);
    }
    pick_engine(init);
    if (pick->win)
        win_del();
    close_pick(init);
    return 0;
}
/* ╭────────────────────────────────────────────────────────────────╮
   │ READ_PICK_INPUT                                                │
   ╰────────────────────────────────────────────────────────────────╯ */
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
    fclose(pick->in_fp);
    if (!pick->obj_idx)
        return (-1);
    pick->obj_cnt = pick->obj_idx;
    pick->obj_idx = 0;
    return 0;
}

/* ╭────────────────────────────────────────────────────────────────╮
   │ PICK_ENGINE                                                    │
   ╰────────────────────────────────────────────────────────────────╯ */
int pick_engine(Init *init) {
    int n, chyron_l, rc;
    int maxy, maxx, win_maxy, win_maxx;

    /* ╭────────────────────────────────────────────────────────────╮
       │ PICK TABLE LAYOUT                                          │
       ╰────────────────────────────────────────────────────────────╯ */
    for (n = 0; key_cmd[n].end_pos != -1; n++)
        key_cmd[n].text[0] = '\0';
    strncpy(key_cmd[1].text, "F1 Help", 32);
    strncpy(key_cmd[9].text, "F9 Cancel", 32);
    strncpy(key_cmd[10].text, "F10 Accept", 32);
    strncpy(key_cmd[11].text, "PgUp", 32);
    strncpy(key_cmd[12].text, "PgDn", 32);
    strncpy(key_cmd[13].text, "Space", 32);
    strncpy(key_cmd[14].text, "Enter", 32);
    chyron_l = chyron_mk(key_cmd, pick->chyron_s);
    getmaxyx(stdscr, maxy, maxx);
    win_maxy = (maxy * 8) / 10;
    if (win_maxy > (maxy - pick->begy) - 2)
        win_maxy = (maxy - pick->begy) - 2;
    win_maxx = (maxx * 9) / 10;
    if (win_maxx > (maxx - pick->begx) - 2)
        win_maxx = (maxx - pick->begx) - 2;
    if (chyron_l > win_maxx)
        chyron_l = strnz(pick->chyron_s, win_maxx);
    if (pick->tbl_col_width < 4)
        pick->tbl_col_width = 4;
    if (pick->tbl_col_width > win_maxx - 2)
        pick->tbl_col_width = win_maxx - 2;
    pick->tbl_cols = (win_maxx / (pick->tbl_col_width + 1));
    pick->win_width = (pick->tbl_col_width + 1) * pick->tbl_cols;
    if (pick->win_width < chyron_l)
        pick->win_width = chyron_l;
    pick->tbl_lines = ((pick->obj_cnt - 1) / pick->tbl_cols) + 1;
    pick->tbl_pages = (lines / (win_maxy - 1)) + 1;
    pick->pg_lines = (pick->tbl_lines / pick->tbl_pages) + 1;
    pick->win_lines = pick->pg_lines + 1;
    pick->tbl_page = 0;
    rc = open_pick_win(init);
    if (rc)
        return (rc);
    display_page(pick);
    reverse_object(pick);
    /* ╭────────────────────────────────────────────────────────────╮
       │ PICK MAIN LOOP                                             │
       ╰────────────────────────────────────────────────────────────╯ */
    pick->obj_idx = 0;
    pick->x = 1;
    mousemask(BUTTON1_CLICKED | BUTTON1_DOUBLE_CLICKED, NULL);
    picker(init);
    if (pick->select_cnt > 0) {
        if (pick->f_out_spec && pick->out_spec[0])
            rc = output_objects(pick);
        if (pick->f_cmd_spec && pick->cmd_spec[0])
            rc = exec_objects(init);
    }
    return (rc);
}
/* ╭────────────────────────────────────────────────────────────╮
   │ SAVE_OBJECT                                                │
   ╰────────────────────────────────────────────────────────────╯ */
void save_object(Pick *pick, char *s) {
    int tbl_col_width = 0;
    char *p;

    if (pick->obj_idx < OBJ_MAXCNT) {
        p = s;
        while (*p != '\0' && *p != '\n' && *p != '\r' &&
               tbl_col_width < OBJ_MAXLEN) {
            p++;
            tbl_col_width++;
        }
        *p = '\0';
        if (tbl_col_width > pick->tbl_col_width)
            pick->tbl_col_width = tbl_col_width;
        pick->object[pick->obj_idx] = (char *)malloc(tbl_col_width + 1);
        p = pick->object[pick->obj_idx];
        if (p == NULL) {
            sprintf(errmsg,
                    "fpick: pick->object[pick->obj_idx]=(char *)malloc(%d)\n",
                    tbl_col_width + 1);
            abend(-1, errmsg);
        } else
            strncpy(p, s, tbl_col_width + 1);
        pick->f_selected[pick->obj_idx] = FALSE;
        pick->obj_idx++;
    }
}

/* ╭────────────────────────────────────────────────────────────────╮
   │ PICKER                                                         │
   ╰────────────────────────────────────────────────────────────────╯ */
int picker(Init *init) {
    int display_tbl_page;

    pick = init->pick;
    MEVENT event;
    event.y = event.x = -1;
    cmd_key = 0;
    while (1) {
        tcflush(tty_fd, TCIFLUSH);
        if (cmd_key == 0)
            cmd_key = wgetch(pick->win);
        switch (cmd_key) {
        case 'q':
        case 'Q':
        case KEY_F(9):
            return -1;
        case 'h':
        case 'H': // Help
            display_pick_help(init);
            display_page(pick);
            reverse_object(pick);
            cmd_key = 0;
            break;
        case ' ':
        case 't':
        case 'T': // Toggle
            toggle_object(pick);
            if (pick->select_cnt == pick->select_max)
                return pick->select_cnt;
            cmd_key = 0;
            break;
        case '\015': // CR
        case KEY_F(10):
        case KEY_ENTER:
            return pick->select_cnt;
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
        case KEY_RIGHT:
        case '\014':
            mvwaddstr_fill(pick->win, pick->y, pick->x,
                           pick->object[pick->obj_idx], pick->tbl_col_width);

            display_tbl_page = pick->tbl_page;

            // pick->obj_idx += pick->tbl_lines -> next column

            if ((pick->obj_idx + pick->tbl_lines) < (pick->obj_cnt - 1))
                pick->obj_idx += pick->tbl_lines;

            if (pick->tbl_col < pick->tbl_cols - 1 &&
                pick->obj_idx + pick->pg_lines < pick->obj_cnt) {
                pick->tbl_col++;
            } else {
                pick->tbl_col = 0;
                if (pick->tbl_page < pick->tbl_pages - 1) {
                    pick->tbl_page = 0;
                }
            }
            pick->obj_idx = pick->tbl_page * pick->pg_lines * pick->tbl_cols +
                            pick->tbl_col * pick->pg_lines + pick->tbl_line;

            if (display_tbl_page != pick->tbl_page)
                display_page(pick);
            reverse_object(pick);
            cmd_key = 0;
            break;
        case KEY_LEFT:
        case '\010':
        case KEY_BACKSPACE:
            mvwaddstr_fill(pick->win, pick->y, pick->x,
                           pick->object[pick->obj_idx], pick->tbl_col_width);
            if (pick->tbl_col == 0) {
                pick->tbl_col = pick->tbl_cols - 1;
                if (pick->pg_line > 0)
                    pick->pg_line--;
                else {
                    pick->tbl_line = pick->pg_lines - 1;
                    if (pick->tbl_page > 0)
                        pick->tbl_page--;
                    else
                        pick->tbl_page = pick->tbl_pages - 1;
                }
            } else
                pick->tbl_col--;
            pick->obj_idx = pick->tbl_page * pick->pg_lines * pick->tbl_cols +
                            pick->tbl_col * pick->pg_lines + pick->tbl_line;
            reverse_object(pick);
            cmd_key = 0;
            break;
        case KEY_DOWN:
        case '\012':
            mvwaddstr_fill(pick->win, pick->y, pick->x,
                           pick->object[pick->obj_idx], pick->tbl_col_width);
            display_tbl_page = pick->tbl_page;

            // pick->obj_idx++ column down
            if ((pick->obj_idx + pick->tbl_cols) < (pick->obj_cnt - 1))
                pick->obj_idx++;
            pick->tbl_page = pick->obj_idx / (pick->pg_lines * pick->tbl_cols);
            pick->tbl_line = pick->obj_idx % pick->pg_lines;
            pick->tbl_col = (pick->obj_idx / pick->pg_lines) % pick->tbl_cols;
            pick->y = pick->tbl_line;
            pick->obj_idx = pick->tbl_page * pick->pg_lines * pick->tbl_cols +
                            pick->tbl_col * pick->pg_lines + pick->tbl_line;
            if (display_tbl_page != pick->tbl_page) {
                display_page(pick);
            }
            reverse_object(pick);
            cmd_key = 0;
            break;
        case KEY_UP:
        case '\013':
            mvwaddstr_fill(pick->win, pick->y, pick->x,
                           pick->object[pick->obj_idx], pick->tbl_col_width);
            display_tbl_page = pick->tbl_page;
            if (pick->obj_idx > 0)
                pick->obj_idx--;
            pick->tbl_page = pick->obj_idx / (pick->pg_lines * pick->tbl_cols);
            pick->tbl_line = pick->obj_idx % pick->pg_lines;
            pick->tbl_col = (pick->obj_idx / pick->pg_lines) % pick->tbl_cols;
            pick->y = pick->tbl_line;
            pick->obj_idx = pick->tbl_page * pick->pg_lines * pick->tbl_cols +
                            pick->tbl_col * pick->pg_lines + pick->tbl_line;
            if (display_tbl_page != pick->tbl_page)
                display_page(pick);
            reverse_object(pick);
            cmd_key = 0;
            break;
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
        case KEY_LL:
            pick->tbl_page = pick->tbl_pages - 1;
            pick->obj_idx = pick->tbl_page * pick->pg_lines * pick->tbl_cols +
                            pick->tbl_cols * pick->pg_line + pick->tbl_col;
            display_page(pick);
            reverse_object(pick);
            cmd_key = 0;
            break;
        /*  ╭───────────────────────────────────────────────────────╮
            │ PICK MOUSE EVENT                                      │
            ╰───────────────────────────────────────────────────────╯ */
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
/* ╭────────────────────────────────────────────────────────────────╮
   │ DISPLAY_PAGE                                                   │
   ╰────────────────────────────────────────────────────────────────╯ */
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
/*  ╭───────────────────────────────────────────────────────────────╮
    │ PICK_DISPLAY_CHYRON                                           │
    ╰───────────────────────────────────────────────────────────────╯ */
void pick_display_chyron(Pick *pick) {
    int l;
    char tmp_str[MAXLEN];
    ssnprintf(tmp_str, MAXLEN - 65, "%s| Page %d of %d ", pick->chyron_s,
              pick->tbl_page + 1, pick->tbl_pages);
    l = strlen(tmp_str);
    wattron(pick->win, A_REVERSE);
    mvwaddstr(pick->win, pick->pg_lines, 0, tmp_str);
    wattroff(pick->win, A_REVERSE);
    wmove(pick->win, pick->pg_lines, l);
    wclrtoeol(pick->win);
}
/* ╭────────────────────────────────────────────────────────────────╮
   │ REVERSE_OBJECT                                                 │
   ╰────────────────────────────────────────────────────────────────╯ */
void reverse_object(Pick *pick) {
    pick->x = pick->tbl_col * (pick->tbl_col_width + 1) + 1;
    wmove(pick->win, pick->y, pick->x);
    wattron(pick->win, A_REVERSE);
    mvwaddstr_fill(pick->win, pick->y, pick->x, pick->object[pick->obj_idx],
                   pick->tbl_col_width);
    wattroff(pick->win, A_REVERSE);
    wrefresh(pick->win);
    wmove(pick->win, pick->y, pick->x - 1);
}
/* ╭────────────────────────────────────────────────────────────────╮
   │ UNREVERSE_OBJECT                                               │
   ╰────────────────────────────────────────────────────────────────╯ */
void unreverse_object(Pick *pick) {
    pick->x = pick->tbl_col * (pick->tbl_col_width + 1) + 1;
    wmove(pick->win, pick->y, pick->x);
    mvwaddstr_fill(pick->win, pick->y, pick->x, pick->object[pick->obj_idx],
                   pick->tbl_col_width);
    wrefresh(pick->win);
    wmove(pick->win, pick->y, pick->x - 1);
}
/* ╭────────────────────────────────────────────────────────────────╮
   │ TOGGLE_OBJECTS                                                 │
   ╰────────────────────────────────────────────────────────────────╯ */
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
/* ╭────────────────────────────────────────────────────────────────╮
   │ OUTPUT_OBJECTS                                                 │
   ╰────────────────────────────────────────────────────────────────╯ */
int output_objects(Pick *pick) {
    int m;
    if ((pick->out_fp = fopen(pick->out_spec, "w")) == NULL) {
        /*  ╭───────────────────────────────────────────────╮
            │ OUT SPEC IS A FILE                            │
            ╰───────────────────────────────────────────────╯ */
        m = MAXLEN - 30;
        strncpy(tmp_str, "Can't open pick output file: ", m);
        m -= strlen(pick->in_spec);
        strncat(tmp_str, pick->out_spec, m);
    }
    for (pick->obj_idx = 0; pick->obj_idx < pick->obj_cnt; pick->obj_idx++) {
        if (pick->f_selected[pick->obj_idx])
            fprintf(stdout, "%s\n", pick->object[pick->obj_idx]);
    }
    fflush(stdout);
    fclose(pick->out_fp);
    return (0);
}
/* ╭────────────────────────────────────────────────────────────────╮
   │ EXEC_OBJECTS                                                   │
   ╰────────────────────────────────────────────────────────────────╯ */
int exec_objects(Init *init) {
    int rc = -1;
    int m;
    int margc;
    char *margv[MAXARGS];
    char tmp_str[MAXLEN];
    int i = 0;
    pid_t pid = 0;

    // ╭────────────────────────────────────────────────────╮
    // │ Are we providing input via pipe?                   │
    // ╰────────────────────────────────────────────────────╯
    if (pick->cmd_spec[0] == '\0')
        return -1;
    if (pick->cmd_spec[0] == '\\' || pick->cmd_spec[0] == '\"') {
        /* Remove surrounding quotes if present */
        size_t len = strlen(pick->cmd_spec);
        if (len > 1 && pick->cmd_spec[len - 1] == '\"') {
            memmove(pick->cmd_spec, pick->cmd_spec + 1, len - 2);
            pick->cmd_spec[len - 2] = '\0';
        }
    }
    margc = str_to_args(margv, pick->cmd_spec, MAXARGS - 1);
    base_name(tmp_str, margv[0]);
    while (pagers_editors[i][0] != '\0') {
        if (!strcmp(tmp_str, pagers_editors[i]))
            break;
        i++;
    }
    if (pagers_editors[i][0] != '\0') {
        for (pick->obj_idx = 0; pick->obj_idx < pick->obj_cnt;
             pick->obj_idx++) {
            if (pick->f_selected[pick->obj_idx])
                if (margc < MAXARGS)
                    margv[margc++] = strdup(pick->object[pick->obj_idx]);
        }
        margv[margc] = NULL;
        if (!strcmp(tmp_str, "mview")) {
            if (pick->title[0] == '\0')
                strncpy(pick->title, "Pick", MAXLEN - 1);
            strip_quotes(pick->title);
            mview(init, margc, margv, 0, 0, pick->begy + 1, pick->begx + 4,
                  pick->title);
        }
    } else {
        tmp_str[0] = '\0';
        if (pick->f_multiple_cmd_args) {
            for (pick->obj_idx = 0; pick->obj_idx < pick->obj_cnt;
                 pick->obj_idx++) {
                if (pick->f_selected[pick->obj_idx]) {
                    if (tmp_str[0] != '\0')
                        strncat(tmp_str, " ", MAXLEN - strlen(tmp_str) - 1);
                    strncat(tmp_str, pick->object[pick->obj_idx],
                            MAXLEN - strlen(tmp_str) - 1);
                }
            }
            margv[margc++] = strdup(tmp_str);
        } else {
            for (pick->obj_idx = 0; pick->obj_idx < pick->obj_cnt;
                 pick->obj_idx++) {
                if (pick->f_selected[pick->obj_idx]) {
                    if (margc < MAXARGS)
                        margv[margc++] = strdup(pick->object[pick->obj_idx]);
                }
            }
        }
    }
    margv[margc] = NULL;
    if ((pid = fork()) == -1) {
        Perror("fork() failed in exec_objects");
        return (1);
    }
    if (pid == 0) { // Child
        // dup2(init->stdin_fd, STDIN_FILENO);
        // dup2(init->stdout_fd, STDOUT_FILENO);
        execvp(margv[0], margv);
        m = MAXLEN - 26;
        strncpy(tmp_str, "Can't exec pick cmd_spec: ", m);
        m -= strlen(margv[0]);
        strncat(tmp_str, margv[0], m);
        Perror(tmp_str);
        exit(EXIT_FAILURE);
    }
    // Parent
    margc = 0;
    while (margv[margc] != NULL)
        free(margv[margc++]);
    waitpid(pid, NULL, 0);
    restore_curses_tioctl();
    sig_prog_mode();
    keypad(pick->win, true);
    restore_wins();
    return rc;
}
/* ╭────────────────────────────────────────────────────────────────╮
   │ OPEN_PICK_WIN                                                  │
   ╰────────────────────────────────────────────────────────────────╯ */
int open_pick_win(Init *init) {
    char tmp_str[MAXLEN];
    pick = init->pick;
    if (win_new(pick->win_lines, pick->win_width, pick->begy, pick->begx,
                pick->title)) {
        ssnprintf(tmp_str, MAXLEN - 65, "win_new(%d, %d, %d, %d, %s) failed",
                  pick->win_lines, pick->win_width, pick->begy, pick->begx,
                  pick->title);
        Perror(tmp_str);

        return (1);
    }
    pick->win = win_win[win_ptr];
    pick->box = win_box[win_ptr];
    scrollok(pick->win, FALSE);
    keypad(pick->win, true);
    // idcok(pick->win, true);
    return 0;
}
/* ╭────────────────────────────────────────────────────────────────╮
   │ DISPLAY_PICK_HELP                                              │
   ╰────────────────────────────────────────────────────────────────╯ */
void display_pick_help(Init *init) {
    pick = init->pick;
    char tmp_spec[MAXLEN];
    int margc = 0;
    char *margv[MAXARGS];
    strncpy(tmp_spec, "~/menuapp/help/pick.help", MAXLEN - 1);
    pick->f_help_spec = verify_spec_arg(
        pick->help_spec, tmp_spec, "~/menuapp/help", init->mapp_help, R_OK);
    margv[margc++] = "mview";
    margv[margc++] = pick->help_spec;
    margv[margc] = NULL;
    mview(init, margc, margv, 0, 0, pick->begy + 1, pick->begx + 4,
          pick->title);
}
/* ╭────────────────────────────────────────────────────────────────╮
   │ CALC_TBL                                                       │
   ╰────────────────────────────────────────────────────────────────╯ */
void calc_tbl_coord(int pg_lines, int tbl_cols, int obj_idx, int *tbl_page,
                    int *tbl_line, int *tbl_col) {
    *tbl_page = obj_idx / (pg_lines * tbl_cols);
    *tbl_line = (obj_idx / tbl_cols) % pg_lines;
    *tbl_col = obj_idx % tbl_cols;
}

int calc_tbl_page(int pg_lines, int tbl_cols, int obj_idx) {
    return obj_idx / (pg_lines * tbl_cols);
}
int calc_tbl_line(int tbl_cols, int obj_idx) {
    return (obj_idx / tbl_cols) % pg_lines;
}
int calc_tbl_col(int obj_idx) { return obj_idx % tbl_cols; }

int calc_obj_idx(int tbl_pages, int pg_lines, int tbl_cols, int tbl_page,
                 int tbl_line, int tbl_col) {
    obj_idx = tbl_page * (pg_lines * tbl_cols) + tbl_line * tbl_cols + tbl_col;
    return obj_idx;
}
