/** @file ui_notcurses_draw.c
   @ingroup ui_notcurses
   @brief NotCurses UI backend — drawing functions.

   Implements the drawing operations declared in ui_backend.h using the
   NotCurses API.
*/

#include "ui_notcurses_internal.h"
#include <string.h>

/* -------------------------------------------------------------------------
   Text
   ------------------------------------------------------------------------- */

/** @brief Draw UTF-8 text at (y, x) with optional style. */
int ui_draw_text(UiSurface *s, int y, int x, const UiStyle *style,
                 const char *text) {
    if (!s || !text)
        return -1;
    if (style) {
        ncplane_set_channels(s->plane, ui_notcurses_channels_from_style(style));
        ncplane_set_styles(s->plane, ui_notcurses_attrs_from_style(style));
    }
    ncplane_putstr_yx(s->plane, y, x, text);
    return 0;
}

/** @brief Draw at most @p n bytes of UTF-8 text at (y, x). */
int ui_draw_text_n(UiSurface *s, int y, int x, const UiStyle *style,
                   const char *text, size_t n) {
    if (!s || !text)
        return -1;
    if (style) {
        ncplane_set_channels(s->plane, ui_notcurses_channels_from_style(style));
        ncplane_set_styles(s->plane, ui_notcurses_attrs_from_style(style));
    }
    ncplane_putnstr_yx(s->plane, y, x, n, text);
    return 0;
}

/* -------------------------------------------------------------------------
   Lines
   ------------------------------------------------------------------------- */

/** @brief Draw a horizontal line of length @p len at (y, x). */
int ui_draw_hline(UiSurface *s, int y, int x, int len, const UiStyle *style) {
    if (!s || len <= 0)
        return -1;
    uint64_t channels = ui_notcurses_channels_from_style(style);
    uint32_t attrs    = ui_notcurses_attrs_from_style(style);
    ncplane_set_channels(s->plane, channels);
    ncplane_set_styles(s->plane, attrs);
    for (int i = 0; i < len; i++)
        ncplane_putegc_yx(s->plane, y, x + i, "\xe2\x94\x80", NULL); /* U+2500 ─ */
    return 0;
}

/** @brief Draw a vertical line of length @p len at (y, x). */
int ui_draw_vline(UiSurface *s, int y, int x, int len, const UiStyle *style) {
    if (!s || len <= 0)
        return -1;
    uint64_t channels = ui_notcurses_channels_from_style(style);
    uint32_t attrs    = ui_notcurses_attrs_from_style(style);
    ncplane_set_channels(s->plane, channels);
    ncplane_set_styles(s->plane, attrs);
    for (int i = 0; i < len; i++)
        ncplane_putegc_yx(s->plane, y + i, x, "\xe2\x94\x82", NULL); /* U+2502 │ */
    return 0;
}

/* -------------------------------------------------------------------------
   Borders
   ------------------------------------------------------------------------- */

/** @brief Draw a border around the surface using @p kind style. */
int ui_draw_border(UiSurface *s, UiBorderKind kind, const UiStyle *style) {
    if (!s)
        return -1;
    uint64_t channels = ui_notcurses_channels_from_style(style);
    uint32_t attrs    = ui_notcurses_attrs_from_style(style);
    switch (kind) {
    case UI_BORDER_NONE:
        return 0;
    case UI_BORDER_ASCII: {
        /* Draw ASCII border manually. */
        unsigned int rows, cols;
        ncplane_dim_yx(s->plane, &rows, &cols);
        ncplane_putegc_yx(s->plane, 0, 0, "+", NULL);
        ncplane_putegc_yx(s->plane, 0, (int)cols - 1, "+", NULL);
        ncplane_putegc_yx(s->plane, (int)rows - 1, 0, "+", NULL);
        ncplane_putegc_yx(s->plane, (int)rows - 1, (int)cols - 1, "+", NULL);
        for (unsigned int c = 1; c < cols - 1; c++) {
            ncplane_putegc_yx(s->plane, 0, (int)c, "-", NULL);
            ncplane_putegc_yx(s->plane, (int)rows - 1, (int)c, "-", NULL);
        }
        for (unsigned int r = 1; r < rows - 1; r++) {
            ncplane_putegc_yx(s->plane, (int)r, 0, "|", NULL);
            ncplane_putegc_yx(s->plane, (int)r, (int)cols - 1, "|", NULL);
        }
        return 0;
    }
    case UI_BORDER_ROUNDED:
        ncplane_perimeter_rounded(s->plane, attrs, channels, 0);
        return 0;
    case UI_BORDER_LIGHT:
    default:
        ncplane_perimeter_double(s->plane, attrs, channels, 0);
        return 0;
    }
}

/** @brief Write @p title into the top border row at column @p x. */
int ui_draw_box_title(UiSurface *s, int x, const UiStyle *style,
                      const char *title) {
    if (!s || !title)
        return -1;
    if (style) {
        ncplane_set_channels(s->plane, ui_notcurses_channels_from_style(style));
        ncplane_set_styles(s->plane, ui_notcurses_attrs_from_style(style));
    }
    ncplane_putstr_yx(s->plane, 0, x, title);
    return 0;
}
