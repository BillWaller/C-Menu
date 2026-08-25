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

int mbstr_to_cc(char *in_str, UiCell *cmplx_buf_s);
void parse_ansi(char *ansi_str, attr_t *attr, uint *cpx);

/* -------------------------------------------------------------------------
   Surface style
   ------------------------------------------------------------------------- */

int ui_wclrtobot(UiSurface *s, uint w) {
    if (!s)
        return -1;
    wclrtobot(s->mwin[w]);
    return 0;
}
int ui_wclrtoeol(UiSurface *s, uint w) {
    if (!s)
        return -1;
    wclrtoeol(s->mwin[w]);
    return 0;
}
/* -------------------------------------------------------------------------
   Text with UiStyle
   ------------------------------------------------------------------------- */
//  text character single
int ui_draw_ch(UiSurface *s, uint w, const char c) {
    if (!s || !c)
        return -1;
    waddch(s->mwin[w], c);
    return 0;
}
int ui_draw_ch_yx(UiSurface *s, uint w, uint y, uint x, const char c) {
    if (!s)
        return -1;
    mvwaddch(s->mwin[w], y, x, c);
    return 0;
}
//  text string
int ui_draw_text(UiSurface *s, uint w, uint y, uint x, const char *text) {
    if (!s || !text)
        return -1;
    mvwaddstr(s->mwin[w], y, x, text);
    return 0;
}
// text - limit length
int ui_draw_text_n(UiSurface *s, uint w, uint y, uint x, const char *text, int n) {
    if (!s || !text)
        return -1;
    mvwaddnstr(s->mwin[w], y, x, text, (int)n);
    return 0;
}
//  text string - pad length
int ui_draw_text_fill(UiSurface *s, uint w, uint y, uint x, const char *text, int n) {
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
// NCURSES COMPATIBILITY FUNCTIONS
// ---------------------------------------------------------------------------
// Wide Character Strings (wstr)
// ---------------------------------------------------------------------------
int ui_waddwstr(UiSurface *s, uint w, const wchar_t *wstr) {
    if (!s || !wstr)
        return -1;
    waddwstr(s->mwin[w], wstr);
    return 0;
}
int ui_mvwaddwstr(UiSurface *s, uint w, uint y, uint x, const wchar_t *wstr) {
    if (!s || !wstr)
        return -1;
    mvwaddwstr(s->mwin[w], y, x, wstr);
    return 0;
}
int ui_waddnwstr(UiSurface *s, uint w, const wchar_t *wstr, int n) {
    if (!s || !wstr)
        return -1;
    waddnwstr(s->mwin[w], wstr, n);
    return 0;
}
int ui_mvwaddnwstr(UiSurface *s, uint w, uint y, uint x, const wchar_t *wstr, int n) {
    if (!s || !wstr)
        return -1;
    mvwaddnwstr(s->mwin[w], y, x, wstr, n);
    return 0;
}
// -------------------------------------------------------------------------
// Text - Sanstyle - These should probably be implemented as macros
// -------------------------------------------------------------------------
int ui_waddstr(UiSurface *s, uint w, const char *text) {
    if (!s || !text)
        return -1;
    waddstr(s->mwin[w], text);
    return 0;
}
//  text string
int ui_mvaddstr(UiSurface *s, uint w, uint y, uint x, const char *text) {
    if (!s || !text)
        return -1;
    mvwaddstr(s->mwin[w], y, x, text);
    return 0;
}
//  text character single
int ui_mvwaddch(UiSurface *s, uint w, uint y, uint x, const char c) {
    if (!s || !c)
        return -1;
    mvwaddch(s->mwin[w], y, x, c);
    return 0;
}
// text string
int ui_mvwaddstr(UiSurface *s, uint w, uint y, uint x, const char *text) {
    if (!s || !text)
        return -1;
    mvwaddstr(s->mwin[w], y, x, text);
    return 0;
}
// text string - pad length
int ui_mvwaddstr_fill(UiSurface *s, uint w, uint y, uint x, const char *str, int l) {
    char *d, *e;
    uint maxy, maxx;
    char tmp_str[MAXLEN];
    getmaxyx(s->mwin[w], maxy, maxx);
    y = min(y, maxy);
    l = min(l, maxx);
    l = min(l, (uint)MAXLEN - 1);
    e = d = tmp_str;
    e += l;
    while (d < e) {
        if (*str == '\0' || *str == '\n')
            *d++ = ' ';
        else
            *d++ = *str++;
    }
    *d = '\0';
    l = strlen(tmp_str);
    mvwaddstr(s->mwin[w], y, x, tmp_str);
    return 0;
}
// ---------------------------------------------------------------------------
// Complex Characters (cc)
// ---------------------------------------------------------------------------
/** cc character single      */
int ui_wadd_cell(UiSurface *s, uint w, UiCell *cc) {
    if (!s || !cc)
        return -1;
    wadd_wch(s->mwin[w], cc);
    return 0;
}
int ui_mvwadd_cell(UiSurface *s, uint w, uint y, uint x, UiCell *cc) {
    if (!s || !cc)
        return -1;
    mvwadd_wch(s->mwin[w], y, x, cc);
    return 0;
}
//  cc string
int ui_wadd_cellstr(UiSurface *s, uint w, UiCell *cmplx_buf) {
    if (!s || !cmplx_buf)
        return -1;
    wadd_wchstr(s->mwin[w], cmplx_buf);
    return 0;
}
//  cc string
int ui_mvwadd_cellstr(UiSurface *s, uint w, uint y, uint x, UiCell *cmplx_buf) {
    if (!s || !cmplx_buf)
        return -1;
    mvwadd_wchstr(s->mwin[w], y, x, cmplx_buf);
    return 0;
}
//  cc string - limit length
int ui_wadd_cellnstr(UiSurface *s, uint w, UiCell *cmplx_buf, uint n) {
    if (!s || !cmplx_buf)
        return -1;
    wadd_wchnstr(s->mwin[w], cmplx_buf, n);
    return 0;
}
int ui_mvwadd_cellnstr(UiSurface *s, uint w, uint y, uint x, UiCell *cmplx_buf, uint n) {
    if (!s || !cmplx_buf)
        return -1;
    mvwadd_wchnstr(s->mwin[w], y, x, cmplx_buf, n);
    return 0;
}
// ---------------------------------------------------------------------------
// Convert Multi-byte String to Complex Character Array
// ---------------------------------------------------------------------------
//  mb string to cc
int ui_mvwadd_mbstr(UiSurface *s, uint w, uint y, uint x, const char *text) {
    if (!s || !text)
        return -1;
    UiCell cmplx_buf[MAXLEN];
    uint cols = mbstr_to_cc((char *)text, cmplx_buf);
    //  if (style)
    //      ui_ncurses_style_apply(s, w, style);
    mvwadd_wchnstr(s->mwin[w], y, x, cmplx_buf, cols);
    return 0;
}
//  mb string to cc - limit length
int ui_mvwadd_mbnstr(UiSurface *s, uint w, uint y, uint x, const char *text, int n) {
    if (!s || !text)
        return -1;
    UiCell cmplx_buf[MAXLEN];
    int cols = mbstr_to_cc((char *)text, cmplx_buf);
    cols = (cols > n) ? n : cols;
    //  if (style)
    //      ui_ncurses_style_apply(s, w, style);
    mvwadd_wchnstr(s->mwin[w], y, x, cmplx_buf, cols);
    return 0;
}
//  mb string to cc - pad length
int ui_mvwadd_mbnstr_fill(UiSurface *s, uint w, uint y, uint x, const char *text, int n) {
    if (!s || !text)
        return -1;
    UiCell cmplx_buf[MAXLEN];
    int cols = mbstr_to_cc((char *)text, cmplx_buf);
    if (cols < n) {
        UiCell fill_cc;
        wchar_t wstr[2] = {L' ', L'\0'};
        setcchar(&fill_cc, wstr, WA_NORMAL, cp_nt, nullptr);
        for (int i = cols; i < n; i++)
            cmplx_buf[i] = fill_cc;
    }
    //  if (style)
    //      ui_ncurses_style_apply(s, w, style);
    mvwadd_wchnstr(s->mwin[w], y, x, cmplx_buf, cols);
    return 0;
}

/* -------------------------------------------------------------------------
   Screen management
   ------------------------------------------------------------------------- */
void ui_restore_wins() {
    touchwin(stdscr);
    for (int s = 0; s <= sfc_ptr; s++) {
        for (uint w = 0; w < SUB_SFC_MAX; w++)
            if (ui_surface[s]->mwin[w] != nullptr)
                touchwin(ui_surface[s]->mwin[w]);
    }
    ui_render();
}
/* -------------------------------------------------------------------------
   Formatting
   ------------------------------------------------------------------------- */

/** @brief Convert a multibyte string to a complex character array. */
int mbstr_to_cc(char *in_str, UiCell *cmplx_buf_s) {
    char ansi_tok[MAXLEN];
    uint i = 0, j = 0, x = 0;
    uint len = 0;
    uint tab_stop = 4;
    uint tab_spaces = 0;
    uint char_width;
    attr_t attr = WA_NORMAL;
    uint cpx = cp_nt;
    UiCell cc = {0};
    wchar_t wstr[2] = {L'\0', L'\0'};
    UiCell *cmplx_buf = cmplx_buf_s;
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
                    for (uint z = 0; z < tab_spaces; z++) {
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
void parse_ansi(char *ansi_str, attr_t *attr, uint *cpx) {
    char *tok;
    char t0, t1;
    char tstr[3];
    uint len, x_idx;
    uint fg, bg;
    int _fg_clr, _bg_clr;
    uint fg_clr, bg_clr;
    char *ansi_p = ansi_str + 2;
    extended_pair_content(*cpx, &_fg_clr, &_bg_clr);
    fg = fg_clr = (uint)_fg_clr;
    bg = fg_clr = (uint)_bg_clr;
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
                        fg_clr = ui_add_color_rgb(&rgb);
                    else if (t0 == '4')
                        bg_clr = ui_add_color_rgb(&rgb);
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
                        fg_clr = ui_add_color_rgb(&rgb);
                    } else if (t0 == '4') {
                        tstr[0] = t1;
                        tstr[1] = '\0';
                        x_idx = a_toi(tstr, &a_toi_error);
                        rgb = xterm256_idx_to_rgb(x_idx);
                        bg_clr = ui_add_color_rgb(&rgb);
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
    if (!a_toi_error && (fg_clr != fg || bg_clr != bg))
        *cpx = ui_add_pair(fg_clr, bg_clr);
    return;
}
