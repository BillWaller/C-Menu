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

typedef struct {
    union {
        struct {
            uint8_t a; // alpha    LSB (Little Endian order)
            uint8_t b; // blue
            uint8_t g; // green
            uint8_t r; // red      MSB (Little Endian order)
        };
        struct {
            uint32_t rgba; // 0xRRGGBBAA   red, green, blue, alpha
        };
    };
    bool use_rgb;
    uint32_t index;
} UiColor;

typedef struct {
    UiColor fg;
    UiColor bg;
    uint32_t idx;
} UiColorPair;

typedef struct {
    UiColor fg;
    UiColor bg;
    int cp;
    bool bold;
    bool dim;
    bool italic;
    bool underline;
    bool blink;
    bool reverse;
    bool invis;
} UiStyle;

typedef struct {
    UiKey key;
    uint32_t ch; /* Unicode codepoint when key == UI_KEY_CHAR */
    bool alt;
    bool ctrl;
    bool shift;
    int y;
    int x;
    UiMouseAction mouse_action;
    bool mouse_inside;
    char keybound[32];
} UiEvent;

typedef struct {
    int y;
    int x;
    int rows;
    int cols;
} UiRect;

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
void ui_get_screen_size(UiRuntime *ui, int *rows, int *cols);
int ui_render(UiRuntime *ui);
int ui_clear_screen(UiRuntime *ui);
int ui_suspend(UiRuntime *ui);
int ui_resume(UiRuntime *ui);

UiSurface *ui_surface_new(UiRuntime *ui, UiSurface *parent, int rows, int cols, int y, int x);
UiSurface *ui_box_surface_new(UiRuntime *ui, UiSurface *parent, int rows, int cols, int y, int x, char *title);
void ui_surface_destroy(UiSurface *s);
int ui_surface_move(UiSurface *s, int y, int x);
int ui_surface_resize(UiSurface *s, int rows, int cols);
int ui_surface_clear(UiSurface *s);
int ui_surface_erase(UiSurface *s);
int ui_surface_set_base(UiSurface *s, const UiStyle *style, uint32_t fill_ch);
int ui_surface_set_style(UiSurface *s, const UiStyle *style);
int ui_mvwaddstr(UiSurface *s, int y, int x, const char *text);
int ui_wclrtoeol(UiSurface *s);

int ui_draw_text(UiSurface *s, int y, int x, const UiStyle *style, const char *text);
int ui_draw_text_n(UiSurface *s, int y, int x, const UiStyle *style, const char *text, size_t n);
int ui_draw_hline(UiSurface *s, int y, int x, int len, const UiStyle *style);
int ui_draw_vline(UiSurface *s, int y, int x, int len, const UiStyle *style);
int ui_draw_border(UiSurface *s, UiBorderKind kind, const UiStyle *style);
int ui_draw_box_title(UiSurface *s, int x, const UiStyle *style, const char *title);
int ui_bkgrnd(UiSurface *, const UiStyle *, const char *);
int ui_bkgrndset(UiSurface *, const UiStyle *, const char *);
/* @brief clipping / visibility
@ingroup ui_backend */
int ui_surface_show(UiSurface *s);
int ui_surface_hide(UiSurface *s);
int ui_get_event(UiRuntime *ui, UiSurface *target, UiEvent *ev, int timeout_ms);
int ui_cursor_move(UiSurface *s, int y, int x);
int ui_cursor_enable(UiRuntime *ui, bool visible);

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

extern UiRuntime *ui_runtime;
extern UiSurface *ui_surface[MAXWIN];

#endif
