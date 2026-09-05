/** @file ui_hello.c
    @brief Small program to test the ncurses UI
    @author Bill Waller
    Copyright (c) 2026
    MIT License
    billxwaller@gmail.com
    @date 2026-09-02
 */
#define _GNU_SOURCE
#include "common.h"

int main(int argc, char **argv) {
    // ------------------------------------------------------------------
    // 1 - UI Initialization
    Init *init = new_init(argc, argv);     // Create Init structure
    mapp_initialization(init, argc, argv); // Populate Init structure
    UiConfig ui_config = {                 // UI Runtime configuration
                          .border_style = UI_BORDER_ROUNDED,
                          .log_file = "/tmp/mylog.log",
                          .log_level = LOG_LEVEL_COUNT};
    ui_init(&ui_config, init->sio);
    // ------------------------------------------------------------------
    // 2 - UI Create surface
    if (ui_tracked_sfc_box(12, 50, 5, 5, "Test UI Application")) {
        ui_log(ERROR, "ui_surface_box_win_new failed");
        exit(EXIT_FAILURE);
    }
    UiSurface *sfc = ui_surface[sfc_ptr];
    // ------------------------------------------------------------------
    // 3 - Application - display a bordered window and wait for user input
    ui_keypad(sfc, WIN, true);
    ui_mvwaddstr(sfc, WIN, 1, 4, "Hello!");
    ui_mvwaddstr(sfc, WIN, 3, 4, "Press a key or activate the mouse:");
    UiEvent ev;
    int c = ui_get_event(sfc, WIN, NULL, &ev, -1);
    // ------------------------------------------------------------------
    // 4 - UI Shutdown
    ui_shutdown();
    destroy_init(init);
    exit(EXIT_SUCCESS);
}
