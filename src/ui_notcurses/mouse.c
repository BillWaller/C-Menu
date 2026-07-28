#include <locale.h>
#include <notcurses/notcurses.h>
#include <unistd.h>

struct ncplane *find_clicked_plane(struct ncplane *pile_member, ncinput *ni);

int main(void) {
    setlocale(LC_ALL, "");

    // 1. Initialize Notcurses with mouse tracking enabled
    struct notcurses_options options = {
        .flags = NCOPTION_NO_ALTERNATE_SCREEN, // Keep terminal logs visible if desired
    };
    struct notcurses *nc = notcurses_init(&options, NULL);
    if (!nc)
        return 1;

    // Enable mouse events
    notcurses_mice_enable(nc, NCMICE_ALL_EVENTS);

    // 2. Create a test plane in the middle of the screen
    struct ncplane *stdn = notcurses_stdplane(nc);
    struct ncplane_options p_opts = {
        .y = 5,
        .x = 10,
        .rows = 10,
        .cols = 30,
        .name = "TestPlane"};
    struct ncplane *my_plane = ncplane_create(stdn, &p_opts);

    // Style the test plane so it is visibly distinct
    uint64_t channels = 0;
    ncchannels_set_fg_rgb(&channels, 0x00FF00);
    ncchannels_set_bg_rgb(&channels, 0x222222);
    ncplane_set_base(my_plane, " ", 0, channels);
    ncplane_putstr_yx(my_plane, 1, 1, "Click inside this box!");

    notcurses_render(nc);

    // 3. Main event loop
    ncinput ni;
    uint32_t id;
    while ((id = notcurses_get_blocking(nc, &ni)) != 'q') {
        // Check if the event is a mouse press
        if (nckey_mouse_p(id)) {
            struct ncplane *clicked_plane = NULL;

            // This function modifies ni.x and ni.y to be relative to 'clicked_plane'
            // clicked_plane = ncinput_drop_plane_relative(stdn, &ni);
            clicked_plane = find_clicked_plane(my_plane, &ni);

            // Clear the standard plane background for status text
            ncplane_erase(stdn);

            if (clicked_plane == my_plane) {
                ncplane_printf_yx(stdn, 0, 0,
                                  "Clicked: TestPlane | Relative Coordinates: Y=%d, X=%d",
                                  ni.y, ni.x);
            } else if (clicked_plane == stdn) {
                ncplane_printf_yx(stdn, 0, 0,
                                  "Clicked: Standard Plane | Absolute Coordinates: Y=%d, X=%d",
                                  ni.y, ni.x);
            } else {
                ncplane_printf_yx(stdn, 0, 0, "Clicked an unknown plane.");
            }

            notcurses_render(nc);
        }
    }

    // 4. Cleanup
    notcurses_mice_disable(nc);
    notcurses_stop(nc);
    return 0;
}
/* Walk the pile from topmost plane downward, find the first (highest
z-order)
plane that encloses the click, transform coordinates to plane-relative.
Returns the hit plane, or NULL if no plane encloses the click.
On return, ni->y and ni->x are relative to the returned plane. */
struct ncplane *find_clicked_plane(struct ncplane *pile_member, ncinput *ni) {
    // ncpile_top() gives the topmost (highest z) plane in the pile
    struct ncplane *cur = ncpile_top(pile_member);
    while (cur != NULL) {
        int y = ni->y, x = ni->x;
        if (ncplane_translate_abs(cur, &y, &x)) {
            // Point is inside cur; coordinates are now relative to cur
            ni->y = y;
            ni->x = x;
            return cur;
        }
        cur = ncplane_below(cur); // move down the z-stack
    }
    return NULL;
}
