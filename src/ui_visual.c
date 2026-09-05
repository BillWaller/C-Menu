// ui_visual.c
// Small program to test UI Notcurses image display
#define _GNU_SOURCE
#define _XOPEN_SOURCE 600
#include <common.h>

int main(int argc, char **argv) {
    UiMultiMedia mm;
    char image_file[256] = "mountainside_flowers.jpg";
    Init *init = new_init(argc, argv);
    mapp_initialization(init, argc, argv);
    UiConfig ui_config = {.border_style = UI_BORDER_HEAVY};
    ui_init(&ui_config, init->sio);
    ui_display_image(ui->nc, &mm, image_file, -1, -1, 30, 0);
    UiEvent ev;
    int c = ui_get_event(mm.sfc, WIN, NULL, &ev, -1);
    ncvisual_destroy(mm.ncv);
    ui_surface_destroy(mm.sfc);
    ui_render();
    ui_shutdown();
    exit(EXIT_SUCCESS);
}
