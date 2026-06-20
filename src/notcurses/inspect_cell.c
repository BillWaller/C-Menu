#include <notcurses/notcurses.h>

void inspect_cell_at_coordinate(struct ncplane *cp, int y, int x) {
    uint32_t stylemask;
    uint64_t channels;

    // ncplane_at_yx returns a heap-allocated UTF-8 string (EGC)
    // It also populates your style and color variables by reference
    char *egc = ncplane_at_yx(cp, y, x, &stylemask, &channels);

    if (egc) {
        printf("Character: %s\n", egc);
        printf("Is Bold? %s\n", (stylemask & NCSTYLE_BOLD) ? "Yes" : "No");
        printf("Foreground RGB: 0x%06x\n", ncchannels_fg_rgb(channels));

        // Always free the returned EGC string string when done!
        free(egc);
    }
}
