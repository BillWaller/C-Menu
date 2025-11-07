/* fpick.c
 * pick from a list of choices for MENU
 * Bill Waller
 * billxwaller@gmail.com
 */

#include "menu.h"
#include <stdlib.h>
#include <string.h>
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
int picker(Pick *);
void display_page(Pick *);
void reverse_object(Pick *);
void toggle_object(Pick *);
int output_objects(Pick *);
int exec_objects(Pick *);
int open_pick_win(Pick *);

/* ╭───────────────────────────────────────────────────────────────────╮
   │ INIT_PICK                                                         │
   ╰───────────────────────────────────────────────────────────────────╯ */
int init_pick(Init *init, int argc, char **argv, int begy, int begx) {
    Pick *pick = new_pick(init, argc, argv, begy, begx);
    char tmp_str[MAXLEN];

    pick->win_lines = lines;
    pick->win_width = cols;
    pick->begy = begy + 1;
    pick->begx = begx + 4;
    strncpy(pick->title, init->title, MAXLEN - 1);
    if (pick->f_in_spec) {
        if ((pick->in_fp = fopen(pick->in_spec, "rb")) == NULL) {
            strncpy(tmp_str, "Can\'t open input ", MAXLEN - 25);
            strncat(tmp_str, pick->in_spec, MAXLEN - 25);
            display_error_message(tmp_str);
            return (1);
        }
    } else if (argc == 1 || (argv[1][0] == '-' && argv[1][1] == '\0')) {
        if (isatty(0)) {
            display_error_message("pick: Can't take input from tty");
            return (1);
        }
        pick->in_fp = stdin;
    }
    if (pick->f_out_spec) {
        if ((pick->out_fp = fopen(pick->out_spec, "w")) == NULL) {
            strncpy(tmp_str, "Can\'t open output ", MAXLEN - 5);
            strncat(tmp_str, pick->out_spec, MAXLEN - 5);
            display_error_message(tmp_str);
            return (1);
        }
    } else {
        if (!pick->f_cmd_spec) {
            pick->out_fp = stdout;
        }
    }
    pick_engine(init);
    if (pick->win) {
        win_del();
        pick->win = win_win[win_ptr];
        pick->box = win_box[win_ptr];
    }
    free(pick);
    return 0;
}
/* ╭───────────────────────────────────────────────────────────────────╮
   │ PICK_ENGINE                                                       │
   ╰───────────────────────────────────────────────────────────────────╯ */
int pick_engine(Init *init) {
    int i, rc;
    int maxx, win_maxx;

    Pick *pick = init->pick;
    pick->obj_cnt = pick->tbl_pages = pick->pg_lines = pick->tbl_cols = 0;
    pick->obj_idx = pick->tbl_page = pick->y = pick->tbl_col = pick->x =
        pick->tbl_col_width = 0;

    if (pick->in_fp) {
        while (fgets(pick->in_buf, sizeof(pick->in_buf), pick->in_fp) != NULL)
            save_object(pick, pick->in_buf);
    } else
        for (i = 1; i < pick->argc; i++)
            save_object(pick, pick->argv[i]);

    if (!pick->obj_idx)
        return (-1);
    pick->obj_cnt = pick->obj_idx;
    pick->obj_idx = 0;

    /* ╭───────────────────────────────────────────────────────────────────╮
       │ PICK TABLE LAYOUT                                                 │
       ╰───────────────────────────────────────────────────────────────────╯ */
    getmaxyx(stdscr, i, maxx);

    win_maxx = maxx * 8 / 10;
    pick->tbl_cols = win_maxx / pick->tbl_col_width;
    pick->win_width = pick->tbl_cols * (pick->tbl_col_width + 1) + 2;
    pick->pg_lines = (pick->obj_cnt - 1) / pick->tbl_cols + 1;
    pick->win_lines = pick->pg_lines + 1;
    if (pick->tbl_cols > pick->obj_cnt)
        pick->tbl_cols = pick->obj_cnt;
    pick->tbl_pages = pick->obj_cnt / (pick->tbl_cols * pick->pg_lines) + 1;
    pick->tbl_page = 0;
    rc = open_pick_win(pick);
    if (rc)
        return (rc);
    display_page(pick);
    reverse_object(pick);

    /* ╭───────────────────────────────────────────────────────────────────╮
       │ PICK MAIN LOOP                                                    │
       ╰───────────────────────────────────────────────────────────────────╯ */
    pick->obj_idx = 0;
    while (!picker((Pick *)pick))
        ;
    if (pick->obj_idx > 0) {
        if (pick->f_out_spec && pick->out_spec[0])
            rc = output_objects(pick);
        if (pick->f_cmd_spec && pick->cmd_spec[0])
            rc = exec_objects(pick);
    }
    for (pick->obj_idx = 0; pick->obj_idx < pick->obj_cnt; pick->obj_idx++)
        free(pick->object[pick->obj_idx]);
    return (rc);
}

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

/* ╭───────────────────────────────────────────────────────────────────╮
   │ PICKER                                                            │
   ╰───────────────────────────────────────────────────────────────────╯ */
int picker(Pick *pick) {
    int tbl_col, tbl_line, tbl_page;
    int obj_idx, display_tbl_page;
    cmd_key = wgetch(pick->win);
    switch (cmd_key) {
    case 'q':
    case 'Q':
    case '\033':
    case KEY_F(9):
        return -1;
    case '\040': // SPACE
        toggle_object(pick);
        break;
    case '\15': // CR
    case KEY_F(10):
    case KEY_ENTER:
        if (pick->select_cnt > 0)
            return pick->select_cnt;
        return 0;
    case KEY_END:
        mvwaddstr_fill(pick->win, pick->y, pick->x, pick->object[pick->obj_idx],
                       pick->tbl_col_width);
        display_tbl_page = pick->tbl_page;
        pick->obj_idx = pick->obj_cnt - 1;
        pick->tbl_page = pick->obj_idx / (pick->pg_lines * pick->tbl_cols);
        pick->tbl_line = (pick->obj_idx / pick->tbl_cols) % pick->pg_lines;
        pick->tbl_col = pick->obj_idx % pick->tbl_cols;
        pick->y = pick->tbl_line;
        if (display_tbl_page != pick->tbl_page) {
            display_page(pick);
            display_tbl_page = pick->tbl_page;
        }
        reverse_object(pick);
        break;
    case KEY_RIGHT:
    case KEY_CTLL:
        mvwaddstr_fill(pick->win, pick->y, pick->x, pick->object[pick->obj_idx],
                       pick->tbl_col_width);
        display_tbl_page = pick->tbl_page;
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
                        pick->tbl_col * pick->pg_lines + pick->y;
        if (display_tbl_page != pick->tbl_page) {
            display_page(pick);
            display_tbl_page = pick->tbl_page;
        }
        reverse_object(pick);
        break;
    case KEY_LEFT:
    case KEY_CTLH:
    case KEY_BACKSPACE:
        mvwaddstr_fill(pick->win, pick->y, pick->x, pick->object[pick->obj_idx],
                       pick->tbl_col_width);
        pick->obj_idx = pick->tbl_page * pick->pg_lines * pick->tbl_cols +
                        pick->tbl_col * pick->pg_lines + pick->y;

        tbl_col = pick->tbl_col;
        tbl_line = pick->tbl_line;
        tbl_page = pick->tbl_page;
        tbl_col--;
        if (tbl_col < 0) {
            tbl_col = pick->tbl_cols - 1;
            tbl_line--;
            if (tbl_line < 0) {
                tbl_line = pick->pg_lines - 1;
                tbl_page--;
                if (tbl_page < 0)
                    tbl_page = pick->tbl_pages - 1;
            }
        }
        obj_idx = tbl_page * pick->pg_lines * pick->tbl_cols +
                  tbl_line * pick->tbl_cols + tbl_col;
        if (obj_idx < pick->obj_cnt) {
            pick->obj_idx = obj_idx;
            pick->tbl_col = tbl_col;
            pick->y = tbl_line;
            pick->tbl_page = tbl_page;
        }
        reverse_object(pick);
        break;
    case KEY_DOWN:
    case KEY_CTLJ:
        mvwaddstr_fill(pick->win, pick->y, pick->x, pick->object[pick->obj_idx],
                       pick->tbl_col_width);
        display_tbl_page = pick->tbl_page;
        if (pick->obj_idx < pick->obj_cnt - 1) {
            pick->obj_idx++;
        } else {
            pick->obj_idx = 0;
        }
        pick->tbl_page = pick->obj_idx / (pick->pg_lines * pick->tbl_cols);
        pick->tbl_line = pick->obj_idx % pick->pg_lines;
        pick->tbl_col = pick->obj_idx / pick->pg_lines;

        pick->y = pick->tbl_line;
        if (display_tbl_page != pick->tbl_page) {
            display_page(pick);
            display_tbl_page = pick->tbl_page;
        }
        reverse_object(pick);
        break;
    case KEY_UP:
    case KEY_CTLK:
        mvwaddstr_fill(pick->win, pick->y, pick->x, pick->object[pick->obj_idx],
                       pick->tbl_col_width);
        display_tbl_page = pick->tbl_page;
        pick->tbl_line = (pick->obj_idx / pick->tbl_cols) % pick->pg_lines;
        pick->tbl_lines = pick->obj_idx % pick->tbl_cols;
        if (pick->tbl_line > 0) {
            pick->obj_idx -= pick->tbl_cols;
            pick->tbl_page = pick->obj_idx / (pick->pg_lines * pick->tbl_cols);
            pick->tbl_line = (pick->obj_idx / pick->tbl_cols) % pick->pg_lines;
            pick->tbl_col = pick->obj_idx % pick->tbl_cols;
        } else {
            pick->tbl_page = pick->tbl_pages - 1;
            pick->tbl_line = pick->pg_lines - 1;
            pick->tbl_col = pick->tbl_cols - 1;
            pick->obj_idx = pick->tbl_page * pick->pg_lines * pick->tbl_cols +
                            pick->tbl_line * pick->tbl_cols + pick->tbl_col;
            if (pick->obj_idx >= pick->obj_cnt) {
                pick->obj_idx = pick->obj_cnt - 1;
                pick->tbl_page =
                    pick->obj_idx / (pick->pg_lines * pick->tbl_cols);
                pick->tbl_line =
                    (pick->obj_idx / pick->tbl_cols) % pick->pg_lines;
                pick->tbl_col = pick->obj_idx % pick->tbl_cols;
            }
        }
        pick->y = pick->tbl_line;
        if (display_tbl_page != pick->tbl_page) {
            display_page(pick);
            display_tbl_page = pick->tbl_page;
        }
        reverse_object(pick);
        break;
    case KEY_NPAGE:
    case KEY_CTLF:
        pick->tbl_page++;
        if (pick->tbl_page >= pick->tbl_pages)
            pick->tbl_page = 0;
        display_page(pick);
        reverse_object(pick);
        break;
    case KEY_PPAGE:
    case KEY_CTLB:
        if (pick->tbl_page <= 0)
            pick->tbl_page = pick->tbl_pages;
        pick->tbl_page--;
        display_page(pick);
        reverse_object(pick);
        break;
    case KEY_HOME:
        pick->tbl_page = 0;
        display_page(pick);
        reverse_object(pick);
        break;
    case KEY_LL:
        pick->tbl_page = pick->tbl_pages - 1;
        display_page(pick);
        reverse_object(pick);
        break;
    case KEY_CTLR:
        restore_wins();
    default:
        break;
    }
    return 0;
}

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
    (void)snprintf(tmp_str, MAXLEN - 1,
                   " PgUp PgDn Space Enter Escape %d of %d ",
                   pick->tbl_page + 1, pick->tbl_pages);
    wattron(pick->win, A_REVERSE);
    mvwaddstr_fill(pick->win, pick->win_lines - 1, 0, tmp_str, pick->win_width);
    wattroff(pick->win, A_REVERSE);
}

void reverse_object(Pick *pick) {
    pick->x = pick->tbl_col * (pick->tbl_col_width + 1) + 1;
    wattron(pick->win, A_REVERSE);
    wmove(pick->win, pick->y, pick->x);
    waddch(pick->win, pick->object[pick->obj_idx][0]);
    wrefresh(pick->win);
    mvwaddstr_fill(pick->win, pick->y, pick->x, pick->object[pick->obj_idx],
                   pick->tbl_col_width);
    wmove(pick->win, pick->y, pick->x - 1);
    wattroff(pick->win, A_REVERSE);
}

void toggle_object(Pick *pick) {
    pick->x = pick->tbl_col * (pick->tbl_col_width + 1) + 1;
    if (pick->f_selected[pick->obj_idx]) {
        pick->select_cnt--;
        pick->f_selected[pick->obj_idx] = FALSE;
        mvwaddstr(pick->win, pick->y, pick->x - 1, " ");
    } else {
        pick->select_cnt++;
        pick->f_selected[pick->obj_idx] = TRUE;
        mvwaddstr(pick->win, pick->y, pick->x - 1, "*");
    }
}

int output_objects(Pick *pick) {
    for (pick->obj_idx = 0; pick->obj_idx < pick->obj_cnt; pick->obj_idx++) {
        if (pick->f_selected[pick->obj_idx])
            fprintf(pick->out_fp, "%s\n", pick->object[pick->obj_idx]);
    }
    return (0);
}

int exec_objects(Pick *pick) {
    int rc = -1;
    int margc;
    char *optarg;
    char *margv[MAXARGS];
    char *s;
    char tmp_str[MAXLEN];

    fprintf(stderr, "\n");
    fflush(stderr);
    wclear(stdscr);
    wmove(stdscr, 0, 0);
    wrefresh(stdscr);
    s = pick->cmd_spec;
    for (margc = 0; margc < MAXARGS; margc++) {
        optarg = strtok(s, " \t");
        if (*optarg == '\0')
            break;
        margv[margc++] = strdup(optarg);
        s = (char *)0;
    }
    strncpy(tmp_str, optarg, MAXLEN - 1);
    margv[margc] = NULL;
    base_name(tmp_str, optarg);
    if (tmp_str[0] && strcmp(tmp_str, "view") == 0) {
        for (pick->obj_idx = 0; pick->obj_idx < pick->obj_cnt;
             pick->obj_idx++) {
            if (pick->f_selected[pick->obj_idx])
                if (margc < MAXARGS)
                    margv[margc++] = pick->object[pick->obj_idx];
        }
        margv[margc] = NULL;
        mview(init, margc, margv, 10, 40, pick->begy + 1, pick->begx + 4);
    } else {
        if (pick->f_multiple_cmd_args) {
            for (pick->obj_idx = 0; pick->obj_idx < pick->obj_cnt;
                 pick->obj_idx++) {
                if (pick->f_selected[pick->obj_idx])
                    if (margc < MAXARGS)
                        margv[margc++] = pick->object[pick->obj_idx];
            }
            margv[margc] = NULL;
            rc = fork_exec(margv);
            for (pick->obj_idx = 0; pick->obj_idx < pick->obj_cnt;
                 pick->obj_idx++)
                free(pick->object[pick->obj_idx]);
        } else {
            for (pick->obj_idx = 0; pick->obj_idx < pick->obj_cnt;
                 pick->obj_idx++) {
                if (pick->f_selected[pick->obj_idx]) {
                    if (margc < MAXARGS)
                        margv[margc] = pick->object[pick->obj_idx];
                    margv[margc + 1] = NULL;
                    rc = fork_exec(margv);
                }
                free(pick->object[pick->obj_idx]);
            }
        }
    }
    restore_wins();
    return (rc);
}

int open_pick_win(Pick *pick) {
    char tmp_str[MAXLEN];
    if (win_new(pick->win_lines, pick->win_width, pick->begy, pick->begx,
                "pick")) {
        sprintf(tmp_str, "win_new(%d, %d, %d, %d, %s) failed", pick->win_lines,
                pick->win_width, pick->begy, pick->begx, "pick");
        display_error_message(tmp_str);
        free(pick);
        return (1);
    }
    pick->win = win_win[win_ptr];
    pick->box = win_box[win_ptr];
    scrollok(pick->win, FALSE);
    keypad(pick->win, TRUE);
    // idcok(pick->win, TRUE);
    return 0;
}

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
