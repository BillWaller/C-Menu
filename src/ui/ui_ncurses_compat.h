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

// Either of the following two structs can be used to represent a character with
// attributes and color information. The ui_cchar struct is compatible with the
// NCurses cchar_t type, while the ui_xchar struct is a custom implementation
// that uses 4-byte integers for attributes and color information.

typedef struct {      //                                8-bytes
    wchar_t ch;       // Wide character                 4-bytes
    short attr;       // attributes (color, bold, etc.) 2-bytes
    short color_pair; // color pair index               2-bytes
} UiCchar;

typedef struct { //                16-bytes
    wchar_t ch;  // wide character  4-bytes
    int attr;    // attributes      4-bytes
    int fg;      //                 4-bytes
    int bg;      //                 4-bytes
} UiXchar;

// UiCchar ui_cchar_str[4096];
// UiXchar ui_xchar_str[4096];

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
   @param w UiSurface window
   @return The PANEL pointer, or NULL if @p s is NULL.
*/
PANEL *ui_ncurses_surface_get_panel(const UiSurface *s, int w);

#endif
