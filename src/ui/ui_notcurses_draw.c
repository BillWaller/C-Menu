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
    cell->stylemask = bkgd_cell.stylemask;
    cell->channels = bkgd_cell.channels;
    return 0;
}
int ui_draw_ch(UiSurface *s, uint w, char c) {
    if (!s || !c)
        return -1;
    nccell cell = DEFAULT_INITIALIZER(c);
    ncplane_putc_yx(s->mplane[w], -1, -1, &cell);
    return 0;
}
int ui_draw_ch_yx(UiSurface *s, uint w, uint y, uint x, char c) {
    if (!s || !c)
        return -1;
    nccell cell = DEFAULT_INITIALIZER(c);
    ncplane_putc_yx(s->mplane[w], y, x, &cell);
    return 0;
}
int ui_draw_text(UiSurface *s, uint w, uint y, uint x, const char *text) {
    if (!s || !text)
        return -1;
    ncplane_putstr_yx(s->mplane[w], y, x, text);
    return 0;
}
int ui_draw_text_n(UiSurface *s, uint w, uint y, uint x, const char *text, int m) {
    if (!s || !text)
        return -1;
    ncplane_putnstr_yx(s->mplane[w], y, x, m, text);
    return 0;
}
int ui_draw_text_fill(UiSurface *s, uint w, uint y, uint x, const char *text, int m) {
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
int ui_mvwaddch(UiSurface *s, uint w, uint y, uint x, const char c) {
    if (!s || !c)
        return -1;
    ui_wmove(s, w, y, x);
    nccell cell = CELL_INITIALIZER(c, ncplane_styles(s->mplane[w]),
                                   ncplane_channels(s->mplane[w]));
    return ncplane_putc_yx(s->mplane[w], -1, -1, &cell);
    return 0;
}
int ui_waddstr(UiSurface *s, uint w, const char *text) {
    if (!s || !text)
        return -1;
    ncplane_putstr(s->mplane[w], text);
    return 0;
}
int ui_waddnstr(UiSurface *s, uint w, const char *text, int m) {
    if (!s || !text)
        return -1;
    ncplane_putnstr(s->mplane[w], m, text);
    return 0;
}
int ui_mvwaddstr(UiSurface *s, uint w, uint y, uint x, const char *text) {
    if (!s || !text)
        return -1;
    ncplane_putstr_yx(s->mplane[w], y, x, text);
    return 0;
}
int ui_mvwaddnstr(UiSurface *s, uint w, uint y, uint x, const char *text, int m) {
    if (!s || !text)
        return -1;
    ncplane_putnstr_yx(s->mplane[w], y, x, m, text);
    return 0;
}
int ui_mvwaddnstr_fill(UiSurface *s, uint w, uint y, uint x, const char *text, int m) {
    if (!s || !text)
        return -1;
    uint l = strlen(text);
    if (l < (uint)m) {
        char *tmp_str = (char *)malloc(m + 1);
        if (!tmp_str)
            return -1;
        strcpy(tmp_str, text);
        for (int i = l; i < (int)m; i++) {
            tmp_str[i] = ' ';
        }
        tmp_str[m] = '\0';
        ncplane_putnstr_yx(s->mplane[w], y, x, m, text);
        free(tmp_str);
    }
    return 0;
}
int ui_mvwaddstr_fill(UiSurface *s, uint w, uint y, uint x, const char *text, int m) {
    if (!s || !text)
        return -1;
    uint l = strlen(text);
    if (l < (uint)m) {
        char *tmp_str = (char *)malloc(m + 1);
        if (!tmp_str)
            return -1;
        strcpy(tmp_str, text);
        for (int i = l; i < (int)m; i++) {
            tmp_str[i] = ' ';
        }
        tmp_str[m] = '\0';
        ncplane_putstr_yx(s->mplane[w], y, x, text);
        free(tmp_str);
    }
    return 0;
}
// ---------------------------------------------------------------------------
// Wide Characters
// ---------------------------------------------------------------------------
int ui_waddnwstr(UiSurface *s, uint w, const wchar_t *wstr, int m) {
    if (!s || !wstr)
        return -1;
    wchar_t wc;
    for (int i = 0; i < m && wstr[i] != L'\0'; i++) {
        wc = wstr[i];
        ncplane_putwc_yx(s->mplane[w], -1, -1, wc);
    }
    return 0;
}
int ui_mvwaddnwstr(UiSurface *s, uint w, uint y, uint x, const wchar_t *wstr, int m) {
    if (!s || !wstr)
        return -1;
    ncplane_cursor_move_yx(s->mplane[w], y, x);
    wchar_t wc;
    for (int i = 0; i < m && wstr[i] != L'\0'; i++) {
        wc = wstr[i];
        ncplane_putwc_yx(s->mplane[w], -1, -1, wc);
    }
    return 0;
}

// ---------------------------------------------------------------------------
// UiCells
// ---------------------------------------------------------------------------
int ui_wadd_cell(UiSurface *s, uint w, UiCell *uic) {
    if (!s)
        return -1;
    ncplane_putc(s->mplane[w], uic);
    return 0;
}
int ui_mvwadd_cell(UiSurface *s, uint w, uint y, uint x, UiCell *uic) {
    if (!s)
        return -1;
    GCluster gc;
    gc.u32 = uic->gcluster;
    uint8_t a, b, c, d;
    a = (gc.u32 >> 24) & 0xFF;
    b = (gc.u32 >> 16) & 0xFF;
    c = (gc.u32 >> 8) & 0xFF;
    d = gc.u32 & 0xFF;
    gc.u8[0] = d;
    gc.u8[1] = c;
    gc.u8[2] = b;
    gc.u8[3] = a;
    uic->gcluster = gc.u32;
    // nccell_load_egc32(s->mplane[w], uic, gc.u32);
    ncplane_putc_yx(s->mplane[w], y, x, uic);
    return 0;
}
int ui_mvwadd_cellstr(UiSurface *s, uint w, uint y, uint x, UiCell *uic) {
    if (!s)
        return -1;
    ui_wmove(s, w, y, x);
    ncplane_putc(s->mplane[w], uic);
    return 0;
}
int ui_wadd_cellnstr(UiSurface *s, uint w, UiCell *uic, uint m) {
    if (!s)
        return -1;
    for (uint i = 0; i < m; i++) {
        ncplane_putc(s->mplane[w], uic);
    }
    return 0;
}
// uic strings
int ui_mvwadd_cellnstr(UiSurface *s, uint w, uint y, uint x, UiCell *uic, uint m) {
    if (!s)
        return -1;
    ui_wmove(s, w, y, x);
    ncplane_putc(s->mplane[w], uic);
    return 0;
}
// uic strings
// ---------------------------------------------------------------------------
// nccells
// ---------------------------------------------------------------------------
int ui_wadd_wch(UiSurface *s, uint w, const nccell *uic) {
    if (!s)
        return -1;

    ncplane_putc(s->mplane[w], uic);
    return 0;
}
int ui_mvwadd_wch(UiSurface *s, uint w, uint y, uint x, const nccell *uic) {
    if (!s)
        return -1;
    ncplane_putc_yx(s->mplane[w], y, x, uic);
    return 0;
}
// uic strings
int ui_wadd_wchstr(UiSurface *s, uint w, nccell *uic) {
    if (!s)
        return -1;
    uint i = 0;
    while (uic[i].gcluster != '\0') {
        ncplane_putc(s->mplane[w], uic);
        i++;
    }
    return 0;
}
int ui_mvwadd_wchstr(UiSurface *s, uint w, uint y, uint x, nccell *uic) {
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
int ui_wadd_wchnstr(UiSurface *s, uint w, nccell *uic, uint m) {
    if (!s)
        return -1;
    uint i = 0;
    while (uic[i].gcluster != '\0' && i < m) {
        ncplane_putc(s->mplane[w], uic);
        i++;
    }
    return 0;
}

int ui_mvwadd_wchnstr(UiSurface *s, uint w, uint y, uint x, nccell *uic, uint m) {
    if (!s)
        return -1;
    uint i = 0;
    ui_wmove(s, w, y, x);
    while (i < m) {
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
