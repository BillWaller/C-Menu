#include <notcurses/notcurses.h>

struct NotcursesPad {
    struct ncplane *view_window; // The small visible viewport on screen
    struct ncplane *data_plane;  // The massive internal plane holding text
    int current_scroll_x;
    int current_scroll_y;
};

// Initialize a scrolling pad structure
struct NotcursesPad *create_pad(struct ncplane *parent, int view_h, int view_w, int view_y, int view_x, int pad_h, int pad_w) {
    struct NotcursesPad *pad = malloc(sizeof(struct NotcursesPad));

    struct ncplane_options view_opts = {.y = view_y, .x = view_x, .rows = view_h, .cols = view_w};
    pad->view_window = ncplane_create(parent, &view_opts);

    // Bind the data plane directly inside the view window plane
    struct ncplane_options data_opts = {.y = 0, .x = 0, .rows = pad_h, .cols = pad_w};
    pad->data_plane = ncplane_create(pad->view_window, &data_opts);

    pad->current_scroll_x = 0;
    pad->current_scroll_y = 0;
    return pad;
}

// Perform horizontal scrolling action by shifting the child data plane position
void scroll_pad_horizontal(struct NotcursesPad *pad, int columns_to_shift) {
    pad->current_scroll_x += columns_to_shift;
    // Moving a child plane relative to its parent viewport masks out-of-bounds text
    ncplane_move_yx(pad->data_plane, pad->current_scroll_y, -pad->current_scroll_x);
}
