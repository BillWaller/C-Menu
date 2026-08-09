#include <locale.h>
#include <notcurses/notcurses.h>
#include <stdlib.h>
#include <string.h>

int main() {
    setlocale(LC_ALL, "");
    struct notcurses_options opts = {0};
    struct notcurses *nc = notcurses_init(&opts, stdout);
    if (!nc)
        return 1;

    struct ncplane *n = notcurses_stdplane(nc);

    const char *text = "Hi!";
    size_t len = strlen(text);
    struct nccell *cells = calloc(len, sizeof(struct nccell));
    uint64_t channels = 0;
    ncchannels_set_fg_rgb8(&channels, 255, 255, 0); // Yellow

    size_t count = 0;
    const char *p = text;

    while (*p) {
        nccell_init(&cells[count]);
        int bytes = nccell_load(n, &cells[count], p);
        if (bytes < 0)
            break;

        nccell_set_styles(&cells[count], NCSTYLE_BOLD);
        nccell_set_channels(&cells[count], channels);

        p += bytes;
        count++;
    }

    for (size_t i = 0; i < count; i++) {
        ncplane_putc(n, &cells[i]);
        nccell_release(n, &cells[i]);
    }

    notcurses_render(nc);

    free(cells);
    notcurses_stop(nc);
    return 0;
}
