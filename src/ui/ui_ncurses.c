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

int ui_color_cnt = 0;
int ui_pair_cnt = 0;

UiSurface *stdsfc;

STDRGB std_color[] = {
    {0, 0, 0},
    {128, 0, 0},
    {0, 128, 0},
    {128, 128, 0},
    {0, 0, 128},
    {128, 0, 128},
    {0, 128, 128},
    {192, 192, 192},
    {128, 128, 128},
    {255, 0, 0},
    {0, 255, 0},
    {255, 255, 0},
    {0, 0, 255},
    {255, 0, 255},
    {0, 255, 255},
    {255, 255, 255}};

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
int ui_add_pair(int fg, int bg) {
    int rc, i;
    int pfg, pbg;
    for (i = 1; i < ui_pair_cnt; i++) {
        extended_pair_content(i, &pfg, &pbg);
        if (pfg == fg && pbg == bg)
            return i;
    }
    if (i + 1 >= COLOR_PAIRS) {
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
    return ui_pair_cnt - 1;
}
int ui_chg_pair(int pair, int fg, int bg) {
    if (pair + 1 >= COLOR_PAIRS)
        return -1;
    init_extended_pair(pair, fg, bg);
    return 0;
}
int ui_add_color_rgb(RGB *rgb) {
    int i;
    RGB tmp;
    apply_gamma(rgb);
    rgb->r = (rgb->r * 1000) / 255;
    rgb->g = (rgb->g * 1000) / 255;
    rgb->b = (rgb->b * 1000) / 255;
    for (i = 0; i < ui_color_cnt && i < COLORS; i++) {
        extended_color_content(i, &tmp.r, &tmp.g, &tmp.b);
        if (rgb->r == tmp.r && rgb->g == tmp.g && rgb->b == tmp.b)
            return i;
    }
    if (i < COLORS) {
        if (i < 16) {
            std_color[i].r = rgb->r;
            std_color[i].g = rgb->g;
            std_color[i].b = rgb->b;
        }
        init_extended_color(i, rgb->r, rgb->g, rgb->b);
        if (ui_color_cnt + 1 < COLORS)
            ui_color_cnt++;
        return ui_color_cnt - 1;
    }
    return 0;
}
int ui_add_color_hex(char *s) {
    int i;
    RGB rgb;
    RGB tmp;
    rgb = ui_hex_to_rgb(s);
    apply_gamma(&rgb);
    for (i = 0; i < ui_color_cnt && i < COLORS; i++) {
        extended_color_content(i, &tmp.r, &tmp.g, &tmp.b);
        if (rgb.r == tmp.r && rgb.g == tmp.g && rgb.b == tmp.b)
            return i;
    }
    if (i < COLORS) {
        if (i < 16) {
            std_color[i].r = rgb.r;
            std_color[i].g = rgb.g;
            std_color[i].b = rgb.b;
        }
        rgb.r = (rgb.r * 1000) / 255;
        rgb.g = (rgb.g * 1000) / 255;
        rgb.b = (rgb.b * 1000) / 255;
        init_extended_color(i, rgb.r, rgb.g, rgb.b);
        if (ui_color_cnt + 1 < COLORS)
            ui_color_cnt++;
        return ui_color_cnt - 1;
    }
    return 0;
}
int ui_chg_color_rgb(int color, RGB *rgb) {
    if (color + 1 >= COLORS)
        return -1;
    apply_gamma(rgb);
    rgb->r = (rgb->r * 1000) / 255;
    rgb->g = (rgb->g * 1000) / 255;
    rgb->b = (rgb->b * 1000) / 255;
    if (color < 16) {
        std_color[color].r = rgb->r;
        std_color[color].g = rgb->g;
        std_color[color].b = rgb->b;
    }
    init_extended_color(color, rgb->r, rgb->g, rgb->b);
    return 0;
}
int ui_chg_color_hex(int color, char *s) {
    RGB rgb;
    if (color + 1 >= COLORS)
        return -1;
    rgb = ui_hex_to_rgb(s);
    apply_gamma(&rgb);
    if (color < 16) {
        std_color[color].r = rgb.r;
        std_color[color].g = rgb.g;
        std_color[color].b = rgb.b;
    }
    rgb.r = (rgb.r * 1000) / 255;
    rgb.g = (rgb.g * 1000) / 255;
    rgb.b = (rgb.b * 1000) / 255;
    init_extended_color(color, rgb.r, rgb.g, rgb.b);
    return 0;
}
int ui_get_color(int color, RGB *rgb) {
    if (color + 1 >= COLORS)
        return -1;
    extended_color_content(color, &rgb->r, &rgb->g, &rgb->b);
    rgb->r = (rgb->r * 255) / 1000;
    rgb->g = (rgb->g * 255) / 1000;
    rgb->b = (rgb->b * 255) / 1000;
    return 0;
}
RGB ui_hex_to_rgb(char *s) {
    RGB rgb;
    sscanf(s, "#%02x%02x%02x", &rgb.r, &rgb.g, &rgb.b);
    return rgb;
}
int ui_extended_color_content(int color, int *r, int *g, int *b) {
    extended_color_content(color, r, g, b);
    return 0;
}

int ui_init_extended_color(int color, int r, int g, int b) {
    init_extended_color(color, r, g, b);
    return 0;
}

int ui_extended_pair_content(int pair, int *fg, int *bg) {
    extended_pair_content(pair, fg, bg);
    return 0;
}
int ui_init_extended_pair(int pair, int fg, int bg) {
    init_extended_pair(pair, fg, bg);
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

UiStyle ui_style_from_hex(const char *fg, const char *bg, const attr_t attrs, const wchar_t *wstr) {
    UiStyle style;
    RGB rgb;
    sscanf(fg, "#%02x%02x%02x", &rgb.r, &rgb.g, &rgb.b);
    int f_idx = ui_add_color_rgb(&rgb);
    sscanf(bg, "#%02x%02x%02x", &rgb.r, &rgb.g, &rgb.b);
    int b_idx = ui_add_color_rgb(&rgb);
    style.cp = ui_add_pair(f_idx, b_idx);
    style.attrs = attrs;
    if (wstr)
        wcsncpy(style.wstr, wstr, sizeof(style.wstr) / sizeof(wchar_t) - 1);
    else
        style.wstr[0] = L' ';
    return style;
}

/* -------------------------------------------------------------------------
   Lifecycle
   ------------------------------------------------------------------------- */

struct UiRuntime *ui_init(const UiConfig *cfg) {
    setlocale(LC_ALL, "");
    UiRuntime *ui = calloc(1, sizeof(*ui));
    if (!ui)
        return NULL;
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
    // screen = ui->screen;
    // tty_fp = ui->tty_fp;
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

    ui_bkgrnd(stdscr, &style_nt);
    if (ui->mouse_enabled)
        mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);

    curs_set(ui->cursor_visible ? 1 : 0);
    getmaxyx(stdscr, ui->lines, ui->cols);
    stdsfc = calloc(1, sizeof(*stdsfc));
    if (!stdsfc)
        return NULL;
    stdsfc->runtime = ui;
    stdsfc->parent = NULL;
    stdsfc->lines = ui->lines;
    stdsfc->cols = ui->cols;
    stdsfc->y = 0;
    stdsfc->x = 0;
    stdsfc->mwin[WIN] = stdscr;
    if (!stdsfc->mwin[WIN]) {
        free(stdsfc);
        return NULL;
    }
    stdsfc->mpan[WIN] = new_panel(stdsfc->mwin[WIN]);
    ui->panel_main = stdsfc->mpan[WIN];
    if (!stdsfc->mpan[WIN]) {
        delwin(stdsfc->mwin[WIN]);
        free(stdsfc);
        return NULL;
    }
    sfc_ptr = -1;
    return ui;
}

void ui_endwin() {
    ui_shutdown(ui_runtime);
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
        ui->screen = NULL;
    }
    if (ui->tty_fp) {
        fclose(ui->tty_fp);
    }
    f_curses_open = false;
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
    ui_bkgdset(s, BOX, &style_box);
    ui_scrollok(s, BOX, false);
    border_draw(s);
    border_title(s, wtitle);
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

int ui_surface_addwin(UiSurface *s, int w, int p, int lines, int cols, int y, int x) {
    s->mwin[w] = derwin(s->mwin[p], lines, cols, y, x);
    if (!s->mwin[w]) {
        free(s);
        return -1;
    }
    s->mpan[w] = new_panel(s->mwin[w]);
    keypad(s->mwin[w], true);
    ui_bkgd(s, w, &style_nt);
    ui_bkgdset(s, w, &style_nt);
    return 0;
}
void ui_wscrl(UiSurface *s, int w, int n) {
    if (!s->mwin[w])
        return;
    wscrl(s->mwin[w], n);
    ui_render(ui_runtime);
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

int ui_top_panel(UiSurface *s, int w) {
    if (!s)
        return -1;
    top_panel(s->mpan[w]);
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
    if (!s || !s->mwin[w])
        return -1;
    return wmove(s->mwin[w], y, x);
}
int ui_werase(UiSurface *s, int w) {
    if (!s || !s->mwin[w])
        return -1;
    return werase(s->mwin[w]);
}
int ui_wmove(UiSurface *s, int w, int y, int x) {
    if (!s || !s->mwin[w])
        return -1;
    return wmove(s->mwin[w], y, x);
}
// -------------------------------------------------------------------------
// Background and style management
// -------------------------------------------------------------------------

// for the entire window
int ui_bkgd(UiSurface *s, int w, const UiStyle *style) {
    if (!s)
        return -1;
    UiCell cch = ui_style_to_cch(style);
    wbkgrnd(s->mwin[w], &cch);
    return 0;
}
// for new content to be written to the window
int ui_bkgdset(UiSurface *s, int w, const UiStyle *style) {
    if (!s)
        return -1;
    UiCell cch = ui_style_to_cch(style);
    wbkgrndset(s->mwin[w], &cch);
    return 0;
}
// for the entire window
int ui_bkgrnd(WINDOW *win, const UiStyle *style) {
    if (!win)
        return -1;
    UiCell cc = ui_style_to_cch(style);
    wbkgrnd(win, &cc);
    return 0;
}
// for new content to be written to the window
int ui_bkgrndset(WINDOW *win, const UiStyle *style) {
    if (!win)
        return -1;
    UiCell cc = ui_style_to_cch(style);
    wbkgrndset(win, &cc);
    return 0;
}
/* ------------------------------------------------------------------------- */
void ui_qiflush() {
    qiflush();
}

int ui_getcchar(const UiCell *uc, wchar_t *wstr, attr_t *attrs, short *pair, void *opts) {
    return getcchar(uc, wstr, attrs, pair, opts);
}

int ui_setcchar(UiCell *uc, const wchar_t *wstr, attr_t attrs, short pair, const void *opts) {
    return setcchar(uc, wstr, attrs, pair, opts);
}

int ui_render(UiRuntime *ui) {
    (void)ui;
    update_panels();
    doupdate();
    return 0;
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
    RGB rgb;
    rgb.r = 255;
    rgb.g = 255;
    rgb.b = 255;
    int fg = ui_add_color_rgb(&rgb);
    rgb.r = 0;
    rgb.g = 0;
    rgb.b = 0;
    int bg = ui_add_color_rgb(&rgb);
    style->cp = ui_add_pair(fg, bg);
    return style;
}

void ui_style_destroy(UiStyle *style) {
    free(style);
}

UiStyle *ui_style_copy(const UiStyle *src) {
    UiStyle *dst = calloc(1, sizeof(*dst));
    memcpy(dst, src, sizeof(*dst));
    return dst;
}

UiStyle *ui_style_from_cch(const UiCell *cc) {
    UiStyle *style = calloc(1, sizeof(*style));
    if (!style)
        return NULL;
    getcchar(cc, style->wstr, &style->attrs, &style->cp, NULL);
    return style;
}

UiCell ui_style_to_cch(const UiStyle *style) {
    UiCell cc;
    setcchar(&cc, style->wstr, style->attrs, style->cp, NULL);
    return cc;
}

int ui_ncurses_color_pair_from_style(const UiStyle *style) {
    (void)style;
    return 0;
}

int ui_ncurses_style_apply(UiSurface *s, int w, const UiStyle *style) {
    if (!style)
        return -1;
    ui_bkgdset(s, w, style);
    return 0;
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
