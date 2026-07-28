#ifndef UI_NCURSES_INTERNAL_H
#define UI_NCURSES_INTERNAL_H 1

/** @file ui_ncurses_internal.h
   @ingroup ui_backend
   @brief Internal header for ncurses UI backend.
   This header defines internal structures and functions used by the ncurses
   implementation of the UI backend. It should not be included directly by
   external code.
 */

#include "../include/ui_backend.h"
#include <cm.h>
#include <ncursesw/ncurses.h>
#include <ncursesw/panel.h>
#include <stdbool.h>

#define XLEN 256

/** @struct UiRuntime
   @ingroup ui_backend
   @brief Runtime state for the ncurses UI backend.
   This structure holds global state related to the ncurses UI, such as
   whether mouse support is enabled, whether the alternate screen is active,
   and the current terminal dimensions.
 */
struct UiRuntime {
    bool mouse_enabled;
    bool alt_screen;
    bool cursor_visible;
    int rows;
    int cols;
};

/** @struct UiSurface
   @ingroup ui_backend
   @brief Represents a surface in the ncurses UI backend.
   This structure represents a drawable surface in the ncurses UI, which may
   correspond to a window or panel. It holds references to the underlying
   ncurses WINDOW and PANEL, as well as its position, size, and visibility
   state.
 */
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
UiStyle *ui_style_new();
void ui_style_destroy(UiStyle *);
UiStyle *ui_style_from_cch(const cchar_t *);
cchar_t ui_style_to_cch(const UiStyle *, const char *);
int ui_bkgrnd(UiSurface *, const UiStyle *, const char *);
int ui_bkgrnd_set(UiSurface *, const UiStyle *, const char *);

#endif
