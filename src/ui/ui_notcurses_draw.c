/** @file ui_notcurses_draw.c
   @ingroup ui_notcurses
   @brief NotCurses UI backend — drawing functions.

   Implements the drawing operations declared in ui_backend.h using the
   NotCurses API.
*/
#define _GNU_SOURCE
#define _XOPEN_SOURCE 700
#define _XOPEN_SOURCE_EXTENDED 1

#include "ui_notcurses_internal.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

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
// Complex Characters (cc)
// typedef struct {
//     wchar_t wc;       // Wide character    4-bytes
//     short attr;       // attributes        2-bytes
//     short color_pair; // color pair index  2-bytes
// } UiCchar64;          //           total   8-bytes
//
// typedef struct { // 16-bytes
//     wchar_t wc;  //  4-bytes
//     int attrs;   //  4-bytes
//     UiColor fg;  //  4-bytes
//     UiColor bg;  //  4-bytes
// } UiStyle;
//
// ---------------------------------------------------------------------------
// uicc character single
int ui_wadd_wch(UiSurface *s, int w, const UiCchar128 uicc) {
    UiStyle *style = calloc(1, sizeof(UiStyle));
    if (!s)
        return -1;
    style->attrs = uicc.attrs;
    style->fg.rgba = uicc.fg.rgba;
    style->bg.rgba = uicc.bg.rgba;
    ncplane_set_channels(s->mplane[w], ui_notcurses_channels_from_style(style));
    ncplane_set_styles(s->mplane[w], ui_notcurses_attrs_from_style(style));
    ncplane_putwc(s->mplane[w], uicc.wc);
    free(style);
    return 0;
}
int ui_mvwadd_wch(UiSurface *s, int w, int y, int x, const UiCchar128 uicc) {
    UiStyle *style = calloc(1, sizeof(UiStyle));
    if (!s)
        return -1;
    ui_cursor_move(s, w, y, x);
    style->attrs = uicc.attrs;
    style->fg.rgba = uicc.fg.rgba;
    style->bg.rgba = uicc.bg.rgba;
    ncplane_set_channels(s->mplane[w], ui_notcurses_channels_from_style(style));
    ncplane_set_styles(s->mplane[w], ui_notcurses_attrs_from_style(style));
    ncplane_putwc(s->mplane[w], uicc.wc);
    free(style);
    return 0;
}
//  uicc string
int ui_wadd_wchstr(UiSurface *s, int w, const UiCchar128 *uicc) {
    UiStyle *style = calloc(1, sizeof(UiStyle));
    if (!s || !uicc)
        return -1;
    int i = 0;
    while (uicc[i].wc != L'\0') {
        if (uicc[0].attrs != style->attrs ||
            uicc[0].fg.rgba != style->fg.rgba ||
            uicc[0].bg.rgba != style->bg.rgba ||
            i == 0) {
            style->attrs = uicc[0].attrs;
            style->fg.rgba = uicc[0].fg.rgba;
            style->bg.rgba = uicc[0].bg.rgba;
            ncplane_set_channels(s->mplane[w], ui_notcurses_channels_from_style(style));
            ncplane_set_styles(s->mplane[w], ui_notcurses_attrs_from_style(style));
        }
        ncplane_putwc(s->mplane[w], uicc[i].wc);
        i++;
    }
    free(style);
    return 0;
}
int ui_mvwadd_wchstr(UiSurface *s, int w, int y, int x, const UiCchar128 *uicc) {
    UiStyle *style = calloc(1, sizeof(UiStyle));
    if (!s || !uicc)
        return -1;
    ui_cursor_move(s, w, y, x);
    int i = 0;
    while (uicc[i].wc != L'\0') {
        if (uicc[0].attrs != style->attrs ||
            uicc[0].fg.rgba != style->fg.rgba ||
            uicc[0].bg.rgba != style->bg.rgba ||
            i == 0) {
            style->attrs = uicc[0].attrs;
            style->fg.rgba = uicc[0].fg.rgba;
            style->bg.rgba = uicc[0].bg.rgba;
            ncplane_set_channels(s->mplane[w], ui_notcurses_channels_from_style(style));
            ncplane_set_styles(s->mplane[w], ui_notcurses_attrs_from_style(style));
        }
        ncplane_putwc(s->mplane[w], uicc[i].wc);
        i++;
    }
    free(style);
    return 0;
}
int ui_mvwadd_wchnstr(UiSurface *s, int w, int y, int x, const UiCchar128 *uicc, int count) {
    UiStyle *style = calloc(1, sizeof(UiStyle));
    if (!s || !uicc || count <= 0)
        return -1;
    ui_cursor_move(s, w, y, x);
    for (int i = 0; i < count; i++) {
        if (uicc[0].attrs != style->attrs ||
            uicc[0].fg.rgba != style->fg.rgba ||
            uicc[0].bg.rgba != style->bg.rgba ||
            i == 0) {
            style->attrs = uicc[0].attrs;
            style->fg.rgba = uicc[0].fg.rgba;
            style->bg.rgba = uicc[0].bg.rgba;
            ncplane_set_channels(s->mplane[w], ui_notcurses_channels_from_style(style));
            ncplane_set_styles(s->mplane[w], ui_notcurses_attrs_from_style(style));
        }
        ncplane_putwc(s->mplane[w], uicc[i].wc);
    }
    free(style);
    return 0;
}
bool ui_style_from_uicc(const UiCchar128 *uicc, UiStyle *style) {
    if (!uicc || !style)
        return false;
    style->attrs = uicc->attrs;
    style->fg.rgba = uicc->fg.rgba;
    style->bg.rgba = uicc->bg.rgba;
    return true;
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
int ui_draw_border(UiSurface *s, int w, UiBorderKind kind, const UiStyle *style) {
    if (!s)
        return -1;
    uint64_t channels = ui_notcurses_channels_from_style(style);
    uint32_t attrs = ui_notcurses_attrs_from_style(style);
    switch (kind) {
    case UI_BORDER_NONE:
        return 0;
    case UI_BORDER_ASCII: {
        /* Draw ASCII border manually. */
        unsigned int lines, cols;
        ncplane_dim_yx(s->mplane[w], &lines, &cols);
        ncplane_putegc_yx(s->mplane[w], 0, 0, "+", NULL);
        ncplane_putegc_yx(s->mplane[w], 0, (int)cols - 1, "+", NULL);
        ncplane_putegc_yx(s->mplane[w], (int)lines - 1, 0, "+", NULL);
        ncplane_putegc_yx(s->mplane[w], (int)lines - 1, (int)cols - 1, "+", NULL);
        for (unsigned int c = 1; c < cols - 1; c++) {
            ncplane_putegc_yx(s->mplane[w], 0, (int)c, "-", NULL);
            ncplane_putegc_yx(s->mplane[w], (int)lines - 1, (int)c, "-", NULL);
        }
        for (unsigned int r = 1; r < lines - 1; r++) {
            ncplane_putegc_yx(s->mplane[w], (int)r, 0, "|", NULL);
            ncplane_putegc_yx(s->mplane[w], (int)r, (int)cols - 1, "|", NULL);
        }
        return 0;
    }
    case UI_BORDER_ROUNDED:
        ncplane_perimeter_rounded(s->mplane[w], attrs, channels, 0);
        return 0;
    case UI_BORDER_LIGHT:
    default:
        ncplane_perimeter_double(s->mplane[w], attrs, channels, 0);
        return 0;
    }
}

/** @brief Write @p title into the top border row at column @p x. */
int ui_draw_box_title(UiSurface *s, int w, int x, const UiStyle *style,
                      const char *title) {
    if (!s || !title)
        return -1;
    if (style) {
        ncplane_set_channels(s->mplane[w], ui_notcurses_channels_from_style(style));
        ncplane_set_styles(s->mplane[w], ui_notcurses_attrs_from_style(style));
    }
    ncplane_putstr_yx(s->mplane[w], 0, x, title);
    return 0;
}
