/** @file ui_notcurses_draw.c
   @ingroup ui_notcurses
   @brief NotCurses UI backend — drawing functions.

   Implements the drawing operations declared in ui_backend.h using the
   NotCurses API.
*/
#define _GNU_SOURCE
#define _XOPEN_SOURCE_EXTENDED 1

#include "ui_backend.h"
#include "ui_notcurses_internal.h"
#include <notcurses/notcurses.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

/* -------------------------------------------------------------------------
   Housekeeping functions
   ------------------------------------------------------------------------- */
/** @brief clear the window from the current cursor position to the end of the window
 * @param s surface
 * @param w window
 * @return 0 on success, -1 on error
 */
int ui_wclrtoeol(UiSurface *s, ss_t w) {
    if (!s)
        return -1;
    uint y, x, ylen, xlen, maxy, maxx;
    ncplane_cursor_yx(s->mplane[w], &y, &x);
    ncplane_dim_yx(s->mplane[w], &maxy, &maxx);
    xlen = maxx - x;
    ylen = 1;
    ncplane_erase_region(s->mplane[w], y, x, ylen, xlen);
    return 0;
}
/** @brief clear from current cursor position to bottom of window
 * @param s surface
 * @param w window
 * @return 0 on success, -1 on error
 */
int ui_wclrtobot(UiSurface *s, ss_t w) {
    if (!s)
        return -1;
    uint y, x, ylen, xlen, maxy, maxx;
    ncplane_cursor_yx(s->mplane[w], &y, &x);
    ncplane_dim_yx(s->mplane[w], &maxy, &maxx);
    xlen = maxx - x;
    ylen = 1;
    ncplane_erase_region(s->mplane[w], y, x, ylen, xlen);
    ylen = maxy - y - 1;
    xlen = maxx;
    ncplane_erase_region(s->mplane[w], y + 1, x = 0, ylen, maxx);
    return 0;
}

/* -------------------------------------------------------------------------
   Text
   ------------------------------------------------------------------------- */
/** @brief create a cell with specified character, using background cell style
 * and channels
 * @param cell pointer to UiCell
 * @param c character
 * @return 0 on success, -1 on error
 */
int mk_chimera(UiCell *cell, char c) {
    cell->gcluster = c;
    cell->gcluster_backstop = 0;
    cell->stylemask = bkgd_cell.stylemask;
    cell->channels = bkgd_cell.channels;
    return 0;
}
/** @brief on specified window, at current location, add character
 * @param s surface
 * @param w window
 * @param c character
 * @return 0 on success, -1 on error
 */
int ui_draw_ch(UiSurface *s, ss_t w, char c) {
    if (!s || !c)
        return -1;
    nccell cell = CELL_INITIALIZER(c, s->meta[w].bkgd_cell.stylemask, s->meta[w].bkgd_cell.channels);
    ncplane_putc_yx(s->mplane[w], -1, -1, &cell);
    return 0;
}
/** @brief on specified window, move to y, x, add character
 * @param s surface
 * @param w window
 * @param y row
 * @param x column
 * @param c character
 * @return 0 on success, -1 on error
 */
int ui_draw_ch_yx(UiSurface *s, ss_t w, uint y, uint x, char c) {
    if (!s || !c)
        return -1;
    nccell cell = CELL_INITIALIZER(c, s->meta[w].bkgd_cell.stylemask, s->meta[w].bkgd_cell.channels);
    ncplane_putc_yx(s->mplane[w], y, x, &cell);
    return 0;
}
/** @brief on specified window, move to y, x, add string
 * @param s surface
 * @param w window
 * @param y row
 * @param x column
 * @param text string
 * @return 0 on success, -1 on error
 */
int ui_draw_text(UiSurface *s, ss_t w, uint y, uint x, const char *text) {
    if (!s || !text)
        return -1;
    ncplane_putstr_yx(s->mplane[w], y, x, text);
    return 0;
}
/** @brief on specified window, move to y, x, add string, at most m columns
 * @param s surface
 * @param w window
 * @param y row
 * @param x column
 * @param text string
 * @param m maximum number of columns to write
 * @return 0 on success, -1 on error
 */
int ui_draw_text_n(UiSurface *s, ss_t w, uint y, uint x, const char *text, int m) {
    if (!s || !text)
        return -1;
    ncplane_putnstr_yx(s->mplane[w], y, x, m, text);
    return 0;
}
/** @brief on specified window, move to y, x, add string, at most m columns, fill with spaces if text is shorter than m
 * @param s surface
 * @param w window
 * @param y row
 * @param x column
 * @param text string
 * @param m maximum number of columns to write
 * @return 0 on success, -1 on error
 */
int ui_draw_text_fill(UiSurface *s, ss_t w, uint y, uint x, const char *text, int m) {
    if (!s || !text)
        return -1;
    char tmp_str[MAXLEN];
    strncpy(tmp_str, text, m);
    int l = strlen(text);
    for (int i = l; i < m; i++) {
        tmp_str[i] = ' ';
    }
    tmp_str[m] = '\0';
    ncplane_putnstr_yx(s->mplane[w], y, x, m, tmp_str);
    ui_render();
    return 0;
}
// -------------------------------------------------------------------------
/** @brief on specified window, at current location, add character
 * @param s surface
 * @param w window
 * @param c character
 * @return 0 on success, -1 on error
 */
int ui_waddch(UiSurface *s, ss_t w, const char c) {
    if (!s || !c)
        return -1;
    nccell cell = CELL_INITIALIZER(c, s->meta[w].bkgd_cell.stylemask, s->meta[w].bkgd_cell.channels);
    ncplane_putc_yx(s->mplane[w], -1, -1, &cell);
    return 0;
}
/** @brief on specified window, move to y, x, add character
 * @param s surface
 * @param w window
 * @param y row
 * @param x column
 * @param c character
 * @return 0 on success, -1 on error
 */
int ui_mvwaddch(UiSurface *s, ss_t w, uint y, uint x, const char c) {
    if (!s || !c)
        return -1;
    nccell cell = CELL_INITIALIZER(c, s->meta[w].bkgd_cell.stylemask, s->meta[w].bkgd_cell.channels);
    ncplane_putc_yx(s->mplane[w], y, x, &cell);
    return 0;
}
/** @brief on specified window, at current location, add string
 * @param s surface
 * @param w window
 * @param text string
 * @return 0 on success, -1 on error
 */
int ui_waddstr(UiSurface *s, ss_t w, const char *text) {
    if (!s || !text)
        return -1;
    ncplane_putstr(s->mplane[w], text);
    return 0;
}
/** @brief on specified window, at current location, add string, at most m columns
 * @param s surface
 * @param w window
 * @param text string
 * @param m maximum number of columns to write
 * @return 0 on success, -1 on error
 */
int ui_waddnstr(UiSurface *s, ss_t w, const char *text, int m) {
    if (!s || !text)
        return -1;
    ncplane_putnstr(s->mplane[w], m, text);
    return 0;
}
/** @brief on specified window, move to y, x, add string
 * @param s surface
 * @param w window
 * @param y row
 * @param x column
 * @param text string
 * @return 0 on success, -1 on error
 */
int ui_mvwaddstr(UiSurface *s, ss_t w, uint y, uint x, const char *text) {
    if (!s || !text)
        return -1;
    ncplane_putstr_yx(s->mplane[w], y, x, text);
    return 0;
}
/** @brief on specified window, move to y, x, add string, at most m columns
 * @param s surface
 * @param w window
 * @param y row
 * @param x column
 * @param text string
 * @param m maximum number of columns to write
 * @return 0 on success, -1 on error
 */
int ui_mvwaddnstr(UiSurface *s, ss_t w, uint y, uint x, const char *text, int m) {
    if (!s || !text)
        return -1;
    ncplane_putnstr_yx(s->mplane[w], y, x, m, text);
    return 0;
}
/** @brief on specified window, move to y, x, add string, at most m columns, fill with spaces if text is shorter than m
 * @param s surface
 * @param w window
 * @param y row
 * @param x column
 * @param text string
 * @param m maximum number of columns to write
 * @return 0 on success, -1 on error
 */
int ui_mvwaddstr_fill(UiSurface *s, ss_t w, uint y, uint x, const char *text, int m) {
    if (!s || !text)
        return -1;
    int l = strlen(text);
    if (l <= m) {
        char *tmp_str = (char *)malloc(m + 1);
        if (!tmp_str)
            return -1;
        strcpy(tmp_str, text);
        for (int i = l; i < (int)m; i++) {
            tmp_str[i] = ' ';
        }
        tmp_str[m] = '\0';
        ncplane_putnstr_yx(s->mplane[w], y, x, m, tmp_str);
        free(tmp_str);
    }
    return 0;
}
// ---------------------------------------------------------------------------
// Wide Characters
// ---------------------------------------------------------------------------
/** @brief on specified window, at current location, add wchar_t string
 * @param s surface
 * @param w window
 * @param wchar_t string
 * @return 0 on success, -1 on error
 */
int ui_waddwstr(UiSurface *s, ss_t w, const wchar_t *wstr) {
    if (!s || !wstr)
        return -1;
    while (*wstr != L'\0')
        ncplane_putwc_yx(s->mplane[w], -1, -1, *wstr++);
    return 0;
}
/** @brief on specified window, move to y, x, add wchar_t string
 * @param s surface
 * @param w window
 * @param wchar_t string
 * @return 0 on success, -1 on error
 */
int ui_mvwaddwstr(UiSurface *s, ss_t w, uint y, uint x, const wchar_t *wstr) {
    if (!s || !wstr)
        return -1;
    while (*wstr != L'\0')
        ncplane_putwc_yx(s->mplane[w], y, x++, *wstr++);
    return 0;
}
/** @brief on specified window, at current location, add wchar_t string, at most
 * m columns
 * @param s surface
 * @param w window
 * @param wchar_t string
 * @param m maximum number of columns to write
 * @return 0 on success, -1 on error
 */
int ui_waddnwstr(UiSurface *s, ss_t w, const wchar_t *wstr, int m) {
    if (!s || !wstr)
        return -1;
    int cols = 0;
    int width;
    while (*wstr != L'\0') {
        width = wcwidth(*wstr);
        if (width < 0)
            width = 0;
        if (cols + width > m)
            break;
        ncplane_putwc_yx(s->mplane[w], -1, -1, *wstr++);
    }
    return 0;
}
/** @brief on specified window, move to y, x, add string, at most m columns
 * @param s surface
 * @param w window
 * @param y row
 * @param x column
 * @param wchar_t string
 * @param m maximum number of columns to write
 * @return 0 on success, -1 on error
 */
int ui_mvwaddnwstr(UiSurface *s, ss_t w, uint y, uint x, const wchar_t *wstr, int m) {
    if (!s || !wstr)
        return -1;
    int cols = 0;
    int width;
    while (*wstr != L'\0') {
        width = wcwidth(*wstr);
        if (width < 0)
            width = 0;
        if (cols + width > m)
            break;
        ncplane_putwc_yx(s->mplane[w], y, x++, *wstr++);
        cols += wcwidth(*wstr);
    }
    return 0;
}
// ---------------------------------------------------------------------------
// UiCells
// ---------------------------------------------------------------------------
/** @brief on specified window, at current location, add cell
 * @param s surface
 * @param w window
 * @param cell cell string
 * @return 0 on success, -1 on error
 */
int ui_wadd_wch(UiSurface *s, ss_t w, const UiCell *cell) {
    if (!s)
        return -1;
    ncplane_putc(s->mplane[w], cell);
    return 0;
}
/** @brief on specified window, move to y, x, add cell
 * @param s surface
 * @param w window
 * @param y row
 * @param x column
 * @param cell cell string
 * @return 0 on success, -1 on error
 */
int ui_mvwadd_wch(UiSurface *s, ss_t w, uint y, uint x, const UiCell *cell) {
    if (!s)
        return -1;
    ncplane_putc_yx(s->mplane[w], y, x, cell);
    return 0;
}

/** @brief on specified window, at current location, add cell string
 * @param s surface
 * @param w window
 * @param cell cell string
 * @return 0 on success, -1 on error
 */
int ui_wadd_wchstr(UiSurface *s, ss_t w, const UiCell *cell) {
    if (!s)
        return -1;
    while (cell->gcluster != 0) {
        ncplane_putc(s->mplane[w], cell);
        cell++;
    }
    return 0;
}
/** @brief on specified window, move to y, x, add cell string
 * @param s surface
 * @param w window
 * @param y row
 * @param x column
 * @param cell cell string
 * @return 0 on success, -1 on error
 */
int ui_mvwadd_wchstr(UiSurface *s, ss_t w, uint y, uint x, const UiCell *cell) {
    if (!s)
        return -1;
    ui_wmove(s, w, y, x);
    while (cell->gcluster != 0) {
        ncplane_putc(s->mplane[w], cell);
        cell++;
    }
    return 0;
}
/** @brief on specified window, add cell string, at most m columns
 * @param s surface
 * @param w window
 * @param cell cell string
 * @param m maximum number of columns to write
 * @return 0 on success, -1 on error
 */
int ui_wadd_wchnstr(UiSurface *s, ss_t w, const UiCell *cell, uint m) {
    if (!s)
        return -1;
    uint cols = 0;
    while (cell->gcluster != 0 && cols < m) {
        ncplane_putc(s->mplane[w], cell);
        cols += cell->width;
        cell++;
    }
    return 0;
}
/** @brief on specified window, move to y, x, add cell string, at most m columns
 * @param s surface
 * @param w window
 * @param y row
 * @param x column
 * @param cell cell string
 * @param m maximum number of columns to write
 * @return 0 on success, -1 on error
 */
int ui_mvwadd_wchnstr(UiSurface *s, ss_t w, uint y, uint x, const UiCell *cell, uint m) {
    if (!s)
        return -1;
    ui_wmove(s, w, y, x);
    uint cols = 0;
    while (cell->gcluster != 0 && cols < m) {
        ncplane_putc(s->mplane[w], cell);
        cols += cell->width;
        cell++;
    }
    return 0;
}
/* -------------------------------------------------------------------------
   Screen managemen t
   ------------------------------------------------------------------------- */
void ui_restore_wins() {
    //  for (int s = 0; s <= sfc_ptr; s++) {
    //      for (int w = 0; w < 8; w++)
    //          if (ui_surface[s]->mplane[w] != nullptr)
    //      touchwin(ui_surface[s]->mplane[w]);
    //  }
    //  ui_render(ui);
}
/* -------------------------------------------------------------------------
   Image Display
   ------------------------------------------------------------------------- */
/** @brief display an image on the terminal using NotCurses
 * @param nc NotCurses context
 * @param mm UiMultiMedia structure to hold the image and surface
 * @param image_file path to the image file
 * @param y height of the image display area (or -1 for full height)
 * @param x width of the image display area (or -1 for full width)
 * @param begy starting row for the image display area (or -1 for top)
 * @param begx starting column for the image display area (or -1 for left)
 * @return pointer to the ncvisual structure on success, nullptr on error
 */
struct ncvisual *ui_display_image(struct notcurses *nc, UiMultiMedia *mm, const char *image_file, int y, int x, int begy, int begx) {
    struct ncplane *stdn = notcurses_stdplane(nc);
    unsigned term_rows, term_cols;
    ncplane_dim_yx(stdn, &term_rows, &term_cols);
    mm->ncv = ncvisual_from_file(image_file);
    if (!mm->ncv) {
        fprintf(stderr, "Error: Could not load image file.\n");
        return nullptr;
    }
    struct ncvgeom geom;
    if (ncvisual_geom(nc, mm->ncv, NULL, &geom) < 0) {
        ncvisual_destroy(mm->ncv);
        return nullptr;
    }
    if (begy == -1)
        begy = 0;
    if (begx == -1)
        begx = 0;
    if (y == -1)
        y = (int)term_rows - begy;
    if (x == -1)
        x = (int)term_cols - begx;
    if (begy + y > (int)term_rows)
        y = (int)term_rows - begy;
    if (begx + x > (int)term_cols)
        x = (int)term_cols - begx;
    int max_rows = y - 2;
    int max_cols = x - 2;
    if (max_rows <= 0) {
        ncvisual_destroy(mm->ncv);
        return nullptr;
    }
    struct ncvisual_options vopts_calc = {
        .scaling = NCSCALE_SCALE_HIRES,
        .blitter = NCBLIT_PIXEL,
    };
    struct ncplane_options nopts = {
        .y = begy,
        .x = x,
        .rows = max_rows,
        .cols = max_cols,
    };
    struct ncplane *tmp_bound_plane = ncplane_create(stdn, &nopts);
    if (!tmp_bound_plane) {
        ncvisual_destroy(mm->ncv);
        return nullptr;
    }
    vopts_calc.n = tmp_bound_plane;
    if (ncvisual_geom(nc, mm->ncv, &vopts_calc, &geom) < 0) {
        ncplane_destroy(tmp_bound_plane);
        ncvisual_destroy(mm->ncv);
        return nullptr;
    }
    unsigned rows = geom.rcelly;
    unsigned cols = geom.rcellx;
    // dimensions
    ncplane_destroy(tmp_bound_plane);
    mm->sfc = ui_surface_box(stdsfc, BOX, rows + 2, cols + 2, begy, 0, image_file);
    ui_surface_addwin(mm->sfc, WIN, BOX, rows, cols, 1, 1);
    struct ncvisual_options vopts = {
        .n = mm->sfc->mplane[WIN],
        .scaling = NCSCALE_SCALE_HIRES,
        .blitter = NCBLIT_PIXEL,
    };
    ncvisual_blit(nc, mm->ncv, &vopts);
    notcurses_render(nc);
    return mm->ncv;
}
