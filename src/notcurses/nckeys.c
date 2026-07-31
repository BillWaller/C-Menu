/**
 * @file nc_ckeys.c
 * @brief Notcurses input handling example
 *
 * This example demonstrates how to handle keyboard and mouse input events using
 * the Notcurses library.
 * It creates two planes and allows the user to interact with them using mouse
 * clicks and keyboard inputs.
 * @author Bill Waller - Copyright 2026
 * @ License: MIT
 */

#define _XOPEN_SOURCE 700

#include <inttypes.h>
#include <locale.h>
#include <notcurses/notcurses.h>
#include <string.h>
#include <unistd.h>

#define MAXLEN 256
#define MAXPLANE 30
#define MAXSFC 30
#define U_VE L'\x2502' /**< vertical line */

typedef struct notcurses NotCurses;
typedef struct ncplane NcPlane;
typedef struct notcurses_options NotCursesOptions;
typedef struct ncplane_options NcPlaneOptions;

typedef struct {
    unsigned int r, g, b;
} RGB;

struct NcSurface {
    NcPlane *box; // box rows+2 and cols+2
    NcPlane *win;
};

typedef struct NcSurface NcSurface;

NcSurface nc_surface[MAXSFC];

int sfc_ptr = -1;

RGB hex_clr_str_to_rgb(char *s);
char *notcurses_key_str(unsigned int, char *);
void ncplane_printf_yx_clrtoeol(NcPlane *n, int y, int x, const char *fmt, ...);
void ncplane_move_yx_clrtoeol(NcPlane *n, int y, int x);
int compat_mvwprintw(struct ncplane *, int, int, const char *, ...);
NcPlane *ncplane_clicked(NcPlane *pile_member, ncinput *ni);
NcPlane *plane_new(NotCurses *nc, int rows, int cols, int y, int x,
                   const char *name, const char *title, const char *fg, const char *bg);
NcSurface *surface_new(NotCurses *, int, int, int, int,
                       const char *, const char *, const char *, const char *);

int nc_surface_cnt = 0;

NotCurses *ui_notcurses_init() {
    setlocale(LC_ALL, "");
    NotCursesOptions nc_opts = {
        .flags = NCOPTION_SUPPRESS_BANNERS | NCOPTION_NO_QUIT_SIGHANDLERS};
    NotCurses *nc = notcurses_init(&nc_opts, NULL);
    if (!nc) {
        return NULL;
    }
    NcPlane *stdn = notcurses_stdplane(nc);
    ncplane_erase(stdn);
    return nc;
}

int handle_input(NotCurses *nc, int y, int x, ncinput *ni);

int main(void) {
    unsigned int y, x;
    NotCurses *nc = ui_notcurses_init();

    NcSurface *surface1 = surface_new(nc, 4, 40, 4, 10, "surface1", "┤ Surface 1 ├", "#00d7ff", "#1c1c1c");
    if (!surface1) {
        notcurses_stop(nc);
        return 1;
    }
    ncplane_dim_yx(nc_surface[0].win, &y, &x);
    ncplane_printf_yx_clrtoeol(nc_surface[0].win, 0, 0, "%s (y=%d, x=%d)", ncplane_name(nc_surface[0].win), y, x);
    notcurses_render(nc);

    NcSurface *surface2 = surface_new(nc, 4, 40, 12, 10, "surface2", "┤ Surface 2 ├", "#c0c0c0", "#000714");
    if (!surface2) {
        notcurses_stop(nc);
        return 1;
    }
    ncplane_dim_yx(nc_surface[1].win, &y, &x);
    ncplane_printf_yx_clrtoeol(nc_surface[1].win, 0, 0, "%s (y=%d, x=%d)", ncplane_name(nc_surface[1].win), y, x);
    notcurses_render(nc);

    ncinput ni;
    handle_input(nc, 30, 0, &ni);
    notcurses_mice_disable(nc);

    notcurses_stop(nc);
    return 0;
}
/**
 * @brief Create a new NcSurface with a box and a window plane.
 * @param nc Pointer to the NotCurses context.
 * @param rows Number of rows for the window plane.
 * @param cols Number of columns for the window plane.
 * @param y Y-coordinate for the top-left corner of the box plane.
 * @param x X-coordinate for the top-left corner of the box plane.
 * @param name Name of the surface (used for naming planes).
 * @param title Title to display on the box plane.
 * @param fg Foreground color in hex format (e.g., "#RRGGBB").
 * @param bg Background color in hex format (e.g., "#RRGGBB").
 * @return Pointer to the newly created NcSurface, or NULL on failure.
 * @details This function creates a new NcSurface consisting of a box plane and
 * a window plane. The box plane is created with rounded corners and a title,
 * while the window plane is created inside the box.
 * The reason we create a box plane and a window plane is to provide a visual
 * container (the box) for the content (the window). This allows us to use 0
 * based coordinates and prevents the content from overwriting the box borders.
 * The box plane is created with dimensions of (rows + 2) x (cols + 2) to
 * accommodate the borders, while the window plane is created with the specified
 * rows and cols.
 */
NcSurface *surface_new(NotCurses *nc, int rows, int cols,
                       int y, int x, const char *name, const char *title,
                       const char *fg, const char *bg) {
    char plane_name[MAXLEN];
    snprintf(plane_name, MAXLEN - 1, "%s_box", name);
    RGB rgb_fg = hex_clr_str_to_rgb((char *)fg);
    RGB rgb_bg = hex_clr_str_to_rgb((char *)bg);
    unsigned int dimy, dimx;
    notcurses_stddim_yx(nc, &dimy, &dimx);
    NcPlaneOptions plane_opts = {
        .y = y,
        .x = x,
        .rows = rows + 2,
        .cols = cols + 2,
        .name = plane_name};
    NcPlane *stdn = notcurses_stdplane(nc);
    NcPlane *box = ncplane_create(stdn, &plane_opts);
    if (!box) {
        notcurses_stop(nc);
        return NULL;
    }
    uint64_t channels = 0;
    ncchannels_set_fg_rgb8(&channels, rgb_fg.r, rgb_fg.g, rgb_fg.b);
    ncchannels_set_bg_rgb8(&channels, rgb_bg.r, rgb_bg.g, rgb_bg.b);
    ncplane_set_base(box, " ", 0, channels);
    ncplane_set_channels(box, channels);
    ncplane_perimeter_rounded(box, 0, channels, 0);
    if (!title || strlen(title) == 0) {
        title = "┤ Surface ├";
    }
    int title_len = (int)strlen(title);
    int title_x = (cols - title_len) / 2;
    ncplane_putstr_yx(box, 0, title_x, title);
    plane_opts.y = 1;
    plane_opts.x = 1;
    plane_opts.rows = rows;
    plane_opts.cols = cols;
    snprintf(plane_name, MAXLEN - 1, "%s_win", name);
    plane_opts.name = plane_name;
    NcPlane *win = ncplane_create(box, &plane_opts);
    if (!win) {
        notcurses_stop(nc);
        return NULL;
    }
    ncplane_set_base(win, " ", 0, channels);
    ncplane_set_channels(win, channels);
    sfc_ptr++;
    nc_surface[sfc_ptr].box = box;
    nc_surface[sfc_ptr].win = win;
    return &nc_surface[sfc_ptr];
}
NcPlane *plane_new(NotCurses *nc, int rows, int cols,
                   int y, int x, const char *name, const char *title,
                   const char *fg, const char *bg) {
    RGB rgb_fg = hex_clr_str_to_rgb((char *)fg);
    RGB rgb_bg = hex_clr_str_to_rgb((char *)bg);
    unsigned int dimy, dimx;
    notcurses_stddim_yx(nc, &dimy, &dimx);
    NcPlaneOptions plane_opts = {
        .y = y,
        .x = x,
        .rows = rows + 2,
        .cols = cols + 2,
        .name = name};
    NcPlane *stdn = notcurses_stdplane(nc);
    NcPlane *plane = ncplane_create(stdn, &plane_opts);
    if (!plane) {
        notcurses_stop(nc);
        return NULL;
    }
    uint64_t channels = 0;
    ncchannels_set_fg_rgb8(&channels, rgb_fg.r, rgb_fg.g, rgb_fg.b);
    ncchannels_set_bg_rgb8(&channels, rgb_bg.r, rgb_bg.g, rgb_bg.b);
    ncplane_set_base(plane, " ", 0, channels);
    ncplane_set_channels(plane, channels);
    ncplane_perimeter_rounded(plane, 0, channels, 0);
    int title_len = (int)strlen(title);
    int title_x = (cols - title_len) / 2;
    ncplane_putstr_yx(plane, 0, title_x, title);
    plane_opts.y = 1;
    plane_opts.x = 1;
    plane_opts.rows = rows;
    plane_opts.cols = cols;
    if (!plane) {
        notcurses_stop(nc);
        return NULL;
    }
    return plane;
}
int handle_input(NotCurses *nc, int y, int x, ncinput *ni) {
    uint32_t id;
    bool running = true;
    char kstr[MAXLEN];
    NcPlane *stdn = notcurses_stdplane(nc);
    notcurses_mice_enable(nc, NCMICE_ALL_EVENTS);
    y = 0;
    x = 10;
    ncplane_printf_yx_clrtoeol(stdn, y + 1, x, "%s",
                               "Input Diagnostics - Notcurses");
    y = 18;
    x = 10;
    ncplane_printf_yx_clrtoeol(stdn, y, x, "%s", "Press a key or exercise the mouse");
    ncplane_printf_yx_clrtoeol(stdn, y + 1, x, "%s", "q to exit");
    notcurses_render(nc);
    while (running) {
        id = notcurses_get_blocking(nc, ni);
        if (id == 'q' || id == 'Q')
            running = false;
        NcPlane *plane = ncplane_clicked(stdn, ni);
        if (plane) {
            ncplane_printf_yx_clrtoeol(stdn, y + 2, x, "Clicked plane: %s", ncplane_name(plane));
        } else {
            ncplane_printf_yx_clrtoeol(stdn, y + 2, x, "Clicked plane: None");
        }
        ncplane_move_yx_clrtoeol(stdn, y + 4, x);
        notcurses_render(nc);
        ncplane_move_yx_clrtoeol(stdn, y + 5, x);
        notcurses_render(nc);
        ncplane_move_yx_clrtoeol(stdn, y + 6, x);
        notcurses_render(nc);
        ncplane_move_yx_clrtoeol(stdn, y + 7, x);
        notcurses_render(nc);
        if (id == NCKEY_BUTTON1)
            ncplane_printf_yx_clrtoeol(stdn, y + 4, x, "Left Click at: X=%d, Y=%d", ni->x, ni->y);
        else if (id == NCKEY_BUTTON2)
            ncplane_printf_yx_clrtoeol(stdn, y + 4, x, "Button Click at: X=%d, Y=%d", ni->x, ni->y);
        else if (id == NCKEY_RESIZE)
            ncplane_printf_yx_clrtoeol(stdn, y + 4, x, "%s", "Terminal resized!");
        else {
            ncplane_printf_yx_clrtoeol(stdn, y + 4, x, "      Octal: %3o", id);
            ncplane_printf_yx_clrtoeol(stdn, y + 5, x, "    Decimal: %3d", id);
            ncplane_printf_yx_clrtoeol(stdn, y + 6, x, "        Hex: %3x", id);
            if (id >= 32 && id <= 126)
                ncplane_printf_yx_clrtoeol(stdn, y + 7, x, "      ASCII: %c", id);
            else
                ncplane_printf_yx_clrtoeol(stdn, y + 7, x, "Description: %s", notcurses_key_str(id, kstr));
        }
        notcurses_render(nc);
    }
    return 0;
}
/**
 * @brief Print formatted text to a plane at specified coordinates and clear to
 * the end of the line.
 * @param n Pointer to the NcPlane where the text will be printed.
 * @param y Y-coordinate for the starting position.
 * @param x X-coordinate for the starting position.
 * @param fmt Format string (like printf).
 * @param ... Additional arguments for the format string.
 * @details This function prints formatted text to the specified plane at the
 * given coordinates. After printing, it clears any remaining characters on that
 * line to ensure a clean output. It uses a temporary buffer to hold the
 * formatted string and calculates how many spaces are needed to clear to the
 * end of the line based on the plane's width.
 */
void ncplane_printf_yx_clrtoeol(NcPlane *n, int y, int x, const char *fmt, ...) {
    char tmp_str[MAXLEN];
    va_list args;
    va_start(args, fmt);
    vsnprintf(tmp_str, MAXLEN - 1, fmt, args);
    va_end(args);
    int l = (int)strlen(tmp_str);
    int dimx = ncplane_dim_x(n);
    int dcols = dimx - x - l;
    memset(tmp_str + l, ' ', dcols);
    tmp_str[dimx] = '\0';
    ncplane_putstr_yx(n, y, x, tmp_str);
}
void ncplane_move_yx_clrtoeol(NcPlane *n, int y, int x) {
    char tmp_str[MAXLEN];
    int dimx = ncplane_dim_x(n);
    int dcols = dimx - x;
    memset(tmp_str, ' ', dcols);
    tmp_str[dcols] = '\0';
    ncplane_putstr_yx(n, y, x, tmp_str);
}
int compat_mvwprintw(struct ncplane *nc, int y, int x, const char *fmt, ...) {
    va_list va;
    va_start(va, fmt);
    if (ncplane_vprintf_yx(nc, y, x, fmt, va) < 0) {
        va_end(va);
        return -1;
    }
    va_end(va);
    return 0;
}
/**
 * @brief Determine which plane was clicked based on the input coordinates.
 * @param pile_member Pointer to the top plane in the pile.
 * @param ni Pointer to the ncinput structure containing the click coordinates.
 * @return Pointer to the clicked NcPlane, or NULL if no plane was clicked.
 * @details This function checks each plane in the pile, starting from the top,
 * to see if the click coordinates fall within its bounds. If a plane is found
 * that contains the click coordinates, it returns a pointer to that plane. If
 * no planes contain the click coordinates, it returns NULL.
 */
NcPlane *ncplane_clicked(NcPlane *pile_member, ncinput *ni) {
    NcPlane *cur = ncpile_top(pile_member);
    while (cur != NULL) {
        int y = ni->y, x = ni->x;
        if (ncplane_translate_abs(cur, &y, &x)) {
            ni->y = y;
            ni->x = x;
            return cur;
        }
        cur = ncplane_below(cur);
    }
    return NULL;
}
/** @brief Convert a standard HTML-style hex color string to an RGB structure.
 * @param s Pointer to the hex color string (e.g., "#RRGGBB").
 * @return RGB structure containing the red, green, and blue components.
 * @details This function parses a hex color string and extracts the red, green,
 * and blue components. It uses sscanf to read the values and stores them in an
 * RGB structure. The input string should be in the format "#RRGGBB", where RR,
 * GG, and BB are two-digit hexadecimal numbers representing the color
 * components.
 */
RGB hex_clr_str_to_rgb(char *s) {
    RGB rgb;
    sscanf(s, "#%02x%02x%02x", &rgb.r, &rgb.g, &rgb.b);
    return rgb;
}
char *notcurses_key_str(unsigned int key_id, char *kstr) {
    switch (key_id) {
    case NCKEY_INVALID:
        strcpy(kstr, "NCKEY_INVALID");
        break;
    case NCKEY_RESIZE:
        strcpy(kstr, "NCKEY_RESIZE");
        break;
    case NCKEY_UP:
        strcpy(kstr, "NCKEY_UP");
        break;
    case NCKEY_RIGHT:
        strcpy(kstr, "NCKEY_RIGHT");
        break;
    case NCKEY_DOWN:
        strcpy(kstr, "NCKEY_DOWN");
        break;
    case NCKEY_LEFT:
        strcpy(kstr, "NCKEY_LEFT");
        break;
    case NCKEY_INS:
        strcpy(kstr, "NCKEY_INS");
        break;
    case NCKEY_DEL:
        strcpy(kstr, "NCKEY_DEL");
        break;
    case NCKEY_BACKSPACE:
        strcpy(kstr, "NCKEY_BACKSPACE");
        break;
    case NCKEY_PGDOWN:
        strcpy(kstr, "NCKEY_PGDOWN");
        break;
    case NCKEY_PGUP:
        strcpy(kstr, "NCKEY_PGUP");
        break;
    case NCKEY_HOME:
        strcpy(kstr, "NCKEY_HOME");
        break;
    case NCKEY_END:
        strcpy(kstr, "NCKEY_END");
        break;
    case NCKEY_F00:
        strcpy(kstr, "NCKEY_F00");
        break;
    case NCKEY_F01:
        strcpy(kstr, "NCKEY_F01");
        break;
    case NCKEY_F02:
        strcpy(kstr, "NCKEY_F02");
        break;
    case NCKEY_F03:
        strcpy(kstr, "NCKEY_F03");
        break;
    case NCKEY_F04:
        strcpy(kstr, "NCKEY_F04");
        break;
    case NCKEY_F05:
        strcpy(kstr, "NCKEY_F05");
        break;
    case NCKEY_F06:
        strcpy(kstr, "NCKEY_F06");
        break;
    case NCKEY_F07:
        strcpy(kstr, "NCKEY_F07");
        break;
    case NCKEY_F08:
        strcpy(kstr, "NCKEY_F08");
        break;
    case NCKEY_F09:
        strcpy(kstr, "NCKEY_F09");
        break;
    case NCKEY_F10:
        strcpy(kstr, "NCKEY_F10");
        break;
    case NCKEY_F11:
        strcpy(kstr, "NCKEY_F11");
        break;
    case NCKEY_F12:
        strcpy(kstr, "NCKEY_F12");
        break;
    case NCKEY_F13:
        strcpy(kstr, "NCKEY_F13");
        break;
    case NCKEY_F14:
        strcpy(kstr, "NCKEY_F14");
        break;
    case NCKEY_F15:
        strcpy(kstr, "NCKEY_F15");
        break;
    case NCKEY_F16:
        strcpy(kstr, "NCKEY_F16");
        break;
    case NCKEY_F17:
        strcpy(kstr, "NCKEY_F17");
        break;
    case NCKEY_F18:
        strcpy(kstr, "NCKEY_F18");
        break;
    case NCKEY_F19:
        strcpy(kstr, "NCKEY_F19");
        break;
    case NCKEY_F20:
        strcpy(kstr, "NCKEY_F20");
        break;
    case NCKEY_F21:
        strcpy(kstr, "NCKEY_F21");
        break;
    case NCKEY_F22:
        strcpy(kstr, "NCKEY_F22");
        break;
    case NCKEY_F23:
        strcpy(kstr, "NCKEY_F23");
        break;
    case NCKEY_F24:
        strcpy(kstr, "NCKEY_F24");
        break;
    case NCKEY_F25:
        strcpy(kstr, "NCKEY_F25");
        break;
    case NCKEY_F26:
        strcpy(kstr, "NCKEY_F26");
        break;
    case NCKEY_F27:
        strcpy(kstr, "NCKEY_F27");
        break;
    case NCKEY_F28:
        strcpy(kstr, "NCKEY_F28");
        break;
    case NCKEY_F29:
        strcpy(kstr, "NCKEY_F29");
        break;
    case NCKEY_F30:
        strcpy(kstr, "NCKEY_F30");
        break;
    case NCKEY_F31:
        strcpy(kstr, "NCKEY_F31");
        break;
    case NCKEY_F32:
        strcpy(kstr, "NCKEY_F32");
        break;
    case NCKEY_F33:
        strcpy(kstr, "NCKEY_F33");
        break;
    case NCKEY_F34:
        strcpy(kstr, "NCKEY_F34");
        break;
    case NCKEY_F35:
        strcpy(kstr, "NCKEY_F35");
        break;
    case NCKEY_F36:
        strcpy(kstr, "NCKEY_F36");
        break;
    case NCKEY_F37:
        strcpy(kstr, "NCKEY_F37");
        break;
    case NCKEY_F38:
        strcpy(kstr, "NCKEY_F38");
        break;
    case NCKEY_F39:
        strcpy(kstr, "NCKEY_F39");
        break;
    case NCKEY_F40:
        strcpy(kstr, "NCKEY_F40");
        break;
    case NCKEY_F41:
        strcpy(kstr, "NCKEY_F41");
        break;
    case NCKEY_F42:
        strcpy(kstr, "NCKEY_F42");
        break;
    case NCKEY_F43:
        strcpy(kstr, "NCKEY_F43");
        break;
    case NCKEY_F44:
        strcpy(kstr, "NCKEY_F44");
        break;
    case NCKEY_F45:
        strcpy(kstr, "NCKEY_F45");
        break;
    case NCKEY_F46:
        strcpy(kstr, "NCKEY_F46");
        break;
    case NCKEY_F47:
        strcpy(kstr, "NCKEY_F47");
        break;
    case NCKEY_F48:
        strcpy(kstr, "NCKEY_F48");
        break;
    case NCKEY_F49:
        strcpy(kstr, "NCKEY_F49");
        break;
    case NCKEY_F50:
        strcpy(kstr, "NCKEY_F50");
        break;
    case NCKEY_F51:
        strcpy(kstr, "NCKEY_F51");
        break;
    case NCKEY_F52:
        strcpy(kstr, "NCKEY_F52");
        break;
    case NCKEY_F53:
        strcpy(kstr, "NCKEY_F53");
        break;
    case NCKEY_F54:
        strcpy(kstr, "NCKEY_F54");
        break;
    case NCKEY_F55:
        strcpy(kstr, "NCKEY_F55");
        break;
    case NCKEY_F56:
        strcpy(kstr, "NCKEY_F56");
        break;
    case NCKEY_F57:
        strcpy(kstr, "NCKEY_F57");
        break;
    case NCKEY_F58:
        strcpy(kstr, "NCKEY_F58");
        break;
    case NCKEY_F59:
        strcpy(kstr, "NCKEY_F59");
        break;
    case NCKEY_F60:
        strcpy(kstr, "NCKEY_F60");
        break;
    case NCKEY_ENTER:
        strcpy(kstr, "NCKEY_ENTER");
        break;
    case NCKEY_CLS:
        strcpy(kstr, "NCKEY_CLS");
        break;
    case NCKEY_DLEFT:
        strcpy(kstr, "NCKEY_DLEFT");
        break;
    case NCKEY_DRIGHT:
        strcpy(kstr, "NCKEY_DRIGHT");
        break;
    case NCKEY_ULEFT:
        strcpy(kstr, "NCKEY_ULEFT");
        break;
    case NCKEY_URIGHT:
        strcpy(kstr, "NCKEY_URIGHT");
        break;
    case NCKEY_CENTER:
        strcpy(kstr, "NCKEY_CENTER");
        break;
    case NCKEY_BEGIN:
        strcpy(kstr, "NCKEY_BEGIN");
        break;
    case NCKEY_CANCEL:
        strcpy(kstr, "NCKEY_CANCEL");
        break;
    case NCKEY_CLOSE:
        strcpy(kstr, "NCKEY_CLOSE");
        break;
    case NCKEY_COMMAND:
        strcpy(kstr, "NCKEY_COMMAND");
        break;
    case NCKEY_COPY:
        strcpy(kstr, "NCKEY_COPY");
        break;
    case NCKEY_EXIT:
        strcpy(kstr, "NCKEY_EXIT");
        break;
    case NCKEY_PRINT:
        strcpy(kstr, "NCKEY_PRINT");
        break;
    case NCKEY_REFRESH:
        strcpy(kstr, "NCKEY_REFRESH");
        break;
    case NCKEY_SEPARATOR:
        strcpy(kstr, "NCKEY_SEPARATOR");
        break;
    case NCKEY_CAPS_LOCK:
        strcpy(kstr, "NCKEY_CAPS_LOCK");
        break;
    case NCKEY_SCROLL_LOCK:
        strcpy(kstr, "NCKEY_SCROLL_LOCK");
        break;
    case NCKEY_NUM_LOCK:
        strcpy(kstr, "NCKEY_NUM_LOCK");
        break;
    case NCKEY_PRINT_SCREEN:
        strcpy(kstr, "NCKEY_PRINT_SCREEN");
        break;
    case NCKEY_PAUSE:
        strcpy(kstr, "NCKEY_PAUSE");
        break;
    case NCKEY_MENU:
        strcpy(kstr, "NCKEY_MENU");
        break;
    case NCKEY_MEDIA_PLAY:
        strcpy(kstr, "NCKEY_MEDIA_PLAY");
        break;
    case NCKEY_MEDIA_PAUSE:
        strcpy(kstr, "NCKEY_MEDIA_PAUSE");
        break;
    case NCKEY_MEDIA_PPAUSE:
        strcpy(kstr, "NCKEY_MEDIA_PPAUSE");
        break;
    case NCKEY_MEDIA_REV:
        strcpy(kstr, "NCKEY_MEDIA_REV");
        break;
    case NCKEY_MEDIA_STOP:
        strcpy(kstr, "NCKEY_MEDIA_STOP");
        break;
    case NCKEY_MEDIA_FF:
        strcpy(kstr, "NCKEY_MEDIA_FF");
        break;
    case NCKEY_MEDIA_REWIND:
        strcpy(kstr, "NCKEY_MEDIA_REWIND");
        break;
    case NCKEY_MEDIA_NEXT:
        strcpy(kstr, "NCKEY_MEDIA_NEXT");
        break;
    case NCKEY_MEDIA_PREV:
        strcpy(kstr, "NCKEY_MEDIA_PREV");
        break;
    case NCKEY_MEDIA_RECORD:
        strcpy(kstr, "NCKEY_MEDIA_RECORD");
        break;
    case NCKEY_MEDIA_LVOL:
        strcpy(kstr, "NCKEY_MEDIA_LVOL");
        break;
    case NCKEY_MEDIA_RVOL:
        strcpy(kstr, "NCKEY_MEDIA_RVOL");
        break;
    case NCKEY_MEDIA_MUTE:
        strcpy(kstr, "NCKEY_MEDIA_MUTE");
        break;
    case NCKEY_LSHIFT:
        strcpy(kstr, "NCKEY_LSHIFT");
        break;
    case NCKEY_LCTRL:
        strcpy(kstr, "NCKEY_LCTRL");
        break;
    case NCKEY_LALT:
        strcpy(kstr, "NCKEY_LALT");
        break;
    case NCKEY_LSUPER:
        strcpy(kstr, "NCKEY_LSUPER");
        break;
    case NCKEY_LHYPER:
        strcpy(kstr, "NCKEY_LHYPER");
        break;
    case NCKEY_LMETA:
        strcpy(kstr, "NCKEY_LMETA");
        break;
    case NCKEY_RSHIFT:
        strcpy(kstr, "NCKEY_RSHIFT");
        break;
    case NCKEY_RCTRL:
        strcpy(kstr, "NCKEY_RCTRL");
        break;
    case NCKEY_RALT:
        strcpy(kstr, "NCKEY_RALT");
        break;
    case NCKEY_RSUPER:
        strcpy(kstr, "NCKEY_RSUPER");
        break;
    case NCKEY_RHYPER:
        strcpy(kstr, "NCKEY_RHYPER");
        break;
    case NCKEY_RMETA:
        strcpy(kstr, "NCKEY_RMETA");
        break;
    case NCKEY_L3SHIFT:
        strcpy(kstr, "NCKEY_L3SHIFT");
        break;
    case NCKEY_L5SHIFT:
        strcpy(kstr, "NCKEY_L5SHIFT");
        break;
    case NCKEY_MOTION:
        strcpy(kstr, "NCKEY_MOTION");
        break;
    case NCKEY_BUTTON1:
        strcpy(kstr, "NCKEY_BUTTON1");
        break;
    case NCKEY_BUTTON2:
        strcpy(kstr, "NCKEY_BUTTON2");
        break;
    case NCKEY_BUTTON3:
        strcpy(kstr, "NCKEY_BUTTON3");
        break;
    case NCKEY_BUTTON4:
        strcpy(kstr, "NCKEY_BUTTON4");
        break;
    case NCKEY_BUTTON5:
        strcpy(kstr, "NCKEY_BUTTON5");
        break;
    case NCKEY_BUTTON6:
        strcpy(kstr, "NCKEY_BUTTON6");
        break;
    case NCKEY_BUTTON7:
        strcpy(kstr, "NCKEY_BUTTON7");
        break;
    case NCKEY_BUTTON8:
        strcpy(kstr, "NCKEY_BUTTON8");
        break;
    case NCKEY_BUTTON9:
        strcpy(kstr, "NCKEY_BUTTON9");
        break;
    case NCKEY_BUTTON10:
        strcpy(kstr, "NCKEY_BUTTON10");
        break;
    case NCKEY_BUTTON11:
        strcpy(kstr, "NCKEY_BUTTON11");
        break;
    case NCKEY_SIGNAL:
        strcpy(kstr, "NCKEY_SIGNAL");
        break;
    case NCKEY_EOF:
        strcpy(kstr, "NCKEY_EOF");
        break;
    default:
        snprintf(kstr, 32, "NCKEY_%d", key_id);
        break;
    }
    return kstr;
}
