#include <notcurses/notcurses.h>

// Simulating raising a specific "panel" to the top of the stack
void bring_panel_to_front(struct ncplane *panel_plane) {
    // Moves the plane to the highest Z-order position in its pile
    ncplane_move_top(panel_plane);
}

// Simulating hiding an active dialog box panel
void hide_panel(struct ncplane *panel_plane) {
    // Unmapping it from the active render tree without destroying memory
    ncplane_move_bottom(panel_plane);
    // Or explicitly disable its visibility flag:
    // ncplane_set_scrolling(panel_plane, false);
}

// Create a new modal window panel bound to the root screen
struct ncplane *create_panel(struct notcurses *nc, int rows, int cols, int y, int x) {
    struct ncplane *std = notcurses_stdplane(nc);
    struct ncplane_options opts = {
        .y = y,
        .x = x,
        .rows = rows,
        .cols = cols,
        .name = "dialog_panel",
    };
    // Sub-planes are bound to parents, moving automatically if the parent shifts
    return ncplane_create(std, &opts);
}
