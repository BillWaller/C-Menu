#include <locale.h>
#include <notcurses/notcurses.h>

// Example: Writing an array of custom nccells
int write_cell_array(struct ncplane *n, int starty, int startx, const nccell *cells, int count) {
    for (int i = 0; i < count; i++) {
        if (ncplane_putc_yx(n, starty, startx + i, &cells[i]) < 0) {
            return -1;
        }
    }
    return 0;
}
