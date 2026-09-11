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

int ui_wclrtobot(UiSurface *s, ss_t w) {
    if (!s)
        return -1;
    wclrtobot(s->mwin[w]);
    return 0;
}
int ui_wclrtoeol(UiSurface *s, ss_t w) {
    if (!s)
        return -1;
    wclrtoeol(s->mwin[w]);
    return 0;
}
/* -------------------------------------------------------------------------
   Text with UiStyle
   ------------------------------------------------------------------------- */
int ui_draw_ch(UiSurface *s, ss_t w, const char c) {
    if (!s)
        return -1;
    waddch(s->mwin[w], c);
    return 0;
}
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
// text - limit length
int ui_draw_text_n(UiSurface *s, ss_t w, uint y, uint x, const char *text, int n) {
    if (!s || !text)
        return -1;
    mvwaddnstr(s->mwin[w], y, x, text, (int)n);
    return 0;
}
//  text string - pad length
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
// Text Strings
// -------------------------------------------------------------------------
int ui_waddstr(UiSurface *s, ss_t w, const char *text) {
    if (!s || !text)
        return -1;
    waddstr(s->mwin[w], text);
    return 0;
}
int ui_waddnstr(UiSurface *s, ss_t w, const char *text, int m) {
    if (!s || !text)
        return -1;
    waddnstr(s->mwin[w], text, m);
    return 0;
}
// text string
int ui_mvwaddstr(UiSurface *s, ss_t w, uint y, uint x, const char *text) {
    if (!s || !text)
        return -1;
    mvwaddstr(s->mwin[w], y, x, text);
    return 0;
}
int ui_mvwaddnstr(UiSurface *s, ss_t w, uint y, uint x, const char *text, int m) {
    if (!s || !text)
        return -1;
    mvwaddnstr(s->mwin[w], y, x, text, m);
    return 0;
}
// text string - pad length
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
int ui_waddwstr(UiSurface *s, ss_t w, const wchar_t *wstr) {
    if (!s || !wstr)
        return -1;
    waddwstr(s->mwin[w], wstr);
    return 0;
}
int ui_mvwaddwstr(UiSurface *s, ss_t w, uint y, uint x, const wchar_t *wstr) {
    if (!s || !wstr)
        return -1;
    mvwaddwstr(s->mwin[w], y, x, wstr);
    return 0;
}
int ui_waddnwstr(UiSurface *s, ss_t w, const wchar_t *wstr, int m) {
    if (!s || !wstr)
        return -1;
    waddnwstr(s->mwin[w], wstr, m);
    return 0;
}
int ui_mvwaddnwstr(UiSurface *s, ss_t w, uint y, uint x, const wchar_t *wstr, int m) {
    if (!s || !wstr)
        return -1;
    mvwaddnwstr(s->mwin[w], y, x, wstr, m);
    return 0;
}
// -------------------------------------------------------------------------
// Complex Characters (cc)
// ---------------------------------------------------------------------------
int ui_wadd_wch(UiSurface *s, ss_t w, const UiCell *cell) {
    if (!s)
        return -1;
    wadd_wchnstr(s->mwin[w], cell, 1);
    return 0;
}
int ui_mvwadd_wch(UiSurface *s, ss_t w, uint y, uint x, const UiCell *cell) {
    if (!s)
        return -1;
    mvwadd_wch(s->mwin[w], y, x, cell);
    return 0;
}
int ui_wadd_wchstr(UiSurface *s, ss_t w, const UiCell *cmplx_buf) {
    if (!s || !cmplx_buf)
        return -1;
    wadd_wchstr(s->mwin[w], cmplx_buf);
    return 0;
}
int ui_mvwadd_wchstr(UiSurface *s, ss_t w, uint y, uint x, const UiCell *cmplx_buf) {
    if (!s || !cmplx_buf)
        return -1;
    mvwadd_wchstr(s->mwin[w], y, x, cmplx_buf);
    return 0;
}
int ui_wadd_wchnstr(UiSurface *s, ss_t w, const UiCell *cmplx_buf, uint n) {
    if (!s || !cmplx_buf)
        return -1;
    wadd_wchnstr(s->mwin[w], cmplx_buf, n);
    return 0;
}
int ui_mvwadd_wchnstr(UiSurface *s, ss_t w, uint y, uint x, const UiCell *cmplx_buf, uint n) {
    if (!s || !cmplx_buf)
        return -1;
    mvwadd_wchnstr(s->mwin[w], y, x, cmplx_buf, n);
    return 0;
}
/* -------------------------------------------------------------------------
   Screen management
   ------------------------------------------------------------------------- */
void ui_restore_wins() {
    touchwin(stdscr);
    for (int s = 0; s <= sfc_ptr; s++) {
        for (ss_t w = BOX; w < SUB_SFC_MAX; w++)
            if (ui_surface[s]->mwin[w] != nullptr)
                touchwin(ui_surface[s]->mwin[w]);
    }
    ui_render();
}
