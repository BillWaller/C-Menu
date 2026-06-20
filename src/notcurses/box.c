#include "nc.h"
#include <locale.h>
#include <notcurses/notcurses.h>
#include <stdlib.h>

// static inline int ncplane_perimeter(struct ncplane *n, NULL, NULL, NULL,
// NULL, NULL, NULL, 0);

// static inline int ncplane_perimeter(
//     struct ncplane* n, const nccell* ul, const nccell* ur, const nccell* ll,
//     const nccell* lr, const nccell* hline, const nccell* vline, unsigned
//     ctlword
// );

static inline int ncplane_perimeter(struct ncplane *n, const nccell *ul, const nccell *ur, const nccell *ll, const nccell *lr, const nccell *hline, const nccell *vline, unsigned ctlword);

int main() {
    // 1. Initialize locale for Unicode box-drawing characters
    setlocale(LC_ALL, "");

    // 2. Initialize notcurses
    struct notcurses *nc = notcurses_init(NULL, stdout);
    if (!nc) {
        return EXIT_FAILURE;
    }

    // 3. Get standard plane to find terminal dimensions
    struct ncplane *stdn = notcurses_stdplane(nc);
    unsigned dimy, dimx;
    ncplane_dim_yx(stdn, &dimy, &dimx);

    // 4. Define window properties
    int win_height = 10;
    int win_width = 40;
    // Center the window on the screen
    int start_y = (dimy - win_height) / 2;
    int start_x = (dimx - win_width) / 2;

    // 5. Create a new plane and draw a border around it
    struct ncplane_options ncopts = {
        .y = start_y,
        .x = start_x,
        .rows = win_height,
        .cols = win_width,
    };
    struct ncplane *n = ncplane_create(stdn, &ncopts);

    // Draw a box using simple line characters.
    // You can also use NCSTYLE_NONE for borders.
    if (ncplane_perimeter(n, NULL, NULL, 0) == 0) { // Print text inside the box
        ncplane_printf_yx(n, 4, 4, "Hello from Notcurses!");
        ncplane_printf_yx(n, 5, 4, "Window Size: %dx%d", win_width, win_height);
    }

    // 6. Render the screen
    notcurses_render(nc);

    // 7. Wait for user input before exiting
    notcurses_get(nc, NULL, NULL);

    // 8. Clean up
    notcurses_stop(nc);
    return EXIT_SUCCESS;
}
