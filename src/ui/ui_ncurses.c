// src/ui/ui_ncurses.c
#include "ui_ncurses_internal.h"
#include <stdlib.h>
#include <string.h>

/** @file ui_ncurses.c
 * @ingroup ui_ncurses
 * @brief Ncurses-based UI implementation.
 *
 * This file implements the UI runtime and surface management using the
 * ncurses library. It provides functions for creating and managing surfaces,
 * handling input, and rendering the UI. The implementation is designed to be
 * modular and can be extended to support additional features as needed.
 */

/** @defgroup ui_ncurses Ncurses UI Implementation
 * @brief UI implementation using the ncurses library.
 *
 * This module provides functions for creating and managing UI surfaces, handling
 * input, and rendering the UI using ncurses. It is designed to be used as part
 * of a larger UI framework and can be extended with additional features as
 * needed.
 */

/** * @brief Create a new UI surface.
 * @ingroup ui_ncurses
 * @param ui The UI runtime instance.
 * @param parent The parent surface, or NULL for a top-level surface.
 * @param rect The rectangle defining the position and size of the surface.
 * @return A pointer to the newly created UiSurface, or NULL on failure.
 */
UiSurface *ui_surface_new(UiRuntime *ui, UiSurface *parent, UiRect rect) {
    UiSurface *s = calloc(1, sizeof(*s));
    if (!s)
        return NULL;

    s->runtime = ui;
    s->parent = parent;
    s->y = rect.y;
    s->x = rect.x;
    s->rows = rect.rows;
    s->cols = rect.cols;

    if (parent && parent->win) {
        s->win = derwin(parent->win, rect.rows, rect.cols, rect.y, rect.x);
    } else {
        s->win = newwin(rect.rows, rect.cols, rect.y, rect.x);
    }

    if (!s->win) {
        free(s);
        return NULL;
    }

    s->pan = new_panel(s->win);
    if (!s->pan) {
        delwin(s->win);
        free(s);
        return NULL;
    }

    return s;
}

/** @brief Destroy a UI surface and free its resources.
 * @ingroup ui_ncurses
 * @param s The UiSurface to destroy.
 */
void ui_surface_destroy(UiSurface *s) {
    if (!s)
        return;
    if (s->pan)
        del_panel(s->pan);
    if (s->win)
        delwin(s->win);
    free(s);
}

/** @brief Move a UI surface to a new position.
 * @ingroup ui_ncurses
 * @param s The UiSurface to move.
 * @param y The new y-coordinate of the surface.
 * @param x The new x-coordinate of the surface.
 * @return 0 on success, or -1 on failure.
 */
int ui_surface_move(UiSurface *s, int y, int x) {
    if (!s)
        return -1;
    s->y = y;
    s->x = x;
    return move_panel(s->pan, y, x);
}

/** @brief Resize a UI surface to new dimensions.
 * @ingroup ui_ncurses
 * @param s The UiSurface to resize.
 * @param rows The new number of rows for the surface.
 * @param cols The new number of columns for the surface.
 * @return 0 on success, or -1 on failure.
 */
int ui_surface_resize(UiSurface *s, int rows, int cols) {
    if (!s)
        return -1;
    s->rows = rows;
    s->cols = cols;
    return wresize(s->win, rows, cols);
}

/** @brief Clear the contents of a UI surface.
 * @ingroup ui_ncurses
 * @param s The UiSurface to clear.
 * @return 0 on success, or -1 on failure.
 */
int ui_surface_clear(UiSurface *s) {
    if (!s)
        return -1;
    wclear(s->win);
    return 0;
}

/** @brief Erase the contents of a UI surface.
 * @ingroup ui_ncurses
 * @param s The UiSurface to erase.
 * @return 0 on success, or -1 on failure.
 */
int ui_surface_erase(UiSurface *s) {
    if (!s)
        return -1;
    werase(s->win);
    return 0;
}

/** @brief Show a UI surface.
 * @ingroup ui_ncurses
 * @param s The UiSurface to show.
 * @return 0 on success, or -1 on failure.
 */
int ui_surface_show(UiSurface *s) {
    if (!s)
        return -1;
    show_panel(s->pan);
    s->hidden = false;
    return 0;
}
/** @brief Hide a UI surface.
 * @ingroup ui_ncurses
 * @param s The UiSurface to hide.
 * @return 0 on success, or -1 on failure.
 */
int ui_surface_hide(UiSurface *s) {
    if (!s)
        return -1;
    hide_panel(s->pan);
    s->hidden = true;
    return 0;
}

/** @brief Move the cursor within a UI surface.
 * @ingroup ui_ncurses
 * @param s The UiSurface to move the cursor in.
 * @param y The new y-coordinate of the cursor.
 * @param x The new x-coordinate of the cursor.
 * @return 0 on success, or -1 on failure.
 */
int ui_cursor_move(UiSurface *s, int y, int x) {
    if (!s)
        return -1;
    return wmove(s->win, y, x);
}
/** @brief Enable or disable the cursor visibility.
 * @ingroup ui_ncurses
 * @param cfg The UI runtime instance.
 * @return 0 on success, or -1 on failure.
 */
UiRuntime *ui_init(const UiConfig *cfg) {
    UiRuntime *ui = calloc(1, sizeof(*ui));
    if (!ui)
        return NULL;

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    if (has_colors()) {
        start_color();
        use_default_colors();
    }

    if (cfg) {
        ui->mouse_enabled = cfg->enable_mouse;
        ui->alt_screen = cfg->enable_alt_screen;
        ui->cursor_visible = cfg->cursor_visible;
    } else {
        ui->cursor_visible = true;
    }

    if (ui->mouse_enabled) {
        mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
    }

    curs_set(ui->cursor_visible ? 1 : 0);
    getmaxyx(stdscr, ui->rows, ui->cols);

    return ui;
}

/** @brief Shutdown the UI runtime and free its resources.
 * @ingroup ui_ncurses
 * @param ui The UiRuntime instance to shutdown.
 */
void ui_shutdown(UiRuntime *ui) {
    if (!ui)
        return;
    endwin();
    free(ui);
}

/** @brief Get the current screen size.
 * @ingroup ui_ncurses
 * @param ui The UiRuntime instance.
 * @param rows Output parameter for the number of rows.
 * @param cols Output parameter for the number of columns.
 */
void ui_get_screen_size(UiRuntime *ui, int *rows, int *cols) {
    if (!ui)
        return;
    getmaxyx(stdscr, ui->rows, ui->cols);
    if (rows)
        *rows = ui->rows;
    if (cols)
        *cols = ui->cols;
}

/** @brief Render the UI by updating panels and refreshing the screen.
 * @ingroup ui_ncurses
 * @param ui The UiRuntime instance.
 * @return 0 on success, or -1 on failure.
 */
int ui_render(UiRuntime *ui) {
    (void)ui;
    update_panels();
    doupdate();
    return 0;
}

/** @brief Clear the entire screen.
 * @ingroup ui_ncurses
 * @param ui The UiRuntime instance.
 * @return 0 on success, or -1 on failure.
 */
int ui_clear_screen(UiRuntime *ui) {
    (void)ui;
    erase();
    return 0;
}

/** @brief Suspend the UI, restoring the terminal to its normal state.
 * @ingroup ui_ncurses
 * @param ui The UiRuntime instance.
 * @return 0 on success, or -1 on failure.
 */
int ui_suspend(UiRuntime *ui) {
    (void)ui;
    def_prog_mode();
    endwin();
    return 0;
}

/** @brief Resume the UI after being suspended, reinitializing the screen.
 * @ingroup ui_ncurses
 * @param ui The UiRuntime instance.
 * @return 0 on success, or -1 on failure.
 */
int ui_resume(UiRuntime *ui) {
    (void)ui;
    reset_prog_mode();
    refresh();
    update_panels();
    doupdate();
    return 0;
}

/** @brief Enable or disable the cursor visibility.
 * @ingroup ui_ncurses
 * @param ui The UiRuntime instance.
 * @param visible true to show the cursor, false to hide it.
 * @return 0 on success, or -1 on failure.
 */
int ui_cursor_enable(UiRuntime *ui, bool visible) {
    if (!ui)
        return -1;
    ui->cursor_visible = visible;
    curs_set(visible ? 1 : 0);
    return 0;
}
