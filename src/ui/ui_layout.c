/** @file ui_layout.c
   @ingroup ui_backend
   @brief Backend-agnostic layout helpers.

   Implements utility functions that build higher-level layout concepts (framed
   surfaces, etc.) entirely through the portable UALUI API so they work with
   any compiled-in backend.
*/

#include "../ui/ui_layout.h"
#include "../include/ui_backend.h"
#include <string.h>

/** @brief Create a framed surface (outer border + inner content area).

   The returned UiFramedSurface.outer is the frame panel; .inner is one row
   and one column smaller on each side (the drawable content area).  The
   caller is responsible for destroying the framed surface with
   ui_framed_surface_destroy() when it is no longer needed.

   @param ui     The UiRuntime context.
   @param parent Parent surface, or NULL to make the frame a top-level panel.
   @param rect   Position and size of the outer frame (including the border).
   @return       A UiFramedSurface; both fields are NULL on failure.
*/
UiFramedSurface ui_framed_surface_new(UiRuntime *ui, UiSurface *parent,
                                      UiRect rect) {
    int w = 0;
    UiFramedSurface fs = {NULL, NULL};

    if (!ui || rect.lines < 3 || rect.cols < 3)
        return fs;

    // char title[64] = {0};
    fs.outer = ui_surface_new(ui, w, parent, 0, rect.lines, rect.cols, rect.y, rect.x);
    if (!fs.outer)
        return fs;

    rect.y = 1,
    rect.x = 1,
    rect.lines = rect.lines - 2,
    rect.cols = rect.cols - 2,
    fs.inner = ui_surface_new(ui, w, fs.outer, 0, rect.lines, rect.cols, rect.y, rect.x);
    if (!fs.inner) {
        ui_surface_destroy(fs.outer);
        fs.outer = NULL;
    }

    return fs;
}

/** @brief Destroy a framed surface and release its resources.
   @param fs Pointer to the UiFramedSurface to destroy.
*/
void ui_framed_surface_destroy(UiFramedSurface *fs) {
    if (!fs)
        return;
    /* Destroy inner before outer (child before parent). */
    if (fs->inner)
        ui_surface_destroy(fs->inner);
    if (fs->outer)
        ui_surface_destroy(fs->outer);
    fs->inner = NULL;
    fs->outer = NULL;
}
