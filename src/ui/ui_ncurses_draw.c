#include "ui_ncurses_internal.h"

static short clamp_rgb_1000(uint8_t v) {
    return (short)((v * 1000) / 255);
}

int ui_ncurses_color_pair_from_style(const UiStyle *style) {
    (void)style;
    return 0;
}

int ui_ncurses_style_apply(WINDOW *win, const UiStyle *style) {
    if (!win || !style)
        return -1;

    wattrset(win, A_NORMAL);

    if (style->bold)
        wattron(win, A_BOLD);
    if (style->underline)
        wattron(win, A_UNDERLINE);
    if (style->reverse)
        wattron(win, A_REVERSE);

    return 0;
}

int ui_surface_set_style(UiSurface *s, const UiStyle *style) {
    if (!s || !style)
        return -1;
    return ui_ncurses_style_apply(s->win, style);
}

int ui_surface_set_base(UiSurface *s, const UiStyle *style, uint32_t fill_ch) {
    if (!s)
        return -1;
    if (style)
        ui_ncurses_style_apply(s->win, style);
    wbkgdset(s->win, (chtype)(fill_ch ? fill_ch : ' '));
    return 0;
}

int ui_draw_text(UiSurface *s, int y, int x, const UiStyle *style, const char *text) {
    if (!s || !text)
        return -1;
    if (style)
        ui_ncurses_style_apply(s->win, style);
    mvwaddstr(s->win, y, x, text);
    return 0;
}

int ui_draw_text_n(UiSurface *s, int y, int x, const UiStyle *style, const char *text, size_t n) {
    if (!s || !text)
        return -1;
    if (style)
        ui_ncurses_style_apply(s->win, style);
    mvwaddnstr(s->win, y, x, text, (int)n);
    return 0;
}

int ui_draw_hline(UiSurface *s, int y, int x, int len, const UiStyle *style) {
    if (!s)
        return -1;
    if (style)
        ui_ncurses_style_apply(s->win, style);
    mvwhline(s->win, y, x, 0, len);
    return 0;
}

int ui_draw_vline(UiSurface *s, int y, int x, int len, const UiStyle *style) {
    if (!s)
        return -1;
    if (style)
        ui_ncurses_style_apply(s->win, style);
    mvwvline(s->win, y, x, 0, len);
    return 0;
}

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
    case UI_BORDER_LIGHT:
    case UI_BORDER_ROUNDED:
    default:
        box(s->win, 0, 0);
        return 0;
    }
}

int ui_draw_box_title(UiSurface *s, int x, const UiStyle *style, const char *title) {
    if (!s || !title)
        return -1;
    if (style)
        ui_ncurses_style_apply(s->win, style);
    mvwaddstr(s->win, 0, x, title);
    return 0;
}
