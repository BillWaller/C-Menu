#define _XOPEN_SOURCE 600
#include <notcurses/notcurses.h>
#include <stdio.h>
#include <unistd.h>

int main() {
    char image_file[256] = "ui_log.png";
    struct notcurses_options nopts = {};
    struct notcurses *nc = notcurses_init(&nopts, NULL);
    if (!nc) {
        fprintf(stderr, "Error: Unable to initialize notcurses.\n");
        return EXIT_FAILURE;
    }

    struct ncvisual *ncv = ncvisual_from_file(image_file);
    if (!ncv) {
        fprintf(stderr, "Error: Could not load image file.\n");
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
