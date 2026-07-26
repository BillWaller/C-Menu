#include <notcurses/notcurses.h>
#include <stdlib.h>

// 1. Defining structural positions for our layout elements
typedef enum {
    PIN_CENTER,
    PIN_BOTTOM_RIGHT,
    PIN_TOP_LEFT
} UAL_PinMode;

// Custom user context payload attached to the plane
typedef struct {
    struct ncplane *parent_plane;
    UAL_PinMode pin_mode;
    int fixed_rows;
    int fixed_cols;
} PlaneLayoutConfig;

// 2. The core resize callback matching the Notcurses engine requirements
int ual_plane_resize_cb(struct ncplane *plane) {
    // Extract our configuration geometry attached via the user pointer
    PlaneLayoutConfig *config = ncplane_userptr(plane);
    if (!config)
        return 0;

    unsigned parent_rows, parent_cols;
    ncplane_dim_yx(config->parent_plane, &parent_rows, &parent_cols);

    int new_y = 0;
    int new_x = 0;

    // Recalculate anchor origins on the fly based on layout rules
    switch (config->pin_mode) {
    case PIN_CENTER:
        new_y = (parent_rows - config->fixed_rows) / 2;
        new_x = (parent_cols - config->fixed_cols) / 2;
        break;

    case PIN_BOTTOM_RIGHT:
        // Locks the component right against the bottom edges (e.g. status fields)
        new_y = parent_rows - config->fixed_rows;
        new_x = parent_cols - config->fixed_cols;
        break;

    case PIN_TOP_LEFT:
    default:
        new_y = 0;
        new_x = 0;
        break;
    }

    // Move the plane safely to its newly anchored relative coordinate system
    ncplane_move_yx(plane, new_y, new_x);
    return 0;
}
