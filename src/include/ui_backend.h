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
#define UI_SFC_MAX 30
#define SFC_MAX 30

extern int sfc_ptr;
extern int win_ptr;

typedef struct UiRuntime UiRuntime;
typedef struct UiSurface UiSurface;
typedef struct UiSplitSurface UiSplitSurface;
typedef union UiChannels UiChannels;
typedef struct UiMB UiMB;
typedef uint16_t UiStyle;
typedef uint16_t UiPairIdx;
typedef uint UiColorIdx;

#define UI_COLORS 512
#define UI_PAIRS 512

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
    char border_style;
    const char *tty_path; /**< optional TTY device path; NULL = auto-detect */
    FILE *tty_fp;         /**< optional TTY FILE pointer; NULL = auto-detect */
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

/** BOX WIDE UNICODE CODEPOINTS */

extern wchar_t *border_rounded;
extern wchar_t *border_square;
extern wchar_t *border_double;
extern wchar_t *border_heavy;

typedef union border_wide {
    wchar_t str[11];
    struct {
        wchar_t ho; /**< horizontal line */
        wchar_t ve; /**< vertical line */
        wchar_t lt; /**< left tee */
        wchar_t rt; /**< right tee */
        wchar_t tt; /**< top tee */
        wchar_t bt; /**< bottom tee */
        wchar_t cr; /**< cross */
        wchar_t tl; /**< top left */
        wchar_t tr; /**< top right */
        wchar_t bl; /**< bottom left */
        wchar_t br; /**< bottom right */
    };
} BorderWide;

// extern const wchar_t *bw_ho; /**< horizontal line */
// extern const wchar_t *bw_ve; /**< vertical line */
// extern const wchar_t *bw_lt; /**< left tee */
// extern const wchar_t *bw_rt; /**< right tee */
// extern const wchar_t *bw_tt; /**< top tee */
// extern const wchar_t *bw_bt; /**< bottom tee */
// extern const wchar_t *bw_cr; /**< cross */
// extern const wchar_t *bw_tl; /**< top left */
// extern const wchar_t *bw_tr; /**< top right */
// extern const wchar_t *bw_bl; /**< bottom left */
// extern const wchar_t *bw_br; /**< bottom right */

#define bw_ho bw.ho
#define bw_ve bw.ve
#define bw_lt bw.lt
#define bw_rt bw.rt
#define bw_tt bw.tt
#define bw_bt bw.bt
#define bw_cr bw.cr
#define bw_tl bw.tl
#define bw_tr bw.tr
#define bw_bl bw.bl
#define bw_br bw.br

extern BorderWide bw;
extern const wchar_t *bw_rtl;
extern const wchar_t *bw_rtr;
extern const wchar_t *bw_rbl;
extern const wchar_t *bw_rbr;
extern const wchar_t *bw_sp;
extern const wchar_t *bw_ra;
extern const wchar_t *bw_la;
extern const wchar_t *bw_ua;
extern const wchar_t *bw_da;
extern const wchar_t *bw_ran;
extern const wchar_t *bw_lan;
extern const wchar_t *bw_chk;
extern const wchar_t *bw_h09;

#ifdef UAL_UI
typedef cchar_t UiCell;
typedef struct {
    uint r;
    uint g;
    uint b;
} STDRGB;
#else
#define CCHARW_MAX 5
#define ERR -1
typedef struct nccell UiCell;
typedef struct ncplane NcPlane;
typedef struct notcurses NotCurses;
typedef struct notcurses_options NotCursesOptions;
typedef struct ncplane_options NcPlaneOptions;

typedef uint16_t attr_t;
typedef struct UiPair UiPair;
typedef struct UiColor UiColor;
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} STDRGB;
extern UiSurface *stdsfc;
extern uint LINES, COLS;
#define CELL_CHAR_INITIALIZER(c) { \
    .gcluster = (c),               \
    .gcluster_backstop = 0,        \
    .stylemask = 0,                \
    .channels = 0,                 \
}
#define CELL_INITIALIZER(c, s, chan) { \
    .gcluster = (c),                   \
    .gcluster_backstop = 0,            \
    .stylemask = (s),                  \
    .channels = (chan),                \
}
#define DEFAULT_INITIALIZER(c) {        \
    .gcluster = (c),                    \
    .gcluster_backstop = 0,             \
    .stylemask = (bkgd_cell.stylemask), \
    .channels = (bkgd_cell.channels),   \
}
#endif
typedef struct {
    union {
        uint32_t u32;
        wchar_t u16[2];
        uint8_t u8[4];
        char c[4];
    };
    uint8_t backstop;
    uint8_t width;
} GCluster;

static inline int unicode_to_utf8_gcluster(uint32_t cp, GCluster *g) {
    if (cp <= 0x7F) {
        g->c[0] = (char)cp;
        g->c[1] = '\0';
        g->width = 1;
    } else if (cp <= 0x7FF) {
        g->c[0] = (char)(0xC0 | (cp >> 6));
        g->c[1] = (char)(0x80 | (cp & 0x3F));
        g->c[2] = '\0';
        g->width = 2;
    } else if (cp <= 0xFFFF) {
        g->c[0] = (char)(0xE0 | (cp >> 12));
        g->c[1] = (char)(0x80 | ((cp >> 6) & 0x3F));
        g->c[2] = (char)(0x80 | (cp & 0x3F));
        g->c[3] = '\0';
        g->width = 3;
    } else if (cp <= 0x10FFFF) {
        g->c[0] = (char)(0xF0 | (cp >> 18));
        g->c[1] = (char)(0x80 | ((cp >> 12) & 0x3F));
        g->c[2] = (char)(0x80 | ((cp >> 6) & 0x3F));
        g->c[3] = (char)(0x80 | (cp & 0x3F));
        g->backstop = '\0';
        g->width = 4;
    }
    return 0;
}

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
UiRuntime *ui_init(const UiConfig *config);
void ui_shutdown();
void ui_render();
int ui_clear();
int ui_erase();
int ui_suspend();
int ui_resume();
UiSurface *ui_surface_new(uint w, UiSurface *parent, uint p, uint lines, uint cols, uint y, uint x);
UiSurface *ui_box_surface_new(UiSurface *parent, uint p, uint lines, uint cols, uint y, uint x, char *title);
int ui_surface_addwin(UiSurface *s, uint w, uint p, uint lines, uint cols, uint y, uint x);
int ui_surface_addpad(UiSurface *s, uint w, uint view_win, int lines, int cols);
void ui_surface_destroy(UiSurface *s);
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
int ui_cursor_enable(UiSurface *s, uint w, bool visible);
int ui_cursor_enable_yx(UiSurface *s, uint w, uint y, uint x, bool visible);
int ui_curs_set(int visibility);
int ui_wscrl(UiSurface *s, uint w, uint lines);
int ui_wclrtoeol(UiSurface *s, uint w);
int ui_wclrtobot(UiSurface *s, uint w);
void ui_getyx(UiSurface *s, uint w, uint *lines, uint *cols);
void ui_getmaxyx(UiSurface *s, uint w, uint *lines, uint *cols);
int ui_draw_ch(UiSurface *s, uint w, const char c);
int ui_draw_ch_yx(UiSurface *s, uint w, uint y, uint x, const char c);
int ui_draw_text(UiSurface *s, uint w, uint y, uint x, const char *text);
int ui_draw_text_n(UiSurface *s, uint w, uint y, uint x, const char *text, int m);
int ui_draw_text_fill(UiSurface *s, uint w, uint y, uint x, const char *text, int m);

// ----------------
// char
// ----------------
// waddch
// mvwaddch
// wechochar
// ----------------
// string char
// ----------------
// waddstr
// mvwaddstr
// waddnstr
// mvwaddnstr
// ----------------
// chtype
// ----------------
// waddchstr
// mvwaddchstr
// waddchnstr
// mvwaddchnstr
// ----------------
// wchar_t
// ----------------
// waddwstr
// mvwaddwstr
// waddnwstr
// mvwaddnwstr
// ----------------
// cchar_t
// ----------------
// wadd_wchstr
// mvwadd_wchstr
// wadd_wchnstr
// mvwadd_wchnstr

int ui_getch();
int ui_waddch(UiSurface *s, uint w, const char c);
int ui_mvwaddch(UiSurface *s, uint w, uint y, uint x, const char c);

int ui_waddstr(UiSurface *s, uint w, const char *text);
int ui_waddnstr(UiSurface *s, uint w, const char *text, int m);
int ui_mvwaddstr(UiSurface *s, uint w, uint y, uint x, const char *text);
int ui_mvwaddnstr(UiSurface *s, uint w, uint y, uint x, const char *text, int m);
int ui_mvwaddnstr_fill(UiSurface *s, uint w, uint y, uint x, const char *text, int m);

int ui_waddwstr(UiSurface *s, uint w, const wchar_t *wstr);
int ui_mvwaddwstr(UiSurface *s, uint w, uint y, uint x, const wchar_t *wstr);
int ui_waddnwstr(UiSurface *s, uint w, const wchar_t *wstr, int m);
int ui_mvwaddnwstr(UiSurface *s, uint w, uint y, uint x, const wchar_t *wstr, int m);

// int ui_wadd_chstr(UiSurface *s, uint w, uint y, uint x, const chtype *chstr);
// int ui_mvwadd_chstr(UiSurface *s, uint w, uint y, uint x, const chtype
// *chstr);
// int ui_wadd_chnstr(UiSurface *s, uint w, const chtype *cell);
// int ui_mvwadd_chnstr(UiSurface *s, uint w, uint y, uint x, const chtype
// *chstr);

int ui_waddwch(UiSurface *s, uint w, const UiCell *cell);
int ui_mvwaddwch(UiSurface *s, uint w, uint y, uint x, const UiCell *cell);
int ui_waddwchstr(UiSurface *s, uint w, const UiCell *cell);
int ui_mvwaddwchstr(UiSurface *s, uint w, uint y, uint x, const UiCell *cell);
int ui_waddwchnstr(UiSurface *s, uint w, const UiCell *cell, uint m);
int ui_mvwaddwchnstr(UiSurface *s, uint w, uint y, uint x, const UiCell *cell, uint m);

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
void ui_get_screen_size(uint *lines, uint *cols);
int ui_wclear(UiSurface *s, uint w);
int ui_werase(UiSurface *s, uint w);
int ui_clear();
int ui_erase();
int ui_cursor_move(UiSurface *s, uint w, uint y, uint x);
int ui_add_color_rgb(RGB *rgb);
int ui_surface_show(UiSurface *s, uint w);
int ui_surface_hide(UiSurface *s, uint w);
int ui_surface_move(UiSurface *s, uint w, uint y, uint x);
int ui_surface_resize(UiSurface *s, uint w, uint lines, uint cols);
UiCell ui_cell_from_ucp(const wchar_t *ucp, const uint32_t *fg, const uint32_t *bg);
uint mbstr_to_cellstr(UiCell *cmplx_buf, const char *str, const UiCell *cell_base, uint *pos, const uint atmost);
int ui_bkgd(UiSurface *s, uint w, const UiCell *cell);
int ui_bkgdset(UiSurface *s, uint w, const UiCell *cell);
int ui_chg_color(uint16_t color_idx, uint32_t *color);
uint32_t ui_get_color(uint16_t color_idx);
void fast_exit(UiSurface *s);

#ifdef NOTCURSES_UI
typedef struct nccell UiCell;
extern UiCell bkgd_cell;
extern uint LINES, COLS;
int mk_chimera(UiCell *cell, char c);
int ui_bkgrnd(UiSurface *s, uint w, const UiCell *cell);
int ui_bkgrndset(UiSurface *s, uint w, const UiCell *cell);
int ui_getcchar(const UiCell *cell, wchar_t *wstr, UiStyle *style, UiPairIdx *pair, const void *opts);
int ui_setcchar(UiCell *cell, const wchar_t *wstr, const attr_t style, ushort pair, const void *opts);
// How do you convert "NCurses" to "Notcurses"?
// Insert "ot" after "N".
int ui_init_color(uint16_t color, uint8_t r, uint8_t g, uint8_t b);
int ui_color_content(uint16_t color, uint8_t *r, uint8_t *g, uint8_t *b);
int ui_init_pair(uint16_t pair, uint fg, uint bg);
uint64_t ui_get_channels_from_pair(uint16_t pair);
uint ui_init_color_hex(char *s);
int ui_pair_content(uint16_t pair, uint *fg, uint *bg);
int ui_get_pair(uint16_t pair, uint *fg, uint *bg);
uint ui_add_pair(uint fg, uint bg);
uint ui_add_color(RGB *rgb);
uint ui_add_color_hex(char *s);
int ui_wch_to_utf8(const wchar_t fill_ch);
int ui_get_nccell(const UiCell *cell, wchar_t *wstr, UiStyle *style, UiPairIdx *pair);
int ui_set_nccell(UiCell *cell, const wchar_t *wstr, const UiStyle *style, ushort *pair);
uint get_plane_idx(UiSurface *s, NcPlane *n);
NcPlane *ncplane_clicked(UiSurface *s, uint w, ncinput *ni);
uint ui_getmaxx(UiSurface *s, uint w);
uint ui_getmaxy(UiSurface *s, uint w);
#else
int ui_bkgrnd(UiSurface *s, uint w, const UiCell *cell);
int ui_bkgrndset(UiSurface *s, uint w, const UiCell *cell);
int ui_getcchar(const UiCell *uc, wchar_t *wstr, attr_t *attrs, ushort *pair, void *opts);
int ui_setcchar(UiCell *wch, const wchar_t *wc, const attr_t attrs, short pair, const void *opts);
int ui_init_color(uint color, uint r, uint g, uint b);
int ui_color_content(uint color, uint *r, uint *g, uint *b);
int ui_init_pair(uint pair, uint fg, uint bg);
int ui_pair_content(uint pair, uint *fg, uint *bg);
uint ui_add_pair(uint fg, uint bg);
int ui_chg_pair(uint pair, uint fg, uint bg);
int ui_get_pair(uint pair, uint *fg, uint *bg);
int ui_add_color(uint r, uint g, uint b);
SCREEN *ui_ncurses_get_screen();
int ui_getmaxx(UiSurface *s, uint w);
int ui_getmaxy(UiSurface *s, uint w);
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
UiBackend ui_get_backend();

/** @brief Query the capabilities of the active backend.
   @ingroup ui_backend
   @param ui  The UiRuntime returned by ui_init().
   @param caps Output structure filled with capability flags.
*/
void ui_get_caps(UiCaps *caps);
extern STDRGB std_color[];
extern UiRuntime *ui;
extern UiSurface *ui_surface[UI_SFC_MAX];
extern uint ui_color_cnt;
extern uint ui_pair_cnt;

#endif
