// 2. Draw a form input field using explicit RGB channels
void draw_ual_input_field(struct ncplane *field_plane, const char *style_name, const char *current_value) {
    UAL_Style *s;

    // Find the style inside your uthash table
    HASH_FIND_STR(style_map, style_name, s);
    if (!s) {
        // Fallback default colors if style not found
        ncplane_set_fg_rgb8(field_plane, 255, 255, 255);
        ncplane_set_bg_rgb8(field_plane, 0, 0, 0);
    } else {
        // Apply direct, independent RGB values to the plane
        // No global color pair registration needed!
        ncplane_set_fg_rgb8(field_plane, s->fg_r, s->fg_g, s->fg_b);
        ncplane_set_bg_rgb8(field_plane, s->bg_r, s->bg_g, s->bg_b);
    }

    // Add text formatting attributes if needed (e.g., Underline for an entry field)
    ncplane_set_styles(field_plane, NCSTYLE_UNDERLINE);

    // Clear the input area to paint the background color across the field width
    ncplane_erase(field_plane);

    // Print out the input contents
    ncplane_putstr_yx(field_plane, 0, 0, current_value);
}
