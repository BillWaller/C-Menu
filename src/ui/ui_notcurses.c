/** @file ui_notcurses.c
   @ingroup ui_notcurses
   @brief NotCurses UI backend — lifecycle, surface management, and capabilities.

   Implements all UiRuntime and UiSurface operations declared in ui_backend.h
   using the NotCurses library.
*/

#include <iso646.h>
#define _XOPEN_SOURCE_EXTENDED 1

#include "cm.h"
#include "ui_backend.h"
#include "ui_notcurses_compat.h"
#include "ui_notcurses_internal.h"
#include <errno.h>
#include <inttypes.h>
#include <locale.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

UiRuntime *ui = NULL;
uint ui_color_cnt = 0;
uint ui_pair_cnt = 0;
UiPair *ui_pair;
UiColor *ui_color;
UiSurface *stdsfc;
uint LINES, COLS;
UiRuntime *ui;
UiConfig *ui_config;
UiSurface *ui_surface[UI_SFC_MAX];
UiCell bkgd_cell;

int sfc_ptr = -1;
NcPlane *stdplane;

/* -------------------------------------------------------------------------
   Backend identification and capability query
   ------------------------------------------------------------------------- */
/** @brief Get the current UI backend in use.
 * @return The current UI backend as an enum value of type UiBackend.
 *
 * This function returns the current UI backend being used by the application.
 * In this implementation, it always returns UI_BACKEND_NOTCURSES, indicating
 * that the NotCurses library is being used as the UI backend.
 */
UiBackend ui_get_backend() {
    return UI_BACKEND_NOTCURSES;
}
/** @brief Get the capabilities of the current UI backend.
 * @param caps Pointer to a UiCaps structure to be filled with the capabilities.
 *
 * This function populates the provided UiCaps structure with information about
 * the capabilities of the current UI backend. It sets various fields in the
 * structure, such as truecolor support, palette size, mouse support, Unicode
 * support, and resize capability. If the provided pointer is NULL or if the
 * UI runtime is not initialized, the function does nothing.
 */
void ui_get_caps(UiCaps *caps) {
    if (!caps)
        return;
    memset(caps, 0, sizeof(*caps));
    if (!ui)
        return;
    // I don't know that it isn't truecolor
    caps->truecolor = true;
    caps->palette256 = true;
    caps->mouse = ui->mouse_enabled;
    caps->unicode = true;
    caps->resize = true;
    caps->color_pairs = 0;
}
/* -------------------------------------------------------------------------
   Lifecycle
   ------------------------------------------------------------------------- */
/** @brief Initialize the UI runtime with the specified configuration and SIO.
 * @param cfg Pointer to a UiConfig structure containing configuration options.
 * @param sio Pointer to a SIO structure for input/output operations.
 * @return Pointer to the initialized UiRuntime structure, or NULL on failure.
 *
 * This function initializes the UI runtime using the NotCurses library. It sets
 * up the terminal, creates the standard plane, and initializes various UI
 * components based on the provided configuration. If any step fails, it cleans
 * up and returns NULL.
 */
UiRuntime *ui_init(const UiConfig *cfg, SIO *sio) {
    setlocale(LC_ALL, "en_US.UTF-8");
    ui = calloc(1, sizeof(*ui));
    if (!ui)
        return NULL;
    char tty_name[MAXLEN];
    if (ttyname_r(STDIN_FILENO, tty_name, sizeof(tty_name)) != 0) {
        free(ui);
        ui = NULL;
        return NULL;
    }
    ui->tty_fp = fopen(tty_name, "r+");
    if (ui->tty_fp == NULL) {
        ui = NULL;
        free(ui);
        return NULL;
    }
    NotCursesOptions nc_opts = {
        .flags = NCOPTION_SUPPRESS_BANNERS |
                 NCOPTION_NO_QUIT_SIGHANDLERS,
    };
    ui->nc = notcurses_init(&nc_opts, ui->tty_fp);
    if (ui->nc == NULL) {
        free(ui);
        ui = NULL;
        return NULL;
    }
    f_notcurses_open = true;
    stdplane = notcurses_stdplane(ui->nc);
    notcurses_render(ui->nc);

    if (ui->nc == NULL) {
        fclose(ui->tty_fp);
        ui->tty_fp = NULL;
        free(ui);
        ui = NULL;
        return NULL;
    }
    notcurses_mice_enable(ui->nc, NCMICE_ALL_EVENTS);
    notcurses_cursor_disable(ui->nc);
    notcurses_stddim_yx(ui->nc, &ui->lines, &ui->cols);
    stdsfc = calloc(1, sizeof(*stdsfc));
    if (!stdsfc)
        return NULL;
    for (int i = 0; i < SUB_SFC_MAX; i++) {
        stdsfc->mplane[i] = NULL;
        stdsfc->meta[i].lines = 0;
        stdsfc->meta[i].cols = 0;
        stdsfc->meta[i].y = 0;
        stdsfc->meta[i].x = 0;
        stdsfc->meta[i].hidden = false;
    }
    stdsfc->runtime = ui;
    stdsfc->parent = NULL;
    stdsfc->meta[BOX].lines = ui->lines;
    stdsfc->meta[BOX].cols = ui->cols;
    stdsfc->meta[BOX].y = 0;
    stdsfc->meta[BOX].x = 0;
    stdsfc->mplane[BOX] = stdplane;
    if (!stdsfc->mplane[BOX]) {
        notcurses_stop(ui->nc);
        f_notcurses_open = false;
        return NULL;
    }
    sfc_ptr = -1;
    LINES = ui->lines;
    COLS = ui->cols;
    ui_pair = calloc(UI_PAIRS, sizeof(UiPair));
    if (!ui_pair) {
        free(ui_pair);
        notcurses_stop(ui->nc);
        f_notcurses_open = false;
        free(ui);
        return NULL;
    }
    ui_pair_cnt = 0;
    ui_color = calloc(UI_COLORS, sizeof(UiColor));
    if (!ui_color) {
        free(ui_pair);
        notcurses_stop(ui->nc);
        f_notcurses_open = false;
        free(ui);
        return NULL;
    }
    ui_color_cnt = 0;
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
    stdsfc = calloc(1, sizeof(*stdsfc));
    return ui;
}

/* -------------------------------------------------------------------------
   Surface Creation and Destruction
   ------------------------------------------------------------------------- */
/** @brief Create a new UI surface with the specified parameters.
 * @param w The index of the surface to create.
 * @param parent Pointer to the parent UiSurface, or NULL for no parent.
 * @param p The index of the parent plane to attach to.
 * @param lines The number of lines (rows) for the new surface.
 * @param cols The number of columns for the new surface.
 * @param y The y-coordinate for the new surface's position.
 * @param x The x-coordinate for the new surface's position.
 * @return Pointer to the newly created UiSurface, or NULL on failure.
 *
 * This function creates a new UI surface and initializes its properties based
 * on the provided parameters. It allocates memory for the surface, sets up its
 * metadata, and creates a new ncplane for rendering. If any step fails, it
 * cleans up and returns NULL.
 */
UiSurface *ui_surface_new(ss_t w, UiSurface *parent, uint p, uint lines, uint cols, uint y, uint x) {
    if (!ui)
        return NULL;
    UiSurface *s = calloc(1, sizeof(*s));
    if (!s) {
        ui_log(ERROR, "Failed to allocate memory for UiSurface\n");
        return NULL;
    }
    ui_log(INFO, "Memory allocated for new surface");
    s->runtime = ui;
    s->parent = parent;
    ncplane_options plane_opts = {
        .y = y,
        .x = x,
        .rows = lines,
        .cols = cols};
    s->meta[w].y = y;
    s->meta[w].x = x;
    s->meta[w].lines = lines;
    s->meta[w].cols = cols;
    s->meta[w].hidden = false;
    if (parent && parent->mplane[p]) {
        s->mplane[w] = ncplane_create(parent->mplane[p], &plane_opts);
        if (!s->mplane[w]) {
            ui_log(ERROR, "Failed to create mplane: %d", w);
        }
    } else {
        stdplane = notcurses_stdplane(ui->nc);
        s->mplane[w] = ncplane_create(stdplane, &plane_opts);
        if (!s->mplane[w]) {
            ui_log(ERROR, "Failed to create stdplane");
        }
    }
    ui_log(ERROR, "mplane[%d] created", w);
    if (!s->mplane[w]) {
        notcurses_stop(ui->nc);
        f_notcurses_open = false;
        return NULL;
    }
    return s;
}
/** @brief Create a new UI surface with a border box and optional title.
 * @param parent Pointer to the parent UiSurface, or NULL for no parent.
 * @param p The index of the parent plane to attach to.
 * @param lines The number of lines (rows) for the new surface.
 * @param cols The number of columns for the new surface.
 * @param y The y-coordinate for the new surface's position.
 * @param x The x-coordinate for the new surface's position.
 * @param wtitle The title to display on the border box, or NULL for no title.
 * @return Pointer to the newly created UiSurface, or NULL on failure.
 *
 * This function creates a new UI surface with a border box and an optional
 * title. It allocates memory for the surface, sets up its metadata, and creates
 * a new ncplane for rendering. If any step fails, it cleans up and returns NULL.
 */
UiSurface *ui_surface_box(UiSurface *parent, uint p, uint lines, uint cols, uint y, uint x, const char *wtitle) {
    if (!ui)
        return NULL;
    UiSurface *s = calloc(1, sizeof(*s));
    if (!s)
        return NULL;
    s->runtime = ui;
    s->parent = parent;
    ncplane_options plane_opts = {
        .y = y,
        .x = x,
        .rows = lines,
        .cols = cols,
        .name = NULL};
    s->meta[BOX].y = y;
    s->meta[BOX].x = x;
    s->meta[BOX].lines = lines + 2;
    s->meta[BOX].cols = cols + 2;
    s->meta[BOX].hidden = false;
    strnz__cpy(s->meta[BOX].name, "BOX", XLEN - 1);
    if (parent && parent->mplane[p]) {
        s->mplane[BOX] = ncplane_create(parent->mplane[p], &plane_opts);
    } else {
        stdplane = notcurses_stdplane(ui->nc);
        s->mplane[BOX] = ncplane_create(stdplane, &plane_opts);
    }
    if (!s->mplane[BOX]) {
        free(s);
        return NULL;
    }
    ncplane_set_base(s->mplane[BOX],
                     " ",
                     0,
                     cell_box.channels);
    ncplane_set_channels(s->mplane[BOX], cell_box.channels);
    ui_border_draw(s);
    ui_border_title(s, wtitle);
    return s;
}
/** @brief Add padding to an existing UI surface.
 * @param s Pointer to the UiSurface to which padding will be added.
 * @param w The index of the new padded surface.
 * @param p The index of the parent plane to attach to.
 * @param lines The number of lines (rows) for the padded surface.
 * @param cols The number of columns for the padded surface.
 * @param y The y-coordinate for the padded surface's position.
 * @param x The x-coordinate for the padded surface's position.
 * @return 0 on success, -1 on failure.
 *
 * This function adds padding to an existing UI surface by creating a new
 * ncplane with the specified dimensions and position. It updates the metadata
 * for the new padded surface and sets its background and scrolling properties.
 * If any step fails, it cleans up and returns -1.
 */
int ui_surface_addpad(UiSurface *s, ss_t w, uint p, uint lines, uint cols, uint y, uint x) {
    uint plines, pcols;
    ncplane_dim_yx(s->mplane[p], &plines, &pcols);
    pcols = min(pcols, cols);
    plines = min(plines, lines);
    ncplane_options plane_opts = {
        .y = y,
        .x = x,
        .rows = plines - y,
        .cols = pcols - x,
        .name = NULL};
    s->meta[w].y = y;
    s->meta[w].x = x;
    s->meta[w].lines = plines - y;
    s->meta[w].cols = pcols - x;
    s->meta[w].hidden = false;
    s->mplane[w] = ncplane_create(s->mplane[p], &plane_opts);
    if (!s->mplane[w]) {
        ui_log(ERROR, "failed to create mplane[%d]", w);
        notcurses_stop(ui->nc);
        f_notcurses_open = false;
        return -1;
    }
    ncplane_set_base(s->mplane[w], " ", 0, cell_nt.channels);
    ncplane_set_channels(s->mplane[w], cell_nt.channels);
    ui_bkgd(s, w, &cell_nt);
    ui_bkgdset(s, w, &cell_nt);
    ui_scrollok(s, w, true);
    return 0;
}
/** @brief Add a new window to an existing UI surface.
 * @param s Pointer to the UiSurface to which the window will be added.
 * @param w The index of the new window.
 * @param p The index of the parent plane to attach to.
 * @param lines The number of lines (rows) for the new window.
 * @param cols The number of columns for the new window.
 * @param y The y-coordinate for the new window's position.
 * @param x The x-coordinate for the new window's position.
 * @return 0 on success, -1 on failure.
 *
 * This function adds a new window to an existing UI surface by creating a new
 * ncplane with the specified dimensions and position. It updates the metadata
 * for the new window and sets its background properties. If any step fails, it
 * cleans up and returns -1.
 */
int ui_surface_addwin(UiSurface *s, ss_t w, uint p, uint lines, uint cols, uint y, uint x) {
    ncplane_options plane_opts = {
        .y = y,
        .x = x,
        .rows = lines,
        .cols = cols,
        .name = NULL};
    s->meta[w].y = y;
    s->meta[w].x = x;
    s->meta[w].lines = lines;
    s->meta[w].cols = cols;
    s->meta[w].hidden = false;
    s->mplane[w] = ncplane_create(s->mplane[p], &plane_opts);
    if (!s->mplane[w]) {
        ui_log(ERROR, "failed to create mplane[%d]", w);
        notcurses_stop(ui->nc);
        f_notcurses_open = false;
        return -1;
    }
    ncplane_set_base(s->mplane[w], " ", 0, cell_nt.channels);
    ncplane_set_channels(s->mplane[w], cell_nt.channels);
    ui_bkgd(s, w, &cell_nt);
    ui_bkgdset(s, w, &cell_nt);
    return 0;
}
/** @brief Destroy a UI surface and free its resources.
 * @param s Pointer to the UiSurface to be destroyed.
 *
 * This function destroys a UI surface by freeing its associated ncplanes and
 * metadata. It also frees the memory allocated for the UiSurface structure.
 * If the provided pointer is NULL, the function does nothing.
 */
void ui_endwin() {
    ui_shutdown();
}
/** @brief Shutdown the UI runtime and free all associated resources.
 *
 * This function shuts down the UI runtime by destroying all surfaces, freeing
 * memory, and stopping the NotCurses library. It also closes the terminal file
 * pointer if it was opened. If the UI runtime is not initialized, the function
 * does nothing.
 */
void ui_shutdown() {
    if (ui == NULL)
        return;
    while (sfc_ptr >= 0) {
        if (ui_surface[sfc_ptr]) {
            ui_log(INFO, "Destroying surface: %d", sfc_ptr);
            ui_surface_destroy(ui_surface[sfc_ptr]);
            ui_surface[sfc_ptr] = NULL;
        }
        sfc_ptr--;
    }
    if (stdsfc->mplane[BOX]) {
        ui_log(INFO, "Destroying mplane[%d]", BOX);
        ncplane_erase(stdsfc->mplane[BOX]);
        ncplane_destroy(stdsfc->mplane[BOX]);
        stdsfc->mplane[BOX] = NULL;
    }
    if (ui->tty_fp) {
        // fclose(ui->tty_fp);
        // ui->tty_fp = NULL;
    }
    if (stdsfc != NULL) {
        ui_log(INFO, "Destroying stdsfc");
        free(stdsfc);
        stdsfc = NULL;
    }
    ui_log(INFO, "Calling notcurses_stop");
    notcurses_stop(ui->nc);
    f_notcurses_open = false;
    if (ui != NULL) {
        if (ui_pair != NULL) {
            ui_log(INFO, "Destroying ui_pair");
            free(ui_pair);
            ui_pair = NULL;
        }
        if (ui_color != NULL) {
            ui_log(INFO, "Destroying ui_color");
            free(ui_color);
            ui_color = NULL;
        }
        if (ui->sio) {
            ui_log(INFO, "Destroying ui->sio");
            free(ui->sio);
            ui->sio = nullptr;
        }
        if (ui != NULL) {
            ui_log(INFO, "Destroying ui");
            free(ui);
            ui = NULL;
        }
    }
}
/** @brief Destroy a UI surface and free its resources.
 * @param s Pointer to the UiSurface to be destroyed.
 *
 * This function destroys a UI surface by freeing its associated ncplanes and
 * metadata. It also frees the memory allocated for the UiSurface structure.
 * If the provided pointer is NULL, the function does nothing.
 */
void ui_surface_destroy(UiSurface *s) {
    if (!s)
        return;
    ss_t w = SUB_SFC_MAX;
    while (1) {
        if (s->mplane[w] != NULL) {
            ui_log(INFO, "Destroying mplane[%d]", w);
            ncplane_erase(s->mplane[w]);
            ncplane_destroy(s->mplane[w]);
            s->mplane[w] = NULL;
        }
        if (w == BOX)
            break;
        w--;
    };
    if (s != NULL) {
        ui_log(INFO, "Destroying surface: s");
        free(s);
        s = NULL;
    }
}
/* -------------------------------------------------------------------------
   Surface Navigation and Management
   ------------------------------------------------------------------------- */
/** @brief Move a UI surface to a new position on the screen.
 * @param s Pointer to the UiSurface to be moved.
 * @param w The index of the window within the surface to move.
 * @param y The new y-coordinate for the surface's position.
 * @param x The new x-coordinate for the surface's position.
 * @return 0 on success, -1 on failure.
 *
 * This function moves a UI surface to a new position on the screen by updating
 * its metadata and moving the associated ncplane. If the surface is hidden, it
 * only updates the metadata without moving the ncplane. If any step fails, it
 * returns -1.
 */
int ui_surface_move(UiSurface *s, ss_t w, uint y, uint x) {
    if (!s)
        return -1;
    s->meta[w].y = y;
    s->meta[w].x = x;
    if (!s->meta[w].hidden)
        return ncplane_move_yx(s->mplane[w], y, x) == 0 ? 0 : -1;
    return 0;
}
/** @brief Resize a UI surface to new dimensions.
 * @param s Pointer to the UiSurface to be resized.
 * @param w The index of the window within the surface to resize.
 * @param lines The new number of lines (rows) for the surface.
 * @param cols The new number of columns for the surface.
 * @return 0 on success, -1 on failure.
 *
 * This function resizes a UI surface to new dimensions by updating its metadata
 * and resizing the associated ncplane. If any step fails, it returns -1.
 */
int ui_surface_resize(UiSurface *s, ss_t w, uint lines, uint cols) {
    if (!s)
        return -1;
    return ncplane_resize_simple(s->mplane[w], (unsigned int)lines,
                                 (unsigned int)cols) == 0
               ? 0
               : -1;
}
/** @brief Clear the contents of the standard plane.
 * @return 0 on success, -1 on failure.
 *
 * This function clears the contents of the standard plane by erasing its
 * contents. If the standard plane is not initialized, it returns -1.
 */
int ui_clear() {
    if (!stdplane)
        return -1;
    ncplane_erase_region(stdplane, 0, 0, 0, 0);
    return 0;
}
/** @brief Clear the contents of the specified window within a UI surface.
 * @param s Pointer to the UiSurface containing the window to clear.
 * @param w The index of the window to clear.
 * @return 0 on success, -1 on failure.
 *
 * This function clears the contents of the specified window within a UI surface
 * by erasing its contents. If the surface or window is not initialized, it
 * returns -1.
 */
int ui_erase() {
    if (!stdplane)
        return -1;
    ncplane_erase_region(stdplane, 0, 0, 0, 0);
    return 0;
}
/** @brief Clear the contents of the specified window within a UI surface.
 * @param s Pointer to the UiSurface containing the window to clear.
 * @param w The index of the window to clear.
 * @return 0 on success, -1 on failure.
 *
 * This function clears the contents of the specified window within a UI surface
 * by erasing its contents. If the surface or window is not initialized, it
 * returns -1.
 */
int ui_werase(UiSurface *s, ss_t w) {
    if (!s)
        return -1;
    ncplane_erase_region(s->mplane[w], 0, 0, 0, 0);
    return 0;
}
/** @brief Clear the contents of the specified window within a UI surface.
 * @param s Pointer to the UiSurface containing the window to clear.
 * @param w The index of the window to clear.
 * @return 0 on success, -1 on failure.
 *
 * This function clears the contents of the specified window within a UI surface
 * by erasing its contents. If the surface or window is not initialized, it
 * returns -1.
 */
int ui_wclear(UiSurface *s, ss_t w) {
    if (!s)
        return -1;
    ncplane_erase_region(s->mplane[w], 0, 0, 0, 0);
    return 0;
}
/** @brief Move the specified window within a UI surface to the top of the z-order.
 * @param s Pointer to the UiSurface containing the window to move.
 * @param w The index of the window to move to the top.
 * @return 0 on success, -1 on failure.
 *
 * This function moves the specified window within a UI surface to the top of
 * the z-order, making it the foremost window. If the surface or window is not
 * initialized, it returns -1.
 */
int ui_top_surface(UiSurface *s, ss_t w) {
    if (!s)
        return -1;
    ncplane_move_top(s->mplane[w]);
    return 0;
}
/** @brief Show the specified window within a UI surface.
 * @param s Pointer to the UiSurface containing the window to show.
 * @param w The index of the window to show.
 * @return 0 on success, -1 on failure.
 *
 * This function shows the specified window within a UI surface by updating its
 * metadata and moving it to its designated position. If the surface or window
 * is not initialized, it returns -1.
 */
int ui_surface_show(UiSurface *s, ss_t w) {
    if (!s)
        return -1;
    if (s->meta[w].hidden) {
        s->meta[w].hidden = false;
        ncplane_move_yx(s->mplane[w], s->meta[w].y, s->meta[w].x);
    }
    return 0;
}
/** @brief Hide the specified window within a UI surface.
 * @param s Pointer to the UiSurface containing the window to hide.
 * @param w The index of the window to hide.
 * @return 0 on success, -1 on failure.
 *
 * This function hides the specified window within a UI surface by updating its
 * metadata and moving it far off-screen. If the surface or window is not
 * initialized, it returns -1.
 */
int ui_surface_hide(UiSurface *s, ss_t w) {
    if (!s)
        return -1;
    if (!s->meta[w].hidden) {
        s->meta[w].hidden = true;
        /* Move far off-screen so the plane does not obscure anything. */
        ncplane_move_yx(s->mplane[w], -s->meta[w].lines - 1, 0);
    }
    return 0;
}
// -------------------------------------------------------------------------
// Screen Navigation
// -------------------------------------------------------------------------
/** @brief Move the cursor to the specified position within a window of a UI surface.
 * @param s Pointer to the UiSurface containing the window.
 * @param w The index of the window within the surface.
 * @param y The y-coordinate for the new cursor position.
 * @param x The x-coordinate for the new cursor position.
 * @return 0 on success, -1 on failure.
 *
 * This function moves the cursor to the specified position within a window of
 * a UI surface. If the surface or window is not initialized, it returns -1.
 */
int ui_wmove(UiSurface *s, ss_t w, uint y, uint x) {
    if (!s)
        return -1;
    if (ncplane_cursor_move_yx(s->mplane[w], y, x) != 0)
        return -1;
    return 0;
}
/** @brief Move the cursor to the specified position within a window of a UI surface.
 * @param s Pointer to the UiSurface containing the window.
 * @param w The index of the window within the surface.
 * @param y The y-coordinate for the new cursor position.
 * @param x The x-coordinate for the new cursor position.
 * @return 0 on success, -1 on failure.
 *
 * This function moves the cursor to the specified position within a window of
 * a UI surface. If the surface or window is not initialized, it returns -1.
 */
int ui_cursor_move(UiSurface *s, ss_t w, uint y, uint x) {
    if (!s)
        return -1;
    if (ncplane_cursor_move_yx(s->mplane[w], y, x) != 0)
        return -1;
    return 0;
}
/** @brief Get the current cursor position within a window of a UI surface.
 * @param s Pointer to the UiSurface containing the window.
 * @param w The index of the window within the surface.
 * @param y Pointer to an unsigned integer to store the y-coordinate of the cursor.
 * @param x Pointer to an unsigned integer to store the x-coordinate of the cursor.
 *
 * This function retrieves the current cursor position within a window of a UI
 * surface. If the surface or window is not initialized, it does nothing.
 */
void ui_cursor_yx(int *y, int *x) {
    notcurses_cursor_yx(ui->nc, y, x);
}
/** @brief Get the absolute cursor position within a window of a UI surface.
 * @param s Pointer to the UiSurface containing the window.
 * @param w The index of the window within the surface.
 * @param y Pointer to an integer to store the absolute y-coordinate of the cursor.
 * @param x Pointer to an integer to store the absolute x-coordinate of the cursor.
 *
 * This function retrieves the absolute cursor position within a window of a UI
 * surface. If the surface or window is not initialized, it does nothing.
 */
void ui_abs_yx(UiSurface *s, ss_t w, int *y, int *x) {
    if (!s)
        return;
    ncplane_abs_yx(s->mplane[w], y, x);
}
/** @brief Get the current cursor position within a window of a UI surface.
 * @param s Pointer to the UiSurface containing the window.
 * @param w The index of the window within the surface.
 * @param y Pointer to an unsigned integer to store the y-coordinate of the cursor.
 * @param x Pointer to an unsigned integer to store the x-coordinate of the cursor.
 *
 * This function retrieves the current cursor position within a window of a UI
 * surface. If the surface or window is not initialized, it does nothing.
 */
void ui_getyx(UiSurface *s, ss_t w, uint *y, uint *x) {
    if (!s)
        return;
    ncplane_cursor_yx(s->mplane[w], y, x);
}
/** @brief Get the maximum dimensions of a window within a UI surface.
 * @param s Pointer to the UiSurface containing the window.
 * @param w The index of the window within the surface.
 * @param y Pointer to an unsigned integer to store the maximum number of lines (rows).
 * @param x Pointer to an unsigned integer to store the maximum number of columns.
 *
 * This function retrieves the maximum dimensions of a window within a UI
 * surface. If the surface or window is not initialized, it does nothing.
 */
void ui_getmaxyx(UiSurface *s, ss_t w, uint *y, uint *x) {
    if (!s)
        return;
    ncplane_dim_yx(s->mplane[w], y, x);
}
/** @brief Get the maximum number of lines (rows) of a window within a UI surface.
 * @param s Pointer to the UiSurface containing the window.
 * @param w The index of the window within the surface.
 * @return The maximum number of lines (rows) of the window, or -1 on failure.
 *
 * This function retrieves the maximum number of lines (rows) of a window
 * within a UI surface. If the surface or window is not initialized, it returns
 * -1.
 */
int ui_getmaxy(UiSurface *s, ss_t w) {
    if (!s)
        return -1;
    uint y, x;
    ncplane_dim_yx(s->mplane[w], &y, &x);
    return y;
}
/** @brief Get the maximum number of columns of a window within a UI surface.
 * @param s Pointer to the UiSurface containing the window.
 * @param w The index of the window within the surface.
 * @return The maximum number of columns of the window, or -1 on failure.
 *
 * This function retrieves the maximum number of columns of a window within a
 * UI surface. If the surface or window is not initialized, it returns -1.
 */
int ui_getmaxx(UiSurface *s, ss_t w) {
    if (!s)
        return -1;
    uint y, x;
    ncplane_dim_yx(s->mplane[w], &y, &x);
    return x;
}
/** @brief Scroll the contents of the specified window up or down by the specified
 * number of lines.
 * @param s The surface containing the window to scroll.
 * @param w The index of the window to scroll.
 * @param r The number of lines to scroll. Positive values scroll up, negative
 * values scroll down.
 * @return 0 on success, -1 on error.
 *
 * If r is positive, the contents of the plane are moved up, leaving blank lines
 * at the bottom. In the view application, the view scope is moved toward end of
 * file (eof). - scope_toward_eof()
 *
 * @note ui_wscrl(-n) - If r is negative, the contents of the plane are moved down,
 * leaving blank lines at the top. In the view application, the view scope is
 * moved toward beginning of file (bof). - scope_toward_bof()
 */
int ui_wscrl(UiSurface *s, ss_t w, int r) {
    uint rows, cols;
    ncplane_dim_yx(s->mplane[w], &rows, &cols);

    if (r > 0) {
        // Scroll up (toward EOF): notcurses has this natively
        ncplane_scrollup(s->mplane[w], r);
        return 0;
    }

    // Scroll down (toward BOF): shift content down by |r| rows
    r = -r;
    if ((uint)r >= rows) {
        ncplane_erase(s->mplane[w]);
        return 0;
    }

    // Copy rows bottom-up to avoid overwriting source before reading
    for (int src_row = (int)(rows - r) - 1; src_row >= 0; src_row--) {
        int dst_row = src_row + r;
        for (uint col = 0; col < cols; col++) {
            nccell c = NCCELL_TRIVIAL_INITIALIZER;
            ncplane_at_yx_cell(s->mplane[w], src_row, col, &c);
            ncplane_putc_yx(s->mplane[w], dst_row, col, &c);
            nccell_release(s->mplane[w], &c);
        }
    }
    // Blank the top r rows
    for (int row = 0; row < r; row++) {
        for (uint col = 0; col < cols; col++) {
            ncplane_putchar_yx(s->mplane[w], row, col, ' ');
        }
    }
    return 0;
}

/* -------------------------------------------------------------------------
   Configuration Control
   ------------------------------------------------------------------------- */
/** @brief Enable or disable scrolling for the specified window within a UI surface.
 * @param s Pointer to the UiSurface containing the window.
 * @param w The index of the window within the surface.
 * @param enable True to enable scrolling, false to disable scrolling.
 * @return 0 on success, -1 on failure.
 *
 * This function enables or disables scrolling for the specified window within
 * a UI surface. If the surface or window is not initialized, it returns -1.
 */
int ui_scrollok(UiSurface *s, ss_t w, bool enable) {
    if (!s)
        return -1;
    ncplane_set_scrolling(s->mplane[w], enable);
    return 0;
}
/** @brief Enable or disable the keypad for the specified window within a UI surface.
 * @param s Pointer to the UiSurface containing the window.
 * @param w The index of the window within the surface.
 * @param enable True to enable the keypad, false to disable the keypad.
 * @return 0 on success, -1 on failure.
 *
 * This function does nothing.
 */
int ui_idcok(UiSurface *s, ss_t w, bool enable) {
    (void)s;
    (void)w;
    (void)enable;
    return 0;
}
/** @brief Enable or disable the use of the keypad for the specified window within a UI surface.
 * @param s Pointer to the UiSurface containing the window.
 * @param w The index of the window within the surface.
 * @param enable True to enable the use of the keypad, false to disable it.
 * @return 0 on success, -1 on failure.
 *
 * This function does nothing.
 */
int ui_idlok(UiSurface *s, ss_t w, bool enable) {
    (void)s;
    (void)w;
    (void)enable;
    return 0;
}
/** @brief Set the scrolling region for the specified window within a UI surface.
 * @param s Pointer to the UiSurface containing the window.
 * @param w The index of the window within the surface.
 * @param top The top line of the scrolling region.
 * @param bottom The bottom line of the scrolling region.
 * @return 0 on success, -1 on failure.
 *
 * This function does nothing.
 */
int ui_setscrreg(UiSurface *s, ss_t w, uint top, uint bottom) {
    (void)s;
    (void)w;
    (void)top;
    (void)bottom;
    return 0;
}
/** @brief Enable or disable the keypad for the specified window within a UI surface.
 * @param s Pointer to the UiSurface containing the window.
 * @param w The index of the window within the surface.
 * @param enable True to enable the keypad, false to disable it.
 * @return 0 on success, -1 on failure.
 *
 * This function does nothing.
 */
int ui_keypad(UiSurface *s, ss_t w, bool enable) {
    (void)s;
    (void)w;
    (void)enable;
    return 0;
}
/* -------------------------------------------------------------------------
   Screen management functions
   ------------------------------------------------------------------------- */
/** @brief Get the current screen size in lines and columns.
 * @param lines Pointer to an unsigned integer to store the number of lines (rows).
 * @param cols Pointer to an unsigned integer to store the number of columns.
 *
 * This function retrieves the current screen size in lines and columns.
 */
void ui_get_screen_size(uint *lines, uint *cols) {
    if (!ui)
        return;
    unsigned int r = 0, c = 0;
    notcurses_stddim_yx(ui->nc, &r, &c);
    ui->lines = (int)r;
    ui->cols = (int)c;
    if (lines)
        *lines = ui->lines;
    if (cols)
        *cols = ui->cols;
}
/** @brief Update the panels and render the UI.
 *
 * This function updates the panels and renders the UI.
 */
void ui_update_panels() {
    if (!ui)
        return;
    notcurses_render(ui->nc);
}
/** @brief Render the UI.
 *
 * This function renders the UI.
 */
void ui_render() {
    if (!ui)
        return;
    notcurses_render(ui->nc);
}
/** @brief Suspend the UI and leave the alternate screen.
 *
 * This function suspends the UI and leaves the alternate screen.
 * It is typically used when the application needs to temporarily exit
 * the UI mode, such as when executing a shell command or displaying
 * a message outside of the UI context.
 *
 * @return 0 on success, -1 on failure.
 */
int ui_suspend() {
    if (!ui)
        return -1;
    notcurses_leave_alternate_screen(ui->nc);
    return 0;
}
/** @brief Resume the UI and enter the alternate screen.
 *
 * This function resumes the UI and enters the alternate screen.
 * It is typically used after a suspension of the UI, allowing
 * the application to return to its previous state and continue
 * rendering the UI.
 *
 * @return 0 on success, -1 on failure.
 */
int ui_resume() {
    if (!ui)
        return -1;
    notcurses_enter_alternate_screen(ui->nc);
    notcurses_render(ui->nc);
    return 0;
}
// -------------------------------------------------------------------------
// Cursor Control
// -------------------------------------------------------------------------
/** @brief Enable or disable the cursor visibility.
 * @param visible True to enable the cursor, false to disable it.
 * @return 0 on success, -1 on failure.
 *
 * This function enables or disables the cursor visibility. If the UI is not
 * initialized, it returns -1. When enabling the cursor, it retrieves the
 * current cursor position and enables the cursor at that position.
 */
int ui_curs_set(int visible) {
    if (!ui)
        return -1;
    if (visible == 0) {
        ui->cursor_visible = false;
        notcurses_cursor_disable(ui->nc);
    } else {
        ui->cursor_visible = true;
        int y, x;
        notcurses_cursor_yx(ui->nc, &y, &x);
        int rc = notcurses_cursor_enable(ui->nc, y, x);
        return rc;
    }
    return 0;
}
/** @brief Enable or disable the cursor at a specific position on the surface and plane specified.
 * @param s Pointer to the UiSurface containing the window.
 * @param w The index of the window within the surface.
 * @param y The y-coordinate for the cursor position.
 * @param x The x-coordinate for the cursor position.
 * @param visible True to enable the cursor, false to disable it.
 * @return 0 on success, -1 on failure.
 *
 * This function enables or disables the cursor at a specific position on the
 * surface and plane specified. If the UI is not initialized, it returns -1.
 */
int ui_cursor_enable_yx(UiSurface *s, ss_t w, uint y, uint x, bool visible) {
    if (!s)
        return -1;
    if (!visible) {
        ui->cursor_visible = false;
        notcurses_cursor_disable(ui->nc);
    } else {
        int yy, xx;
        ncplane_abs_yx(s->mplane[w], &yy, &xx);
        y += yy;
        x += xx;
        ui->cursor_visible = true;
        int rc = notcurses_cursor_enable(ui->nc, y, x);
        return rc;
    }
    return 0;
}
/** @brief Enable or disable the cursor at the current position on the surface and plane specified.
 * @param s Pointer to the UiSurface containing the window.
 * @param w The index of the window within the surface.
 * @param visible True to enable the cursor, false to disable it.
 * @return 0 on success, -1 on failure.
 *
 * This function enables or disables the cursor at the current position on the
 * surface and plane specified. If the UI is not initialized, it returns -1.
 */
int ui_cursor_enable(UiSurface *s, ss_t w, bool visible) {
    if (!s)
        return -1;
    if (!visible) {
        ui->cursor_visible = false;
        notcurses_cursor_disable(ui->nc);
    } else {
        uint y, x;
        ncplane_cursor_yx(s->mplane[w], &y, &x);
        int yy, xx;
        ncplane_abs_yx(s->mplane[w], &yy, &xx);
        y += yy;
        x += xx;
        ui->cursor_visible = true;
        return notcurses_cursor_enable(ui->nc, y, x) == 0 ? 0 : -1;
    }
    return 0;
}
/* -------------------------------------------------------------------------
   background
   ------------------------------------------------------------------------- */
/** @brief Set the background cell for the specified window within a UI surface.
 * @param s Pointer to the UiSurface containing the window.
 * @param w The index of the window within the surface.
 * @param cell Pointer to the UiCell representing the background cell.
 * @return 0 on success, -1 on failure.
 *
 * This function sets the background cell for the specified window within a UI
 * surface. It updates the styles and channels of the ncplane associated with
 * the window and stores the background cell in the metadata. If the surface is
 * not initialized, it returns -1.
 */
int ui_bkgd(UiSurface *s, ss_t w, const UiCell *cell) {
    if (!s)
        return -1;
    ncplane_set_styles(s->mplane[w], cell->stylemask);
    ncplane_set_channels(s->mplane[w], cell->channels);
    ncplane_set_base(s->mplane[w], " ",
                     cell->stylemask, cell->channels);
    s->meta[w].bkgd_cell = *cell;
    return 0;
}
/** @brief Set the background cell for the specified window within a UI surface without changing the base character.
 * @param s Pointer to the UiSurface containing the window.
 * @param w The index of the window within the surface.
 * @param cell Pointer to the UiCell representing the background cell.
 * @return 0 on success, -1 on failure.
 *
 * This function sets the background cell for the specified window within a UI
 * surface without changing the base character. It updates the styles and
 * channels of the ncplane associated with the window and stores the background
 * cell in the metadata. If the surface is not initialized, it returns -1.
 */
int ui_bkgdset(UiSurface *s, ss_t w, const UiCell *cell) {
    if (!s)
        return -1;
    ncplane_set_styles(s->mplane[w], cell->stylemask);
    ncplane_set_channels(s->mplane[w], cell->channels);
    s->meta[w].bkgd_cell = *cell;
    // ncplane_set_base(s->mplane[w], " ",
    //                  cell->stylemask, cell->channels);
    return 0;
}
/** @brief Set the background cell for the specified window within a UI surface without changing the base character.
 * @param s Pointer to the UiSurface containing the window.
 * @param w The index of the window within the surface.
 * @param cell Pointer to the UiCell representing the background cell.
 * @return 0 on success, -1 on failure.
 *
 * This function sets the background cell for the specified window within a UI
 * surface without changing the base character. It updates the styles and
 * channels of the ncplane associated with the window and stores the background
 * cell in the metadata. If the surface is not initialized, it returns -1.
 */
int ui_bkgrnd(UiSurface *s, ss_t w, const UiCell *cell) {
    if (!s)
        return -1;
    ncplane_set_styles(s->mplane[w], cell->stylemask);
    ncplane_set_channels(s->mplane[w], cell->channels);
    ncplane_set_base(s->mplane[w], " ",
                     cell->stylemask, cell->channels);
    return 0;
}
/** @brief Set the background cell for the specified window within a UI surface without changing the base character.
 * @param s Pointer to the UiSurface containing the window.
 * @param w The index of the window within the surface.
 * @param cell Pointer to the UiCell representing the background cell.
 * @return 0 on success, -1 on failure.
 *
 * This function sets the background cell for the specified window within a UI
 * surface without changing the base character. It updates the styles and
 * channels of the ncplane associated with the window and stores the background
 * cell in the metadata. If the surface is not initialized, it returns -1.
 */
int ui_bkgrndset(UiSurface *s, ss_t w, const UiCell *cell) {
    if (!s)
        return -1;
    ncplane_set_styles(s->mplane[w], cell->stylemask);
    ncplane_set_channels(s->mplane[w], cell->channels);
    ncplane_set_base(s->mplane[w], " ",
                     cell->stylemask, cell->channels);
    return 0;
}
/* -------------------------------------------------------------------------
   Cell Manipulation
   ------------------------------------------------------------------------- */
/** @brief Get the properties of a cell at the current cursor position in a window of a UI surface.
 * @param sfc Pointer to the UiSurface containing the window.
 * @param w The index of the window within the surface.
 * @param cell Pointer to a UiCell structure to store the cell properties.
 * @param wstr Pointer to a wide character string to store the cell's character(s).
 * @param style Pointer to a UiStyle variable to store the cell's style attributes.
 * @param pair Pointer to a short variable to store the color pair index of the cell.
 * @return 0 on success, -1 on failure.
 *
 * This function retrieves the properties of a cell at the current cursor position
 * in a window of a UI surface. It fills in the provided UiCell structure with
 * the cell's properties, including its character(s), style attributes, and
 * color pair index. If any of the input pointers are NULL, it returns -1.
 */
int ui_get_nccell(
    UiSurface *sfc,
    ss_t w,
    const UiCell *cell,
    wchar_t *wstr,
    UiStyle *style,
    short *pair) {
    (void)sfc;
    (void)w;
    if (!cell || !wstr || !style || !pair)
        return -1;
    GCluster gc;
    gc.u32 = cell->gcluster;
    *wstr = gc.w32;
    *style = cell->stylemask;
    // Convert the channels to a color pair index
    UiChannels channels;
    channels.fb = cell->channels;
    RGB rgb;
    rgb.r = channels.f_r;
    rgb.g = channels.f_g;
    rgb.b = channels.f_b;
    // If the color index does not exist, create a new one
    int fg = ui_color_from_rgb(&rgb);
    rgb.r = channels.b_r;
    rgb.g = channels.b_g;
    rgb.b = channels.b_b;
    // If the color index does not exist, create a new one
    int bg = ui_color_from_rgb(&rgb);
    // If the color pair does not exist, create a new one
    *pair = ui_add_pair(fg, bg);
    return 0;
}
/** @brief Set the properties of a cell at the current cursor position in a window of a UI surface.
 * @param sfc Pointer to the UiSurface containing the window.
 * @param w The index of the window within the surface.
 * @param cell Pointer to a UiCell structure representing the cell properties to set.
 * @param wstr Pointer to a wide character string representing the character(s) to set in the cell.
 * @param style The style attributes to set for the cell.
 * @param pair The color pair index to set for the cell.
 * @return 0 on success, -1 on failure.
 *
 * This function sets the properties of a cell at the current cursor position
 * in a window of a UI surface. It updates the specified UiCell structure with
 * the provided character(s), style attributes, and color pair index. If any
 * of the input pointers are NULL, it returns -1.
 */
int ui_set_nccell(
    UiSurface *sfc,
    ss_t w,
    UiCell *cell,
    const wchar_t *wstr,
    const UiStyle style,
    short pair) {
    if (!cell)
        return -1;
    uint32_t fg, bg;
    nccell_init(cell);
    GCluster gc;
    unicode_to_utf8_gcluster(*wstr, &gc);
    nccell_set_styles(cell, style);
    UiChannels *chan = (UiChannels *)&cell->channels;
    ui_get_pair(pair, &fg, &bg);
    ui_color_content(fg, &chan->f_r, &chan->f_g, &chan->f_b);
    chan->f_a = 0x40;
    ui_color_content(bg, &chan->b_r, &chan->b_g, &chan->b_b);
    chan->b_a = 0x40;
    nccell_load_egc32(sfc->mplane[w], cell, gc.u32);
    return 0;
}
/** @brief Create a UiCell from a Unicode codepoint and specified foreground and background colors.
 * @param ucp Pointer to a wide character string representing the Unicode codepoint.
 * @param fg Pointer to a 32-bit unsigned integer representing the foreground color.
 * @param bg Pointer to a 32-bit unsigned integer representing the background color.
 * @return A UiCell structure representing the created cell.
 *
 * This function creates a UiCell from a Unicode codepoint and specified foreground
 * and background colors. It initializes the cell, sets its width based on the
 * Unicode codepoint, converts the codepoint to UTF-8, and sets the styles and
 * channels for the cell. The resulting UiCell can be used for rendering text
 * in a UI surface.
 */
UiCell ui_cell_from_ucp(const wchar_t *ucp, const uint32_t *fg, const uint32_t *bg) {
    nccell cell;
    nccell_init(&cell);
    GCluster *gc = (GCluster *)&cell.gcluster;
    wchar_t wstr[2] = {ucp[0], L'\0'};
    // wcwidth expects Unicode codepoints, not UTF-8 encoded bytes
    cell.width = wcwidth(wstr[0]);
    // unicode_to_utf8_gcluster converts Unicode codepoints to UTF-8
    unicode_to_utf8_gcluster(wstr[0], gc);
    nccell_set_styles(&cell, WA_NORMAL);
    UiChannels *chan = (UiChannels *)&cell.channels;
    chan->bargb = *bg;
    chan->fargb = *fg;
    chan->b_a = chan->f_a = 0x40;
    return cell;
}
/* -------------------------------------------------------------------------
   Colors, Color Pairs
   ------------------------------------------------------------------------- */
/** @brief Convert an RGB color to a color index in the UI color palette.
 * @param rgb Pointer to an RGB structure representing the color to convert.
 * @return The color index corresponding to the RGB color, or -1 if the color cannot be added.
 *
 * This function converts an RGB color to a color index in the UI color palette.
 * It applies gamma correction to the RGB values and checks if the color already
 * exists in the palette. If it does, it returns the existing index. If not, it
 * adds the new color to the palette and returns its index. If the palette is full,
 * it returns -1.
 */
uint ui_color_from_rgb(RGB *rgb) {
    uint i;
    RGB tmp;
    ui_apply_gamma(rgb);
    for (i = 0; i < ui_color_cnt && i < UI_COLORS; i++) {
        ui_color_content(i, &tmp.r, &tmp.g, &tmp.b);
        if (rgb->r == tmp.r && rgb->g == tmp.g && rgb->b == tmp.b)
            return i;
    }
    if (i < UI_COLORS) {
        if (i < 16) {
            std_color[i].r = rgb->r;
            std_color[i].g = rgb->g;
            std_color[i].b = rgb->b;
        }
        ui_init_color(i, rgb->r, rgb->g, rgb->b);
        if (ui_color_cnt + 1 < UI_COLORS)
            ui_color_cnt++;
        return ui_color_cnt - 1;
    }
    return -1;
}
/* ------------------------------------------------------------------------- */
/** @brief Add a new color pair to the UI color pair palette.
 * @param fg The foreground color index for the new color pair.
 * @param bg The background color index for the new color pair.
 * @return The index of the newly added color pair, or EXIT_FAILURE if the maximum number of pairs is exceeded.
 *
 * This function adds a new color pair to the UI color pair palette. It checks
 * if the specified foreground and background colors already exist in the palette.
 * If they do, it returns the existing index. If not, it adds the new color pair
 * and returns its index. If the maximum number of pairs is exceeded, it displays
 * an error message and returns EXIT_FAILURE.
 */
uint ui_add_pair(uint fg, uint bg) {
    uint16_t i;
    for (i = 1; i < ui_pair_cnt; i++) {
        if (ui_pair[i].fg == fg && ui_pair[i].bg == bg)
            return i;
    }
    if (i + 1 >= UI_PAIRS) {
        ssnprintf(em0, MAXLEN - 1, "%s, line: %d", __FILE__, __LINE__ - 1);
        ssnprintf(em1, MAXLEN - 1, "NotCurses COLOR_PAIRS (%d) exceeded (%d)",
                  UI_PAIRS, i);
        strerror_r(errno, em2, MAXLEN);
        ui_display_error(em0, em1, em2, nullptr);
        return (EXIT_FAILURE);
    }
    if (i < UI_PAIRS) {
        ui_pair[i].fg = fg;
        ui_pair[i].bg = bg;
        ui_pair_cnt++;
    }
    return ui_pair_cnt - 1;
}
/** @brief Get the foreground and background color indices for a given color pair index.
 * @param pair The color pair index.
 * @param fg Pointer to store the foreground color index.
 * @param bg Pointer to store the background color index.
 * @return 0 on success, -1 on error (if the pair index is out of bounds).
 */
int ui_get_pair(uint16_t pair, uint *fg, uint *bg) {
    *fg = ui_pair[pair].fg;
    *bg = ui_pair[pair].bg;
    return 0;
}
/** @brief Get the combined foreground and background color channels for a given color pair index.
 * @param pair The color pair index.
 * @return A 64-bit unsigned integer representing the combined foreground and background color channels.
 *
 * This function retrieves the combined foreground and background color channels
 * for a given color pair index. It uses the ui_get_pair function to obtain the
 * foreground and background color indices, then retrieves their RGB values and
 * constructs a UiChannels structure. The resulting combined channels are returned
 * as a 64-bit unsigned integer.
 */
uint64_t ui_get_channels_from_pair(uint16_t pair) {
    uint fg, bg;
    // ui_channels is a struct that holds the foreground and background color
    // channels
    // use channels.fb to get the combined foreground and background color channels
    UiChannels ui_channels = {};
    ui_get_pair(pair, &fg, &bg);
    ui_color_content(fg, &ui_channels.f_r, &ui_channels.f_g, &ui_channels.f_b);
    ui_channels.f_a = 0x40;
    ui_channels.f_r = ui_color[fg].r;
    ui_channels.f_g = ui_color[fg].g;
    ui_channels.f_b = ui_color[fg].b;
    bg = ui_pair[pair].bg;
    ui_color_content(bg, &ui_channels.b_r, &ui_channels.b_g, &ui_channels.b_b);
    ui_channels.b_a = 0x40;
    ui_channels.b_r = ui_color[bg].r;
    ui_channels.b_g = ui_color[bg].g;
    ui_channels.b_b = ui_color[bg].b;
    return ui_channels.fb;
}
/** @brief Initialize a color from a hexadecimal string representation.
 * @param s A string representing the color in hexadecimal format (e.g., "#RRGGBB").
 * @return The index of the initialized color in the UI color palette, or 0 if the color cannot be added.
 *
 * This function initializes a color from a hexadecimal string representation.
 * It converts the hex string to an RGB structure, applies gamma correction,
 * and checks if the color already exists in the UI color palette. If it does,
 * it returns the existing index. If not, it adds the new color to the palette
 * and returns its index. If the palette is full, it returns 0.
 */
uint ui_init_color_hex(char *s) {
    RGB rgb;
    rgb = ui_hex_to_rgb(s);
    ui_apply_gamma(&rgb);
    uint i;
    for (i = 0; i < ui_color_cnt && i < UI_COLORS; i++) {
        if (rgb.r == ui_color[i].r && rgb.g == ui_color[i].g && rgb.b == ui_color[i].b)
            return i;
    }
    if (i < UI_COLORS) {
        if (i < 16) {
            std_color[i].r = rgb.r;
            std_color[i].g = rgb.g;
            std_color[i].b = rgb.b;
        }
        ui_color[i].r = rgb.r;
        ui_color[i].g = rgb.g;
        ui_color[i].b = rgb.b;
        if (ui_color_cnt + 1 < UI_COLORS)
            ui_color_cnt++;
        return ui_color_cnt - 1;
    }
    return 0;
}
/** @brief Convert a hexadecimal color string to an RGB structure.
 * @param s A string representing the color in hexadecimal format (e.g., "#RRGGBB").
 * @return An RGB structure containing the red, green, and blue components of the color.
 *
 * This function converts a hexadecimal color string to an RGB structure. It
 * uses sscanf to parse the hex string and extract the red, green, and blue
 * components. The resulting RGB structure is returned.
 */
RGB ui_hex_to_rgb(char *s) {
    RGB rgb;
    sscanf(s, "#%02hhX%02hhX%02hhX", &rgb.r, &rgb.g, &rgb.b);
    return rgb;
}
/** @brief Get the RGB components of a color from the UI color palette.
 * @param color The index of the color in the UI color palette.
 * @param r Pointer to store the red component of the color.
 * @param g Pointer to store the green component of the color.
 * @param b Pointer to store the blue component of the color.
 * @return 0 on success, -1 on error (if the color index is out of bounds).
 *
 * This function retrieves the RGB components of a color from the UI color
 * palette. It checks if the specified color index is valid and then fills
 * in the provided pointers with the corresponding RGB values. If the color
 * index is out of bounds, it returns -1.
 */
int ui_color_content(uint16_t color, uint8_t *r, uint8_t *g, uint8_t *b) {
    if (color + 1 >= UI_COLORS)
        return -1;
    *r = ui_color[color].r;
    *g = ui_color[color].g;
    *b = ui_color[color].b;
    return 0;
}
/** @brief Initialize a color in the UI color palette with specified RGB components.
 * @param color The index of the color to initialize in the UI color palette.
 * @param r The red component of the color (0-255).
 * @param g The green component of the color (0-255).
 * @param b The blue component of the color (0-255).
 * @return 0 on success, -1 on error (if the color index is out of bounds).
 *
 * This function initializes a color in the UI color palette with specified
 * RGB components. It checks if the specified color index is valid and then
 * sets the corresponding RGB values in the ui_color array. If the color index
 * is out of bounds, it returns -1.
 */
int ui_init_color(uint16_t color, uint8_t r, uint8_t g, uint8_t b) {
    if (color + 1 >= UI_COLORS)
        return -1;
    ui_color[color].r = r;
    ui_color[color].g = g;
    ui_color[color].b = b;
    return 0;
}
/** @brief Get the foreground and background color indices for a given color pair index.
 * @param pair The color pair index.
 * @param fg Pointer to store the foreground color index.
 * @param bg Pointer to store the background color index.
 * @return 0 on success, -1 on error (if the pair index is out of bounds).
 *
 * This function retrieves the foreground and background color indices for a
 * given color pair index. It checks if the specified pair index is valid and
 * then fills in the provided pointers with the corresponding foreground and
 * background color indices. If the pair index is out of bounds, it returns -1.
 */
int ui_pair_content(uint16_t pair, uint *fg, uint *bg) {
    if (pair + 1 >= UI_PAIRS)
        return -1;
    *fg = ui_pair[pair].fg;
    *bg = ui_pair[pair].bg;
    return 0;
}
/** @brief Initialize a color pair in the UI color pair palette with specified foreground and background colors.
 * @param pair The index of the color pair to initialize in the UI color pair palette.
 * @param fg The foreground color index for the color pair.
 * @param bg The background color index for the color pair.
 * @return 0 on success, -1 on error (if the pair index is out of bounds).
 *
 * This function initializes a color pair in the UI color pair palette with
 * specified foreground and background colors. It checks if the specified
 * pair index is valid and then sets the corresponding foreground and background
 * color indices in the ui_pair array. If the pair index is out of bounds,
 * it returns -1.
 */
int ui_init_pair(uint16_t pair, uint fg, uint bg) {
    if (pair + 1 >= UI_PAIRS)
        return -1;
    ui_pair[pair].fg = fg;
    ui_pair[pair].bg = bg;
    return 0;
}
/** @brief Change the color at a specified index in the UI color palette.
 * @param color_idx The index of the color to change in the UI color palette.
 * @param color Pointer to a 32-bit unsigned integer representing the new color value.
 * @return 0 on success, -1 on error (if the color index is out of bounds).
 *
 * This function changes the color at a specified index in the UI color palette.
 * It applies gamma correction to the new color value and updates both the
 * standard and UI color arrays with the new RGB components. If the color index
 * is out of bounds, it returns -1.
 */
int ui_chg_color(uint16_t color_idx, uint32_t *color) {
    RGB rgb;
    rgb.color = *color;
    if (color_idx + 1 >= UI_COLORS)
        return -1;
    ui_apply_gamma(&rgb);
    if (color_idx < 16) {
        std_color[color_idx].r = rgb.r;
        std_color[color_idx].g = rgb.g;
        std_color[color_idx].b = rgb.b;
    }
    ui_color[color_idx].r = rgb.r;
    ui_color[color_idx].g = rgb.g;
    ui_color[color_idx].b = rgb.b;
    return 0;
}
/* -------------------------------------------------------------------------
   Non-portable escape-hatch getters (see ui_notcurses_compat.h)
   ------------------------------------------------------------------------- */
/** @brief Get the pointer to the Notcurses context associated with the UI.
 * @return A pointer to the Notcurses context, or NULL if the UI is not initialized.
 *
 * This function retrieves the pointer to the Notcurses context associated
 * with the UI. If the UI is not initialized, it returns NULL.
 */
struct notcurses *ui_notcurses_get_nc() {
    if (!ui)
        return NULL;
    return ui->nc;
}
/** @brief Get the pointer to the NcPlane associated with a specific window in a UI surface.
 * @param s Pointer to the UiSurface containing the window.
 * @param w The index of the window within the surface.
 * @return A pointer to the NcPlane associated with the specified window, or NULL if the surface is not initialized.
 *
 * This function retrieves the pointer to the NcPlane associated with a specific
 * window in a UI surface. If the surface is not initialized, it returns NULL.
 */
NcPlane *ui_notcurses_surface_get_plane(const UiSurface *s, ss_t w) {
    (void)w;
    if (!s)
        return NULL;
    return s->mplane[w];
}
