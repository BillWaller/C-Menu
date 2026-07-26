// 3. Creating a dynamically pinned relative child panel
struct ncplane *create_pinned_panel(struct notcurses *nc, struct ncplane *parent,
                                    int rows, int cols, UAL_PinMode mode) {

    // Allocate our layout tracker configuration memory
    PlaneLayoutConfig *config = malloc(sizeof(PlaneLayoutConfig));
    config->parent_plane = parent;
    config->pin_mode = mode;
    config->fixed_rows = rows;
    config->fixed_cols = cols;

    struct ncplane_options opts = {
        .y = 0, // Initial placement (will be overridden instantly by the callback)
        .x = 0,
        .rows = rows,
        .cols = cols,
        .userptr = config, // Pass tracking tracking metrics straight to the handle
        .name = "relative_pinned_panel"};

    // Bind this child plane directly onto the parent structure
    struct ncplane *child_plane = ncplane_create(parent, &opts);

    if (child_plane) {
        // Register the callback function pointer to the sub-plane.
        // It triggers automatically whenever notcurses_render discovers a terminal size shift.
        ncplane_set_resizecb(child_plane, ual_plane_resize_cb);

        // Execute once manually to lock initial alignment geometry layout parameters
        ual_plane_resize_cb(child_plane);
    }

    return child_plane;
}
