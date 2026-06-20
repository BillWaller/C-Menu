// Toggle text styles on a plane before printing
ncplane_set_styles(view->content.plane, NCSTYLE_BOLD | NCSTYLE_UNDERLINE);
ncplane_printf_yx(view->content.plane, 0, 0, "Menu Title");

// Clear styles back to regular plain text
ncplane_off_styles(view->content.plane, NCSTYLE_BOLD | NCSTYLE_UNDERLINE);
