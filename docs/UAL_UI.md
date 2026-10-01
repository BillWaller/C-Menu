![UAL_UI](../screenshots/UAL_UI.png)

# Uniform Abstraction Layer for User Interfaces (UAL_UI)

## Introduction

The UAL_UI provides a Uniform Abstraction Layer that allows interactive programs to choose from multiple user interface backends without changing the program code. Currently, the UAL_UI supports NCurses and Notcurses, which are both terminal-based user interface libraries. In the future, additional backends may be added to support graphical user interfaces, such as GTK or Qt.

The UAL_UI uses NCurses-style function names, with "ui_" prepended. The
abstraction layer provides NCurses-style functionality while obscuring the more
complicated details of the underlying implementation. This allows developers to
write portable code that works in an identical manner across different user
interface backends.

The programmer can use ASCII text, UTF-8 encoded text, Unicode Code Points, or
ANSI SGR (Select Graphic Rendition) escape sequences, which the UAL_UI will transform into NCurses complex character strings or Notcurses extended grapheme clusters. The UAL_UI provides extremely advanced input handling capabilities, returning a data structure that contains chyron zone, surface, and window identification for mouse events, as well as character and keypress information for keyboard events.

Often, the hardest part of learning NCurses, and to a greater degree Notcurses, is getting past the particular implementation details of each library. For example, Notcurses nccells are robust, but they don't support Unicode Code Points. The UAL_UI silently and automatically intercepts Unicode Code Points and encodes them as UTF-8 before handing them off to Notcurses. That's something the developer doesn't have to worry about. NCurses uses color pairs extensively, and stores colors as a closed interval \[0,1000\] while Notcurses uses a closed interval \[0,255\]. Accordingly, NCurses and Notcurses colors are not compatible and their data types aren't compatible. Notcurses uses the standard uint8_t type for color values, while NCurses uses int (32-bits). No need to worry, the UAL_UI takes care of all that for you. NCurses has functions to output strings of complex characters to the screen, but Notcurses doesn't have a function to display strings of nccells. No worries, the UAL_UI takes care of that too. We could go on all day about the intricate differences between NCurses and Notcurses, but why bother. The UAL_UI abstracts away the intricate details of each library, allowing you to create professional applications without having to learn the ins and outs of each user interface library. By providing a consistent and uniform API, the UAL_UI simplifies development and enhances code maintainability.

You may be one of the many developers who have used NCurses for years and you
have seen what Notcurses has to offer. You may have even tried to use Notcurses, but found it difficult to learn and use. The UAL_UI is designed to help you transition from NCurses to Notcurses without having to learn the intricacies of Notcurses. The UAL_UI provides a consistent and uniform API that allows you to write code that works with both NCurses and Notcurses, without having to worry about the differences between the two libraries. Just prepend "ui_" to your NCurses function calls, compile and link. Viola! You now have a program that works with both NCurses and Notcurses.

## Functions

Functions are documented in their respective source files:

### ui_common.c

```c

uint ui__mbstr_to_cellstr(UiCell *cmplx_buf, const char *str, const UiCell *cell_base, 
                          uint *p, const uint atmost)

void ui_abend(int ec, char *s)

bool ui_action_disposition(char *title, char *action_str)

void ui_activate_all_chyron_keys(UiChyron *chyron)

void ui_activate_chyron_key(UiChyron *chyron, uint k)

int ui_answer_yn(char *msg0, char *msg1, char *msg2, char *msg3)

void ui_apply_gamma(RGB *rgb)

int ui_assign_chyron_win(UiChyron *chyron, UiSurface *sfc, ss_t w, char *y)

int ui_border_draw(UiSurface *sfc)

int ui_border_title(UiSurface *sfc, const char *title)

int ui_border_ysplit(UiSurface *sfc, uint y)

int ui_border_ysplit_text(UiSurface *sfc, char *text, uint separator_line)

int ui_cm_surface_destroy(UiSurface *sfc)

void ui_compile_chyron(UiChyron *chyron)

void ui_deactivate_all_chyron_keys(UiChyron *chyron)

void ui_deactivate_chyron_key(UiChyron *chyron, uint k)

UiChyron *ui_destroy_chyron(UiChyron *chyron)

void ui_display_chyron(UiSurface *sfc, ss_t w, UiChyron *chyron, uint line, uint col)

int ui_display_error(char *msg0, char *msg1, char *msg2, char *msg3)

void ui_destroy_curses()

int ui_get_chyron_key(UiChyron *chyron, uint x)

bool ui_init_clr_palette(SIO *sio)

void ui_initialize_sio(SIO *sio)

bool ui_is_set_chyron_key(UiChyron *chyron, uint k)

char *ui_iso8601_timestamp(char *buf, size_t n, bool local)

void ui_mbc_to_wc(wchar_t wc[2], const char mbc)

uint ui_mbstr_to_cellstr(UiSurface *sfc, ss_t w, UiCell *cmplx_buf, const char *str, 
                         const UiCell *cell_base, uint *p, const uint atmost)

wchar_t *ui_mbstr_to_wcstr(const char *mb_str)

UiChyron *ui_new_chyron(UiSurface *sfc, ss_t w)

FILE *ui_open_log()

void ui_parse_ansi(char *ansi_str, attr_t *attr, uint *cpx)

int ui_perror(char *emsg_str)

uint ui_rgb_to_xterm256_idx(RGB *rgb)

void ui_set_chyron_key(UiChyron *chyron, uint k, char *s, uint kc)

void ui_set_chyron_key_cb(UiChyron *chyron, uint k, char *s, uint kc, UiCell cell_base)

const char *ui_sub_surface_str(ss_t w)

int ui_tracked_sfc_box(uint wlines, uint wcols, uint wbegy, uint wbegx, const char *
                       wtitle)

int ui_tracked_sfc_split_box(uint wlines, uint wcols, uint split_y, uint split_x, uint 
                             wbegy, uint wbegx, const char *wtitle)

void ui_unset_chyron_key(UiChyron *chyron, uint k)

RGB ui_xterm256_idx_to_rgb(uint idx)
```

------------------------------------------------------

### ui_ncurses.c

```c

void fast_exit(UiSurface *s)

uint ui_add_pair(uint fg, uint bg)

int ui_bkgd(UiSurface *s, ss_t w, const UiCell *cell)

int ui_bkgdset(UiSurface *s, ss_t w, const UiCell *cell)

int ui_bkgrnd(UiSurface *s, ss_t w, const UiCell *cell)

int ui_bkgrndset(UiSurface *s, ss_t w, const UiCell *cell)

UiCell ui_cell_from_ucp(const wchar_t *ucp, const uint32_t *fg, const uint32_t *bg)

int ui_chg_color(uint16_t color_idx, uint32_t *color)

int ui_chg_pair(uint pair, uint fg, uint bg)

int ui_clear()

int ui_color_content(uint color, uint *r, uint *g, uint *b)

int ui_color_from_rgb(RGB *rgb)

int ui_curs_set(int visibility)

int ui_cursor_enable(UiSurface *s, ss_t w, bool visible)

int ui_cursor_enable_yx(UiSurface *s, ss_t w, uint y, uint x, bool visible)

int ui_cursor_move(UiSurface *s, ss_t w, uint y, uint x)

void ui_def_prog_mode()

int ui_doupdate()

void ui_endwin()

int ui_erase()

UiBackend ui_get_backend()

void ui_get_caps(UiCaps *caps)

uint32_t ui_get_color(uint16_t color_idx)

void ui_get_screen_size(uint *lines, uint *cols)

int ui_getcchar(const UiCell *cell, wchar_t *wstr, attr_t *attrs, short *pair, const 
                void *opts)

int ui_getmaxx(UiSurface *s, ss_t w)

int ui_getmaxy(UiSurface *s, ss_t w)

void ui_getmaxyx(UiSurface *s, ss_t w, uint *lines, uint *cols)

void ui_getyx(UiSurface *s, ss_t w, uint *lines, uint *cols)

RGB ui_hex_to_rgb(char *s)

int ui_idcok(UiSurface *s, ss_t w, bool enable)

int ui_idlok(UiSurface *s, ss_t w, bool enable)

struct UiRuntime *ui_init(const UiConfig *cfg, SIO *sio)

int ui_init_color(uint color, uint r, uint g, uint b)

int ui_init_pair(uint pair, uint fg, uint bg)

int ui_keypad(UiSurface *s, ss_t w, bool enable)

SCREEN *ui_ncurses_get_screen()

PANEL *ui_ncurses_surface_get_panel(const UiSurface *s, ss_t w)

WINDOW *ui_ncurses_surface_get_win(const UiSurface *s, ss_t w)

int ui_pair_content(uint pair, uint *fg, uint *bg)

int ui_pair_from_hex(const char *fg, const char *bg)

void ui_render()

int ui_resume()

int ui_scrollok(UiSurface *s, ss_t w, bool enable)

int ui_setcchar(UiCell *cell, const wchar_t *wstr, const attr_t attrs, short pair, 
                const void *opts)

int ui_setscrreg(UiSurface *s, ss_t w, uint top, uint bottom)

void ui_shutdown()

int ui_surface_addpad(UiSurface *s, ss_t w, uint view_win, uint lines, uint cols, uint 
                      begy, uint begx)

int ui_surface_addwin(UiSurface *s, ss_t w, uint p, uint lines, uint cols, uint y, 
                      uint x)

UiSurface *ui_surface_box(UiSurface *parent, uint p, uint lines, uint cols, uint y, 
                          uint x, const char *wtitle)

void ui_surface_destroy(UiSurface *s)

int ui_surface_hide(UiSurface *s, ss_t w)

int ui_surface_move(UiSurface *s, ss_t w, uint y, uint x)

UiSurface *ui_surface_new(ss_t w, UiSurface *parent, uint p, uint lines, uint cols, 
                          uint y, uint x)

int ui_surface_resize(UiSurface *s, ss_t w, uint lines, uint cols)

int ui_surface_show(UiSurface *s, ss_t w)

int ui_suspend()

int ui_top_surface(UiSurface *s, ss_t w)

void ui_update_panels()

int ui_wclear(UiSurface *s, ss_t w)

int ui_werase(UiSurface *s, ss_t w)

int ui_wmove(UiSurface *s, ss_t w, uint y, uint x)

int ui_wnoutrefresh(UiSurface *s, ss_t w)

int ui_wscrl(UiSurface *s, ss_t w, int n)
```

------------------------------------------------------

### ui_ncurses_draw.c

```c
int ui_draw_ch(UiSurface *s, ss_t w, const char c)

int ui_draw_ch_yx(UiSurface *s, ss_t w, uint y, uint x, const char c)

int ui_draw_text(UiSurface *s, ss_t w, uint y, uint x, const char *text)

int ui_draw_text_fill(UiSurface *s, ss_t w, uint y, uint x, const char *text, int n)

int ui_draw_text_n(UiSurface *s, ss_t w, uint y, uint x, const char *text, int n)

int ui_mvwadd_wch(UiSurface *s, ss_t w, uint y, uint x, const UiCell *cell)

int ui_mvwadd_wchnstr(UiSurface *s, ss_t w, uint y, uint x, const UiCell *cmplx_buf, 
                      uint n)

int ui_mvwadd_wchstr(UiSurface *s, ss_t w, uint y, uint x, const UiCell *cmplx_buf)

int ui_mvwaddch(UiSurface *s, ss_t w, uint y, uint x, const char c)

int ui_mvwaddnstr(UiSurface *s, ss_t w, uint y, uint x, const char *text, int m)

int ui_mvwaddnwstr(UiSurface *s, ss_t w, uint y, uint x, const wchar_t *wstr, int m)

int ui_mvwaddstr(UiSurface *s, ss_t w, uint y, uint x, const char *text)

int ui_mvwaddstr_fill(UiSurface *s, ss_t w, uint y, uint x, const char *str, int m)

int ui_mvwaddwstr(UiSurface *s, ss_t w, uint y, uint x, const wchar_t *wstr)

void ui_restore_wins()

int ui_wadd_wch(UiSurface *s, ss_t w, const UiCell *cell)

int ui_wadd_wchnstr(UiSurface *s, ss_t w, const UiCell *cmplx_buf, uint n)

int ui_wadd_wchstr(UiSurface *s, ss_t w, const UiCell *cmplx_buf)

int ui_waddnstr(UiSurface *s, ss_t w, const char *text, int m)

int ui_waddnwstr(UiSurface *s, ss_t w, const wchar_t *wstr, int m)

int ui_waddstr(UiSurface *s, ss_t w, const char *text)

int ui_waddwstr(UiSurface *s, ss_t w, const wchar_t *wstr)

int ui_wclrtobot(UiSurface *s, ss_t w)

int ui_wclrtoeol(UiSurface *s, ss_t w)
```

------------------------------------------------------

### ui_ncurses_input.c

```c

static UiKey translate_key(int ch)

int ui_get_event(UiSurface *s, ss_t w, UiChyron *chyron, UiEvent *ev, int timeout_ms)

int ui_get_event_no_mouse(UiSurface *s, ss_t w, UiEvent *ev)

int ui_mice_enable(int mask)

int ui_mousemask(int mask)
```

------------------------------------------------------

### ui_notcurses.c

```c
void ui_abs_yx(UiSurface *s, ss_t w, int *y, int *x)

uint ui_add_pair(uint fg, uint bg)

int ui_bkgd(UiSurface *s, ss_t w, const UiCell *cell)

int ui_bkgdset(UiSurface *s, ss_t w, const UiCell *cell)

int ui_bkgrnd(UiSurface *s, ss_t w, const UiCell *cell)

int ui_bkgrndset(UiSurface *s, ss_t w, const UiCell *cell)

UiCell ui_cell_from_ucp(const wchar_t *ucp, const uint32_t *fg, const uint32_t *bg)

int ui_clear()

uint ui_color_from_rgb(RGB *rgb)

int ui_curs_set(int visible)

int ui_cursor_enable(UiSurface *s, ss_t w, bool visible)

int ui_cursor_enable_yx(UiSurface *s, ss_t w, uint y, uint x, bool visible)

int ui_cursor_move(UiSurface *s, ss_t w, uint y, uint x)

void ui_cursor_yx(int *y, int *x)

void ui_def_prog_mode()

void ui_endwin()

int ui_erase()

UiBackend ui_get_backend()

void ui_get_caps(UiCaps *caps)

int ui_get_nccell(

void ui_get_screen_size(uint *lines, uint *cols)

int ui_getmaxx(UiSurface *s, ss_t w)

int ui_getmaxy(UiSurface *s, ss_t w)

void ui_getmaxyx(UiSurface *s, ss_t w, uint *y, uint *x)

void ui_getyx(UiSurface *s, ss_t w, uint *y, uint *x)

int ui_idcok(UiSurface *s, ss_t w, bool enable)

int ui_idlok(UiSurface *s, ss_t w, bool enable)

UiRuntime *ui_init(const UiConfig *cfg, SIO *sio)

int ui_keypad(UiSurface *s, ss_t w, bool enable)

void ui_render()

int ui_resume()

int ui_scrollok(UiSurface *s, ss_t w, bool enable)

int ui_set_nccell(UiSurface *sfc, ss_t w, UiCell *cell, const wchar_t *wstr,
    const UiStyle style, short pair)

int ui_setscrreg(UiSurface *s, ss_t w, uint top, uint bottom)

void ui_shutdown()

int ui_surface_addpad(UiSurface *s, ss_t w, uint p, uint lines, uint cols, uint y, 
                      uint x)

int ui_surface_addwin(UiSurface *s, ss_t w, uint p, uint lines, uint cols, uint y, 
                      uint x)

UiSurface *ui_surface_box(UiSurface *parent, uint p, uint lines, uint cols, uint y, 
                          uint x, const char *wtitle)

void ui_surface_destroy(UiSurface *s)

int ui_surface_hide(UiSurface *s, ss_t w)

int ui_surface_move(UiSurface *s, ss_t w, uint y, uint x)

UiSurface *ui_surface_new(ss_t w, UiSurface *parent, uint p, uint lines, uint cols, 
                          uint y, uint x)

int ui_surface_resize(UiSurface *s, ss_t w, uint lines, uint cols)

int ui_surface_show(UiSurface *s, ss_t w)

int ui_suspend()

int ui_top_surface(UiSurface *s, ss_t w)

void ui_update_panels()

int ui_wclear(UiSurface *s, ss_t w)

int ui_werase(UiSurface *s, ss_t w)

int ui_wmove(UiSurface *s, ss_t w, uint y, uint x)

int ui_wscrl(UiSurface *s, ss_t w, int r)
```

------------------------------------------------------

### ui_notcurses_draw.c

```c

int mk_chimera(UiCell *cell, char c)

struct ncvisual *ui_display_image(struct notcurses *nc, UiMultiMedia *mm, const char *
                                  image_file, int y, int x, int begy, int begx)

int ui_draw_ch(UiSurface *s, ss_t w, char c)

int ui_draw_ch_yx(UiSurface *s, ss_t w, uint y, uint x, char c)

int ui_draw_text(UiSurface *s, ss_t w, uint y, uint x, const char *text)

int ui_draw_text_fill(UiSurface *s, ss_t w, uint y, uint x, const char *text, int m)

int ui_draw_text_n(UiSurface *s, ss_t w, uint y, uint x, const char *text, int m)

int ui_mvwadd_wch(UiSurface *s, ss_t w, uint y, uint x, const UiCell *cell)

int ui_mvwadd_wchnstr(UiSurface *s, ss_t w, uint y, uint x, const UiCell *cell, uint m)

int ui_mvwadd_wchstr(UiSurface *s, ss_t w, uint y, uint x, const UiCell *cell)

int ui_mvwaddch(UiSurface *s, ss_t w, uint y, uint x, const char c)

int ui_mvwaddnstr(UiSurface *s, ss_t w, uint y, uint x, const char *text, int m)

int ui_mvwaddnwstr(UiSurface *s, ss_t w, uint y, uint x, const wchar_t *wstr, int m)

int ui_mvwaddstr(UiSurface *s, ss_t w, uint y, uint x, const char *text)

int ui_mvwaddstr_fill(UiSurface *s, ss_t w, uint y, uint x, const char *text, int m)

int ui_mvwaddwstr(UiSurface *s, ss_t w, uint y, uint x, const wchar_t *wstr)

void ui_restore_wins()

int ui_wadd_wch(UiSurface *s, ss_t w, const UiCell *cell)

int ui_wadd_wchnstr(UiSurface *s, ss_t w, const UiCell *cell, uint m)

int ui_wadd_wchstr(UiSurface *s, ss_t w, const UiCell *cell)

int ui_waddch(UiSurface *s, ss_t w, const char c)

int ui_waddnstr(UiSurface *s, ss_t w, const char *text, int m)

int ui_waddnwstr(UiSurface *s, ss_t w, const wchar_t *wstr, int m)

int ui_waddstr(UiSurface *s, ss_t w, const char *text)

int ui_waddwstr(UiSurface *s, ss_t w, const wchar_t *wstr)

int ui_wclrtobot(UiSurface *s, ss_t w)

int ui_wclrtoeol(UiSurface *s, ss_t w)
```

------------------------------------------------------

### ui_notcurses_input.c

```c

static UiKey translate_nckey(uint32_t id, const ncinput *ni)

int ui_get_event(UiSurface *s, ss_t w, UiChyron *chyron, UiEvent *ev, int timeout_ms)

int ui_get_event_no_mouse(UiSurface *target, ss_t w, UiEvent *ev)

uint ui_get_plane_idx(UiSurface *s, struct ncplane *plane)

int ui_getch()

int ui_mice_enable(int mask)

int ui_mousemask(int mask)

NcPlane *ui_ncplane_clicked(UiSurface *s, ss_t w, ncinput *ni)
```

------------------------------------------------------
