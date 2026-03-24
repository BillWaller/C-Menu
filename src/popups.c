#include "common.h"

int popup_menu(Init *init, int begy, int begx) {
    new_menu(init, init->argc, init->argv, begy, begx);
    menu = init->menu;
    parse_menu_description(init);
    menu_engine(init);
    return 0;
}

int popup_pick(Init *init, int argc, char **argv, int begy, int begx) {
    zero_opt_args(init);
    parse_opt_args(init, argc, argv);
    init_pick(init, argc, argv, begy, begx);
    return 0;
}

int popup_form(Init *init, int argc, char **argv, int begy, int begx) {
    zero_opt_args(init);
    parse_opt_args(init, argc, argv);
    init_form(init, argc, argv, begy + 1, begx + 1);
    return 0;
}

int popup_view(Init *init, int argc, char **argv, int lines, int cols, int begy,
               int begx) {
    zero_opt_args(init);
    parse_opt_args(init, argc, argv);
    mview(init, argc, argv, lines, cols, begy, begx);
    return 0;
}

int popup_ckeys(Init *init) {
    display_curses_keys(init);
    return 0;
}
