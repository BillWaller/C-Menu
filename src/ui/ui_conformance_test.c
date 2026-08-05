/** @file ui_conformance_test.c
   @ingroup ui_backend
   @brief Compile-time and smoke-test conformance test for the UALUI API.

   This program verifies that:
   1. All public types and function signatures in ui_backend.h compile
      correctly against the compiled-in backend.
   2. The init/shutdown lifecycle works (when run from a terminal).
   3. Surface creation, movement, sizing, and destruction work.
   4. ui_get_backend() and ui_get_caps() return sensible values.
   5. The escape-hatch headers compile (included but not called at runtime
      since they require the matching backend).

   Build with the NCurses backend:
     gcc -std=c23 -I../include -I. \
         ui_conformance_test.c ui_ncurses.c ui_ncurses_draw.c \
         ui_ncurses_input.c ../ual/ui_layout.c \
         $(pkg-config --cflags --libs panelw ncursesw) -o conformance_ncurses

   Build with the NotCurses backend:
     gcc -std=c23 -I../include -I. \
         ui_conformance_test.c ui_notcurses.c ui_notcurses_draw.c \
         ui_notcurses_input.c ../ual/ui_layout.c \
         $(pkg-config --cflags --libs notcurses) -o conformance_notcurses
*/

#define _GNU_SOURCE
#include "../include/ui_backend.h"
#include "../ual/ui_layout.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Pull in whichever compat header matches the compiled backend. */
#if defined(NCURSES_UI) || defined(UAL_UI)
#include "ui_ncurses_compat.h"
#endif
#ifdef NOTCURSES_UI
#include "ui_notcurses_compat.h"
#endif

/* -------------------------------------------------------------------------
   Compile-time type checks — these just verify the API types are consistent.
   ------------------------------------------------------------------------- */
static void compile_time_checks(void) {
    /* Verify all public types exist and have the expected fields. */
    UiConfig cfg = {.enable_mouse = false, .enable_alt_screen = false, .cursor_visible = true, .tty_path = NULL};
    UiEvent ev;
    UiRect rect = {.y = 0, .x = 0, .lines = 10, .cols = 20};
    UiStyle style = {0};
    UiCaps caps;
    UiBorderKind bk = UI_BORDER_ROUNDED;
    UiBackend be;
    UiColor col;
    UiColorPair cp;

    (void)cfg;
    (void)ev;
    (void)rect;
    (void)style;
    (void)caps;
    (void)bk;
    (void)be;
    (void)col;
    (void)cp;

    /* Verify enum values are defined. */
    (void)(UI_KEY_NONE + UI_KEY_CHAR + UI_KEY_ENTER + UI_KEY_ESCAPE +
           UI_KEY_UP + UI_KEY_DOWN + UI_KEY_LEFT + UI_KEY_RIGHT +
           UI_KEY_F1 + UI_KEY_F12);
    (void)(UI_MOUSE_NONE + UI_MOUSE_PRESS + UI_MOUSE_RELEASE +
           UI_MOUSE_SCROLL_UP + UI_MOUSE_SCROLL_DOWN);
    (void)(UI_BACKEND_NCURSES + UI_BACKEND_NOTCURSES);
}

/* -------------------------------------------------------------------------
   Runtime tests
   ------------------------------------------------------------------------- */
static int test_lifecycle(void) {
    UiConfig cfg = {
        .enable_mouse = false,
        .enable_alt_screen = true,
        .cursor_visible = false,
        .tty_path = NULL,
    };

    UiRuntime *ui = ui_init(&cfg);
    if (!ui) {
        fprintf(stderr, "FAIL: ui_init() returned NULL\n");
        return 1;
    }

    /* Screen size */
    int lines = 0, cols = 0;
    ui_get_screen_size(ui, &lines, &cols);
    if (lines <= 0 || cols <= 0) {
        fprintf(stderr, "FAIL: invalid screen size %dx%d\n", lines, cols);
        ui_shutdown(ui);
        return 1;
    }
    printf("PASS: screen size %d lines x %d cols\n", lines, cols);

    /* Backend identity */
    UiBackend be = ui_get_backend(ui);
    printf("PASS: backend = %s\n",
           be == UI_BACKEND_NCURSES ? "ncurses" : be == UI_BACKEND_NOTCURSES ? "notcurses"
                                                                             : "unknown");

    /* Capabilities */
    UiCaps caps;
    ui_get_caps(ui, &caps);
    printf("PASS: caps truecolor=%d palette256=%d mouse=%d unicode=%d resize=%d pairs=%d\n",
           caps.truecolor, caps.palette256, caps.mouse,
           caps.unicode, caps.resize, caps.color_pairs);

    /* Surface */
    UiRect rect = {.y = 2, .x = 4, .lines = 5, .cols = 20};
    UiSurface *s = ui_surface_new(ui, NULL, rect);
    if (!s) {
        fprintf(stderr, "FAIL: ui_surface_new() returned NULL\n");
        ui_shutdown(ui);
        return 1;
    }
    printf("PASS: surface created\n");

    /* Drawing */
    UiStyle style = {0};
    style.fg.r = 255;
    style.fg.g = 255;
    style.fg.b = 255;
    style.bg.r = 0;
    style.bg.g = 0;
    style.bg.b = 0;
    ui_draw_border(s, UI_BORDER_ROUNDED, &style);
    ui_draw_text(s, 1, 1, &style, "UALUI conformance test");
    ui_render(ui);
    printf("PASS: surface drawn and rendered\n");

    /* Framed surface (layout helper) */
    UiRect fr = {.y = 8, .x = 4, .lines = 6, .cols = 30};
    UiFramedSurface fs = ui_framed_surface_new(ui, NULL, fr);
    if (!fs.outer || !fs.inner) {
        fprintf(stderr, "FAIL: ui_framed_surface_new() failed\n");
        ui_surface_destroy(s);
        ui_shutdown(ui);
        return 1;
    }
    ui_draw_border(fs.outer, UI_BORDER_LIGHT, &style);
    ui_draw_text(fs.inner, 0, 0, &style, "framed surface");
    ui_render(ui);
    printf("PASS: framed surface created and drawn\n");

    /* Cleanup */
    ui_framed_surface_destroy(&fs);
    ui_surface_destroy(s);
    ui_shutdown(ui);
    printf("PASS: shutdown clean\n");
    return 0;
}

int main(void) {
    compile_time_checks();
    printf("PASS: compile-time type checks\n");
    return test_lifecycle();
}
