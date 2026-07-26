#include "uthash.h" // Popular header-only C hash table library
#include <notcurses/notcurses.h>
#include <stdlib.h>
#include <string.h>

// 1. Define a generic style struct inside your UAL layer
typedef struct {
    char style_key[64]; // Hash key (e.g., "ui.form.focused")

    // Explicit RGB breakdowns for modern backends (Notcurses, GUI)
    uint8_t fg_r, fg_g, fg_b;
    uint8_t bg_r, bg_g, bg_b;

    // Fallback index for the legacy NCurses backend
    short ncurses_pair_id;

    UT_hash_handle hh; // Makes this structure hashable by uthash
} UAL_Style;

// Global style map pointer
UAL_Style *style_map = NULL;

// Helper to register a semantic theme style
void register_ui_style(const char *key, uint8_t fr, uint8_t fg, uint8_t fb,
                       uint8_t br, uint8_t bg, uint8_t bb, short legacy_id) {
    UAL_Style *s = malloc(sizeof(UAL_Style));
    strncpy(s->style_key, key, sizeof(s->style_key) - 1);
    s->fg_r = fr;
    s->fg_g = fg;
    s->fg_b = fb;
    s->bg_r = br;
    s->bg_g = bg;
    s->bg_b = bb;
    s->ncurses_pair_id = legacy_id;

    HASH_ADD_STR(style_map, style_key, s);
}
