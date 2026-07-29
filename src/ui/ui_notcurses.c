/** @file ui_notcurses.c
   @ingroup ui_notcurses
   @brief NotCurses UI backend — lifecycle, surface management, and capabilities.

   Implements all UiRuntime and UiSurface operations declared in ui_backend.h
   using the NotCurses library.
*/

#include "ui_notcurses_internal.h"
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* -------------------------------------------------------------------------
   Global surface arrays (declared extern in ui_backend.h).
   ------------------------------------------------------------------------- */
UiSurface *ui_surface_box[MAXWIN];
UiSurface *ui_surface_win[MAXWIN];
UiSurface *ui_surface_win2[MAXWIN];

/* -------------------------------------------------------------------------
   Lifecycle
   ------------------------------------------------------------------------- */

UiRuntime *ui_init(const UiConfig *cfg) {
    setlocale(LC_ALL, "");

    UiRuntime *ui = calloc(1, sizeof(*ui));
    if (!ui)
        return NULL;

    if (cfg) {
        ui->mouse_enabled  = cfg->enable_mouse;
        ui->alt_screen     = cfg->enable_alt_screen;
        ui->cursor_visible = cfg->cursor_visible;
    } else {
        ui->cursor_visible = true;
        ui->alt_screen     = true;
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
        .flags = NCOPTION_SUPPRESS_BANNERS
                 | (ui->alt_screen ? 0 : NCOPTION_NO_ALTERNATE_SCREEN),
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
    ui->rows = (int)r;
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

void ui_get_screen_size(UiRuntime *ui, int *rows, int *cols) {
    if (!ui)
        return;
    unsigned int r = 0, c = 0;
    notcurses_stddim_yx(ui->nc, &r, &c);
    ui->rows = (int)r;
    ui->cols = (int)c;
    if (rows)
        *rows = ui->rows;
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
    if (!ui) return;
    caps->truecolor   = notcurses_cantruecolor(ui->nc);
    caps->palette256  = true; /* NotCurses always manages its own palette */
    caps->mouse       = ui->mouse_enabled;
    caps->unicode     = notcurses_canutf8(ui->nc);
    caps->resize      = true;   /* NCKEY_RESIZE is delivered as a normal event */
    caps->color_pairs = 0;      /* unlimited — NotCurses uses direct RGB */
}

/* -------------------------------------------------------------------------
   Surface management
   ------------------------------------------------------------------------- */

UiSurface *ui_surface_new(UiRuntime *ui, UiSurface *parent, UiRect rect) {
    if (!ui)
        return NULL;

    UiSurface *s = calloc(1, sizeof(*s));
    if (!s)
        return NULL;

    s->runtime = ui;
    s->parent  = parent;
    s->y       = rect.y;
    s->x       = rect.x;
    s->rows    = rect.rows;
    s->cols    = rect.cols;

    struct ncplane *parent_plane =
        parent ? parent->plane : notcurses_stdplane(ui->nc);

    struct ncplane_options plane_opts = {
        .y    = rect.y,
        .x    = rect.x,
        .rows = (unsigned int)rect.rows,
        .cols = (unsigned int)rect.cols,
        .name = NULL,
    };

    s->plane = ncplane_create(parent_plane, &plane_opts);
    if (!s->plane) {
        free(s);
        return NULL;
    }

    return s;
}

void ui_surface_destroy(UiSurface *s) {
    if (!s)
        return;
    if (s->plane)
        ncplane_destroy(s->plane);
    free(s);
}

int ui_surface_move(UiSurface *s, int y, int x) {
    if (!s)
        return -1;
    s->y = y;
    s->x = x;
    if (!s->hidden)
        return ncplane_move_yx(s->plane, y, x) == 0 ? 0 : -1;
    return 0;
}

int ui_surface_resize(UiSurface *s, int rows, int cols) {
    if (!s)
        return -1;
    s->rows = rows;
    s->cols = cols;
    return ncplane_resize_simple(s->plane, (unsigned int)rows,
                                 (unsigned int)cols) == 0 ? 0 : -1;
}

int ui_surface_clear(UiSurface *s) {
    if (!s)
        return -1;
    ncplane_erase(s->plane);
    return 0;
}

int ui_surface_erase(UiSurface *s) {
    if (!s)
        return -1;
    ncplane_erase(s->plane);
    return 0;
}

int ui_surface_show(UiSurface *s) {
    if (!s)
        return -1;
    if (s->hidden) {
        s->hidden = false;
        ncplane_move_yx(s->plane, s->y, s->x);
    }
    return 0;
}

int ui_surface_hide(UiSurface *s) {
    if (!s)
        return -1;
    if (!s->hidden) {
        s->hidden = true;
        /* Move far off-screen so the plane does not obscure anything. */
        ncplane_move_yx(s->plane, -s->rows - 1, 0);
    }
    return 0;
}

int ui_cursor_move(UiSurface *s, int y, int x) {
    if (!s)
        return -1;
    return ncplane_cursor_move_yx(s->plane, y, x) == 0 ? 0 : -1;
}

int ui_bkgrnd(UiSurface *s, const UiStyle *style, const char *c) {
    if (!s)
        return -1;
    uint64_t channels = ui_notcurses_channels_from_style(style);
    uint32_t attrs    = ui_notcurses_attrs_from_style(style);
    const char *fill  = (c && *c) ? c : " ";
    ncplane_set_base(s->plane, fill, attrs, channels);
    return 0;
}

int ui_bkgd_set(UiSurface *s, const UiStyle *style, const char *c) {
    /* NotCurses has no separate "set without fill" operation; delegate to
       bkgrnd which sets channels and re-fills the plane background. */
    return ui_bkgrnd(s, style, c);
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
    if (!style)
        return 0;
    uint32_t attrs = 0;
    if (style->bold)      attrs |= NCSTYLE_BOLD;
    if (style->italic)    attrs |= NCSTYLE_ITALIC;
    if (style->underline) attrs |= NCSTYLE_UNDERLINE;
    if (style->blink)     attrs |= NCSTYLE_BLINK;
    if (style->reverse)   attrs |= NCSTYLE_REVERSE;
    if (style->invis)     attrs |= NCSTYLE_INVIS;
    return attrs;
}

/* -------------------------------------------------------------------------
   Surface style
   ------------------------------------------------------------------------- */

int ui_surface_set_style(UiSurface *s, const UiStyle *style) {
    if (!s || !style)
        return -1;
    uint64_t channels = ui_notcurses_channels_from_style(style);
    ncplane_set_channels(s->plane, channels);
    ncplane_set_styles(s->plane, ui_notcurses_attrs_from_style(style));
    return 0;
}

int ui_surface_set_base(UiSurface *s, const UiStyle *style, uint32_t fill_ch) {
    if (!s)
        return -1;
    uint64_t channels = ui_notcurses_channels_from_style(style);
    uint32_t attrs    = ui_notcurses_attrs_from_style(style);
    char     utf8[5]  = " ";
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
    ncplane_set_base(s->plane, utf8, attrs, channels);
    return 0;
}

/* -------------------------------------------------------------------------
   Non-portable escape-hatch getters (see ui_notcurses_compat.h)
   ------------------------------------------------------------------------- */

struct notcurses *ui_notcurses_get_nc(const UiRuntime *ui) {
    if (!ui) return NULL;
    return ui->nc;
}

struct ncplane *ui_notcurses_surface_get_plane(const UiSurface *s) {
    if (!s) return NULL;
    return s->plane;
}
