/* fpick.c
 * pick from a list of choices for MENU
 * Bill Waller
 * billxwaller@gmail.com
 */

#include "menu.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define M_JUMP 601

int pick_engine(Init *);
void save_object(Pick *, char *);
bool picker(Pick *);
void display_page(Pick *);
void reverse_object(Pick *);
void toggle_object(Pick *);
int output_objects(Pick *);
int exec_objects(Pick *, char *);
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
    pick->select_idx = 0;
    pick->obj_cnt = pick->tab_pages = pick->tab_lines = pick->tab_cols = 0;
    pick->idx = pick->tab_page = pick->y = pick->tab_col = pick->x =
        pick->tab_width = 0;

    if (pick->in_fp) {
        while (fgets(pick->in_buf, sizeof(pick->in_buf), pick->in_fp) != NULL)
            save_object(pick, pick->in_buf);
    } else
        // if no input file, use command line args
        for (i = 1; i < pick->argc; i++)
            save_object(pick, pick->argv[i]);

    // resolve layout
    if (!pick->idx)
        return (-1);
    pick->obj_cnt = pick->idx;
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
    while (!picker((Pick *)pick))
        ;
    if (rc == 1) {
        if (pick->f_cmd_spec && pick->cmd_spec[0])
            rc = output_objects(pick);
        if (!pick->f_out_spec && pick->out_spec[0])
            rc = exec_objects(pick, pick->title);
    } else
        for (pick->idx = 0; pick->idx < pick->obj_cnt; pick->idx++)
            free(pick->object[pick->idx]);
    return (rc);
}

void save_object(Pick *pick, char *s) {
    int l = 0;
    char *p;

    if (pick->idx < OBJ_MAXCNT) {
        p = s;
        while (*p != '\0' && *p != '\n' && *p != '\r' && l < OBJ_MAXLEN) {
            p++;
            l++;
        }
        *p = '\0';
        if (l > pick->tab_width)
            pick->tab_width = l;
        pick->object[pick->idx] = (char *)malloc(l + 1);
        p = pick->object[pick->idx];
        if (p == NULL) {
            sprintf(errmsg,
                    "fpick: pick->object[pick->idx]=(char *)malloc(%d)\n",
                    l + 1);
            abend(-1, errmsg);
        } else {
            while (*s) {
                *p++ = *s++;
            }
            *p = '\0';
        }
        pick->f_selected[pick->idx] = FALSE;
        pick->idx++;
    }
}

/* ╭───────────────────────────────────────────────────────────────────╮
   │ PICKER                                                            │
   ╰───────────────────────────────────────────────────────────────────╯ */
bool picker(Pick *pick) {
    int tline, tcol, tidx;

    cmd_key = wgetch(pick->win);
    switch (cmd_key) {
    case 'q':
    case 'Q':
    case '\033':
    case KEY_F(9):
        return false;
    case 'y':
    case 'Y':
    case '\040': // SPACE
        toggle_object(pick);
        if (pick->select_cnt > 0 && pick->select_idx >= pick->select_cnt)
            return true;
        break;
    case '\15': // CR
    case KEY_F(10):
    case KEY_ENTER:
        if (pick->select_idx > 0)
            return true;
        return false;
    case M_JUMP:
        mvwaddstr_fill(pick->win, pick->y, pick->x, pick->object[pick->idx],
                       pick->tab_width);
        pick->idx = pick->obj_cnt;
        tidx = pick->tab_page * pick->tab_lines * pick->tab_cols +
               pick->tab_col * pick->tab_lines + pick->y;
        if (tidx >= 0 && tidx < pick->obj_cnt) {
            pick->idx = tidx;
        }
        reverse_object(pick);
        toggle_object(pick);
        if (pick->select_cnt > 0 && pick->select_idx >= pick->select_cnt)
            return true;
        break;
    case KEY_RIGHT:
    case key_right:
    case KEY_CTLF:
        mvwaddstr_fill(pick->win, pick->y, pick->x, pick->object[pick->idx],
                       pick->tab_width);
        tcol = pick->tab_col;
        pick->idx = pick->obj_cnt;
        while (pick->idx >= pick->obj_cnt) {
            tcol++;
            if (tcol >= pick->tab_cols)
                tcol = 0;
            tidx = pick->tab_page * pick->tab_lines * pick->tab_cols +
                   tcol * pick->tab_lines + pick->y;
            if (tidx >= 0 && tidx < pick->obj_cnt) {
                pick->idx = tidx;
                pick->tab_col = tcol;
            }
        }
        reverse_object(pick);
        break;
    case KEY_LEFT:
    case key_left:
    case KEY_BACKSPACE:
    case KEY_CTLB:
        mvwaddstr_fill(pick->win, pick->y, pick->x, pick->object[pick->idx],
                       pick->tab_width);
        tcol = pick->tab_col;
        pick->idx = pick->obj_cnt;
        while (pick->idx >= pick->obj_cnt) {
            if (tcol <= 0)
                tcol = pick->tab_cols;
            tcol--;
            tidx = pick->tab_page * pick->tab_lines * pick->tab_cols +
                   tcol * pick->tab_lines + pick->y;
            if (tidx >= 0 && tidx < pick->obj_cnt) {
                pick->idx = tidx;
                pick->tab_col = tcol;
            }
        }
        reverse_object(pick);
        break;
    case KEY_DOWN:
    case key_down:
    case KEY_CTLN:
        mvwaddstr_fill(pick->win, pick->y, pick->x, pick->object[pick->idx],
                       pick->tab_width);
        tline = pick->y;

        pick->idx = pick->obj_cnt;

        while (pick->idx >= pick->obj_cnt) {
            tline++;
            if (tline >= pick->tab_lines) {

                tline = 0;

                pick->tab_page++;
                if (pick->tab_page >= pick->tab_pages)
                    // wrap
                    pick->tab_page = 0;

                // display new page
                tidx = pick->tab_page * pick->tab_lines * pick->tab_cols +
                       pick->tab_col * pick->tab_lines + tline;
                if (tidx >= 0 && tidx < pick->obj_cnt) {
                    pick->idx = tidx;
                    pick->y = tline;
                }
                display_page(pick);

            } else {

                tidx = pick->tab_page * pick->tab_lines * pick->tab_cols +
                       pick->tab_col * pick->tab_lines + tline;
                if (tidx >= 0 && tidx < pick->obj_cnt) {
                    pick->idx = tidx;
                    pick->y = tline;
                }
            }
        }
        reverse_object(pick);
        break;
    case KEY_UP:
    case key_up:
    case KEY_CTLP:
        mvwaddstr_fill(pick->win, pick->y, pick->x, pick->object[pick->idx],
                       pick->tab_width);
        tline = pick->y;
        pick->idx = pick->obj_cnt;
        while (pick->idx >= pick->obj_cnt) {
            if (tline <= 0) {
                tline = pick->tab_lines;
                if (pick->tab_page <= 0)
                    pick->tab_page = pick->tab_pages;
                pick->tab_page--;
                display_page(pick);
            } else {
                tline--;
                tidx = pick->tab_page * pick->tab_lines * pick->tab_cols +
                       pick->tab_col * pick->tab_lines + tline;
                if (tidx >= 0 && tidx < pick->obj_cnt) {
                    pick->idx = tidx;
                    pick->y = tline;
                }
            }
        }
        reverse_object(pick);
        break;
    case KEY_NPAGE:
        pick->tab_page++;
        if (pick->tab_page >= pick->tab_pages)
            pick->tab_page = 0;
        display_page(pick);
        reverse_object(pick);
        break;
    case KEY_PPAGE:
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
    return false;
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
    pick->tab_col = 0;
    pick->y = 0;
    pick->idx = pick->tab_page * pick->tab_lines * pick->tab_cols;
}

void reverse_object(Pick *pick) {
    pick->x = pick->tab_col * (pick->tab_width + 1) + 1;
    wattron(pick->win, A_REVERSE);
    wmove(pick->win, pick->y, pick->x);
    waddch(pick->win, pick->object[pick->idx][0]);
    wrefresh(pick->win);
    mvwaddstr_fill(pick->win, pick->y, pick->x, pick->object[pick->idx],
                   pick->tab_width);
    wmove(pick->win, pick->y, pick->x - 1);
    wattroff(pick->win, A_REVERSE);
}

void toggle_object(Pick *pick) {
    pick->x = pick->tab_col * (pick->tab_width + 1) + 1;
    if (pick->f_selected[pick->idx]) {
        if (pick->select_idx)
            pick->select_idx--;
        pick->f_selected[pick->idx] = FALSE;
        mvwaddstr(pick->win, pick->y, pick->x - 1, " ");
    } else {
        if (pick->select_idx < OBJ_MAXCNT)
            pick->select_idx++;
        pick->f_selected[pick->idx] = TRUE;
        mvwaddstr(pick->win, pick->y, pick->x - 1, "*");
    }
}

int output_objects(Pick *pick) {
    for (pick->idx = 0; pick->idx < pick->obj_cnt; pick->idx++) {
        if (pick->f_selected[pick->idx])
            fprintf(pick->out_fp, "%s\n", pick->object[pick->idx]);
        free(pick->object[pick->idx]);
    }
    return (0);
}

int exec_objects(Pick *pick, char *title) {
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
        for (pick->idx = 0; pick->idx < pick->obj_cnt; pick->idx++) {
            if (pick->f_selected[pick->idx])
                if (margc < MAXARGS)
                    margv[margc++] = pick->object[pick->idx];
        }
        margv[margc] = NULL;
        mview(init, margc, margv, 10, 40, pick->begy + 1, pick->begx + 4);
        for (pick->idx = 0; pick->idx < pick->obj_cnt; pick->idx++)
            free(pick->object[pick->idx]);
    } else {
        if (pick->f_multiple_cmd_args) {
            for (pick->idx = 0; pick->idx < pick->obj_cnt; pick->idx++) {
                if (pick->f_selected[pick->idx])
                    if (margc < MAXARGS)
                        margv[margc++] = pick->object[pick->idx];
            }
            margv[margc] = NULL;
            rc = fork_exec(margv);
            for (pick->idx = 0; pick->idx < pick->obj_cnt; pick->idx++)
                free(pick->object[pick->idx]);
        } else {
            for (pick->idx = 0; pick->idx < pick->obj_cnt; pick->idx++) {
                if (pick->f_selected[pick->idx]) {
                    if (margc < MAXARGS)
                        margv[margc] = pick->object[pick->idx];
                    margv[margc + 1] = NULL;
                    rc = fork_exec(margv);
                }
                free(pick->object[pick->idx]);
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
