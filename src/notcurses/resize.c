#include <notcurses/notcurses.h>

void run_file_manager_loop(struct notcurses *nc, View *view) {
    ncinput ni;
    uint32_t id;
    int horizontal_scroll_offset = 0;

    // Initial composition layout
    recalculate_and_draw_layout(nc, view, horizontal_scroll_offset);

    // The unified loop catches keyboard, mouse, and screen resizes
    while ((id = notcurses_get(nc, NULL, &ni)) != 'q') {

        switch (id) {
        // 1. Native Terminal Resizing Handler (Replaces manual SIGWINCH)
        case NCKEY_RESIZE:
            // Notcurses has already resized the screen background natively!
            // You just need to update your planes' structural widths and heights.
            recalculate_and_draw_layout(nc, view, horizontal_scroll_offset);
            break;

        // 2. Picker Selector Bar Up/Down Movement
        case NCKEY_UP:
            move_picker_selector_up(view);
            update_viewer_content(view); // Triggers re-read of active file
            break;

        case NCKEY_DOWN:
            move_picker_selector_down(view);
            update_viewer_content(view);
            break;

        // 3. Horizontal Scrolling for the Viewer Plane
        case NCKEY_RIGHT:
            horizontal_scroll_offset++;
            // Shift the visual plane window rightward
            ncplane_scroll_yx(view->viewer_plane, 0, 1);
            break;

        case NCKEY_LEFT:
            if (horizontal_scroll_offset > 0) {
                horizontal_scroll_offset--;
                // Shift the visual plane window leftward
                ncplane_scroll_yx(view->viewer_plane, 0, -1);
            }
            break;
        }

        // Single optimized pass renders picker, viewer, or video frames
        notcurses_render(nc);
    }
}
