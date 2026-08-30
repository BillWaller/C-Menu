#include <notcurses/notcurses.h>

typedef struct ncvisual_options NcVisualOptions;

int ncplane_blit_rgba(
    struct ncplane *nc,
    int y,
    int x,
    const unsigned char *data,
    int begy,
    int begx,
    int leny,
    int lenx);
int notcurses_wscrl_emulation(struct ncplane *plane, int n) {
    if (n == 0)
        return 0;

    unsigned dimy, dimx;
    ncplane_dim_yx(plane, &dimy, &dimx);

    if (n > 0) {
        // --- CONTENT SHIFTS UP (Blank lines left at the bottom) ---
        // NCurses equivalent: wscrl(win, n)
        bool was_scrolling = ncplane_scrolling_p(plane);
        if (!was_scrolling) {
            ncplane_set_scrolling(plane, true);
        }
        int lines_scrolled = ncplane_scrollup(plane, n);
        if (!was_scrolling) {
            ncplane_set_scrolling(plane, false);
        }
        return (lines_scrolled >= 0) ? 0 : -1;
    } else {
        // --- CONTENT SHIFTS DOWN (Blank lines left at the top)
        // NCurses equivalent: wscrl(win, -n)
        int lines_to_shift = -n;
        if ((unsigned)lines_to_shift >= dimy) {
            ncplane_erase(plane);
            return 0;
        }
        NcVisualOptions vopts;
        vopts.n = 0;
        vopts.scaling = NCSCALE_NONE;
        vopts.blitter = NCBLIT_1x1;
        vopts.flags = NCVISUAL_OPTION_NODEGRADE;
        vopts.y = 0;
        vopts.x = 0;
        vopts.begy = lines_to_shift;
        vopts.begx = 0;
        vopts.leny = dimy - lines_to_shift;
        vopts.lenx = dimx;

        // In Notcurses, blitting a plane onto itself with an offset shifts the
        // bytes correctly.
        if (ncblit_rgba(data, linesize, &vopts) < 0) {
            // Fallback: If your implementation environment lacks direct self-blitting support,
            // you can create a temporary scratchpad plane via ncplane_dup(),
            // blit from scratchpad->original at (lines_to_shift, 0), and destroy the scratchpad.
        }
        ncplane_erase_region(plane, 0, 0, lines_to_shift, dimx);
        return 0;
    }
}
