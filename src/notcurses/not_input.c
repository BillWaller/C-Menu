#include <notcurses/notcurses.h>

// Note: Replace WINDOW* parameters with struct ncplane*
int dxwgetch(struct notcurses *nc, struct ncplane *plane1, struct ncplane *plane2, Chyron *chyron, int n) {
    uint32_t c = 0;
    ncinput ni;
    struct timespec ts;
    struct timespec *timeout = NULL;

    // Reset global mouse track coordinates
    click_y = -1;
    click_x = -1;
    mouse_plane = NULL; // replacing mouse_win pointer

    // Enable mouse tracking globally (usually done once at app startup, but kept here for 1:1 logic)
    notcurses_mice_enable(nc, NCMICE_ALL);

    // 1. Replicating halfdelay / timeout behavior
    // n == -1 -> Blocking read (timeout remains NULL)
    // n >= 0  -> Calculate non-blocking timeout timespec
    if (n >= 0) {
        int target_ms = (n == 0) ? 100 : (n * 1000); // 1 ncurses delay unit = 100ms
        ts.tv_sec = target_ms / 1000;
        ts.tv_nsec = (target_ms % 1000) * 1000000;
        timeout = &ts;
    }

    do {
        // Show terminal cursor while waiting, hide immediately after
        notcurses_cursor_enable(nc, 1, 1); // Or appropriate cursor coordinates
        c = notcurses_get(nc, timeout, &ni);
        notcurses_cursor_disable(nc);

        // 2. Handle System / Application Signals
        if (sig_received != 0) {
            if (handle_signal(sig_received)) {
                c = display_error(em0, em1, em2, NULL);
            }
            if (c == 'q' || c == 'Q' || c == NCKEY_F09) {
                exit(EXIT_FAILURE);
            }
        }

        // 3. Handle Timeout Expirations (Ncurses ERR equivalent)
        if (n > 0 && c == 0) {
            return 0; // timed out
        }
        if (c == 0)
            continue; // loop again if blocking or empty event

        // 4. Handle Mouse Actions
        if (ni.id == NCKEY_BUTTON4 || ni.id == NCKEY_BUTTON5) {
            // Mouse scroll wheels map straight to direction keys
            return (ni.id == NCKEY_BUTTON4) ? NCKEY_UP : NCKEY_DOWN;
        }

        if (ni.id == NCKEY_BUTTON1) {
            // Notcurses tracks event types natively (Press, Release, Click)
            if (ni.evtype == NCEVTYPE_PRESS) {
                int rel_y = ni.y;
                int rel_x = ni.x;

                // Test Plane 1: Translate global mouse (ni.y, ni.x) to local relative coordinates
                if (ncplane_translate_yx(plane1, notcurses_stdplane(nc), &rel_y, &rel_x)) {
                    // If translation is successful, click falls inside plane1 bounds
                    mouse_plane = plane1;
                    click_y = rel_y;
                    click_x = rel_x;
                }
                // Test Plane 2 (if provided)
                else if (plane2 != NULL) {
                    rel_y = ni.y;
                    rel_x = ni.x;
                    if (ncplane_translate_yx(plane2, notcurses_stdplane(nc), &rel_y, &rel_x)) {
                        mouse_plane = plane2;
                        click_y = rel_y;
                        click_x = rel_x;
                    }
                }

                if (mouse_plane == NULL) {
                    c = 0; // Clicked outside monitored windows, ignore
                    break;
                }

                // 5. Chyron Edge Calculation
                if (mouse_plane == plane2 && chyron) {
                    unsigned max_y, max_x;
                    ncplane_dim_yx(mouse_plane, &max_y, &max_x);
                    if (click_y == (int)max_y - 1) {
                        c = get_chyron_key(chyron, click_x);
                    }
                }
                break;
            }
        }

    } while (c == 0);

    return c;
}
