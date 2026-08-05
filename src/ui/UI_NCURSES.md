<!-- mtoc-start -->

* [NCurses UI Backend](#ncurses-ui-backend)
  * [ui_ncurses.c](#ui_ncursesc)
    * [ui_ncurses_draw.c](#ui_ncurses_drawc)
    * [ui_ncurses_input.c](#ui_ncurses_inputc)
    * [ui_ncurses_internal.h](#ui_ncurses_internalh)
      * [UiRuntime](#uiruntime)
      * [UiSplitSurface](#uisplitsurface)
      * [UiSurface](#uisurface)
  * [ui_backend.h](#ui_backendh)
    * [UiMouseAction](#uimouseaction)
      * [UiBorderKind](#uiborderkind)
      * [UiColor](#uicolor)
      * [UiColorPair](#uicolorpair)
      * [UiStyle](#uistyle)
      * [UiEvent](#uievent)
      * [UiRect](#uirect)
      * [UiConfig](#uiconfig)
      * [UiBackend](#uibackend)
      * [UiCaps](#uicaps)

<!-- mtoc-end -->

# NCurses UI Backend

## ui_ncurses.c

```c
static int nc_alloc_color(uint8_t r, uint8_t g, uint8_t b)
static int nc_alloc_pair(int fg, int bg)
static inline int nc_scale_1000(uint8_t v)
int ui_bkgrnd(UiSurface *s, const UiStyle *style, const char *c)
int ui_bkgrndset(UiSurface *s, const UiStyle *style, const char *c)
UiSurface *ui_box_surface_new(UiRuntime *ui, UiSurface *parent, int lines, int cols, int y, int x, char *wtitle)
int ui_clear_screen(UiRuntime *ui)
int ui_cursor_enable(UiRuntime *ui, bool visible)
int ui_cursor_move(UiSurface *s, int y, int x)
UiBackend ui_get_backend(const UiRuntime *ui)
void ui_get_caps(const UiRuntime *ui, UiCaps *caps)
void ui_get_screen_size(UiRuntime *ui, int *lines, int *cols)
UiRuntime *ui_init(const UiConfig *cfg)
SCREEN *ui_ncurses_get_screen(const UiRuntime *ui)
PANEL *ui_ncurses_surface_get_panel(const UiSurface *s)
WINDOW *ui_ncurses_surface_get_win(const UiSurface *s)
int ui_render(UiRuntime *ui)
int ui_resume(UiRuntime *ui)
void ui_shutdown(UiRuntime *ui)
void ui_style_destroy(UiStyle *style)
UiStyle *ui_style_from_cch(const cchar_t *cch)
UiStyle *ui_style_new(void)
cchar_t ui_style_to_cch(const UiStyle *style, const char *c)
int ui_surface_clear(UiSurface *s)
void ui_surface_destroy(UiSurface *s)
int ui_surface_erase(UiSurface *s)
int ui_surface_hide(UiSurface *s)
int ui_surface_move(UiSurface *s, int y, int x)
UiSurface *ui_surface_new(UiRuntime *ui, UiSurface *parent, int lines, int cols, int y, int x)
int ui_surface_resize(UiSurface *s, int lines, int cols)
int ui_surface_show(UiSurface *s)
int ui_suspend(UiRuntime *ui)
```

### ui_ncurses_draw.c

```c
int mbstr_to_cc(char *in_str, cchar_t *cmplx_buf_s)
void parse_ansi(char *ansi_str, attr_t *attr, int *cpx)
int ui_draw_border(UiSurface *s, UiBorderKind kind, const UiStyle *style)
int ui_draw_box_title(UiSurface *s, int x, const UiStyle *style, const char *title)
int ui_draw_hline(UiSurface *s, int y, int x, int len, const UiStyle *style)
int ui_draw_text(UiSurface *s, int y, int x, const UiStyle *style, const char *text)
int ui_draw_text_n(UiSurface *s, int y, int x, const UiStyle *style, const char *text, int n)
int ui_draw_vline(UiSurface *s, int y, int x, int len, const UiStyle *style)
int ui_mvwadd_mbnstr(UiSurface *s, int y, int x, const char *text, int n)
int ui_mvwadd_mbnstr_fill(UiSurface *s, int y, int x, const char *text, int n)
int ui_ncurses_color_pair_from_style(const UiStyle *style)
int ui_ncurses_style_apply(WINDOW *win, const UiStyle *style)
int ui_surface_set_base(UiSurface *s, const UiStyle *style, uint32_t fill_ch)
int ui_surface_set_style(UiSurface *s, const UiStyle *style)
```

### ui_ncurses_input.c

```c
static UiKey translate_key(int ch)
int ui_get_event(UiRuntime *ui, UiSurface *target, UiEvent *ev, int timeout_ms)
```

### ui_ncurses_internal.h

#### UiRuntime

```c
struct UiRuntime {
    SCREEN *screen;
    FILE *tty_fp; 
    bool mouse_enabled;
    bool alt_screen;
    bool cursor_visible;
    int lines;
    int cols;
    PANEL *panel_main;
};
```

#### UiSplitSurface

```c
struct UiSplitSurface {
    WINDOW *box;
    WINDOW *win[4];
    PANEL *pan;
    int win_cnt;
    struct UiRuntime *runtime;
    struct UiSurface *parent;
    int y;
    int x;
    int lines;
    int cols;
    int split_y;
    int split_x;
    bool hidden;
    char name[XLEN];
    char title[XLEN];
};
```


#### UiSurface

```c
struct UiSurface {
    WINDOW *box;
    WINDOW *win;
    WINDOW *win2; // LEGACY - to be removed in future versions
    PANEL *pan;
    struct UiRuntime *runtime;
    struct UiSurface *parent;
    int y;
    int x;
    int lines;
    int cols;
    bool hidden;
    char name[XLEN];
    char title[XLEN];
};
```

## ui_backend.h


#### UiMouseAction

```c
typedef enum {
    UI_MOUSE_NONE = 0,
    UI_MOUSE_PRESS,
    UI_MOUSE_RELEASE,
    UI_MOUSE_DRAG,
    UI_MOUSE_SCROLL_UP,
    UI_MOUSE_SCROLL_DOWN
} UiMouseAction;
```


#### UiBorderKind

```c
typedef enum {
    UI_BORDER_NONE = 0,
    UI_BORDER_ASCII,
    UI_BORDER_LIGHT,
    UI_BORDER_ROUNDED
} UiBorderKind;
```

#### UiColor

```c
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
```

#### UiColorPair

```c
typedef struct {
    UiColor fg;
    UiColor bg;
    uint32_t idx;
} UiColorPair;
```

#### UiStyle

```c
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
```

#### UiEvent

```c
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
```

#### UiRect

```c
typedef struct {
    int y;
    int x;
    int lines;
    int cols;
} UiRect;
```

#### UiConfig

```c
typedef struct {
    bool enable_mouse;
    bool enable_alt_screen;
    bool cursor_visible;
    const char *tty_path; /**< optional TTY device path; NULL = auto-detect */
} UiConfig;
```

#### UiBackend

```c
typedef enum {
    UI_BACKEND_NCURSES = 0, /**< NCurses / NCursesW backend */
    UI_BACKEND_NOTCURSES    /**< NotCurses backend */
} UiBackend;
```

#### UiCaps

```c
typedef struct {
    bool truecolor;  /**< backend can display 24-bit RGB colors */
    bool palette256; /**< backend supports at least 256 palette entries */
    bool mouse;      /**< backend can deliver mouse events */
    bool unicode;    /**< backend handles full Unicode / UTF-8 correctly */
    bool resize;     /**< backend emits UI_KEY_RESIZE on terminal resize */
    int color_pairs; /**< max simultaneous color pairs; 0 = unlimited */
} UiCaps;
```
