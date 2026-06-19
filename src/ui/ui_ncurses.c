// src/ui/ui_ncurses.c
#include "ui_ncurses_internal.h"
#include <stdlib.h>
#include <string.h>

UiSurface *ui_surface_new(UiRuntime *ui, UiSurface *parent, UiRect rect) {
    UiSurface *s = calloc(1, sizeof(*s));
    if (!s)
        return NULL;

    s->runtime = ui;
    s->parent = parent;
    s->y = rect.y;
    s->x = rect.x;
    s->rows = rect.rows;
    s->cols = rect.cols;

    if (parent && parent->win) {
        s->win = derwin(parent->win, rect.rows, rect.cols, rect.y, rect.x);
    } else {
        s->win = newwin(rect.rows, rect.cols, rect.y, rect.x);
    }

    if (!s->win) {
        free(s);
        return NULL;
    }

    s->pan = new_panel(s->win);
    if (!s->pan) {
        delwin(s->win);
        free(s);
        return NULL;
    }

    return s;
}

void ui_surface_destroy(UiSurface *s) {
    if (!s)
        return;
    if (s->pan)
        del_panel(s->pan);
    if (s->win)
        delwin(s->win);
    free(s);
}

int ui_surface_move(UiSurface *s, int y, int x) {
    if (!s)
        return -1;
    s->y = y;
    s->x = x;
    return move_panel(s->pan, y, x);
}

int ui_surface_resize(UiSurface *s, int rows, int cols) {
    if (!s)
        return -1;
    s->rows = rows;
    s->cols = cols;
    return wresize(s->win, rows, cols);
}

int ui_surface_clear(UiSurface *s) {
    if (!s)
        return -1;
    wclear(s->win);
    return 0;
}

int ui_surface_erase(UiSurface *s) {
    if (!s)
        return -1;
    werase(s->win);
    return 0;
}

int ui_surface_show(UiSurface *s) {
    if (!s)
        return -1;
    show_panel(s->pan);
    s->hidden = false;
    return 0;
}

int ui_surface_hide(UiSurface *s) {
    if (!s)
        return -1;
    hide_panel(s->pan);
    s->hidden = true;
    return 0;
}

int ui_cursor_move(UiSurface *s, int y, int x) {
    if (!s)
        return -1;
    return wmove(s->win, y, x);
}

UiRuntime *ui_init(const UiConfig *cfg) {
    UiRuntime *ui = calloc(1, sizeof(*ui));
    if (!ui)
        return NULL;

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    if (has_colors()) {
        start_color();
        use_default_colors();
    }

    if (cfg) {
        ui->mouse_enabled = cfg->enable_mouse;
        ui->alt_screen = cfg->enable_alt_screen;
        ui->cursor_visible = cfg->cursor_visible;
    } else {
        ui->cursor_visible = true;
    }

    if (ui->mouse_enabled) {
        mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
    }

    curs_set(ui->cursor_visible ? 1 : 0);
    getmaxyx(stdscr, ui->rows, ui->cols);

    return ui;
}

void ui_shutdown(UiRuntime *ui) {
    if (!ui)
        return;
    endwin();
    free(ui);
}

void ui_get_screen_size(UiRuntime *ui, int *rows, int *cols) {
    if (!ui)
        return;
    getmaxyx(stdscr, ui->rows, ui->cols);
    if (rows)
        *rows = ui->rows;
    if (cols)
        *cols = ui->cols;
}

int ui_render(UiRuntime *ui) {
    (void)ui;
    update_panels();
    doupdate();
    return 0;
}

int ui_clear_screen(UiRuntime *ui) {
    (void)ui;
    erase();
    return 0;
}

int ui_suspend(UiRuntime *ui) {
    (void)ui;
    def_prog_mode();
    endwin();
    return 0;
}

int ui_resume(UiRuntime *ui) {
    (void)ui;
    reset_prog_mode();
    refresh();
    update_panels();
    doupdate();
    return 0;
}

int ui_cursor_enable(UiRuntime *ui, bool visible) {
    if (!ui)
        return -1;
    ui->cursor_visible = visible;
    curs_set(visible ? 1 : 0);
    return 0;
}
