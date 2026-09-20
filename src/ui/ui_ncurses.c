/** @file ui_ncurses.c
   @ingroup ui_ncurses
   @brief NCurses UI backend — lifecycle, surface management, and capabilities.

   Implements all UiRuntime and UiSurface operations declared in ui_backend.h
   using the NCurses / panelw library.

   When compiled as part of the main C-Menu build (UAL_LEGACY_COMPAT defined),
   the legacy globals @c screen and @c tty_fp from dwin.c are kept in sync so
   that code not yet migrated to the UAL API continues to work.
*/

#define _XOPEN_SOURCE_EXTENDED 1

#include "cm.h"

#include "ui_ncurses_internal.h"
#ifdef UAL_LEGACY_COMPAT
#include "ui_ncurses_compat.h"
#endif
#include <locale.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/** @defgroup ui_ncurses
   @ingroup ui_backend
   @brief NCurses UI backend — lifecycle, surface management, and capabilities.

   Implements all UiRuntime and UiSurface operations declared in ui_backend.h
   using the NCurses / panelw library.

   When compiled as part of the main C-Menu build (UAL_LEGACY_COMPAT defined),
   the legacy globals @c screen and @c tty_fp from dwin.c are kept in sync so
   that code not yet migrated to the UAL API continues to work.
*/

UiRuntime *ui = NULL;
UiSurface *ui_surface[UI_SFC_MAX];

int sfc_ptr = -1;
uint ui_color_cnt = 0;
uint ui_pair_cnt = 0;

UiSurface *stdsfc;

/* -------------------------------------------------------------------------
   Backend identification and capability query
   ------------------------------------------------------------------------- */
/** @brief Get the UI backend type.
 * @ingroup ui_ncurses
 * @return The UI backend type, which is always UI_BACKEND_NCURSES for this implementation.
 */
UiBackend ui_get_backend() {
    return UI_BACKEND_NCURSES;
}
/** @brief Get the UI backend capabilities.
 *
 * @param caps Pointer to a UiCaps struct to be filled with the backend capabilities.
 */
void ui_get_caps(UiCaps *caps) {
    if (!caps)
        return;
    memset(caps, 0, sizeof(*caps));
    if (!ui)
        return;
    // Will add code to actually check later. For now, just lie.
    caps->truecolor = true;
    caps->palette256 = true;
    caps->mouse = ui->mouse_enabled;
    caps->unicode = true;
    caps->resize = true;
    caps->color_pairs = 0;
}

/* -------------------------------------------------------------------------
   Styles
   ------------------------------------------------------------------------- */
/** @brief Convert hex color strings to a curses color pair index.
 *
 * @param fg Foreground color in hex format (e.g., "#RRGGBB").
 * @param bg Background color in hex format (e.g., "#RRGGBB").
 * @return The curses color pair index corresponding to the given foreground and background colors.
 */
int ui_pair_from_hex(const char *fg, const char *bg) {
    RGB rgb;
    sscanf(fg, "#%02x%02x%02x", &rgb.r, &rgb.g, &rgb.b);
    int f_idx = ui_color_from_rgb(&rgb);
    sscanf(bg, "#%02x%02x%02x", &rgb.r, &rgb.g, &rgb.b);
    int b_idx = ui_color_from_rgb(&rgb);
    return ui_add_pair(f_idx, b_idx);
}
/** @brief Convert a Unicode code point to a curses UiCell with specified foreground and background colors.
 *
 * @param ucp Pointer to a wide character string containing the Unicode code point.
 * @param fg Pointer to a 32-bit integer representing the foreground color in hex format (e.g., 0xRRGGBB).
 * @param bg Pointer to a 32-bit integer representing the background color in hex format (e.g., 0xRRGGBB).
 * @return A UiCell structure representing the Unicode character with the specified colors.
 */
UiCell ui_cell_from_ucp(const wchar_t *ucp, const uint32_t *fg, const uint32_t *bg) {
    UiCell cc = {0};
    RGB rgb;
    rgb.r = (*fg >> 16) & 0xff;
    rgb.g = (*fg >> 8) & 0xff;
    rgb.b = *fg & 0xff;
    int f_idx = ui_color_from_rgb(&rgb);
    rgb.r = (*bg >> 16) & 0xff;
    rgb.g = (*bg >> 8) & 0xff;
    rgb.b = *bg & 0xff;
    int b_idx = ui_color_from_rgb(&rgb);
    short cp = ui_add_pair(f_idx, b_idx);
    wchar_t wstr[2] = {L'\0', L'\0'};
    wstr[0] = *ucp;
    wstr[1] = L'\0';
    setcchar(&cc, wstr, WA_NORMAL, cp, nullptr);
    return cc;
}

/* -------------------------------------------------------------------------
   Lifecycle
   ------------------------------------------------------------------------- */
/** @brief Initialize the UI runtime with the given configuration and SIO.
 *
 * @param cfg Pointer to a UiConfig structure containing the configuration settings.
 * @param sio Pointer to an SIO structure for input/output operations.
 * @return A pointer to the initialized UiRuntime structure, or NULL on failure.
 */
struct UiRuntime *ui_init(const UiConfig *cfg, SIO *sio) {
    setlocale(LC_ALL, "en_US.UTF-8");
    ui = calloc(1, sizeof(*ui));
    if (!ui)
        return NULL;
    char tty_name[MAXLEN];
    if (ttyname_r(STDERR_FILENO, tty_name, sizeof(tty_name)) != 0) {
        free(ui);
        return NULL;
    }
    ui->tty_fp = fopen(tty_name, "r+");
    if (!ui->tty_fp) {
        free(ui);
        return NULL;
    }
    if (cfg->log_level >= FATAL)
        ui_min_log_level = cfg->log_level;

    ui_log(INFO, "ui_init: using tty: %s", tty_name);
    ui->screen = newterm(NULL, ui->tty_fp, ui->tty_fp);
    f_ncurses_open = true;
    if (!ui->screen) {
        fclose(ui->tty_fp);
        free(ui);
        return NULL;
    }
    set_term(ui->screen);
    if (!has_colors() || !can_change_color()) {
        ui_shutdown();
        return NULL;
    }
    start_color();
    use_default_colors();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    if (ui->mouse_enabled)
        mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
    getmaxyx(stdscr, ui->lines, ui->cols);

    stdsfc = calloc(1, sizeof(*stdsfc));
    if (!stdsfc)
        return NULL;
    stdsfc->runtime = ui;
    stdsfc->parent = NULL;
    stdsfc->meta[BOX].lines = (int)ui->lines;
    stdsfc->meta[BOX].cols = (int)ui->cols;
    stdsfc->meta[BOX].y = 0;
    stdsfc->meta[BOX].x = 0;
    stdsfc->mwin[BOX] = stdscr;
    stdsfc->mpan[BOX] = new_panel(stdsfc->mwin[BOX]);
    if (!stdsfc->mpan[BOX]) {
        free(stdsfc);
        ui_log(ERROR, "new_panel failed for stdsfc->mpan[BOX]");
        exit(EXIT_FAILURE);
    }
    if (cfg->border_style)
        ui->border_style = cfg->border_style;
    if (!ui->border_style)
        ui->border_style = UI_BORDER_ROUNDED;
    if (cfg) {
        switch (ui->border_style) {
        case UI_BORDER_SINGLE:
            memcpy(bw.str, border_single, sizeof(bw.str));
            break;
        case UI_BORDER_DOUBLE:
            memcpy(bw.str, border_double, sizeof(bw.str));
            break;
        case UI_BORDER_ROUNDED:
            memcpy(bw.str, border_rounded, sizeof(bw.str));
            break;
        case UI_BORDER_HEAVY:
            memcpy(bw.str, border_heavy, sizeof(bw.str));
            break;
        case UI_BORDER_NONE:
            memcpy(bw.str, border_none, sizeof(bw.str));
            break;
        default:
            memcpy(bw.str, border_rounded, sizeof(bw.str));
            break;
        }
    }
    ui_initialize_sio(sio);
    ui_log(INFO, "ui_init: stdsfc->mwin[BOX]: %p, stdsfc->mpan[BOX]: %p", (void *)stdsfc->mwin[BOX], (void *)stdsfc->mpan[BOX]);
    return ui;
}
// -------------------------------------------------------------------------
// Surface Creation and Destruction
// -------------------------------------------------------------------------
/** @brief Create a new UI surface with the specified parameters.
 *
 * @param w The index of the window within the surface.
 * @param parent Pointer to the parent UiSurface, or NULL for a top-level surface.
 * @param p The index of the parent window to derive from, if applicable.
 * @param lines The number of lines (rows) for the new surface.
 * @param cols The number of columns for the new surface.
 * @param y The y-coordinate (row) for the new surface's position.
 * @param x The x-coordinate (column) for the new surface's position.
 * @return A pointer to the newly created UiSurface, or NULL on failure.
 */
UiSurface *ui_surface_new(ss_t w, UiSurface *parent, uint p, uint lines, uint cols, uint y, uint x) {
    if (!ui)
        return NULL;
    uint maxy, maxx;
    ui_get_screen_size(&maxy, &maxx);
    if (lines > maxy || cols > maxx) {
        ssnprintf(em0, MAXLEN - 1, "%s, line: %d", __FILE__, __LINE__ - 1);
        ssnprintf(em1, MAXLEN - 1, "ui_surface_new failed for lines: %d, cols: %d", lines, cols);
        ssnprintf(em2, MAXLEN - 1, "maxy: %d, maxx: %d", maxy, maxx);
        ui_display_error(em0, em1, em2, nullptr);
        return NULL;
    }

    UiSurface *s = calloc(1, sizeof(*s));
    if (!s)
        return NULL;
    for (int i = 0; i < SUB_SFC_MAX; i++) {
        s->mwin[i] = NULL;
        s->mpan[i] = NULL;
    }
    s->runtime = ui;
    s->parent = parent;
    s->meta[w].lines = lines;
    s->meta[w].cols = cols;
    s->meta[w].y = y;
    s->meta[w].x = x;

    if (parent && parent->mwin[p]) {
        s->mwin[w] = derwin(parent->mwin[p], lines, cols, y, x);
        if (!s->mwin[w]) {
            free(s);
            return NULL;
        }
    } else {
        s->mwin[w] = newwin(lines, cols, y, x);
        if (!s->mwin[w]) {
            free(s);
            return NULL;
        }
        s->mpan[w] = new_panel(s->mwin[w]);
        if (!s->mpan[w]) {
            delwin(s->mwin[w]);
            free(s);
            return NULL;
        }
    }
    return s;
}
/** @brief Create a new UI surface with a border box and optional title.
 *
 * @param parent Pointer to the parent UiSurface, or NULL for a top-level surface.
 * @param p The index of the parent window to derive from, if applicable.
 * @param lines The number of lines (rows) for the new surface.
 * @param cols The number of columns for the new surface.
 * @param y The y-coordinate (row) for the new surface's position.
 * @param x The x-coordinate (column) for the new surface's position.
 * @param wtitle The title string to display in the border box, or NULL for no title.
 * @return A pointer to the newly created UiSurface with a border box, or NULL on failure.
 */
UiSurface *ui_surface_box(UiSurface *parent, uint p, uint lines, uint cols, uint y, uint x, const char *wtitle) {
    if (!ui)
        return NULL;
    uint maxy, maxx;
    ui_get_screen_size(&maxy, &maxx);
    if (lines > maxy || cols > maxx) {
        ssnprintf(em0, MAXLEN - 1, "%s, line: %d", __FILE__, __LINE__ - 1);
        ssnprintf(em1, MAXLEN - 1, "ui_surface_box failed for lines: %d, cols: %d", lines, cols);
        ssnprintf(em2, MAXLEN - 1, "maxy: %d, maxx: %d", maxy, maxx);
        ui_display_error(em0, em1, em2, nullptr);
        return NULL;
    }
    UiSurface *s = calloc(1, sizeof(*s));
    if (!s)
        return NULL;
    s->runtime = ui;
    s->parent = parent;
    s->meta[BOX].y = y;
    s->meta[BOX].x = x;
    s->meta[BOX].lines = lines;
    s->meta[BOX].cols = cols;
    if (parent && parent->mwin[p]) {
        s->mwin[BOX] = derwin(parent->mwin[p], lines, cols, y, x);
        if (!s->mwin[BOX]) {
            free(s);
            return NULL;
        }
    } else {
        s->mwin[BOX] = newwin(lines, cols, y, x);
        if (!s->mwin[BOX]) {
            free(s);
            return NULL;
        }
        s->mpan[BOX] = new_panel(s->mwin[BOX]);
        if (!s->mpan[BOX]) {
            delwin(s->mwin[BOX]);
            return NULL;
        }
    }
    ui_scrollok(s, BOX, false);
    ui_border_draw(s);
    ui_border_title(s, wtitle);
#ifdef DEBUG_UI
    immedok(s->mwin[BOX], true);
#endif
    return s;
}
/** @brief Exit the UI application quickly, cleaning up resources.
 *
 * @param s Pointer to the UiSurface to be cleaned up before exiting.
 */
void fast_exit(UiSurface *s) {
    update_panels();
    doupdate();
    for (int i = SUB_SFC_MAX; i >= 0; i--) {
        if (s->mpan[i]) {
            hide_panel(s->mpan[i]);
            del_panel(s->mpan[i]);
            s->mpan[i] = NULL;
        }
    }
    for (int i = SUB_SFC_MAX; i >= 0; i--) {
        if (s->mwin[i]) {
            delwin(s->mwin[i]);
            s->mwin[i] = NULL;
        }
    }
    endwin();
    exit(EXIT_SUCCESS);
}
/** @brief Add a new pad to the specified UiSurface.
 *
 * @param s Pointer to the UiSurface to which the pad will be added.
 * @param w The index of the window within the surface where the pad will be created.
 * @param view_win The index of the window that will be used to view the pad.
 * @param lines The number of lines (rows) for the new pad.
 * @param cols The number of columns for the new pad.
 * @param begy The y-coordinate (row) for the new pad's position (ignored in this implementation).
 * @param begx The x-coordinate (column) for the new pad's position (ignored in this implementation).
 * @return 0 on success, or -1 on failure.
 */
int ui_surface_addpad(UiSurface *s, ss_t w, uint view_win, uint lines, uint cols, uint begy, uint begx) {
    (void)begy;
    (void)begx;
    s->mwin[w] = newpad(lines, cols);
    if (s->mwin[w] == nullptr)
        return -1;
    s->mwin[view_win] = subpad(s->mwin[PAD], lines, cols, 0, 0);
    if (s->mwin[view_win] == nullptr)
        return -1;
    s->mpan[w] = new_panel(s->mwin[view_win]);
    immedok(s->mwin[w], true);
#ifdef DEBUG_UI
#endif
    return 0;
}
/** @brief Add a new window to the specified UiSurface.
 *
 * @param s Pointer to the UiSurface to which the window will be added.
 * @param w The index of the window within the surface where the new window will be created.
 * @param p The index of the parent window from which the new window will be derived.
 * @param lines The number of lines (rows) for the new window.
 * @param cols The number of columns for the new window.
 * @param y The y-coordinate (row) for the new window's position relative to the parent window.
 * @param x The x-coordinate (column) for the new window's position relative to the parent window.
 * @return 0 on success, or -1 on failure.
 */
int ui_surface_addwin(UiSurface *s, ss_t w, uint p, uint lines, uint cols, uint y, uint x) {
    s->mwin[w] = derwin(s->mwin[p], lines, cols, y, x);
    if (!s->mwin[w]) {
        free(s);
        return -1;
    }
    s->mpan[w] = new_panel(s->mwin[w]);
    keypad(s->mwin[w], true);
    ui_bkgd(s, w, &cell_nt);
    ui_bkgdset(s, w, &cell_nt);
    immedok(s->mwin[w], true);
#ifdef DEBUG_UI
#endif
    return 0;
}
/** @brief End the UI application, cleaning up resources and shutting down the UI runtime.
 */
void ui_endwin() {
    endwin();
    // ui_shutdown();
}
/** @brief Shutdown the UI runtime, cleaning up resources and freeing memory.
 */
void ui_shutdown() {
    if (ui == NULL)
        return;
    ui_log(INFO, "ui_shutdown in progress...");
    while (sfc_ptr >= 0) {
        if (ui_surface[sfc_ptr] != NULL) {
            ui_surface_destroy(ui_surface[sfc_ptr]);
            ui_surface[sfc_ptr] = NULL;
        }
        ui_log(INFO, "surface destroy: %d", sfc_ptr);
        sfc_ptr--;
    }
    if (stdsfc->mpan[0] != NULL) {
        ui_log(INFO, "calling del_panel(stdsfc->mpan[BOX])");
        hide_panel(stdsfc->mpan[0]);
        del_panel(stdsfc->mpan[0]);
        stdsfc->mpan[0] = NULL;
    }
    ui_log(INFO, "calling endwin()");
    endwin();
    f_ncurses_open = false;
    if (ui->screen != NULL) {
        ui_log(INFO, "calling delscreen(ui->screen)");
        delscreen(ui->screen);
        ui->screen = NULL;
    }
    if (ui->tty_fp != NULL) {
        ui_log(INFO, "closing tty_fp");
        fclose(ui->tty_fp);
        ui->tty_fp = NULL;
    }
    if (stdsfc != NULL) {
        ui_log(INFO, "freeing stdsfc");
        free(stdsfc);
        stdsfc = NULL;
    }
    if (ui != NULL) {
        if (ui->sio) {
            free(ui->sio);
            ui->sio = nullptr;
        }
        ui_log(INFO, "freeing ui");
        free(ui);
        ui = NULL;
    }
}
/** @brief Destroy a UiSurface, cleaning up its windows and panels.
 *
 * @param s Pointer to the UiSurface to be destroyed.
 */
void ui_surface_destroy(UiSurface *s) {
    if (!s)
        return;
    for (int i = SUB_SFC_MAX; i >= 0; i--) {
        if (s->mpan[i] != NULL) {
            hide_panel(s->mpan[i]);
            ui_render();
            del_panel(s->mpan[i]);
            s->mpan[i] = NULL;
        }
    }
    for (int i = SUB_SFC_MAX; i >= 0; i--) {
        if (s->mwin[i] != NULL) {
            delwin(s->mwin[i]);
            s->mwin[i] = NULL;
        }
    }
    if (s != NULL) {
        free(s);
        s = NULL;
    }
}
/** @brief Suspend the UI, saving the current program state and ending the window session.
 *
 * @return 0 on success, or -1 on failure.
 */
int ui_suspend() {
    def_prog_mode();
    endwin();
    return 0;
}
/** @brief Resume the UI, restoring the previous program state and updating the display.
 *
 * @return 0 on success, or -1 on failure.
 */
int ui_resume() {
    reset_prog_mode();
    update_panels();
    doupdate();
    return 0;
}

// -------------------------------------------------------------------------
// Surface Management
// -------------------------------------------------------------------------
/** @brief Move a UiSurface to a new position on the screen.
 *
 * @param s Pointer to the UiSurface to be moved.
 * @param w The index of the window within the surface to be moved.
 * @param y The new y-coordinate (row) for the surface's position.
 * @param x The new x-coordinate (column) for the surface's position.
 * @return 0 on success, or -1 on failure.
 */
int ui_surface_move(UiSurface *s, ss_t w, uint y, uint x) {
    if (!s)
        return -1;
    s->meta[w].y = y;
    s->meta[w].x = x;
    return move_panel(s->mpan[w], y, x);
}
/** @brief Resize a UiSurface to new dimensions.
 *
 * @param s Pointer to the UiSurface to be resized.
 * @param w The index of the window within the surface to be resized.
 * @param lines The new number of lines (rows) for the surface.
 * @param cols The new number of columns for the surface.
 * @return 0 on success, or -1 on failure.
 */
int ui_surface_resize(UiSurface *s, ss_t w, uint lines, uint cols) {
    if (!s)
        return -1;
    s->meta[w].lines = lines;
    s->meta[w].cols = cols;
    wresize(s->mwin[w], lines + 2, cols + 2);
    wresize(s->mwin[w + 1], lines, cols);
    return 0;
}
/** @brief Erase the contents of a UiSurface or all surfaces.
 *
 * @param s Pointer to the UiSurface to be erased, or NULL to erase all surfaces.
 * @param w The index of the window within the surface to be erased, or ALLWINS to erase all windows.
 * @return 0 on success, or -1 on failure.
 */
int ui_werase(UiSurface *s, ss_t w) {
    if (w == ALLWINS) {
        for (int i = 1; i < SUB_SFC_MAX; i++) {
            if (s->mwin[i]) {
                werase(s->mwin[i]);
            }
        }
    } else {
        if (s->mwin[w]) {
            werase(s->mwin[w]);
        }
    }
    return 0;
}
/** @brief Clear the contents of a UiSurface or all surfaces.
 *
 * @param s Pointer to the UiSurface to be cleared, or NULL to clear all surfaces.
 * @param w The index of the window within the surface to be cleared, or ALLWINS to clear all windows.
 * @return 0 on success, or -1 on failure.
 */
int ui_wclear(UiSurface *s, ss_t w) {
    if (!s)
        return -1;
    if (w == ALLWINS) {
        for (int i = 1; i < SUB_SFC_MAX; i++) {
            if (s->mwin[i]) {
                wclear(s->mwin[i]);
            }
        }
    } else {
        if (s->mwin[w]) {
            wclear(s->mwin[w]);
        }
    }
    return 0;
}
/** @brief Erase the entire screen.
 *
 * @return 0 on success, or -1 on failure.
 */
int ui_erase() {
    erase();
    return 0;
}
/** @brief Clear the entire screen.
 *
 * @return 0 on success, or -1 on failure.
 */
int ui_clear() {
    clear();
    return 0;
}
/** @brief Show a UiSurface, making it visible on the screen.
 *
 * @param s Pointer to the UiSurface to be shown.
 * @param w The index of the window within the surface to be shown.
 * @return 0 on success, or -1 on failure.
 */
int ui_surface_show(UiSurface *s, ss_t w) {
    if (!s)
        return -1;
    show_panel(s->mpan[w]);
    s->meta[w].hidden = false;
    return 0;
    return 0;
}
/** @brief Hide a UiSurface, making it invisible on the screen.
 *
 * @param s Pointer to the UiSurface to be hidden.
 * @param w The index of the window within the surface to be hidden.
 * @return 0 on success, or -1 on failure.
 */
int ui_top_surface(UiSurface *s, ss_t w) {
    if (!s)
        return -1;
    top_panel(s->mpan[w]);
    return 0;
}
/** @brief Hide a UiSurface, making it invisible on the screen.
 *
 * @param s Pointer to the UiSurface to be hidden.
 * @param w The index of the window within the surface to be hidden.
 * @return 0 on success, or -1 on failure.
 */
int ui_surface_hide(UiSurface *s, ss_t w) {
    if (!s)
        return -1;
    hide_panel(s->mpan[w]);
    s->meta[w].hidden = true;
    return 0;
}
// -------------------------------------------------------------------------
// Screen Navigation
// -------------------------------------------------------------------------
/** @brief Move the cursor to a specified position within a UiSurface.
 *
 * @param s Pointer to the UiSurface where the cursor will be moved.
 * @param w The index of the window within the surface where the cursor will be moved.
 * @param y The new y-coordinate (row) for the cursor's position.
 * @param x The new x-coordinate (column) for the cursor's position.
 * @return 0 on success, or -1 on failure.
 */
int ui_wmove(UiSurface *s, ss_t w, uint y, uint x) {
    if (!s || !s->mwin[w])
        return -1;
    return wmove(s->mwin[w], y, x);
}
/** @brief Move the cursor to a specified position within a UiSurface.
 *
 * @param s Pointer to the UiSurface where the cursor will be moved.
 * @param w The index of the window within the surface where the cursor will be moved.
 * @param y The new y-coordinate (row) for the cursor's position.
 * @param x The new x-coordinate (column) for the cursor's position.
 * @return 0 on success, or -1 on failure.
 */
int ui_cursor_move(UiSurface *s, ss_t w, uint y, uint x) {
    if (!s || !s->mwin[w])
        return -1;
    return wmove(s->mwin[w], y, x);
}
// ui_cursor_yx(*int y, int *x);
//
// ui_abs_yx(UiSurface *s, ss_t w, int *y, int *x);
//
void ui_getyx(UiSurface *s, ss_t w, uint *lines, uint *cols) {
    if (!s->mwin[w])
        return;
    int _lines, _cols;
    getyx(s->mwin[w], _lines, _cols);
    *lines = (uint)(_lines);
    *cols = (uint)(_cols);
}
/** @brief Get the maximum dimensions of a UiSurface.
 *
 * @param s Pointer to the UiSurface for which to retrieve the maximum dimensions.
 * @param w The index of the window within the surface for which to retrieve the maximum dimensions.
 * @param lines Pointer to a uint variable where the maximum number of lines (rows) will be stored.
 * @param cols Pointer to a uint variable where the maximum number of columns will be stored.
 */
void ui_getmaxyx(UiSurface *s, ss_t w, uint *lines, uint *cols) {
    if (!s->mwin[w])
        return;
    getmaxyx(s->mwin[w], *lines, *cols);
}
/** @brief Get the maximum number of lines (rows) of a UiSurface.
 *
 * @param s Pointer to the UiSurface for which to retrieve the maximum number of lines.
 * @param w The index of the window within the surface for which to retrieve the maximum number of lines.
 * @return The maximum number of lines (rows) of the specified UiSurface, or -1 on failure.
 */
int ui_getmaxy(UiSurface *s, ss_t w) {
    if (!s->mwin[w])
        return -1;
    return (int)(getmaxy(s->mwin[w]));
}
/** @brief Get the maximum number of columns of a UiSurface.
 *
 * @param s Pointer to the UiSurface for which to retrieve the maximum number of columns.
 * @param w The index of the window within the surface for which to retrieve the maximum number of columns.
 * @return The maximum number of columns of the specified UiSurface, or -1 on failure.
 */
int ui_getmaxx(UiSurface *s, ss_t w) {
    if (!s->mwin[w])
        return -1;
    return (int)(getmaxx(s->mwin[w]));
}
/** @brief Get the current screen size (number of lines and columns).
 *
 * @param lines Pointer to a uint variable where the number of lines (rows) will be stored.
 * @param cols Pointer to a uint variable where the number of columns will be stored.
 */
void ui_get_screen_size(uint *lines, uint *cols) {
    if (!ui)
        return;
    getmaxyx(stdscr, ui->lines, ui->cols);
    if (lines)
        *lines = ui->lines;
    if (cols)
        *cols = ui->cols;
}
/** @brief Scroll the contents of a UiSurface by a specified number of lines.
 *
 * @param s Pointer to the UiSurface to be scrolled.
 * @param w The index of the window within the surface to be scrolled.
 * @param n The number of lines to scroll. Positive values scroll down, negative values scroll up.
 * @return 0 on success, or -1 on failure.
 */
int ui_wscrl(UiSurface *s, ss_t w, int n) {
    if (!s->mwin[w])
        return -1;
    wscrl(s->mwin[w], n);
    return 0;
}

// -------------------------------------------------------------------------
// Configuration Control
// -------------------------------------------------------------------------
/** @brief Enable or disable scrolling for a UiSurface.
 *
 * @param s Pointer to the UiSurface for which to enable or disable scrolling.
 * @param w The index of the window within the surface for which to enable or disable scrolling.
 * @param enable A boolean value indicating whether to enable (true) or disable (false) scrolling.
 * @return 0 on success, or -1 on failure.
 */
int ui_scrollok(UiSurface *s, ss_t w, bool enable) {
    if (!s->mwin[w])
        return -1;
    if (enable)
        scrollok(s->mwin[w], true);
    else
        scrollok(s->mwin[w], false);
    return 0;
}
/** @brief Enable or disable the keypad for a UiSurface.
 *
 * @param s Pointer to the UiSurface for which to enable or disable the keypad.
 * @param w The index of the window within the surface for which to enable or disable the keypad.
 * @param enable A boolean value indicating whether to enable (true) or disable (false) the keypad.
 * @return 0 on success, or -1 on failure.
 */
int ui_keypad(UiSurface *s, ss_t w, bool enable) {
    if (!s->mwin[w])
        return -1;
    if (enable)
        keypad(s->mwin[w], true);
    else
        keypad(s->mwin[w], false);
    return 0;
}
/** @brief Enable or disable the use of the insert/delete line capabilities for a UiSurface.
 *
 * @param s Pointer to the UiSurface for which to enable or disable the insert/delete line capabilities.
 * @param w The index of the window within the surface for which to enable or disable the insert/delete line capabilities.
 * @param enable A boolean value indicating whether to enable (true) or disable (false) the insert/delete line capabilities.
 * @return 0 on success, or -1 on failure.
 */
int ui_idlok(UiSurface *s, ss_t w, bool enable) {
    if (!s->mwin[w])
        return -1;
    if (enable)
        idlok(s->mwin[w], true);
    else
        idlok(s->mwin[w], false);
    return 0;
}
/** @brief Enable or disable the use of the insert/delete character capabilities for a UiSurface.
 *
 * @param s Pointer to the UiSurface for which to enable or disable the insert/delete character capabilities.
 * @param w The index of the window within the surface for which to enable or disable the insert/delete character capabilities.
 * @param enable A boolean value indicating whether to enable (true) or disable (false) the insert/delete character capabilities.
 * @return 0 on success, or -1 on failure.
 */
int ui_idcok(UiSurface *s, ss_t w, bool enable) {
    if (!s->mwin[w])
        return -1;
    if (enable)
        idcok(s->mwin[w], true);
    else
        idcok(s->mwin[w], false);
    return 0;
}
/** @brief Set the scrolling region for a UiSurface.
 *
 * @param s Pointer to the UiSurface for which to set the scrolling region.
 * @param w The index of the window within the surface for which to set the scrolling region.
 * @param top The top line of the scrolling region (0-based).
 * @param bottom The bottom line of the scrolling region (0-based).
 * @return 0 on success, or -1 on failure.
 */
int ui_setscrreg(UiSurface *s, ss_t w, uint top, uint bottom) {
    if (!s->mwin[w])
        return -1;
    wsetscrreg(s->mwin[w], top, bottom);
    return 0;
}
// -------------------------------------------------------------------------
// Cursor Control
// -------------------------------------------------------------------------
/** @brief Set the visibility of the cursor.
 *
 * @param visibility An integer value indicating the desired cursor visibility:
 *                   0 - Invisible
 *                   1 - Visible
 *                   2 - Very visible (e.g., block cursor)
 * @return 0 on success, or -1 on failure.
 */
int ui_curs_set(int visibility) {
    curs_set(visibility);
    return 0;
}
// included for symetry with notcurses, which needs it to compensate
// for cursor positioning bug
/** @brief Enable or disable the cursor at a specific position within a UiSurface.
 *
 * @param s Pointer to the UiSurface where the cursor will be enabled or disabled.
 * @param w The index of the window within the surface where the cursor will be enabled or disabled.
 * @param y The y-coordinate (row) for the cursor's position.
 * @param x The x-coordinate (column) for the cursor's position.
 * @param visible A boolean value indicating whether to make the cursor visible (true) or invisible (false).
 * @return 0 on success, or -1 on failure.
 */
int ui_cursor_enable_yx(UiSurface *s, ss_t w, uint y, uint x, bool visible) {
    if (!ui)
        return -1;
    wmove(s->mwin[w], y, x);
    ui->cursor_visible = visible;
    curs_set(visible ? 2 : 0);
    return 0;
}
/** @brief Enable or disable the cursor for a UiSurface.
 *
 * @param s Pointer to the UiSurface where the cursor will be enabled or disabled.
 * @param w The index of the window within the surface where the cursor will be enabled or disabled.
 * @param visible A boolean value indicating whether to make the cursor visible (true) or invisible (false).
 * @return 0 on success, or -1 on failure.
 */
int ui_cursor_enable(UiSurface *s, ss_t w, bool visible) {
    (void)s;
    (void)w;
    if (!ui)
        return -1;
    ui->cursor_visible = visible;
    curs_set(visible ? 1 : 0);
    return 0;
}
/* -------------------------------------------------------------------------
   Rendering
   ------------------------------------------------------------------------- */
/** @brief Render the UI by updating panels and refreshing the display.
 */
void ui_render() {
    update_panels();
    doupdate();
}
/** @brief Update the panels in the UI.
 */
void ui_update_panels() {
    update_panels();
}
/** @brief Refresh the display by updating the screen.
 */
int ui_doupdate() {
    doupdate();
    return 0;
}
/** -------------------------------------------------------------------------
   Refreshing
   ------------------------------------------------------------------------- */
/** @brief Refresh a specific window within a UiSurface.
 *
 * @param s Pointer to the UiSurface containing the window to be refreshed.
 * @param w The index of the window within the surface to be refreshed.
 * @return 0 on success, or -1 on failure.
 */
int ui_wnoutrefresh(UiSurface *s, ss_t w) {
    if (!s)
        return -1;
    wnoutrefresh(s->mwin[w]);
    return 0;
}

// -------------------------------------------------------------------------
// Background
// -------------------------------------------------------------------------

// for the entire window
/** @brief Set the background of a specific window within a UiSurface.
 *
 * @param s Pointer to the UiSurface containing the window.
 * @param w The index of the window within the surface for which to set the background.
 * @param cell Pointer to a UiCell structure representing the desired background attributes.
 * @return 0 on success, or -1 on failure.
 */
int ui_bkgd(UiSurface *s, ss_t w, const UiCell *cell) {
    if (!s)
        return -1;
    wbkgrnd(s->mwin[w], cell);
    return 0;
}
// for new content to be written to the window
/** @brief Set the background for new content to be written to a specific window within a UiSurface.
 *
 * @param s Pointer to the UiSurface containing the window.
 * @param w The index of the window within the surface for which to set the background for new content.
 * @param cell Pointer to a UiCell structure representing the desired background attributes for new content.
 * @return 0 on success, or -1 on failure.
 */
int ui_bkgdset(UiSurface *s, ss_t w, const UiCell *cell) {
    if (!s)
        return -1;
    wbkgrndset(s->mwin[w], cell);
    return 0;
}
// for the entire window
/** @brief Set the background of a specific window within a UiSurface.
 *
 * @param s Pointer to the UiSurface containing the window.
 * @param w The index of the window within the surface for which to set the background.
 * @param cell Pointer to a UiCell structure representing the desired background attributes.
 * @return 0 on success, or -1 on failure.
 */
int ui_bkgrnd(UiSurface *s, ss_t w, const UiCell *cell) {
    if (!s->mwin[w])
        return -1;
    wbkgrnd(s->mwin[w], cell);
    return 0;
}
// for new content to be written to the window
/** @brief Set the background for new content to be written to a specific window within a UiSurface.
 *
 * @param s Pointer to the UiSurface containing the window.
 * @param w The index of the window within the surface for which to set the background for new content.
 * @param cell Pointer to a UiCell structure representing the desired background attributes for new content.
 * @return 0 on success, or -1 on failure.
 */
int ui_bkgrndset(UiSurface *s, ss_t w, const UiCell *cell) {
    if (!s->mwin[w])
        return -1;
    wbkgrndset(s->mwin[w], cell);
    return 0;
}
/* -------------------------------------------------------------------------
   Cell Manipulation
   ------------------------------------------------------------------------- */
/** @brief Get the character and attributes from a UiCell.
 *
 * @param cell Pointer to the UiCell from which to retrieve the character and attributes.
 * @param wstr Pointer to a wchar_t array where the retrieved character will be stored.
 * @param attrs Pointer to an attr_t variable where the retrieved attributes will be stored.
 * @param pair Pointer to a short variable where the retrieved color pair will be stored.
 * @param opts Pointer to additional options (not used in this implementation).
 * @return 0 on success, or -1 on failure.
 */
int ui_getcchar(const UiCell *cell, wchar_t *wstr, attr_t *attrs, short *pair, const void *opts) {
    (void)opts;
    getcchar(cell, wstr, attrs, pair, nullptr);
    return 0;
}
/** @brief Set the character and attributes of a UiCell.
 *
 * @param cell Pointer to the UiCell to be modified.
 * @param wstr Pointer to a wchar_t array containing the character to be set.
 * @param attrs The attributes to be applied to the cell.
 * @param pair The color pair to be applied to the cell.
 * @param opts Pointer to additional options (not used in this implementation).
 * @return 0 on success, or -1 on failure.
 */
int ui_setcchar(UiCell *cell, const wchar_t *wstr, const attr_t attrs, short pair, const void *opts) {
    (void)opts;
    return setcchar(cell, wstr, attrs, pair, NULL);
}
/* -------------------------------------------------------------------------
   Colors, Color Pairs
   ------------------------------------------------------------------------- */
/** @brief Add a new color pair to the UI, or retrieve the index of an existing pair.
 *
 * @param fg The foreground color index for the new color pair.
 * @param bg The background color index for the new color pair.
 * @return The index of the new or existing color pair, or EXIT_FAILURE on failure.
 */
uint ui_add_pair(uint fg, uint bg) {
    int rc;
    uint i;
    uint pfg, pbg;
    for (i = 1; i < ui_pair_cnt; i++) {
        ui_pair_content(i, &pfg, &pbg);
        if (pfg == fg && pbg == bg)
            return i;
    }
    if (i + 1 >= UI_PAIRS) {
        ssnprintf(em0, MAXLEN - 1, "%s, line: %d", __FILE__, __LINE__ - 1);
        ssnprintf(em1, MAXLEN - 1, "ui_add_pair failed for pair: %d", i);
        strerror_r(errno, em2, MAXLEN);
        ui_display_error(em0, em1, em2, nullptr);
        return (EXIT_FAILURE);
    }
    rc = init_extended_pair(i, fg, bg);
    if (rc == ERR) {
        ssnprintf(em0, MAXLEN - 1, "%s, line: %d", __FILE__, __LINE__ - 1);
        ssnprintf(em1, MAXLEN - 1, "init_extended_pair failed for pair: %d", i);
        ssnprintf(em2, MAXLEN - 1, "fg: %d, bg: %d, ui_pair_cnt: %d", fg, bg, ui_pair_cnt);
        ui_display_error(em0, em1, em2, nullptr);
        return (EXIT_FAILURE);
    }
    ui_pair_cnt++;
    return (int)(ui_pair_cnt - 1);
}
/** @brief Change the foreground and background colors of an existing color pair.
 *
 * @param pair The index of the color pair to be modified.
 * @param fg The new foreground color index for the color pair.
 * @param bg The new background color index for the color pair.
 * @return 0 on success, or -1 on failure.
 */
int ui_chg_pair(uint pair, uint fg, uint bg) {
    if (pair + 1 >= UI_PAIRS)
        return -1;
    init_extended_pair(pair, fg, bg);
    return 0;
}

/** was rgb_to_curses_clr
 * @brief Convert RGB to curses color index, adding new color if necessary.
 * @param rgb Pointer to RGB struct with r, g, b values (0-255).
 * @return Color index in curses color table, or 0 on failure.
 */
int ui_color_from_rgb(RGB *rgb) {
    if (rgb->r == rgb->g && rgb->g == rgb->b && rgb->b == 0)
        return 0;
    // #ifdef DEBUG_COLOR
    if (rgb->r > 255 || rgb->g > 255 || rgb->b > 255) {
        ssnprintf(em0, MAXLEN - 1, "%s, line: %d", __FILE__, __LINE__ - 1);
        ui_log(ERROR, "%s", em0);
        ssnprintf(em1, MAXLEN - 1, "ui_color_from_rgb failed for RGB: %d,%d,%d", rgb->r, rgb->g, rgb->b);
        ui_log(ERROR, "%s", em0);
        // return (EXIT_FAILURE);
    }
    // #endif
    uint i;
    RGB tmp;
    ui_apply_gamma(rgb);
    rgb->r = (rgb->r * 1000) / 255;
    rgb->g = (rgb->g * 1000) / 255;
    rgb->b = (rgb->b * 1000) / 255;
    for (i = 0; i < ui_color_cnt && i < UI_COLORS; i++) {
        extended_color_content(i, &tmp.r, &tmp.g, &tmp.b);
        if (rgb->r == tmp.r && rgb->g == tmp.g && rgb->b == tmp.b)
            return i;
    }
    if (i < UI_COLORS) {
        if (i < 16) {
            std_color[i].r = rgb->r;
            std_color[i].g = rgb->g;
            std_color[i].b = rgb->b;
        }
        init_extended_color(i, rgb->r, rgb->g, rgb->b);
        if (ui_color_cnt + 1 < UI_COLORS)
            ui_color_cnt++;
        return ui_color_cnt - 1;
    }
    return 0;
}
/** @brief Get the RGB color value for a given color index.
 *
 * @param color_idx The index of the color in the curses color table.
 * @return The RGB color value as a 32-bit unsigned integer (0xRRGGBB), or -1 on failure.
 */
uint32_t ui_get_color(uint16_t color_idx) {
    if (color_idx + 1 >= UI_COLORS)
        return -1;
    RGB rgb;
    extended_color_content(color_idx, &rgb.r, &rgb.g, &rgb.b);
    rgb.r = (rgb.r * 255) / 1000;
    rgb.g = (rgb.g * 255) / 1000;
    rgb.b = (rgb.b * 255) / 1000;
    return 0;
}
RGB ui_hex_to_rgb(char *s) {
    RGB rgb;
    sscanf(s, "#%02x%02x%02x", &rgb.r, &rgb.g, &rgb.b);
    return rgb;
}
int ui_color_content(uint color, uint *r, uint *g, uint *b) {
    int _color = (int)color;
    int _r, _g, _b;
    extended_color_content(_color, &_r, &_g, &_b);
    *r = (uint)_r;
    *g = (uint)_g;
    *b = (uint)_b;
    return 0;
}
/** @brief Initialize a color in the curses color table with specified RGB values.
 *
 * @param color The index of the color to be initialized.
 * @param r The red component of the color (0-255).
 * @param g The green component of the color (0-255).
 * @param b The blue component of the color (0-255).
 * @return 0 on success, or -1 on failure.
 */
int ui_init_color(uint color, uint r, uint g, uint b) {
    init_extended_color(color, r, g, b);
    return 0;
}
/** @brief Get the foreground and background colors of a specified color pair.
 *
 * @param pair The index of the color pair to be queried.
 * @param fg Pointer to a uint variable where the foreground color index will be stored.
 * @param bg Pointer to a uint variable where the background color index will be stored.
 * @return 0 on success, or -1 on failure.
 */
int ui_pair_content(uint pair, uint *fg, uint *bg) {
    int _pair = (int)pair;
    int _fg, _bg;
    extended_pair_content(_pair, &_fg, &_bg);
    *fg = (uint)_fg;
    *bg = (uint)_bg;
    return 0;
}
/** @brief Initialize a color pair in the curses color table with specified foreground and background colors.
 *
 * @param pair The index of the color pair to be initialized.
 * @param fg The foreground color index for the color pair.
 * @param bg The background color index for the color pair.
 * @return 0 on success, or -1 on failure.
 */
int ui_init_pair(uint pair, uint fg, uint bg) {
    init_extended_pair(pair, fg, bg);
    return 0;
}
/** @brief Change the RGB color value for a given color index.
 *
 * @param color_idx The index of the color in the curses color table to be changed.
 * @param color Pointer to a 32-bit unsigned integer representing the new RGB color value (0xRRGGBB).
 * @return 0 on success, or -1 on failure.
 */
int ui_chg_color(uint16_t color_idx, uint32_t *color) {
    if (color_idx + 1 >= UI_COLORS)
        return -1;
    RGB rgb;
    rgb.r = (*color >> 16) & 0xff;
    rgb.g = (*color >> 8) & 0xff;
    rgb.b = *color & 0xff;
    ui_apply_gamma(&rgb);
    if (color_idx < 16) {
        std_color[color_idx].r = rgb.r;
        std_color[color_idx].g = rgb.g;
        std_color[color_idx].b = rgb.b;
    }
    rgb.r = (rgb.r * 1000) / 255;
    rgb.g = (rgb.g * 1000) / 255;
    rgb.b = (rgb.b * 1000) / 255;
    init_extended_color(color_idx, rgb.r, rgb.g, rgb.b);
    return 0;
}
/* -------------------------------------------------------------------------
   Non-portable escape-hatch getters (see ui_ncurses_compat.h)
   ------------------------------------------------------------------------- */
/** @brief Get the current SCREEN pointer used by the UI.
 *
 * @return The SCREEN pointer if the UI is initialized, or NULL if the UI is not initialized.
 */
SCREEN *ui_ncurses_get_screen() {
    if (!ui)
        return NULL;
    return ui->screen;
}
/** @brief Get the current FILE pointer for the TTY used by the UI.
 *
 * @return The FILE pointer for the TTY if the UI is initialized, or NULL if the UI is not initialized.
 */
WINDOW *ui_ncurses_surface_get_win(const UiSurface *s, ss_t w) {
    if (!s)
        return NULL;
    return s->mwin[w];
}
/** @brief Get the current PANEL pointer for a specific window within a UiSurface.
 *
 * @param s Pointer to the UiSurface containing the window.
 * @param w The index of the window within the surface for which to retrieve the PANEL pointer.
 * @return The PANEL pointer for the specified window, or NULL if the UiSurface is NULL.
 */
PANEL *ui_ncurses_surface_get_panel(const UiSurface *s, ss_t w) {
    if (!s)
        return NULL;
    return s->mpan[w];
}
