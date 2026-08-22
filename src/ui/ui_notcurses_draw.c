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

#define CELL_CHAR_INITIALIZER(c) { \
    .gcluster = (c),               \
    .gcluster_backstop = 0,        \
    .stylemask = 0,                \
    .channels = 0,                 \
}
#define CELL_INITIALIZER(c, s, chan) { \
    .gcluster = (c),                   \
    .gcluster_backstop = 0,            \
    .stylemask = (s),                  \
    .channels = (chan),                \
}
#define CHIMERA_INITIALIZER(c) {         \
    .gcluster = (c),                     \
    .gcluster_backstop = 0,              \
    .stylemask = (donor_cell.stylemask), \
    .channels = (donor_cell.channels),   \
}

/* -------------------------------------------------------------------------
   Housekeeping functions
   ------------------------------------------------------------------------- */

int ui_wclrtoeol(UiSurface *s, uint w) {
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

int ui_wclrtobot(UiSurface *s, uint w) {
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
    cell->stylemask = donor_cell.stylemask;
    cell->channels = donor_cell.channels;
    return 0;
}
/** @brief Draw a single character using a stylemask and channels from a donor
 * cell set by bkgdset().
 * @details To accomodate legacy NCurses colors and color pairs, we use named
 * and indexed nccells. It is very convenient, but not required here as it is
 * with NCurses.
 * @notes A Chimera is a single hybrid cell created by joining parts from two
 * different cells. Got the idea from Dr. Fauci.
 **/
int ui_draw_ch(UiSurface *s, uint w, uint y, uint x, char c) {
    if (!s || !c)
        return -1;
    // mk_chimera(&cell, c);
    nccell cell = CHIMERA_INITIALIZER(c);
    ncplane_putc_yx(s->mplane[w], y, x, &cell);
    return 0;
}
int ui_draw_cell(UiSurface *s, uint w, uint y, uint x, char c) {
    if (!s || !c)
        return -1;
    nccell cell = CHIMERA_INITIALIZER(c);
    ncplane_putc_yx(s->mplane[w], y, x, &cell);
    return 0;
}

/** @brief Draw UTF-8 text at (y, x) with optional style. */
int ui_draw_text(UiSurface *s, uint w, uint y, uint x, const char *text) {
    if (!s || !text)
        return -1;
    ncplane_putstr_yx(s->mplane[w], y, x, text);
    return 0;
}

/** @brief Draw at most @p n bytes of UTF-8 text at (y, x). */
int ui_draw_text_n(UiSurface *s, uint w, uint y, uint x, const char *text, int n) {
    if (!s || !text)
        return -1;
    ncplane_putnstr_yx(s->mplane[w], y, x, n, text);
    return 0;
}
int ui_draw_text_fill(UiSurface *s, uint w, uint y, uint x, const char *text, int n) {
    if (!s || !text)
        return -1;
    char tmp_str[MAXLEN];
    strncpy(tmp_str, text, n);
    int l = strlen(text);
    for (int i = l; i < n; i++) {
        tmp_str[i] = ' ';
    }
    tmp_str[n] = '\0';
    ncplane_putnstr_yx(s->mplane[w], y, x, n, tmp_str);
    ui_render();
    return 0;
}
// -------------------------------------------------------------------------
int ui_mvwaddch(UiSurface *s, uint w, uint y, uint x, char c) {
    if (!s || !c)
        return -1;
    nccell cell = CELL_INITIALIZER(c, ncplane_styles(s->mplane[w]),
                                   ncplane_channels(s->mplane[w]));
    return ncplane_putc_yx(s->mplane[w], y, x, &cell);
    return 0;
}
int ui_waddstr(UiSurface *s, uint w, const char *text) {
    if (!s || !text)
        return -1;
    ncplane_putstr(s->mplane[w], text);
    return 0;
}
int ui_mvwaddstr(UiSurface *s, uint w, uint y, uint x, const char *text) {
    if (!s || !text)
        return -1;
    ncplane_putstr_yx(s->mplane[w], y, x, text);
    return 0;
}
int ui_mvwaddnstr(UiSurface *s, uint w, uint y, uint x, const char *text, int n) {
    if (!s || !text)
        return -1;
    ncplane_putnstr_yx(s->mplane[w], y, x, n, text);
    return 0;
}
int ui_mvwaddnstr_fill(UiSurface *s, uint w, uint y, uint x, const char *text, int n) {
    if (!s || !text)
        return -1;
    uint l = strlen(text);
    if (l < (uint)n) {
        char *tmp_str = (char *)malloc(n + 1);
        if (!tmp_str)
            return -1;
        strcpy(tmp_str, text);
        for (int i = l; i < (int)n; i++) {
            tmp_str[i] = ' ';
        }
        tmp_str[n] = '\0';
        ncplane_putnstr_yx(s->mplane[w], y, x, n, text);
        free(tmp_str);
    }
    return 0;
}
int ui_mvwaddstr_fill(UiSurface *s, uint w, uint y, uint x, const char *text, int n) {
    if (!s || !text)
        return -1;
    uint l = strlen(text);
    if (l < (uint)n) {
        char *tmp_str = (char *)malloc(n + 1);
        if (!tmp_str)
            return -1;
        strcpy(tmp_str, text);
        for (int i = l; i < (int)n; i++) {
            tmp_str[i] = ' ';
        }
        tmp_str[n] = '\0';
        ncplane_putstr_yx(s->mplane[w], y, x, text);
        free(tmp_str);
    }
    return 0;
}
// ---------------------------------------------------------------------------
// Wide Characters
// ---------------------------------------------------------------------------
int ui_waddnwstr(UiSurface *s, uint w, const wchar_t *wstr, int n) {
    if (!s || !wstr)
        return -1;
    wchar_t wc;
    for (int i = 0; i < n && wstr[i] != L'\0'; i++) {
        wc = wstr[i];
        ncplane_putwc_yx(s->mplane[w], -1, -1, wc);
    }
    return 0;
}
int ui_mvwaddnwstr(UiSurface *s, uint w, uint y, uint x, const wchar_t *wstr, int n) {
    if (!s || !wstr)
        return -1;
    ncplane_cursor_move_yx(s->mplane[w], y, x);
    wchar_t wc;
    for (int i = 0; i < n && wstr[i] != L'\0'; i++) {
        wc = wstr[i];
        ncplane_putwc_yx(s->mplane[w], -1, -1, wc);
    }
    return 0;
}

// ---------------------------------------------------------------------------
// UiCells
// ---------------------------------------------------------------------------

int ui_wadd_cell(UiSurface *s, uint w, const UiCell *uic) {
    if (!s)
        return -1;
    ncplane_putc(s->mplane[w], uic);
    return 0;
}
int ui_mvwadd_cell(UiSurface *s, uint w, uint y, uint x, UiCell *uic) {
    if (!s)
        return -1;
    ncplane_putc_yx(s->mplane[w], y, x, uic);
    return 0;
}
// uic strings
int ui_wadd_cellstr(UiSurface *s, uint w, UiCell *uic) {
    if (!s)
        return -1;
    uint i = 0;
    while (uic[i].gcluster != L'\0') {
        ncplane_putc(s->mplane[w], uic);
        i++;
    }
    return 0;
}
int ui_mvwadd_cellstr(UiSurface *s, uint w, uint y, uint x, UiCell *uic) {
    if (!s)
        return -1;
    ui_wmove(s, w, y, x);
    uint i = 0;
    while (uic[i].gcluster != L'\0') {
        ncplane_putc(s->mplane[w], uic);
        i++;
    }
    return 0;
}
int ui_wadd_cellnstr(UiSurface *s, uint w, UiCell *uic, uint n) {
    if (!s)
        return -1;
    uint i = 0;
    while (uic[i].gcluster != L'\0' && i < n) {
        ncplane_putc(s->mplane[w], uic);
        i++;
    }
    return 0;
}
int ui_mvwadd_cellnstr(UiSurface *s, uint w, uint y, uint x, UiCell *uic, uint n) {
    if (!s)
        return -1;
    uint i = 0;
    ui_wmove(s, w, y, x);
    while (uic[i].gcluster != L'\0' && i < n) {
        ncplane_putc(s->mplane[w], uic);
        i++;
    }
    return 0;
}
// ---------------------------------------------------------------------------
// UiCells
// ---------------------------------------------------------------------------
int ui_wadd_wch(UiSurface *s, uint w, const UiCell *uic) {
    if (!s)
        return -1;

    ncplane_putc(s->mplane[w], uic);
    return 0;
}
int ui_mvwadd_wch(UiSurface *s, uint w, uint y, uint x, const UiCell *uic) {
    if (!s)
        return -1;
    ncplane_putc_yx(s->mplane[w], y, x, uic);
    return 0;
}
// uic strings
int ui_wadd_wchstr(UiSurface *s, uint w, UiCell *uic) {
    if (!s)
        return -1;
    uint i = 0;
    while (uic[i].gcluster != '\0') {
        ncplane_putc(s->mplane[w], uic);
        i++;
    }
    return 0;
}
int ui_mvwadd_wchstr(UiSurface *s, uint w, uint y, uint x, UiCell *uic) {
    if (!s)
        return -1;
    ui_wmove(s, w, y, x);
    uint i = 0;
    while (uic[i].gcluster != '\0') {
        ncplane_putc(s->mplane[w], uic);
        i++;
    }
    return 0;
}
int ui_wadd_wchnstr(UiSurface *s, uint w, UiCell *uic, uint n) {
    if (!s)
        return -1;
    uint i = 0;
    while (uic[i].gcluster != '\0' && i < n) {
        ncplane_putc(s->mplane[w], uic);
        i++;
    }
    return 0;
}

int ui_mvwadd_wchnstr(UiSurface *s, uint w, uint y, uint x, UiCell *uic, uint n) {
    if (!s)
        return -1;
    uint i = 0;
    ui_wmove(s, w, y, x);
    while (i < n) {
        ncplane_putc(s->mplane[w], uic);
        i++;
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
    //      // touchwin(ui_surface[s]->mplane[w]);
    //  }
    //  ui_render(ui);
}
