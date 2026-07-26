// Safely tearing down a complex UI panel containing multiple sub-widgets
void ual_destroy_panel_hierarchy(struct ncplane *parent_plane) {
    if (!parent_plane)
        return;

    // A helper macro logic to walk the sub-tree and free allocations safely
    struct ncplane *child;
    while ((child = ncplane_first_child(parent_plane)) != NULL) {
        // Recursively clean up children from the bottom up
        ual_destroy_panel_hierarchy(child);
    }

    // Now that all sub-children allocations are freed, clear the parent
    ual_destroy_panel(parent_plane);
}
