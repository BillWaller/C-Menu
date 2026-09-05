/** @file ui_hello.c
    @brief Small program to test the ncurses UI
    @author Bill Waller
    Copyright (c) 2026
    MIT License
    billxwaller@gmail.com
    @date 2026-09-02
 */
#define _GNU_SOURCE
#define _XOPEN_SOURCE 600
#include <common.h>

struct ncvisual *display_image(struct notcurses *nc, const char *image_file, int y, int x, int begy, int begx);

int main(int argc, char **argv) {
    char image_file[256] = "mountainside_flowers.jpg";
    if (argc == 2) {
        if (strcmp(argv[1], "--help") == 0) {
            printf("Usage: %s [image_file]\n", argv[0]);
            return EXIT_SUCCESS;
        } else {
            strncpy(image_file, argv[1], sizeof(image_file) - 1);
            image_file[sizeof(image_file) - 1] = '\0';
        }
    }
    // ------------------------------------------------
    // 1 - UI Initialization
    Init *init = new_init(argc, argv);     // Create Init structure
    mapp_initialization(init, argc, argv); // Populate Init structure
    UiConfig ui_config = {                 // UI Runtime configuration
                          .border_style = UI_BORDER_ROUNDED,
                          .log_file = "/tmp/mylog.log",
                          .log_level = LOG_LEVEL_COUNT};
    ui_init(&ui_config, init->sio);
    // ------------------------------------------------------------------
    // 2 - UI Create surface
    UiSurface *sfc = ui_surface_box(stdsfc, BOX, 24, 95, 0, 0, "Test UI Application");
    //
    if (sfc == NULL) {
        ui_log(ERROR, "ui_surface_box failed");
        exit(EXIT_FAILURE);
    }
    if (ui_surface_addwin(sfc, WIN, BOX, 22, 93, 1, 1)) {
        ui_log(ERROR, "ui_surface_addwin failed");
        exit(EXIT_FAILURE);
    }
    struct ncvisual *ncv;
    ncv = display_image(ui->nc, image_file, -1, -1, 25, 0);
    UiEvent ev;
    if (!ncv)
        goto end;
    int c = ui_get_event(sfc, WIN, NULL, &ev, -1);

    ncvisual_destroy(ncv);
end:
    ui_render();
    ui_shutdown();
    exit(EXIT_SUCCESS);
}

struct ncvisual *display_image(struct notcurses *nc, const char *image_file, int y, int x, int begy, int begx) {
    // 1. Get the current standard plane size
    struct ncplane *stdn = notcurses_stdplane(nc);
    unsigned term_rows, term_cols;
    ncplane_dim_yx(stdn, &term_rows, &term_cols);
    struct ncvisual *ncv = ncvisual_from_file(image_file);
    if (!ncv) {
        fprintf(stderr, "Error: Could not load image file.\n");
        return nullptr;
    }
    // 3. Query the image cell dimensions
    struct ncvgeom geom;
    if (ncvisual_geom(nc, ncv, NULL, &geom) < 0) {
        ncvisual_destroy(ncv);
        return nullptr;
    }
    // 4. Calculate available bounding box below the UI
    if (begy == -1) {
        begy = 0;
    }
    if (begx == -1) {
        begx = 0;
    }
    if (y == -1) {
        y = (int)term_rows - begy;
    }
    if (x == -1) {
        x = (int)term_cols - begx;
    }
    if (begy + y > (int)term_rows) {
        y = (int)term_rows - begy;
    }
    if (begx + x > (int)term_cols) {
        x = (int)term_cols - begx;
    }
    int max_rows = y - 2;
    int max_cols = x - 2;
    if (max_rows <= 0) {
        ncvisual_destroy(ncv);
        return nullptr;
    }
    // 5. Scale image to fit the bounding box while preserving aspect ratio
    struct ncvisual_options vopts_calc = {
        .scaling = NCSCALE_SCALE_HIRES,
        .blitter = NCBLIT_PIXEL,
    };
    // Create a dummy/temporary plane to define the maximum bounding box for the
    // layout engine
    struct ncplane_options nopts = {
        .y = begy,
        .x = x,
        .rows = max_rows,
        .cols = max_cols,
    };
    struct ncplane *tmp_bound_plane = ncplane_create(stdn, &nopts);
    if (!tmp_bound_plane) {
        ncvisual_destroy(ncv);
        return nullptr;
    }
    vopts_calc.n = tmp_bound_plane;

    // Let Notcurses populate rcelly and rcellx based on the bounding box plane
    if (ncvisual_geom(nc, ncv, &vopts_calc, &geom) < 0) {
        ncplane_destroy(tmp_bound_plane);
        ncvisual_destroy(ncv);
        return nullptr;
    }

    // Extract the exact rendered cell dimensions
    unsigned rows = geom.rcelly;
    unsigned cols = geom.rcellx;

    // Destroy the temporary bounding plane now that we have the exact dimensions
    ncplane_destroy(tmp_bound_plane);

    // 5b. Allocate the perfectly sized UI surfaces

    UiSurface *visual_sfc = ui_surface_box(stdsfc, BOX, rows + 2, cols + 2, begy, 0, image_file);
    ui_surface_addwin(visual_sfc, WIN, BOX, rows, cols, 1, 1);

    // 6. Setup the blit options to create a subplane for you
    struct ncvisual_options vopts = {
        .n = visual_sfc->mplane[WIN],
        .scaling = NCSCALE_SCALE_HIRES,
        .blitter = NCBLIT_PIXEL,
        .flags = NCVISUAL_OPTION_CHILDPLANE,
    };
    ncvisual_blit(nc, ncv, &vopts);
    notcurses_render(nc);
    return ncv;
}
