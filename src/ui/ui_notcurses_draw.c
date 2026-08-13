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
    .reserved = 0,                 \
    .stylemask = 0,                \
    .channels = 0,                 \
}
#define CELL_INITIALIZER(c, s, chan) { \
    .gcluster = (c),                   \
    .gcluster_backstop = 0,            \
    .reserved = 0,                     \
    .stylemask = (s),                  \
    .channels = (chan),                \
}

// static inline void
// nccell_init(nccell* c){
//   memset(c, 0, sizeof(*c));
// }

/* -------------------------------------------------------------------------
   Text
   ------------------------------------------------------------------------- */

/** @brief Draw UTF-8 text at (y, x) with optional style. */
int ui_draw_text(UiSurface *s, int w, int y, int x, const UiStyle *style,
                 const char *text) {
    if (!s || !text)
        return -1;
    if (style) {
        ncplane_set_channels(s->mplane[w], ui_notcurses_channels_from_style(style));
        ncplane_set_styles(s->mplane[w], ui_notcurses_attrs_from_style(style));
    }
    ncplane_putstr_yx(s->mplane[w], y, x, text);
    return 0;
}

/** @brief Draw at most @p n bytes of UTF-8 text at (y, x). */
int ui_draw_text_n(UiSurface *s, int w, int y, int x, const UiStyle *style,
                   const char *text, size_t n) {
    if (!s || !text)
        return -1;
    if (style) {
        ncplane_set_channels(s->mplane[w], ui_notcurses_channels_from_style(style));
        ncplane_set_styles(s->mplane[w], ui_notcurses_attrs_from_style(style));
    }
    ncplane_putnstr_yx(s->mplane[w], y, x, n, text);
    return 0;
}
// ---------------------------------------------------------------------------
// nccells
// ---------------------------------------------------------------------------
// uic character single
int ui_wadd_wch(UiSurface *s, int w, const UiCell uic) {
    if (!s)
        return -1;
    struct nccell c;
    nccell_prime(s->mplane[w], &c, uic.gcluster, uic.stylemask, uic.channels);
    ncplane_putc(s->mplane[w], &c);
    return 0;
}
int ui_mvwadd_wch(UiSurface *s, int w, int y, int x, const UiCell uic) {
    if (!s)
        return -1;
    struct nccell c;
    nccell_prime(s->mplane[w], &c, uic.gcluster, uic.stylemask, uic.channels);
    ncplane_putc_yx(s->mplane[w], y, x, &c);
    return 0;
}
// uic strings
int ui_wadd_wchstr(UiSurface *s, int w, UiCell *uic) {
    if (!s)
        return -1;
    struct nccell c;
    int i = 0;
    while (uic[i].gcluster[i] != '\0') {
        nccell_prime(s->mplane[w], &c, &uic[i].gcluster[0], uic[i].stylemask, uic[i].channels);
        ncplane_putc(s->mplane[w], &c);
        i++;
    }
    return 0;
}
int ui_mvwadd_wchstr(UiSurface *s, int w, int y, int x, UiCell *uic) {
    if (!s)
        return -1;
    struct nccell c;
    int i = 0;
    ui_cursor_move(s, w, y, x);
    while (uic[i].gcluster[i] != '\0') {
        nccell_prime(s->mplane[w], &c, &uic[i].gcluster[0], uic[i].stylemask, uic[i].channels);
        ncplane_putc(s->mplane[w], &c);
        i++;
    }
    return 0;
}
int ui_wadd_wchnstr(UiSurface *s, int w, UiCell *uic, int n) {
    if (!s)
        return -1;
    struct nccell c;
    int i = 0;
    while (uic[i].gcluster[i] != '\0' && i < n) {
        nccell_prime(s->mplane[w], &c, &uic[i].gcluster[0], uic[i].stylemask, uic[i].channels);
        ncplane_putc(s->mplane[w], &c);
        i++;
    }
    return 0;
}
int ui_mvwadd_wchnstr(UiSurface *s, int w, int y, int x, UiCell *uic, int n) {
    if (!s)
        return -1;
    struct nccell c;
    int i = 0;
    ui_cursor_move(s, w, y, x);
    while (uic[i].gcluster[i] != '\0' && i < n) {
        nccell_prime(s->mplane[w], &c, &uic[i].gcluster[0], uic[i].stylemask, uic[i].channels);
        ncplane_putc(s->mplane[w], &c);
        i++;
    }
    return 0;
}
/* -------------------------------------------------------------------------
   Lines
   ------------------------------------------------------------------------- */

/** @brief Draw a horizontal line of length @p len at (y, x). */
int ui_draw_hline(UiSurface *s, int w, int y, int x, int len, const UiStyle *style) {
    if (!s || len <= 0)
        return -1;
    uint64_t channels = ui_notcurses_channels_from_style(style);
    uint32_t attrs = ui_notcurses_attrs_from_style(style);
    ncplane_set_channels(s->mplane[w], channels);
    ncplane_set_styles(s->mplane[w], attrs);
    for (int i = 0; i < len; i++)
        ncplane_putegc_yx(s->mplane[w], y, x + i, "\xe2\x94\x80", NULL); /* U+2500 ─ */
    return 0;
}

/** @brief Draw a vertical line of length @p len at (y, x). */
int ui_draw_vline(UiSurface *s, int w, int y, int x, int len, const UiStyle *style) {
    if (!s || len <= 0)
        return -1;
    uint64_t channels = ui_notcurses_channels_from_style(style);
    uint32_t attrs = ui_notcurses_attrs_from_style(style);
    ncplane_set_channels(s->mplane[w], channels);
    ncplane_set_styles(s->mplane[w], attrs);
    for (int i = 0; i < len; i++)
        ncplane_putegc_yx(s->mplane[w], y + i, x, "\xe2\x94\x82", NULL); /* U+2502 │ */
    return 0;
}

/* -------------------------------------------------------------------------
   Borders
   ------------------------------------------------------------------------- */

/** @brief Draw a border around the surface using @p kind style. */
int ui_draw_border(UiSurface *s, int w) {
    if (!s)
        return -1;
    uint64_t channels = ui_notcurses_channels_from_style(style_box);
    uint32_t attrs = ui_notcurses_attrs_from_style(style_box);
    ncplane_perimeter_rounded(s->mplane[w], attrs, channels, 0);
    return 0;
}
