#ifndef UI_NCURSES_COMPAT_H
#define UI_NCURSES_COMPAT_H 1

/** @file ui_ncurses_compat.h
   @ingroup ui_ncurses
   @brief Non-portable escape hatch for the NCurses backend.

   @warning Including this header couples application code to the NCurses
   backend.  Use only when NCurses-specific functionality is required and
   portability to other backends (e.g., NotCurses) is not needed.  These
   functions are intentionally not declared in ui_backend.h.
*/

#include "../include/ui_backend.h"
#include <ncursesw/ncurses.h>
#include <ncursesw/panel.h>

typedef struct {
    wchar_t ch;       /**< The wide character. */
    attr_t attr;      /**< The attributes (color, bold, etc.). */
    short color_pair; /**< The color pair index. */
} ui_cchar;

typedef struct {
    wchar_t ch; /**< The wide character.  4-bytes */
    short attr; /**< The attributes (color, bold, etc.). */
    int fg;
    int bg;
} ui_xchar;

/** @brief Return the raw @c SCREEN* for the NCurses session.
   @param ui The UiRuntime returned by ui_init().
   @return The NCurses SCREEN pointer, or NULL if @p ui is NULL.
*/
SCREEN *ui_ncurses_get_screen(const UiRuntime *ui);

/** @brief Return the raw NCurses @c WINDOW* underlying a surface.
   @param s The UiSurface to inspect.
   @return The WINDOW pointer, or NULL if @p s is NULL.
*/
WINDOW *ui_ncurses_surface_get_win(const UiSurface *s);

/** @brief Return the raw NCurses @c PANEL* underlying a surface.
   @param s The UiSurface to inspect.
   @return The PANEL pointer, or NULL if @p s is NULL.
*/
PANEL *ui_ncurses_surface_get_panel(const UiSurface *s, int w);

#endif
