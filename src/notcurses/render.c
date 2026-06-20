#include <notcurses/notcurses.h>
#include <string.h>

// Assuming 'view->content.plane' is the destination, and 'current_selection' tracks the active line index
void render_menu_viewport(View *view, int current_selection) {
    Menu *menu = view->menu;
    struct ncplane *cp = view->content.plane;

    // 1. Clear the viewport plane completely before redrawing
    ncplane_erase(cp);

    // 2. Loop through your processed structure items
    for (int i = 0; i < menu->item_count; i++) {
        Line *line = menu->line[i];
        int target_row = i; // Map item index directly to terminal plane row

        // Set cursor to the beginning of the row
        ncplane_cursor_move_yx(cp, target_row, 0);

        if (line->type == MT_TEXT) {
            // Apply standard text attributes
            ncplane_set_channels(cp, CC_NT_CHANNELS);
            ncplane_set_styles(cp, NCSTYLE_NONE);
            ncplane_putstr(cp, line->raw_text);
        } else if (line->type == MT_CHOICE) {
            // Check if this specific item is the one selected by the user
            if (i == current_selection) {
                // Highlight choice: Swap the foreground/background or use a vibrant brand color
                // For instance, white text on a bright blue background:
                uint64_t highlight_channels = 0;
                ncchannels_set_fchannel(&highlight_channels, 0xFFFFFF); // White text
                ncchannels_set_bchannel(&highlight_channels, 0x0055FF); // Blue background

                ncplane_set_channels(cp, highlight_channels);
                ncplane_set_styles(cp, NCSTYLE_BOLD);

                // Print the choice text cleanly over the whole highlighted block
                ncplane_putstr(cp, line->choice_text);
            } else {
                // Regular, unselected menu option
                ncplane_set_channels(cp, CC_NT_CHANNELS);
                ncplane_set_styles(cp, NCSTYLE_NONE);

                // Advanced refinement: Draw the text, but visually highlight the structural shortcut hotkey
                // Print leading spacer " x - " or similar prefix up to the target letter
                ncplane_putstr(cp, line->choice_text);

                // Repaint just the trigger hotkey cell to be underlined and colored differently
                // Parameters: plane, y, x, height, width, stylemask, channel mask
                uint64_t hotkey_channels = 0;
                ncchannels_set_fchannel(&hotkey_channels, 0x00FF00); // Green letter
                ncchannels_set_bchannel(&hotkey_channels, 0x000000); // Black background

                // line->letter_pos dictates the index column offset calculated during parsing
                ncplane_highlighter(cp, target_row, line->letter_pos, 1, 1,
                                    NCSTYLE_UNDERLINE | NCSTYLE_BOLD, hotkey_channels);
            }
        }
    }

    // 3. Final structural step: Push the virtual changes out to the physical terminal display
    // Make sure this is called once at the end of your complete redraw frame cycle, not inside the loop!
    notcurses_render(ncplane_notcurses(cp));
}
