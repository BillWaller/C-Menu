#ifndef UI_BACKEND_H
#define UI_BACKEND_H 1

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#define MAXWIN 30

typedef struct UiRuntime UiRuntime;

typedef struct UiSurface UiSurface;

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
} UiEvent;

typedef struct {
    int y;
    int x;
    int rows;
    int cols;
} UiRect;

typedef struct {
    bool enable_mouse;
    bool enable_alt_screen;
    bool cursor_visible;
} UiConfig;

UiRuntime *ui_init(UiRuntime *ui);
void ui_shutdown(UiRuntime *ui);
void ui_get_screen_size(UiRuntime *ui, int *rows, int *cols);
int ui_render(UiRuntime *ui);
int ui_clear_screen(UiRuntime *ui);
int ui_suspend(UiRuntime *ui);
int ui_resume(UiRuntime *ui);

UiSurface *ui_surface_new(UiRuntime *ui, UiSurface *parent, UiRect rect);
void ui_surface_destroy(UiSurface *s);
int ui_surface_move(UiSurface *s, int y, int x);
int ui_surface_resize(UiSurface *s, int rows, int cols);
int ui_surface_clear(UiSurface *s);
int ui_surface_erase(UiSurface *s);
int ui_surface_set_base(UiSurface *s, const UiStyle *style, uint32_t fill_ch);
int ui_surface_set_style(UiSurface *s, const UiStyle *style);

int ui_draw_text(UiSurface *s, int y, int x, const UiStyle *style, const char *text);
int ui_draw_text_n(UiSurface *s, int y, int x, const UiStyle *style, const char *text, size_t n);
int ui_draw_hline(UiSurface *s, int y, int x, int len, const UiStyle *style);
int ui_draw_vline(UiSurface *s, int y, int x, int len, const UiStyle *style);
int ui_draw_border(UiSurface *s, UiBorderKind kind, const UiStyle *style);
int ui_draw_box_title(UiSurface *s, int x, const UiStyle *style, const char *title);
int ui_bkgrnd(UiSurface *, const UiStyle *, const char *);
int ui_bkgd_set(UiSurface *, const UiStyle *, const char *);
/* @brief clipping / visibility
@ingroup ui_backend */
int ui_surface_show(UiSurface *s);
int ui_surface_hide(UiSurface *s);
int ui_get_event(UiRuntime *ui, UiSurface *target, UiEvent *ev, int timeout_ms);
int ui_cursor_move(UiSurface *s, int y, int x);
int ui_cursor_enable(UiRuntime *ui, bool visible);

extern UiSurface *ui_surface_box[MAXWIN];
extern UiSurface *ui_surface_win[MAXWIN];
extern UiSurface *ui_surface_win2[MAXWIN];

#endif
