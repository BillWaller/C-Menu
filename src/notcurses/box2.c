#include "nc.h"
#include <locale.h>
#include <notcurses/notcurses.h>
#include <stdlib.h>

int main(void) {
    // Ensure UTF-8 support for box drawing characters
    setlocale(LC_ALL, "");

    // Initialize Notcurses in standard mode
    struct notcurses_options opts = {
        .flags = NCOPTION_NO_ALTERNATE_SCREEN // Keep terminal content after exit
    };
    struct notcurses *nc = notcurses_init(&opts, NULL);
    if (!nc) {
        fprintf(stderr, "Error: Could not initialize Notcurses.\n");
        return EXIT_FAILURE;
    }

    // Get the standard plane (full screen)
    struct ncplane *stdn = notcurses_stddim_yx(nc, NULL, NULL);

    // Create a subplane for our "window"
    int win_height = 10;
    int win_width = 40;
    int start_y = 3;
    int start_x = 5;
    struct ncplane_options nopts = {
        .y = start_y,
        .x = start_x,
        .rows = win_height,
        .cols = win_width,
        .name = "box_window"};
    struct ncplane *win = ncplane_create(stdn, &nopts);
    if (!win) {
        fprintf(stderr, "Error: Could not create subplane.\n");
        notcurses_stop(nc);
        return EXIT_FAILURE;
    }

    // Set background color for the window
    ncplane_set_bg_rgb(win, 0x20, 0x20, 0x40); // dark blue
    ncplane_erase(win);

    // Draw a box around the window
    ncplane_perimeter(win, 0, 0, 0); // default box style

    // Add some centered text inside the box
    const char *msg = "Hello from Notcurses!";
    ncplane_printf_yx(win, win_height / 2, (win_width - strlen(msg)) / 2, "%s", msg);

    // Render to the screen
    notcurses_render(nc);

    // Wait for a key press
    ncinput ni;
    notcurses_get_blocking(nc, &ni);

    // Clean up
    notcurses_stop(nc);
    return EXIT_SUCCESS;
}
