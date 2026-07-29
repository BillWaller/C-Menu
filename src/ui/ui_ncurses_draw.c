/** @file ui_ncurses_draw.c
   @ingroup ui_ncurses
   @brief NCurses UI backend — drawing functions.

   Implements the drawing operations declared in ui_backend.h using the
   NCurses wide-character API.
*/

#include "ui_ncurses_internal.h"

/* -------------------------------------------------------------------------
   Internal style helpers
   ------------------------------------------------------------------------- */

int ui_ncurses_color_pair_from_style(const UiStyle *style) {
    (void)style;
    return 0;
}

int ui_ncurses_style_apply(WINDOW *win, const UiStyle *style) {
    if (!win || !style)
        return -1;
    attr_t attrs = 0;
    attrs |= style->bold      ? WA_BOLD      : 0;
    attrs |= style->dim       ? WA_DIM       : 0;
    attrs |= style->italic    ? WA_ITALIC    : 0;
    attrs |= style->underline ? WA_UNDERLINE : 0;
    attrs |= style->blink     ? WA_BLINK     : 0;
    attrs |= style->reverse   ? WA_REVERSE   : 0;
    attrs |= style->invis     ? WA_INVIS     : 0;
    wattr_set(win, attrs, 0, NULL);
    return 0;
}

/* -------------------------------------------------------------------------
   Surface style
   ------------------------------------------------------------------------- */

/** @brief Apply a style to a surface's default rendering attributes. */
int ui_surface_set_style(UiSurface *s, const UiStyle *style) {
    if (!s || !style)
        return -1;
    return ui_ncurses_style_apply(s->win, style);
}

/** @brief Set the background fill character and style for a surface. */
int ui_surface_set_base(UiSurface *s, const UiStyle *style, uint32_t fill_ch) {
    if (!s)
        return -1;
    if (style)
        ui_ncurses_style_apply(s->win, style);
    wbkgdset(s->win, (chtype)(fill_ch ? fill_ch : ' '));
    return 0;
}

/* -------------------------------------------------------------------------
   Text
   ------------------------------------------------------------------------- */

/** @brief Draw UTF-8 text at (y, x) with optional style. */
int ui_draw_text(UiSurface *s, int y, int x, const UiStyle *style,
                 const char *text) {
    if (!s || !text)
        return -1;
    if (style)
        ui_ncurses_style_apply(s->win, style);
    mvwaddstr(s->win, y, x, text);
    return 0;
}

/** @brief Draw at most @p n bytes of UTF-8 text at (y, x). */
int ui_draw_text_n(UiSurface *s, int y, int x, const UiStyle *style,
                   const char *text, size_t n) {
    if (!s || !text)
        return -1;
    if (style)
        ui_ncurses_style_apply(s->win, style);
    mvwaddnstr(s->win, y, x, text, (int)n);
    return 0;
}

/* -------------------------------------------------------------------------
   Lines
   ------------------------------------------------------------------------- */

/** @brief Draw a horizontal line of length @p len at (y, x). */
int ui_draw_hline(UiSurface *s, int y, int x, int len, const UiStyle *style) {
    if (!s)
        return -1;
    if (style)
        ui_ncurses_style_apply(s->win, style);
    mvwhline(s->win, y, x, 0, len);
    return 0;
}

/** @brief Draw a vertical line of length @p len at (y, x). */
int ui_draw_vline(UiSurface *s, int y, int x, int len, const UiStyle *style) {
    if (!s)
        return -1;
    if (style)
        ui_ncurses_style_apply(s->win, style);
    mvwvline(s->win, y, x, 0, len);
    return 0;
}

/* -------------------------------------------------------------------------
   Borders
   ------------------------------------------------------------------------- */

/** @brief Draw a border around the surface using @p kind style. */
int ui_draw_border(UiSurface *s, UiBorderKind kind, const UiStyle *style) {
    if (!s)
        return -1;
    if (style)
        ui_ncurses_style_apply(s->win, style);
    switch (kind) {
    case UI_BORDER_NONE:
        return 0;
    case UI_BORDER_ASCII:
        box(s->win, '|', '-');
        return 0;
    case UI_BORDER_ROUNDED: {
        /* Use Unicode rounded-corner box-drawing characters. */
        cchar_t tl, tr, bl, br, ho, ve;
        wchar_t wcs[2] = {0, 0};
        attr_t  a = 0;
        short   cp = 0;
        wcs[0] = L'\x256d'; setcchar(&tl, wcs, a, cp, NULL);
        wcs[0] = L'\x256e'; setcchar(&tr, wcs, a, cp, NULL);
        wcs[0] = L'\x2570'; setcchar(&bl, wcs, a, cp, NULL);
        wcs[0] = L'\x256f'; setcchar(&br, wcs, a, cp, NULL);
        wcs[0] = L'\x2500'; setcchar(&ho, wcs, a, cp, NULL);
        wcs[0] = L'\x2502'; setcchar(&ve, wcs, a, cp, NULL);
        wborder_set(s->win, &ve, &ve, &ho, &ho, &tl, &tr, &bl, &br);
        return 0;
    }
    case UI_BORDER_LIGHT:
    default:
        box(s->win, 0, 0);
        return 0;
    }
}

/** @brief Write @p title into the top border row at column @p x. */
int ui_draw_box_title(UiSurface *s, int x, const UiStyle *style,
                      const char *title) {
    if (!s || !title)
        return -1;
    if (style)
        ui_ncurses_style_apply(s->win, style);
    mvwaddstr(s->win, 0, x, title);
    return 0;
}
