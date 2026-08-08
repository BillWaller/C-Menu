#include <locale.h>
#include <notcurses/notcurses.h>

int main(void) {
    setlocale(LC_ALL, "");

    struct notcurses_options opts = {
        .flags = NCOPTION_INHIBIT_SETLOCALE, // Use the locale we just set
    };
    struct notcurses *nc = notcurses_core_init(&opts, NULL);
    if (!nc)
        return 1;
    struct ncplane *stdplane = notcurses_stdplane(nc);
    const char *text = "Hello, Notcurses!";
    int len = strlen(text);
    ncplane_cursor_move_yx(stdplane, 5, 10);
    for (int i = 0; i < len; i++) {
        struct nccell cell = NCCELL_TRIVIAL_INITIALIZER;
        if (nccell_loadb(stdplane, &cell, &text[i]) < 0)
            break;
        nccell_set_styles(&cell, NCSTYLE_UNDERLINE);
        ncplane_putc(stdplane, &cell);
        nccell_release(stdplane, &cell);
    }
    notcurses_render(nc);
    ncinput ni;
    notcurses_get_blocking(nc, &ni);
    notcurses_stop(nc);
    return 0;
}
