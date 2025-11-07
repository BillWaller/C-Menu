/* fpick.c
 * pick from a list of choices for MENU
 * Bill Waller
 * billxwaller@gmail.com
 */

#include "menu.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int tab_col, tab_line, tab_page, tab_cols, tab_lines, tab_pages;
int obj_idx, calculated_idx;
int calc_obj_idx(int, int, int, int, int, int);
void calc_tab_coord(int, int, int, int *, int *, int *);
int calc_tab_page(int, int, int);
int calc_tab_line(int, int);
int calc_tab_col(int);
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
    pick->scr_lines = lines;
    pick->scr_cols = cols;
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
    if (pick->out_spec[0]) {
        if ((pick->out_fp = fopen(pick->out_spec, "w")) == NULL) {
            strncpy(tmp_str, "Can\'t open output ", MAXLEN - 5);
            strncat(tmp_str, pick->out_spec, MAXLEN - 5);
            display_error_message(tmp_str);
            return (1);
        }
    } else {
        if (pick->cmd_spec[0]) {
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
    int i, rc, tab_lines;

    Pick *pick = init->pick;
    pick->obj_cnt = pick->tab_pages = pick->tab_lines = pick->tab_cols = 0;
    pick->obj_idx = pick->tab_page = pick->y = pick->tab_col = pick->x =
        pick->tab_width = 0;

    if (pick->in_fp) {
        while (fgets(pick->in_buf, sizeof(pick->in_buf), pick->in_fp) != NULL)
            save_object(pick, pick->in_buf);
    } else
        // if no input file, use command line args
        for (i = 1; i < pick->argc; i++)
            save_object(pick, pick->argv[i]);

    // resolve layout
    if (!pick->obj_idx)
        return (-1);
    pick->obj_cnt = pick->obj_idx;
    pick->obj_idx = 0;
    if (pick->tab_width > pick->scr_cols - 1)
        pick->tab_width = pick->scr_cols - 1;
    pick->tab_cols = pick->scr_cols / (pick->tab_width + 1);
    if (pick->scr_cols > pick->tab_width)
        pick->scr_cols = pick->tab_cols * (pick->tab_width + 1);
    if (!pick->tab_cols)
        pick->tab_cols = 1;
    if (pick->tab_cols > pick->obj_cnt)
        pick->tab_cols = pick->obj_cnt;
    tab_lines = (pick->obj_cnt - 1) / pick->tab_cols + 1;
    if (tab_lines > pick->scr_lines - 1)
        pick->tab_lines = pick->scr_lines - 1;
    else
        pick->tab_lines = tab_lines;
    pick->tab_pages = pick->obj_cnt / (pick->tab_cols * pick->tab_lines) + 1;
    pick->tab_page = 0;
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
        if (!pick->f_out_spec && pick->out_spec[0])
            rc = output_objects(pick);
        if (pick->f_cmd_spec && pick->cmd_spec[0])
            rc = exec_objects(pick);
    } else
        for (pick->obj_idx = 0; pick->obj_idx < pick->obj_cnt; pick->obj_idx++)
            free(pick->object[pick->obj_idx]);
    return (rc);
}

void save_object(Pick *pick, char *s) {
    int l = 0;
    char *p;

    if (pick->obj_idx < OBJ_MAXCNT) {
        p = s;
        while (*p != '\0' && *p != '\n' && *p != '\r' && l < OBJ_MAXLEN) {
            p++;
            l++;
        }
        *p = '\0';
        if (l > pick->tab_width)
            pick->tab_width = l;
        pick->object[pick->obj_idx] = (char *)malloc(l + 1);
        p = pick->object[pick->obj_idx];
        if (p == NULL) {
            sprintf(errmsg,
                    "fpick: pick->object[pick->obj_idx]=(char *)malloc(%d)\n",
                    l + 1);
            abend(-1, errmsg);
        } else
            strncpy(p, s, l + 1);
        pick->f_selected[pick->obj_idx] = FALSE;
        pick->obj_idx++;
    }
}

/* ╭───────────────────────────────────────────────────────────────────╮
   │ PICKER                                                            │
   ╰───────────────────────────────────────────────────────────────────╯ */
int picker(Pick *pick) {
    int tab_col, tab_line, tab_page;
    int obj_idx, display_tab_page;
    cmd_key = wgetch(pick->win);
    switch (cmd_key) {
    case 'q':
    case 'Q':
    case '\033':
    case KEY_F(9):
        return 0;
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
                       pick->tab_width);

        pick->obj_idx = pick->tab_page * pick->tab_lines * pick->tab_cols +
                        pick->tab_col * pick->tab_lines + (pick->tab_lines - 1);
        while (pick->obj_idx >= pick->obj_cnt) {
            pick->tab_page--;
            if (pick->tab_page < 0)
                pick->tab_page = pick->tab_pages - 1;
            display_page(pick);
            pick->obj_idx = pick->tab_page * pick->tab_lines * pick->tab_cols +
                            pick->tab_col * pick->tab_lines +
                            (pick->tab_lines - 1);
        }
        pick->y = pick->tab_lines - 1;
        reverse_object(pick);
        toggle_object(pick);
        if (pick->select_max > 0 && pick->select_idx >= pick->select_max)
            return pick->select_idx;
        break;
    case KEY_RIGHT:
    case KEY_CTLL:
        mvwaddstr_fill(pick->win, pick->y, pick->x, pick->object[pick->obj_idx],
                       pick->tab_width);
        pick->obj_idx = pick->tab_page * pick->tab_lines * pick->tab_cols +
                        pick->tab_col * pick->tab_lines + pick->y;
        tab_col = pick->tab_col;
        tab_line = pick->tab_line;
        tab_page = pick->tab_page;
        tab_col++;
        if (tab_col == pick->tab_cols) {
            tab_col = 0;
            tab_line++;
            if (tab_line == pick->tab_lines)
                tab_line = 0;
            tab_page++;
            if (tab_page == pick->tab_pages)
                tab_page = 0;
        }
        obj_idx = tab_page * pick->tab_lines * pick->tab_cols +
                  tab_line * pick->tab_cols + tab_col;
        if (obj_idx < pick->obj_cnt) {
            pick->obj_idx = obj_idx;
            pick->tab_col = tab_col;
            pick->y = tab_line;
            pick->tab_page = tab_page;
        }
        reverse_object(pick);
        break;
    case KEY_LEFT:
    case KEY_CTLH:
    case KEY_BACKSPACE:
        mvwaddstr_fill(pick->win, pick->y, pick->x, pick->object[pick->obj_idx],
                       pick->tab_width);
        pick->obj_idx = pick->tab_page * pick->tab_lines * pick->tab_cols +
                        pick->tab_col * pick->tab_lines + pick->y;
        tab_col = pick->tab_col;
        tab_line = pick->tab_line;
        tab_page = pick->tab_page;
        tab_col--;
        if (tab_col < 0) {
            tab_col = pick->tab_cols - 1;
            tab_line--;
            if (tab_line < 0) {
                tab_line = pick->tab_lines - 1;
                tab_page--;
                if (tab_page < 0)
                    tab_page = pick->tab_pages - 1;
            }
        }
        obj_idx = tab_page * pick->tab_lines * pick->tab_cols +
                  tab_line * pick->tab_cols + tab_col;
        if (obj_idx < pick->obj_cnt) {
            pick->obj_idx = obj_idx;
            pick->tab_col = tab_col;
            pick->y = tab_line;
            pick->tab_page = tab_page;
        }
        reverse_object(pick);
        break;
    case KEY_DOWN:
    case KEY_CTLJ:
        mvwaddstr_fill(pick->win, pick->y, pick->x, pick->object[pick->obj_idx],
                       pick->tab_width);
        display_tab_page = pick->tab_page;
        //    tab_page = pick->tab_page;
        //    if (pick->tab_line < pick->tab_lines)
        //        tab_line = pick->tab_line + 1;
        //    if (tab_line >= pick->tab_lines) {
        //        tab_line = 0;
        //        if (pick->tab_col < pick->tab_cols - 1)
        //            tab_col = pick->tab_col + 1;
        //        else {
        //            tab_col = 0;
        //            if (pick->tab_page < pick->tab_pages - 1)
        //                tab_page++;
        //            else
        //                tab_page = 0;
        //        }
        //    }
        if (pick->obj_idx < pick->obj_cnt - 1) {
            pick->obj_idx++;
            //    pick->tab_page = tab_page;
            //    pick->tab_line = tab_line;
            //    pick->tab_col = tab_col;
        } else {
            pick->obj_idx = 0;
            pick->tab_page = 0;
            pick->tab_line = 0;
            pick->tab_col = 0;
        }
        //    obj_idx = tab_page * (pick->tab_lines * pick->tab_cols) +
        //              tab_line * pick->tab_cols + tab_col;
        pick->tab_page = pick->obj_idx / (pick->tab_lines * pick->tab_cols);
        //    if (pick->tab_page != tab_page) {
        //        snprintf(tmp_str, MAXLEN - 1,
        //                 "tab_page mismatch in KEY_DOWN: %d != %d",
        //                 pick->tab_page, tab_page);
        //        display_error_message(tmp_str);
        //    }
        pick->tab_line = (pick->obj_idx / pick->tab_cols) % pick->tab_lines;
        //    if (pick->tab_line != tab_line) {
        //        snprintf(tmp_str, MAXLEN - 1,
        //                 "tab_page mismatch in KEY_DOWN: %d != %d",
        //                 pick->tab_line, tab_line);
        //        display_error_message(tmp_str);
        //    }
        pick->tab_col = pick->obj_idx % pick->tab_cols;
        //    if (pick->tab_col != tab_col) {
        //        snprintf(tmp_str, MAXLEN - 1,
        //                 "tab_page mismatch in KEY_DOWN: %d != %d",
        //                 pick->tab_col, tab_col);
        //        display_error_message(tmp_str);
        //    }
        pick->y = pick->tab_line;
        if (display_tab_page != pick->tab_page) {
            display_page(pick);
            display_tab_page = pick->tab_page;
        }
        reverse_object(pick);
        break;
    case KEY_UP:
    case KEY_CTLK:
        mvwaddstr_fill(pick->win, pick->y, pick->x, pick->object[pick->obj_idx],
                       pick->tab_width);
        tab_line = pick->tab_line;
        if (tab_line > 0) {
            tab_line = pick->tab_line - 1;
        }
        obj_idx = pick->tab_page * (pick->tab_lines * pick->tab_cols) +
                  tab_line * pick->tab_cols + pick->tab_col;
        if (pick->obj_idx < pick->obj_cnt) {
            pick->obj_idx++;
            pick->tab_page = pick->obj_idx / (pick->tab_lines * pick->tab_cols);
            pick->tab_line = (pick->obj_idx / pick->tab_cols) % pick->tab_lines;
            pick->tab_col = pick->obj_idx % pick->tab_cols;
            pick->y = tab_line;
            display_page(pick);
            reverse_object(pick);
        }
        break;
        break;
    case KEY_NPAGE:
    case KEY_CTLF:
        pick->tab_page++;
        if (pick->tab_page >= pick->tab_pages)
            pick->tab_page = 0;
        display_page(pick);
        reverse_object(pick);
        break;
    case KEY_PPAGE:
    case KEY_CTLB:
        if (pick->tab_page <= 0)
            pick->tab_page = pick->tab_pages;
        pick->tab_page--;
        display_page(pick);
        reverse_object(pick);
        break;
    case KEY_HOME:
        pick->tab_page = 0;
        display_page(pick);
        reverse_object(pick);
        break;
    case KEY_LL:
        pick->tab_page = pick->tab_pages - 1;
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

    for (y = 0; y < pick->tab_lines; y++) {
        wmove(pick->win, y, 0);
        wclrtoeol(pick->win);
    }
    pidx = pick->tab_page * pick->tab_lines * pick->tab_cols;
    for (col = 0; col < pick->tab_cols; col++) {
        pick->x = col * (pick->tab_width + 1) + 1;
        for (y = 0; y < pick->tab_lines; y++, pidx++) {
            if (pidx < pick->obj_cnt) {
                if (pick->f_selected[pidx])
                    mvwaddstr(pick->win, y, pick->x - 1, "*");
                mvwaddstr_fill(pick->win, y, pick->x, pick->object[pidx],
                               pick->tab_width);
            }
        }
    }
    (void)snprintf(tmp_str, MAXLEN - 1,
                   " PgUp PgDn Space Enter Escape %d of %d ",
                   pick->tab_page + 1, pick->tab_pages);
    wattron(pick->win, A_REVERSE);
    mvwaddstr_fill(pick->win, pick->scr_lines - 1, 0, tmp_str, pick->scr_cols);
    wattroff(pick->win, A_REVERSE);
}

void reverse_object(Pick *pick) {
    pick->x = pick->tab_col * (pick->tab_width + 1) + 1;
    wattron(pick->win, A_REVERSE);
    wmove(pick->win, pick->y, pick->x);
    waddch(pick->win, pick->object[pick->obj_idx][0]);
    wrefresh(pick->win);
    mvwaddstr_fill(pick->win, pick->y, pick->x, pick->object[pick->obj_idx],
                   pick->tab_width);
    wmove(pick->win, pick->y, pick->x - 1);
    wattroff(pick->win, A_REVERSE);
}

void toggle_object(Pick *pick) {
    pick->x = pick->tab_col * (pick->tab_width + 1) + 1;
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
        free(pick->object[pick->obj_idx]);
    }
    return (0);
}

int exec_objects(Pick *pick) {
    int rc = -1;
    int margc;
    char *margv[MAXARGS];
    char *s;

    fprintf(stderr, "\n");
    fflush(stderr);
    wclear(stdscr);
    wmove(stdscr, 0, 0);
    wrefresh(stdscr);
    s = pick->cmd_spec;
    for (margc = 0; margc < MAXARGS; margc++) {
        if ((margv[margc] = strtok(s, " \t")) == (char *)0)
            break;
        s = (char *)0;
    }
    s = margv[0];
    if (s && strcmp(s, "view") == 0) {
        for (pick->obj_idx = 0; pick->obj_idx < pick->obj_cnt;
             pick->obj_idx++) {
            if (pick->f_selected[pick->obj_idx])
                if (margc < MAXARGS)
                    margv[margc++] = pick->object[pick->obj_idx];
        }
        margv[margc] = NULL;
        mview(init, margc, margv, 10, 40, pick->begy + 1, pick->begx + 4);
        for (pick->obj_idx = 0; pick->obj_idx < pick->obj_cnt; pick->obj_idx++)
            free(pick->object[pick->obj_idx]);
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
    if (win_new(pick->scr_lines, pick->scr_cols, pick->begy, pick->begx,
                "pick")) {
        sprintf(tmp_str, "win_new(%d, %d, %d, %d, %s) failed", pick->scr_lines,
                pick->scr_cols, pick->begy, pick->begx, "pick");
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

void calc_tab_coord(int tab_lines, int tab_cols, int obj_idx, int *tab_page,
                    int *tab_line, int *tab_col) {
    *tab_page = obj_idx / (tab_lines * tab_cols);
    *tab_line = (obj_idx / tab_cols) % tab_lines;
    *tab_col = obj_idx % tab_cols;
}

int calc_tab_page(int tab_lines, int tab_cols, int obj_idx) {
    return obj_idx / (tab_lines * tab_cols);
}
int calc_tab_line(int tab_cols, int obj_idx) {
    return (obj_idx / tab_cols) % tab_lines;
}
int calc_tab_col(int obj_idx) { return obj_idx % tab_cols; }

int calc_obj_idx(int tab_pages, int tab_lines, int tab_cols, int tab_page,
                 int tab_line, int tab_col) {
    obj_idx = tab_page * (tab_lines * tab_cols) + tab_line * tab_cols + tab_col;
    return obj_idx;
}
