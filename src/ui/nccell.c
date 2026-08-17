#include <locale.h>

#define _XOPEN_SOURCE_EXTENDED 1
#define __USE_XOPEN 1
#include <notcurses/notcurses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>

size_t ui_nccells_from_text(struct ncplane *stdn, const char *text, struct nccell *cell);

int main() {
    size_t count = 0;
    setlocale(LC_ALL, "");
    struct notcurses_options opts = {0};
    struct notcurses *nc = notcurses_init(&opts, stdout);
    if (!nc)
        return 1;
    struct ncplane *stdn = notcurses_stdplane(nc);
    const char *text = "Hi!";

    size_t len = strlen(text);
    struct nccell *cell_buf = calloc(len, sizeof(struct nccell));
    count = ui_nccells_from_text(stdn, text, cell_buf);

    for (size_t i = 0; i < count; i++) {
        ncplane_putc(stdn, &cell_buf[i]);
        nccell_release(stdn, &cell_buf[i]);
    }
    notcurses_render(nc);
    free(cell_buf);
    notcurses_stop(nc);
    return 0;
}

size_t ui_nccells_from_text(struct ncplane *stdn, const char *text, struct nccell *cell_buf) {
    uint64_t channels = 0;
    ncchannels_set_fg_rgb8(&channels, 255, 255, 0); // Yellow
    size_t count = 0;
    const char *p = text;
    while (*p) {
        nccell_init(&cell_buf[count]);
        int bytes = nccell_load(stdn, &cell_buf[count], p);
        if (bytes < 0)
            break;
        nccell_set_styles(&cell_buf[count], NCSTYLE_NONE);
        nccell_set_channels(&cell_buf[count], channels);
        p += bytes;
        count++;
    }
    return count;
}
