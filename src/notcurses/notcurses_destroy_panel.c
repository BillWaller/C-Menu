#include <notcurses/notcurses.h>
#include <stdlib.h>

// Assuming the same tracking structure used in your pinning layout setup:
typedef struct {
    struct ncplane *parent_plane;
    int fixed_rows;
    int fixed_cols;
    // ... any other abstraction data properties you added
} PlaneLayoutConfig;

// Core UAL Component Destructor Wrapper
void ual_destroy_panel(struct ncplane *plane) {
    if (!plane)
        return;

    // 1. Extract the allocation reference cleanly from the runtime engine
    PlaneLayoutConfig *config = ncplane_userptr(plane);

    // 2. Free your custom context payload first to prevent memory tracking leaks
    if (config) {
        free(config);

        // Good hygiene: Clear out the plane reference pointer to avoid dangling lookups
        ncplane_set_userptr(plane, NULL);
    }

    // 3. Hand control to Notcurses to safely pull the plane out of the rendering pile
    ncplane_destroy(plane);
}
