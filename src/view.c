/* view.c
 * file viewer
 * Bill Waller
 * billxwaller@gmail.com
 */
#include "view.h"
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

void command_line_help();

int main(int argc, char **argv) {
    extern int optind;
    extern char *optarg;
    /*
    all    c: d: e: g: h i: n: o: s: t: v x y z w D B: F: O: P: V Z
    init   c: d: e: g: h i: n: o:       v x y z w D B: F: O:    V Z
    view            g: h          s: t: v x y z     B: F: O: P: V Z
        c: command executable                   cmd_str
        d: application directory                mapp_dir
        e: menu application dscription file     mapp_desc
    >   g: lllcccLLLCCC (lines,cols,begy,begx)  geometry
    >   h  display command line help            f_help
    >   i: input file_name                      in_file
        n: number of selections                 selections
        o: output file_name                     out_file
    >   s  squeeze multiple blank lines         f_squeeze
    >   t: number of spaces in tab              tab_stop
    >   v: version                              VIEW_VERSION
    >   w  write configuration
    >   x  ignore case in search                f_ignore_case
    >   y  remove file at end of program        f_at_end_remove
    >   z  clear screen at end                  f_at_end_clear
        D  dump configuration
    >   B: background color                     bg_color
    >   F: foreground color                     fg_color
    >   O: border color                         bo_color
    >   P: {S-Short, L-Long, N-None}[string]    prompt_type
    >   V  version                              VIEW_VERSION
    >   Z  (undocumented) stop on error         f_stop_on_error
    >   +  execute command on startup           startup_cmd
     */

    char *optstring = "g:hst:v x y zB:F:MO:P:VZ+";
    int o;
    int vo;
    char f_help = 0;

    initialization(argc, argv);
    init_view_struct();
    view->lines = 0;
    view->cols = 0;
    view->begy = 0;
    view->begx = 0;
    if (view->argc > 0) {
        optind = 0;
        o = vo = getopt(view->argc, view->argv, optstring);
        if (vo == EOF)
            optind = 1;
    } else
        vo = EOF;
    if (vo == EOF)
        o = getopt(argc, argv, optstring);
    while (o != EOF) {
        switch (o) {
        case 'g':
            parse_geometry_str(optarg, &(view->lines), &(view->cols),
                               &(view->begx), &(view->begy));
            break;
        case 'h':
            f_help = TRUE;
            break;
        case 'r':
            view->f_at_end_remove = TRUE;
            break;
        case 's':
            view->f_squeeze = TRUE;
            break;
        case 't':
            view->tabstop = atoi(optarg);
            if (view->tabstop < 0)
                view->tabstop = 1;
            if (view->tabstop > 12)
                view->tabstop = 12;
            break;
        case 'v':
            f_help = TRUE;
            break;
        case 'x':
            view->f_ignore_case = TRUE;
            break;
        case 'y':
            view->f_at_end_remove = TRUE;
            break;
        case 'z':
            view->f_at_end_clear = TRUE;
            break;
        case 'B':
            option->bg_color = get_color_number(optarg);
            if (option->bg_color == -1)
                f_help++;
            break;
        case 'F':
            option->fg_color = get_color_number(optarg);
            if (option->fg_color == -1)
                f_help++;
            break;
        case 'M':
            view->prompt_type = 'L';
            break;
        case 'O':
            option->bo_color = get_color_number(optarg);
            if (option->bo_color == -1)
                f_help++;
            break;
        case 'P':
            switch (*optarg++) {
            case 's':
            case 'S':
                view->prompt_type = 'S';
                break;
            case 'l':
            case 'L':
                view->prompt_type = 'L';
                break;
            case 'n':
            case 'N':
                view->prompt_type = 'N';
                break;
            }
            if (*optarg != '\0')
                strncpy(view->def_prompt_ptr, optarg, MAX_COLS);
            break;
        case '+':
            if (*optarg == '+') {
                optarg++;
                strnz_cpy(view->startup_cmd_str_all_files, optarg, MAXLEN);
            }
            strnz_cpy(view->startup_cmd_str, optarg, MAXLEN);
            break;
        default:
            f_help++;
            break;
        }
        if (vo != EOF) {
            o = vo = getopt(view->argc, view->argv, optstring);
            if (vo == EOF)
                optind = 1;
        }
        if (vo == EOF)
            o = getopt(argc, argv, optstring);
    }
    build_color_opt_str();
    if (f_help) {
        command_line_help();
        exit(1);
    }
    view->curr_argc = 0;
    view->argc = 0;
    while (1) {
        if (optind >= argc)
            break;
        if (view->argc >= MAXARGS)
            abend(1, "too many arguments");
        view->argv[view->argc++] = strdup(argv[optind++]);
    }
    view->argv[view->argc] = NULL;
    view->fd = -1;
    view->f_stdout_is_tty = isatty(1);
    if (!view->f_stdout_is_tty) {
        if (view->argc < 1) {
            if (initialize_file("-") > 0)
                if (view->fd >= 0)
                    cat_file();
        } else {
            while (view->curr_argc < view->argc) {
                if (initialize_file(view->argv[view->curr_argc]) > 0)
                    if (view->fd >= 0)
                        cat_file();
                view->curr_argc++;
            }
        }
        exit(0);
    }
    f_restore_screen = TRUE;
    if (open_view_stdscr() == 0)
        view_file();
    if (f_curses_open) {
        if (view->f_at_end_clear) {
            wclear(stdscr);
            wrefresh(stdscr);
        }
        if (view->win != stdscr)
            win_del();
        endwin();
    }
    free(view);
    if (f_shell_ioctl) {
        tcsetattr(2, TCSAFLUSH, &shell_ioctl);
        f_shell_ioctl = FALSE;
    }
    fprintf(stderr, "\n");
    exit(0);
}

void command_line_help() {
    fprintf(stderr, "\nview\n\n");
    fprintf(stderr, "  execute command on startup +view-command\n");
    fprintf(stderr, "         clear screen at end -c\n");
    fprintf(stderr, "display command line options -h\n");
    fprintf(stderr, "       ignore case in search -i\n");
    fprintf(stderr,
            "               prompt string -P {L | M | N } prompt-string\n");
    fprintf(stderr,
            "  (L = Long, M = Medium, N = No Prompt, string = user-defined)\n");
    fprintf(stderr, "          remove file at end -r\n");
    fprintf(stderr, "   skip multiple blank lines -s\n");
    fprintf(stderr, "window initialization string -w lllcccLLLCCC\n");
    fprintf(stderr,
            "  (lll = lines, ccc = Columns, LLL = Beginning Line, CCC = "
            "Beginning Column)\n");
    fprintf(stderr, "            foreground color -F ColorName\n");
    fprintf(stderr, "            background color -B ColorName\n");
    fprintf(stderr, "                border color -O ColorName\n");
    list_colors();
    exit(1);
}
