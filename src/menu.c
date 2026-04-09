/** @file menu.c
    @brief Command line start-up for C-Menu components
    @author Bill Waller
    Copyright (c) 2025
    MIT License
    billxwaller@gmail.com
    @date 2026-02-09
 */

#include <common.h>
#include <string.h>

/** @brief This function is responsible for cleaning up the terminal state and
   exiting the program.
    @note This function is typically registered to be called when the program
   exits, ensuring that the terminal is properly restored to its original state,
   even if the program encounters an error or is terminated unexpectedly. The
   end_pgm function performs the following actions:
    1. It calls win_del() to delete any windows that may have been created
   during the program's execution.
    2. It calls destroy_curses() to clean up the ncurses library and restore the
   terminal to its normal state.
    3. It calls restore_shell_tioctl() to restore the terminal's input/output
   settings to their original state.
    4. Finally, it calls exit(EXIT_FAILURE) to terminate the program with a
   failure status, indicating that the program has encountered an error or is
   exiting due to an unexpected condition. By defining this function and
   registering it to be called on program exit, you can help ensure that the
   terminal is properly cleaned up and restored, preventing any issues with the
   terminal state after the program has exited. */
static void end_pgm(void) {
    curs_set(1);
    win_del();
    destroy_curses();
    restore_shell_tioctl();
    sig_dfl_mode();
    exit(EXIT_SUCCESS);
}

int mview(Init *);

int main(int argc, char **argv) {
    int rc;
    rc = atexit(end_pgm);
    if (rc != 0) {
        fprintf(stderr, "\nCannot set exit function\n");
        exit(EXIT_FAILURE);
    }
    char pgm_name[MAXLEN];
    capture_shell_tioctl();
    Init *init = new_init(argc, argv);
    SIO *sio = init->sio;
    mapp_initialization(init, argc, argv);
    open_curses(sio);
    sig_prog_mode();
    capture_curses_tioctl();
    win_init_attrs();

    base_name(pgm_name, argv[0]);

    if (!strcmp(pgm_name, "menu")) {
        new_menu(init, init->argc, init->argv, LINES / 14, COLS / 14);
        menu = init->menu;
        menu_engine(init);
    } else if (!strcmp(pgm_name, "form")) {
        init_form(init, init->argc, init->argv, LINES / 14, COLS / 14);
    } else if (!strcmp(pgm_name, "pick")) {
        init_pick(init, init->argc, init->argv, 0, 0);
    } else if (!strcmp(pgm_name, "view")) {
        view = new_view(init);
        if (init->lines > 0 || init->cols > 0)
            mview(init);
        else if (!init_view_full_screen(init))
            view_file(init);
    } else if (!strcmp(pgm_name, "ckeys")) {
        popup_ckeys();
    }
    exit(EXIT_SUCCESS);
}

int mview(Init *init) {
    view = init->view;
    if (init->lines != 0 && view->lines == 0)
        view->lines = init->lines;
    if (view->lines == 0)
        view->lines = LINES * 3 / 4;
    if (view->lines > LINES - 3)
        view->lines = LINES - 3;
    if (init->cols != 0 && view->lines == 0)
        view->cols = init->cols;
    if (view->cols == 0)
        view->cols = COLS * 3 / 4;
    if (view->cols > COLS - 3)
        view->cols = COLS - 3;
    if (init->begy != 0 && view->begy == 0)
        view->begy = init->begy;
    if (view->begy == 0)
        view->begy = (LINES - view->lines) / 5;
    if (view->begy + view->lines > LINES - 2)
        view->begy = LINES - view->lines - 2;
    if (init->begx != 0 && view->begx == 0)
        view->begx = init->begx;
    if (view->begx == 0)
        view->begx = (COLS - view->cols) / 5;
    if (view->begx + view->cols > COLS - 1)
        view->begx = COLS - view->cols - 1;
    view->f_full_screen = false;
    if (!init_view_boxwin(init, init->title)) {
        view_file(init);
        win_del();
    }
    destroy_view(init);
    return 0;
}
