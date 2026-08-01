/** @file ui_ncurses_draw.c
   @ingroup ui_ncurses
   @brief NCurses UI backend — drawing functions.

   Implements the drawing operations declared in ui_backend.h using the
   NCurses wide-character API.
*/

#define _XOPEN_SOURCE_EXTENDED 1

#include "ui_ncurses_internal.h"
#include <ncurses/panel.h>
#include <ncursesw/ncurses.h>
#define UAL_LEGACY_COMPAT 1
#ifdef UAL_LEGACY_COMPAT
#include "cm.h"
#endif
#include "cm.h"
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

int ui_mvwadd_mbnstr(UiSurface *s, int y, int x, const char *text, int n);
int ui_mvwadd_mbnstr_fill(UiSurface *s, int y, int x, const char *text, int n);
int mbstr_to_cc(char *in_str, cchar_t *cmplx_buf_s);
void parse_ansi(char *ansi_str, attr_t *attr, int *cpx);

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
    attrs |= style->bold ? WA_BOLD : 0;
    attrs |= style->dim ? WA_DIM : 0;
    attrs |= style->italic ? WA_ITALIC : 0;
    attrs |= style->underline ? WA_UNDERLINE : 0;
    attrs |= style->blink ? WA_BLINK : 0;
    attrs |= style->reverse ? WA_REVERSE : 0;
    attrs |= style->invis ? WA_INVIS : 0;
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

int ui_mvwaddstr(UiSurface *s, int y, int x, const char *txt) {
    if (!s || !txt)
        return -1;
    mvwaddstr(s->win, y, x, txt);
    return 0;
}

void ui_mvwaddstr_fill(UiSurface *sfc, int y, int x, char *s, int l) {
    char *d, *e;
    int maxy, maxx;
    char tmp_str[MAXLEN];
    getmaxyx(sfc->win, maxy, maxx);
    y = min(y, maxy);
    l = min(l, maxx);
    l = min(l, MAXLEN - 1);
    e = d = tmp_str;
    e += l;
    while (d < e) {
        if (*s == '\0' || *s == '\n')
            *d++ = ' ';
        else
            *d++ = *s++;
    }
    *d = '\0';
    l = strlen(tmp_str);
    mvwaddstr(sfc->win, y, x, tmp_str);
}

int ui_wclrtoeol(UiSurface *s) {
    if (!s)
        return -1;
    wclrtoeol(s->win);
    return 0;
}

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

/** @brief Draw exactly n bytes of UTF-8 text at (y, x). */
int ui_draw_text_fill(UiSurface *s, int y, int x, const UiStyle *style,
                      const char *text, size_t n) {
    if (!s || !text)
        return -1;
    if (style)
        ui_ncurses_style_apply(s->win, style);
    int l = strlen(text);
    if (l < (int)n) {
        char *tmp_str = (char *)malloc(n + 1);
        if (!tmp_str)
            return -1;
        strcpy(tmp_str, text);
        for (int i = l; i < (int)n; i++) {
            tmp_str[i] = ' ';
        }
        tmp_str[n] = '\0';
        mvwaddstr(s->win, y, x, tmp_str);
        free(tmp_str);
    } else {
        mvwaddnstr(s->win, y, x, text, (int)n);
    }
    return 0;
}
/** @brief Draw UTF-8 character at (y, x) with optional style. */
int ui_draw_ch(UiSurface *s, int y, int x, const UiStyle *style, const char c) {
    if (!s || !c)
        return -1;
    if (style)
        ui_ncurses_style_apply(s->win, style);
    mvwaddch(s->win, y, x, c);
    return 0;
}

/** @brief Display at most n columns. */
int ui_mvwadd_mbnstr(UiSurface *s, int y, int x, const char *text, int n) {
    if (!s || !text)
        return -1;
    cchar_t cmplx_buf[MAXLEN];
    int cols = mbstr_to_cc((char *)text, cmplx_buf);
    cols = (cols > n) ? n : cols;
    mvwadd_wchnstr(s->win, y, x, cmplx_buf, cols);
    return 0;
}

/** @brief Display n columns, space filling if necessary */
int ui_mvwadd_mbnstr_fill(UiSurface *s, int y, int x, const char *text, int n) {
    if (!s || !text)
        return -1;
    cchar_t cmplx_buf[MAXLEN];
    int cols = mbstr_to_cc((char *)text, cmplx_buf);
    if (cols < n) {
        cchar_t fill_cc;
        wchar_t wstr[2] = {L' ', L'\0'};
        setcchar(&fill_cc, wstr, WA_NORMAL, cp_nt, nullptr);
        for (int i = cols; i < n; i++) {
            cmplx_buf[i] = fill_cc;
        }
    }
    mvwadd_wchnstr(s->win, y, x, cmplx_buf, cols);
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
        cchar_t ho, ve;
        wchar_t wcs[2] = {0, 0};
        attr_t a = 0;
        short cp = 0;
        wcs[0] = L'\x256d';
        setcchar(&tl, wcs, a, cp, NULL);
        wcs[0] = L'\x256e';
        setcchar(&tr, wcs, a, cp, NULL);
        wcs[0] = L'\x2570';
        setcchar(&bl, wcs, a, cp, NULL);
        wcs[0] = L'\x256f';
        setcchar(&br, wcs, a, cp, NULL);
        wcs[0] = L'\x2500';
        setcchar(&ho, wcs, a, cp, NULL);
        wcs[0] = L'\x2502';
        setcchar(&ve, wcs, a, cp, NULL);
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
/** @brief Convert a multibyte string to a complex character array. */
int mbstr_to_cc(char *in_str, cchar_t *cmplx_buf_s) {
    char ansi_tok[MAXLEN];
    int i = 0, j = 0, x = 0;
    int len = 0;
    int tab_stop = 4;
    int tab_spaces = 0;
    int char_width;
    attr_t attr = WA_NORMAL;
    int cpx = cp_nt;
    cchar_t cc = {0};
    wchar_t wstr[2] = {L'\0', L'\0'};
    cchar_t *cmplx_buf = cmplx_buf_s;
    mbstate_t mbstate;
    memset(&mbstate, 0, sizeof(mbstate));
    while (in_str[i] != '\0') {        // line
        while (1) {                    // ANSI SGR, Character, and Word
            if (in_str[i] == '\033') { // ANSI SGR
                if (in_str[i + 1] == '[') {
                    len = strcspn(&in_str[i], "mK ") + 1;
                    memcpy(ansi_tok, &in_str[i], len + 1);
                    ansi_tok[len] = '\0';
                    if (ansi_tok[0] == '\0') {
                        if (i + 2 < MAXLEN)
                            i += 2;
                        continue;
                    }
                    if (len == 0 || in_str[i + len - 1] == ' ') {
                        i += 2;
                        continue;
                    } else if (in_str[i + len - 1] == 'K') {
                        i += len;
                        continue;
                    }
                    parse_ansi(ansi_tok, &attr, &cpx);
                    i += len;
                } else {
                    i++;
                    continue;
                }
            } else { // Characters
                if (in_str[i] == ' ') {
                    wstr[0] = L' ';
                    wstr[1] = L'\0';
                    setcchar(&cc, wstr, attr, cpx, nullptr);
                    cmplx_buf[j++] = cc;
                    x++;
                    i++;
                    continue;
                }
                if (in_str[i] == '\0') {
                    break;
                }
                if (in_str[i] == '\t') {
                    tab_spaces = tab_stop - j % tab_stop;
                    wstr[0] = L' ';
                    wstr[1] = L'\0';
                    setcchar(&cc, wstr, attr, cpx, nullptr);
                    for (int z = 0; z < tab_spaces; z++) {
                        cmplx_buf[j++] = cc;
                        x++;
                    }
                    i++;
                    continue;
                }
                wstr[1] = L'\0';
                len = mbrtowc(wstr, &in_str[i], MB_CUR_MAX, &mbstate);
                if (len <= 0) {
                    wstr[0] = L'?';
                    wstr[1] = L'\0';
                    len = 1;
                }
                char_width = wcwidth(wstr[0]);
                x += char_width;
                setcchar(&cc, wstr, attr, cpx, nullptr);
                cmplx_buf[j++] = cc;
                i += len;
                continue;
            } // END Character and Word
        } // END WHILE - ANSI SGR, Character, and Word
    } // END WHILE - line
    wstr[0] = '\0';
    wstr[1] = '\0';
    setcchar(&cc, wstr, WA_NORMAL, cpx, nullptr);
    cmplx_buf[j] = cc;
    return x;
}
/** @brief Parse an ANSI SGR sequence and update attributes and color pair index. */
void parse_ansi(char *ansi_str, attr_t *attr, int *cpx) {
    char *tok;
    char t0, t1;
    char tstr[3];
    int len, x_idx;
    int fg, bg;
    int fg_clr, bg_clr;
    char *ansi_p = ansi_str + 2;
    extended_pair_content(*cpx, &fg_clr, &bg_clr);
    fg = fg_clr;
    bg = bg_clr;
    RGB rgb;
    tok = strtok((char *)ansi_p, ";m");
    bool a_toi_error = false;
    while (1) {
        if (tok == nullptr || *tok == '\0')
            break;
        len = strlen(tok);
        if (len == 2) {
            t0 = tok[0];
            t1 = tok[1];
            if (t0 == '3' || t0 == '4') {
                if (t1 == '8') {
                    tok = strtok(nullptr, ";m");
                    if (tok != nullptr) {
                        if (*tok == '5') {
                            tok = strtok(nullptr, ";m");
                            if (tok != nullptr) {
                                x_idx = a_toi(tok, &a_toi_error);
                                rgb = xterm256_idx_to_rgb(x_idx);
                            }
                        } else if (*tok == '2') {
                            tok = strtok(nullptr, ";m");
                            rgb.r = a_toi(tok, &a_toi_error);
                            tok = strtok(nullptr, ";m");
                            rgb.g = a_toi(tok, &a_toi_error);
                            tok = strtok(nullptr, ";m");
                            rgb.b = a_toi(tok, &a_toi_error);
                        }
                    }
                    if (t0 == '3')
                        fg_clr = rgb_to_curses_clr(&rgb);
                    else if (t0 == '4')
                        bg_clr = rgb_to_curses_clr(&rgb);
                } else if (t1 == '9') {
                    if (t0 == '3')
                        fg_clr = CLR_NT_FG;
                    else if (t0 == '4')
                        bg_clr = CLR_NT_BG;
                } else if (t1 >= '0' && t1 <= '7') {
                    if (t0 == '3') {
                        tstr[0] = t1;
                        tstr[1] = '\0';
                        x_idx = a_toi(tstr, &a_toi_error);
                        rgb = xterm256_idx_to_rgb(x_idx);
                        fg_clr = rgb_to_curses_clr(&rgb);
                    } else if (t0 == '4') {
                        tstr[0] = t1;
                        tstr[1] = '\0';
                        x_idx = a_toi(tstr, &a_toi_error);
                        rgb = xterm256_idx_to_rgb(x_idx);
                        bg_clr = rgb_to_curses_clr(&rgb);
                    }
                }
            } else if (t0 == '0') {
                *tok = t1;
                len = 1;
            }
        }
        if (len == 1) {
            if (*tok == '0') {
                *attr = WA_NORMAL;
                fg_clr = CLR_NT_FG;
                bg_clr = CLR_NT_BG;
            } else {
                switch (a_toi(tok, &a_toi_error)) {
                case 1:
                    *attr |= WA_BOLD;
                    break;
                case 2:
                    *attr |= WA_DIM;
                    break;
                case 3:
                    *attr |= WA_ITALIC;
                    break;
                case 4:
                    *attr |= WA_UNDERLINE;
                    break;
                case 5:
                    *attr |= WA_BLINK;
                    break;
                case 7:
                    *attr |= WA_REVERSE;
                    break;
                case 8:
                    *attr |= WA_INVIS;
                    break;
                default:
                    break;
                }
            }
        } else if (len == 0) {
            *attr = WA_NORMAL;
            fg_clr = CLR_NT_FG;
            bg_clr = CLR_NT_BG;
        }
        tok = strtok(nullptr, ";m");
    }
    if (!a_toi_error && (fg_clr != fg || bg_clr != bg)) {
        clr_pair_idx = get_clr_pair(fg_clr, bg_clr);
        *cpx = clr_pair_idx;
    }
    return;
}
