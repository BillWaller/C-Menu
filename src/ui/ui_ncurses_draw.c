/** @file ui_ncurses_draw.c
   @ingroup ui_ncurses
   @brief NCurses UI backend — drawing functions.

   Implements the drawing operations declared in ui_backend.h using the
   NCurses wide-character API.
*/

#define _XOPEN_SOURCE_EXTENDED 1

#include "cm.h"
#include "ui_backend.h"
#include "ui_ncurses_internal.h"
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

/* -------------------------------------------------------------------------
   Surface style
   ------------------------------------------------------------------------- */
/** @brief Set the style of a surface window.
    @param s The surface to modify.
    @param w The window index within the surface.
    @param style The style to apply (UiStyle).
    @return 0 on success, -1 on error.
*/
int ui_wclrtobot(UiSurface *s, ss_t w) {
    if (!s)
        return -1;
    wclrtobot(s->mwin[w]);
    return 0;
}
/** @brief Clear to the end of the line in a surface window.
    @param s The surface to modify.
    @param w The window index within the surface.
    @return 0 on success, -1 on error.
*/
int ui_wclrtoeol(UiSurface *s, ss_t w) {
    if (!s)
        return -1;
    wclrtoeol(s->mwin[w]);
    return 0;
}
/* -------------------------------------------------------------------------
   Text with UiStyle
   ------------------------------------------------------------------------- */
/** @brief Draw a character in a surface window.
    @param s The surface to modify.
    @param w The window index within the surface.
    @param c The character to draw.
    @return 0 on success, -1 on error.
*/
int ui_draw_ch(UiSurface *s, ss_t w, const char c) {
    if (!s)
        return -1;
    waddch(s->mwin[w], c);
    return 0;
}
/** @brief Draw a character at a specific position in a surface window.
    @param s The surface to modify.
    @param w The window index within the surface.
    @param y The y-coordinate (row) to draw the character.
    @param x The x-coordinate (column) to draw the character.
    @param c The character to draw.
    @return 0 on success, -1 on error.
*/
int ui_draw_ch_yx(UiSurface *s, ss_t w, uint y, uint x, const char c) {
    if (!s)
        return -1;
    mvwaddch(s->mwin[w], y, x, c);
    return 0;
}
//  text string
int ui_draw_text(UiSurface *s, ss_t w, uint y, uint x, const char *text) {
    if (!s || !text)
        return -1;
    mvwaddstr(s->mwin[w], y, x, text);
    return 0;
}
/** @brief Draw a text string at a specific position in a surface window.
    @param s The surface to modify.
    @param w The window index within the surface.
    @param y The y-coordinate (row) to draw the text.
    @param x The x-coordinate (column) to draw the text.
    @param text The text string to draw.
    @return 0 on success, -1 on error.
*/
int ui_draw_text_n(UiSurface *s, ss_t w, uint y, uint x, const char *text, int n) {
    if (!s || !text)
        return -1;
    mvwaddnstr(s->mwin[w], y, x, text, (int)n);
    return 0;
}
/** @brief Draw a text string at a specific position in a surface window, filling the remaining space with spaces if the string is shorter than n.
    @param s The surface to modify.
    @param w The window index within the surface.
    @param y The y-coordinate (row) to draw the text.
    @param x The x-coordinate (column) to draw the text.
    @param text The text string to draw.
    @param n The number of characters to draw, padding with spaces if necessary.
    @return 0 on success, -1 on error.
*/
int ui_draw_text_fill(UiSurface *s, ss_t w, uint y, uint x, const char *text, int n) {
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
        mvwaddstr(s->mwin[w], y, x, tmp_str);
        free(tmp_str);
    } else {
        mvwaddnstr(s->mwin[w], y, x, text, (int)n);
    }
    return 0;
}
// -------------------------------------------------------------------------
// Text Characters
// -------------------------------------------------------------------------
/** @brief Draw a character in a surface window.
    @param s The surface to modify.
    @param w The window index within the surface.
    @param c The character to draw.
    @return 0 on success, -1 on error.
*/
int ui_mvwaddch(UiSurface *s, ss_t w, uint y, uint x, const char c) {
    if (!s)
        return -1;
    mvwaddch(s->mwin[w], y, x, c);
    return 0;
}
// -------------------------------------------------------------------------
// Text Strings
// -------------------------------------------------------------------------
/** @brief Draw a text string in a surface window.
    @param s The surface to modify.
    @param w The window index within the surface.
    @param text The text string to draw.
    @return 0 on success, -1 on error.
*/
int ui_waddstr(UiSurface *s, ss_t w, const char *text) {
    if (!s || !text)
        return -1;
    waddstr(s->mwin[w], text);
    return 0;
}
/** @brief Draw a text string in a surface window, with a maximum length.
    @param s The surface to modify.
    @param w The window index within the surface.
    @param text The text string to draw.
    @param m The maximum number of characters to draw from the text string.
    @return 0 on success, -1 on error.
*/
int ui_waddnstr(UiSurface *s, ss_t w, const char *text, int m) {
    if (!s || !text)
        return -1;
    waddnstr(s->mwin[w], text, m);
    return 0;
}
/** @brief Draw a text string at a specific position in a surface window.
    @param s The surface to modify.
    @param w The window index within the surface.
    @param y The y-coordinate (row) to draw the text.
    @param x The x-coordinate (column) to draw the text.
    @param text The text string to draw.
    @return 0 on success, -1 on error.
*/
int ui_mvwaddstr(UiSurface *s, ss_t w, uint y, uint x, const char *text) {
    if (!s || !text)
        return -1;
    mvwaddstr(s->mwin[w], y, x, text);
    return 0;
}
/** @brief Draw a text string at a specific position in a surface window, with a maximum length.
    @param s The surface to modify.
    @param w The window index within the surface.
    @param y The y-coordinate (row) to draw the text.
    @param x The x-coordinate (column) to draw the text.
    @param text The text string to draw.
    @param m The maximum number of characters to draw from the text string.
    @return 0 on success, -1 on error.
*/
int ui_mvwaddnstr(UiSurface *s, ss_t w, uint y, uint x, const char *text, int m) {
    if (!s || !text)
        return -1;
    mvwaddnstr(s->mwin[w], y, x, text, m);
    return 0;
}
/** @brief Draw a text string at a specific position in a surface window, filling the remaining space with spaces if the string is shorter than m.
    @param s The surface to modify.
    @param w The window index within the surface.
    @param y The y-coordinate (row) to draw the text.
    @param x The x-coordinate (column) to draw the text.
    @param str The text string to draw.
    @param m The number of characters to draw, padding with spaces if necessary.
    @return 0 on success, -1 on error.
*/
int ui_mvwaddstr_fill(UiSurface *s, ss_t w, uint y, uint x, const char *str, int m) {
    char *d, *e;
    uint maxy, maxx;
    char tmp_str[MAXLEN];
    getmaxyx(s->mwin[w], maxy, maxx);
    y = min(y, maxy);
    m = min(m, maxx);
    m = min(m, (uint)MAXLEN - 1);
    e = d = tmp_str;
    e += m;
    while (d < e) {
        if (*str == '\0' || *str == '\n')
            *d++ = ' ';
        else
            *d++ = *str++;
    }
    *d = '\0';
    m = strlen(tmp_str);
    mvwaddnstr(s->mwin[w], y, x, tmp_str, m);
    return 0;
}
// -------------------------------------------------------------------------
// Wide Character Strings
// ---------------------------------------------------------------------------
/** @brief Draw a wide-character string in a surface window.
    @param s The surface to modify.
    @param w The window index within the surface.
    @param wstr The wide-character string to draw.
    @return 0 on success, -1 on error.
*/
int ui_waddwstr(UiSurface *s, ss_t w, const wchar_t *wstr) {
    if (!s || !wstr)
        return -1;
    waddwstr(s->mwin[w], wstr);
    return 0;
}
/** @brief Draw a wide-character string at a specific position in a surface window.
    @param s The surface to modify.
    @param w The window index within the surface.
    @param y The y-coordinate (row) to draw the wide-character string.
    @param x The x-coordinate (column) to draw the wide-character string.
    @param wstr The wide-character string to draw.
    @return 0 on success, -1 on error.
*/
int ui_mvwaddwstr(UiSurface *s, ss_t w, uint y, uint x, const wchar_t *wstr) {
    if (!s || !wstr)
        return -1;
    mvwaddwstr(s->mwin[w], y, x, wstr);
    return 0;
}
/** @brief Draw a wide-character string in a surface window, with a maximum length.
    @param s The surface to modify.
    @param w The window index within the surface.
    @param wstr The wide-character string to draw.
    @param m The maximum number of wide characters to draw from the wide-character string.
    @return 0 on success, -1 on error.
*/
int ui_waddnwstr(UiSurface *s, ss_t w, const wchar_t *wstr, int m) {
    if (!s || !wstr)
        return -1;
    waddnwstr(s->mwin[w], wstr, m);
    return 0;
}
/** @brief Draw a wide-character string at a specific position in a surface window, with a maximum length.
    @param s The surface to modify.
    @param w The window index within the surface.
    @param y The y-coordinate (row) to draw the wide-character string.
    @param x The x-coordinate (column) to draw the wide-character string.
    @param wstr The wide-character string to draw.
    @param m The maximum number of wide characters to draw from the wide-character string.
    @return 0 on success, -1 on error.
*/
int ui_mvwaddnwstr(UiSurface *s, ss_t w, uint y, uint x, const wchar_t *wstr, int m) {
    if (!s || !wstr)
        return -1;
    mvwaddnwstr(s->mwin[w], y, x, wstr, m);
    return 0;
}
// -------------------------------------------------------------------------
// Complex Characters (cc)
// ---------------------------------------------------------------------------
/** @brief Draw a complex character in a surface window.
    @param s The surface to modify.
    @param w The window index within the surface.
    @param cell The complex character (UiCell) to draw.
    @return 0 on success, -1 on error.
*/
int ui_wadd_wch(UiSurface *s, ss_t w, const UiCell *cell) {
    if (!s)
        return -1;
    wadd_wchnstr(s->mwin[w], cell, 1);
    return 0;
}
/** @brief Draw a complex character at a specific position in a surface window.
    @param s The surface to modify.
    @param w The window index within the surface.
    @param y The y-coordinate (row) to draw the complex character.
    @param x The x-coordinate (column) to draw the complex character.
    @param cell The complex character (UiCell) to draw.
    @return 0 on success, -1 on error.
*/
int ui_mvwadd_wch(UiSurface *s, ss_t w, uint y, uint x, const UiCell *cell) {
    if (!s)
        return -1;
    mvwadd_wch(s->mwin[w], y, x, cell);
    return 0;
}
/** @brief Draw a complex character string in a surface window.
    @param s The surface to modify.
    @param w The window index within the surface.
    @param cmplx_buf The complex character string (array of UiCell) to draw.
    @return 0 on success, -1 on error.
*/
int ui_wadd_wchstr(UiSurface *s, ss_t w, const UiCell *cmplx_buf) {
    if (!s || !cmplx_buf)
        return -1;
    wadd_wchstr(s->mwin[w], cmplx_buf);
    return 0;
}
/** @brief Draw a complex character string at a specific position in a surface window.
    @param s The surface to modify.
    @param w The window index within the surface.
    @param y The y-coordinate (row) to draw the complex character string.
    @param x The x-coordinate (column) to draw the complex character string.
    @param cmplx_buf The complex character string (array of UiCell) to draw.
    @return 0 on success, -1 on error.
 */
int ui_mvwadd_wchstr(UiSurface *s, ss_t w, uint y, uint x, const UiCell *cmplx_buf) {
    if (!s || !cmplx_buf)
        return -1;
    mvwadd_wchstr(s->mwin[w], y, x, cmplx_buf);
    return 0;
}
/** @brief Draw a complex character string in a surface window, with a maximum length.
    @param s The surface to modify.
    @param w The window index within the surface.
    @param cmplx_buf The complex character string (array of UiCell) to draw.
    @param n The maximum number of complex characters to draw from the complex character string.
    @return 0 on success, -1 on error.
*/
int ui_wadd_wchnstr(UiSurface *s, ss_t w, const UiCell *cmplx_buf, uint n) {
    if (!s || !cmplx_buf)
        return -1;
    wadd_wchnstr(s->mwin[w], cmplx_buf, n);
    return 0;
}
/** @brief Draw a complex character string at a specific position in a surface window, with a maximum length.
    @param s The surface to modify.
    @param w The window index within the surface.
    @param y The y-coordinate (row) to draw the complex character string.
    @param x The x-coordinate (column) to draw the complex character string.
    @param cmplx_buf The complex character string (array of UiCell) to draw.
    @param n The maximum number of complex characters to draw from the complex character string.
    @return 0 on success, -1 on error.
*/
int ui_mvwadd_wchnstr(UiSurface *s, ss_t w, uint y, uint x, const UiCell *cmplx_buf, uint n) {
    if (!s || !cmplx_buf)
        return -1;
    mvwadd_wchnstr(s->mwin[w], y, x, cmplx_buf, n);
    return 0;
}
/* -------------------------------------------------------------------------
   Screen management
   ------------------------------------------------------------------------- */
/** @brief Restore the state of all windows in the UI.
    This function refreshes the standard screen and all surface windows, ensuring that their contents are displayed correctly.
*/
void ui_restore_wins() {
    touchwin(stdscr);
    for (int s = 0; s <= sfc_ptr; s++) {
        for (ss_t w = BOX; w < SUB_SFC_MAX; w++)
            if (ui_surface[s]->mwin[w] != nullptr)
                touchwin(ui_surface[s]->mwin[w]);
    }
    ui_render();
}
