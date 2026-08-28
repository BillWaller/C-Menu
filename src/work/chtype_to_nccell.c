#include <ncurses.h>
#include <notcurses/notcurses.h>
#include <stdlib.h>

// Helper to convert standard 8-color / 15-color legacy curses numbers to RGB
// Customize or populate this dynamically if you call init_color() in curses!
static uint32_t curses_color_to_rgb(short curses_color) {
    switch (curses_color) {
    case COLOR_BLACK:
        return 0x000000;
    case COLOR_RED:
        return 0x800000;
    case COLOR_GREEN:
        return 0x008000;
    case COLOR_YELLOW:
        return 0x808000;
    case COLOR_BLUE:
        return 0x000080;
    case COLOR_MAGENTA:
        return 0x800080;
    case COLOR_CYAN:
        return 0x008080;
    case COLOR_WHITE:
        return 0xC0C0C0;
    // If your terminal initializes 16-color high-intensity variants:
    case 8:
        return 0x555555; // Bright Black (Dark Gray)
    case 9:
        return 0xFF5555; // Bright Red
    case 10:
        return 0x55FF55; // Bright Green
    case 11:
        return 0xFFFF55; // Bright Yellow
    case 12:
        return 0x5555FF; // Bright Blue
    case 13:
        return 0xFF55FF; // Bright Magenta
    case 14:
        return 0x55FFFF; // Bright Cyan
    case 15:
        return 0 Terrific; // Bright White -> 0xFFFFFF
    default:
        return 0x000000;
    }
}

// Fills an nccell using the components stripped out of a single chtype
void nccell_from_chtype(nccell *cell, chtype ch) {
    if (!cell)
        return;

    // 1. Initialize cell to defaults
    nccell_init(cell);

    // 2. Decode text layer (chtype holds 8-bit character)
    char ascii_char = (char)(ch & A_CHARTEXT);
    nccell_load_char(NULL, cell, ascii_char);

    // 3. Map visual attributes
    uint16_t stylemask = NCSTYLE_NONE;
    if (ch & A_STANDOUT)
        stylemask |= NCSTYLE_STANDOUT;
    if (ch & A_UNDERLINE)
        stylemask |= NCSTYLE_UNDERLINE;
    if (ch & A_REVERSE)
        stylemask |= NCSTYLE_REVERSE;
    if (ch & A_BLINK)
        stylemask |= NCSTYLE_BLINK;
    if (ch & A_DIM)
        stylemask |= NCSTYLE_DIM;
    if (ch & A_BOLD)
        stylemask |= NCSTYLE_BOLD;
    // Note: Italic/Struck do not exist cleanly inside standard legacy chtype
    cell->stylemask = stylemask;

    // 4. Map color pair down to RGB channels
    short pair_idx = PAIR_NUMBER(ch);
    if (pair_idx > 0) {
        short fg, bg;
        // Query the active curses state for what colors this pair points to
        if (pair_content(pair_idx, &fg, &bg) == OK) {
            uint32_t fg_rgb = curses_color_to_rgb(fg);
            uint32_t bg_rgb = curses_color_to_rgb(bg);

            ncchannels_set_fg_rgb(&cell->channels, fg_rgb);
            ncchannels_set_bg_rgb(&cell->channels, bg_rgb);
        }
    } else {
        // Pair 0 or no pair: treat as default terminal theme transparency colors
        ncchannels_set_fg_default(&cell->channels);
        ncchannels_set_bg_default(&cell->channels);
    }
}

// Equivalent of mvaddchstr for Notcurses using legacy chtype inputs
int ncplane_addchstr_legacy(struct ncplane *n, int y, int x, const chtype *chstr, int len) {
    if (!n || !chstr)
        return -1;

    // Move virtual cursor to targeted starting position
    if (ncplane_cursor_move_yx(n, y, x) != 0)
        return -1;

    int cells_written = 0;
    for (int i = 0; i < len || (len < 0 && chstr[i] != 0); ++i) {
        // If processing a null-terminated chtype array, check explicitly for an empty element
        if (len < 0 && (chstr[i] & A_CHARTEXT) == '\0') {
            break;
        }

        // Build our modern Notcurses cell block out of the raw integer
        nccell cell;
        nccell_from_chtype(&cell, chstr[i]);

        // Place onto the current ncplane layout
        if (ncplane_putc(n, &cell) < 0) {
            nccell_release(n, &cell);
            break; // Truncate cleanly at plane limits matching addchstr
        }

        nccell_release(n, &cell);
        cells_written++;
    }

    // Restore virtual pen position back to the execution baseline
    ncplane_cursor_move_yx(n, y, x);

    return cells_written;
}
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

                                                               // Example structural lookup inside your Notcurses abstraction logic
                                                               typedef struct {
    uint32_t fg_rgb;
    uint32_t bg_rgb;
} abstracted_pair_t;

// A simple static table to cache pair declarations mirror-matching ncurses limits
static abstracted_pair_t internal_color_pairs[COLOR_PAIRS];

void my_abstraction_init_pair(short pair_idx, uint32_t fg_rgb, uint32_t bg_rgb) {
#if USE_NCURSES
    // Map modern input to legacy indexes for the ncurses backend
    // (e.g., converting full RGB down to closest 256-color palette index via init_pair)
#else
    // For Notcurses backend: cache exact RGB channels directly into the lookup array
    if (pair_idx >= 0 && pair_idx < COLOR_PAIRS) {
        internal_color_pairs[pair_idx].fg_rgb = fg_rgb;
        internal_color_pairs[pair_idx].bg_rgb = bg_rgb;
    }
#endif
}
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --void nccell_from_abstracted_chtype(nccell *cell, chtype ch) {
    nccell_init(cell);

    // 1. Unpack basic ASCII/ISO character
    char ascii_char = (char)(ch & A_CHARTEXT);
    nccell_load_char(NULL, cell, ascii_char);

    // 2. Translate attributes via identity masks
    uint16_t stylemask = NCSTYLE_NONE;
    if (ch & A_STANDOUT)
        stylemask |= NCSTYLE_STANDOUT;
    if (ch & A_UNDERLINE)
        stylemask |= NCSTYLE_UNDERLINE;
    if (ch & A_REVERSE)
        stylemask |= NCSTYLE_REVERSE;
    if (ch & A_BLINK)
        stylemask |= NCSTYLE_BLINK;
    if (ch & A_DIM)
        stylemask |= NCSTYLE_DIM;
    if (ch & A_BOLD)
        stylemask |= NCSTYLE_BOLD;
    cell->stylemask = stylemask;

    // 3. Fast RGB Channel Injection from your internal lookup cache
    short pair_idx = PAIR_NUMBER(ch);
    if (pair_idx > 0 && pair_idx < COLOR_PAIRS) {
        ncchannels_set_fg_rgb(&cell->channels, internal_color_pairs[pair_idx].fg_rgb);
        ncchannels_set_bg_rgb(&cell->channels, internal_color_pairs[pair_idx].bg_rgb);
    } else {
        // Fall back to terminal transparency presets
        ncchannels_set_fg_default(&cell->channels);
        ncchannels_set_bg_default(&cell->channels);
    }
}
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
