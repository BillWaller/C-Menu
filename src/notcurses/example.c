#define _XOPEN_SOURCE_EXTENDED 1

#include "nc.h"
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>

int display_grapheme_cluster(struct ncplane *ncp, const char *gc, int y, int x) {
    nccell cell = NCCELL_TRIVIAL_INITIALIZER;
    if (nccell_load(ncp, &cell, gc) < 0) {
        fprintf(stderr, "Error: Failed to load character into nccell.\n");
        return -1;
    }
    nccell_set_fg_rgb(&cell, 0xFF4500); // Orange-Red foreground
    nccell_set_bg_rgb(&cell, 0x1A0D00); // Dark brown background
    nccell_set_styles(&cell, NCSTYLE_BOLD | NCSTYLE_ITALIC);
    ncplane_putc_yx(ncp, y, x, &cell);
    nccell_release(ncp, &cell);
    return 0;
}

int main(void) {
    if (!setlocale(LC_ALL, "")) {
        fprintf(stderr, "Error: Could not set locale to UTF-8.\n");
        return EXIT_FAILURE;
    }

    struct notcurses_options options = {0};
    struct notcurses *nc = notcurses_init(&options, NULL);
    if (nc == NULL)
        return EXIT_FAILURE;

    struct ncplane *stdplane = notcurses_stdplane(nc);
    display_grapheme_cluster(stdplane, "🔥", 5, 1);
    notcurses_render(nc);
    ncinput ni;
    uint32_t key;
    do {
        key = notcurses_get_blocking(nc, &ni);
        if (key == (uint32_t)-1) {
            break;
        }
    } while (key == 0);
    uint32_t key1 = key;

    display_grapheme_cluster(stdplane, "👩‍🚀", 7, 1);
    notcurses_render(nc);

    do {
        key = notcurses_get_blocking(nc, &ni);
        if (key == (uint32_t)-1) {
            break;
        }
    } while (key == 0);
    uint32_t key2 = key;

    notcurses_stop(nc);
    printf("key1=%u, key2=%u\n", key1, key2);
    return EXIT_SUCCESS;
}
