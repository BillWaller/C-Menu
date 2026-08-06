#ifndef UI_NOTCURSES_COMPAT_H
#define UI_NOTCURSES_COMPAT_H 1

/** @file ui_notcurses_compat.h
   @ingroup ui_notcurses
   @brief Non-portable escape hatch for the NotCurses backend.

   @warning Including this header couples application code to the NotCurses
   backend.  Use only when NotCurses-specific functionality is required and
   portability to other backends (e.g., NCurses) is not needed.  These
   functions are intentionally not declared in ui_backend.h.
*/

#include "../include/ui_backend.h"
#include <notcurses/notcurses.h>

/** @brief Return the raw @c struct notcurses* for the NotCurses session.
   @param ui The UiRuntime returned by ui_init().
   @return The NotCurses context pointer, or NULL if @p ui is NULL.
*/
struct notcurses *ui_notcurses_get_nc(const UiRuntime *ui);

/** @brief Return the raw @c struct ncplane* underlying a surface.
   @param s The UiSurface to inspect.
   @return The ncplane pointer, or NULL if @p s is NULL.
*/
struct ncplane *ui_notcurses_surface_get_plane(const UiSurface *s);

#endif
