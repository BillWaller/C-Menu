# NCurses UI Backend

## Surface Arrays

```c
ui_surface_box[MAXWIN]
ui_surface_win[MAXWIN]
ui_surface_win2[MAXWIN]
```

## UI Functions

```c
UiRuntime *ui_init(const UiConfig *cfg)
UiBackend ui_get_backend(const UiRuntime *ui)
void ui_get_caps(const UiRuntime *ui, UiCaps *caps)
int ui_suspend(UiRuntime *ui)
int ui_resume(UiRuntime *ui)
void ui_shutdown(UiRuntime *ui)
SCREEN *ui_ncurses_get_screen(const UiRuntime *ui)
PANEL *ui_ncurses_surface_get_panel(const UiSurface *s)
WINDOW *ui_ncurses_surface_get_win(const UiSurface *s)
UiSurface *ui_surface_new(UiRuntime *ui, UiSurface *parent, UiRect rect)
void ui_surface_destroy(UiSurface *s)
int ui_surface_set_base(UiSurface *s, const UiStyle *style, uint32_t fill_ch)
int ui_surface_set_style(UiSurface *s, const UiStyle *style)
static int nc_alloc_color(uint8_t r, uint8_t g, uint8_t b)
static int nc_alloc_pair(int fg, int bg)
static inline int nc_scale_1000(uint8_t v)
UiStyle *ui_style_new(void)
UiStyle *ui_style_from_cch(const cchar_t *cch)
cchar_t ui_style_to_cch(const UiStyle *style, const char *c)
int ui_ncurses_color_pair_from_style(const UiStyle *style)
int ui_ncurses_style_apply(WINDOW *win, const UiStyle *style)
void ui_style_destroy(UiStyle *style)
int ui_bkgd_set(UiSurface *s, const UiStyle *style, const char *c)
int ui_bkgrnd(UiSurface *s, const UiStyle *style, const char *c)
int ui_clear_screen(UiRuntime *ui)
int ui_cursor_enable(UiRuntime *ui, bool visible)
int ui_cursor_move(UiSurface *s, int y, int x)
void ui_get_screen_size(UiRuntime *ui, int *rows, int *cols)
int ui_draw_text(UiSurface *s, int y, int x, const UiStyle *style, const char *text)
int ui_draw_text_n(UiSurface *s, int y, int x, const UiStyle *style, const char *text, size_t n)
int ui_render(UiRuntime *ui)
int ui_surface_clear(UiSurface *s)
int ui_surface_erase(UiSurface *s)
int ui_surface_hide(UiSurface *s)
int ui_surface_move(UiSurface *s, int y, int x)
int ui_surface_resize(UiSurface *s, int rows, int cols)
int ui_surface_show(UiSurface *s)
int ui_draw_border(UiSurface *s, UiBorderKind kind, const UiStyle *style)
int ui_draw_box_title(UiSurface *s, int x, const UiStyle *style,
int ui_draw_hline(UiSurface *s, int y, int x, int len, const UiStyle *style)
int ui_draw_vline(UiSurface *s, int y, int x, int len, const UiStyle *style)
static UiKey translate_key(int ch)
int ui_get_event(UiRuntime *ui, UiSurface *target, UiEvent *ev, int timeout)
```

## ui_ncurses_internal.h

```c
struct UiRuntime {
    SCREEN *screen;
    FILE *tty_fp;
    bool mouse_enabled;
    bool alt_screen;
    bool cursor_visible;
    int rows;
    int cols;
};

struct UiSurface {
    WINDOW *win;
    PANEL *pan;
    struct UiRuntime *runtime;
    struct UiSurface *parent;
    int y;
    int x;
    int rows;
    int cols;
    bool hidden;
    char name[XLEN];
    char title[XLEN];
};
```


## ui_ncurses.h

```c
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
    uint32_t ch;
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
    const char *tty_path;
} UiConfig;

typedef enum {
    UI_BACKEND_NCURSES = 0,
    UI_BACKEND_NOTCURSES
} UiBackend;

typedef struct {
    bool truecolor;
    bool palette256;
    bool mouse;
    bool unicode;
    bool resize;
    int  color_pairs;
} UiCaps;

```
