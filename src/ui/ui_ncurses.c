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

#include "ui_ncurses_compat.h"
#include "ui_ncurses_internal.h"
#include <ncurses/panel.h>
#include <ncursesw/ncurses.h>
#define UAL_LEGACY_COMPAT 1
#ifdef UAL_LEGACY_COMPAT
#include "cm.h"
#endif
#include <locale.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/** Translate an 8-bit channel value (0-255) to the 1000-based NCurses scale. */
static inline int nc_scale_1000(uint8_t v) {
    return (int)((v * 1000) / 255);
}

/**
 * Allocate (or find) an extended NCurses color index for an RGB triple.
 * Returns the color index, or -1 on failure.
 */
static int ui_init_extended_color(uint8_t r, uint8_t g, uint8_t b) {
    int r1000 = nc_scale_1000(r);
    int g1000 = nc_scale_1000(g);
    int b1000 = nc_scale_1000(b);
    for (int i = 0; i < ui_color_cnt && i < NC_MAX_COLORS; i++) {
        int cr, cg, cb;
        extended_color_content(i, &cr, &cg, &cb);
        if (cr == r1000 && cg == g1000 && cb == b1000)
            return i;
    }
    if (ui_color_cnt >= NC_MAX_COLORS || ui_color_cnt >= COLORS)
        return -1;
    init_extended_color(ui_color_cnt, r1000, g1000, b1000);
    return ui_color_cnt++;
}

/* -------------------------------------------------------------------------
   Lifecycle
   ------------------------------------------------------------------------- */

UiRuntime *ui_init(const UiConfig *cfg) {
    setlocale(LC_ALL, "");
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
    screen = ui->screen;
    tty_fp = ui->tty_fp;
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
        ui->mouse_enabled = cfg->enable_mouse;
        ui->alt_screen = cfg->enable_alt_screen;
        ui->cursor_visible = cfg->cursor_visible;
    } else {
        ui->cursor_visible = false;
    }
    ui->panel_main = new_panel(stdscr);

    wbkgrnd(stdscr, &CC_NT);
    if (ui->mouse_enabled)
        mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);

    curs_set(ui->cursor_visible ? 1 : 0);
    getmaxyx(stdscr, ui->lines, ui->cols);

#ifdef UAL_LEGACY_COMPAT
    sfc_ptr = -1;
#endif
    curs_set(ui->cursor_visible ? 1 : 0);
    return ui;
}

void ui_shutdown(UiRuntime *ui) {
    if (!ui)
        return;
    if (sfc_ptr >= 0) {
        for (int i = sfc_ptr; i >= 0; i--) {
            UiSurface *sfc = ui_surface[i];
            if (sfc) {
                ui_surface_destroy(sfc);
                sfc = NULL;
            }
        }
    }
    sfc_ptr = -1;
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

void ui_get_screen_size(UiRuntime *ui, int *lines, int *cols) {
    if (!ui)
        return;
    getmaxyx(stdscr, ui->lines, ui->cols);
    if (lines)
        *lines = ui->lines;
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
    caps->truecolor = can_change_color() && (COLORS >= 256);
    caps->palette256 = (COLORS >= 256);
    caps->mouse = ui ? ui->mouse_enabled : false;
    caps->unicode = true; /* compiled with NCURSES_WIDECHAR */
    caps->resize = true;  /* KEY_RESIZE is supported */
    caps->color_pairs = COLOR_PAIRS;
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

UiSurface *ui_box_surface_new(UiRuntime *ui, UiSurface *parent, int p, int lines, int cols, int y, int x, char *wtitle) {
    UiSurface *s = calloc(1, sizeof(*s));
    if (!s)
        return NULL;
    s->runtime = ui;
    s->parent = parent;
    s->y = y;
    s->x = x;
    s->lines = lines;
    s->cols = cols;
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
    ui_bkgdset(s, BOX, &style_box, " ");
    ui_scrollok(s, BOX, false);
    border_draw(s);
    border_title(s, wtitle);
    s->mwin[WIN] = derwin(s->mwin[BOX], lines, cols, 1, 1);
    if (!s->mwin[WIN]) {
        free(s);
        return NULL;
    }
    s->mpan[WIN] = new_panel(s->mwin[WIN]);
    keypad(s->mwin[WIN], true);
    ui_bkgdset(s, WIN, &style_nt, " ");
    return s;
}
int ui_surface_addpad(UiSurface *sfc, int w, int view_win, int lines, int cols) {
    sfc->mwin[w] = newpad(lines, cols);
    if (sfc->mwin[w] == nullptr)
        return -1;
    sfc->mwin[view_win] = subpad(sfc->mwin[PAD], lines, cols, 0, 0);
    if (sfc->mwin[view_win] == nullptr)
        return -1;
    sfc->mpan[w] = new_panel(sfc->mwin[view_win]);
    return 0;
}

int ui_surface_addwin(UiSurface *s, int w, int der_from, int lines, int cols, int y, int x) {
    s->mwin[w] = derwin(s->mwin[der_from], lines, cols, y, x);
    if (!s->mwin[w])
        return -1;
    s->mpan[w] = new_panel(s->mwin[w]);
    return 0;
}
void ui_wscrl(UiSurface *s, int w, int n) {
    if (!s->mwin[w])
        return;
    wscrl(s->mwin[w], n);
}
void ui_scrollok(UiSurface *s, int w, bool enable) {
    if (!s->mwin[w])
        return;
    if (enable)
        scrollok(s->mwin[w], true);
    else
        scrollok(s->mwin[w], false);
}
void ui_keypad(UiSurface *s, int w, bool enable) {
    if (!s->mwin[w])
        return;
    if (enable)
        keypad(s->mwin[w], true);
    else
        keypad(s->mwin[w], false);
}
void ui_idlok(UiSurface *s, int w, bool enable) {
    if (!s->mwin[w])
        return;
    if (enable)
        idlok(s->mwin[w], true);
    else
        idlok(s->mwin[w], false);
}
void ui_idcok(UiSurface *s, int w, bool enable) {
    if (!s->mwin[w])
        return;
    if (enable)
        idcok(s->mwin[w], true);
    else
        idcok(s->mwin[w], false);
}
void ui_setscrreg(UiSurface *s, int w, int top, int bottom) {
    if (!s->mwin[w])
        return;
    wsetscrreg(s->mwin[w], top, bottom);
}
void ui_getyx(UiSurface *sfc, int w, int *lines, int *cols) {
    if (!sfc->mwin[w])
        return;
    getyx(sfc->mwin[w], *lines, *cols);
}
void ui_getmaxyx(UiSurface *sfc, int w, int *lines, int *cols) {
    if (!sfc->mwin[w])
        return;
    getmaxyx(sfc->mwin[w], *lines, *cols);
}
int ui_getmaxy(UiSurface *sfc, int w) {
    if (!sfc->mwin[w])
        return -1;
    return getmaxy(sfc->mwin[w]);
}
int ui_getmaxx(UiSurface *sfc, int w) {
    if (!sfc->mwin[w])
        return -1;
    return getmaxx(sfc->mwin[w]);
}
void ui_surface_destroy(UiSurface *s) {
    if (!s)
        return;
    for (int i = 0; i < 8; i++) {
        if (s->mpan[i]) {
            hide_panel(s->mpan[i]);
            del_panel(s->mpan[i]);
        }
    }
    for (int i = 1; i < 8; i++) {
        if (s->mwin[i]) {
            werase(s->mwin[i]);
            delwin(s->mwin[i]);
        }
    }
    ui_render(ui_runtime);
    free(s);
}
int ui_surface_move(UiSurface *s, int w, int y, int x) {
    if (!s)
        return -1;
    s->y = y;
    s->x = x;
    return move_panel(s->mpan[w], y, x);
}

int ui_surface_resize(UiSurface *s, int w, int lines, int cols) {
    if (!s)
        return -1;
    s->lines = lines;
    s->cols = cols;
    wresize(s->mwin[w], lines + 2, cols + 2);
    wresize(s->mwin[w + 1], lines, cols);
    return 0;
}

int ui_surface_clear(UiSurface *s, int w) {
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

int ui_surface_erase(UiSurface *s, int w) {
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

int ui_surface_show(UiSurface *s, int w) {
    if (!s)
        return -1;
    show_panel(s->mpan[w]);
    s->hidden = false;
    return 0;
    return 0;
}

int ui_surface_hide(UiSurface *s, int w) {
    if (!s)
        return -1;
    hide_panel(s->mpan[w]);
    s->hidden = true;
    return 0;
}

int ui_cursor_move(UiSurface *s, int w, int y, int x) {
    if (!s)
        return -1;
    return wmove(s->mwin[w], y, x);
}
// -------------------------------------------------------------------------
// Background and style management
// -------------------------------------------------------------------------

// for the entire window
int ui_bkgd(UiSurface *s, int w, const UiStyle *style, const char *c) {
    if (!s)
        return -1;
    cchar_t cch = ui_style_to_cch(style, c);
    wbkgrnd(s->mwin[w], &cch);
    return 0;
}
// for new content to be written to the window
int ui_bkgdset(UiSurface *s, int w, const UiStyle *style, const char *c) {
    if (!s)
        return -1;
    cchar_t cch = ui_style_to_cch(style, c);
    wbkgrndset(s->mwin[w], &cch);
    return 0;
}
// for the entire window
int ui_bkgrnd(UiSurface *s, int w, const cchar_t *cc) {
    if (!s)
        return -1;
    wbkgrnd(s->mwin[w], cc);
    return 0;
}
// for new content to be written to the window
int ui_bkgrndset(UiSurface *s, int w, const cchar_t *cc) {
    if (!s)
        return -1;
    wbkgrndset(s->mwin[w], cc);
    return 0;
}
/* ------------------------------------------------------------------------- */
void ui_qiflush() {
    qiflush();
}
int ui_setcchar(cchar_t *wch, const wchar_t *wc, attr_t attrs, short pair, const void *opts) {
    return setcchar(wch, wc, attrs, pair, opts);
}
void ui_update_panels() {
    update_panels();
}
void ui_doupdate() {
    doupdate();
}
void ui_wnoutrefresh(UiSurface *s, int w) {
    if (!s)
        return;
    wnoutrefresh(s->mwin[w]);
}
void ui_curs_set(int visibility) {
    curs_set(visibility);
}
void ui_erase() {
    erase();
}
/* -------------------------------------------------------------------------
   Style helpers (shared with draw and input modules)
   ------------------------------------------------------------------------- */

UiStyle *ui_style_new(void) {
    UiStyle *style = calloc(1, sizeof(*style));
    if (!style)
        return NULL;
    style->frgb.r = 255;
    style->frgb.g = 255;
    style->frgb.b = 255;
    return style;
}

void ui_style_destroy(UiStyle *style) {
    free(style);
}

UiStyle ui_style_from_hex(const char *fg, const char *bg, int attrs, const char *str) {
    UiStyle *style = calloc(1, sizeof(*style));
    RGB rgb;
    sscanf(fg, "#%02x%02x%02x", &rgb.r, &rgb.g, &rgb.b);
    int fg_idx = rgb_to_curses_clr(&rgb);
    sscanf(bg, "#%02x%02x%02x", &rgb.r, &rgb.g, &rgb.b);
    int bg_idx = rgb_to_curses_clr(&rgb);
    style->cp = get_clr_pair(fg_idx, bg_idx);
    style->attrs = attrs;
    if (str && *str && *str != ' ') {
        wchar_t wc = {L'\0'};
        mbstate_t mbst;
        memset(&mbst, 0, sizeof(mbst));
        size_t n = mbrtowc(&wc, str, MB_CUR_MAX, &mbst);
        if ((ssize_t)n > 0)
            style->wc = wc;
    } else {
        style->wc = L' ';
    }
    return *style;
}

UiStyle *ui_style_copy(const UiStyle *src) {
    if (!src)
        return NULL;
    UiStyle *copy = calloc(1, sizeof(*copy));
    if (!copy)
        return NULL;
    memcpy(copy, src, sizeof(*copy));
    return copy;
}

UiStyle *ui_style_from_cch(const cchar_t *cch) {
    UiStyle *style = calloc(1, sizeof(*style));
    if (!style)
        return NULL;
    wchar_t wc[2] = {L'\0', L'\0'};
    getcchar(cch, wc, &style->attrs, &style->cp, NULL);
    return style;
}

cchar_t ui_style_to_cch(const UiStyle *style, const char *c) {
    mbstate_t mbst;
    memset(&mbst, 0, sizeof(mbst));
    wchar_t wstr[2] = {L' ', L'\0'};
    if (c && *c) {
        wchar_t wc = L'\0';
        size_t n = mbrtowc(&wc, c, MB_CUR_MAX, &mbst);
        if ((ssize_t)n > 0)
            wstr[0] = wc;
    }
    cchar_t cc;
    setcchar(&cc, wstr, style->attrs, style->cp, NULL);
    return cc;
}

/* -------------------------------------------------------------------------
   Non-portable escape-hatch getters (see ui_ncurses_compat.h)
   ------------------------------------------------------------------------- */

SCREEN *ui_ncurses_get_screen(const UiRuntime *ui) {
    if (!ui)
        return NULL;
    return ui->screen;
}

WINDOW *ui_ncurses_surface_get_win(const UiSurface *s, int w) {
    if (!s)
        return NULL;
    return s->mwin[w];
}

PANEL *ui_ncurses_surface_get_panel(const UiSurface *s, int w) {
    if (!s)
        return NULL;
    return s->mpan[w];
}
