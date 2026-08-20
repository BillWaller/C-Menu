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
#include <locale.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef uint16_t UiPairIdx;
typedef struct NcPlane NcPlane;
uint ui_color_cnt = 0;
uint ui_pair_cnt = 0;
UiPair *ui_pair;
UiColor *ui_color;
UiSurface *stdsfc;
unsigned int LINES, COLS;

STDRGB std_color[16] = {{0, 0, 0}, {128, 0, 0}, {0, 128, 0}, {128, 128, 0}, {0, 0, 128}, {128, 0, 128}, {0, 128, 128}, {192, 192, 192}, {128, 128, 128}, {255, 0, 0}, {0, 255, 0}, {255, 255, 0}, {0, 0, 255}, {255, 0, 255}, {0, 255, 255}, {255, 255, 255}};

/* -------------------------------------------------------------------------
   Backend identification and capability query
   ------------------------------------------------------------------------- */

UiBackend ui_get_backend(const UiRuntime *ui) {
    (void)ui;
    return UI_BACKEND_NOTCURSES;
}

void ui_get_caps(const UiRuntime *ui, UiCaps *caps) {
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
    UiRuntime *ui = calloc(1, sizeof(*ui));
    if (!ui)
        return NULL;
    if (cfg) {
        ui->mouse_enabled = cfg->enable_mouse;
        ui->alt_screen = cfg->enable_alt_screen;
        ui->cursor_visible = cfg->cursor_visible;
    } else {
        ui->cursor_visible = false;
        ui->alt_screen = false;
    }
    FILE *tty = NULL;
    if (cfg && cfg->tty_path) {
        tty = fopen(cfg->tty_path, "r+");
        if (!tty) {
            free(ui);
            return NULL;
        }
    }
    struct notcurses_options opts = {
        .flags = NCOPTION_SUPPRESS_BANNERS | (ui->alt_screen ? 0 : NCOPTION_NO_ALTERNATE_SCREEN),
    };
    ui->nc = notcurses_init(&opts, tty);
    if (!ui->nc) {
        if (tty)
            fclose(tty);
        free(ui);
        return NULL;
    }
    if (ui->mouse_enabled)
        notcurses_mice_enable(ui->nc, NCMICE_ALL_EVENTS);
    if (!ui->cursor_visible)
        notcurses_cursor_disable(ui->nc);
    notcurses_stddim_yx(ui->nc, &LINES, &COLS);

    stdsfc = calloc(1, sizeof(*stdsfc));
    if (!stdsfc)
        return NULL;
    stdsfc->runtime = ui;
    stdsfc->parent = NULL;
    stdsfc->lines = LINES;
    stdsfc->cols = COLS;
    stdsfc->y = 0;
    stdsfc->x = 0;
    struct ncplane *stdn = notcurses_stdplane(ui->nc);
    stdsfc->mplane[WIN] = stdn;
    if (!stdsfc->mplane[WIN]) {
        notcurses_stop(ui->nc);
        return NULL;
    }
    ncplane_erase(stdn);

    ui->lines = LINES;
    ui->cols = COLS;
    ui_color = calloc(UI_COLORS, sizeof(UiColor));
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
    ui_color_cnt = 0;
    ui_pair_cnt = 0;
    return ui;
}

void ui_endwin() {
    UiRuntime *ui = ui_runtime;
    if (!ui)
        return;
    if (ui->nc) {
        if (ui->mouse_enabled)
            notcurses_mice_disable(ui->nc);
        notcurses_stop(ui->nc);
    }
    free(ui);
}

void ui_shutdown(UiRuntime *ui) {
    if (!ui)
        return;
    if (ui->nc) {
        if (ui->mouse_enabled)
            notcurses_mice_disable(ui->nc);
        notcurses_stop(ui->nc);
    }
    free(ui);
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

int ui_get_channels_from_pair(uint16_t pair, UiChannels *channels) {
    uint fg, bg;
    ui_get_pair(pair, &fg, &bg);
    ui_color_content(fg, &channels->f_r, &channels->f_g, &channels->f_b);
    channels->f_a = 0x40;
    channels->f_r = ui_color[fg].r;
    channels->f_g = ui_color[fg].g;
    channels->f_b = ui_color[fg].b;
    bg = ui_pair[pair].bg;
    ui_color_content(bg, &channels->b_r, &channels->b_g, &channels->b_b);
    channels->b_a = 0x40;
    channels->b_r = ui_color[bg].r;
    channels->b_g = ui_color[bg].g;
    channels->b_b = ui_color[bg].b;
    return 0;
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

/* -------------------------------------------------------------------------
   Screen management functions
   ------------------------------------------------------------------------- */

int ui_mousemask(int mask) {
    if (!ui_runtime)
        return 0;
    if (mask)
        notcurses_mice_enable(ui_runtime->nc, mask);
    else
        notcurses_mice_enable(ui_runtime->nc, NCMICE_ALL_EVENTS);
    return 0;
}

int ui_mice_enable(int mask) {
    if (mask)
        notcurses_mice_enable(ui_runtime->nc, mask);
    else
        notcurses_mice_enable(ui_runtime->nc, NCMICE_ALL_EVENTS);
    return 0;
}

void ui_get_screen_size(UiRuntime *ui, uint *lines, uint *cols) {
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
    if (!ui_runtime)
        return;
    notcurses_render(ui_runtime->nc);
}

void ui_render() {
    if (!ui_runtime)
        return;
    notcurses_render(ui_runtime->nc);
}

int ui_suspend(UiRuntime *ui) {
    if (!ui)
        return -1;
    notcurses_leave_alternate_screen(ui->nc);
    return 0;
}

int ui_resume(UiRuntime *ui) {
    if (!ui)
        return -1;
    notcurses_enter_alternate_screen(ui->nc);
    notcurses_render(ui->nc);
    return 0;
}

int ui_curs_set(int visible) {
    if (!ui_runtime)
        return -1;
    if (visible == 0)
        ui_runtime->cursor_visible = false;
    else if (visible == 1)
        ui_runtime->cursor_visible = true;
    else
        return -1;
    if (visible)
        return notcurses_cursor_enable(ui_runtime->nc, 0, 0) == 0 ? 0 : -1;
    notcurses_cursor_disable(ui_runtime->nc);
    return 0;
}

int ui_cursor_enable(UiRuntime *ui, bool visible) {
    if (!ui)
        return -1;
    ui->cursor_visible = visible;
    if (visible)
        return notcurses_cursor_enable(ui->nc, 0, 0) == 0 ? 0 : -1;
    notcurses_cursor_disable(ui->nc);
    return 0;
}

/* -------------------------------------------------------------------------
   Surface Creation and Destruction
   ------------------------------------------------------------------------- */

UiSurface *ui_surface_new(UiRuntime *ui, uint w, UiSurface *parent, uint p, uint lines, uint cols, uint y, uint x) {
    if (!ui)
        return NULL;
    UiSurface *s = calloc(1, sizeof(*s));
    if (!s)
        return NULL;
    s->runtime = ui;
    s->parent = parent;
    s->lines = lines;
    s->cols = cols;
    s->y = y;
    s->x = x;
    ncplane_options plane_opts = {
        .y = y,
        .x = x,
        .rows = lines,
        .cols = cols};
    if (parent && parent->mplane[p]) {
        s->mplane[w] = ncplane_create(parent->mplane[p], &plane_opts);
    } else {
        struct ncplane *stdn = notcurses_stdplane(ui->nc);
        s->mplane[w] = ncplane_create(stdn, &plane_opts);
    }
    if (!s->mplane[w]) {
        notcurses_stop(ui->nc);
        return NULL;
    }
    return s;
}

UiSurface *ui_box_surface_new(UiRuntime *ui, UiSurface *parent, uint p, uint lines, uint cols, uint y, uint x, char *wtitle) {
    if (!ui)
        return NULL;
    UiSurface *s = calloc(1, sizeof(*s));
    if (!s)
        return NULL;
    s->runtime = ui;
    s->parent = parent;
    s->lines = lines;
    s->cols = cols;
    s->y = y;
    s->x = x;

    ncplane_options plane_opts = {
        .y = y,
        .x = x,
        .rows = lines + 2,
        .cols = cols + 2,
        .name = NULL};
    if (parent && parent->mplane[p]) {
        s->mplane[BOX] = ncplane_create(parent->mplane[p], &plane_opts);
    } else {
        struct ncplane *stdn = notcurses_stdplane(ui->nc);
        s->mplane[BOX] = ncplane_create(stdn, &plane_opts);
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
    ncplane_perimeter_rounded(s->mplane[BOX], 0, cell_box.channels, 0);

    // Title
    int title_len = (int)strlen(wtitle);
    int title_x = (cols - title_len) / 2;
    ncplane_putwc_yx(s->mplane[BOX], 0, title_x, BW_RT);
    ncplane_putstr(s->mplane[BOX], " ");
    ncplane_putstr(s->mplane[BOX], wtitle);
    ncplane_putstr(s->mplane[BOX], " ");
    ncplane_putwc(s->mplane[BOX], BW_LT);

    // Content plane
    plane_opts.y = 1;
    plane_opts.x = 1;
    plane_opts.rows = lines;
    plane_opts.cols = cols;
    s->mplane[WIN] = ncplane_create(s->mplane[BOX], &plane_opts);
    if (!s->mplane[WIN]) {
        notcurses_stop(ui_runtime->nc);
        return NULL;
    }
    ncplane_set_base(s->mplane[WIN], " ", 0, cell_nt.channels);
    ncplane_set_channels(s->mplane[WIN], cell_nt.channels);
    return s;
}

int ui_surface_addpad(UiSurface *s, uint w, uint view_win, int lines, int cols) {
    if (!s->mplane[BOX])
        return -1;
    uint y, x;
    if (s->parent && s->parent->mplane[BOX])
        ncplane_dim_yx(s->mplane[BOX], &y, &x);
    ncplane_options plane_opts = {
        .y = y,
        .x = x,
        .rows = lines,
        .cols = cols,
        .name = NULL};
    s->mplane[w] = ncplane_create(s->mplane[PAD], &plane_opts);
    if (!s->mplane[PAD]) {
        notcurses_stop(ui_runtime->nc);
        return -1;
    }
    ncplane_set_base(s->mplane[PAD], " ", 0, cell_nt.channels);
    ncplane_set_channels(s->mplane[PAD], cell_nt.channels);
    ui_keypad(s, PAD, true);
    ui_bkgd(s, PAD, &cell_nt);
    ui_bkgdset(s, PAD, &cell_nt);
    return 0;
}

int ui_surface_addwin(UiSurface *s, uint w, uint p, uint lines, uint cols, uint y, uint x) {
    ncplane_options plane_opts = {
        .y = y,
        .x = x,
        .rows = lines + 2,
        .cols = cols + 2,
        .name = NULL};
    s->mplane[w] = ncplane_create(s->mplane[p], &plane_opts);
    if (!s->mplane[w]) {
        notcurses_stop(ui_runtime->nc);
        return -1;
    }
    ncplane_set_base(s->mplane[w], " ", 0, cell_nt.channels);
    ncplane_set_channels(s->mplane[w], cell_nt.channels);
    ui_keypad(s, w, true);
    ui_bkgd(s, w, &cell_nt);
    ui_bkgdset(s, w, &cell_nt);
    return 0;
}

void ui_surface_destroy(UiSurface *s) {
    if (!s)
        return;
    for (int w = 0; w < SUB_SFC_MAX; ++w)
        if (s->mplane[w])
            ncplane_destroy(s->mplane[w]);
    free(s);
}

int ui_surface_move(UiSurface *s, uint w, uint y, uint x) {
    if (!s)
        return -1;
    s->y = y;
    s->x = x;
    if (!s->hidden)
        return ncplane_move_yx(s->mplane[w], y, x) == 0 ? 0 : -1;
    return 0;
}

int ui_surface_resize(UiSurface *s, uint w, uint lines, uint cols) {
    if (!s)
        return -1;
    s->lines = lines;
    s->cols = cols;
    return ncplane_resize_simple(s->mplane[w], (unsigned int)lines,
                                 (unsigned int)cols) == 0
               ? 0
               : -1;
}

int ui_surface_clear(UiSurface *s, uint w) {
    if (!s)
        return -1;
    ncplane_erase(s->mplane[w]);
    return 0;
}
int ui_erase() {
    if (!stdsfc)
        return -1;
    ncplane_erase(stdsfc->mplane[BOX]);
    return 0;
}
int ui_werase(UiSurface *s, uint w) {
    if (!stdsfc)
        return -1;
    ncplane_erase(s->mplane[w]);
    return 0;
}
int ui_clear() {
    if (!stdsfc)
        return -1;
    ncplane_erase(stdsfc->mplane[BOX]);
    return 0;
}
int ui_surface_erase(UiSurface *s, uint w) {
    if (!s)
        return -1;
    ncplane_erase(s->mplane[w]);
    return 0;
}

int ui_surface_show(UiSurface *s, uint w) {
    if (!s)
        return -1;
    if (s->hidden) {
        s->hidden = false;
        ncplane_move_yx(s->mplane[w], s->y, s->x);
    }
    return 0;
}

int ui_surface_hide(UiSurface *s, uint w) {
    if (!s)
        return -1;
    if (!s->hidden) {
        s->hidden = true;
        /* Move far off-screen so the plane does not obscure anything. */
        ncplane_move_yx(s->mplane[w], -s->lines - 1, 0);
    }
    return 0;
}

int ui_wmove(UiSurface *s, uint w, uint y, uint x) {
    if (!s)
        return -1;
    return ncplane_cursor_move_yx(s->mplane[w], y, x) == 0 ? 0 : -1;
}
int ui_cursor_move(UiSurface *s, uint w, uint y, uint x) {
    if (!s)
        return -1;
    return ncplane_cursor_move_yx(s->mplane[w], y, x) == 0 ? 0 : -1;
}

void ui_getyx(UiSurface *s, uint w, uint *y, uint *x) {
    if (!s->mplane[w])
        return;
    ncplane_cursor_yx(s->mplane[w], y, x);
}
void ui_getmaxyx(UiSurface *s, uint w, uint *y, uint *x) {
    if (!s->mplane[w])
        return;
    ncplane_dim_yx(s->mplane[w], y, x);
}
uint ui_getmaxy(UiSurface *s, uint w) {
    if (!s->mplane[w])
        return -1;
    uint y, x;
    ncplane_dim_yx(s->mplane[w], &y, &x);
    return y;
}
uint ui_getmaxx(UiSurface *s, uint w) {
    if (!s->mplane[w])
        return -1;
    uint y, x;
    ncplane_dim_yx(s->mplane[w], &y, &x);
    return x;
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
    if (!s)
        return -1;
    (void)w;
    (void)enable;
    return 0;
}
int ui_idlok(UiSurface *s, uint w, bool enable) {
    if (!s)
        return -1;
    (void)w;
    (void)enable;
    return 0;
}
int ui_keypad(UiSurface *s, uint w, bool enable) {
    if (!s)
        return -1;
    (void)w;
    (void)enable;
    return 0;
}
/* -------------------------------------------------------------------------
   Style helpers (shared with draw and input modules)
   ------------------------------------------------------------------------- */

UiChannels ui_channels_from_hex(const char *fg, const char *bg) {
    UiChannels channels = {0};
    RGB rgb;
    sscanf(fg, "#%02hhX%02hhX%02hhX", &rgb.r, &rgb.g, &rgb.b);
    ncchannels_set_fg_rgb8(&channels.fb, rgb.r, rgb.g, rgb.b);
    sscanf(bg, "#%02hhX%02hhX%02hhX", &rgb.r, &rgb.g, &rgb.b);
    ncchannels_set_bg_rgb8(&channels.fb, rgb.r, rgb.g, rgb.b);
    return channels;
}

UiCell ui_cell_from_hex(const char *fg,
                        const char *bg,
                        const uint16_t *stylemask,
                        const wchar_t *wstr) {
    int i = 0;
    UiCell cell;
    GCluster gcluster = {0};
    while (wstr[i] != L'\0' && i < WCHAR_MAX_UTF8BYTES) {
        gcluster.wstr[i] = wstr[i];
        i++;
    }
    if (i < WCHAR_MAX_UTF8BYTES)
        gcluster.wstr[i] = L'\0';
    cell.gcluster = gcluster.gi32;
    cell.gcluster_backstop = 0;
    cell.width = gcluster.gi8[0] ? 1 : 0;
    cell.stylemask = *stylemask;
    RGB rgb;
    UiChannels channels = {0};
    sscanf(fg, "#%02hhX%02hhX%02hhX", &rgb.r, &rgb.g, &rgb.b);
    channels.f_r = rgb.r;
    channels.f_g = rgb.g;
    channels.f_b = rgb.b;
    channels.f_a = 0x40;
    sscanf(bg, "#%02hhX%02hhX%02hhX", &rgb.r, &rgb.g, &rgb.b);
    channels.b_r = rgb.r;
    channels.b_g = rgb.g;
    channels.b_b = rgb.b;
    channels.b_a = 0x40;
    cell.channels = channels.fb;
    return cell;
}

// -------------------------------------------------------------------------
// Background and style management
// -------------------------------------------------------------------------
int ui_bkgd(UiSurface *s, uint w, const UiCell *cell) {
    if (!s)
        return -1;
    ncplane_set_base(s->mplane[w], " ",
                     cell->stylemask,
                     cell->channels);
    return 0;
}

int ui_bkgdset(UiSurface *s, uint w, const UiCell *cell) {
    if (!s)
        return -1;
    ncplane_set_base(s->mplane[w], " ",
                     cell->stylemask,
                     cell->channels);
    return 0;
}
int ui_bkgrnd(NCPlane *plane, const UiCell *cell) {
    ncplane_set_base(plane, " ",
                     cell->stylemask,
                     cell->channels);
    return 0;
}

int ui_bkgrndset(NCPlane *plane, const UiCell *cell) {
    ncplane_set_base(plane, " ",
                     cell->stylemask,
                     cell->channels);
    return 0;
}
/* -------------------------------------------------------------------------
   Cell Manipulation
   ------------------------------------------------------------------------- */

int ui_getcchar(const UiCell *uic, wchar_t *wstr, UiStyle *style, UiPairIdx *pair, const void *opts) {
    (void)opts;
    if (!uic || !wstr || !style || !pair)
        return -1;
    GCluster gcluster;
    gcluster.gi32 = uic->gcluster;
    int i = 0;
    while (gcluster.wstr[i] != L'\0') {
        wstr[i] = gcluster.wstr[i];
        i++;
    }
    wstr[i] = L'\0';
    *style = uic->stylemask;
    // Convert the channels to a color pair index
    UiChannels channels;
    channels.fb = uic->channels;
    RGB rgb = {0};
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

int ui_setcchar(UiCell *uic, const wchar_t *wstr, const attr_t style, ushort pair, const void *opts) {
    (void)opts;
    if (!uic || !wstr || !style || !pair)
        return -1;
    int i = 0;
    GCluster gcluster;
    while (wstr[i] != L'\0') {
        gcluster.wstr[i] = wstr[i];
        i++;
    }
    gcluster.wstr[i] = L'\0';
    uic->gcluster = gcluster.gi32;
    uic->stylemask = style;
    UiChannels channels;
    // Convert the color pair index to channels
    ui_get_channels_from_pair(pair, &channels);
    uic->channels = channels.fb;
    return 0;
}

int ui_wch_to_utf8(const wchar_t fill_ch) {
    char utf8[5] = " ";
    if (fill_ch >= 0x20) {
        /* Encode the Unicode codepoint as UTF-8. */
        if (fill_ch < 0x80) {
            utf8[0] = (char)fill_ch;
            utf8[1] = '\0';
        } else if (fill_ch < 0x800) {
            utf8[0] = (char)(0xC0 | (fill_ch >> 6));
            utf8[1] = (char)(0x80 | (fill_ch & 0x3F));
            utf8[2] = '\0';
        } else if (fill_ch < 0x10000) {
            utf8[0] = (char)(0xE0 | (fill_ch >> 12));
            utf8[1] = (char)(0x80 | ((fill_ch >> 6) & 0x3F));
            utf8[2] = (char)(0x80 | (fill_ch & 0x3F));
            utf8[3] = '\0';
        } else {
            utf8[0] = (char)(0xF0 | (fill_ch >> 18));
            utf8[1] = (char)(0x80 | ((fill_ch >> 12) & 0x3F));
            utf8[2] = (char)(0x80 | ((fill_ch >> 6) & 0x3F));
            utf8[3] = (char)(0x80 | (fill_ch & 0x3F));
            utf8[4] = '\0';
        }
    }
    return 0;
}
/* -------------------------------------------------------------------------
   Non-portable escape-hatch getters (see ui_notcurses_compat.h)
   ------------------------------------------------------------------------- */

struct notcurses *ui_notcurses_get_nc(const UiRuntime *ui) {
    if (!ui)
        return NULL;
    return ui->nc;
}

struct ncplane *ui_notcurses_surface_get_plane(const UiSurface *s, uint w) {
    if (!s)
        return NULL;
    return s->mplane[w];
}
