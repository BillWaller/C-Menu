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
    nccell cell = DEFAULT_INITIALIZER(c);
    ncplane_putc_yx(s->mplane[w], -1, -1, &cell);
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
        ncplane_putnstr_yx(s->mplane[w], y, x, m, text);
        free(tmp_str);
    }
    return 0;
}
// ---------------------------------------------------------------------------
// Wide Characters
// ---------------------------------------------------------------------------
int ui_waddwstr(UiSurface *s, uint w, const wchar_t *wstr) {
    if (!s || !wstr)
        return -1;
    while (*wstr != L'\0')
        ncplane_putwc_yx(s->mplane[w], -1, -1, *wstr++);
    return 0;
}
int ui_mvwaddwstr(UiSurface *s, uint w, uint y, uint x, const wchar_t *wstr) {
    if (!s || !wstr)
        return -1;
    while (*wstr != L'\0')
        ncplane_putwc_yx(s->mplane[w], y, x++, *wstr++);
    return 0;
}
int ui_waddnwstr(UiSurface *s, uint w, const wchar_t *wstr, int m) {
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
int ui_mvwaddnwstr(UiSurface *s, uint w, uint y, uint x, const wchar_t *wstr, int m) {
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
int ui_waddwch(UiSurface *s, uint w, const UiCell *cell) {
    if (!s)
        return -1;
    ncplane_putc(s->mplane[w], cell);
    return 0;
}
int ui_mvwaddwch(UiSurface *s, uint w, uint y, uint x, const UiCell *cell) {
    if (!s)
        return -1;
    ncplane_putc_yx(s->mplane[w], y, x, cell);
    return 0;
}
int ui_waddwchstr(UiSurface *s, uint w, const UiCell *cell) {
    if (!s)
        return -1;
    while (cell->gcluster != 0)
        ncplane_putc(s->mplane[w], cell++);
    return 0;
}
int ui_mvwaddwchstr(UiSurface *s, uint w, uint y, uint x, const UiCell *cell) {
    if (!s)
        return -1;
    ui_wmove(s, w, y, x);
    while (cell->gcluster != 0)
        ncplane_putc(s->mplane[w], cell++);
    return 0;
}
int ui_waddwchnstr(UiSurface *s, uint w, const UiCell *cell, uint m) {
    if (!s)
        return -1;
    uint cols = 0;
    while (cell->gcluster != 0 && cols < m) {
        ncplane_putc(s->mplane[w], cell++);
        cols += cell->width;
    }
    return 0;
}

int ui_mvwaddwchnstr(UiSurface *s, uint w, uint y, uint x, const UiCell *cell, uint m) {
    if (!s)
        return -1;
    ui_wmove(s, w, y, x);
    uint cols = 0;
    while (cell->gcluster != 0 && cols < m) {
        ncplane_putc(s->mplane[w], cell++);
        cols += cell->width;
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
