// Dynamically highlight an active line or alert string
ncplane_set_fg_rgb(view->content.plane, 0xFF0000); // Set upcoming text to Red
ncplane_set_bg_rgb(view->content.plane, 0xFFFF00); // Set upcoming background to Yellow
ncplane_putstr_yx(view->content.plane, 2, 0, "CRITICAL WARNING!");
