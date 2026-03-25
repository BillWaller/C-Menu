#include "common.h"

int popup_menu(Init *, int, int);
int popup_form(Init *, int, char **, int, int);
int popup_pick(Init *, int, char **, int, int);
int popup_view(Init *, int, char **);

int popup_menu(Init *init, int begy, int begx) {
    new_menu(init, init->argc, init->argv, begy, begx);
    menu = init->menu;
    parse_menu_description(init);
    menu_engine(init);
    destroy_menu(init);
    return 0;
}

int popup_pick(Init *init, int argc, char **argv, int begy, int begx) {
    zero_opt_args(init);
    parse_opt_args(init, argc, argv);
    Pick *sav_pick = init->pick;
    init_pick(init, argc, argv, begy, begx);
    destroy_pick(init);
    init->pick = sav_pick;
    return 0;
}

int popup_form(Init *init, int argc, char **argv, int begy, int begx) {
    zero_opt_args(init);
    parse_opt_args(init, argc, argv);
    Form *sav_form = init->form;
    init_form(init, argc, argv, begy + 1, begx + 1);
    destroy_form(init);
    init->form = sav_form;
    return 0;
}

int popup_view(Init *init, int argc, char **argv) {
    zero_opt_args(init);
    parse_opt_args(init, argc, argv);
    View *sav_view = init->view;
    view = init->view;
    if (!view)
        view = new_view(init, argc, argv);
    else
        view = init->view;
    if (init->lines != 0 && view->lines == 0)
        view->lines = init->lines;
    if (view->lines == 0)
        view->lines = LINES * 3 / 4;
    if (view->lines > LINES - 3)
        view->lines = LINES - 3;
    if (init->cols != 0 && view->lines == 0)
        view->cols = init->cols;
    if (view->cols == 0)
        view->cols = COLS * 3 / 4;
    if (view->cols > COLS - 3)
        view->cols = COLS - 3;
    if (init->begy != 0 && view->begy == 0)
        view->begy = init->begy;
    if (view->begy == 0)
        view->begy = (LINES - view->lines) / 5;
    if (view->begy + view->lines > LINES - 2)
        view->begy = LINES - view->lines - 2;
    if (init->begx != 0 && view->begx == 0)
        view->begx = init->begx;
    if (view->begx == 0)
        view->begx = (COLS - view->cols) / 5;
    if (view->begx + view->cols > COLS - 1)
        view->begx = COLS - view->cols - 1;
    view->f_full_screen = false;
    if (!init_view_boxwin(init, init->title)) {
        view_file(init);
        win_del();
    }
    destroy_view(init);
    init->view = sav_view;
    return 0;
}

int popup_ckeys() {
    display_curses_keys();
    return 0;
}
