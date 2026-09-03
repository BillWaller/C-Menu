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
#include <string.h>
#ifdef NCURSES_UI
#include "../ui/ui_ncurses_internal.h"
#include "ui_backend.h"
#include <ncursesw/ncurses.h>
#include <ncursesw/panel.h>
#endif

static void end_pgm(void) {
    exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {
    // Preamble
    int rc;
    char pgm_name[MAXLEN];
    capture_shell_tioctl();

    //
    Init *init = new_init(argc, argv);     // Create Init structure
    mapp_initialization(init, argc, argv); // Populate Init structure
    UiConfig ui_config = {                 // UI Runtime configuration
                          .border_style = UI_BORDER_ROUNDED,
                          .log_file = "/tmp/mylog.log",
                          .log_level = INFO};
    ui_init(&ui_config, init->sio); // Initialize the UI

    rc = atexit(end_pgm);
    if (rc != 0) {
        fprintf(stderr, "\nCannot set exit function\n");
        exit(EXIT_FAILURE);
    }
    sig_prog_mode();
    capture_program_tioctl();
    base_name(pgm_name, argv[0]);

    popup_ckeys();

    // Postamble
    ui_shutdown(); // Shutdown the UI
    destroy_init(init);
    exit(EXIT_SUCCESS);
}
