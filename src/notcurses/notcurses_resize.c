#include <notcurses/notcurses.h>
#include <stdio.h>

// Resize callback triggered automatically by the layout cascade engine
int handle_ui_resize(struct ncplane *std_plane) {
    unsigned rows, cols;
    // Safely pull the new terminal canvas limits
    ncplane_dim_yx(std_plane, &rows, &cols);

    // Perform structural UI updates here if you want to reposition
    // static dialog frames or wrap text paragraphs to the new edges
    return 0;
}

void main_event_loop(struct notcurses *nc) {
    struct ncplane *std = notcurses_stdplane(nc);

    // Optional: Register a specific layout resize callback directly on the base plane
    notcurses_resize_cb cb = handle_ui_resize;

    ncinput ni;
    while (1) {
        // Non-blocking or blocking input fetch
        uint32_t keypress = notcurses_get(nc, NULL, &ni);

        if (keypress == 'q') {
            break;
        }

        if (keypress == NCKEY_RESIZE) {
            // 1. The hardware has changed sizes.
            // 2. Notcurses has already adjusted the 'std' plane parameters safely.
            // 3. We call render to perform a flicker-free diff paint onto the screen.
            notcurses_render(nc);
            continue;
        }

        // Handle routine alphanumeric keystrokes for forms/pickers here...
    }
}
