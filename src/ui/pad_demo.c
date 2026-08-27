#include <notcurses/notcurses.h>
#include <unistd.h>

int main(void) {
    // 1. Initialize Notcurses
    struct notcurses_options opts = {
        .flags = NCOPTION_SUPPRESS_BANNERS,
    };
    struct notcurses *nc = notcurses_init(&opts, NULL);
    if (!nc)
        return 1;

    // Get the root plane (matches terminal size)
    struct ncplane *stdn = notcurses_stdplane(nc);
    unsigned term_y, term_x;
    ncplane_dim_yx(stdn, &term_y, &term_x);

    // 2. Create an oversized plane ("pad") bound to the standard plane
    // Dimensions: 100 rows by 200 columns (much larger than typical terminal)
    struct ncplane_options plane_opts = {
        .y = 0,
        .x = 0,
        .rows = 100,
        .cols = 200,
    };
    struct ncplane *pad = ncplane_create(stdn, &plane_opts);

    // Set a background color so you can see the pad boundaries
    uint64_t channels = 0;
    channels_set_fchannel(&channels, 0x002244); // Dark blue background
    channels_set_bchannel(&channels, 0x000000);
    ncplane_set_base(pad, " ", 0, channels);

    // 3. Populate the oversized plane with data
    for (int y = 0; y < 100; y += 5) {
        for (int x = 0; x < 200; x += 20) {
            ncplane_printf_yx(pad, y, x, "[Row %d, Col %d]", y, x);
        }
    }

    // 4. Navigate across the oversized plane
    // Moving the plane to negative coordinates shifts the viewport
    for (int i = 0; i <= 40; i++) {
        // Pan diagonally by moving the origin up and left
        ncplane_move_yx(pad, -i, -(i * 2));

        // Render the scene (Notcurses automatically clips boundaries)
        notcurses_render(nc);

        usleep(100000); // 100ms delay
    }

    // 5. Clean up
    notcurses_stop(nc);
    return 0;
}

/* Key Implementation Details
 * Virtual Coordinates: The y and x positions in ncplane_options are relative to
 * the parent plane (stdn).
 * Negative Moving: To view lower or rightward sections of your large canvas,
 * pass negative coordinates to ncplane_move_yx. This slides the canvas up or
 * left behind the terminal viewport.
 * Automatic Clipping: You do not need to calculate intersection windows like
 * ncurses prefresh. Notcurses discards pixels outside the screen space
 * automatically during notcurses_render.
 */
