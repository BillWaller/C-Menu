/** @file ui_notcurses.c
   @ingroup ui_notcurses
   @brief NotCurses UI backend — lifecycle, surface management, and capabilities.

   Implements all UiRuntime and UiSurface operations declared in ui_backend.h
   using the NotCurses library.
*/

#include "cm.h"
#include "ui_notcurses_compat.h"
#include "ui_notcurses_internal.h"
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

    unsigned int r = 0, c = 0;
    notcurses_stddim_yx(ui->nc, &r, &c);
    ui->lines = (int)r;
    ui->cols = (int)c;

    return ui;
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

void ui_get_screen_size(UiRuntime *ui, int *lines, int *cols) {
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

int ui_render(UiRuntime *ui) {
    if (!ui)
        return -1;
    return notcurses_render(ui->nc) == 0 ? 0 : -1;
}

int ui_clear_screen(UiRuntime *ui) {
    if (!ui)
        return -1;
    struct ncplane *stdn = notcurses_stdplane(ui->nc);
    ncplane_erase(stdn);
    return 0;
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
    caps->truecolor = notcurses_cantruecolor(ui->nc);
    caps->palette256 = true; /* NotCurses always manages its own palette */
    caps->mouse = ui->mouse_enabled;
    caps->unicode = notcurses_canutf8(ui->nc);
    caps->resize = true;   /* NCKEY_RESIZE is delivered as a normal event */
    caps->color_pairs = 0; /* unlimited — NotCurses uses direct RGB */
}

/* -------------------------------------------------------------------------
   Surface management
   ------------------------------------------------------------------------- */

UiSurface *ui_surface_new(UiRuntime *ui, int w, UiSurface *parent, int p, int lines, int cols, int y, int x) {
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
    NcPlaneOptions plane_opts = {
        .y = y,
        .x = x,
        .rows = lines + 2,
        .cols = cols + 2};
    if (parent && parent->mplane[p]) {
        s->mplane[BOX] = ncplane_create(parent->mplane[p], &plane_opts);
    } else {
        NcPlane *stdn = notcurses_stdplane(ui->nc);
        s->mplane[BOX] = ncplane_create(stdn, &plane_opts);
    }
    if (!s->mplane[w]) {
        notcurses_stop(ui->nc);
        return NULL;
    }
    uint64_t channels = ui_notcurses_channels_from_style(&style_box);
    uint32_t attrs = ui_notcurses_attrs_from_style(&style_box);
    ncplane_set_base(s->mplane[w], " ", attrs, 0);
    ncplane_set_channels(s->mplane[w], channels);
    ncplane_perimeter_rounded(s->mplane[w], 0, channels, 0);
    return s;
}

UiSurface *ui_box_surface_new(UiRuntime *ui, UiSurface *parent, int p, int lines, int cols, int y, int x, char *wtitle) {
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

    NcPlaneOptions plane_opts = {
        .y = y,
        .x = x,
        .rows = lines + 2,
        .cols = cols + 2,
        .name = NULL};
    if (parent && parent->mplane[p]) {
        s->mplane[BOX] = ncplane_create(parent->mplane[p], &plane_opts);
    } else {
        NcPlane *stdn = notcurses_stdplane(ui->nc);
        s->mplane[BOX] = ncplane_create(stdn, &plane_opts);
    }
    if (!s->mplane[BOX]) {
        free(s);
        return NULL;
    }
    uint64_t channels = ui_notcurses_channels_from_style(&style_box);
    uint32_t attrs = ui_notcurses_attrs_from_style(&style_box);
    ncplane_set_base(s->mplane[BOX], " ", attrs, 0);
    ncplane_set_channels(s->mplane[BOX], channels);
    ncplane_perimeter_rounded(s->mplane[BOX], 0, channels, 0);

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
    ncplane_set_base(s->mplane[WIN], " ", 0, channels);
    ncplane_set_channels(s->mplane[WIN], channels);
    sfc_ptr++;
    return s;
}

void ui_surfac_destroy(UiSurface *s) {
    if (!s)
        return;
    for (int w = 0; w < SUB_SFC_MAX; ++w)
        if (s->mplane[w])
            ncplane_destroy(s->mplane[w]);
    free(s);
}

int ui_surface_move(UiSurface *s, int w, int y, int x) {
    if (!s)
        return -1;
    s->y = y;
    s->x = x;
    if (!s->hidden)
        return ncplane_move_yx(s->mplane[w], y, x) == 0 ? 0 : -1;
    return 0;
}

int ui_surface_resize(UiSurface *s, int w, int lines, int cols) {
    if (!s)
        return -1;
    s->lines = lines;
    s->cols = cols;
    return ncplane_resize_simple(s->mplane[w], (unsigned int)lines,
                                 (unsigned int)cols) == 0
               ? 0
               : -1;
}

int ui_surface_clear(UiSurface *s, int w) {
    if (!s)
        return -1;
    ncplane_erase(s->mplane[w]);
    return 0;
}

int ui_surface_erase(UiSurface *s, int w) {
    if (!s)
        return -1;
    ncplane_erase(s->mplane[w]);
    return 0;
}

int ui_surface_show(UiSurface *s, int w) {
    if (!s)
        return -1;
    if (s->hidden) {
        s->hidden = false;
        ncplane_move_yx(s->mplane[w], s->y, s->x);
    }
    return 0;
}

int ui_surface_hide(UiSurface *s, int w) {
    if (!s)
        return -1;
    if (!s->hidden) {
        s->hidden = true;
        /* Move far off-screen so the plane does not obscure anything. */
        ncplane_move_yx(s->mplane[w], -s->lines - 1, 0);
    }
    return 0;
}

int ui_cursor_move(UiSurface *s, int w, int y, int x) {
    if (!s)
        return -1;
    return ncplane_cursor_move_yx(s->mplane[w], y, x) == 0 ? 0 : -1;
}

int ui_bkgrnd(UiSurface *s, int w, const UiStyle *style, const char *c) {
    if (!s)
        return -1;
    uint64_t channels = ui_notcurses_channels_from_style(style);
    uint32_t attrs = ui_notcurses_attrs_from_style(style);
    const char *fill = (c && *c) ? c : " ";
    ncplane_set_base(s->mplane[BOX], fill, attrs, channels);
    return 0;
}

int ui_bkgd_set(UiSurface *s, int w, const UiStyle *style, const char *c) {
    /* NotCurses has no separate "set without fill" operation; delegate to
       bkgrnd which sets channels and re-fills the plane background. */
    return ui_bkgrnd(s, w, style, c);
}

/* -------------------------------------------------------------------------
   Style helpers (shared with draw and input modules)
   ------------------------------------------------------------------------- */

uint64_t ui_notcurses_channels_from_style(const UiStyle *style) {
    uint64_t channels = 0;
    if (!style)
        return channels;
    ncchannels_set_fg_rgb8(&channels, style->fg.r, style->fg.g, style->fg.b);
    ncchannels_set_bg_rgb8(&channels, style->bg.r, style->bg.g, style->bg.b);
    return channels;
}

uint32_t ui_notcurses_attrs_from_style(const UiStyle *style) {
    return style->attrs;
}

/* -------------------------------------------------------------------------
   Surface style
   ------------------------------------------------------------------------- */

int ui_surface_set_style(UiSurface *s, int w, const UiStyle *style) {
    if (!s || !style)
        return -1;
    uint64_t channels = ui_notcurses_channels_from_style(style);
    ncplane_set_channels(s->mplane[w], channels);
    ncplane_set_styles(s->mplane[w], ui_notcurses_attrs_from_style(style));
    return 0;
}

int ui_surface_set_base(UiSurface *s, int w, const UiStyle *style, uint32_t fill_ch) {
    if (!s)
        return -1;
    uint64_t channels = ui_notcurses_channels_from_style(style);
    uint32_t attrs = ui_notcurses_attrs_from_style(style);
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
    ncplane_set_base(s->mplane[w], utf8, attrs, channels);
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

struct ncplane *ui_notcurses_surface_get_plane(const UiSurface *s, int w) {
    if (!s)
        return NULL;
    return s->mplane[w];
}
