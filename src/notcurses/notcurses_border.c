#define _XOPEN_SOURCE 700

#include <locale.h>
#include <notcurses/notcurses.h>
#include <string.h>
#include <unistd.h>

int main(void) {
    // 1. Notcurses requires a UTF-8 locale initialized before starting
    setlocale(LC_ALL, "");

    struct notcurses_options nc_opts = {
        .flags = NCOPTION_SUPPRESS_BANNERS,
        // | NCOPTION_NO_ALTERNATE_SCREEN,
    };
    struct notcurses *nc = notcurses_init(&nc_opts, NULL);
    if (!nc) {
        return 1;
    }

    // 2. Fetch the standard backdrop plane
    struct ncplane *std_plane = notcurses_stdplane(nc);

    // 3. Define and create a smaller child plane
    int plane_rows = 12;
    int plane_cols = 40;
    struct ncplane_options plane_opts = {
        .y = 4,  // Row offset from top
        .x = 10, // Column offset from left
        .rows = plane_rows,
        .cols = plane_cols,
        .name = "my_panel" // Used for debugging purposes
    };
    struct ncplane *panel = ncplane_create(std_plane, &plane_opts);
    if (!panel) {
        notcurses_stop(nc);
        return 1;
    }

    // 4. Style the plane colors (Cyan text on Dark Gray background)
    uint64_t channels = 0;
    // channels_set_fg_rgb(&channels, 0x00, 0xD7, 0xFF);
    // channels_set_bg_rgb(&channels, 0x1C, 0x1C, 0x1C);
    ncchannels_set_fg_rgb8(&channels, 0x00, 0xD7, 0xFF);
    ncchannels_set_bg_rgb8(&channels, 0x1C, 0x1C, 0x1C);
    ncplane_set_base(panel, " ", 0, channels);
    ncplane_set_channels(panel, channels);

    // 5. Draw a modern rounded border around the plane perimeter
    // 0 is used for style masks and control words to inherit default styles
    ncplane_perimeter_rounded(panel, 0, channels, 0);

    // 6. Dynamically center and burn a title into the top border row
    const char *title = " [ Server Monitor ] ";
    int title_len = (int)strlen(title);
    int title_x = (plane_cols - title_len) / 2;

    // Print text precisely onto row 0 (the top border)
    ncplane_putstr_yx(panel, 0, title_x, title);

    // 7. Render a message inside the bounded box
    ncplane_putstr_yx(panel, 5, 4, "Notcurses Plane Example!");
    ncplane_putstr_yx(panel, 7, 4, "Press any key to exit...");

    // 8. Refresh the physical display and block for user input
    notcurses_render(nc);

    ncinput ni;
    notcurses_get_blocking(nc, &ni);

    // 9. Clean up resources and restore the terminal
    notcurses_stop(nc);
    return 0;
}
