#ifndef UI_NCURSES_INTERNAL_H
#define UI_NCURSES_INTERNAL_H 1

/** @file ui_ncurses_internal.h
   @ingroup ui_ncurses
   @brief Internal header for the NCurses UI backend.

   This header defines the concrete (non-opaque) layouts of UiRuntime and
   UiSurface for the NCurses backend.  It must only be included by NCurses
   backend source files — never by application code.
*/

#define _XOPEN_SOURCE_EXTENDED 1
#define _GNU_SOURCE
#define NCURSES_WIDECHAR 1

#include "ui_backend.h"
#include <ncursesw/ncurses.h>
#include <ncursesw/panel.h>
#include <stdbool.h>

#define XLEN 256

/** @struct UiRuntime
   @ingroup ui_ncurses
   @brief Runtime state for the NCurses UI backend.

   Holds all NCurses session resources so that multiple independent sessions
   can coexist and the codebase is not dependent on global variables.
   The legacy globals @c screen and @c tty_fp in @c dwin.c are updated by
   ui_init() / ui_shutdown() for backward compatibility with code that has
   not yet been migrated to the UAL API.
*/
struct UiRuntime {
    SCREEN *screen; /**< NCurses SCREEN created by newterm() */
    FILE *tty_fp;   /**< TTY file handle opened by ui_init() */
    bool mouse_enabled;
    bool alt_screen;
    bool cursor_visible;
    int rows;
    int cols;
    PANEL *panel_main;
};

/** @struct UiSplitSurface
    @brief Split surface containing multiple child surfaces.
    @verbatim

    split_y > 0 && split_x > 0: 4 quadrants
    0: top-left, 1: top-right, 2: bottom-left, 3: bottom-right

    split_y > 0 && split_x == 0: 2 rows
    0: top, 1: bottom

    split_y == 0 && split_x > 0: 2 cols
    0: left, 1: right

    @endverbatim
 */
struct UiSplitSurface {
    WINDOW *box;
    WINDOW *win[4];
    PANEL *pan;
    int win_cnt;
    struct UiRuntime *runtime;
    struct UiSurface *parent;
    int y;
    int x;
    int rows;
    int cols;
    int split_y;
    int split_x;
    bool hidden;
    char name[XLEN];
    char title[XLEN];
};

/** @struct UiSurface
   @ingroup ui_ncurses
   @brief A drawable surface in the NCurses backend.

   Wraps an NCurses WINDOW and its associated PANEL.
*/
struct UiSurface {
    WINDOW *box;
    WINDOW *win;
    WINDOW *mwin[4];
    WINDOW *win2; // LEGACY - to be removed in future versions
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

/* Internal style helpers */
int ui_ncurses_style_apply(WINDOW *win, const UiStyle *style);
int ui_ncurses_color_pair_from_style(const UiStyle *style);
UiStyle *ui_style_new(void);
void ui_style_destroy(UiStyle *);
UiStyle *ui_style_from_cch(const cchar_t *);
cchar_t ui_style_to_cch(const UiStyle *, const char *);
int ui_mvwadd_mbnstr(UiSurface *s, int y, int x, const char *text, int n);
int ui_bkgrndset_cch(UiSurface *, cchar_t *cch);

#endif
