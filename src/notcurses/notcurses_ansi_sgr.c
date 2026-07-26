#include <notcurses/notcurses.h>

// Writing raw ANSI terminal strings directly to an isolated plane
void write_ansi_stream_to_plane(struct ncplane *plane, const char *ansi_string) {
    size_t bytes_written = 0;
    // Automatically translates color escapes, bold, italics, and underlines natively
    ncplane_contents(plane, ansi_string, strlen(ansi_string), &bytes_written);
}

// Processing non-blocking input events cleanly for view/forms
void process_input_pipeline(struct notcurses *nc) {
    ncinput ni;
    // Clean, non-blocking single-keypress evaluation matching modern OS events
    uint32_t keypress = notcurses_get_nblock(nc, &ni);

    if (keypress != 0) {
        if (keypress == NCKEY_ENTER) {
            // Handle form field submission
        } else if (keypress == NCKEY_RIGHT) {
            // Trigger horizontal pad scrolling
        }
    }
}
