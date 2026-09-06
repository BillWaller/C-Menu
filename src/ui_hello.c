// ui_hello.c
// Small program to test the ncurses/notcurses UI backends
#define _GNU_SOURCE
#include "common.h"

int main(int argc, char **argv) {
    Init *init = new_init(argc, argv);
    mapp_initialization(init, argc, argv);
    UiConfig ui_config = {.border_style = UI_BORDER_ROUNDED};
    ui_init(&ui_config, init->sio);
    ui_tracked_sfc_box(12, 50, 5, 5, "Test UI Application");
    UiSurface *sfc = ui_surface[sfc_ptr];
    ui_mvwaddstr(sfc, WIN, 1, 4, "Hello!");
    ui_mvwaddstr(sfc, WIN, 3, 4, "Press a key or activate the mouse:");
    ui_render();
    UiEvent ev;
    ui_get_event(sfc, WIN, NULL, &ev, -1);
    ui_shutdown();
    destroy_init(init);
    exit(EXIT_SUCCESS);
}
