#include <notcurses/notcurses.h>

// Assuming 'nc' is your struct notcurses* and 'ni' is your ncinput*
if (ni->id == NCKEY_BUTTON1) { // Example: Left Click
    int plane_y, plane_x;

    // Get the absolute position of your target plane on the screen
    ncplane_yx(my_plane, &plane_y, &plane_x);

    // Calculate the coordinates relative to 'my_plane'
    int relative_y = ni->y - plane_y;
    int relative_x = ni->x - plane_x;

    // Check if the click actually falls inside the bounds of the plane
    unsigned dim_y, dim_x;
    ncplane_dim_yx(my_plane, &dim_y, &dim_x);

    if (relative_y >= 0 && relative_y < (int)dim_y &&
        relative_x >= 0 && relative_x < (int)dim_x) {
        // The click is inside 'my_plane'. Use relative_y and relative_x safely.
    }
}
