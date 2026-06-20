#include <notcurses/notcurses.h>

typedef struct {
    Menu *menu;
    struct ncplane *content_plane;
    struct ncplane *cmdln_plane;
    int chyron;
} View;

void run_main_application_loop(struct notcurses *nc, View *view) {
    int current_selection = 0;
    bool running = true;

    // Initial draw of the menu before waiting for input
    render_menu_viewport(view, current_selection);

    while (running) {
        // Call your ported input function.
        // We pass -1 for blocking mode (equivalent to your original logic)
        int key = dxwgetch(nc, view->content.plane, view->cmdln.plane, view->chyron, -1);

        switch (key) {
        case NCKEY_UP:
            if (current_selection > 0) {
                current_selection--;

                // If your selection moves off the top of the visible screen,
                // scroll the target plane down to bring it into view
                // (Assuming you enabled scrolling via ncplane_set_scrolling)
                // ncplane_scroll_yx(view->content.plane, -1, 0);
            }
            break;

        case NCKEY_DOWN:
            // Guard selection against your item count bounds calculated in parse_menu_description
            if (current_selection < view->menu->item_count - 1) {
                current_selection++;

                // If selection moves off the bottom edge, scroll the plane up
                // ncplane_scroll_yx(view->content.plane, 1, 0);
            }
            break;

        case NCKEY_ENTER:
            // Execute action for the selected item index
            execute_menu_action(view->menu->line[current_selection]);
            break;

        case 'q':
        case 'Q':
            running = false;
            break;

        default:
            // Handle mouse clicks that set 'mouse_plane', 'click_y', and 'click_x' inside dxwgetch
            if (mouse_plane == view->content.plane) {
                // Map the relative mouse click row directly to your menu selection index!
                if (click_y >= 0 && click_y < view->menu->item_count) {
                    current_selection = click_y;
                }
            }
            break;
        }

        // Redraw the screen viewport with the new active selection highlighted
        render_menu_viewport(view, current_selection);
    }
}
