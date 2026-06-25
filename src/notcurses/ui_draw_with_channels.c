#include <locale.h>
#include <notcurses/notcurses.h>

void draw_with_channels(struct ncplane *plane) {
    // 1. Create a combined channels pair (starts completely transparent)
    uint64_t my_palette = 0;

    // 2. Set individual RGB colors for foreground and background
    ncchannels_set_fg_rgb(&my_palette, 0x00FFCC); // Bright Teal
    ncchannels_set_bg_rgb(&my_palette, 0x220033); // Dark Purple

    // 3. Initialize your cell
    nccell cell = NCCELL_TRIVIAL_INITIALIZER;
    nccell_load(plane, &cell, "▒");

    // 4. Apply the entire color pair to the cell in one operation
    nccell_set_channels(&cell, my_palette);

    // 5. Reuse the cell to draw a pattern quickly
    for (int y = 2; y < 7; y++) {
        for (int x = 5; x < 20; x++) {
            ncplane_putc_yx(plane, y, x, &cell);
        }
    }

    // 6. Clean up
    nccell_release(plane, &cell);
}
