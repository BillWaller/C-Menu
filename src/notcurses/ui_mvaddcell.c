#include "nc.h"
#include <argp.h>
#include <notcurses/notcurses.h>

// ui_mvaddcell(plane, y, x, "🚀");

void ui_mvaddcell(struct ncplane *plane, int y, int x, gcluster gc) {
    nccell cell = NCCELL_TRIVIAL_INITIALIZER;
    if (nccell_load(plane, &cell, "🚀") < 0)
        return;
    nccell_set_styles(&cell, NCSTYLE_BOLD | NCSTYLE_UNDERLINE);
    nccell_set_fg_rgb(&cell, 0xFF5733); // Vibrant orange foreground
    nccell_set_bg_rgb(&cell, 0x1A1A2E); // Dark blue background
    ncplane_putc_yx(plane, y, x, &cell);
    nccell_release(plane, &cell);
}
nccell source_cell = NCCELL_TRIVIAL_INITIALIZER;
nccell dest_cell = NCCELL_TRIVIAL_INITIALIZER;

nccell_load(plane, &source_cell, "A");

// Correct way to copy a cell
nccell_duplicate(plane, &dest_cell, &source_cell);

// Clean up both when done
nccell_release(plane, &source_cell);
nccell_release(plane, &dest_cell);

nccell read_cell = NCCELL_TRIVIAL_INITIALIZER;
ncplane_at_yx(plane, 2, 5, &read_cell);

// Extract the text string (EGC)
const char *egc = nccell_egc(&read_cell);

// Extract colors and styles
uint32_t fg_rgb = nccell_fg_rgb(&read_cell);
uint16_t styles = nccell_styles(&read_cell);

nccell_release(plane, &read_cell);
