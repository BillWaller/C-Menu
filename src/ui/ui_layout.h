#ifndef UI_LAYOUT_H
#define UI_LAYOUT_H 1

#include "ui_backend.h"

typedef struct {
    UiSurface *outer;
    UiSurface *inner;
} UiFramedSurface;

UiFramedSurface ui_framed_surface_new(UiRuntime *ui, UiSurface *parent, UiRect rect);
void ui_framed_surface_destroy(UiFramedSurface *fs);

#endif
