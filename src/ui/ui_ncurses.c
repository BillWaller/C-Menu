/** @file ui_ncurses.c
   @ingroup ui_ncurses
   @brief NCurses UI backend — lifecycle, surface management, and capabilities.

   Implements all UiRuntime and UiSurface operations declared in ui_backend.h
   using the NCurses / panelw library.

   When compiled as part of the main C-Menu build (UAL_LEGACY_COMPAT defined),
   the legacy globals @c screen and @c tty_fp from dwin.c are kept in sync so
   that code not yet migrated to the UAL API continues to work.
*/

#include "ui_ncurses_internal.h"
#include "ui_ncurses_compat.h"
#ifdef UAL_LEGACY_COMPAT
# include "../include/cm.h"
#endif
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* -------------------------------------------------------------------------
   Internal color-pair cache — independent of dwin.c.
   Keeps a simple linear map of (fg_color_idx, bg_color_idx) → pair_id.
   ------------------------------------------------------------------------- */
#define NC_MAX_COLORS     512
#define NC_MAX_PAIRS      512

typedef struct {
    int fg;
    int bg;
    int pair_id;
} NcColorCache;

static NcColorCache nc_color_cache[NC_MAX_PAIRS];
static int nc_color_cache_cnt = 0;
static int nc_color_cnt       = 0; /* colors allocated via init_extended_color */

/** Translate an 8-bit channel value (0-255) to the 1000-based NCurses scale. */
static inline int nc_scale_1000(uint8_t v) {
    return (int)((v * 1000) / 255);
}

/**
 * Allocate (or find) an extended NCurses color index for an RGB triple.
 * Returns the color index, or -1 on failure.
 */
static int nc_alloc_color(uint8_t r, uint8_t g, uint8_t b) {
    int r1000 = nc_scale_1000(r);
    int g1000 = nc_scale_1000(g);
    int b1000 = nc_scale_1000(b);
    /* Search for an existing allocation. */
    for (int i = 0; i < nc_color_cnt && i < NC_MAX_COLORS; i++) {
        int cr, cg, cb;
        extended_color_content(i, &cr, &cg, &cb);
        if (cr == r1000 && cg == g1000 && cb == b1000)
            return i;
    }
    if (nc_color_cnt >= NC_MAX_COLORS || nc_color_cnt >= COLORS)
        return -1;
    init_extended_color(nc_color_cnt, r1000, g1000, b1000);
    return nc_color_cnt++;
}

/**
 * Find or allocate an NCurses extended color pair for (fg, bg).
 * Returns the pair index.
 */
static int nc_alloc_pair(int fg, int bg) {
    for (int i = 0; i < nc_color_cache_cnt; i++) {
        if (nc_color_cache[i].fg == fg && nc_color_cache[i].bg == bg)
            return nc_color_cache[i].pair_id;
    }
    if (nc_color_cache_cnt >= NC_MAX_PAIRS)
        return 0;
    int pair_id = nc_color_cache_cnt + 1;
    init_extended_pair(pair_id, fg, bg);
    nc_color_cache[nc_color_cache_cnt].fg      = fg;
    nc_color_cache[nc_color_cache_cnt].bg      = bg;
    nc_color_cache[nc_color_cache_cnt].pair_id = pair_id;
    nc_color_cache_cnt++;
    return pair_id;
}

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
    UiRuntime *ui = calloc(1, sizeof(*ui));
    if (!ui)
        return NULL;

    /* Determine the TTY device to use. */
    char tty_name[XLEN];
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

    /* Keep legacy globals in sync when libcm (dwin.c) is linked. */
#ifdef UAL_LEGACY_COMPAT
    screen       = ui->screen;
    tty_fp       = ui->tty_fp;
    f_curses_open = true;
#endif

    if (!has_colors() || !can_change_color()) {
        ui_shutdown(ui);
        return NULL;
    }
    start_color();
    use_default_colors();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    if (cfg) {
        ui->mouse_enabled  = cfg->enable_mouse;
        ui->alt_screen     = cfg->enable_alt_screen;
        ui->cursor_visible = cfg->cursor_visible;
    } else {
        ui->cursor_visible = true;
    }

    if (ui->mouse_enabled)
        mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);

    curs_set(ui->cursor_visible ? 1 : 0);
    getmaxyx(stdscr, ui->rows, ui->cols);

#ifdef UAL_LEGACY_COMPAT
    win_ptr = -1;
#endif

    return ui;
}

void ui_shutdown(UiRuntime *ui) {
    if (!ui)
        return;
    endwin();
    if (ui->screen) {
        delscreen(ui->screen);
#ifdef UAL_LEGACY_COMPAT
        screen = NULL;
#endif
    }
    if (ui->tty_fp) {
        fclose(ui->tty_fp);
#ifdef UAL_LEGACY_COMPAT
        tty_fp = NULL;
#endif
    }
#ifdef UAL_LEGACY_COMPAT
    f_curses_open = false;
#endif
    free(ui);
}

void ui_get_screen_size(UiRuntime *ui, int *rows, int *cols) {
    if (!ui)
        return;
    getmaxyx(stdscr, ui->rows, ui->cols);
    if (rows)
        *rows = ui->rows;
    if (cols)
        *cols = ui->cols;
}

int ui_render(UiRuntime *ui) {
    (void)ui;
    update_panels();
    doupdate();
    return 0;
}

int ui_clear_screen(UiRuntime *ui) {
    (void)ui;
    erase();
    return 0;
}

int ui_suspend(UiRuntime *ui) {
    (void)ui;
    def_prog_mode();
    endwin();
    return 0;
}

int ui_resume(UiRuntime *ui) {
    (void)ui;
    reset_prog_mode();
    refresh();
    update_panels();
    doupdate();
    return 0;
}

int ui_cursor_enable(UiRuntime *ui, bool visible) {
    if (!ui)
        return -1;
    ui->cursor_visible = visible;
    curs_set(visible ? 1 : 0);
    return 0;
}

/* -------------------------------------------------------------------------
   Backend identification and capability query
   ------------------------------------------------------------------------- */

UiBackend ui_get_backend(const UiRuntime *ui) {
    (void)ui;
    return UI_BACKEND_NCURSES;
}

void ui_get_caps(const UiRuntime *ui, UiCaps *caps) {
    if (!caps)
        return;
    memset(caps, 0, sizeof(*caps));
    caps->truecolor   = can_change_color() && (COLORS >= 256);
    caps->palette256  = (COLORS >= 256);
    caps->mouse       = ui ? ui->mouse_enabled : false;
    caps->unicode     = true;  /* compiled with NCURSES_WIDECHAR */
    caps->resize      = true;  /* KEY_RESIZE is supported */
    caps->color_pairs = COLOR_PAIRS;
}

/* -------------------------------------------------------------------------
   Surface management
   ------------------------------------------------------------------------- */

UiSurface *ui_surface_new(UiRuntime *ui, UiSurface *parent, UiRect rect) {
    UiSurface *s = calloc(1, sizeof(*s));
    if (!s)
        return NULL;

    s->runtime = ui;
    s->parent  = parent;
    s->y       = rect.y;
    s->x       = rect.x;
    s->rows    = rect.rows;
    s->cols    = rect.cols;

    if (parent && parent->win)
        s->win = derwin(parent->win, rect.rows, rect.cols, rect.y, rect.x);
    else
        s->win = newwin(rect.rows, rect.cols, rect.y, rect.x);

    if (!s->win) {
        free(s);
        return NULL;
    }

    s->pan = new_panel(s->win);
    if (!s->pan) {
        delwin(s->win);
        free(s);
        return NULL;
    }

    return s;
}

void ui_surface_destroy(UiSurface *s) {
    if (!s)
        return;
    if (s->pan)
        del_panel(s->pan);
    if (s->win)
        delwin(s->win);
    free(s);
}

int ui_surface_move(UiSurface *s, int y, int x) {
    if (!s)
        return -1;
    s->y = y;
    s->x = x;
    return move_panel(s->pan, y, x);
}

int ui_surface_resize(UiSurface *s, int rows, int cols) {
    if (!s)
        return -1;
    s->rows = rows;
    s->cols = cols;
    return wresize(s->win, rows, cols);
}

int ui_surface_clear(UiSurface *s) {
    if (!s)
        return -1;
    wclear(s->win);
    return 0;
}

int ui_surface_erase(UiSurface *s) {
    if (!s)
        return -1;
    werase(s->win);
    return 0;
}

int ui_surface_show(UiSurface *s) {
    if (!s)
        return -1;
    show_panel(s->pan);
    s->hidden = false;
    return 0;
}

int ui_surface_hide(UiSurface *s) {
    if (!s)
        return -1;
    hide_panel(s->pan);
    s->hidden = true;
    return 0;
}

int ui_cursor_move(UiSurface *s, int y, int x) {
    if (!s)
        return -1;
    return wmove(s->win, y, x);
}

int ui_bkgrnd(UiSurface *s, const UiStyle *style, const char *c) {
    if (!s)
        return -1;
    cchar_t cch = ui_style_to_cch(style, c);
    wbkgrnd(s->win, &cch);
    return 0;
}

int ui_bkgd_set(UiSurface *s, const UiStyle *style, const char *c) {
    if (!s)
        return -1;
    cchar_t cch = ui_style_to_cch(style, c);
    wbkgrndset(s->win, &cch);
    return 0;
}

/* -------------------------------------------------------------------------
   Style helpers (shared with draw and input modules)
   ------------------------------------------------------------------------- */

UiStyle *ui_style_new(void) {
    UiStyle *style = calloc(1, sizeof(*style));
    if (!style)
        return NULL;
    style->fg.r = 255;
    style->fg.g = 255;
    style->fg.b = 255;
    return style;
}

void ui_style_destroy(UiStyle *style) {
    free(style);
}

UiStyle *ui_style_from_cch(const cchar_t *cch) {
    UiStyle *style = calloc(1, sizeof(*style));
    if (!style)
        return NULL;
    wchar_t  wc[2] = {L'\0', L'\0'};
    attr_t   attrs;
    short    cpx;
    int      fg, bg;
    getcchar(cch, wc, &attrs, &cpx, NULL);
    style->bold      = (attrs & WA_BOLD) != 0;
    style->dim       = (attrs & WA_DIM) != 0;
    style->italic    = (attrs & WA_ITALIC) != 0;
    style->underline = (attrs & WA_UNDERLINE) != 0;
    style->blink     = (attrs & WA_BLINK) != 0;
    style->reverse   = (attrs & WA_REVERSE) != 0;
    style->invis     = (attrs & WA_INVIS) != 0;
    extended_pair_content(cpx, &fg, &bg);
    int r, g, b;
    extended_color_content(fg, &r, &g, &b);
    style->fg.r = (uint8_t)((r * 255) / 1000);
    style->fg.g = (uint8_t)((g * 255) / 1000);
    style->fg.b = (uint8_t)((b * 255) / 1000);
    extended_color_content(bg, &r, &g, &b);
    style->bg.r = (uint8_t)((r * 255) / 1000);
    style->bg.g = (uint8_t)((g * 255) / 1000);
    style->bg.b = (uint8_t)((b * 255) / 1000);
    return style;
}

cchar_t ui_style_to_cch(const UiStyle *style, const char *c) {
    attr_t   attrs  = 0;
    uint32_t cpx    = 0;

    if (style) {
        attrs |= style->bold      ? WA_BOLD      : 0;
        attrs |= style->dim       ? WA_DIM       : 0;
        attrs |= style->italic    ? WA_ITALIC    : 0;
        attrs |= style->underline ? WA_UNDERLINE : 0;
        attrs |= style->blink     ? WA_BLINK     : 0;
        attrs |= style->reverse   ? WA_REVERSE   : 0;
        attrs |= style->invis     ? WA_INVIS     : 0;
        int fg = nc_alloc_color(style->fg.r, style->fg.g, style->fg.b);
        int bg = nc_alloc_color(style->bg.r, style->bg.g, style->bg.b);
        if (fg >= 0 && bg >= 0)
            cpx = (uint32_t)nc_alloc_pair(fg, bg);
    }

    /* Encode the first character (or space) as a cchar_t. */
    mbstate_t mbst;
    memset(&mbst, 0, sizeof(mbst));
    wchar_t wstr[2] = {L' ', L'\0'};
    if (c && *c) {
        wchar_t wc = L'\0';
        size_t  n  = mbrtowc(&wc, c, MB_CUR_MAX, &mbst);
        if ((ssize_t)n > 0)
            wstr[0] = wc;
    }
    cchar_t cc;
    setcchar(&cc, wstr, attrs, (short)cpx, NULL);
    return cc;
}

/* -------------------------------------------------------------------------
   Non-portable escape-hatch getters (see ui_ncurses_compat.h)
   ------------------------------------------------------------------------- */

SCREEN *ui_ncurses_get_screen(const UiRuntime *ui) {
    if (!ui) return NULL;
    return ui->screen;
}

WINDOW *ui_ncurses_surface_get_win(const UiSurface *s) {
    if (!s) return NULL;
    return s->win;
}

PANEL *ui_ncurses_surface_get_panel(const UiSurface *s) {
    if (!s) return NULL;
    return s->pan;
}
