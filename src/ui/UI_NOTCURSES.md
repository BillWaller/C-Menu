# NotCurses UI Backend


## Surface Arrays

```c
ui_surface_box[MAXWIN]
ui_surface_win[MAXWIN]
ui_surface_win2[MAXWIN]
```

## UI Functions

```c

uint32_t ui_notcurses_attrs_from_style(const UiStyle *style)
uint64_t ui_notcurses_channels_from_style(const UiStyle *style)
struct notcurses *ui_notcurses_get_nc(const UiRuntime *ui)
struct ncplane *ui_notcurses_surface_get_plane(const UiSurface *s)

int ui_bkgd_set(UiSurface *s, const UiStyle *style, const char *c)
int ui_bkgrnd(UiSurface *s, const UiStyle *style, const char *c)
int ui_clear_screen(UiRuntime *ui)
int ui_cursor_enable(UiRuntime *ui, bool visible)
int ui_cursor_move(UiSurface *s, int y, int x)
UiBackend ui_get_backend(const UiRuntime *ui)
void ui_get_caps(const UiRuntime *ui, UiCaps *caps)
void ui_get_screen_size(UiRuntime *ui, int *rows, int *cols)
UiRuntime *ui_init(const UiConfig *cfg)
int ui_render(UiRuntime *ui)
int ui_resume(UiRuntime *ui)
void ui_shutdown(UiRuntime *ui)
int ui_surface_clear(UiSurface *s)
void ui_surface_destroy(UiSurface *s)
int ui_surface_erase(UiSurface *s)
int ui_surface_hide(UiSurface *s)
int ui_surface_move(UiSurface *s, int y, int x)
UiSurface *ui_surface_new(UiRuntime *ui, UiSurface *parent, UiRect rect)
int ui_surface_resize(UiSurface *s, int rows, int cols)
int ui_surface_set_base(UiSurface *s, const UiStyle *style, uint32_t fill_ch)
int ui_surface_set_style(UiSurface *s, const UiStyle *style)
int ui_surface_show(UiSurface *s)
int ui_suspend(UiRuntime *ui)
int ui_draw_border(UiSurface *s, UiBorderKind kind, const UiStyle *style)
int ui_draw_box_title(UiSurface *s, int x, const UiStyle *style, const char *title)
int ui_draw_hline(UiSurface *s, int y, int x, int len, const UiStyle *style)
int ui_draw_text(UiSurface *s, int y, int x, const UiStyle *style, const char *text)
int ui_draw_text_n(UiSurface *s, int y, int x, const UiStyle *style, const char *text, size_t n)
int ui_draw_vline(UiSurface *s, int y, int x, int len, const UiStyle *style)
static UiKey translate_nckey(uint32_t id, const ncinput *ni)
int ui_get_event(UiRuntime *ui, UiSurface *target, UiEvent *ev, int timeout)
```
