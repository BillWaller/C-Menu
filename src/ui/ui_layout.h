#ifndef UI_LAYOUT_H
#define UI_LAYOUT_H 1

/** @file
   @brief UI layout management.
    @ingroup ui_backend

   This module provides functions for creating and managing framed surfaces, which consist of an outer surface (the frame) and an inner surface (the content area).
 */

#include "ui_backend.h"

/** @struct UiFramedSurface
    @ingroup ui_backend
   Represents a framed surface, consisting of an outer surface (the frame) and
   an inner surface (the content area).
 */
typedef struct {
    UiSurface *outer;
    UiSurface *inner;
} UiFramedSurface;

/**
   @brief Creates a new framed surface with the specified parent and rectangle.
   @ingroup ui_backend
   @param ui The UI runtime context.
   @param parent The parent surface to which the framed surface will be attached.
   @param rect The rectangle defining the position and size of the framed surface.
   @return A new UiFramedSurface instance, or NULL on failure.
 */
UiFramedSurface ui_framed_surface_new(UiRuntime *ui, UiSurface *parent, UiRect rect);
/**
   @brief Destroys a framed surface and releases its resources.
   @ingroup ui_backend
   @param fs The UiFramedSurface to destroy.
 */
void ui_framed_surface_destroy(UiFramedSurface *fs);

#endif
