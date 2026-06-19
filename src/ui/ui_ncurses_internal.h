#ifndef UI_NCURSES_INTERNAL_H
#define UI_NCURSES_INTERNAL_H 1

#include "../include/ui_backend.h"
#include <ncursesw/ncurses.h>
#include <ncursesw/panel.h>
#include <stdbool.h>

struct UiRuntime {
    bool mouse_enabled;
    bool alt_screen;
    bool cursor_visible;
    int rows;
    int cols;
};

struct UiSurface {
    WINDOW *win;
    PANEL *pan;
    struct UiRuntime *runtime;
    struct UiSurface *parent;
    int y;
    int x;
    int rows;
    int cols;
    bool hidden;
};

int ui_ncurses_style_apply(WINDOW *win, const UiStyle *style);
int ui_ncurses_color_pair_from_style(const UiStyle *style);

#endif
