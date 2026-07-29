#ifndef UI_NCURSES_INTERNAL_H
#define UI_NCURSES_INTERNAL_H 1

#include "../include/ui_backend.h"
#include <ncursesw/ncurses.h>
#include <ncursesw/panel.h>
#include <stdbool.h>

#define XLEN 256

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
    char name[XLEN];
    char title[XLEN];
};

int ui_style_apply(WINDOW *win, const UiStyle *style);
int ui_color_pair_from_style(const UiStyle *style);
UiStyle *ui_style_new();
void ui_style_destroy(UiStyle *);
UiStyle *ui_style_from_cch(const cchar_t *);
cchar_t ui_style_to_cch(const UiStyle *, const char *);
int ui_bkgrnd(UiSurface *, const UiStyle *, const char *);
int ui_bkgrnd_set(UiSurface *, const UiStyle *, const char *);

#endif
