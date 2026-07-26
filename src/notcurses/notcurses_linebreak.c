#include <notcurses/notcurses.h>

void render_wrapped_unicode_text(struct ncplane *plane, int start_y, int start_x, const char *utf8_text) {
    size_t bytes_validated = 0;
    // Moves cursor to origin within target plane area
    ncplane_cursor_move_yx(plane, start_y, start_x);

    // Automatically wraps text gracefully using structural breaks at plane boundary edges
    ncplane_puttext(plane, start_y, NCALIGN_LEFT, utf8_text, &bytes_validated);
}
