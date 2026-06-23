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

/** @struct UiNcursesBackend
   @ingroup ui_backend
   @param style The UiStyle to apply to the window.
   @brief Represents the ncurses UI backend.
   This structure implements the UiBackend interface for the ncurses UI
   backend. It holds a reference to the runtime state and any necessary
   function pointers for backend operations.
 */
int ui_ncurses_style_apply(WINDOW *win, const UiStyle *style);

/** @brief Converts a UiStyle to an ncurses color pair.
   @ingroup ui_backend
   @param style The UiStyle to convert.
   @return The ncurses color pair corresponding to the given style.
 */
int ui_ncurses_color_pair_from_style(const UiStyle *style);

#endif
