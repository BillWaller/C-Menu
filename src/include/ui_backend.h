#ifndef UI_BACKEND_H
#define UI_BACKEND_H 1

#ifdef __cplusplus
extern "C" {
#endif

/** @file ui_backend.h
    @ingroup ui_backend
    @brief Backend API for terminal UI library
*/
#define _XOPEN_SOURCE_EXTENDED 1
#define _GNU_SOURCE

#ifdef UAL_UI
#define NCURSES_WIDECHAR 1
#include <ncursesw/ncurses.h>
#include <ncursesw/panel.h>
// #include "../ui/ui_ncurses_internal.h"
#endif
#ifdef NOTCURSES_UI
#include <notcurses/notcurses.h>
// #include "../ui/ui_notcurses_internal.h"
#endif
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
extern int sfc_ptr;
extern int win_ptr;
extern bool f_ncurses_open;
extern bool f_notcurses_open;
// ---------------------------------------------------------------
// Surface enums
// ---------------------------------------------------------------
#define SUB_SURFACE_LIST(X) \
    X(BOX)                  \
    X(WIN)                  \
    X(WIN2)                 \
    X(LNNO)                 \
    X(CMDLN)                \
    X(PAD)                  \
    X(WIN3)
#define AS_ENUM(NAME) NAME,
typedef enum {
    SUB_SURFACE_LIST(AS_ENUM)
        SUB_SFC_MAX
} ss_t;
// ---------------------------------------------------------------
// Logging enums
// ---------------------------------------------------------------
#define LOG_LEVEL_LIST(X) \
    X(FATAL)              \
    X(ERROR)              \
    X(WARN)               \
    X(INFO)               \
    X(VERBOSE)            \
    X(DEBUG)              \
    X(SILENT)
#define AS_ENUM(NAME) NAME,
typedef enum {
    LOG_LEVEL_LIST(AS_ENUM)
        LOG_LEVEL_COUNT
} UiLogLevel;
// ---------------------------------------------------------------
// File Types
// ---------------------------------------------------------------
#define FILE_TYPE_LIST(X) \
    X(FT_TEXT)            \
    X(FT_IMAGE)           \
    X(FT_VIDEO)           \
    X(FT_AUDIO)           \
    X(FT_BINARY_GARBAGE)  \
    X(FT_EMPTY)           \
    X(FT_UNREADABLE)
#define AS_ENUM(NAME) NAME,
typedef enum {
    FILE_TYPE_LIST(AS_ENUM)
        FT_MAX
} FileType;
// ---------------------------------------------------------------
// Input
// ---------------------------------------------------------------
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

typedef struct UiRuntime UiRuntime;
typedef struct SIO SIO;
typedef struct UiSurface UiSurface;
typedef struct UiSplitSurface UiSplitSurface;
typedef union UiChannels UiChannels;
typedef struct UiMB UiMB;
typedef uint16_t UiStyle;
typedef uint16_t UiPairIdx;
typedef uint UiColorIdx;
typedef struct ncinput NcInput;
typedef struct ncvisual NcVisual;

// ---------------------------------------------------------------
// Input
// ---------------------------------------------------------------
#define MOUSE_ACTION(X)   \
    X(UI_MOUSE_NONE)      \
    X(UI_MOUSE_PRESS)     \
    X(UI_MOUSE_RELEASE)   \
    X(UI_MOUSE_DRAG)      \
    X(UI_MOUSE_SCROLL_UP) \
    X(UI_MOUSE_SCROLL_DOWN)
#define AS_ENUM(NAME) NAME,
typedef enum {
    MOUSE_ACTION(AS_ENUM)
        MOUSE_ACTION_MAX
} UiMouseAction;

typedef uint32_t UiKey;
typedef struct {
    UiKey key;
    uint32_t ch; /* Unicode codepoint when key == UIKEY_CHAR */
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
    wchar_t wc;       // Wide character    4-bytes
    short attrs;      // attributes        2-bytes
    short color_pair; // color pair index  2-bytes
} UiCchar64;          //           total   8-bytes

#define BORDER_STYLE(X)  \
    X(UI_BORDER_NONE)    \
    X(UI_BORDER_SINGLE)  \
    X(UI_BORDER_DOUBLE)  \
    X(UI_BORDER_ROUNDED) \
    X(UI_BORDER_HEAVY)
#define AS_ENUM(NAME) NAME,
typedef enum {
    BORDER_STYLE(AS_ENUM)
        BORDER_MAX
} UiBorderStyle;

typedef struct {
    int y;
    int x;
    int lines;
    int cols;
} UiRect;

// ---------------------------------------------------------------
// Configuration
// ---------------------------------------------------------------
#define CCHARW_MAX 5
#define UI_COLORS 512
#define UI_PAIRS 512
#define ALLWINS (uint)1000
#define UI_SFC_MAX 30
#define SFC_MAX 30
#define MAXLEN 256

#ifdef UAL_UI
// ---------------------------------------------------------------
// UAL_UI Specific
// ---------------------------------------------------------------
extern bool f_curses_open;
typedef cchar_t UiCell;
typedef struct {
    uint r, g, b;
} STDRGB;
typedef struct {
    int r, g, b;
} RGB;
struct UiRuntime {
    SCREEN *screen; /**< NCurses SCREEN created by newterm() */
    FILE *tty_fp;   /**< TTY file handle opened by ui_init() */
    bool mouse_enabled;
    bool alt_screen;
    bool cursor_visible;
    UiBorderStyle border_style;
    uint lines;
    uint cols;
    PANEL *panel_main;
    SIO *sio;
};
struct UiSurfaceMeta {
    unsigned int y;
    unsigned int x;
    unsigned int lines;
    unsigned int cols;
    cchar_t bkgd_cell;
    bool hidden;
    char name[MAXLEN];
    char title[MAXLEN];
};
struct UiSurface {
    union {
        struct { // DEPRECATED - will be removed
            PANEL *box_pan;
            PANEL *win_pan;
            PANEL *win2_pan;
            PANEL *lnno_pan;
            PANEL *cmdln_pan;
            PANEL *pad_pan;
            PANEL *plane1_pan;
            PANEL *plane2_pan;
        }; // END DEPRECATION
        struct {
            PANEL *mpan[SUB_SFC_MAX];
        };
    };
    union {
        struct { // DEPRECATED - will be removed
            WINDOW *box;
            WINDOW *win;
            WINDOW *win2;
            WINDOW *lnno;
            WINDOW *cmdln;
            WINDOW *pad;
            WINDOW *plane1;
            WINDOW *plane2;
        }; // END DEPRECATION
        struct {
            WINDOW *mwin[SUB_SFC_MAX];
        };
    };
    struct UiRuntime *runtime;
    struct UiSurfaceMeta meta[SUB_SFC_MAX];
    struct UiSurface *parent;
    int sfc_idx;
    int sub_cnt;
};
union UiChannels {
    struct {
        union {
            struct {
                uint8_t b_b, b_g, b_r, b_a;
            };
            uint32_t bargb;
        };
        union {
            struct {
                uint8_t f_b, f_g, f_r, f_a;
            };
            uint32_t fargb;
        };
    };
    uint64_t fb;
};
struct UiStyle {
    wchar_t wstr[5];
    attr_t attrs;
    short cp;
};
#else
// ---------------------------------------------------------------
// Notcurses Specific
// ---------------------------------------------------------------
typedef struct {
    uint8_t r, g, b;
} STDRGB;
typedef struct {
    union {
        struct {
            uint8_t b, g, r, a;
        };
        uint32_t color;
    };
} RGB;

struct UiRuntime {
    struct notcurses *nc;
    bool mouse_enabled;
    bool alt_screen;
    bool cursor_visible;
    UiBorderStyle border_style;
    unsigned int lines;
    unsigned int cols;
    FILE *tty_fp;
    SIO *sio;
};

struct UiSurfaceMeta {
    unsigned int y;
    unsigned int x;
    unsigned int lines;
    unsigned int cols;
    struct nccell bkgd_cell;
    bool hidden;
    char name[MAXLEN];
    char title[MAXLEN];
};

struct UiSurface {
    union {
        struct { // DEPRECATED - will be removed
            struct ncplane *box;
            struct ncplane *win;
            struct ncplane *win2;
            struct ncplane *lnno;
            struct ncplane *cmdln;
            struct ncplane *pad;
            struct ncplane *plane1;
            struct ncplane *plane2;
        }; // END DEPRECATION
        struct {
            struct ncplane *mplane[SUB_SFC_MAX];
        };
    };
    struct UiSurfaceMeta meta[SUB_SFC_MAX];
    struct UiRuntime *runtime;
    struct UiSurface *parent;
    int sfc_idx;
    int sub_cnt;
};

union UiChannels {
    struct {
        union {
            struct {
                uint8_t b_b, b_g, b_r, b_a;
            };
            uint32_t bargb;
        };
        union {
            struct {
                uint8_t f_b, f_g, f_r, f_a;
            };
            uint32_t fargb;
        };
    };
    uint64_t fb;
};

struct UiCell {
    union {
        uint32_t gcluster;
        uint32_t u32;
        wchar_t u16[2];
        uint8_t u8[4];
        char c[4];
    };
    char gcluster_backstop;
    uint8_t width;
    uint16_t stylemask;
    uint32_t chhannels;
    union {
        struct {
            union {
                struct {
                    uint8_t b_b, b_g, b_r, b_a;
                };
                uint32_t bargb;
            };
            union {
                struct {
                    uint8_t f_b, f_g, f_r, f_a;
                };
                uint32_t fargb;
            };
        };
        uint64_t channels;
    };
};
// Linus doesn't consider typedefs for inclusion into the linux kernel because
// they can obscure the underlying type leading to performance destroying
// pass-by-value mistakes. We use them because they improve code readibility
// and maintainability, but remain aware of that caveat.
typedef uint16_t attr_t;
typedef struct nccell UiCell;
typedef struct ncplane NcPlane;
typedef struct notcurses NotCurses;
typedef struct notcurses_options NotCursesOptions;
typedef struct ncplane_options NcPlaneOptions;
typedef struct UiPair UiPair;
typedef struct UiColor UiColor;
extern NcPlane *stdplane;
extern uint LINES, COLS;
#define ERR -1
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
#endif

extern UiSurface *stdsfc;

typedef struct {
    UiBorderStyle border_style;
    char *log_file;       /**< log_file name or NULL for no logging */
    UiLogLevel log_level; /**< log_level FATAL, ERROR, WARN, INFO, VERBOSE,
    DEBUG, SILENT */
} UiConfig;

typedef enum {
    UI_BACKEND_NCURSES = 0, /**< NCurses / NCursesW backend */
    UI_BACKEND_NOTCURSES    /**< NotCurses backend */
} UiBackend;

typedef struct {
    bool truecolor;  /**< backend can display 24-bit RGB colors */
    bool palette256; /**< backend supports at least 256 palette entries */
    bool mouse;      /**< backend can deliver mouse events */
    bool unicode;    /**< backend handles full Unicode / UTF-8 correctly */
    bool resize;     /**< backend emits UIKEY_RESIZE on terminal resize */
    int color_pairs; /**< max simultaneous color pairs; 0 = unlimited */
} UiCaps;

// ---------------------------------------------------------------
// Colors
// ---------------------------------------------------------------

extern int click_y; /**< the y coordinate of a mouse click */
extern int click_x; /**< the x coordinate of a mouse click */

extern ushort cp_default;            /**< default color pair index */
extern ushort cp_box;                /**< box color pair index */
extern ushort cp_ind;                /**< indicator color pair index */
extern ushort cp_bold;               /**< bold color pair index */
extern ushort cp_title;              /**< title color pair index */
extern ushort cp_highlight;          /**< highlight color pair index */
extern ushort cp_fill_char;          /**< fill character color pair index */
extern ushort cp_brackets;           /**< color pair index for field brackets */
extern ushort cp_nt;                 /**< normal color pair index */
extern ushort cp_nt_rev;             /**< reverse color pair index */
extern ushort cp_nt_hl;              /**< highlight color pair index */
extern ushort cp_nt_hl_rev;          /**< highlight reverse color pair index */
extern ushort cp_ln_fg;              /**< line number color pair index */
extern ushort cp_ln_bg;              /**< line number background color pair index */
extern ushort cp_cmdln_fg;           /**< command line number color pair index */
extern ushort cp_cmdln_bg;           /**< command line number background color pair index */
extern ushort cp_red;                /**< red background color pair index */
extern ushort cp_green;              /**< green background color pair index */
extern ushort cp_yellow;             /**< yellow background color pair index */
extern ushort cp_blue;               /**< blue background color pair index */
extern uint clr_idx;                 /**< current color index */
extern uint clr_cnt;                 /**< number of colors used */
extern uint clr_pair_idx;            /**< current color pair index */
extern uint clr_pair_cnt;            /**< number of color pairs supported by the terminal */
extern char const colors_text[][10]; /**< color codes for the 16 basic colors */

#define COLOR_LEN 8   /**< length of color code strings */
#define FG_COLOR 2    /**< default foreground color */
#define BG_COLOR 0    /**< default background color */
#define BO_COLOR 1    /**< default bold foreground color */
#define LN_COLOR 4    /**< default line number color */
#define LN_BG_COLOR 7 /**< default line number background */

typedef enum {
    CLR_BLACK,
    CLR_RED,
    CLR_GREEN,
    CLR_YELLOW,
    CLR_BLUE,
    CLR_MAGENTA,
    CLR_CYAN,
    CLR_WHITE,
    CLR_BBLACK,
    CLR_BRED,
    CLR_BGREEN,
    CLR_BYELLOW,
    CLR_BBLUE,
    CLR_BMAGENTA,
    CLR_BCYAN,
    CLR_BWHITE,
    CLR_BORANGE,
    CLR_FG,
    CLR_BG,
    CLR_BOX_FG,
    CLR_BOX_BG,
    CLR_IND_FG,
    CLR_IND_BG,
    CLR_BRACKETS_FG,
    CLR_BRACKETS_BG,
    CLR_FILL_CHAR_FG,
    CLR_FILL_CHAR_BG,
    CLR_LN_FG,
    CLR_LN_BG,
    CLR_CMDLN_FG,
    CLR_CMDLN_BG,
    CLR_NT_FG,
    CLR_NT_BG,
    CLR_NT_REV_FG,
    CLR_NT_REV_BG,
    CLR_NT_HL_FG,
    CLR_NT_HL_BG,
    CLR_NT_HL_REV_FG,
    CLR_NT_HL_REV_BG,
    CLR_TITLE_FG,
    CLR_TITLE_BG,
    CLR_NCOLORS
} ColorsEnum;

// ---------------------------------------------------------------
// Borders
// ---------------------------------------------------------------

/** BOX WIDE UNICODE CODEPOINTS */

extern const wchar_t *border_single;
extern const wchar_t *border_double;
extern const wchar_t *border_rounded;
extern const wchar_t *border_heavy;
extern const wchar_t *border_none;

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

// Temporary macros to access the border wide characters more easily

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

extern UiCell cell_default;
extern UiCell cell_fill_char;
extern UiCell cell_brktl;
extern UiCell cell_brktr;
extern UiCell cell_nt;
extern UiCell cell_nt_rev;
extern UiCell cell_nt_hl;
extern UiCell cell_nt_hl_rev;
extern UiCell cell_box;
extern UiCell cell_ind;
extern UiCell cell_cmdln;
extern UiCell cell_title;
extern UiCell cell_ln;
extern UiCell cell_ran;
extern UiCell cell_chk;
extern UiCell cell_ls;
extern UiCell cell_rs;
extern UiCell cell_ts;
extern UiCell cell_bs;
extern UiCell cell_tl;
extern UiCell cell_tr;
extern UiCell cell_ho;
extern UiCell cell_ve;
extern UiCell cell_bl;
extern UiCell cell_br;
extern UiCell cell_lt;
extern UiCell cell_rt;
extern UiCell cell_tt;
extern UiCell cell_bt;
extern UiCell cell_cr;
extern UiCell cell_sp;

#define XTERM_256COLOR 1
#if defined(XTERM_256COLOR)
#define KEY_ALTINS 0x223
#define KEY_ALTHOME 0x21e
#define KEY_ALTPGUP 0x232
#define KEY_ALTDEL 0x20e
#define KEY_ALTEND 0x219
#define KEY_ALTPGDN 0x22d
#define KEY_ALTUP 0x23d
#define KEY_ALTLEFT 0x228
#define KEY_ALTDOWN 0x214
#define KEY_ALTRIGHT 0x237
#elif defined(XTERM_GHOSTTY)
#define KEY_ALTINS 0x228
#define KEY_ALTHOME 0x223
#define KEY_ALTPGUP 0x237
#define KEY_ALTDEL 0x213
#define KEY_ALTEND 0x21e
#define KEY_ALTPGDN 0x232
#define KEY_ALTUP 0x242
#define KEY_ALTLEFT 0x22d
#define KEY_ALTDOWN 0x219
#define KEY_ALTRIGHT 0x23c
#endif

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

struct SIO {
    double red_gamma;      /**< red gamma correction value */
    double green_gamma;    /**< green gamma correction value */
    double blue_gamma;     /**< blue gamma correction value */
    double gray_gamma;     /**< gray gamma correction value */
    uint32_t black;        /**< black */
    uint32_t red;          /**< red */
    uint32_t green;        /**< green */
    uint32_t yellow;       /**< yellow */
    uint32_t blue;         /**< blue */
    uint32_t magenta;      /**< magenta */
    uint32_t cyan;         /**< cyan */
    uint32_t white;        /**< white */
    uint32_t orange;       /**< orange */
    uint32_t bblack;       /**< bold black */
    uint32_t bred;         /**< bold red */
    uint32_t bgreen;       /**< bold green */
    uint32_t byellow;      /**< bold yellow */
    uint32_t bblue;        /**< bold blue */
    uint32_t bmagenta;     /**< bold magenta */
    uint32_t bcyan;        /**< bold cyan */
    uint32_t bwhite;       /**< bold white */
    uint32_t borange;      /**< bold orange */
    uint32_t abg;          /**< background with alpha */
    uint32_t fg;           /**< foreground color */
    uint32_t bg;           /**< background color */
    uint32_t box_fg;       /**< box foreground */
    uint32_t box_bg;       /**< box background */
    uint32_t ind_fg;       /**< indicator foreground */
    uint32_t ind_bg;       /**< indicator background */
    uint32_t brackets_fg;  /**< brackets foreground */
    uint32_t brackets_bg;  /**< brackets background */
    uint32_t fill_char_fg; /**< fill character foreground */
    uint32_t fill_char_bg; /**< fill character background */
    uint32_t ln_fg;        /**< line number color index */
    uint32_t ln_bg;        /**< line number background index */
    uint32_t cmdln_fg;     /**< line number color index */
    uint32_t cmdln_bg;     /**< line number background index */
    uint32_t nt_fg;        /**< color code for normal text foreground */
    uint32_t nt_bg;        /**< color code for normal text background */
    uint32_t nt_rev_fg;    /**< normal text reverse foreground */
    uint32_t nt_rev_bg;    /**< normal text reverse background */
    uint32_t nt_hl_fg;     /**< normal text highlight foreground */
    uint32_t nt_hl_bg;     /**< normal text highlight background */
    uint32_t nt_hl_rev_fg; /**< normal text highlight reverse foreground */
    uint32_t nt_hl_rev_bg; /**< normal text highlight reverse background */
    uint32_t title_fg;     /**< title foreground */
    uint32_t title_bg;     /**< title background */
    uint32_t ran_fg;       /**< right angle foreground */
    uint32_t ran_bg;       /**< right angle background */
    FILE *stdin_fp;        /**< stdin stream pointer */
    FILE *stdout_fp;       /**< stdout stream pointer */
    FILE *stderr_fp;       /**< stderr stream pointer */
    FILE *tty_fp;          /**< terminal device stream pointer */
    int stdin_fd;          /**< stdin file descriptor */
    int stdout_fd;         /**< stdout file descriptor */
    int stderr_fd;         /**< stderr file descriptor */
    int tty_fd;            /**< terminal device file descriptor */
    uint clr_cnt;          /**< number of colors currently in use */
    uint clr_pair_cnt;     /**< number of color pairs currently in use */
    uint clr_idx;          /**< current color index */
    uint clr_pair_idx;     /**< current color pair index */
    char brackets[3];      /**< field brackets for Form */
    char fill_char[2];     /**< fill character for Form fields */
    char border;           /**< Rounded, Single, Double, None */
    char tty_name[MAXLEN]; /**< name of the terminal device */
    ushort cp_default;     /**< default color pair index */
    ushort cp_fill_char;   /**< fill character color pair index */
    ushort cp_brackets;    /**< brackets color pair index */
    ushort cp_nt;          /**< normal text color pair index */
    ushort cp_nt_rev;      /**< reverse color pair index */
    ushort cp_nt_hl;       /**< highlight color pair index */
    ushort cp_nt_hl_rev;   /**< reverse highlight color pair index */
    ushort cp_box;         /**< box color pair index */
    ushort cp_ind;         /**< indicator color pair index */
    ushort cp_cmdln;       /**< command line color pair index */
    ushort cp_title;       /**< title color pair index */
    ushort cp_ln;          /**< line number color pair index */
    ushort cp_ran;         /**< right angle color pair index */
    ushort cp_chk;         /**< checkmark color pair index */
    ushort cp_bold;        /**< bold color pair index */
}; /**< Shared Internal Objects */
/* available to the application and the backend implementation */

typedef struct {
    NcVisual *ncv;
    UiSurface *sfc;
} UiMultiMedia;

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
UiRuntime *ui_init(const UiConfig *config, SIO *sio);
void ui_shutdown();
void ui_render();
int ui_clear();
int ui_erase();
int ui_suspend();
int ui_resume();

int ui_sfc_box_com(uint wlines, uint wcols, uint wbegy, uint wbegx, const char *wtitle);
UiSurface *ui_sfc_box_ll(UiSurface *parent, uint p, uint lines, uint cols, uint y, uint x, const char *title);

int ui_surface_split_box_win_new(uint wlines, uint wcols, uint split_y, uint split_x, uint wbegy, uint wbegx, const char *wtitle);

UiSurface *ui_surface_new(ss_t w, UiSurface *parent, uint p, uint lines, uint cols, uint y, uint x);

int ui_surface_addwin(UiSurface *s, ss_t w, uint p, uint lines, uint cols, uint y, uint x);
int ui_surface_addpad(UiSurface *s, ss_t w, uint view_win, uint lines, uint cols, uint begy, uint begx);
void ui_surface_destroy(UiSurface *s);
int ui_wresize(UiSurface *s, ss_t w, uint lines, uint cols);

int ui_wclear(UiSurface *s, ss_t w);

int ui_werase(UiSurface *s, ss_t w);

int ui_wshow(UiSurface *s, ss_t w);

int ui_whide(UiSurface *s, ss_t w);

int ui_get_event_no_mouse(UiSurface *surface, ss_t w, UiEvent *ev);

int ui_wmove(UiSurface *s, ss_t w, uint y, uint x);

int ui_cursor_enable(UiSurface *s, ss_t w, bool visible);

int ui_cursor_enable_yx(UiSurface *s, ss_t w, uint y, uint x, bool visible);

int ui_curs_set(int visibility);
int ui_wscrl(UiSurface *s, ss_t w, int rows);
int ui_wclrtoeol(UiSurface *s, ss_t w);
int ui_wclrtobot(UiSurface *s, ss_t w);
void ui_getyx(UiSurface *s, ss_t w, uint *lines, uint *cols);
void ui_getmaxyx(UiSurface *s, ss_t w, uint *lines, uint *cols);
int ui_draw_ch(UiSurface *s, ss_t w, const char c);
int ui_draw_ch_yx(UiSurface *s, ss_t w, uint y, uint x, const char c);
int ui_draw_text(UiSurface *s, ss_t w, uint y, uint x, const char *text);
int ui_draw_text_n(UiSurface *s, ss_t w, uint y, uint x, const char *text, int m);
int ui_draw_text_fill(UiSurface *s, ss_t w, uint y, uint x, const char *text, int m);

int ui_getch();
int ui_waddch(UiSurface *s, ss_t w, const char c);
int ui_mvwaddch(UiSurface *s, ss_t w, uint y, uint x, const char c);

int ui_waddstr(UiSurface *s, ss_t w, const char *text);
int ui_mvaddstr(UiSurface *s, ss_t w, uint y, uint x, const char *text);
int ui_mvwaddstr(UiSurface *s, ss_t w, uint y, uint x, const char *text);
int ui_waddnstr(UiSurface *s, ss_t w, const char *text, int m);
int ui_mvwaddstr(UiSurface *s, ss_t w, uint y, uint x, const char *text);
int ui_mvwaddstr_fill(UiSurface *s, ss_t w, uint y, uint x, const char *str, int m);
int ui_mvwaddnstr(UiSurface *s, ss_t w, uint y, uint x, const char *text, int m);

int ui_waddwstr(UiSurface *s, ss_t w, const wchar_t *wstr);
int ui_mvwaddwstr(UiSurface *s, ss_t w, uint y, uint x, const wchar_t *wstr);
int ui_waddnwstr(UiSurface *s, ss_t w, const wchar_t *wstr, int m);
int ui_mvwaddnwstr(UiSurface *s, ss_t w, uint y, uint x, const wchar_t *wstr, int m);

// int ui_wadd_chstr(UiSurface *s, ss_t w, uint y, uint x, const chtype *chstr);
// int ui_mvwadd_chstr(UiSurface *s, ss_t w, uint y, uint x, const chtype
// *chstr);
// int ui_wadd_chnstr(UiSurface *s, ss_t w, const chtype *cell);
// int ui_mvwadd_chnstr(UiSurface *s, ss_t w, uint y, uint x, const chtype
// *chstr);

int ui_wadd_wch(UiSurface *s, ss_t w, const UiCell *cell);
int ui_mvwadd_wch(UiSurface *s, ss_t w, uint y, uint x, const UiCell *cell);
int ui_wadd_wchstr(UiSurface *s, ss_t w, const UiCell *cell);
int ui_mvwadd_wchstr(UiSurface *s, ss_t w, uint y, uint x, const UiCell *cell);
int ui_wadd_wchnstr(UiSurface *s, ss_t w, const UiCell *cell, uint m);
int ui_mvwadd_wchnstr(UiSurface *s, ss_t w, uint y, uint x, const UiCell *cell, uint m);

int ui_setscrreg(UiSurface *s, ss_t w, uint top, uint bottom);
int ui_scrollok(UiSurface *s, ss_t w, bool enable);
int ui_keypad(UiSurface *s, ss_t w, bool enable);
int ui_idlok(UiSurface *s, ss_t w, bool enable);
int ui_idcok(UiSurface *s, ss_t w, bool enable);
void ui_update_panels();
int ui_doupdate();
int ui_wnoutrefresh(UiSurface *s, ss_t w);
int ui_draw_hline(UiSurface *s, ss_t w, uint y, uint x, uint len, const UiStyle *style);
int ui_mousemask(int mask);
int ui_mice_enable(int mask);
void ui_get_screen_size(uint *lines, uint *cols);
int ui_wclear(UiSurface *s, ss_t w);
int ui_werase(UiSurface *s, ss_t w);
int ui_clear();
int ui_erase();
int ui_cursor_move(UiSurface *s, ss_t w, uint y, uint x);

int ui_surface_show(UiSurface *s, ss_t w);
int ui_surface_hide(UiSurface *s, ss_t w);
int ui_surface_move(UiSurface *s, ss_t w, uint y, uint x);
int ui_surface_resize(UiSurface *s, ss_t w, uint lines, uint cols);
UiCell ui_cell_from_ucp(const wchar_t *ucp, const uint32_t *fg, const uint32_t *bg);
uint ui_mbstr_to_cellstr(UiCell *cmplx_buf, const char *str, const UiCell *cell_base, uint *pos, const uint atmost);
int ui_bkgd(UiSurface *s, ss_t w, const UiCell *cell);
int ui_bkgdset(UiSurface *s, ss_t w, const UiCell *cell);
int ui_chg_color(uint16_t color_idx, uint32_t *color);
uint32_t ui_get_color(uint16_t color_idx);
void fast_exit(UiSurface *s);
int ui_bkgrnd(UiSurface *s, ss_t w, const UiCell *cell);
int ui_bkgrndset(UiSurface *s, ss_t w, const UiCell *cell);
int ui_getmaxx(UiSurface *s, ss_t w);
int ui_getmaxy(UiSurface *s, ss_t w);
int ui_pair_from_hex(const char *fg, const char *bg);
// WINDOW *ui_ncurses_surface_get_win(const UiSurface *s, ss_t w);
// PANEL *ui_ncurses_surface_get_panel(const UiSurface *s, ss_t w);
const char *ui_sub_surface_str(ss_t w);
void ui_mbc_to_wc(wchar_t wc[2], const char mbc);
wchar_t *ui_mbstr_to_wcstr(const char *mb_str);

int ui_perror(char *emsg_str);
uint ui_rgb_to_xterm256_idx(RGB *rgb);
RGB ui_xterm256_idx_to_rgb(uint idx);
FileType file_type(const char *filename);

#ifdef NOTCURSES_UI
struct UiColor {
    union {
        struct {
            uint8_t b, g, r;
        };
        uint32_t rgb;
    };
};
struct UiPair {
    uint fg;
    uint bg;
};
typedef struct nccell UiCell;
extern UiCell bkgd_cell;
extern uint LINES, COLS;
void ui_cursor_yx(int *y, int *x);
void ui_abs_yx(UiSurface *s, ss_t w, int *y, int *x);
int mk_chimera(UiCell *cell, char c);
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
uint ui_color_from_rgb(RGB *rgb);
uint ui_add_color_hex(char *s);
int ui_wch_to_utf8(const wchar_t fill_ch);
int ui_get_nccell(const UiCell *cell, wchar_t *wstr, UiStyle *style, UiPairIdx *pair);
int ui_set_nccell(UiCell *cell, const wchar_t *wstr, const UiStyle *style, ushort *pair);
uint ui_get_plane_idx(UiSurface *s, NcPlane *n) __attribute__((nonnull(1, 2)));
NcPlane *ui_ncplane_clicked(UiSurface *s, ss_t w, NcInput *ni);
struct ncvisual *ui_display_image(struct notcurses *nc, UiMultiMedia *mm, const char *image_file, int y, int x, int begy, int begx) __attribute__((nonnull(1, 2, 3)));
#else
int ui_color_from_rgb(RGB *rgb);
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
#endif
void ui_endwin();
RGB ui_hex_to_rgb(char *s);
void ui_restore_wins();
int ui_top_surface(UiSurface *s, ss_t w);

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

// ---------------------------------------------------------------
// Chyron API
// ---------------------------------------------------------------

#define CHYRON_KEY_MAXLEN 64 /**< maximum length of the command text */
#define CHYRON_KEYS 20       /**< maximum number of key bindings for the chyron */

typedef struct {
    bool active;                  /**< whether the key binding is active */
    char text[CHYRON_KEY_MAXLEN]; /**< command text associated with the key code */
    uint keycode;                 /**< key code associated with the command */
    uint end_pos;                 /**< end position of the command text */
    UiCell cell_base;             /**< cell base for colors/attributes */
} UiChyronKey;

typedef struct {
    UiChyronKey *key[CHYRON_KEYS]; /**< array of key bindings for the chyron */
    char s[MAXLEN];                /**< the chyron string, for displaying messages in */
    UiCell cmplx_buf[MAXLEN];      /**< the chyron wide character string */
    // wchar_t wstr[MAXLEN];        /**< the chyron wide character string */
    uint l;                /**< length of the chyron string, for display */
    struct UiSurface *sfc; /** pointer to surface for the chyron */
    uint win;              /** index to window of surface */
    uint y;                /** y coordinante of the chyron in the window */
} UiChyron;

// extern void ui_activate_chyron_key(UiChyron *, uint);
// extern void ui_activate_all_chyron_keys(UiChyron *);
// extern int ui_assign_chyron_win(UiChyron *chyron, struct UiSurface *s, ss_t
// w, char *);
extern void ui_deactivate_chyron_key(UiChyron *, uint);
extern void ui_deactivate_all_chyron_keys(UiChyron *);
extern void ui_compile_chyron(UiChyron *);
// extern void ui_display_chyron(struct UiSurface *, uint n, UiChyron *, uint,
// uint);
extern int ui_get_chyron_key(UiChyron *, uint);
extern bool ui_is_set_chyron_key(UiChyron *, uint);
extern void ui_set_chyron_key(UiChyron *, uint, char *, uint);
extern void ui_set_chyron_key_cb(UiChyron *, uint, char *, uint, UiCell cell_base);
extern void ui_unset_chyron_key(UiChyron *, uint);
extern UiChyron *ui_new_chyron();
extern UiChyron *ui_destroy_chyron(UiChyron *chyron);
extern void ui_abend(int, char *);
int ui_get_event(UiSurface *s, ss_t w, UiChyron *chyron, UiEvent *ev, int timeout_ms);

// ---------------------------------------------------------------
// Chyron API
// ---------------------------------------------------------------

void destroy_curses();
bool ui_init_clr_palette(SIO *sio);
void ui_initialize_sio(SIO *sio);
void ui_abend(int ec, char *s);
bool ui_action_disposition(char *title, char *action_str);
void ui_activate_all_chyron_keys(UiChyron *chyron);
void ui_activate_chyron_key(UiChyron *chyron, uint k);
int ui_answer_yn(char *msg0, char *msg1, char *msg2, char *msg3);
void ui_apply_gamma(RGB *rgb);
int ui_assign_chyron_win(UiChyron *chyron, UiSurface *sfc, ss_t w, char *y);
int ui_border_draw(UiSurface *sfc);
int ui_border_title(UiSurface *sfc, const char *title);
int ui_border_ysplit(UiSurface *sfc, uint y);
int ui_border_ysplit_text(UiSurface *sfc, char *text, uint separator_line);
int ui_cm_surface_destroy(UiSurface *sfc);
void ui_compile_chyron(UiChyron *chyron);
void ui_deactivate_all_chyron_keys(UiChyron *chyron);
void ui_deactivate_chyron_key(UiChyron *chyron, uint k);
UiChyron *ui_destroy_chyron(UiChyron *chyron);
void ui_display_chyron(UiSurface *sfc, ss_t w, UiChyron *chyron, uint line, uint col);
int ui_display_error(char *msg0, char *msg1, char *msg2, char *msg3);
int ui_get_chyron_key(UiChyron *chyron, uint x);
bool ui_is_set_chyron_key(UiChyron *chyron, uint k);
UiChyron *ui_new_chyron();
void ui_set_chyron_key(UiChyron *chyron, uint k, char *s, uint kc);
void ui_set_chyron_key_cb(UiChyron *chyron, uint k, char *s, uint kc, UiCell cell_base);
void ui_unset_chyron_key(UiChyron *chyron, uint k);

// ---------------------------------------------------------------
// Logging
// ---------------------------------------------------------------
#define ANSI_RESET "\033[0m"
char *ui_iso8601_timestamp(char *buf, size_t n, bool local);
extern const char *iso8601_time(void);
extern FILE *ui_log_fp;
extern char ui_log_file_name[];
extern bool ui_timestamp_local;
extern const char *const ui_logcolor[];
extern const char *const ui_log_level_s[];
extern UiLogLevel ui_min_log_level;
extern char ui_timestamp[32];
FILE *ui_open_log();

static inline void ui_logrec(const UiLogLevel level, const char *file, const char *func, const int line, const char *fmt, ...) __attribute__((format(printf, 5, 6)));

static inline void ui_logrec(const UiLogLevel level, const char *file, const char *func, const int line, const char *fmt, ...) {
    UiLogLevel safe_level = (level >= LOG_LEVEL_COUNT) ? INFO : level;
    fprintf(ui_log_fp, "[%s] %s[%s]%s <%s:%s:%d> ",
            ui_iso8601_timestamp(ui_timestamp, sizeof(ui_timestamp), ui_timestamp_local),
            ui_logcolor[safe_level],
            ui_log_level_s[safe_level],
            ANSI_RESET,
            file, func, line);
    va_list args;
    va_start(args, fmt);
    vfprintf(ui_log_fp, fmt, args);
    va_end(args);
    fprintf(ui_log_fp, "\n");
    fflush(ui_log_fp);
}
#define ui_log(level, fmt, ...)                                                 \
    do {                                                                        \
        if (level <= ui_min_log_level) {                                        \
            ui_logrec(level, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__); \
        }                                                                       \
    } while (0)

#ifdef __cplusplus
}
#endif
#endif
