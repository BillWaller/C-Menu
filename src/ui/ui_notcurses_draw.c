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
int mk_chimera(UiCell *cell, char c) {
    cell->gcluster = c;
    cell->gcluster_backstop = 0;
    cell->stylemask = bkgd_cell.stylemask;
    cell->channels = bkgd_cell.channels;
    return 0;
}
int ui_draw_ch(UiSurface *s, ss_t w, char c) {
    if (!s || !c)
        return -1;
    nccell cell = CELL_INITIALIZER(c, s->meta[w].bkgd_cell.stylemask, s->meta[w].bkgd_cell.channels);
    ncplane_putc_yx(s->mplane[w], -1, -1, &cell);
    return 0;
}
int ui_draw_ch_yx(UiSurface *s, ss_t w, uint y, uint x, char c) {
    if (!s || !c)
        return -1;
    nccell cell = CELL_INITIALIZER(c, s->meta[w].bkgd_cell.stylemask, s->meta[w].bkgd_cell.channels);
    ncplane_putc_yx(s->mplane[w], y, x, &cell);
    return 0;
}
int ui_draw_text(UiSurface *s, ss_t w, uint y, uint x, const char *text) {
    if (!s || !text)
        return -1;
    ncplane_putstr_yx(s->mplane[w], y, x, text);
    return 0;
}
int ui_draw_text_n(UiSurface *s, ss_t w, uint y, uint x, const char *text, int m) {
    if (!s || !text)
        return -1;
    ncplane_putnstr_yx(s->mplane[w], y, x, m, text);
    return 0;
}
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
int ui_waddch(UiSurface *s, ss_t w, const char c) {
    if (!s || !c)
        return -1;
    nccell cell = CELL_INITIALIZER(c, s->meta[w].bkgd_cell.stylemask, s->meta[w].bkgd_cell.channels);
    ncplane_putc_yx(s->mplane[w], -1, -1, &cell);
    return 0;
}
int ui_mvwaddch(UiSurface *s, ss_t w, uint y, uint x, const char c) {
    if (!s || !c)
        return -1;
    nccell cell = CELL_INITIALIZER(c, s->meta[w].bkgd_cell.stylemask, s->meta[w].bkgd_cell.channels);
    ncplane_putc_yx(s->mplane[w], y, x, &cell);
    return 0;
}
int ui_waddstr(UiSurface *s, ss_t w, const char *text) {
    if (!s || !text)
        return -1;
    ncplane_putstr(s->mplane[w], text);
    return 0;
}
int ui_waddnstr(UiSurface *s, ss_t w, const char *text, int m) {
    if (!s || !text)
        return -1;
    ncplane_putnstr(s->mplane[w], m, text);
    return 0;
}
int ui_mvwaddstr(UiSurface *s, ss_t w, uint y, uint x, const char *text) {
    if (!s || !text)
        return -1;
    ncplane_putstr_yx(s->mplane[w], y, x, text);
    return 0;
}
int ui_mvwaddnstr(UiSurface *s, ss_t w, uint y, uint x, const char *text, int m) {
    if (!s || !text)
        return -1;
    ncplane_putnstr_yx(s->mplane[w], y, x, m, text);
    return 0;
}
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
int ui_waddwstr(UiSurface *s, ss_t w, const wchar_t *wstr) {
    if (!s || !wstr)
        return -1;
    while (*wstr != L'\0')
        ncplane_putwc_yx(s->mplane[w], -1, -1, *wstr++);
    return 0;
}
int ui_mvwaddwstr(UiSurface *s, ss_t w, uint y, uint x, const wchar_t *wstr) {
    if (!s || !wstr)
        return -1;
    while (*wstr != L'\0')
        ncplane_putwc_yx(s->mplane[w], y, x++, *wstr++);
    return 0;
}
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
int ui_wadd_wch(UiSurface *s, ss_t w, const UiCell *cell) {
    if (!s)
        return -1;
    ncplane_putc(s->mplane[w], cell);
    return 0;
}
int ui_mvwadd_wch(UiSurface *s, ss_t w, uint y, uint x, const UiCell *cell) {
    if (!s)
        return -1;
    ncplane_putc_yx(s->mplane[w], y, x, cell);
    return 0;
}
int ui_wadd_wchstr(UiSurface *s, ss_t w, const UiCell *cell) {
    if (!s)
        return -1;
    while (cell->gcluster != 0) {
        ncplane_putc(s->mplane[w], cell);
        cell++;
    }
    return 0;
}
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
struct ncvisual *ui_display_image(struct notcurses *nc, UiMultiMedia *mm, const char *image_file, int y, int x, int begy, int begx) {
    // 1. Get the current standard plane size
    struct ncplane *stdn = notcurses_stdplane(nc);
    unsigned term_rows, term_cols;
    ncplane_dim_yx(stdn, &term_rows, &term_cols);
    mm->ncv = ncvisual_from_file(image_file);
    if (!mm->ncv) {
        fprintf(stderr, "Error: Could not load image file.\n");
        return nullptr;
    }
    // 3. Query the image cell dimensions
    struct ncvgeom geom;
    if (ncvisual_geom(nc, mm->ncv, NULL, &geom) < 0) {
        ncvisual_destroy(mm->ncv);
        return nullptr;
    }
    // 4. Calculate available bounding box below the UI
    if (begy == -1) {
        begy = 0;
    }
    if (begx == -1) {
        begx = 0;
    }
    if (y == -1) {
        y = (int)term_rows - begy;
    }
    if (x == -1) {
        x = (int)term_cols - begx;
    }
    if (begy + y > (int)term_rows) {
        y = (int)term_rows - begy;
    }
    if (begx + x > (int)term_cols) {
        x = (int)term_cols - begx;
    }
    int max_rows = y - 2;
    int max_cols = x - 2;
    if (max_rows <= 0) {
        ncvisual_destroy(mm->ncv);
        return nullptr;
    }
    // 5. Scale image to fit the bounding box while preserving aspect ratio
    struct ncvisual_options vopts_calc = {
        .scaling = NCSCALE_SCALE_HIRES,
        .blitter = NCBLIT_PIXEL,
    };
    // Create a dummy/temporary plane to define the maximum bounding box for the
    // layout engine
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

    // Let Notcurses populate rcelly and rcellx based on the bounding box plane
    if (ncvisual_geom(nc, mm->ncv, &vopts_calc, &geom) < 0) {
        ncplane_destroy(tmp_bound_plane);
        ncvisual_destroy(mm->ncv);
        return nullptr;
    }

    // Extract the exact rendered cell dimensions
    unsigned rows = geom.rcelly;
    unsigned cols = geom.rcellx;

    // Destroy the temporary bounding plane now that we have the exact dimensions
    ncplane_destroy(tmp_bound_plane);

    // 5b. Allocate the perfectly sized UI surfaces

    mm->sfc = ui_surface_box(stdsfc, BOX, rows + 2, cols + 2, begy, 0, image_file);
    ui_surface_addwin(mm->sfc, WIN, BOX, rows, cols, 1, 1);

    // 6. Setup the blit options to create a subplane for you
    struct ncvisual_options vopts = {
        .n = mm->sfc->mplane[WIN],
        .scaling = NCSCALE_SCALE_HIRES,
        .blitter = NCBLIT_PIXEL,
        //      .flags = NCVISUAL_OPTION_CHILDPLANE,
    };
    ncvisual_blit(nc, mm->ncv, &vopts);
    notcurses_render(nc);
    return mm->ncv;
}
