#include <notcurses/notcurses.h>

void ual_destroy_surface(struct ncplane *plane) {
    if (!plane)
        return;
    ncplane_destroy(plane);
}
