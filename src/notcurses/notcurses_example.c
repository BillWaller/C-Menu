#define _XOPEN_SOURCE 700

#include <locale.h>
#include <notcurses/notcurses.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    // Required: set locale for proper character rendering
    if (!setlocale(LC_ALL, "")) {
        fprintf(stderr, "A valid locale is required.\n");
        return EXIT_FAILURE;
    }

    // Configure options, specifying a custom termtype override if desired
    struct notcurses_options opts = {
        .termtype = NULL, // Set to NULL to default to $TERM, or pass e.g. "xterm-256color"
        .loglevel = NCLOGLEVEL_INFO,
        .flags = NCOPTION_SUPPRESS_BANNERS};

    struct notcurses *nc = notcurses_init(&opts, NULL);
    if (!nc) {
        fprintf(stderr, "Failed to initialize notcurses\n");
        return EXIT_FAILURE;
    }

    // Get the standard drawing plane
    struct ncplane *std = notcurses_stdplane(nc);
    ncplane_putstr(std, "Hello from Notcurses with custom init!");
    ncplane_putstr(std, "Goodbye from Notcurses with custom init!");

    // Force write output to screen and wait briefly
    notcurses_render(nc);
    notcurses_refresh(nc, 0, 0);

    // Clean up terminal state
    if (notcurses_stop(nc)) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
