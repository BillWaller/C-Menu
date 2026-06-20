#include <notcurses/notcurses.h>

int display_multimedia_in_viewer(struct notcurses *nc, struct ncplane *viewer_plane, const char *filepath) {
    // 1. Clear any old text artifacts out of the viewer plane
    ncplane_erase(viewer_plane);

    // 2. Open the image or video file structure
    struct ncvisual *ncv = ncvisual_from_file(filepath);
    if (!ncv) {
        // Fallback or handle error if codec missing
        return -1;
    }

    // 3. Configure the media presentation options
    struct ncvisual_options vopts = {
        .ncp = viewer_plane,             // Render directly over our viewer layout
        .scaling = NCSCALE_SCALE_ASPECT, // Auto-scale to fit window without distortion
        .blitter = NCBLIT_2x2,           // High-density subpixel graphic rendering
    };

    // 4. Blit the image cells straight onto the terminal screen matrix
    ncvisual_blit(nc, ncv, &vopts);

    // Clean up memory holding the source image properties
    ncvisual_destroy(ncv);
    return 0;
}
