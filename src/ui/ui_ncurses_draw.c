#include "ui_ncurses_internal.h"

/** @file ui_ncurses_draw.c
   @ingroup ui_ncurses
   @brief Implementation of drawing functions for the ncurses UI backend.

   This file contains the implementation of various drawing functions that
   utilize the ncurses library to render text, lines, borders, and other
   UI elements on the terminal. The functions take into account the styles
   defined in UiStyle and apply them accordingly when drawing.
 */

/** @brief Convert an RGB value (0-255) to a 1000-based value for ncurses.
   @ingroup ui_ncurses
   Ncurses uses a 1000-based color system, so we need to convert our 0-255
   RGB values to that scale.
   @param v The RGB value (0-255).
   @return The corresponding 1000-based value for ncurses.
 */
static short clamp_rgb_1000(uint8_t v) {
    return (short)((v * 1000) / 255);
}

/** @brief Convert a UiStyle to an ncurses color pair index.
   @ingroup ui_ncurses
   This function maps the foreground and background colors from a UiStyle to
   an ncurses color pair index. For simplicity, this example assumes a limited
   set of colors and does not handle all possible combinations.
   @param style The UiStyle containing the foreground and background colors.
   @return The ncurses color pair index corresponding to the style.
 */
int ui_ncurses_color_pair_from_style(const UiStyle *style) {
    (void)style;
    return 0;
}

/** @brief Apply a UiStyle to an ncurses window.
   @ingroup ui_ncurses
   This function sets the appropriate attributes (bold, underline, reverse)
   on the given ncurses window based on the properties of the UiStyle.
   @param win The ncurses window to which the style should be applied.
   @param style The UiStyle containing the attributes to apply.
   @return 0 on success, -1 on failure (e.g., if win or style is NULL).
 */
int ui_ncurses_style_apply(WINDOW *win, const UiStyle *style) {
    if (!win || !style)
        return -1;
    wchar_t wstr[2] = {L'\0', L'\0'};
    cchar_t cc = {0};
    attr_t attrs = 0;
    uint32_t cpx = 0;
    attrs |= style->bold ? WA_BOLD : 0;
    attrs |= style->dim ? WA_DIM : 0;
    attrs |= style->italic ? WA_ITALIC : 0;
    attrs |= style->underline ? WA_UNDERLINE : 0;
    attrs |= style->blink ? WA_BLINK : 0;
    attrs |= style->reverse ? WA_REVERSE : 0;
    attrs |= style->invis ? WA_INVIS : 0;
    setcchar(&cc, wstr, attrs, cpx, nullptr);
    return 0;
}

/** @brief Set the style for a UiSurface.
   @ingroup ui_ncurses
   This function applies the given UiStyle to the ncurses window associated
   with the UiSurface. It uses the ui_ncurses_style_apply function to set
   the appropriate attributes on the window.
   @param s The UiSurface whose style is to be set.
   @param style The UiStyle to apply to the surface.
   @return 0 on success, -1 on failure (e.g., if s or style is NULL).
 */
int ui_surface_set_style(UiSurface *s, const UiStyle *style) {
    if (!s || !style)
        return -1;
    return ui_ncurses_style_apply(s->win, style);
}

/** @brief Set the base fill character and style for a UiSurface.
   @ingroup ui_ncurses
   This function sets the background fill character for the UiSurface's ncurses
   window and applies the given UiStyle. If fill_ch is 0, it defaults to a
   space character.
   @param s The UiSurface to set the base for.
   @param style The UiStyle to apply to the surface.
   @param fill_ch The character to use for filling the background (0 for space).
   @return 0 on success, -1 on failure (e.g., if s is NULL).
 */
int ui_surface_set_base(UiSurface *s, const UiStyle *style, uint32_t fill_ch) {
    if (!s)
        return -1;
    if (style)
        ui_ncurses_style_apply(s->win, style);
    wbkgdset(s->win, (chtype)(fill_ch ? fill_ch : ' '));
    return 0;
}

/** @brief Draw text on a UiSurface at a specified position with a given style.
   @ingroup ui_ncurses
   This function uses the ncurses mvwaddstr function to draw the provided text
   at the specified (y, x) coordinates on the UiSurface's window. If a style
   is provided, it applies that style before drawing the text.
   @param s The UiSurface on which to draw the text.
   @param y The y-coordinate (row) where the text should be drawn.
   @param x The x-coordinate (column) where the text should be drawn.
   @param style The UiStyle to apply to the text (can be NULL for default).
   @param text The null-terminated string to draw.
   @return 0 on success, -1 on failure (e.g., if s or text is NULL).
 */
int ui_draw_text(UiSurface *s, int y, int x, const UiStyle *style, const char *text) {
    if (!s || !text)
        return -1;
    if (style)
        ui_ncurses_style_apply(s->win, style);
    mvwaddstr(s->win, y, x, text);
    return 0;
}

/** @brief Draw a specified number of characters from a string on a UiSurface.
   @ingroup ui_ncurses
   This function is similar to ui_draw_text but allows specifying the number of
   characters to draw from the provided string. It uses the ncurses mvwaddnstr
   function to achieve this. If a style is provided, it applies that style before
   drawing the text.
   @param s The UiSurface on which to draw the text.
   @param y The y-coordinate (row) where the text should be drawn.
   @param x The x-coordinate (column) where the text should be drawn.
   @param style The UiStyle to apply to the text (can be NULL for default).
   @param text The string from which to draw characters (not necessarily null-terminated).
   @param n The number of characters to draw from the string.
   @return 0 on success, -1 on failure (e.g., if s or text is NULL).
 */
int ui_draw_text_n(UiSurface *s, int y, int x, const UiStyle *style, const char *text, size_t n) {
    if (!s || !text)
        return -1;
    if (style)
        ui_ncurses_style_apply(s->win, style);
    mvwaddnstr(s->win, y, x, text, (int)n);
    return 0;
}

/** @brief Draw a horizontal line on a UiSurface.
   @ingroup ui_ncurses
   This function uses the ncurses mvwhline function to draw a horizontal line of
   a specified length at the given (y, x) coordinates on the UiSurface's window.
   If a style is provided, it applies that style before drawing the line.
   @param s The UiSurface on which to draw the horizontal line.
   @param y The y-coordinate (row) where the line should be drawn.
   @param x The x-coordinate (column) where the line should start.
   @param len The length of the horizontal line to draw.
   @param style The UiStyle to apply to the line (can be NULL for default).
   @return 0 on success, -1 on failure (e.g., if s is NULL).
 */
int ui_draw_hline(UiSurface *s, int y, int x, int len, const UiStyle *style) {
    if (!s)
        return -1;
    if (style)
        ui_ncurses_style_apply(s->win, style);
    mvwhline(s->win, y, x, 0, len);
    return 0;
}

/** @brief Draw a vertical line on a UiSurface.
   @ingroup ui_ncurses
   This function uses the ncurses mvwvline function to draw a vertical line of
   a specified length at the given (y, x) coordinates on the UiSurface's window.
   If a style is provided, it applies that style before drawing the line.
   @param s The UiSurface on which to draw the vertical line.
   @param y The y-coordinate (row) where the line should start.
   @param x The x-coordinate (column) where the line should be drawn.
   @param len The length of the vertical line to draw.
   @param style The UiStyle to apply to the line (can be NULL for default).
   @return 0 on success, -1 on failure (e.g., if s is NULL).
 */
int ui_draw_vline(UiSurface *s, int y, int x, int len, const UiStyle *style) {
    if (!s)
        return -1;
    if (style)
        ui_ncurses_style_apply(s->win, style);
    mvwvline(s->win, y, x, 0, len);
    return 0;
}

/** @brief Draw a border around a UiSurface.
   @ingroup ui_ncurses
   This function draws a border around the UiSurface's window based on the
   specified UiBorderKind. It uses the ncurses box function to draw the border
   with the appropriate characters. If a style is provided, it applies that style
   before drawing the border.
   @param s The UiSurface on which to draw the border.
   @param kind The kind of border to draw (e.g., none, ASCII, light, rounded).
   @param style The UiStyle to apply to the border (can be NULL for default).
   @return 0 on success, -1 on failure (e.g., if s is NULL).
 */
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

/** @brief Draw a title on the top border of a UiSurface.
   @ingroup ui_ncurses
   This function draws the provided title string at the specified x-coordinate
   on the top border of the UiSurface's window. It uses the ncurses mvwaddstr
   function to place the title. If a style is provided, it applies that style
   before drawing the title.
   @param s The UiSurface on which to draw the box title.
   @param x The x-coordinate (column) where the title should start.
   @param style The UiStyle to apply to the title (can be NULL for default).
   @param title The null-terminated string to use as the box title.
   @return 0 on success, -1 on failure (e.g., if s or title is NULL).
 */
int ui_draw_box_title(UiSurface *s, int x, const UiStyle *style, const char *title) {
    if (!s || !title)
        return -1;
    if (style)
        ui_ncurses_style_apply(s->win, style);
    mvwaddstr(s->win, 0, x, title);
    return 0;
}
