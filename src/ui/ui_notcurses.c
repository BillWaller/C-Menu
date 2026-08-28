/** @file ui_notcurses.c
   @ingroup ui_notcurses
   @brief NotCurses UI backend — lifecycle, surface management, and capabilities.

   Implements all UiRuntime and UiSurface operations declared in ui_backend.h
   using the NotCurses library.
*/

#define _XOPEN_SOURCE_EXTENDED 1

#include "cm.h"
#include "ui_backend.h"
#include "ui_notcurses_compat.h"
#include "ui_notcurses_internal.h"
#include <errno.h>
#include <inttypes.h>
#include <locale.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

UiRuntime *ui = NULL;
uint ui_color_cnt = 0;
uint ui_pair_cnt = 0;
UiPair *ui_pair;
UiColor *ui_color;
UiSurface *stdsfc;
uint LINES, COLS;
UiRuntime *ui;
UiConfig *ui_config;
UiSurface *ui_surface[UI_SFC_MAX];
UiCell bkgd_cell;

int sfc_ptr = -1;
int win_ptr = -1;

NcPlane *stdplane;

/** BOX WIDE UNICODE CODEPOINTS */

wchar_t *border_single = L"─│├┤┬┴┼┌┐└┘";
wchar_t *border_rounded = L"─│├┤┬┴┼╭╮╰╯";
wchar_t *border_double = L"═║╠╣╦╩╬╔╗╚╝";
wchar_t *border_heavy = L"━┃┣┫┳┻╋┏┓┗┛";
wchar_t *border_none = L"           ";

BorderWide bw;

const wchar_t *bw_rtl = L"\x256d"; /**< rounded top left */
const wchar_t *bw_rtr = L"\x256e"; /**< rounded top right */
const wchar_t *bw_rbl = L"\x2570"; /**< rounded bottom left */
const wchar_t *bw_rbr = L"\x256f"; /**< rounded bottom right */
const wchar_t *bw_sp = L"\x20";    /**< space */
const wchar_t *bw_ra = L"\x2192";  /**< large right arrow */
const wchar_t *bw_la = L"\x2190";  /**< large left arrow */
const wchar_t *bw_ua = L"\x2191";  /**< large up arrow */
const wchar_t *bw_da = L"\x2193";  /**< large down arrow */
const wchar_t *bw_ran = L"\x276F"; /**< right_angle */
const wchar_t *bw_lan = L"\x276E"; /**< left_angle */
const wchar_t *bw_chk = L"\x2611"; /**< left_angle */
const wchar_t *bw_h09 = L"\x23BD"; /**< horizontal line 9 */

STDRGB std_color[] = {{0, 0, 0}, {128, 0, 0}, {0, 128, 0}, {128, 128, 0}, {0, 0, 128}, {128, 0, 128}, {0, 128, 128}, {192, 192, 192}, {128, 128, 128}, {255, 0, 0}, {0, 255, 0}, {255, 255, 0}, {0, 0, 255}, {255, 0, 255}, {0, 255, 255}, {255, 255, 255}};

/* -------------------------------------------------------------------------
   Backend identification and capability query
   ------------------------------------------------------------------------- */

UiBackend ui_get_backend() {
    return UI_BACKEND_NOTCURSES;
}

void ui_get_caps(UiCaps *caps) {
    if (!caps)
        return;
    memset(caps, 0, sizeof(*caps));
    if (!ui)
        return;
    // I don't know that it isn't truecolor
    caps->truecolor = true;
    caps->palette256 = true;
    caps->mouse = ui->mouse_enabled;
    caps->unicode = true;
    caps->resize = true;
    caps->color_pairs = 0;
}
/* -------------------------------------------------------------------------
   Lifecycle
   ------------------------------------------------------------------------- */
UiRuntime *ui_init(const UiConfig *cfg) {
    setlocale(LC_ALL, "");
    ui = calloc(1, sizeof(*ui));
    if (!ui)
        return NULL;
    char tty_name[MAXLEN];
    if (cfg && cfg->tty_path) {
        strncpy(tty_name, cfg->tty_path, sizeof(tty_name) - 1);
        tty_name[sizeof(tty_name) - 1] = '\0';
    } else {
        if (ttyname_r(STDERR_FILENO, tty_name,
                      sizeof(tty_name)) != 0) {
            free(ui);
            ui = NULL;
            return NULL;
        }
    }
    ui->tty_fp = fopen(tty_name, "r+");
    if (ui->tty_fp == NULL) {
        ui = NULL;
        free(ui);
        return NULL;
    }
    NotCursesOptions nc_opts = {
        .flags = NCOPTION_SUPPRESS_BANNERS |
                 NCOPTION_NO_QUIT_SIGHANDLERS
        //               NCOPTION_PRESERVE_CURSOR,
        //      .loglevel = NCLOGLEVEL_SILENT,
    };
    ui->nc = notcurses_init(&nc_opts, ui->tty_fp);
    if (ui->nc == NULL) {
        free(ui);
        ui = NULL;
        return NULL;
    }
    f_curses_open = true;
    stdplane = notcurses_stdplane(ui->nc);
    notcurses_render(ui->nc);
    if (ui->nc == NULL) {
        fclose(ui->tty_fp);
        ui->tty_fp = NULL;
        free(ui);
        ui = NULL;
        return NULL;
    }
    if (cfg) {
        ui->mouse_enabled = cfg->enable_mouse;
        ui->alt_screen = cfg->enable_alt_screen;
        ui->cursor_visible = cfg->cursor_visible;
        ui->border_style = cfg->border_style;
    } else {
        ui->cursor_visible = false;
        ui->alt_screen = false;
    }
    //     if (ui->mouse_enabled)
    //         notcurses_mice_enable(ui->nc, NCMICE_ALL_EVENTS);
    if (!ui->cursor_visible)
        notcurses_cursor_disable(ui->nc);
    notcurses_stddim_yx(ui->nc, &ui->lines, &ui->cols);
    stdsfc = calloc(1, sizeof(*stdsfc));
    if (!stdsfc)
        return NULL;
    for (int i = 0; i < SUB_SFC_MAX; i++) {
        stdsfc->mplane[i] = NULL;
        stdsfc->meta[i].lines = 0;
        stdsfc->meta[i].cols = 0;
        stdsfc->meta[i].y = 0;
        stdsfc->meta[i].x = 0;
        stdsfc->meta[i].hidden = false;
    }
    stdsfc->runtime = ui;
    stdsfc->parent = NULL;
    stdsfc->meta[BOX].lines = ui->lines;
    stdsfc->meta[BOX].cols = ui->cols;
    stdsfc->meta[BOX].y = 0;
    stdsfc->meta[BOX].x = 0;
    stdsfc->mplane[BOX] = stdplane;
    if (!stdsfc->mplane[BOX]) {
        notcurses_stop(ui->nc);
        return NULL;
    }
    sfc_ptr = -1;
    LINES = ui->lines;
    COLS = ui->cols;
    ui_pair = calloc(UI_PAIRS, sizeof(UiPair));
    if (!ui_pair) {
        free(ui_pair);
        notcurses_stop(ui->nc);
        free(ui);
        return NULL;
    }
    ui_color = calloc(UI_COLORS, sizeof(UiColor));
    if (!ui_color) {
        free(ui_pair);
        notcurses_stop(ui->nc);
        free(ui);
        return NULL;
    }
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
    ui_color_cnt = 0;
    ui_pair_cnt = 0;
    return ui;
}

void ui_endwin() {
    // if (ui == NULL)
    //     return;
    // if (ui->nc) {
    //     if (ui->mouse_enabled)
    //         notcurses_mice_disable(ui->nc);
    //     notcurses_stop(ui->nc);
    // }
    // free(ui);
    // ui = NULL;
}

void ui_shutdown() {
    if (ui == NULL || f_curses_open == false)
        return;
    if (ui->nc) {
        if (ui->mouse_enabled)
            notcurses_mice_disable(ui->nc);
        notcurses_stop(ui->nc);
        f_curses_open = false;
    }
    free(ui);
    ui = NULL;
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
    s->runtime = ui;
    s->parent = parent;
    ncplane_options plane_opts = {
        .y = y,
        .x = x,
        .rows = lines,
        .cols = cols};
    s->meta[w].y = y;
    s->meta[w].x = x;
    s->meta[w].lines = lines;
    s->meta[w].cols = cols;
    s->meta[w].hidden = false;
    if (parent && parent->mplane[p]) {
        s->mplane[w] = ncplane_create(parent->mplane[p], &plane_opts);
    } else {
        stdplane = notcurses_stdplane(ui->nc);
        s->mplane[w] = ncplane_create(stdplane, &plane_opts);
    }
    if (!s->mplane[w]) {
        notcurses_stop(ui->nc);
        return NULL;
    }
    return s;
}

UiSurface *ui_box_surface_new(UiSurface *parent, uint p, uint lines, uint cols, uint y, uint x, char *wtitle) {
    if (!ui)
        return NULL;
    UiSurface *s = calloc(1, sizeof(*s));
    if (!s)
        return NULL;
    s->runtime = ui;
    s->parent = parent;
    ncplane_options plane_opts = {
        .y = y,
        .x = x,
        .rows = lines + 2,
        .cols = cols + 2,
        .name = NULL};
    s->meta[BOX].y = y;
    s->meta[BOX].x = x;
    s->meta[BOX].lines = lines + 2;
    s->meta[BOX].cols = cols + 2;
    s->meta[BOX].hidden = false;
    strnz__cpy(s->meta[BOX].name, "BOX", XLEN - 1);
    if (parent && parent->mplane[p]) {
        s->mplane[BOX] = ncplane_create(parent->mplane[p], &plane_opts);
    } else {
        stdplane = notcurses_stdplane(ui->nc);
        s->mplane[BOX] = ncplane_create(stdplane, &plane_opts);
    }
    if (!s->mplane[BOX]) {
        free(s);
        return NULL;
    }
    ncplane_set_base(s->mplane[BOX],
                     " ",
                     0,
                     cell_box.channels);
    ncplane_set_channels(s->mplane[BOX], cell_box.channels);
    border_draw(s);
    border_title(s, wtitle);
    return s;
}

int ui_surface_addpad(UiSurface *s, uint w, uint p, int lines, int cols) {
    uint y = 0, x = 0;
    // if (s->mplane[p])
    //     ncplane_dim_yx(s->mplane[p], &y, &x);
    ncplane_options plane_opts = {
        .y = y,
        .x = x,
        .rows = lines,
        .cols = cols,
        .name = NULL};
    s->meta[w].y = y;
    s->meta[w].x = x;
    s->meta[w].lines = lines;
    s->meta[w].cols = cols;
    s->meta[w].hidden = false;
    s->mplane[w] = ncplane_create(s->mplane[p], &plane_opts);
    if (!s->mplane[w]) {
        notcurses_stop(ui->nc);
        return -1;
    }
    ncplane_set_base(s->mplane[w], " ", 0, cell_nt.channels);
    ncplane_set_channels(s->mplane[w], cell_nt.channels);
    ui_keypad(s, w, false);
    ui_bkgd(s, w, &cell_nt);
    ui_bkgdset(s, w, &cell_nt);
    return 0;
}

int ui_surface_addwin(UiSurface *s, uint w, uint p, uint lines, uint cols, uint y, uint x) {
    ncplane_options plane_opts = {
        .y = y,
        .x = x,
        .rows = lines,
        .cols = cols,
        .name = NULL};
    s->meta[w].y = y;
    s->meta[w].x = x;
    s->meta[w].lines = lines;
    s->meta[w].cols = cols;
    s->meta[w].hidden = false;
    s->mplane[w] = ncplane_create(s->mplane[p], &plane_opts);
    if (!s->mplane[w]) {
        notcurses_stop(ui->nc);
        return -1;
    }
    ncplane_set_base(s->mplane[w], " ", 0, cell_nt.channels);
    ncplane_set_channels(s->mplane[w], cell_nt.channels);
    ui_bkgd(s, w, &cell_nt);
    ui_bkgdset(s, w, &cell_nt);
    return 0;
}

void ui_surface_destroy(UiSurface *s) {
    if (!s)
        return;
    for (int w = SUB_SFC_MAX; w >= 0; w--)
        if (s->mplane[w] != NULL) {
            ncplane_erase(s->mplane[w]);
            ncplane_destroy(s->mplane[w]);
            s->mplane[w] = NULL;
        }
    if (s != NULL) {
        free(s);
        s = NULL;
    }
}
/* -------------------------------------------------------------------------
   Configuration Control
   ------------------------------------------------------------------------- */

int ui_scrollok(UiSurface *s, uint w, bool enable) {
    if (!s)
        return -1;
    ncplane_set_scrolling(s->mplane[w], enable);
    return 0;
}
int ui_idcok(UiSurface *s, uint w, bool enable) {
    (void)s;
    (void)w;
    (void)enable;
    return 0;
}
int ui_idlok(UiSurface *s, uint w, bool enable) {
    (void)s;
    (void)w;
    (void)enable;
    return 0;
}
int ui_setscrreg(UiSurface *s, uint w, uint top, uint bottom) {
    (void)s;
    (void)w;
    (void)top;
    (void)bottom;
    return 0;
}
int ui_keypad(UiSurface *s, uint w, bool enable) {
    (void)s;
    (void)w;
    (void)enable;
    return 0;
}
/* -------------------------------------------------------------------------
   Screen management functions
   ------------------------------------------------------------------------- */

int ui_mousemask(int mask) {
    if (!ui)
        return 0;
    if (mask)
        notcurses_mice_enable(ui->nc, mask);
    else
        notcurses_mice_enable(ui->nc, NCMICE_ALL_EVENTS);
    return 0;
}

int ui_mice_enable(int mask) {
    if (mask)
        notcurses_mice_enable(ui->nc, mask);
    else
        notcurses_mice_enable(ui->nc, NCMICE_ALL_EVENTS);
    return 0;
}

void ui_get_screen_size(uint *lines, uint *cols) {
    if (!ui)
        return;
    unsigned int r = 0, c = 0;
    notcurses_stddim_yx(ui->nc, &r, &c);
    ui->lines = (int)r;
    ui->cols = (int)c;
    if (lines)
        *lines = ui->lines;
    if (cols)
        *cols = ui->cols;
}
void ui_update_panels() {
    if (!ui)
        return;
    notcurses_render(ui->nc);
}

void ui_render() {
    if (!ui)
        return;
    notcurses_render(ui->nc);
}

int ui_suspend() {
    if (!ui)
        return -1;
    notcurses_leave_alternate_screen(ui->nc);
    return 0;
}

int ui_resume() {
    if (!ui)
        return -1;
    notcurses_enter_alternate_screen(ui->nc);
    notcurses_render(ui->nc);
    return 0;
}

int ui_curs_set(int visible) {
    if (!ui)
        return -1;
    if (visible == 0) {
        ui->cursor_visible = false;
        notcurses_cursor_disable(ui->nc);
    } else {
        ui->cursor_visible = true;
        int y, x;
        notcurses_cursor_yx(ui->nc, &y, &x);
        int rc = notcurses_cursor_enable(ui->nc, y, x);
        return rc;
    }
    return 0;
}
/** @brief Disable (visible = false) or enable (visibile = true) the cursor at a
 * specified position on the surface and plane specified.
 * Include workaround for NotCurses cursor position bug
 */
int ui_cursor_enable_yx(UiSurface *s, uint w, uint y, uint x, bool visible) {
    if (!s)
        return -1;
    if (!visible) {
        ui->cursor_visible = false;
        notcurses_cursor_disable(ui->nc);
    } else {
        int yy, xx;
        ncplane_abs_yx(s->mplane[w], &yy, &xx);
        y += yy;
        x += xx;
        ui->cursor_visible = true;
        int rc = notcurses_cursor_enable(ui->nc, y, x);
        return rc;
    }
    return 0;
}
/** @brief Disable (visible = false) or enable (visibile = true) the cursor at
 * its current position on the surface and plane specified.
 */
int ui_cursor_enable(UiSurface *s, uint w, bool visible) {
    if (!s)
        return -1;
    if (!visible) {
        ui->cursor_visible = false;
        notcurses_cursor_disable(ui->nc);
    } else {
        uint y, x;
        ncplane_cursor_yx(s->mplane[w], &y, &x);
        int yy, xx;
        ncplane_abs_yx(s->mplane[w], &yy, &xx);
        y += yy;
        x += xx;
        ui->cursor_visible = true;
        return notcurses_cursor_enable(ui->nc, y, x) == 0 ? 0 : -1;
    }
    return 0;
}
/* -------------------------------------------------------------------------
   Surface management
   ------------------------------------------------------------------------- */
int ui_surface_move(UiSurface *s, uint w, uint y, uint x) {
    if (!s)
        return -1;
    s->meta[w].y = y;
    s->meta[w].x = x;
    if (!s->meta[w].hidden)
        return ncplane_move_yx(s->mplane[w], y, x) == 0 ? 0 : -1;
    return 0;
}

int ui_surface_resize(UiSurface *s, uint w, uint lines, uint cols) {
    if (!s)
        return -1;
    return ncplane_resize_simple(s->mplane[w], (unsigned int)lines,
                                 (unsigned int)cols) == 0
               ? 0
               : -1;
}

int ui_clear() {
    if (!stdplane)
        return -1;
    ncplane_erase_region(stdplane, 0, 0, 0, 0);
    return 0;
}
int ui_erase() {
    if (!stdplane)
        return -1;
    ncplane_erase_region(stdplane, 0, 0, 0, 0);
    return 0;
}
int ui_werase(UiSurface *s, uint w) {
    if (!s)
        return -1;
    ncplane_erase_region(s->mplane[w], 0, 0, 0, 0);
    return 0;
}
int ui_wclear(UiSurface *s, uint w) {
    if (!s)
        return -1;
    ncplane_erase_region(s->mplane[w], 0, 0, 0, 0);
    return 0;
}

int ui_surface_show(UiSurface *s, uint w) {
    if (!s)
        return -1;
    if (s->meta[w].hidden) {
        s->meta[w].hidden = false;
        ncplane_move_yx(s->mplane[w], s->meta[w].y, s->meta[w].x);
    }
    return 0;
}

int ui_surface_hide(UiSurface *s, uint w) {
    if (!s)
        return -1;
    if (!s->meta[w].hidden) {
        s->meta[w].hidden = true;
        /* Move far off-screen so the plane does not obscure anything. */
        ncplane_move_yx(s->mplane[w], -s->meta[w].lines - 1, 0);
    }
    return 0;
}

int ui_wmove(UiSurface *s, uint w, uint y, uint x) {
    if (!s)
        return -1;
    if (ncplane_cursor_move_yx(s->mplane[w], y, x) != 0)
        return -1;
    return 0;
}
int ui_cursor_move(UiSurface *s, uint w, uint y, uint x) {
    if (!s)
        return -1;
    if (ncplane_cursor_move_yx(s->mplane[w], y, x) != 0)
        return -1;
    return 0;
}
void ui_getyx(UiSurface *s, uint w, uint *y, uint *x) {
    if (!s)
        return;
    ncplane_cursor_yx(s->mplane[w], y, x);
}
void ui_getmaxyx(UiSurface *s, uint w, uint *y, uint *x) {
    if (!s)
        return;
    ncplane_dim_yx(s->mplane[w], y, x);
}
uint ui_getmaxy(UiSurface *s, uint w) {
    if (!s)
        return -1;
    uint y, x;
    ncplane_dim_yx(s->mplane[w], &y, &x);
    return y;
}
uint ui_getmaxx(UiSurface *s, uint w) {
    if (!s)
        return -1;
    uint y, x;
    ncplane_dim_yx(s->mplane[w], &y, &x);
    return x;
}
int ui_wscrl(UiSurface *s, uint w, uint r) {
    if (!r)
        return -1;
    ncplane_scrollup(s->mplane[w], r);
    ui_render();
    return 0;
}
int ui_top_panel(UiSurface *s, uint w) {
    (void)w;
    if (!s)
        return -1;
    return 0;
}
int ui_wnoutrefresh(UiSurface *s, uint w) {
    (void)w;
    if (!s)
        return -1;
    return 0;
}
/* -------------------------------------------------------------------------
   Cell Manipulation
   ------------------------------------------------------------------------- */

int ui_getcchar(const UiCell *uic, wchar_t *wc, UiStyle *style, UiPairIdx *pair, const void *opts) {
    (void)opts;
    if (!uic || !wc || !style || !pair)
        return -1;
    GCluster gcluster;
    gcluster.u32 = uic->gcluster;
    wc = gcluster.u16;
    *style = uic->stylemask;
    // Convert the channels to a color pair index
    UiChannels channels;
    channels.fb = uic->channels;
    RGB rgb;
    rgb.r = channels.f_r;
    rgb.g = channels.f_g;
    rgb.b = channels.f_b;
    // If the color index does not exist, create a new one
    int fg = ui_add_color_rgb(&rgb);
    rgb.r = channels.b_r;
    rgb.g = channels.b_g;
    rgb.b = channels.b_b;
    // If the color index does not exist, create a new one
    int bg = ui_add_color_rgb(&rgb);
    // If the color pair does not exist, create a new one
    *pair = ui_add_pair(fg, bg);
    return 0;
}

int ui_setcchar(UiCell *cell, const wchar_t *wstr, const uint16_t style, ushort pair, const void *opts) {
    (void)opts;
    if (!cell)
        return -1;
    uint32_t fg, bg;
    GCluster *gc = (GCluster *)&cell->gcluster;
    gc->u16[0] = wstr[0];
    gc->u16[1] = wstr[1];
    cell->width = wcwidth((wchar_t)*wstr);
    nccell_set_styles(cell, style);
    UiChannels *chan = (UiChannels *)&cell->channels;
    ui_get_pair(pair, &fg, &bg);
    ui_color_content(fg, &chan->f_r, &chan->f_g, &chan->f_b);
    chan->f_a = 0x40;
    ui_color_content(bg, &chan->b_r, &chan->b_g, &chan->b_b);
    chan->b_a = 0x40;
    return 0;
}

UiCell ui_cell_from_ucp(const wchar_t *ucp, const uint32_t *fg, const uint32_t *bg) {
    nccell cell;
    nccell_init(&cell);
    GCluster *gc = (GCluster *)&cell.gcluster;
    wchar_t wstr[2] = {ucp[0], L'\0'};
    // wcwidth expects Unicode codepoints, not UTF-8 encoded bytes
    cell.width = wcwidth(wstr[0]);
    // unicode_to_utf8_gcluster converts Unicode codepoints to UTF-8
    unicode_to_utf8_gcluster(wstr[0], gc);
    nccell_set_styles(&cell, WA_NORMAL);
    UiChannels *chan = (UiChannels *)&cell.channels;
    chan->bargb = *bg;
    chan->fargb = *fg;
    chan->b_a = chan->f_a = 0x40;
    return cell;
}

/* -------------------------------------------------------------------------
   Colors, Color Pairs
   ------------------------------------------------------------------------- */

int ui_add_color_rgb(RGB *rgb) {
    uint i;
    RGB tmp;
    apply_gamma(rgb);
    for (i = 0; i < ui_color_cnt && i < UI_COLORS; i++) {
        ui_color_content(i, &tmp.r, &tmp.g, &tmp.b);
        if (rgb->r == tmp.r && rgb->g == tmp.g && rgb->b == tmp.b)
            return i;
    }
    if (i < UI_COLORS) {
        if (i < 16) {
            std_color[i].r = rgb->r;
            std_color[i].g = rgb->g;
            std_color[i].b = rgb->b;
        }
        ui_init_color(i, rgb->r, rgb->g, rgb->b);
        if (ui_color_cnt + 1 < UI_COLORS)
            ui_color_cnt++;
        return ui_color_cnt - 1;
    }
    return -1;
}
/* ------------------------------------------------------------------------- */
uint ui_add_pair(uint fg, uint bg) {
    uint16_t i;
    for (i = 1; i < ui_pair_cnt; i++) {
        if (ui_pair[i].fg == fg && ui_pair[i].bg == bg)
            return i;
    }
    if (i + 1 >= UI_PAIRS) {
        ssnprintf(em0, MAXLEN - 1, "%s, line: %d", __FILE__, __LINE__ - 1);
        ssnprintf(em1, MAXLEN - 1, "NotCurses COLOR_PAIRS (%d) exceeded (%d)",
                  UI_PAIRS, i);
        strerror_r(errno, em2, MAXLEN);
        display_error(em0, em1, em2, nullptr);
        return (EXIT_FAILURE);
    }
    if (i < UI_PAIRS) {
        ui_pair[i].fg = fg;
        ui_pair[i].bg = bg;
        ui_pair_cnt++;
    }
    return ui_pair_cnt - 1;
}
int ui_get_pair(uint16_t pair, uint *fg, uint *bg) {
    *fg = ui_pair[pair].fg;
    *bg = ui_pair[pair].bg;
    return 0;
}

uint64_t ui_get_channels_from_pair(uint16_t pair) {
    uint fg, bg;
    // ui_channels is a struct that holds the foreground and background color
    // channels
    // use channels.fb to get the combined foreground and background color channels
    UiChannels ui_channels = {0};
    ui_get_pair(pair, &fg, &bg);
    ui_color_content(fg, &ui_channels.f_r, &ui_channels.f_g, &ui_channels.f_b);
    ui_channels.f_a = 0x40;
    ui_channels.f_r = ui_color[fg].r;
    ui_channels.f_g = ui_color[fg].g;
    ui_channels.f_b = ui_color[fg].b;
    bg = ui_pair[pair].bg;
    ui_color_content(bg, &ui_channels.b_r, &ui_channels.b_g, &ui_channels.b_b);
    ui_channels.b_a = 0x40;
    ui_channels.b_r = ui_color[bg].r;
    ui_channels.b_g = ui_color[bg].g;
    ui_channels.b_b = ui_color[bg].b;
    return ui_channels.fb;
}

uint ui_init_color_hex(char *s) {
    RGB rgb;
    rgb = ui_hex_to_rgb(s);
    apply_gamma(&rgb);
    uint i;
    for (i = 0; i < ui_color_cnt && i < UI_COLORS; i++) {
        if (rgb.r == ui_color[i].r && rgb.g == ui_color[i].g && rgb.b == ui_color[i].b)
            return i;
    }
    if (i < UI_COLORS) {
        if (i < 16) {
            std_color[i].r = rgb.r;
            std_color[i].g = rgb.g;
            std_color[i].b = rgb.b;
        }
        ui_color[i].r = rgb.r;
        ui_color[i].g = rgb.g;
        ui_color[i].b = rgb.b;
        if (ui_color_cnt + 1 < UI_COLORS)
            ui_color_cnt++;
        return ui_color_cnt - 1;
    }
    return 0;
}

RGB ui_hex_to_rgb(char *s) {
    RGB rgb;
    sscanf(s, "#%02hhX%02hhX%02hhX", &rgb.r, &rgb.g, &rgb.b);
    return rgb;
}

int ui_color_content(uint16_t color, uint8_t *r, uint8_t *g, uint8_t *b) {
    if (color + 1 >= UI_COLORS)
        return -1;
    *r = ui_color[color].r;
    *g = ui_color[color].g;
    *b = ui_color[color].b;
    return 0;
}

int ui_init_color(uint16_t color, uint8_t r, uint8_t g, uint8_t b) {
    if (color + 1 >= UI_COLORS)
        return -1;
    ui_color[color].r = r;
    ui_color[color].g = g;
    ui_color[color].b = b;
    return 0;
}

int ui_pair_content(uint16_t pair, uint *fg, uint *bg) {
    if (pair + 1 >= UI_PAIRS)
        return -1;
    *fg = ui_pair[pair].fg;
    *bg = ui_pair[pair].bg;
    return 0;
}

int ui_init_pair(uint16_t pair, uint fg, uint bg) {
    if (pair + 1 >= UI_PAIRS)
        return -1;
    ui_pair[pair].fg = fg;
    ui_pair[pair].bg = bg;
    return 0;
}

int ui_chg_color(uint16_t color_idx, uint32_t *color) {
    RGB rgb;
    rgb.color = *color;
    if (color_idx + 1 >= UI_COLORS)
        return -1;
    apply_gamma(&rgb);
    if (color_idx < 16) {
        std_color[color_idx].r = rgb.r;
        std_color[color_idx].g = rgb.g;
        std_color[color_idx].b = rgb.b;
    }
    ui_color[color_idx].r = rgb.r;
    ui_color[color_idx].g = rgb.g;
    ui_color[color_idx].b = rgb.b;
    return 0;
}
/* -------------------------------------------------------------------------
   Style helpers (shared with draw and input modules)
   ------------------------------------------------------------------------- */

int ui_bkgd(UiSurface *s, uint w, const UiCell *cell) {
    if (!s)
        return -1;
    ncplane_set_styles(s->mplane[w], cell->stylemask);
    ncplane_set_channels(s->mplane[w], cell_box.channels);
    ncplane_set_base(s->mplane[w], " ",
                     cell->stylemask, cell->channels);
    return 0;
}

int ui_bkgdset(UiSurface *s, uint w, const UiCell *cell) {
    if (!s)
        return -1;
    ncplane_set_styles(s->mplane[w], cell->stylemask);
    ncplane_set_channels(s->mplane[w], cell->channels);
    bkgd_cell = *cell;
    return 0;
}
int ui_bkgrnd(UiSurface *s, uint w, const UiCell *cell) {
    if (!s)
        return -1;
    ncplane_set_styles(s->mplane[w], cell->stylemask);
    ncplane_set_channels(s->mplane[w], cell->channels);
    ncplane_set_base(s->mplane[w], " ",
                     cell->stylemask, cell->channels);
    return 0;
}

int ui_bkgrndset(UiSurface *s, uint w, const UiCell *cell) {
    if (!s)
        return -1;
    ncplane_set_styles(s->mplane[w], cell->stylemask);
    ncplane_set_channels(s->mplane[w], cell->channels);
    return 0;
}
/* -------------------------------------------------------------------------
   Non-portable escape-hatch getters (see ui_notcurses_compat.h)
   ------------------------------------------------------------------------- */

struct notcurses *ui_notcurses_get_nc() {
    if (!ui)
        return NULL;
    return ui->nc;
}

NcPlane *ui_notcurses_surface_get_plane(const UiSurface *s, uint w) {
    (void)w;
    if (!s)
        return NULL;
    return s->mplane[w];
}
