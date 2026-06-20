#include "nc.h"

#include <unistd.h>

int click_y;
int click_x;
int dxwgetch(struct notcurses *nc);

int main(void) {
    struct notcurses_options nopts = {
        .flags = NCOPTION_INHIBIT_SETLOCALE | NCOPTION_SUPPRESS_BANNERS,
    };
    struct notcurses *nc = notcurses_init(&nopts, stdout);
    if (!nc)
        return 1;
    ncplane_putstr_yx(notcurses_stdplane(nc), 0, 0, "Press a key or mouse button: ");
    notcurses_render(nc);
    int c = 0;
    while (c != 'q') {
        c = dxwgetch(nc);
    }
    notcurses_stop(nc);
    return 0;
}

int dxwgetch(struct notcurses *nc) {
    uint32_t c = 0;
    ncinput ni;

    click_y = -1;
    click_x = -1;

    notcurses_mice_enable(nc, NCMICE_ALL_EVENTS);
    while (1) {
        notcurses_cursor_enable(nc, 1, 0);
        // coordinates
        c = notcurses_get(nc, NULL, &ni);
        notcurses_cursor_disable(nc);

        if (c == (uint32_t)NCKEY_EOF || c == 'q') {
            break; // EOF or error
        } else if (c > 0 && c < 0x110000) {
            printf("%lc UTF-8\n", c);
        } else {
            printf("%lc special key or mouse button\n", c);
        }
        if (c == 0)
            continue; // loop again if blocking or empty event
        if (ni.id == NCKEY_BUTTON4 || ni.id == NCKEY_BUTTON5) {
            return (ni.id == NCKEY_BUTTON4) ? NCKEY_UP : NCKEY_DOWN;
        }
        if (ni.id == NCKEY_BUTTON1) {
            if (ni.evtype == NCTYPE_PRESS) {
                int rel_y = ni.y;
                int rel_x = ni.x;
                if (ncplane_translate_abs(notcurses_stdplane(nc), &rel_y, &rel_x)) {
                    // printf("%3d %3d absolute\n", ni.y, ni.x);
                    printf("%3d %3d press\n", rel_y, rel_x);
                }
                break;
            }
            if (ni.evtype == NCTYPE_RELEASE) {
                int rel_y = ni.y;
                int rel_x = ni.x;
                if (ncplane_translate_abs(notcurses_stdplane(nc), &rel_y, &rel_x)) {
                    // printf("%3d %3d absolute\n", ni.y, ni.x);
                    printf("%3d %3d release\n", rel_y, rel_x);
                }
                break;
            }
        }
    }
    return c;
}
