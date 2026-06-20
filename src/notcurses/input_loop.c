#include <notcurses/notcurses.h>

void run_menu_loop(struct notcurses *nc) {
    ncinput ni;
    uint32_t id;

    // notcurses_get blocks until an event happens, populating the 'ni' struct
    // It returns the Unicode codepoint, or a unique ID if it's a special function key
    while ((id = notcurses_get(nc, NULL, &ni)) != 'q') {

        // Optional: Filter out key releases if you only care about down-presses
        if (ni.evtype == NCEVTYPE_RELEASE) {
            continue;
        }

        switch (id) {
        case NCKEY_UP:
            move_menu_up();
            break;

        case NCKEY_DOWN:
            move_menu_down();
            break;

        case NCKEY_ENTER:
            execute_action();
            break;

        // Easily check for hotkeys like Ctrl+C or Alt+X
        case 'c':
            if (ni.ctrl) {
                // Handle explicit Ctrl+C execution
            }
            break;
        }

        // After managing your state machine changes, redraw the terminal screen
        notcurses_render(nc);
    }
}
