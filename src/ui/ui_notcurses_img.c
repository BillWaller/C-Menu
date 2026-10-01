#define _XOPEN_SOURCE 600
#include <notcurses/notcurses.h>
#include <stdio.h>
#include <unistd.h>

#ifdef __USE_GNU_GETTEXT
#include <libintl.h>
#define _(String) gettext(String)
#define gettext_noop(String) String
#define N_(String) gettext_noop(String)
#else
#define _(String) (String)
#define N_(String) String
#define textdomain(Domain)
#define bindtextdomain(Package, Directory)
#endif

int main() {
    struct notcurses_options nopts = {};
    struct notcurses *nc = notcurses_init(&nopts, NULL);
    if (!nc) {
        fprintf(stderr, _("Error: Unable to initialize notcurses.\n"));
        return EXIT_FAILURE;
    }
    struct ncvisual *ncv = ncvisual_from_file("test.png");
    if (!ncv) {
        fprintf(stderr, _("Error: Could not load image file.\n"));
        goto end;
    }
    struct ncvisual_options vopts = {
        .n = notcurses_stdplane(nc),
        .blitter = NCBLIT_PIXEL,
        .flags = NCVISUAL_OPTION_CHILDPLANE,
    };
    struct ncplane *cn = ncvisual_blit(nc, ncv, &vopts);
    if (!cn)
        goto end;
    notcurses_render(nc);
    sleep(2);
end:
    ncvisual_destroy(ncv);
    notcurses_stop(nc);
    return 0;
}
