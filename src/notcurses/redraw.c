#include "../include/cm.h"
#include "../include/common.h"
#include "../include/nc.h"
#include "../include/view.h"
#include <notcurses/notcurses.h>

void recalculate_and_draw_layout(struct notcurses *nc, View *view, int h_scroll) {
    unsigned screen_y, screen_x;
    notcurses_term_dim_yx(nc, &screen_y, &screen_x);

    // Split screen: Picker takes 30% width, Viewer takes remaining 70%
    int picker_width = (int)(screen_x * 0.30);
    int viewer_width = screen_x - picker_width;

    // Resize the planes dynamically on-the-fly
    ncplane_resize(view->picker_plane, 0, 0, screen_y, picker_width);
    ncplane_resize(view->viewer_plane, 0, 0, screen_y, viewer_width);

    // Reposition viewer plane next to the picker
    ncplane_move_yx(view->viewer_plane, 0, picker_width);

    // Redraw text and assets safely inside the fresh geometry bounds
    redraw_picker_elements(view);
    redraw_viewer_text(view, h_scroll);
}
