#include "common.h"

int popup_menu(Init *, int, char **, int, int);
int popup_form(Init *, int, char **, int, int);
int popup_pick(Init *, int, char **, int, int);
int popup_view(Init *, int, char **, int, int, int, int);

/** @brief instantiate a menu popup window
    @param init the Init struct pointer
    @param argc the number of command line arguments
    @param argv the command line arguments
    @param begy the y coordinate for the menu window
    @param begx the x coordinate for the menu window
    @details begy and begx may be set as command line option arguments, in which
   case, they will take precedence over arguments passed in the function
   arguments.
    @verbatim
    begy, and begx may be set by
       1. the calling function
          or
       2. command line arguments
    Non-zero command line arguments will override the calling function's
   arguments.
    @endverbatim
*/
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
/** @brief instantiate a pick popup window
    @param init the Init struct pointer
    @param argc the number of command line arguments
    @param argv the command line arguments
    @param begy the y coordinate for the form window
    @param begx the x coordinate for the form window
    @details begy and begx may be set as command line option arguments, in which
   case, they will take precedence over arguments passed in the function
   arguments.
    @verbatim
    begy, and begx may be set by
       1. the calling function
          or
       2. command line arguments
    Non-zero command line arguments will override the calling function's
   arguments.
    @endverbatim
*/
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
/** @brief instantiate a form popup window
    @param init the Init struct pointer
    @param argc the number of command line arguments
    @param argv the command line arguments
    @param begy the y coordinate for the form window
    @param begx the x coordinate for the form window
    @details begy and begx may be set as command line option arguments, in which
   case, they will take precedence over arguments passed in the function
   arguments.
    @verbatim
    begy, and begx may be set by
       1. the calling function
          or
       2. command line arguments
    Non-zero command line arguments will override the calling function's
   arguments.
    @endverbatim
*/
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

/** @brief instantiate a view popup window
    @param init the Init struct pointer
    @param argc the number of command line arguments
    @param argv the command line arguments
    @param lines the number of lines for the view window
    @param cols the number of columns for the view window
    @param begy the y coordinate for the view window
    @param begx the x coordinate for the view window
    @details lines, cols, begy, and begx may also be set as command line option
   arguments, in which case, they will take precedence over arguments passed in
   the function arguments.
    @verbatim
    lines, cols, begy, and begx may be set by
       1. the calling function
          or
       2. command line arguments
    Non-zero command line arguments will override the calling function's
   arguments.
    @endverbatim
*/
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

    if (init->lines > 0 && init->cols > 0) {
        lines = init->lines;
        cols = init->cols;
    }
    if (init->begy > 0 || init->begx > 0) {
        begy = init->begy;
        begx = init->begx;
    }
    view->lines = lines;
    view->cols = cols;
    view->begy = begy;
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
