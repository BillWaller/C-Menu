/** @file ui_ncurses.c
   @ingroup ui_ncurses
   @brief NCurses UI backend — lifecycle, surface management, and capabilities.

   Implements all UiRuntime and UiSurface operations declared in ui_backend.h
   using the NCurses / panelw library.

   When compiled as part of the main C-Menu build (UAL_LEGACY_COMPAT defined),
   the legacy globals @c screen and @c tty_fp from dwin.c are kept in sync so
   that code not yet migrated to the UAL API continues to work.
*/

#define _XOPEN_SOURCE_EXTENDED 1

#include "cm.h"

#include "ui_ncurses_internal.h"
#ifdef UAL_LEGACY_COMPAT
#include "ui_ncurses_compat.h"
#endif
#include <locale.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

UiRuntime *ui = NULL;
UiSurface *ui_surface[UI_SFC_MAX];

int win_ptr = -1;
int sfc_ptr = -1;

uint ui_color_cnt = 0;
uint ui_pair_cnt = 0;

UiSurface *stdsfc;

/* -------------------------------------------------------------------------
   Backend identification and capability query
   ------------------------------------------------------------------------- */

UiBackend ui_get_backend() {
    return UI_BACKEND_NCURSES;
}

void ui_get_caps(UiCaps *caps) {
    if (!caps)
        return;
    memset(caps, 0, sizeof(*caps));
    if (!ui)
        return;
    // Will add code to actually check later. For now, just lie.
    caps->truecolor = true;
    caps->palette256 = true;
    caps->mouse = ui->mouse_enabled;
    caps->unicode = true;
    caps->resize = true;
    caps->color_pairs = 0;
}
/* -------------------------------------------------------------------------
   Colors, Color Pairs
   ------------------------------------------------------------------------- */
uint ui_add_pair(uint fg, uint bg) {
    int rc;
    uint i;
    uint pfg, pbg;
    for (i = 1; i < ui_pair_cnt; i++) {
        ui_pair_content(i, &pfg, &pbg);
        if (pfg == fg && pbg == bg)
            return i;
    }
    if (i + 1 >= UI_PAIRS) {
        ssnprintf(em0, MAXLEN - 1, "%s, line: %d", __FILE__, __LINE__ - 1);
        ssnprintf(em1, MAXLEN - 1, "ui_add_pair failed for pair: %d", i);
        strerror_r(errno, em2, MAXLEN);
        display_error(em0, em1, em2, nullptr);
        return (EXIT_FAILURE);
    }
    rc = init_extended_pair(i, fg, bg);
    if (rc == ERR) {
        ssnprintf(em0, MAXLEN - 1, "%s, line: %d", __FILE__, __LINE__ - 1);
        ssnprintf(em1, MAXLEN - 1, "init_extended_pair failed for pair: %d", i);
        ssnprintf(em2, MAXLEN - 1, "fg: %d, bg: %d, ui_pair_cnt: %d", fg, bg, ui_pair_cnt);
        display_error(em0, em1, em2, nullptr);
        return (EXIT_FAILURE);
    }
    ui_pair_cnt++;
    return (int)(ui_pair_cnt - 1);
}
int ui_chg_pair(uint pair, uint fg, uint bg) {
    if (pair + 1 >= UI_PAIRS)
        return -1;
    init_extended_pair(pair, fg, bg);
    return 0;
}
int ui_add_color_rgb(RGB *rgb) {
    uint i;
    RGB tmp;
    apply_gamma(rgb);
    rgb->r = (rgb->r * 1000) / 255;
    rgb->g = (rgb->g * 1000) / 255;
    rgb->b = (rgb->b * 1000) / 255;
    rgb->a = 0;
    for (i = 0; i < ui_color_cnt && i < UI_COLORS; i++) {
        extended_color_content(i, &tmp.r, &tmp.g, &tmp.b);
        if (rgb->r == tmp.r && rgb->g == tmp.g && rgb->b == tmp.b)
            return i;
    }
    if (i < UI_COLORS) {
        if (i < 16) {
            std_color[i].r = rgb->r;
            std_color[i].g = rgb->g;
            std_color[i].b = rgb->b;
        }
        init_extended_color(i, rgb->r, rgb->g, rgb->b);
        if (ui_color_cnt + 1 < UI_COLORS)
            ui_color_cnt++;
        return ui_color_cnt - 1;
    }
    return 0;
}
uint32_t ui_get_color(uint16_t color_idx) {
    if (color_idx + 1 >= UI_COLORS)
        return -1;
    RGB rgb;
    extended_color_content(color_idx, &rgb.r, &rgb.g, &rgb.b);
    rgb.r = (rgb.r * 255) / 1000;
    rgb.g = (rgb.g * 255) / 1000;
    rgb.b = (rgb.b * 255) / 1000;
    return 0;
}
RGB ui_hex_to_rgb(char *s) {
    RGB rgb;
    sscanf(s, "#%02x%02x%02x", &rgb.r, &rgb.g, &rgb.b);
    return rgb;
}
int ui_color_content(uint color, uint *r, uint *g, uint *b) {
    int _color = (int)color;
    int _r, _g, _b;
    extended_color_content(_color, &_r, &_g, &_b);
    *r = (uint)_r;
    *g = (uint)_g;
    *b = (uint)_b;
    return 0;
}

int ui_init_color(uint color, uint r, uint g, uint b) {
    init_extended_color(color, r, g, b);
    return 0;
}

int ui_pair_content(uint pair, uint *fg, uint *bg) {
    int _pair = (int)pair;
    int _fg, _bg;
    extended_pair_content(_pair, &_fg, &_bg);
    *fg = (uint)_fg;
    *bg = (uint)_bg;
    return 0;
}
int ui_init_pair(uint pair, uint fg, uint bg) {
    init_extended_pair(pair, fg, bg);
    return 0;
}
int ui_chg_color(uint16_t color_idx, uint32_t *color) {
    if (color_idx + 1 >= UI_COLORS)
        return -1;
    RGB rgb;
    rgb.r = (*color >> 16) & 0xff;
    rgb.g = (*color >> 8) & 0xff;
    rgb.b = *color & 0xff;
    apply_gamma(&rgb);
    rgb.r = (rgb.r * 1000) / 255;
    rgb.g = (rgb.g * 1000) / 255;
    rgb.b = (rgb.b * 1000) / 255;
    if (color_idx < 16) {
        std_color[color_idx].r = rgb.r;
        std_color[color_idx].g = rgb.g;
        std_color[color_idx].b = rgb.b;
    }
    init_extended_color(color_idx, rgb.r, rgb.g, rgb.b);
    return 0;
}

/* -------------------------------------------------------------------------
   Styles
   ------------------------------------------------------------------------- */

int ui_pair_from_hex(const char *fg, const char *bg) {
    RGB rgb;
    sscanf(fg, "#%02x%02x%02x", &rgb.r, &rgb.g, &rgb.b);
    int f_idx = ui_add_color_rgb(&rgb);
    sscanf(bg, "#%02x%02x%02x", &rgb.r, &rgb.g, &rgb.b);
    int b_idx = ui_add_color_rgb(&rgb);
    return ui_add_pair(f_idx, b_idx);
}

UiCell ui_cell_from_ucp(const wchar_t *ucp, const uint32_t *fg, const uint32_t *bg) {
    UiCell cc = {0};
    RGB rgb;
    rgb.r = (*fg >> 16) & 0xff;
    rgb.g = (*fg >> 8) & 0xff;
    rgb.b = *fg & 0xff;
    int f_idx = ui_add_color_rgb(&rgb);
    rgb.r = (*bg >> 16) & 0xff;
    rgb.g = (*bg >> 8) & 0xff;
    rgb.b = *bg & 0xff;
    int b_idx = ui_add_color_rgb(&rgb);
    short cp = ui_add_pair(f_idx, b_idx);
    wchar_t wstr[2] = {L'\0', L'\0'};
    wstr[0] = *ucp;
    wstr[1] = L'\0';
    setcchar(&cc, wstr, WA_NORMAL, cp, nullptr);
    return cc;
}

/* -------------------------------------------------------------------------
   Lifecycle
   ------------------------------------------------------------------------- */
struct UiRuntime *ui_init(const UiConfig *cfg) {
    setlocale(LC_ALL, "");
    ui = calloc(1, sizeof(*ui));
    if (!ui)
        return NULL;
    char tty_name[MAXLEN];
    if (cfg && cfg->tty_path) {
        strncpy(tty_name, cfg->tty_path, sizeof(tty_name) - 1);
        tty_name[sizeof(tty_name) - 1] = '\0';
    } else {
        if (ttyname_r(STDERR_FILENO, tty_name, sizeof(tty_name)) != 0) {
            free(ui);
            return NULL;
        }
    }
    ui->tty_fp = fopen(tty_name, "r+");
    if (!ui->tty_fp) {
        free(ui);
        return NULL;
    }
    ui->screen = newterm(NULL, ui->tty_fp, ui->tty_fp);
    if (!ui->screen) {
        fclose(ui->tty_fp);
        free(ui);
        return NULL;
    }
    set_term(ui->screen);
    f_curses_open = true;
    if (!has_colors() || !can_change_color()) {
        ui_shutdown();
        return NULL;
    }
    start_color();
    use_default_colors();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    if (cfg) {
        ui->mouse_enabled = cfg->enable_mouse;
        ui->alt_screen = cfg->enable_alt_screen;
        ui->cursor_visible = cfg->cursor_visible;
        ui->border_style = cfg->border_style;
    } else {
        ui->cursor_visible = false;
        ui->alt_screen = false;
    }
    if (ui->mouse_enabled)
        mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
    getmaxyx(stdscr, ui->lines, ui->cols);
    if (cfg) {
        if (ui->border_style == 'r' || ui->border_style == 'R') {
            memcpy(bw.str, border_rounded, sizeof(bw.str));
        } else if (ui->border_style == 's' || ui->border_style == 'S') {
            memcpy(bw.str, border_single, sizeof(bw.str));
        } else if (ui->border_style == 'd' || ui->border_style == 'D') {
            memcpy(bw.str, border_double, sizeof(bw.str));
        } else if (ui->border_style == 'h' || ui->border_style == 'H') {
            memcpy(bw.str, border_heavy, sizeof(bw.str));
        } else if (ui->border_style == 'n' || ui->border_style == 'N') {
            memcpy(bw.str, border_none, sizeof(bw.str));
        }
    }
    stdsfc = calloc(1, sizeof(*stdsfc));
    if (!stdsfc)
        return NULL;
    stdsfc->runtime = ui;
    stdsfc->parent = NULL;
    stdsfc->meta[BOX].lines = (int)ui->lines;
    stdsfc->meta[BOX].cols = (int)ui->cols;
    stdsfc->meta[BOX].y = 0;
    stdsfc->meta[BOX].x = 0;
    stdsfc->mwin[BOX] = stdscr;
    stdsfc->mpan[BOX] = new_panel(stdsfc->mwin[BOX]);
    if (!stdsfc->mpan[BOX]) {
        free(stdsfc);
        Perror("new_panel failed for stdsfc->mpan[BOX]");
        exit(EXIT_FAILURE);
    }
    return ui;
}
void ui_endwin() {
    ui_shutdown();
}
void ui_shutdown() {
    if (ui == NULL)
        return;
    while (sfc_ptr >= 0) {
        if (ui_surface[sfc_ptr] != NULL) {
            ui_surface_destroy(ui_surface[sfc_ptr]);
            ui_surface[sfc_ptr] = NULL;
        }
        sfc_ptr--;
    }
    if (stdsfc->mpan[0] != NULL) {
        hide_panel(stdsfc->mpan[0]);
        del_panel(stdsfc->mpan[0]);
        stdsfc->mpan[0] = NULL;
    }
    f_curses_open = false;
    endwin();
    if (ui->screen != NULL) {
        delscreen(ui->screen);
        ui->screen = NULL;
    }
    if (ui->tty_fp != NULL) {
        fclose(ui->tty_fp);
        ui->tty_fp = NULL;
    }
    if (stdsfc != NULL) {
        free(stdsfc);
        stdsfc = NULL;
    }
    if (ui != NULL) {
        free(ui);
        ui = NULL;
    }
}
void ui_surface_destroy(UiSurface *s) {
    if (!s)
        return;
    for (int i = SUB_SFC_MAX; i >= 0; i--) {
        if (s->mpan[i] != NULL) {
            hide_panel(s->mpan[i]);
            del_panel(s->mpan[i]);
            s->mpan[i] = NULL;
        }
    }
    for (int i = SUB_SFC_MAX; i >= 0; i--) {
        if (s->mwin[i] != NULL) {
            delwin(s->mwin[i]);
            s->mwin[i] = NULL;
        }
    }
    if (s != NULL) {
        free(s);
        s = NULL;
    }
}
int ui_suspend() {
    def_prog_mode();
    endwin();
    return 0;
}
int ui_resume() {
    reset_prog_mode();
    update_panels();
    doupdate();
    return 0;
}

/* -------------------------------------------------------------------------
   Surface Creation and Destruction
   ------------------------------------------------------------------------- */

UiSurface *ui_surface_new(uint w, UiSurface *parent, uint p, uint lines, uint cols, uint y, uint x) {
    if (!ui)
        return NULL;
    UiSurface *s = calloc(1, sizeof(*s));
    if (!s)
        return NULL;
    for (int i = 0; i < SUB_SFC_MAX; i++) {
        s->mwin[i] = NULL;
        s->mpan[i] = NULL;
    }
    s->runtime = ui;
    s->parent = parent;
    s->meta[w].lines = lines;
    s->meta[w].cols = cols;
    s->meta[w].y = y;
    s->meta[w].x = x;

    if (parent && parent->mwin[p]) {
        s->mwin[w] = derwin(parent->mwin[p], lines, cols, y, x);
        if (!s->mwin[w]) {
            free(s);
            return NULL;
        }
    } else {
        s->mwin[w] = newwin(lines, cols, y, x);
        if (!s->mwin[w]) {
            free(s);
            return NULL;
        }
        s->mpan[w] = new_panel(s->mwin[w]);
        if (!s->mpan[w]) {
            delwin(s->mwin[w]);
            free(s);
            return NULL;
        }
    }
    return s;
}

UiSurface *ui_box_surface_new(UiSurface *parent, uint p, uint lines, uint cols, uint y, uint x, char *wtitle) {
    UiSurface *s = calloc(1, sizeof(*s));
    if (!s)
        return NULL;
    s->runtime = ui;
    s->parent = parent;
    s->meta[BOX].y = y;
    s->meta[BOX].x = x;
    s->meta[BOX].lines = lines;
    s->meta[BOX].cols = cols;
    if (parent && parent->mwin[p]) {
        s->mwin[BOX] = derwin(parent->mwin[p], lines + 2, cols + 2, y, x);
        if (!s->mwin[BOX]) {
            free(s);
            return NULL;
        }
    } else {
        s->mwin[BOX] = newwin(lines + 2, cols + 2, y, x);
        if (!s->mwin[BOX]) {
            free(s);
            return NULL;
        }
        s->mpan[BOX] = new_panel(s->mwin[BOX]);
        if (!s->mpan[BOX]) {
            delwin(s->mwin[BOX]);
            return NULL;
        }
    }
    ui_scrollok(s, BOX, false);
    ui_border_draw(s);
    ui_border_title(s, wtitle);
#ifdef DEBUG_UI
    immedok(s->mwin[BOX], true);
#endif
    return s;
}

void fast_exit(UiSurface *s) {
    update_panels();
    doupdate();
    for (int i = SUB_SFC_MAX; i >= 0; i--) {
        if (s->mpan[i]) {
            hide_panel(s->mpan[i]);
            del_panel(s->mpan[i]);
            s->mpan[i] = NULL;
        }
    }
    for (int i = SUB_SFC_MAX; i >= 0; i--) {
        if (s->mwin[i]) {
            delwin(s->mwin[i]);
            s->mwin[i] = NULL;
        }
    }
    endwin();
    exit(EXIT_SUCCESS);
}
int ui_surface_addpad(UiSurface *s, uint w, uint view_win, uint lines, uint cols, uint begy, uint begx) {
    (void)begy;
    (void)begx;
    s->mwin[w] = newpad(lines, cols);
    if (s->mwin[w] == nullptr)
        return -1;
    s->mwin[view_win] = subpad(s->mwin[PAD], lines, cols, 0, 0);
    if (s->mwin[view_win] == nullptr)
        return -1;
    s->mpan[w] = new_panel(s->mwin[view_win]);
    immedok(s->mwin[w], true);
#ifdef DEBUG_UI
#endif
    return 0;
}

int ui_surface_addwin(UiSurface *s, uint w, uint p, uint lines, uint cols, uint y, uint x) {
    s->mwin[w] = derwin(s->mwin[p], lines, cols, y, x);
    if (!s->mwin[w]) {
        free(s);
        return -1;
    }
    s->mpan[w] = new_panel(s->mwin[w]);
    keypad(s->mwin[w], true);
    ui_bkgd(s, w, &cell_nt);
    ui_bkgdset(s, w, &cell_nt);
    immedok(s->mwin[w], true);
#ifdef DEBUG_UI
#endif
    return 0;
}
// -------------------------------------------------------------------------
// Surface Management
// -------------------------------------------------------------------------

int ui_surface_move(UiSurface *s, uint w, uint y, uint x) {
    if (!s)
        return -1;
    s->meta[w].y = y;
    s->meta[w].x = x;
    return move_panel(s->mpan[w], y, x);
}

int ui_surface_resize(UiSurface *s, uint w, uint lines, uint cols) {
    if (!s)
        return -1;
    s->meta[w].lines = lines;
    s->meta[w].cols = cols;
    wresize(s->mwin[w], lines + 2, cols + 2);
    wresize(s->mwin[w + 1], lines, cols);
    return 0;
}

int ui_werase(UiSurface *s, uint w) {
    if (w == ALLWINS) {
        for (int i = 1; i < SUB_SFC_MAX; i++) {
            if (s->mwin[i]) {
                werase(s->mwin[i]);
            }
        }
    } else {
        if (s->mwin[w]) {
            werase(s->mwin[w]);
        }
    }
    return 0;
}
int ui_wclear(UiSurface *s, uint w) {
    if (!s)
        return -1;
    if (w == ALLWINS) {
        for (int i = 1; i < SUB_SFC_MAX; i++) {
            if (s->mwin[i]) {
                wclear(s->mwin[i]);
            }
        }
    } else {
        if (s->mwin[w]) {
            wclear(s->mwin[w]);
        }
    }
    return 0;
}
int ui_erase() {
    erase();
    return 0;
}
int ui_clear() {
    clear();
    return 0;
}
int ui_surface_show(UiSurface *s, uint w) {
    if (!s)
        return -1;
    show_panel(s->mpan[w]);
    s->meta[w].hidden = false;
    return 0;
    return 0;
}

int ui_top_surface(UiSurface *s, uint w) {
    if (!s)
        return -1;
    top_panel(s->mpan[w]);
    return 0;
}

int ui_surface_hide(UiSurface *s, uint w) {
    if (!s)
        return -1;
    hide_panel(s->mpan[w]);
    s->meta[w].hidden = true;
    return 0;
}

// -------------------------------------------------------------------------
// Controls and Settings
// -------------------------------------------------------------------------

int ui_scrollok(UiSurface *s, uint w, bool enable) {
    if (!s->mwin[w])
        return -1;
    if (enable)
        scrollok(s->mwin[w], true);
    else
        scrollok(s->mwin[w], false);
    return 0;
}
int ui_keypad(UiSurface *s, uint w, bool enable) {
    if (!s->mwin[w])
        return -1;
    if (enable)
        keypad(s->mwin[w], true);
    else
        keypad(s->mwin[w], false);
    return 0;
}
int ui_idlok(UiSurface *s, uint w, bool enable) {
    if (!s->mwin[w])
        return -1;
    if (enable)
        idlok(s->mwin[w], true);
    else
        idlok(s->mwin[w], false);
    return 0;
}
int ui_idcok(UiSurface *s, uint w, bool enable) {
    if (!s->mwin[w])
        return -1;
    if (enable)
        idcok(s->mwin[w], true);
    else
        idcok(s->mwin[w], false);
    return 0;
}
int ui_setscrreg(UiSurface *s, uint w, uint top, uint bottom) {
    if (!s->mwin[w])
        return -1;
    wsetscrreg(s->mwin[w], top, bottom);
    return 0;
}
void ui_getyx(UiSurface *s, uint w, uint *lines, uint *cols) {
    if (!s->mwin[w])
        return;
    int _lines, _cols;
    getyx(s->mwin[w], _lines, _cols);
    *lines = (uint)(_lines);
    *cols = (uint)(_cols);
}
void ui_getmaxyx(UiSurface *s, uint w, uint *lines, uint *cols) {
    if (!s->mwin[w])
        return;
    getmaxyx(s->mwin[w], *lines, *cols);
}
int ui_getmaxy(UiSurface *s, uint w) {
    if (!s->mwin[w])
        return -1;
    return (int)(getmaxy(s->mwin[w]));
}
int ui_getmaxx(UiSurface *s, uint w) {

    if (!s->mwin[w])
        return -1;
    return (int)(getmaxx(s->mwin[w]));
}
void ui_get_screen_size(uint *lines, uint *cols) {
    if (!ui)
        return;
    getmaxyx(stdscr, ui->lines, ui->cols);
    if (lines)
        *lines = ui->lines;
    if (cols)
        *cols = ui->cols;
}
// included for symetry with notcurses, which needs it to compensate
// for cursor positioning bug
int ui_cursor_enable_yx(UiSurface *s, uint w, uint y, uint x, bool visible) {
    if (!ui)
        return -1;
    wmove(s->mwin[w], y, x);
    ui->cursor_visible = visible;
    curs_set(visible ? 2 : 0);
    return 0;
}
int ui_cursor_enable(UiSurface *s, uint w, bool visible) {
    (void)s;
    (void)w;
    if (!ui)
        return -1;
    ui->cursor_visible = visible;
    curs_set(visible ? 1 : 0);
    return 0;
}

// -------------------------------------------------------------------------
// Screen Navigation
// -------------------------------------------------------------------------

int ui_cursor_move(UiSurface *s, uint w, uint y, uint x) {
    if (!s || !s->mwin[w])
        return -1;
    return wmove(s->mwin[w], y, x);
}
int ui_wmove(UiSurface *s, uint w, uint y, uint x) {
    if (!s || !s->mwin[w])
        return -1;
    return wmove(s->mwin[w], y, x);
}
int ui_curs_set(int visibility) {
    curs_set(visibility);
    return 0;
}
int ui_wscrl(UiSurface *s, uint w, int n) {
    if (!s->mwin[w])
        return -1;
    wscrl(s->mwin[w], n);
    ui_render();
    return 0;
}
// -------------------------------------------------------------------------
// Background
// -------------------------------------------------------------------------

// for the entire window
int ui_bkgd(UiSurface *s, uint w, const UiCell *cell) {
    if (!s)
        return -1;
    wbkgrnd(s->mwin[w], cell);
    return 0;
}
// for new content to be written to the window
int ui_bkgdset(UiSurface *s, uint w, const UiCell *cell) {
    if (!s)
        return -1;
    wbkgrndset(s->mwin[w], cell);
    return 0;
}
// for the entire window
int ui_bkgrnd(UiSurface *s, uint w, const UiCell *cell) {
    if (!s->mwin[w])
        return -1;
    wbkgrnd(s->mwin[w], cell);
    return 0;
}
// for new content to be written to the window
int ui_bkgrndset(UiSurface *s, uint w, const UiCell *cell) {
    if (!s->mwin[w])
        return -1;
    wbkgrndset(s->mwin[w], cell);
    return 0;
}
/* -------------------------------------------------------------------------
   Cell Manipulation
   ------------------------------------------------------------------------- */

int ui_getcchar(const UiCell *uc, wchar_t *wstr, attr_t *attrs, uint16_t *pair, void *opts) {
    short p;
    getcchar(uc, wstr, attrs, &p, opts);
    pair = (uint16_t *)&p;
    p = (short)(*pair); // Tell compiler to forget about it!
    return 0;
}

int ui_setcchar(cchar_t *wch, const wchar_t *wc, const attr_t attrs, short pair, const void *opts) {
    (void)opts;
    return setcchar(wch, wc, attrs, pair, NULL);
}
/* -------------------------------------------------------------------------
   Rendering
   ------------------------------------------------------------------------- */

void ui_render() {
    update_panels();
    doupdate();
}
void ui_update_panels() {
    update_panels();
}
int ui_doupdate() {
    doupdate();
    return 0;
}
int ui_wnoutrefresh(UiSurface *s, uint w) {
    if (!s)
        return -1;
    wnoutrefresh(s->mwin[w]);
    return 0;
}
/* -------------------------------------------------------------------------
   Non-portable escape-hatch getters (see ui_ncurses_compat.h)
   ------------------------------------------------------------------------- */

SCREEN *ui_ncurses_get_screen() {
    if (!ui)
        return NULL;
    return ui->screen;
}

WINDOW *ui_ncurses_surface_get_win(const UiSurface *s, uint w) {
    if (!s)
        return NULL;
    return s->mwin[w];
}

PANEL *ui_ncurses_surface_get_panel(const UiSurface *s, uint w) {
    if (!s)
        return NULL;
    return s->mpan[w];
}
