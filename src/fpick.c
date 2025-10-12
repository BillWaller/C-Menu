/* fpick.c
 * pick from a list of choices for MENU
 * Bill Waller
 * billxwaller@gmail.com
 */

#include "menu.h"
#include <stdlib.h>
#include <string.h>

#define M_JUMP 601

int pick_obj();
void save_object(char *);
int pick_object();
void display_page();
void reverse_object();
void toggle_object();
int output_objects();
int exec_objects(char *);
int open_pick_win();

int pick_obj() {
    int i, rc;

    pick->select_idx = 0;
    pick->obj_cnt = pick->page_cnt = pick->line_cnt = pick->row_cnt =
        pick->col_cnt = 0;
    pick->idx = pick->page = pick->line = pick->col = pick->x = pick->width = 0;
    if (pick->in_fp != NULL) {
        while (fgets(pick->in_buf, sizeof(pick->in_buf), pick->in_fp) != NULL)
            save_object(pick->in_buf);
    } else
        for (i = 1; i < pick->argc; i++)
            save_object(pick->argv[i]);
    if (!pick->idx)
        return (-1);
    pick->obj_cnt = pick->idx;
    if (pick->width > pick->cols - 1)
        pick->width = pick->cols - 1;
    pick->col_cnt = pick->cols / (pick->width + 1);
    if (pick->cols > pick->width)
        pick->cols = pick->col_cnt * (pick->width + 1);
    if (!pick->col_cnt)
        pick->col_cnt = 1;
    if (pick->col_cnt > pick->obj_cnt)
        pick->col_cnt = pick->obj_cnt;
    pick->row_cnt = (pick->obj_cnt - 1) / pick->col_cnt + 1;
    if (pick->row_cnt > pick->lines - 1)
        pick->line_cnt = pick->lines - 1;
    else
        pick->line_cnt = pick->row_cnt;
    pick->page_cnt = pick->obj_cnt / (pick->col_cnt * pick->line_cnt) + 1;
    pick->page = 0;
    rc = open_pick_win();
    if (rc)
        return (rc);
    display_page();
    reverse_object();
    while (!(rc = pick_object()))
        ;
    if (rc == 1) {
        if (!pick->cmd_str[0])
            rc = output_objects();
        else
            rc = exec_objects(pick->title);
    } else
        for (pick->idx = 0; pick->idx < pick->obj_cnt; pick->idx++)
            free(pick->object[pick->idx]);
    return (rc);
}

void save_object(char *s) {
    int l = 0;
    char *p;

    if (pick->idx < OBJ_MAXCNT) {
        p = s;
        while (*p != '\0' && *p != '\n' && *p != '\r' && l < OBJ_MAXLEN) {
            p++;
            l++;
        }
        *p = '\0';
        if (l > pick->width)
            pick->width = l;
        pick->object[pick->idx] = (char *)malloc(l + 1);
        p = pick->object[pick->idx];
        if (p == NULL) {
            sprintf(errmsg,
                    "fpick: pick->object[pick->idx]=(char *)malloc(%d)\n",
                    l + 1);
            abend(-1, errmsg);
        } else {
            while (*s != '\0') {
                *p++ = *s++;
            }
            *p = '\0';
        }
        pick->f_selected[pick->idx] = FALSE;
        pick->idx++;
    }
}

int pick_object() {
    int tline, tcol, tidx;

    cmd_key = wgetch(pick->win);
    switch (cmd_key) {
    case 'q':
    case 'Q':
    case '\033':
    case KEY_F(9):
        return (-1);
    case 'y':
    case 'Y':
    case '\040': // SPACE
        toggle_object();
        if (pick->select_cnt > 0 && pick->select_idx >= pick->select_cnt)
            return (1);
        break;
    case '\15': // CR
    case KEY_F(10):
    case KEY_ENTER:
        if (pick->select_idx > 0)
            return (1);
        return (-1);
    case M_JUMP:
        mvwaddstr_fill(pick->win, pick->line, pick->x, pick->object[pick->idx],
                       pick->width);
        pick->idx = pick->obj_cnt;
        tidx = pick->page * pick->line_cnt * pick->col_cnt +
               pick->col * pick->line_cnt + pick->line;
        if (tidx >= 0 && tidx < pick->obj_cnt) {
            pick->idx = tidx;
        }
        reverse_object();
        toggle_object();
        if (pick->select_cnt > 0 && pick->select_idx >= pick->select_cnt)
            return (1);
        break;
    case KEY_RIGHT:
    case key_right:
    case key_ctlf:
        mvwaddstr_fill(pick->win, pick->line, pick->x, pick->object[pick->idx],
                       pick->width);
        tcol = pick->col;
        pick->idx = pick->obj_cnt;
        while (pick->idx >= pick->obj_cnt) {
            tcol++;
            if (tcol >= pick->col_cnt)
                tcol = 0;
            tidx = pick->page * pick->line_cnt * pick->col_cnt +
                   tcol * pick->line_cnt + pick->line;
            if (tidx >= 0 && tidx < pick->obj_cnt) {
                pick->idx = tidx;
                pick->col = tcol;
            }
        }
        reverse_object();
        break;
    case KEY_LEFT:
    case key_left:
    case KEY_BACKSPACE:
    case key_ctlb:
        mvwaddstr_fill(pick->win, pick->line, pick->x, pick->object[pick->idx],
                       pick->width);
        tcol = pick->col;
        pick->idx = pick->obj_cnt;
        while (pick->idx >= pick->obj_cnt) {
            if (tcol <= 0)
                tcol = pick->col_cnt;
            tcol--;
            tidx = pick->page * pick->line_cnt * pick->col_cnt +
                   tcol * pick->line_cnt + pick->line;
            if (tidx >= 0 && tidx < pick->obj_cnt) {
                pick->idx = tidx;
                pick->col = tcol;
            }
        }
        reverse_object();
        break;
    case KEY_DOWN:
    case key_down:
    case key_ctln:
        mvwaddstr_fill(pick->win, pick->line, pick->x, pick->object[pick->idx],
                       pick->width);
        tline = pick->line;
        pick->idx = pick->obj_cnt;
        while (pick->idx >= pick->obj_cnt) {
            tline++;
            if (tline >= pick->line_cnt) {
                tline = 0;
                pick->page++;
                if (pick->page >= pick->page_cnt)
                    pick->page = 0;
                tidx = pick->page * pick->line_cnt * pick->col_cnt +
                       pick->col * pick->line_cnt + tline;
                if (tidx >= 0 && tidx < pick->obj_cnt) {
                    pick->idx = tidx;
                    pick->line = tline;
                }
                display_page();
            } else {
                tidx = pick->page * pick->line_cnt * pick->col_cnt +
                       pick->col * pick->line_cnt + tline;
                if (tidx >= 0 && tidx < pick->obj_cnt) {
                    pick->idx = tidx;
                    pick->line = tline;
                }
            }
        }
        reverse_object();
        break;
    case KEY_UP:
    case key_up:
    case key_ctlp:
        mvwaddstr_fill(pick->win, pick->line, pick->x, pick->object[pick->idx],
                       pick->width);
        tline = pick->line;
        pick->idx = pick->obj_cnt;
        while (pick->idx >= pick->obj_cnt) {
            if (tline <= 0) {
                tline = pick->line_cnt;
                if (pick->page <= 0)
                    pick->page = pick->page_cnt;
                pick->page--;
                display_page();
            } else {
                tline--;
                tidx = pick->page * pick->line_cnt * pick->col_cnt +
                       pick->col * pick->line_cnt + tline;
                if (tidx >= 0 && tidx < pick->obj_cnt) {
                    pick->idx = tidx;
                    pick->line = tline;
                }
            }
        }
        reverse_object();
        break;
    case KEY_NPAGE:
        pick->page++;
        if (pick->page >= pick->page_cnt)
            pick->page = 0;
        display_page();
        reverse_object();
        break;
    case KEY_PPAGE:
        if (pick->page <= 0)
            pick->page = pick->page_cnt;
        pick->page--;
        display_page();
        reverse_object();
        break;
    case KEY_HOME:
        pick->page = 0;
        display_page();
        reverse_object();
        break;
    case KEY_LL:
        pick->page = pick->page_cnt - 1;
        display_page();
        reverse_object();
        break;
    case key_ctlr:
        restore_wins();
    default:
        break;
    }
    return (0);
}

void display_page() {
    int line, col, pidx;

    for (line = 0; line < pick->line_cnt; line++) {
        wmove(pick->win, line, 0);
        wclrtoeol(pick->win);
    }
    pidx = pick->page * pick->line_cnt * pick->col_cnt;
    for (col = 0; col < pick->col_cnt; col++) {
        pick->x = col * (pick->width + 1) + 1;
        for (line = 0; line < pick->line_cnt; line++, pidx++) {
            if (pidx < pick->obj_cnt) {
                if (pick->f_selected[pidx])
                    mvwaddstr(pick->win, line, pick->x - 1, "*");
                mvwaddstr_fill(pick->win, line, pick->x, pick->object[pidx],
                               pick->width);
            }
        }
    }
    sprintf(tmp_str, " PgUp PgDn Space Enter Escape %d of %d ", pick->page + 1,
            pick->page_cnt);
    wattron(pick->win, A_REVERSE);
    mvwaddstr_fill(pick->win, pick->lines - 1, 0, tmp_str, pick->cols);
    wattroff(pick->win, A_REVERSE);
    pick->col = 0;
    pick->line = 0;
    pick->idx = pick->page * pick->line_cnt * pick->col_cnt;
}

void reverse_object() {
    pick->x = pick->col * (pick->width + 1) + 1;
    wattron(pick->win, A_REVERSE);
    wmove(pick->win, pick->line, pick->x);
    waddch(pick->win, pick->object[pick->idx][0]);
    wrefresh(pick->win);
    mvwaddstr_fill(pick->win, pick->line, pick->x, pick->object[pick->idx],
                   pick->width);
    wmove(pick->win, pick->line, pick->x - 1);
    wattroff(pick->win, A_REVERSE);
}

void toggle_object() {
    pick->x = pick->col * (pick->width + 1) + 1;
    if (pick->f_selected[pick->idx]) {
        if (pick->select_idx)
            pick->select_idx--;
        pick->f_selected[pick->idx] = FALSE;
        mvwaddstr(pick->win, pick->line, pick->x - 1, " ");
    } else {
        if (pick->select_idx < OBJ_MAXCNT)
            pick->select_idx++;
        pick->f_selected[pick->idx] = TRUE;
        mvwaddstr(pick->win, pick->line, pick->x - 1, "*");
    }
}

int output_objects() {
    for (pick->idx = 0; pick->idx < pick->obj_cnt; pick->idx++) {
        if (pick->f_selected[pick->idx])
            fprintf(pick->out_fp, "%s\n", pick->object[pick->idx]);
        free(pick->object[pick->idx]);
    }
    return (0);
}

int exec_objects(char *title) {
    int rc = -1;
    int margc;
    char *margv[MAXARGS];
    char *s;

    fprintf(stderr, "\n");
    fflush(stderr);
    wclear(stdscr);
    wmove(stdscr, 0, 0);
    wrefresh(stdscr);
    s = pick->cmd_str;
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
        mview(margc, margv, 10, 40, pick->begy + 1, pick->begx + 4, "pick",
              win_attr);
        for (pick->idx = 0; pick->idx < pick->obj_cnt; pick->idx++)
            free(pick->object[pick->idx]);
    } else {
        if (pick->f_append_cmd_args) {
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

int open_pick_win() {
    if (win_new(pick->lines, pick->cols, pick->begy, pick->begx, "pick")) {
        sprintf(tmp_str, "win_new(%d, %d, %d, %d, %s) failed", pick->lines,
                pick->cols, pick->begy, pick->begx, "pick");
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
