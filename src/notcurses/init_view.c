#include "nc.h"
#include "../include/common.h"
#include "../include/cm.h"
#include "../include/view.h"
#include <notcurses/notcurses.h>

// Assuming 'nc' is your global or passed structural Notcurses context pointer
// 'view->box.plane', 'view->lnno.plane', etc., are now type 'struct ncplane*'

// -------------------> 1. Box Plane (The Base) <-------------------
struct ncplane_options box_opts = {
    .y = view->begy,
    .x = view->begx,
    .rows = view->lines + 2,
    .cols = view->cols + 2,
    .userptr = view,
    .name = "box_plane"};
// Bound to the standard plane pile base
view->box.plane = ncplane_create(notcurses_stdplane(nc), &box_opts);
if (!view->box.plane) { /* handle error */
    return -1;
}

// Style the background color channel and draw your border
ncplane_set_base(view->box.plane, "", 0, CC_BOX_CHANNELS);
// Note: Notcurses provides a high-level perimeter box function
ncplane_perimeter(view->box.plane, NULL, NULL, 0);

// -------------------> 2. Line Number Plane <-------------------
struct ncplane_options lnno_opts = {
    .y = 1, // Relative to box_plane top-left
    .x = 1,
    .rows = view->lines - 1,
    .cols = view->ln_win_cols,
    .name = "lnno_plane"};
view->lnno.plane = ncplane_create(view->box.plane, &lnno_opts);
if (!view->lnno.plane) { /* handle error */
    return -1;
}

ncplane_set_base(view->lnno.plane, "", 0, CC_LN_CHANNELS);
ncplane_set_scrolling(view->lnno.plane, true); // Replaces scrollok()

// -------------------> 3. Content Plane (Replaces the Pad) <-------------------
// Instead of a giant off-screen pad, Notcurses lets any regular plane grow or scroll.
struct ncplane_options content_opts = {
    .y = 1,
    .x = 1 + view->ln_win_cols,
    .rows = view->lines - 1,
    .cols = view->cols - view->ln_win_cols,
    .name = "content_plane"};
view->content.plane = ncplane_create(view->box.plane, &content_opts);
if (!view->content.plane) { /* handle error */
    return -1;
}

ncplane_set_base(view->content.plane, "", 0, CC_NT_CHANNELS);
ncplane_set_scrolling(view->content.plane, true);

// -------------------> 4. Command Line Plane <-------------------
struct ncplane_options cmdln_opts = {
    .y = view->lines, // Placed on the bottom row inside the border
    .x = 1,
    .rows = 1,
    .cols = view->cols,
    .name = "cmdln_plane"};
view->cmdln.plane = ncplane_create(view->box.plane, &cmdln_opts);
if (!view->cmdln.plane) { /* handle error */
    return -1;
}

ncplane_set_base(view->cmdln.plane, "", 0, CC_NT_CHANNELS);
