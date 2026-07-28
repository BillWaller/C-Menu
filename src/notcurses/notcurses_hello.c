#define _XOPEN_SOURCE

#include <notcurses/notcurses.h>
#include <unistd.h>

int main(void) {
    struct notcurses *nc = notcurses_core_init(NULL, NULL);
    if (!nc) {
        return 1;
    }
    struct ncplane *n = notcurses_stdplane(nc);
    ncplane_putstr_yx(n, 0, 0, "Hello, Notcurses!");
    notcurses_render(nc);
    sleep(2);
    notcurses_stop(nc);
    return 0;
}
