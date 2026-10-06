/** @file init.c
    @brief Initialization for Menu Application Programs
    @author Bill Waller
    Copyright (c) 2025
    MIT License
    billxwaller@gmail.com
    @date 2026-02-09
 */

/**
   @defgroup init C-Menu Initialization
   @brief Capture Data from the Environment, Command Line, and
   Configuration File and Populate the Init and SIO Data Structures
   @verbatim
       SIO   Struct for screen I/O settings (colors, gamma, etc.)
       Init  Struct for application settings (file paths, commands, flags, etc.)
   @endverbatim
 */

#define _GNU_SOURCE
#include <argp.h>
#include <common.h>
#include <locale.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>

typedef enum {
    BG = 257,
    FG,
    BOX_FG,
    BOX_BG,
    IND_FG,
    IND_BG,
    BRACKETS_FG,
    BRACKETS_BG,
    FILL_CHAR_FG,
    FILL_CHAR_BG,
    NT_FG,
    NT_BG,
    NT_REV_FG,
    NT_REV_BG,
    NT_HL_FG,
    NT_HL_BG,
    NT_HL_REV_FG,
    NT_HL_REV_BG,
    TITLE_FG,
    TITLE_BG,
    LN_FG,
    LN_BG,
    CMDLN_FG,
    CMDLN_BG,
    RAN_FG,
    RAN_BG,
    BORDER,
    XBBLACK,
    XBBLUE,
    XBCYAN,
    XBGREEN,
    XBLACK,
    XBLUE,
    XBMAGENTA,
    XBRED,
    XBWHITE,
    XBYELLOW,
    XCYAN,
    CM_EDITOR,
    XGREEN,
    XMAGENTA,
    XRED,
    XWHITE,
    XYELLOW,
    GM_BLUE,
    GM_GRAY,
    GM_GREEN,
    GM_RED,
    MAPP_DATA,
    MAPP_HELP,
    MAPP_HOME,
    MAPP_MSRC,
    MAPP_USER,
    MAPP_SPEC,
    HELP_SPEC,
    MAPP_THEME,
    END_INIT_VARS
} InitVariables;

bool f_write_config = false;
int write_config(Init *init);
void display_version();

// Init *init = nullptr;
void mapp_initialization(Init *, int, char **);
void zero_opt_args(Init *);
int process_config_files(Init *);
int process_config_file(char *, Init *);
int parse_opt_args(Init *, int, char **);
void opt_prt_char(const char *o, const char *name, const char *value);
void opt_prt_str(const char *o, const char *name, const char *value);
void opt_prt_int(const char *o, const char *name, int value);
void opt_prt_double(const char *o, const char *name, double value);
void opt_prt_bool(const char *o, const char *name, bool value);

bool derive_file_spec(char *, char *, char *);
void print_argp_doc(FILE *, char *, char *);
int executor = 0;

const char *argp_program_version = CM_VERSION;
const char *argp_program_bug_address = _("billxwaller@gmail.com");
static char doc[] = _("C-Menu - User Interface Toolkit");
static char args_doc[] = _("[INPUT] [OUTPUT] [HELP] [ARG4] [ARG5]");
// const int opt_doc_col = 33;

static struct argp_option options[] = {
    {_("f_write_config"), 'W', 0, OPTION_ARG_OPTIONAL, _("write configuration"), 0},
    {_("minitrc"), 'a', _("file_spec"), 0, _("configuration file spec"), 1},
    {_("parent_cmd"), 'k', 0, 0, _("parent command"), 1},
    {_("begx"), 'X', _("number"), 0, _("begin on column"), 2},
    {_("begy"), 'Y', _("number"), 0, _("begin on line"), 2},
    {_("cols"), 'C', _("number"), 0, _("width in columns"), 2},
    {_("lines"), 'L', _("number"), 0, _("height in lines"), 2},
    {_("title"), 'T', _("text"), 0, _("Window title"), 2},
    {_("out_spec"), 'o', _("file_spec"), 0, _("output file spec"), 3},
    {_("cmd"), 'c', _("file_spec"), 0, _("view cmd, first file"), 3},
    {_("cmd_all"), 'A', _("file_spec"), 0, _("view cmd, all files"), 3},
    {_("help_spec"), 'H', _("file_spec"), 0, _("help file spec"), 3},
    {_("in_spec"), 'i', _("file_spec"), 0, _("input file spec"), 3},
    {_("log_level"), 'l', _("text"), 0, _("FATAL, ERROR, WARN, INFO, VERBOSE, DEBUG"), 3},
    {_("log_file_spec"), 'g', _("file_spec"), 0, _("log file spec"), 3},
    {_("mapp_spec"), 'd', _("file_spec"), 0, _("description file spec"), 3},
    {_("provider_cmd"), 'S', _("file_spec"), 0, _("execute provider of piped input"), 3},
    {_("receiver_cmd"), 'R', _("file_spec"), 0, _("execute receiver of piped output"), 3},
    {_("select_max"), 'n', _("number"), 0, _("number of selections"), 5},
    {_("f_erase_remainder"), 'e', _("bool"), OPTION_ARG_OPTIONAL, _("erase remainder of line on enter"), 5},
    {_("f_strip_ansi"), 'j', _("bool"), OPTION_ARG_OPTIONAL, _("always strip ansi when writing"), 5},
    {_("f_multiple_cmd_args"), 'M', _("bool"), OPTION_ARG_OPTIONAL, _("allow multiple command arguments"), 5},
    {_("f_read_theme"), 'r', _("bool"), OPTION_ARG_OPTIONAL, _("read and process theme file"), 5},
    {_("f_squeeze"), 's', _("bool"), OPTION_ARG_OPTIONAL, _("squeeze multiple blank lines"), 5},
    {_("f_ignore_case"), 'x', _("bool"), OPTION_ARG_OPTIONAL, _("ignore case in search"), 5},
    {_("p_view_files"), 'v', _("bool"), OPTION_ARG_OPTIONAL, _("File View in Pick"), 5},
    {_("wrap"), 'w', _("bool"), OPTION_ARG_OPTIONAL, _("view wrap lines"), 5},
    {_("f_ln"), 'N', _("bool"), OPTION_ARG_OPTIONAL, _("line numbers in view"), 5},
    {_("fill_char"), 'f', _("char"), 0, _("field fill_char (_,.,empty)"), 5},
    {_("brackets"), 'u', _("text"), 0, _("brackets around fields ([]{}<>)"), 5},
    {_("editor"), CM_EDITOR, _("text"), 0, _("default editor"), 5},
    {_("tab_stop"), 't', _("number"), 0, _("number of spaces per tab (4)"), 5},
    {_("timeout_secs"), 'Z', _("text"), 0, _("seconds to wait for input"), 3},
    {_("h_shift"), 'z', _("number"), 0, _("horizontal shift width (16)"), 5},
    {_("border"), 'b', _("text"), 0, _("single, rounded, double, heavy, none"), 5},
    {_("bg"), BG, _("hex_clr"), 0, _("Terminal (stdscr) background (#000000)"), 6},
    {_("fg"), FG, _("hex_clr"), 0, _("Terminal (stdscr) foreground (#d0d0d0)"), 6},
    {_("box_fg"), BOX_FG, _("hex_clr"), 0, _("box foreground (#d0d0d0)"), 6},
    {_("box_bg"), BOX_BG, _("hex_clr"), 0, _("box background (#000000)"), 6},
    {_("ind_fg"), IND_FG, _("hex_clr"), 0, _("indicator foreground (#d0d0d0)"), 6},
    {_("ind_bg"), IND_BG, _("hex_clr"), 0, _("indicator background (#000000)"), 6},
    {_("brackets_fg"), BRACKETS_FG, _("hex_clr"), 0, _("brackets foreground (#d0d0d0)"), 6},
    {_("brackets_bg"), BRACKETS_BG, _("hex_clr"), 0, _("brackets background (#000000)"), 6},
    {_("fill_char_fg"), FILL_CHAR_FG, _("hex_clr"), 0, _("fill character foreground (#d0d0d0)"), 6},
    {_("fill_char_bg"), FILL_CHAR_BG, _("hex_clr"), 0, _("fill character background (#000000)"), 6},
    {_("nt_fg"), NT_FG, _("hex_clr"), 0, _("normal foreground (#d0d0d0)"), 6},
    {_("nt_bg"), NT_BG, _("hex_clr"), 0, _("normal background (#000000)"), 6},
    {_("nt_rev_fg"), NT_REV_FG, _("hex_clr"), 0, _("normal reverse foreground (#000000)"), 6},
    {_("nt_rev_bg"), NT_REV_BG, _("hex_clr"), 0, _("normal reverse background (#d0d0d0)"), 6},
    {_("nt_hl_fg"), NT_HL_FG, _("hex_clr"), 0, _("normal highlight foreground (#ffffff)"), 6},
    {_("nt_hl_bg"), NT_HL_BG, _("hex_clr"), 0, _("normal highlight background (#000000)"), 6},
    {_("nt_hl_rev_fg"), NT_HL_REV_FG, _("hex_clr"), 0, _("normal highlight reverse foreground (#f00000)"), 6},
    {_("nt_hl_rev_bg"), NT_HL_REV_BG, _("hex_clr"), 0, _("normal highlight reverse background (#d0d0d0)"), 6},
    {_("ln_fg"), LN_FG, _("hex_clr"), 0, _("line number foreground (#0000b0)"), 6},
    {_("ln_bg"), LN_BG, _("hex_clr"), 0, _("line number background (#202020)"), 6},
    {_("cmdln_fg"), CMDLN_FG, _("hex_clr"), 0, _("line number foreground (#0000b0)"), 6},
    {_("cmdln_bg"), CMDLN_BG, _("hex_clr"), 0, _("line number background (#202020)"), 6},
    {_("title_fg"), TITLE_FG, _("hex_clr"), 0, _("title foreground (#d0d0d0)"), 6},
    {_("title_bg"), TITLE_BG, _("hex_clr"), 0, _("title background (#000000)"), 6},
    {_("ran_fg"), RAN_FG, _("hex_clr"), 0, _("ran foreground (#d0d0d0)"), 6},
    {_("ran_bg"), RAN_BG, _("hex_clr"), 0, _("ran background (#000000)"), 6},
    {_("blue_gamma"), GM_BLUE, _("float"), 0, _("blue_gamma (1.2)"), 7},
    {_("gray_gamma"), GM_GRAY, _("float"), 0, _("gray gamma (1.2)"), 7},
    {_("green_gamma"), GM_GREEN, _("float"), 0, _("green gamma (1.2)"), 7},
    {_("red_gamma"), GM_RED, _("float"), 0, _("red gamma (View)"), 7},
    {_("black"), XBLACK, _("hex_clr"), 0, _("black (#000000)"), 8},
    {_("red"), XRED, _("hex_clr"), 0, _("red (#bf0000)"), 8},
    {_("green"), XGREEN, _("hex_clr"), 0, _("green (#00cf00)"), 8},
    {_("yellow"), XYELLOW, _("hex_clr"), 0, _("yellow (#efbf00)"), 8},
    {_("blue"), XBLUE, _("hex_clr"), 0, _("blue (#0000FF)"), 8},
    {_("magenta"), XMAGENTA, _("hex_clr"), 0, _("magenta (#9f009f)"), 8},
    {_("cyan"), XCYAN, _("hex_clr"), 0, _("cyan (#00dfdf)"), 8},
    {_("white"), XWHITE, _("hex_clr"), 0, _("white (#d0d0d0)"), 8},
    {_("bblack"), XBBLACK, _("hex_clr"), 0, _("bright black (#7f7f7f)"), 8},
    {_("bred"), XBRED, _("hex_clr"), 0, _("bright red (#FF3737)"), 8},
    {_("bgreen"), XBGREEN, _("hex_clr"), 0, _("bright green (#00FF7f)"), 8},
    {_("byellow"), XBYELLOW, _("hex_clr"), 0, _("bright yellow (#FFeF00)"), 8},
    {_("bblue"), XBBLUE, _("hex_clr"), 0, _("bright blue (#00cfFF)"), 8},
    {_("bmagenta"), XMAGENTA, _("hex_clr"), 0, _("bright magenta (#FF00FF)"), 8},
    {_("bcyan"), XBCYAN, _("hex_clr"), 0, _("bright cyan (#00FFFF)"), 8},
    {_("bwhite"), XBWHITE, _("hex_clr"), 0, _("bright white (#FFFFFF)"), 8},
    {_("mapp_data"), MAPP_DATA, _("directory"), 0, _("data directory"), 9},
    {_("mapp_help"), MAPP_HELP, _("directory"), 0, _("help directory"), 9},
    {_("mapp_home"), MAPP_HOME, _("directory"), 0, _("home directory"), 9},
    {_("mapp_msrc"), MAPP_MSRC, _("directory"), 0, _("source directory"), 9},
    {_("mapp_user"), MAPP_USER, _("directory"), 0, _("user directory"), 9},
    {_("mapp_theme"), MAPP_THEME, _("file"), 0, _("default theme file"), 9},
    {0},
};

static error_t
parse_opt(int key, char *arg, struct argp_state *state) {
    Init *init = state->input;
    SIO *sio = init->sio;
    switch (key) {
    case 'W':
        f_write_config = true;
        break;
    case 'a':
        strnz__cpy(init->minitrc, arg, MAXLEN - 1);
        break;
    case 'k':
        strnz__cpy(init->parent_cmd, arg, MAXLEN - 1);
        break;
    case 'C':
        init->cols = atoi(arg);
        break;
    case 'L':
        init->lines = atoi(arg);
        break;
    case 'T':
        strnz__cpy(init->title, arg, MAXLEN - 1);
        break;
    case 'X':
        init->begx = atoi(arg);
        break;
    case 'Y':
        init->begy = atoi(arg);
        break;
    case 'A':
        strnz__cpy(init->cmd_all, arg, MAXLEN - 1);
        break;
    case 'c':
        strnz__cpy(init->cmd, arg, MAXLEN - 1);
        break;
    case 'd':
        strnz__cpy(init->mapp_spec, arg, MAXLEN - 1);
        break;
    case 'H':
        strnz__cpy(init->help_spec, arg, MAXLEN - 1);
        break;
    case 'i':
        strnz__cpy(init->in_spec, arg, MAXLEN - 1);
        break;
    case 'g':
        strnz__cpy(init->log_file_spec, arg, MAXLEN - 1);
        break;
    case 'l':
        str_to_upper(arg);
        if (!strcmp(arg, _("SILENT")))
            init->min_log_level = SILENT;
        else if (!strcmp(arg, _("DEBUG")))
            init->min_log_level = DEBUG;
        else if (!strcmp(arg, _("INFO")))
            init->min_log_level = INFO;
        else if (!strcmp(arg, _("WARN")))
            init->min_log_level = WARN;
        else if (!strcmp(arg, _("ERROR")))
            init->min_log_level = ERROR;
        else if (!strcmp(arg, _("FATAL")))
            init->min_log_level = FATAL;
        else
            init->min_log_level = SILENT;
        break;
    case 'o':
        strnz__cpy(init->out_spec, arg, MAXLEN - 1);
        break;
    case 'R':
        strnz__cpy(init->receiver_cmd, arg, MAXLEN - 1);
        break;
    case 'S':
        strnz__cpy(init->provider_cmd, arg, MAXLEN - 1);
        break;
    case 'e':
        init->f_erase_remainder = true;
        break;
    case 'f':
        strnz__cpy(init->fill_char, arg, 1);
        break;
    case 'j':
        init->f_strip_ansi = true;
        break;
    case 'M':
        init->f_multiple_cmd_args = true;
        break;
    case 'n':
        init->select_max = atoi(arg);
        break;
    case 'N':
        if (arg)
            init->f_ln = str_to_bool(arg);
        if (arg && arg[0] == 't')
            init->f_ln = true;
        else if (arg && arg[0] == 'f')
            init->f_ln = false;
        else
            init->f_ln = true;
        break;
    case 'r':
        init->f_read_theme = true;
        break;
    case 's':
        init->f_squeeze = true;
        break;
    case 't':
        init->tab_stop = atoi(arg);
        if (init->tab_stop < 1)
            init->tab_stop = 1;
        break;
    case 'Z':
        init->timeout_secs = atoi(arg);
        break;
    case 'z':
        init->h_shift = atoi(arg);
        if (init->h_shift < 1)
            init->h_shift = 1;
        break;
    case 'u':
        strnz__cpy(init->brackets, arg, 2);
        break;
    case 'w':
        if (arg && arg[0] == 't')
            init->wrap = true;
        else if (arg && arg[0] == 'f')
            init->wrap = false;
        else
            init->wrap = true;
        break;
    case 'v':
        init->p_view_files = true;
        if (arg)
            init->p_view_files = str_to_bool(arg);
        break;
    case BG:
        sscanf(arg, _("#%06x"), &sio->bg);
        break;
    case FG:
        sscanf(arg, _("#%06x"), &sio->fg);
        break;
    case BORDER:
        char c = arg[0];
        if (c == 'r' || c == 'R')
            sio->border = 'r';
        else if (c == 's' || c == 'S')
            sio->border = 's';
        else if (c == 'd' || c == 'D')
            sio->border = 'd';
        else if (c == 'h' || c == 'H')
            sio->border = 'h';
        else
            sio->border = 'n';
        break;
    case BOX_FG:
        sscanf(arg, _("#%06x"), &sio->box_fg);
        break;
    case BOX_BG:
        sscanf(arg, _("#%06x"), &sio->box_bg);
        break;
    case IND_FG:
        sscanf(arg, _("#%06x"), &sio->ind_fg);
        break;
    case IND_BG:
        sscanf(arg, _("#%06x"), &sio->ind_bg);
        break;
    case BRACKETS_FG:
        sscanf(arg, _("#%06x"), &sio->brackets_fg);
        break;
    case BRACKETS_BG:
        sscanf(arg, _("#%06x"), &sio->brackets_bg);
        break;
    case FILL_CHAR_FG:
        sscanf(arg, _("#%06x"), &sio->fill_char_fg);
        break;
    case FILL_CHAR_BG:
        sscanf(arg, _("#%06x"), &sio->fill_char_bg);
        break;
    case NT_FG:
        sscanf(arg, _("#%06x"), &sio->nt_fg);
        break;
    case NT_BG:
        sscanf(arg, _("#%06x"), &sio->nt_bg);
        break;
    case NT_REV_FG:
        sscanf(arg, _("#%06x"), &sio->nt_rev_fg);
        break;
    case NT_REV_BG:
        sscanf(arg, _("#%06x"), &sio->nt_rev_bg);
        break;
    case NT_HL_FG:
        sscanf(arg, _("#%06x"), &sio->nt_hl_fg);
        break;
    case NT_HL_BG:
        sscanf(arg, _("#%06x"), &sio->nt_hl_bg);
        break;
    case NT_HL_REV_FG:
        sscanf(arg, _("#%06x"), &sio->nt_hl_rev_fg);
        break;
    case NT_HL_REV_BG:
        sscanf(arg, _("#%06x"), &sio->nt_hl_rev_bg);
        break;
    case TITLE_FG:
        sscanf(arg, _("#%06x"), &sio->title_fg);
        break;
    case TITLE_BG:
        sscanf(arg, _("#%06x"), &sio->title_bg);
        break;
    case LN_FG:
        sscanf(arg, _("#%06x"), &sio->ln_fg);
        break;
    case LN_BG:
        sscanf(arg, _("#%06x"), &sio->ln_bg);
        break;
    case CMDLN_FG:
        sscanf(arg, _("#%06x"), &sio->cmdln_fg);
        break;
    case CMDLN_BG:
        sscanf(arg, _("#%06x"), &sio->cmdln_bg);
        break;
    case RAN_FG:
        sscanf(arg, _("#%06x"), &sio->ran_fg);
        break;
    case RAN_BG:
        sscanf(arg, _("#%06x"), &sio->ran_bg);
        break;
    case GM_BLUE:
        sio->blue_gamma = str_to_double(arg);
        break;
    case GM_GRAY:
        sio->gray_gamma = str_to_double(arg);
        break;
    case GM_GREEN:
        sio->green_gamma = str_to_double(arg);
        break;
    case GM_RED:
        sio->red_gamma = str_to_double(arg);
        break;
    case MAPP_USER:
        strnz__cpy(init->mapp_user, arg, MAXLEN - 1);
        break;
    case MAPP_DATA:
        strnz__cpy(init->mapp_data, arg, MAXLEN - 1);
        break;
    case MAPP_HELP:
        strnz__cpy(init->mapp_help, arg, MAXLEN - 1);
        break;
    case MAPP_HOME:
        strnz__cpy(init->mapp_home, arg, MAXLEN - 1);
        break;
    case MAPP_MSRC:
        strnz__cpy(init->mapp_msrc, arg, MAXLEN - 1);
        break;
    case MAPP_SPEC:
        strnz__cpy(init->mapp_spec, arg, MAXLEN - 1);
        break;
    case HELP_SPEC:
        strnz__cpy(init->help_spec, arg, MAXLEN - 1);
        break;
    case MAPP_THEME:
        strnz__cpy(init->mapp_theme, arg, MAXLEN - 1);
        break;
    case ARGP_KEY_ARG:
        if (state->arg_num >= 35)
            argp_usage(state);
        init->argv[state->arg_num] = strdup(arg);
        break;
    case ARGP_KEY_END:
        init->argc = state->arg_num;
        init->argv[state->arg_num + 1] = nullptr;
        break;
    default:
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp argp = {options, parse_opt, args_doc, doc,
                           nullptr, nullptr, nullptr};

/** @brief Main initialization function for MAPP - Menu Application
    @ingroup init
    @param init - pointer to Init struct to be initialized
    @param argc - argument count from main()
    @param argv - argument vector from main()
    @code
    1. Read environment variables and set defaults
    2. Parse configuration file
    3. Parse command-line options
    4. Set up SIO struct with colors and other settings
    5. Handle special options like help and version
    @endcode
 */
void mapp_initialization(Init *init, int argc, char **argv) {
    char term[MAXLEN];
    char tmp_str[MAXLEN];
    char *e;
    setlocale(LC_ALL, _("en_US.UTF-8"));

    init->sio = (SIO *)calloc(1, sizeof(SIO));
    if (!init->sio) {
        ui_perror(_("calloc init->sio failed"));
        exit(EXIT_FAILURE);
    }
    SIO *sio = init->sio;
    if (!init) {
        ssnprintf(tmp_str, sizeof(tmp_str), _("%s"),
                  _("init struct not allocated on entry"));
        ui_abend(-1, tmp_str);
        exit(-1);
    }
    e = getenv(_("CMENU_HOME"));
    if (!e || *e == '\0')
        strnz__cpy(init->mapp_home, _("~/menuapp"), MAXLEN);
    else
        strnz__cpy(init->mapp_home, e, MAXLEN);

    if (init->mapp_home[0] != '\0') {
        expand_tilde(init->mapp_home, MAXLEN - 1);
        if (!verify_dir(init->mapp_home, R_OK))
            ui_abend(-1, _("MAPP_HOME directory invalid"));
    }
    // CMENU_RC should be an absolute path
    e = getenv(_("CMENU_RC"));
    if (!e || *e == '\0') {
        strnz__cpy(init->minitrc, init->mapp_home, MAXLEN - 1);
        strnz__cat(init->minitrc, _("/.minitrc"), MAXLEN);
    } else
        strnz__cpy(init->minitrc, e, MAXLEN);
    if (init->mapp_user[0] == '\0') {
        strnz__cpy(init->mapp_user, init->mapp_home, MAXLEN - 1);
        strnz__cat(init->mapp_user, _("/user"), MAXLEN - 1);
    }
    if (init->mapp_theme[0] == '\0') {
        strnz__cpy(init->mapp_theme, init->mapp_home, MAXLEN - 1);
        strnz__cat(init->mapp_theme, _("/themes/default"), MAXLEN - 1);
    }
    if (init->mapp_msrc[0] == '\0') {
        strnz__cpy(init->mapp_msrc, init->mapp_home, MAXLEN - 1);
        strnz__cat(init->mapp_msrc, _("/msrc"), MAXLEN - 1);
    }
    if (init->mapp_data[0] == '\0') {
        strnz__cpy(init->mapp_data, init->mapp_home, MAXLEN - 1);
        strnz__cat(init->mapp_data, _("/data"), MAXLEN - 1);
    }
    if (init->mapp_help[0] == '\0') {
        strnz__cpy(init->mapp_help, init->mapp_home, MAXLEN - 1);
        strnz__cat(init->mapp_help, _("/help"), MAXLEN - 1);
    }
    init->mapp_spec[0] = '\0'; /**< menu specification file */
    // Set default colors and settings in SIO struct
    // These can be overridden by the config file or command-line options
    // Included here to ensure SIO has valid defaults even if config parsing fails

    sio->border = 'r'; /**< default border style */
    sio->bg = 0x000000;
    sio->fg = 0xc0c0c0;
    sio->border = 'r';
    sio->box_fg = 0xf00000;
    sio->box_bg = 0x000000;
    sio->ind_fg = 0xf00000;
    sio->ind_bg = 0x000000;
    sio->ran_fg = 0xf00000;
    sio->ran_bg = 0x000000;
    sio->title_fg = 0xf0f0f0;
    sio->title_bg = 0x000000;
    sio->nt_fg = 0xc0c0c0;
    sio->nt_bg = 0x000000;
    sio->nt_rev_fg = 0x000000;
    sio->nt_rev_bg = 0xc0c0c0;
    sio->nt_hl_fg = 0xf00000;
    sio->nt_hl_bg = 0x000000;
    sio->nt_hl_rev_fg = 0x000000;
    sio->nt_hl_rev_bg = 0xc0c0c0;
    sio->ln_fg = 0x0070ff;
    sio->ln_bg = 0x101010;
    sio->cmdln_fg = 0xd0d0d0;
    sio->cmdln_bg = 0x000000;
    init->f_erase_remainder = true;               /**< erase remainder on enter */
    init->brackets[0] = '\0';                     /**< field enclosure brackets */
    strnz__cpy(init->fill_char, " ", MAXLEN - 1); /**< field fill character */
    e = getenv(_("TERM"));
    if (e == nullptr || *e == '\0')
        strnz__cpy(term, _("xterm-256color"), MAXLEN);
    else
        strnz__cpy(term, e, MAXLEN - 1);
    e = getenv(_("EDITOR"));
    if (e && *e != '\0')
        strnz__cpy(init->editor, _("vi"), MAXLEN - 1);
    else
        strnz__cpy(init->editor, e, MAXLEN - 1);
    process_config_files(init);
    ui_min_log_level = init->min_log_level;
    ui_log_fp = ui_open_log();
    ui_log(INFO, _("mapp_initialization"));
    ui_log(INFO, _("config files processed"));
    init->mapp_spec[0] = '\0';
    init->argc = argc;
    argp_parse(&argp, argc, argv, 0, 0, init);
    strnz__cpy(sio->fill_char, init->fill_char, 1);
    strnz__cpy(sio->brackets, init->brackets, 2);
    if (f_write_config) {
        write_config(init);
        exit(EXIT_SUCCESS);
    }
}
/** @brief Parse command-line options and set Init struct values accordingly
    @ingroup init
    @param init - pointer to Init struct to be populated with option values
    @param argc - argument count from main()
    @param argv - argument vector from main()
    @returns 0 on success, -1 on failure
    @details This function uses the argp library to parse command-line
   options defined in the options array. It updates the Init struct with
   values from the options and handles any special flags for dumping or
   writing configuration.
 */
int parse_opt_args(Init *init, int argc, char **argv) {
    init->argc = destroy_argv(init->argc, init->argv);
    argp_parse(&argp, argc, argv, 0, 0, init);
    return 0;
}

/** @brief Initialize optional arguments in the Init struct to default
   values
    @ingroup init
    @param init - pointer to Init struct to be initialized This function
   sets all optional argument fields in the Init struct to their default
   values before parsing command-line options or configuration file. This
   ensures that any fields not specified by the user will have known default
   values.
 */
void zero_opt_args(Init *init) {
    init->lines = 0;
    init->cols = 0;
    init->begx = 0;
    init->begy = 0;
    init->f_mapp_desc = false;
    init->f_provider_cmd = false;
    init->f_receiver_cmd = false;
    init->f_title = false;
    init->f_mapp_spec = false;
    init->f_help_spec = false;
    init->f_in_spec = false;
    init->f_out_spec = false;
    init->p_view_files = false;
    init->h_shift = 0;
    init->mapp_spec[0] = init->help_spec[0] = '\0';
    init->provider_cmd[0] = init->receiver_cmd[0] = '\0';
    init->title[0] = '\0';
    init->cmd[0] = init->cmd_all[0] = '\0';
    init->parent_cmd[0] = '\0';
    init->in_spec[0] = init->out_spec[0] = '\0';
    init->help_spec[0] = '\0';
    init->in_spec[0] = '\0';
    init->out_spec[0] = '\0';
}
/** @brief parse the configuration file specified in init->minitrc and set
   Init struct values accordingly
    @ingroup init
    @returns on success, -1 on failure
    @details Lines beginning with '#" are comments, discard.
    Copy line to tmp_str removing quotes, spaces, semicolons, and
   newlines.
    Record structure is _("parse key=value pairs").
    Skip lines without '='.
    Set init struct values based on key.
    Skip unknown keys. */
int process_config_files(Init *init) {
    char config_file_name[MAXLEN];
    int rc;
    if (!init->minitrc[0]) {
        char *e = getenv(_("MINITRC"));
        if (e)
            strnz__cpy(init->minitrc, e, MAXLEN - 1);
        else
            strnz__cpy(init->minitrc, _("~/.minitrc"), MAXLEN - 1);
    }

    expand_tilde(init->minitrc, MAXLEN - 1);
    strnz__cpy(config_file_name, init->minitrc, MAXLEN - 1);
    rc = process_config_file(config_file_name, init);

    expand_tilde(init->mapp_theme, MAXLEN - 1);
    strnz__cpy(config_file_name, init->mapp_theme, MAXLEN - 1);
    rc = process_config_file(config_file_name, init);
    return rc;
}

int process_config_file(char *config_file_name, Init *init) {
    char include_file_name[MAXLEN];
    char tmp_str[MAXLEN];
    char *src_p, *dp;
    SIO *sio = init->sio;
    char hex_clr_str[8];
    char key[MAXLEN];
    char value[MAXLEN];
    FILE *config_fp = fopen(config_file_name, "r");
    char quote_char = '\0';
    if (!config_fp) {
        fprintf(stderr, "failed to read file: %s %s\n", config_file_name, strerror(errno));
        return (-1);
    }
    while (fgets(tmp_str, sizeof(tmp_str), config_fp)) {
        bool inquotes = false;
        if (tmp_str[0] == '#')
            continue;
        src_p = tmp_str;
        key[0] = '\0';
        dp = key;
        // copy delimited by "=" into value
        while (*src_p != '\0') {
            if (*src_p == '\n')
                *dp = *src_p = '\0';
            if (*src_p == '=') {
                *dp = '\0';
                src_p++;
                break;
            }
            if (*src_p == '"' && *src_p == ' ') {
                src_p++;
                continue;
            }
            *dp++ = *src_p++;
        }
        value[0] = '\0';
        dp = value;
        // copy delimited by newline or unquoted "#" into value, removing quotes, spaces, semicolons, and newlines, but respecting quotes
        while (*src_p != '\0') {
            if ((*src_p == '"' || *src_p == '\'') && (*(src_p + 1) != '\\')) {
                if (!inquotes) {
                    inquotes = true;
                    quote_char = *src_p++;
                } else if (*src_p == quote_char) {
                    inquotes = false;
                    quote_char = '\0';
                }
            }
            if (!inquotes) {
                if (*src_p == '#') {
                    if (unstr_hex_clr(hex_clr_str, src_p)) {
                        strnz__cpy(value, hex_clr_str, MAXLEN - 1);
                        dp = value + strlen(value);
                    }
                    break;
                }
            }
            if (*src_p == ' ' || *src_p == ';') {
                src_p++;
                continue;
            }
            if (*src_p == '\n') {
                *src_p = '\0';
                break;
            }
            *dp++ = *src_p++;
        }
        *dp = '\0';
        if (key[0] == '\0')
            continue;
        if (value[0] == '\0')
            continue;
        if (!strcmp(key, _("include"))) {
            strnz__cpy(include_file_name, value, MAXLEN - 1);
            expand_tilde(include_file_name, MAXLEN - 1);
            process_config_file(include_file_name, init);
            continue;
        }
        if (!strcmp(key, _("minitrc"))) {
            strnz__cpy(init->minitrc, value, MAXLEN - 1);
            continue;
        }
        if (!strcmp(key, _("lines"))) {
            init->lines = atoi(value);
            continue;
        }
        if (!strcmp(key, _("cols"))) {
            init->cols = atoi(value);
            continue;
        }
        if (!strcmp(key, _("begy"))) {
            init->begy = atoi(value);
            continue;
        }
        if (!strcmp(key, _("begx"))) {
            init->begx = atoi(value);
            continue;
        }
        if (!strcmp(key, _("f_ln"))) {
            init->f_ln = str_to_bool(value);
            continue;
        }
        if (!strcmp(key, _("f_at_end_remove"))) {
            init->f_at_end_remove = str_to_bool(value);
            continue;
        }
        if (!strcmp(key, _("f_erase_remainder"))) {
            init->f_erase_remainder = str_to_bool(value);
            continue;
        }
        if (!strcmp(key, _("brackets"))) {
            strnz__cpy(init->brackets, value, 2);
            continue;
        }
        if (!strcmp(key, _("fill_char"))) {
            if (strlen(value) > 1)
                value[1] = '\0';
            if (wcwidth((int)value[0]) > 1)
                value[0] = '?';
            strnz__cpy(init->fill_char, value, 4);
            continue;
        }
        if (!strcmp(key, _("f_ignore_case"))) {
            init->f_ignore_case = str_to_bool(value);
            continue;
        }
        if (!strcmp(key, _("p_view_files"))) {
            init->p_view_files = str_to_bool(value);
            continue;
        }
        if (!strcmp(key, _("f_read_theme"))) {
            init->f_read_theme = str_to_bool(value);
            continue;
        }
        if (!strcmp(key, _("f_squeeze"))) {
            init->f_squeeze = str_to_bool(value);
            continue;
        }
        if (!strcmp(key, _("f_strip_ansi"))) {
            init->f_strip_ansi = str_to_bool(value);
            continue;
        }
        if (!strcmp(key, _("f_multiple_cmd_args"))) {
            init->f_multiple_cmd_args = str_to_bool(value);
            continue;
        }
        if (!strcmp(key, _("select_max"))) {
            init->select_max = atoi(value);
            continue;
        }
        if (!strcmp(key, _("tab_stop"))) {
            init->tab_stop = atoi(value);
            continue;
        }
        if (!strcmp(key, _("timeout_secs"))) {
            init->timeout_secs = atoi(value);
            continue;
        }
        if (!strcmp(key, _("h_shift"))) {
            init->h_shift = atoi(value);
            continue;
        }
        if (!strcmp(key, _("wrap"))) {
            init->wrap = str_to_bool(value);
            continue;
        }
        if (!strcmp(key, _("title"))) {
            strnz__cpy(init->title, value, MAXLEN - 1);
            continue;
        }
        if (!strcmp(key, _("cmd"))) {
            strnz__cpy(init->cmd, value, MAXLEN - 1);
            continue;
        }
        if (!strcmp(key, _("cmd_all"))) {
            strnz__cpy(init->cmd_all, value, MAXLEN - 1);
            continue;
        }
        if (!strcmp(key, _("parent_cmd"))) {
            strnz__cpy(init->parent_cmd, value, MAXLEN - 1);
            continue;
        }
        if (!strcmp(key, _("provider_cmd"))) {
            strnz__cpy(init->provider_cmd, value, MAXLEN - 1);
            continue;
        }
        if (!strcmp(key, _("receiver_cmd"))) {
            strnz__cpy(init->receiver_cmd, value, MAXLEN - 1);
            continue;
        }
        if (!strcmp(key, _("editor"))) {
            strnz__cpy(init->editor, value, MAXLEN - 1);
            continue;
        }
        if (!strcmp(key, _("border"))) {
            char c = value[0];
            if (c == 'r' || c == 'R')
                sio->border = 'r';
            else if (c == 's' || c == 'S')
                sio->border = 's';
            else if (c == 'd' || c == 'D')
                sio->border = 'd';
            else if (c == 'h' || c == 'H')
                sio->border = 'h';
            else
                sio->border = 'n';
            continue;
        }
        if (!strcmp(key, _("fg"))) {
            sscanf(value, _("#%06x"), &sio->fg);
            continue;
        }
        if (!strcmp(key, _("bg"))) {
            sscanf(value, _("#%06x"), &sio->bg);
            continue;
        }
        if (!strcmp(key, _("box_fg"))) {
            sscanf(value, _("#%06x"), &sio->fg);
            continue;
        }
        if (!strcmp(key, _("box_bg"))) {
            sscanf(value, _("#%06x"), &sio->box_bg);
            continue;
        }
        if (!strcmp(key, _("ind_fg"))) {
            sscanf(value, _("#%06x"), &sio->ind_fg);
            continue;
        }
        if (!strcmp(key, _("ind_bg"))) {
            sscanf(value, _("#%06x"), &sio->ind_bg);
            continue;
        }
        if (!strcmp(key, _("brackets_fg"))) {
            sscanf(value, _("#%06x"), &sio->brackets_fg);
            continue;
        }
        if (!strcmp(key, _("brackets_bg"))) {
            sscanf(value, _("#%06x"), &sio->brackets_bg);
            continue;
        }
        if (!strcmp(key, _("fill_char_fg"))) {
            sscanf(value, _("#%06x"), &sio->fill_char_fg);
            continue;
        }
        if (!strcmp(key, _("fill_char_bg"))) {
            sscanf(value, _("#%06x"), &sio->fill_char_bg);
            continue;
        }
        if (!strcmp(key, _("ln_fg"))) {
            sscanf(value, _("#%06x"), &sio->ln_fg);
            continue;
        }
        if (!strcmp(key, _("ln_bg"))) {
            sscanf(value, _("#%06x"), &sio->ln_bg);
            continue;
        }
        if (!strcmp(key, _("cmdln_fg"))) {
            sscanf(value, _("#%06x"), &sio->cmdln_fg);
            continue;
        }
        if (!strcmp(key, _("cmdln_bg"))) {
            sscanf(value, _("#%06x"), &sio->cmdln_bg);
            continue;
        }
        if (!strcmp(key, _("nt_fg"))) {
            sscanf(value, _("#%06x"), &sio->nt_fg);
            continue;
        }
        if (!strcmp(key, _("nt_bg"))) {
            sscanf(value, _("#%06x"), &sio->nt_bg);
            continue;
        }
        if (!strcmp(key, _("nt_rev_fg"))) {
            sscanf(value, _("#%06x"), &sio->nt_rev_fg);
            continue;
        }
        if (!strcmp(key, _("nt_rev_bg"))) {
            sscanf(value, _("#%06x"), &sio->nt_rev_bg);
            continue;
        }
        if (!strcmp(key, _("nt_hl_fg"))) {
            sscanf(value, _("#%06x"), &sio->nt_hl_fg);
            continue;
        }
        if (!strcmp(key, _("nt_hl_bg"))) {
            sscanf(value, _("#%06x"), &sio->nt_hl_bg);
            continue;
        }
        if (!strcmp(key, _("nt_hl_rev_fg"))) {
            sscanf(value, _("#%06x"), &sio->nt_hl_rev_fg);
            continue;
        }
        if (!strcmp(key, _("nt_hl_rev_bg"))) {
            sscanf(value, _("#%06x"), &sio->nt_hl_rev_bg);
            continue;
        }
        if (!strcmp(key, _("title_fg"))) {
            sscanf(value, _("#%06x"), &sio->title_fg);
            continue;
        }
        if (!strcmp(key, _("title_bg"))) {
            sscanf(value, _("#%06x"), &sio->title_bg);
            continue;
        }
        if (!strcmp(key, _("ran_fg"))) {
            sscanf(value, _("#%06x"), &sio->ran_fg);
            continue;
        }
        if (!strcmp(key, _("ran_bg"))) {
            sscanf(value, _("#%06x"), &sio->ran_bg);
            continue;
        }
        if (!strcmp(key, _("red_gamma"))) {
            sio->red_gamma = str_to_double(value);
            continue;
        }
        if (!strcmp(key, _("green_gamma"))) {
            sio->green_gamma = str_to_double(value);
            continue;
        }
        if (!strcmp(key, _("blue_gamma"))) {
            sio->blue_gamma = str_to_double(value);
            continue;
        }
        if (!strcmp(key, _("gray_gamma"))) {
            sio->gray_gamma = str_to_double(value);
            continue;
        }
        if (!strcmp(key, _("black"))) {
            sscanf(value, _("#%06x"), &sio->black);
            continue;
        }
        if (!strcmp(key, _("red"))) {
            sscanf(value, _("#%06x"), &sio->red);
            continue;
        }
        if (!strcmp(key, _("green"))) {
            sscanf(value, _("#%06x"), &sio->green);
            continue;
        }
        if (!strcmp(key, _("yellow"))) {
            sscanf(value, _("#%06x"), &sio->yellow);
            continue;
        }
        if (!strcmp(key, _("blue"))) {
            sscanf(value, _("#%06x"), &sio->blue);
            continue;
        }
        if (!strcmp(key, _("magenta"))) {
            sscanf(value, _("#%06x"), &sio->magenta);
            continue;
        }
        if (!strcmp(key, _("cyan"))) {
            sscanf(value, _("#%06x"), &sio->cyan);
            continue;
        }
        if (!strcmp(key, _("white"))) {
            sscanf(value, _("#%06x"), &sio->white);
            continue;
        }
        if (!strcmp(key, _("orange"))) {
            sscanf(value, _("#%06x"), &sio->orange);
            continue;
        }
        if (!strcmp(key, _("bblack"))) {
            sscanf(value, _("#%06x"), &sio->bblack);
            continue;
        }
        if (!strcmp(key, _("bred"))) {
            sscanf(value, _("#%06x"), &sio->bred);
            continue;
        }
        if (!strcmp(key, _("bgreen"))) {
            sscanf(value, _("#%06x"), &sio->bgreen);
            continue;
        }
        if (!strcmp(key, _("byellow"))) {
            sscanf(value, _("#%06x"), &sio->byellow);
            continue;
        }
        if (!strcmp(key, _("bblue"))) {
            sscanf(value, _("#%06x"), &sio->bblue);
            continue;
        }
        if (!strcmp(key, _("bmagenta"))) {
            sscanf(value, _("#%06x"), &sio->bmagenta);
            continue;
        }
        if (!strcmp(key, _("bcyan"))) {
            sscanf(value, _("#%06x"), &sio->bcyan);
            continue;
        }
        if (!strcmp(key, _("bwhite"))) {
            sscanf(value, _("#%06x"), &sio->bwhite);
            continue;
        }
        if (!strcmp(key, _("borange"))) {
            sscanf(value, _("#%06x"), &sio->borange);
            continue;
        }
        if (!strcmp(key, _("mapp_spec"))) {
            strnz__cpy(init->mapp_spec, value, MAXLEN - 1);
            continue;
        }
        if (!strcmp(key, _("mapp_data"))) {
            strnz__cpy(init->mapp_data, value, MAXLEN - 1);
            continue;
        }
        if (!strcmp(key, _("mapp_help"))) {
            strnz__cpy(init->mapp_help, value, MAXLEN - 1);
            continue;
        }
        if (!strcmp(key, _("mapp_home"))) {
            strnz__cpy(init->mapp_home, value, MAXLEN - 1);
            continue;
        }
        if (!strcmp(key, _("mapp_msrc"))) {
            strnz__cpy(init->mapp_msrc, value, MAXLEN - 1);
            continue;
        }
        if (!strcmp(key, _("mapp_user"))) {
            strnz__cpy(init->mapp_user, value, MAXLEN - 1);
            continue;
        }
        if (!strcmp(key, _("mapp_theme"))) {
            strnz__cpy(init->mapp_theme, value, MAXLEN - 1);
            continue;
        }
        if (!strcmp(key, _("log_file_spec"))) {
            strnz__cpy(init->log_file_spec, value, MAXLEN - 1);
            continue;
        }
        if (!strcmp(key, _("log_level"))) {
            str_to_upper(value);
            if (!strcmp(value, _("SILENT")))
                init->min_log_level = SILENT;
            else if (!strcmp(value, _("DEBUG")))
                init->min_log_level = DEBUG;
            else if (!strcmp(value, _("INFO")))
                init->min_log_level = INFO;
            else if (!strcmp(value, _("WARN")))
                init->min_log_level = WARN;
            else if (!strcmp(value, _("ERROR")))
                init->min_log_level = ERROR;
            else if (!strcmp(value, _("FATAL")))
                init->min_log_level = FATAL;
            else
                init->min_log_level = SILENT;
            continue;
        }
    }
    (void)fclose(config_fp);
    return 0;
}
/** @brief Write the current configuration to a file specified in
   init->minitrc
    @ingroup init
    @param init - pointer to Init struct containing current configuration
    @returns 0 on success, -1 on failure
    @details The configuration is written in key=value format, one per line.
    Lines beginning with '#' are comments and are ignored when reading
   the config file.
    The file is created if it does not exist, and overwritten if it
   does exist
 */
int write_config(Init *init) {
    char *e;
    char minitrc_dmp[MAXLEN];
    char tmp_str[MAXLEN];
    SIO *sio = init->sio;
    e = getenv(_("CMENU_HOME"));
    minitrc_dmp[0] = '\0';
    char config_s[MAXLEN];
    if (e) {
        strnz__cpy(minitrc_dmp, e, MAXLEN - 1);
        strnz__cat(minitrc_dmp, "/", MAXLEN - 1);
    }
    strnz__cat(minitrc_dmp, _("minitrc.dmp"), MAXLEN - 1);
    ui_log(INFO, _("writing config file to: %s"), minitrc_dmp);
    FILE *minitrc_fp = fopen(minitrc_dmp, "w");
    if (minitrc_fp == (FILE *)0) {
        ssnprintf(em0, MAXLEN - 1, _("failed to open file: %s"), minitrc_dmp);
        ui_log(ERROR, _("em0"));
        return (-1);
    }
    (void)fprintf(minitrc_fp, "# %s\n", minitrc_dmp);
    char *doc_tbl[50] = {
        _("# C-Menu example configuration file"),
        "#",
        _("# This file is generated by C-Menu when run with the -W option."),
        _("# Copy this file and edit the copy because this file will be"),
        _("# overwritten each time C-Menu is run with the - W option."),
        "#",
        _("# C-Menu processes key value pairs in reading order from its"),
        _("# main configuraiton file, ~/ menuapp /.minitrc, and any other"),
        _("# configuration files sourced with include statements such as"),
        _("# the following:"),
        "#",
        _("# include = ~/menuapp/theme/default"),
        "#",
        _("# Assuming your configuration file is in ~/menuapp/theme/Red,"),
        _("# you could create a symbolic link named default that points to"),
        _("# Red, and then include default in your main configuration file."),
        _("# (Actually ~/menuapp/.minitrc already includes default, so you"),
        _("# would only need to create the theme file and the symbolic link."),
        "#",
        _("# ln -s Red default"),
        "#",
        _("# Key value pairs included from configuration files are"),
        _("# processed in reading order as they are included."),
        "#",
        _("# This is significant in the event that a key is included more"),
        _("# than once in the configuration file and / or included files."),
        _("# Only the last value read for a key will be used by C-Menu."),
        "#",
        _("# If you want to override key values in the C-Menu configuration"),
        _("# file, you can do so by inserting an include statement for a"),
        _("# supplemental configuration file below the keys you want to"),
        _("# override in the C-Menu configuration file. Conversely, if you"),
        _("# want to use a supplemental configuration as the default,"),
        _("# include it first."),
        "#",
        _("# Parsing: Lines beginning with # are comments and are ignored."),
        _("# Lines containing key=value pairs are parsed and the key and"),
        _("# value are extracted. Lines without an '=' are ignored. Values"),
        _("# are stripped of leading and trailing whitespace and quotes."),
        _("# Values can be enclosed in single or double quotes to preserve"),
        _("# leading and trailing whitespace. Values can also be specified"),
        _("# as hex color codes such as #ff0000 for red. If a value is"),
        _("# specified as a hex color code, it is parsed and stored as a"),
        _("# hex color code in the configuration. An unquoted '#' that is"),
        _("# not part of a six digit hex color code and after key values"),
        _("# have been extracted is the beginning of a comment."), ""};
    for (int i = 0; doc_tbl[i][0] != '\0'; i++)
        (void)fprintf(minitrc_fp, "%s\n", doc_tbl[i]);
    (void)fprintf(minitrc_fp, "#\n");
    ssnprintf(config_s, MAXLEN - 1, _("%s=%s"), _("parent_cmd"), init->parent_cmd);
    print_argp_doc(minitrc_fp, config_s, _("parent_cmd"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=%d"), _("cols"), init->cols);
    print_argp_doc(minitrc_fp, config_s, _("cols"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=%d"), _("lines"), init->lines);
    print_argp_doc(minitrc_fp, config_s, _("lines"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=%s"), _("title"), init->title);
    print_argp_doc(minitrc_fp, config_s, _("title"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=%d"), _("begx"), init->begx);
    print_argp_doc(minitrc_fp, config_s, _("begx"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=%d"), _("begy"), init->begy);
    print_argp_doc(minitrc_fp, config_s, _("begy"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=%s"), _("cmd_all"), init->cmd_all);
    print_argp_doc(minitrc_fp, config_s, _("cmd_all"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=%s"), _("cmd"), init->cmd);
    print_argp_doc(minitrc_fp, config_s, _("cmd"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=%s"), _("mapp_spec"), init->mapp_spec);
    print_argp_doc(minitrc_fp, config_s, _("mapp_spec"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=%s"), _("help_spec"), init->help_spec);
    print_argp_doc(minitrc_fp, config_s, _("help_spec"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=%s"), _("in_spec"), init->in_spec);
    print_argp_doc(minitrc_fp, config_s, _("in_spec"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=%s"), _("out_spec"), init->out_spec);
    print_argp_doc(minitrc_fp, config_s, _("out_spec"));

    ssnprintf(config_s, MAXLEN - 1, _("%s=%s"), _("log_file_spec"), init->log_file_spec);
    print_argp_doc(minitrc_fp, config_s, _("log_file_spec"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=%s"), _("log_level"), ui_log_level_s[init->min_log_level]);
    print_argp_doc(minitrc_fp, config_s, _("log_level"));

    ssnprintf(config_s, MAXLEN - 1, _("%s=%s"), _("receiver_cmd"), init->receiver_cmd);
    print_argp_doc(minitrc_fp, config_s, _("receiver_cmd"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=%s"), _("provider_cmd"), init->provider_cmd);
    print_argp_doc(minitrc_fp, config_s, _("provider_cmd"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=%s"), _("f_erase_remainder"), init->f_erase_remainder ? _("true") : _("false"));
    print_argp_doc(minitrc_fp, config_s, _("f_erase_remainder"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=%s"), _("fill_char"), init->fill_char);
    print_argp_doc(minitrc_fp, config_s, _("fill_char"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=%s"), _("f_strip_ansi"), init->f_strip_ansi ? _("true") : _("false"));
    print_argp_doc(minitrc_fp, config_s, _("f_strip_ansi"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=%s"), _("f_multiple_cmd_args"), init->f_multiple_cmd_args ? _("true") : _("false"));
    print_argp_doc(minitrc_fp, config_s, _("f_multiple_cmd_args"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=%d"), _("select_max"), init->select_max);
    print_argp_doc(minitrc_fp, config_s, _("select_max"));

    ssnprintf(config_s, MAXLEN - 1, _("%s=%s"), _("f_ln"), init->f_ln ? _("true") : _("false"));
    print_argp_doc(minitrc_fp, config_s, _("f_ln"));

    ssnprintf(config_s, MAXLEN - 1, _("%s=%s"), _("f_squeeze"), init->f_squeeze ? _("true") : _("false"));
    print_argp_doc(minitrc_fp, config_s, _("f_squeeze"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=%d"), _("tab_stop"), init->tab_stop);
    print_argp_doc(minitrc_fp, config_s, _("tab_stop"));

    ssnprintf(config_s, MAXLEN - 1, _("%s=%d"), _("timeout_secs"), init->timeout_secs);
    print_argp_doc(minitrc_fp, config_s, _("timeout_secs"));

    ssnprintf(config_s, MAXLEN - 1, _("%s=%d"), _("h_shift"), init->h_shift);
    print_argp_doc(minitrc_fp, config_s, _("h_shift"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=%s"), _("brackets"), init->brackets);
    print_argp_doc(minitrc_fp, config_s, _("brackets"));

    ssnprintf(config_s, MAXLEN - 1, _("%s=%s"), _("wrap"), init->wrap ? _("true") : _("false"));
    print_argp_doc(minitrc_fp, config_s, _("wrap"));

    ssnprintf(config_s, MAXLEN - 1, _("%s=%s"), _("f_ignore_case"), init->f_ignore_case ? _("true") : _("false"));
    print_argp_doc(minitrc_fp, config_s, _("f_ignore_case"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=%s"), _("p_view_files"), init->p_view_files ? _("true") : _("false"));
    print_argp_doc(minitrc_fp, config_s, _("p_view_files"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=%s"), _("f_read_theme"), init->f_read_theme ? _("true") : _("false"));
    print_argp_doc(minitrc_fp, config_s, _("f_read_theme"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=%s"), _("editor"), init->editor);
    print_argp_doc(minitrc_fp, config_s, _("editor"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=%c"), _("border"), sio->border);
    print_argp_doc(minitrc_fp, config_s, _("border"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=#%06x"), _("bg"), sio->bg);
    print_argp_doc(minitrc_fp, config_s, _("bg"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=#%6x"), _("fg"), sio->fg);
    print_argp_doc(minitrc_fp, config_s, _("fg"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=#%06x"), _("box_fg"), sio->box_fg);
    print_argp_doc(minitrc_fp, config_s, _("box_fg"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=#%06x"), _("box_bg"), sio->box_bg);
    print_argp_doc(minitrc_fp, config_s, _("box_bg"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=#%06x"), _("ind_fg"), sio->ind_fg);
    print_argp_doc(minitrc_fp, config_s, _("ind_fg"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=#%06x"), _("ind_bg"), sio->ind_bg);
    print_argp_doc(minitrc_fp, config_s, _("ind_bg"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=#%06x"), _("brackets_fg"), sio->brackets_fg);
    print_argp_doc(minitrc_fp, config_s, _("brackets_fg"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=#%06x"), _("brackets_bg"), sio->brackets_bg);
    print_argp_doc(minitrc_fp, config_s, _("brackets_bg"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=#%06x"), _("fill_char_fg"), sio->fill_char_fg);
    print_argp_doc(minitrc_fp, config_s, _("fill_char_fg"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=#%06x"), _("fill_char_bg"), sio->fill_char_bg);
    print_argp_doc(minitrc_fp, config_s, _("fill_char_bg"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=#%06x"), _("ln_bg"), sio->ln_bg);
    print_argp_doc(minitrc_fp, config_s, _("ln_bg"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=#%06x"), _("ln_fg"), sio->ln_fg);
    print_argp_doc(minitrc_fp, config_s, _("ln_bg"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=#%06x"), _("cmdln_bg"), sio->cmdln_bg);
    print_argp_doc(minitrc_fp, config_s, _("cmdln_bg"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=#%06x"), _("cmdln_fg"), sio->cmdln_fg);
    print_argp_doc(minitrc_fp, config_s, _("cmdln_fg"));

    ssnprintf(config_s, MAXLEN - 1, _("%s=#%06x"), _("nt_fg"), sio->nt_fg);
    print_argp_doc(minitrc_fp, config_s, _("nt_fg"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=#%06x"), _("nt_bg"), sio->nt_bg);
    print_argp_doc(minitrc_fp, config_s, _("nt_bg"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=#%06x"), _("nt_hl_fg"), sio->nt_hl_fg);
    print_argp_doc(minitrc_fp, config_s, _("nt_hl_fg"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=#%06x"), _("nt_hl_bg"), sio->nt_hl_bg);
    print_argp_doc(minitrc_fp, config_s, _("nt_hl_bg"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=#%06x"), _("nt_rev_fg"), sio->nt_rev_fg);
    print_argp_doc(minitrc_fp, config_s, _("nt_rev_fg"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=#%06x"), _("nt_rev_bg"), sio->nt_rev_bg);
    print_argp_doc(minitrc_fp, config_s, _("nt_rev_bg"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=#%06x"), _("nt_hl_rev_fg"), sio->nt_hl_rev_fg);
    print_argp_doc(minitrc_fp, config_s, _("nt_hl_rev_fg"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=#%06x"), _("nt_hl_rev_bg"), sio->nt_hl_rev_bg);
    print_argp_doc(minitrc_fp, config_s, _("nt_hl_rev_bg"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=#%06x"), _("title_fg"), sio->title_fg);
    print_argp_doc(minitrc_fp, config_s, _("title_fg"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=#%06x"), _("title_bg"), sio->title_bg);
    print_argp_doc(minitrc_fp, config_s, _("title_bg"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=#%06x"), _("ran_fg"), sio->ran_fg);
    print_argp_doc(minitrc_fp, config_s, _("ran_fg"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=#%06x"), _("ran_bg"), sio->ran_bg);
    print_argp_doc(minitrc_fp, config_s, _("ran_bg"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=%0.2f"), _("blue_gamma"), sio->blue_gamma);
    print_argp_doc(minitrc_fp, config_s, _("blue_gamma"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=%0.2f"), _("gray_gamma"), sio->gray_gamma);
    print_argp_doc(minitrc_fp, config_s, _("gray_gamma"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=%0.2f"), _("green_gamma"), sio->green_gamma);
    print_argp_doc(minitrc_fp, config_s, _("green_gamma"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=%0.2f"), _("red_gamma"), sio->red_gamma);
    print_argp_doc(minitrc_fp, config_s, _("red_gamma"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=#%06x"), _("black"), sio->black);
    print_argp_doc(minitrc_fp, config_s, _("black"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=#%06x"), _("red"), sio->red);
    print_argp_doc(minitrc_fp, config_s, _("red"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=#%06x"), _("green"), sio->green);
    print_argp_doc(minitrc_fp, config_s, _("green"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=#%06x"), _("yellow"), sio->yellow);
    print_argp_doc(minitrc_fp, config_s, _("yellow"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=#%06x"), _("blue"), sio->blue);
    print_argp_doc(minitrc_fp, config_s, _("blue"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=#%06x"), _("magenta"), sio->magenta);
    print_argp_doc(minitrc_fp, config_s, _("magenta"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=#%06x"), _("cyan"), sio->cyan);
    print_argp_doc(minitrc_fp, config_s, _("cyan"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=#%06x"), _("white"), sio->white);
    print_argp_doc(minitrc_fp, config_s, _("white"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=#%06x"), _("bblack"), sio->bblack);
    print_argp_doc(minitrc_fp, config_s, _("bblack"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=#%06x"), _("bred"), sio->bred);
    print_argp_doc(minitrc_fp, config_s, _("bred"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=#%06x"), _("bgreen"), sio->bgreen);
    print_argp_doc(minitrc_fp, config_s, _("bgreen"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=#%06x"), _("byellow"), sio->byellow);
    print_argp_doc(minitrc_fp, config_s, _("byellow"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=#%06x"), _("bblue"), sio->bblue);
    print_argp_doc(minitrc_fp, config_s, _("bblue"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=#%06x"), _("bmagenta"), sio->bmagenta);
    print_argp_doc(minitrc_fp, config_s, _("bmagenta"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=#%06x"), _("bcyan"), sio->bcyan);
    print_argp_doc(minitrc_fp, config_s, _("bcyan"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=#%06x"), _("bwhite"), sio->bwhite);
    print_argp_doc(minitrc_fp, config_s, _("bwhite"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=%s"), _("mapp_data"), init->mapp_data);
    print_argp_doc(minitrc_fp, config_s, _("mapp_data"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=%s"), _("mapp_help"), init->mapp_help);
    print_argp_doc(minitrc_fp, config_s, _("mapp_help"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=%s"), _("mapp_home"), init->mapp_home);
    print_argp_doc(minitrc_fp, config_s, _("mapp_home"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=%s"), _("mapp_msrc"), init->mapp_msrc);
    print_argp_doc(minitrc_fp, config_s, _("mapp_msrc"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=%s"), _("mapp_user"), init->mapp_user);
    print_argp_doc(minitrc_fp, config_s, _("mapp_user"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=%s"), _("mapp_theme"), init->mapp_theme);
    print_argp_doc(minitrc_fp, config_s, _("mapp_theme"));
    ssnprintf(config_s, MAXLEN - 1, _("%s=%s"), _("include"), init->mapp_theme);
    (void)fprintf(minitrc_fp, "%-34s # default theme file\n", config_s);
    strnz__cpy(tmp_str, _("Configuration written to file: "), MAXLEN - 1);
    strnz__cat(tmp_str, minitrc_dmp, MAXLEN - 1);
    ui_perror(tmp_str);
    return 0;
}
void print_argp_doc(FILE *minitrc_fp, char *config_s, char *key) {
    char comment[MAXLEN];
    comment[0] = '\0';
    if (get_argp_doc_by_name(comment, options, key))
        (void)fprintf(minitrc_fp, "%-34s # %s\n", config_s, comment);
    else
        (void)fprintf(minitrc_fp, "%-34s #\n", config_s);
}
/** @brief Derive full file specification from directory and file name
    @ingroup init
    @param file_spec - output full file specification
    @param dir - directory path
    @param file_name - file name
    @returns true if file_spec is derived, false otherwise
    @details If dir is nullptr, use MAPP_DIR environment variable or default
   directory, ~/menuapp.
    file_spec should be a pre-allocated char array of size MAXLEN to
   hold the resulting file specification
 */
bool derive_file_spec(char *file_spec, char *dir, char *file_name) {
    char tmp_str[MAXLEN];
    char ts2[MAXLEN];
    char *e;

    if (!file_name || !*file_name) {
        *file_spec = '\0';
        return false;
    }
    if (dir) {
        strnz__cpy(tmp_str, dir, MAXLEN - 1);
    } else {
        e = getenv(_("MAPP_DIR"));
        if (e) {
            strnz__cpy(tmp_str, e, MAXLEN - 1);
        } else {
            strnz__cpy(tmp_str, _("~/menuapp"), MAXLEN - 1);
        }
    }
    trim_path(tmp_str);
    strnz__cpy(ts2, tmp_str, MAXLEN - 1);
    // construct the full file specification
    // check that the file exists and is readable
    strnz__cpy(file_spec, ts2, MAXLEN - 1);
    strnz__cat(file_spec, "/", MAXLEN - 1);
    strnz__cat(file_spec, file_name, MAXLEN - 1);
    return true;
}
/** @brief Display the version information of the application
    @ingroup init
    @details The version information is defined in the mapp_version variable
   and is printed to stdout when this function is called. */
void display_version() {
    fprintf(stdout, "\nC-Menu %s\n", CM_VERSION);
    fprintf(stdout, "\nC-Menu %s\n", CM_VERSION);
    fprintf(stdout, "C version: %ld\n", __STDC_VERSION__);
}
/** @brief Print an option and its value in a formatted manner
    @ingroup init
    @param o - option flag (e.g., _("-a:"))
    @param name - option name (e.g., _("--minitrc"))
    @param value - option value to print
    @details This function is used to display the current configuration options
   and their values in a readable format. */
void opt_prt_char(const char *o, const char *name, const char *value) {
    fprintf(stdout, "%3s %-15s: %s\n", o, name, value);
}
/** @brief Print an option and its value in a formatted manner for integer
   values
    @ingroup init
    @param o - option flag (e.g., _("-C:"))
    @param name - option name (e.g., _("--cols"))
    @param value - integer option value to print
    @details This function is used to display the current configuration options
   and their integer values in a readable format. */
void opt_prt_str(const char *o, const char *name, const char *value) {
    fprintf(stdout, "%3s %-15s: %s\n", o, name, value);
}
/** @brief Print an option and its value in a formatted manner for integer
   values
    @ingroup init
    @param o - option flag (e.g., _("-C:"))
    @param name - option name (e.g., _("--cols"))
    @param value - integer option value to print
    @details This function is used to display the current configuration options
   and their integer values in a readable format. */
void opt_prt_int(const char *o, const char *name, int value) {
    fprintf(stdout, "%3s %-15s: %d\n", o, name, value);
}
/** @brief Print an option and its value in a formatted manner for double
   values
    @ingroup init
    @param o - option flag (e.g., _("-r:"))
    @param name - option name (e.g., _("red_gamma"))
    @param value - double option value to print
    @details This function is used to display the current configuration options
   and their double values in a readable format. */
void opt_prt_double(const char *o, const char *name, double value) {
    fprintf(stdout, "%3s %-15s: %0.2f\n", o, name, value);
}
/** @brief Print an option and its value in a formatted manner for boolean
   values
    @ingroup init
    @param o - option flag (e.g., _("-z"))
    @param name - option name (e.g., _("f_squeeze"))
    @param value - boolean option value to print
    @details This function is used to display the current configuration options
   and their boolean values in a readable format, printing _("true") or _("false")
   based on the value. */
void opt_prt_bool(const char *o, const char *name, bool value) {
    fprintf(stdout, "%3s %-15s: %s\n_(", o, name, value ? ")true_(" : ")false");
}
