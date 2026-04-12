#include "common.h"

int popup_menu(Init *, int, char **, int, int);
int popup_form(Init *, int, char **, int, int);
int popup_pick(Init *, int, char **, int, int);
int popup_view(Init *, int, char **, int, int, int, int);

int popup_menu(Init *init, int argc, char **argv, int begy, int begx) {
    int rc;
    zero_opt_args(init);
    parse_opt_args(init, argc, argv);
    Menu *sav_menu = init->menu;
    init->menu = nullptr;
    init->menu = new_menu(init, init->argc, init->argv, begy, begx);
    rc = menu_engine(init);
    destroy_menu(init);
    init->menu = sav_menu;
    return rc;
}

int popup_pick(Init *init, int argc, char **argv, int begy, int begx) {
    int rc;
    zero_opt_args(init);
    parse_opt_args(init, argc, argv);
    Pick *sav_pick = init->pick;
    init->pick = NULL;
    new_pick(init, init->argc, init->argv, begy, begx);
    rc = init_pick(init, init->argc, init->argv, begy, begx);
    destroy_pick(init);
    init->pick = sav_pick;
    return rc;
}

int popup_form(Init *init, int argc, char **argv, int begy, int begx) {
    int rc;
    zero_opt_args(init);
    parse_opt_args(init, argc, argv);
    Form *sav_form = init->form;
    init->form = NULL;
    new_form(init, init->argc, init->argv, begy, begx);
    rc = init_form(init, init->argc, init->argv, begy, begx);
    destroy_form(init);
    init->form = sav_form;
    return rc;
}

int popup_view(Init *init, int argc, char **argv, int lines, int cols, int begy,
               int begx) {
    int rc = 0;
    zero_opt_args(init);
    parse_opt_args(init, argc, argv);
    View *sav_view = init->view;
    init->view = NULL;
    view = NULL;
    if (!view)
        view = new_view(init);
    else
        view = init->view;
    if (lines > 0)
        view->lines = lines;
    if (cols > 0)
        view->cols = cols;
    if (begy > 0)
        view->begy = begy;
    if (begx > 0)
        view->begx = begx;
    view_calc_win_dimensions(init, view->title);
    view->f_full_screen = false;
    if (!init_view_boxwin(init, view->title)) {
        rc = view_file(init);
        win_del();
    }
    destroy_view(init);
    init->view = sav_view;
    return rc;
}
