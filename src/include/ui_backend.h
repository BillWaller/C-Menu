#ifndef UI_BACKEND_H
#define UI_BACKEND_H 1

/** @file ui_backend.h
    @ingroup ui_backend
    @brief Backend API for terminal UI library
*/
#define _XOPEN_SOURCE_EXTENDED 1
#define _GNU_SOURCE
#define NCURSES_WIDECHAR 1

#ifdef UAL_UI
#include "../ui/ui_ncurses_internal.h"
#include <ncursesw/ncurses.h>
#include <ncursesw/panel.h>
#endif
#ifdef NOTCURSES_UI
#include "../ui/ui_notcurses_internal.h"
#include <notcurses/notcurses.h>
#endif
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#define MAX_SFC 30
#define MAXWIN 30

typedef union UiChannels UiChannels;
typedef struct UiPair UiPair;
typedef uint16_t UiStyle;
typedef uint16_t UiColor;

typedef struct UiRuntime UiRuntime;
typedef struct UiSurface UiSurface;
typedef struct UiSplitSurface UiSplitSurface;

typedef enum {
    UI_KEY_NONE = 0,
    UI_KEY_CHAR,
    UI_KEY_ENTER,
    UI_KEY_ESCAPE,
    UI_KEY_BACKSPACE,
    UI_KEY_TAB,
    UI_KEY_BTAB,
    UI_KEY_UP,
    UI_KEY_DOWN,
    UI_KEY_LEFT,
    UI_KEY_RIGHT,
    UI_KEY_HOME,
    UI_KEY_END,
    UI_KEY_PGUP,
    UI_KEY_PGDN,
    UI_KEY_INSERT,
    UI_KEY_DELETE,
    UI_KEY_RESIZE,
    UI_KEY_MOUSE,
    UI_KEY_F1,
    UI_KEY_F2,
    UI_KEY_F3,
    UI_KEY_F4,
    UI_KEY_F5,
    UI_KEY_F6,
    UI_KEY_F7,
    UI_KEY_F8,
    UI_KEY_F9,
    UI_KEY_F10,
    UI_KEY_F11,
    UI_KEY_F12
} UiKey;

typedef enum {
    UI_MOUSE_NONE = 0,
    UI_MOUSE_PRESS,
    UI_MOUSE_RELEASE,
    UI_MOUSE_DRAG,
    UI_MOUSE_SCROLL_UP,
    UI_MOUSE_SCROLL_DOWN
} UiMouseAction;

typedef enum {
    UI_BORDER_NONE = 0,
    UI_BORDER_ASCII,
    UI_BORDER_LIGHT,
    UI_BORDER_ROUNDED
} UiBorderKind;

#define ALLWINS (uint)1000

typedef struct {
    wchar_t wc;       // Wide character    4-bytes
    short attrs;      // attributes        2-bytes
    short color_pair; // color pair index  2-bytes
} UiCchar64;          //           total   8-bytes

typedef struct {
    UiKey key;
    uint32_t ch; /* Unicode codepoint when key == UI_KEY_CHAR */
    bool alt;
    bool ctrl;
    bool shift;
    uint y;
    uint x;
    bool active;
    int chyron;
    uint in_win;
    int bstate;
    UiMouseAction mouse_action;
    bool mouse_inside;
    char keybound[16];
} UiEvent;

typedef struct {
    int y;
    int x;
    int lines;
    int cols;
} UiRect;

#define UI_COLORS 512
#define UI_PAIRS 512

/** @struct UiConfig
   @brief Structure representing the configuration options for the UI runtime.
   @ingroup ui_backend
   @details The UiConfig structure encapsulates the configuration options that
   can be set when initializing the UI runtime. This includes enabling mouse
   support, using an alternate screen buffer, and controlling cursor visibility.
   The enable_mouse field allows you to specify whether mouse input should be
   captured and processed by the UI. The enable_alt_screen field determines
   whether the UI should use an alternate screen buffer, which can help prevent
   cluttering the main terminal screen. The cursor_visible field controls
   whether the cursor is visible while the UI is active, which can enhance the
   user experience in certain applications. The tty_path field, if non-NULL,
   specifies the terminal device to use; if NULL, the backend auto-detects the
   terminal (typically via stderr). By configuring these options appropriately,
   you can tailor the behavior of the UI runtime to suit your application's
   needs.
   @see ui_init
*/
typedef struct {
    bool enable_mouse;
    bool enable_alt_screen;
    bool cursor_visible;
    const char *tty_path; /**< optional TTY device path; NULL = auto-detect */
} UiConfig;

/** @enum UiBackend
   @brief Identifies which terminal library is powering the UI runtime.
   @ingroup ui_backend
*/
typedef enum {
    UI_BACKEND_NCURSES = 0, /**< NCurses / NCursesW backend */
    UI_BACKEND_NOTCURSES    /**< NotCurses backend */
} UiBackend;

/** @struct UiCaps
   @brief Capability flags reported by the active UI backend.
   @ingroup ui_backend
   @details Query via ui_get_caps() after ui_init() succeeds.  Differences
   between backends are made explicit through these flags so application code
   can adapt rather than assume.
   @see ui_get_caps
*/
typedef struct {
    bool truecolor;  /**< backend can display 24-bit RGB colors */
    bool palette256; /**< backend supports at least 256 palette entries */
    bool mouse;      /**< backend can deliver mouse events */
    bool unicode;    /**< backend handles full Unicode / UTF-8 correctly */
    bool resize;     /**< backend emits UI_KEY_RESIZE on terminal resize */
    int color_pairs; /**< max simultaneous color pairs; 0 = unlimited */
} UiCaps;

/* @name UI Backend API
   @ingroup ui_backend
   @details The following functions define the API for implementing a backend
   for the terminal UI library. These functions cover the initialization and
   shutdown of the UI runtime, management of surfaces, drawing operations,
   input handling, and cursor control. Each function is designed to be
   implemented by the backend according to the specifications provided in
   the function comments. By implementing these functions, you can create a
   functional backend that allows the terminal UI library to render and
   interact with users effectively.
    @note: ui_init should return NULL on failure and set an appropriate error
   message that can be retrieved by the caller. The error message should provide
   details about the reason for the failure, such as issues with terminal
   capabilities, resource allocation failures, or unsupported features. This
   allows the caller to handle initialization errors gracefully and provide
   feedback to the user.
   @see ui_backend.h
*/
UiRuntime *ui_init(const UiConfig *cfg);
void ui_shutdown(UiRuntime *ui);
int ui_render(UiRuntime *ui);
int ui_clear();
int ui_erase();
int ui_suspend(UiRuntime *ui);
int ui_resume(UiRuntime *ui);
UiSurface *ui_surface_new(UiRuntime *ui, uint w, UiSurface *parent, uint p, uint lines, uint cols, uint y, uint x);
UiSurface *ui_box_surface_new(UiRuntime *ui, UiSurface *parent, uint p, uint lines, uint cols, uint y, uint x, char *title);
int ui_surface_addwin(UiSurface *s, uint w, uint p, uint lines, uint cols, uint y, uint x);
int ui_surface_addpad(UiSurface *s, uint w, uint view_win, uint lines, uint cols);
void ui_surface_destroy(UiSurface *s);
int ui_wmove(UiSurface *s, uint w, uint y, uint x);
int ui_wresize(UiSurface *s, uint w, uint lines, uint cols);
int ui_wclear(UiSurface *s, uint w);
int ui_werase(UiSurface *s, uint w);
int ui_wset_base(UiSurface *s, uint w, const UiStyle *style, uint32_t fill_ch);
int ui_wset_style(UiSurface *s, uint w, const UiStyle *style);
int ui_draw_vline(UiSurface *s, uint w, uint y, uint x, uint len, const UiStyle *style);
int ui_draw_border(UiSurface *s, uint w, UiBorderKind kind, const UiStyle *style);
int ui_draw_box_title(UiSurface *s, uint w, uint x, const UiStyle *style, const char *title);
int ui_wshow(UiSurface *s, uint w);
int ui_whide(UiSurface *s, uint w);
int ui_get_event(UiSurface *s, uint w, UiEvent *ev, int timeout_ms);
int ui_get_event_multi(UiSurface *s, uint w, UiEvent *ev, int timeout_ms);
int ui_get_event_no_mouse(UiSurface *surface, uint w, UiEvent *ev);
int ui_wmove(UiSurface *s, uint w, uint y, uint x);
int ui_cursor_enable(UiRuntime *ui, bool visible);
int ui_curs_set(bool visibility);
int ui_bkgd(UiSurface *s, uint w, const UiCell *cell);
int ui_bkgdset(UiSurface *s, uint w, const UiCell *cell);
int ui_bkgrnd(WINDOW *win, const UiCell *cell);
int ui_bkgrndset(WINDOW *win, const UiCell *cell);

int ui_wscrl(UiSurface *s, uint w, uint n);
int ui_wclrtoeol(UiSurface *s, uint w);
int ui_wclrtobot(UiSurface *s, uint w);
void ui_getyx(UiSurface *s, uint w, uint *lines, uint *cols);
void ui_getmaxyx(UiSurface *s, uint w, uint *lines, uint *cols);
uint ui_getmaxx(UiSurface *s, uint w);
uint ui_getmaxy(UiSurface *s, uint w);
int ui_draw_ch(UiSurface *s, uint w, uint y, uint x, const char c);
int ui_draw_text(UiSurface *s, uint w, uint y, uint x, const char *text);
int ui_draw_text_n(UiSurface *s, uint w, uint y, uint x, const char *text, size_t n);
int ui_draw_text_fill(UiSurface *s, uint w, uint y, uint x, const char *text, size_t n);
int ui_waddstr(UiSurface *s, uint w, const char *text);
int ui_mvaddstr(UiSurface *s, uint w, uint y, uint x, const char *text);
int ui_mvwaddch(UiSurface *s, uint w, uint y, uint x, const char c);
int ui_mvwaddstr(UiSurface *s, uint w, uint y, uint x, const char *text);
int ui_mvwadd_style(UiSurface *s, uint w, uint y, uint x, const UiStyle *style);
int ui_waddwstr(UiSurface *s, uint w, const wchar_t *wstr);
int ui_mvwaddwstr(UiSurface *s, uint w, uint y, uint x, const wchar_t *wstr);
int ui_waddnwstr(UiSurface *s, uint w, const wchar_t *wstr, uint n);
int ui_mvwaddnwstr(UiSurface *s, uint w, uint y, uint x, const wchar_t *wstr, uint n);
int ui_mvwaddstr_fill(UiSurface *s, uint w, uint y, uint x, char *str, uint n);
int ui_mvwadd_mbstr(UiSurface *s, uint w, uint y, uint x, const char *text);
int ui_mvwadd_mbnstr(UiSurface *s, uint w, uint y, uint x, const char *text, uint n);
int ui_mvwadd_mbnstr_fill(UiSurface *s, uint w, uint y, uint x, const char *text, uint n);
int ui_setscrreg(UiSurface *s, uint w, uint top, uint bottom);
int ui_scrollok(UiSurface *s, uint w, bool enable);
int ui_keypad(UiSurface *s, uint w, bool enable);
int ui_idlok(UiSurface *s, uint w, bool enable);
int ui_idcok(UiSurface *s, uint w, bool enable);
void ui_update_panels();
int ui_doupdate();
int ui_wnoutrefresh(UiSurface *s, uint w);
int ui_draw_hline(UiSurface *s, uint w, uint y, uint x, uint len, const UiStyle *style);
int ui_mousemask(int mask);
int ui_mice_enable(int mask);
void ui_get_screen_size(UiRuntime *ui, uint *lines, uint *cols);
int ui_add_color_rgb(RGB *rgb);
int ui_add_color_hex(char *s);
UiCell ui_cell_from_hex(const char *fg, const char *bg, const attr_t attrs, const wchar_t *wstr);

uint mbstr_to_cellstr(UiCell *cmplx_buf, const char *str, const UiCell *cell_base, uint *pos, const uint atmost);
int ui_getcchar(const UiCell *uc, wchar_t *wstr, attr_t *attrs, ushort *pair, void *opts);
int ui_setcchar(cchar_t *wch, const wchar_t *wc, const attr_t attrs, short pair, const void *opts);
int ui_surface_move(UiSurface *s, uint w, uint y, uint x);
int ui_surface_resize(UiSurface *s, uint w, uint lines, uint cols);
int ui_surface_clear(UiSurface *s, uint w);
int ui_surface_erase(UiSurface *s, uint w);
int ui_surface_show(UiSurface *s, uint w);
int ui_surface_hide(UiSurface *s, uint w);
int ui_cursor_move(UiSurface *s, uint w, uint y, uint x);
int ui_ncurses_apply_style_from_cell(UiSurface *s, uint w, const UiCell *cell);

#ifdef NOTCURSES_UI
// How do you convert "NCurses" to "Notcurses"?
// Insert "ot" after "N".
typedef uint16_t attr_t;
typedef struct UiCell UiCell;
int ui_init_color(uint16_t color, uint8_t r, uint8_t g, uint8_t b);
int ui_color_content(uint16_t color, uint8_t *r, uint8_t *g, uint8_t *b);
int ui_init_pair(uint16_t pair, uint16_t fg, uint16_t bg);
int ui_pair_content(uint16_t pair, uint16_t *fg, uint16_t *bg);
uint ui_add_pair(uint16_t fg, uint16_t bg);
int ui_chg_pair(uint16_t pair, uint16_t fg, uint16_t bg);
int ui_get_pair(uint16_t pair, uint16_t *fg, uint16_t *bg);
int ui_add_color(uint16_t r, uint16_t g, uint16_t b);
int ui_chg_color_rgb(uint16_t color, RGB *rgb);
int ui_chg_color_hex(uint16_t color, char *s);
int ui_get_color(uint16_t color, RGB *rgb);

int ui_wadd_cell(UiSurface *s, uint w, const nccell *uic);
int ui_mvwadd_cell(UiSurface *s, uint w, uint y, uint x, const nccell *uic);
int ui_wadd_cellstr(UiSurface *s, uint w, nccell *uic);
int ui_mvwadd_cellstr(UiSurface *s, uint w, uint y, uint x, nccell *uic);
int ui_wadd_cellnstr(UiSurface *s, uint w, nccell *uic, uint n);
int ui_mvwadd_cellnstr(UiSurface *s, uint w, uint y, uint x, nccell *uic, uint n);
int ui_wadd_wch(UiSurface *s, uint w, const nccell *uic);
int ui_mvwadd_wch(UiSurface *s, uint w, uint y, uint x, const struct nccell *uic);
int ui_wadd_wchstr(UiSurface *s, uint w, struct nccell *uic);
int ui_mvwadd_wchstr(UiSurface *s, uint w, uint y, uint x, struct nccell *uic);
int ui_wadd_wchnstr(UiSurface *s, uint w, struct nccell *uic, uint n);
int ui_mvwadd_wchnstr(UiSurface *s, uint w, uint y, uint x, struct nccell *uic, uint n);
int ui_setnccell(UiCell *uic, const wchar_t *wstr, const UiStyle *style, const UiPair *pair);
int ui_getnccell(UiCell *uic, wchar_t *wstr, UiStyle *style, UiPair *pair);
UiChannels ui_channels_from_hex(const char *fg, const char *bg);
extern UiSurface *stdsfc;
extern uint LINES, COLS;
int ui_compose_nccell(struct UiCell *uic, const wchar_t *wstr, const UiStyle *style);
int ui_decompose_nccell(struct UiCell *uic, wchar_t *wstr, UiStyle *style, uint64_t *channels);
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} STDRGB;
#define ERR -1
#else
int ui_init_color(uint color, uint r, uint g, uint b);
int ui_color_content(uint color, uint *r, uint *g, uint *b);
int ui_init_pair(uint pair, uint fg, uint bg);
int ui_pair_content(uint pair, uint *fg, uint *bg);
int ui_channels_from_pair(uint pair, union UiChannels *ui_channels);
uint ui_add_pair(uint fg, uint bg);
int ui_chg_pair(uint pair, uint fg, uint bg);
int ui_get_pair(uint pair, uint *fg, uint *bg);
int ui_add_color(uint r, uint g, uint b);
int ui_add_color_hex(char *s);
int ui_chg_color_rgb(uint color, RGB *rgb);
int ui_chg_color_hex(uint color, char *s);
int ui_get_color(uint color, RGB *rgb);
typedef struct {
    uint r;
    uint g;
    uint b;
} STDRGB;
#endif
void ui_endwin();
RGB ui_hex_to_rgb(char *s);
void ui_restore_wins();
int ui_top_panel(UiSurface *s, uint w);

/* @brief backend identification and capability query
   @ingroup ui_backend */

/** @brief Return which backend is powering this runtime.
   @ingroup ui_backend
   @param ui The UiRuntime returned by ui_init().
   @return The UiBackend enum value for the compiled-in backend.
*/
UiBackend ui_get_backend(const UiRuntime *ui);

/** @brief Query the capabilities of the active backend.
   @ingroup ui_backend
   @param ui  The UiRuntime returned by ui_init().
   @param caps Output structure filled with capability flags.
*/
void ui_get_caps(const UiRuntime *ui, UiCaps *caps);
extern STDRGB std_color[];
extern UiRuntime *ui_runtime;
extern UiSurface *ui_surface[MAX_SFC];

extern STDRGB std_color[];
extern uint ui_color_cnt;
extern uint ui_pair_cnt;

#endif
