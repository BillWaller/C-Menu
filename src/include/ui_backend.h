#ifndef UI_BACKEND_H
#define UI_BACKEND_H 1

/** @file ui_backend.h
    @ingroup ui_backend
    @brief Backend API for terminal UI library
*/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#define MAXWIN 30

typedef struct UiRuntime UiRuntime;
typedef struct UiSurface UiSurface;
typedef struct UiSplitSurface UiSplitSurface;
typedef struct UiStyle UiStyle;

#define ALLWINS -1

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

// @brief UiCchar is a structure representing a wide character with attributes
// and color pair index.
//
// At the moment, the following 64-bit structure, UiCchar is the primary
// candidate for substitution of the NCursews cchar_t structure. I believe
// cchar_t is at least 16-bytes.
//
// View applies colors and attributes derived from ANSI SGR escape sequences,
// highlights matching search results, and breaks lines at word boundaries while
// maintaining Unicode character boundaries. Currently, output to the display is
// deferred until the formatting is complete, and the entire formatted line is
// transferred to the pad.
//
// Unlike NCurses, Notcurses only provides a function to move a single nccell at
// a time to a plane.
//
//
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
    int y;
    int x;
    int chyron;
    int in_win;
    UiMouseAction mouse_action;
    bool mouse_inside;
    char keybound[256];
} UiEvent;

typedef struct {
    int y;
    int x;
    int lines;
    int cols;
} UiRect;

typedef struct {
    union {
        struct {
            uint8_t b, g, r, a;
        };
        uint32_t argb;
    };
} UiRGBA;

#define NC_MAX_COLORS 512
#define NC_MAX_PAIRS 512

static int ui_color_idx = 1;
static int ui_color_cnt = 1; /* colors allocated via init_extended_color */
static int ui_color_pair_idx = 1;
static int ui_color_pair_cnt = 1;
static int ui_style_idx = 1;
static int ui_style_cnt = 1;

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
void ui_get_screen_size(UiRuntime *ui, int *lines, int *cols);
int ui_render(UiRuntime *ui);
int ui_clear_screen(UiRuntime *ui);
int ui_suspend(UiRuntime *ui);
int ui_resume(UiRuntime *ui);
UiSurface *ui_surface_new(UiRuntime *ui, int w, UiSurface *parent, int p, int lines, int cols, int y, int x);
UiSurface *ui_box_surface_new(UiRuntime *ui, UiSurface *parent, int p, int lines, int cols, int y, int x, char *title);
int ui_surface_addwin(UiSurface *s, int w, int p, int lines, int cols, int y, int x);
int ui_surface_addpad(UiSurface *s, int w, int view_win, int lines, int cols);
void ui_surface_destroy(UiSurface *s);
int ui_surface_move(UiSurface *s, int w, int y, int x);
int ui_surface_resize(UiSurface *s, int w, int lines, int cols);
int ui_surface_clear(UiSurface *s, int w);
int ui_surface_erase(UiSurface *s, int w);
int ui_surface_set_base(UiSurface *s, int w, const UiStyle *style, uint32_t fill_ch);
int ui_surface_set_style(UiSurface *s, int w, const UiStyle *style);
int ui_draw_vline(UiSurface *s, int w, int y, int x, int len, const UiStyle *style);
int ui_draw_border(UiSurface *s, int w, UiBorderKind kind, const UiStyle *style);
int ui_draw_box_title(UiSurface *s, int w, int x, const UiStyle *style, const char *title);
/* @brief clipping / visibility
@ingroup ui_backend */
int ui_surface_show(UiSurface *s, int w);
int ui_surface_hide(UiSurface *s, int w);
int ui_get_event(UiRuntime *ui, UiSurface *sfc, int w, UiEvent *ev, int timeout_ms);
int ui_get_event_multi(UiRuntime *ui, UiSurface *sfc, int w, UiEvent *ev, int timeout_ms);
int ui_get_event_no_mouse(UiSurface *surface, int w, UiEvent *ev);
int ui_cursor_move(UiSurface *s, int w, int y, int x);
int ui_cursor_enable(UiRuntime *ui, bool visible);
void ui_curs_set(int visibility);
int ui_bkgd(UiSurface *s, int w, const UiStyle *style, const char *c);
int ui_bkgdset(UiSurface *s, int w, const UiStyle *style, const char *c);
void ui_qiflush();
void ui_wscrl(UiSurface *s, int w, int n);
int ui_wclrtoeol(UiSurface *s, int w);
int ui_wclrtobot(UiSurface *s, int w);
void ui_getyx(UiSurface *sfc, int w, int *lines, int *cols);
void ui_getmaxyx(UiSurface *s, int w, int *lines, int *cols);
int ui_getmaxx(UiSurface *s, int w);
int ui_getmaxy(UiSurface *s, int w);
int ui_draw_ch(UiSurface *s, int w, int y, int x, const UiStyle *style, const char c);
int ui_draw_text(UiSurface *s, int w, int y, int x, const UiStyle *style, const char *text);
int ui_draw_text_n(UiSurface *s, int w, int y, int x, const UiStyle *style, const char *text, size_t n);
int ui_draw_text_fill(UiSurface *s, int w, int y, int x, const UiStyle *style, const char *text, size_t n);
int ui_waddstr(UiSurface *s, int w, const char *text);
int ui_waddnwstr(UiSurface *s, int w, const UiStyle *style, const wchar_t *wstr, int n);
int ui_mvaddstr(UiSurface *s, int w, int y, int x, const char *text);
int ui_mvwaddch(UiSurface *s, int w, int y, int x, const char c);
int ui_mvwaddstr(UiSurface *s, int w, int y, int x, const char *text);
int ui_mvwaddnwstr(UiSurface *s, int w, int y, int x, const UiStyle *style, const wchar_t *wstr, int n);
int ui_mvwaddstr_fill(UiSurface *sfc, int w, int y, int x, char *s, int n);
int ui_mvwadd_mbstr(UiSurface *s, int w, int y, int x, const char *text);
int ui_mvwadd_mbnstr(UiSurface *s, int w, int y, int x, const char *text, int n);
int ui_mvwadd_mbnstr_fill(UiSurface *s, int w, int y, int x, const char *text, int n);

void ui_setscrreg(UiSurface *s, int w, int top, int bottom);
void ui_scrollok(UiSurface *s, int w, bool enable);
void ui_keypad(UiSurface *s, int w, bool enable);
void ui_idlok(UiSurface *s, int w, bool enable);
void ui_idcok(UiSurface *s, int w, bool enable);
void ui_update_panels();
void ui_doupdate();
void ui_wnoutrefresh(UiSurface *s, int w);
int ui_draw_hline(UiSurface *s, int w, int y, int x, int len, const UiStyle *style);
void ui_erase();
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

UiStyle ui_style_from_hex(const char *fg, const char *bg, int attrs, const char *str);
UiStyle *ui_style_copy(const UiStyle *src);

extern UiRuntime *ui_runtime;
extern UiSurface *ui_surface[MAXWIN];

#endif
