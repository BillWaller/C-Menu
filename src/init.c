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
#include <time.h>
#include <unistd.h>
#include <wchar.h>

typedef enum {
    BG = 257,
    FG,
    BOX_FG,
    BOX_BG,
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

Init *init = nullptr;
void mapp_initialization(Init *, int, char **);
void zero_opt_args(Init *);
int process_config_files(Init *);
int process_config_file(char *, Init *);
int parse_opt_args(Init *, int, char **);

bool derive_file_spec(char *, char *, char *);
void print_argp_doc(FILE *, char *, char *);
int executor = 0;

const char *argp_program_version = CM_VERSION;
const char *argp_program_bug_address = "billxwaller@gmail.com";
static char doc[] = "C-Menu - User Interface Toolkit";
static char args_doc[] = "[INPUT] [OUTPUT] [HELP] [ARG4] [ARG5]";
// const int opt_doc_col = 33;

static struct argp_option options[] = {
    {"f_write_config", 'W', 0, OPTION_ARG_OPTIONAL, "write configuration", 0},
    {"minitrc", 'a', "file_spec", 0, "configuration file spec", 1},
    {"parent_cmd", 'k', 0, 0, "parent command", 1},
    {"begx", 'X', "number", 0, "begin on column", 2},
    {"begy", 'Y', "number", 0, "begin on line", 2},
    {"cols", 'C', "number", 0, "width in columns", 2},
    {"lines", 'L', "number", 0, "height in lines", 2},
    {"title", 'T', "text", 0, "Window title", 2},
    {"out_spec", 'o', "file_spec", 0, "output file spec", 3},
    {"cmd", 'c', "file_spec", 0, "view cmd, first file", 3},
    {"cmd_all", 'A', "file_spec", 0, "view cmd, all files", 3},
    {"help_spec", 'H', "file_spec", 0, "help file spec", 3},
    {"in_spec", 'i', "file_spec", 0, "input file spec", 3},
    {"mapp_spec", 'd', "file_spec", 0, "description file spec", 3},
    {"provider_cmd", 'S', "file_spec", 0, "execute provider of piped input", 3},
    {"receiver_cmd", 'R', "file_spec", 0, "execute receiver of piped output", 3},
    {"wait_timeout", 'w', "seconds", 0, "Wait timer", 5},
    {"select_max", 'n', "number", 0, "number of selections", 5},
    {"f_erase_remainder", 'e', "bool", OPTION_ARG_OPTIONAL, "erase remainder of line on enter", 5},
    {"f_strip_ansi", 'j', "bool", OPTION_ARG_OPTIONAL, "always strip ansi when writing", 5},
    {"f_multiple_cmd_args", 'M', "bool", OPTION_ARG_OPTIONAL, "allow multiple command arguments", 5},
    {"f_read_theme", 'r', "bool", OPTION_ARG_OPTIONAL, "read and process theme file", 5},
    {"f_squeeze", 's', "bool", OPTION_ARG_OPTIONAL, "squeeze multiple blank lines", 5},
    {"f_ignore_case", 'x', "bool", OPTION_ARG_OPTIONAL, "ignore case in search", 5},
    {"f_ln", 'N', "bool", OPTION_ARG_OPTIONAL, "line numbers in view", 5},
    {"fill_char", 'f', "char", 0, "field fill_char (_,.,empty)", 5},
    {"brackets", 'u', "text", 0, "brackets around fields ([]{}<>)", 5},
    {"editor", CM_EDITOR, "text", 0, "default editor", 5},
    {"tab_stop", 't', "number", 0, "number of spaces per tab (4)", 5},
    {"h_shift", 'z', "number", 0, "horizontal shift width (16)", 5},
    {"bg", BG, "hex_clr", 0, "Terminal (stdscr) background (#000000)", 6},

    {"fg", FG, "hex_clr", 0, "Terminal (stdscr) foreground (#d0d0d0)", 6},

    {"box_fg", BOX_FG, "hex_clr", 0, "box foreground (#d0d0d0)", 6},

    {"box_bg", BOX_BG, "hex_clr", 0, "box background (#000000)", 6},

    {"brackets_fg", BRACKETS_FG, "hex_clr", 0, "brackets foreground (#d0d0d0)", 6},

    {"brackets_bg", BRACKETS_BG, "hex_clr", 0, "brackets background (#000000)", 6},

    {"fill_char_fg", FILL_CHAR_FG, "hex_clr", 0, "fill character foreground (#d0d0d0)", 6},

    {"fill_char_bg", FILL_CHAR_BG, "hex_clr", 0, "fill character background (#000000)", 6},

    {"nt_fg", NT_FG, "hex_clr", 0, "normal text foreground (#d0d0d0)", 6},

    {"nt_bg", NT_BG, "hex_clr", 0, "normal text background (#000000)", 6},

    {"nt_rev_fg", NT_REV_FG, "hex_clr", 0, "normal text reverse foreground (#000000)", 6},

    {"nt_rev_bg", NT_REV_BG, "hex_clr", 0, "normal text reverse background (#d0d0d0)", 6},

    {"nt_hl_fg", NT_HL_FG, "hex_clr", 0, "normal text highlight foreground (#ffffff)", 6},

    {"nt_hl_bg", NT_HL_BG, "hex_clr", 0, "normal text highlight background (#000000)", 6},

    {"nt_hl_rev_fg", NT_HL_REV_FG, "hex_clr", 0, "normal text highlight reverse foreground (#f00000)", 6},

    {"nt_hl_rev_bg", NT_HL_REV_BG, "hex_clr", 0, "normal text highlight reverse background (#d0d0d0)", 6},

    {"ln_fg", LN_FG, "hex_clr", 0, "line number foreground (#0000b0)", 6},

    {"ln_bg", LN_BG, "hex_clr", 0, "line number background (#202020)", 6},

    {"title_fg", TITLE_FG, "hex_clr", 0, "title foreground (#d0d0d0)", 6},

    {"title_bg", TITLE_BG, "hex_clr", 0, "title background (#000000)", 6},

    {"blue_gamma", GM_BLUE, "float", 0, "blue_gamma (1.2)", 7},
    {"gray_gamma", GM_GRAY, "float", 0, "gray gamma (1.2)", 7},
    {"green_gamma", GM_GREEN, "float", 0, "green gamma (1.2)", 7},
    {"red_gamma", GM_RED, "float", 0, "red gamma (View)", 7},

    {"black", XBLACK, "hex_clr", 0, "black (#000000)", 8},
    {"red", XRED, "hex_clr", 0, "red (#bf0000)", 8},
    {"green", XGREEN, "hex_clr", 0, "green (#00cf00)", 8},
    {"yellow", XYELLOW, "hex_clr", 0, "yellow (#efbf00)", 8},
    {"blue", XBLUE, "hex_clr", 0, "blue (#0000FF)", 8},
    {"magenta", XMAGENTA, "hex_clr", 0, "magenta (#9f009f)", 8},
    {"cyan", XCYAN, "hex_clr", 0, "cyan (#00dfdf)", 8},
    {"white", XWHITE, "hex_clr", 0, "white (#d0d0d0)", 8},
    {"bblack", XBBLACK, "hex_clr", 0, "bright black (#7f7f7f)", 8},
    {"bred", XBRED, "hex_clr", 0, "bright red (#FF3737)", 8},
    {"bgreen", XBGREEN, "hex_clr", 0, "bright green (#00FF7f)", 8},
    {"byellow", XBYELLOW, "hex_clr", 0, "bright yellow (#FFeF00)", 8},
    {"bblue", XBBLUE, "hex_clr", 0, "bright blue (#00cfFF)", 8},
    {"bmagenta", XMAGENTA, "hex_clr", 0, "bright magenta (#FF00FF)", 8},
    {"bcyan", XBCYAN, "hex_clr", 0, "bright cyan (#00FFFF)", 8},
    {"bwhite", XBWHITE, "hex_clr", 0, "bright white (#FFFFFF)", 8},
    {"mapp_data", MAPP_DATA, "directory", 0, "data directory", 9},
    {"mapp_help", MAPP_HELP, "directory", 0, "help directory", 9},
    {"mapp_home", MAPP_HOME, "directory", 0, "home directory", 9},
    {"mapp_msrc", MAPP_MSRC, "directory", 0, "source directory", 9},
    {"mapp_user", MAPP_USER, "directory", 0, "user directory", 9},
    {"mapp_theme", MAPP_THEME, "file", 0, "default theme file", 9},
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
        init->f_ln = true;
        if (arg)
            init->f_ln = str_to_bool(arg);
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
    case 'z':
        init->h_shift = atoi(arg);
        if (init->h_shift < 1)
            init->h_shift = 1;
        break;
    case 'u':
        strnz__cpy(init->brackets, arg, 2);
        break;
    case 'w':
        wait_timeout = atoi(arg);
        wait_timeout = min(wait_timeout, 1);
        wait_timeout = max(wait_timeout, 29);
        break;
    case 'x':
        init->f_ignore_case = true;
        if (arg)
            init->f_ignore_case = str_to_bool(arg);
        break;
    case BG:
        strnz__cpy(sio->bg, arg, MAXLEN - 1);
        break;
    case FG:
        strnz__cpy(sio->fg, arg, MAXLEN - 1);
        break;
    case BOX_FG:
        strnz__cpy(sio->box_fg, arg, MAXLEN - 1);
        break;
    case BOX_BG:
        strnz__cpy(sio->box_bg, arg, MAXLEN - 1);
        break;
    case BRACKETS_FG:
        strnz__cpy(sio->brackets_fg, arg, MAXLEN - 1);
        break;
    case BRACKETS_BG:
        strnz__cpy(sio->brackets_bg, arg, MAXLEN - 1);
        break;
    case FILL_CHAR_FG:
        strnz__cpy(sio->fill_char_fg, arg, MAXLEN - 1);
        break;
    case FILL_CHAR_BG:
        strnz__cpy(sio->fill_char_bg, arg, MAXLEN - 1);
        break;
    case NT_FG:
        strnz__cpy(sio->nt_fg, arg, MAXLEN - 1);
        break;
    case NT_BG:
        strnz__cpy(sio->nt_bg, arg, MAXLEN - 1);
        break;
    case NT_REV_FG:
        strnz__cpy(sio->nt_rev_fg, arg, MAXLEN - 1);
        break;
    case NT_REV_BG:
        strnz__cpy(sio->nt_rev_bg, arg, MAXLEN - 1);
        break;
    case NT_HL_FG:
        strnz__cpy(sio->nt_hl_fg, arg, MAXLEN - 1);
        break;
    case NT_HL_BG:
        strnz__cpy(sio->nt_hl_bg, arg, MAXLEN - 1);
        break;
    case NT_HL_REV_FG:
        strnz__cpy(sio->nt_hl_rev_fg, arg, MAXLEN - 1);
        break;
    case NT_HL_REV_BG:
        strnz__cpy(sio->nt_hl_rev_bg, arg, MAXLEN - 1);
        break;
    case TITLE_FG:
        strnz__cpy(sio->title_fg, arg, MAXLEN - 1);
        break;
    case TITLE_BG:
        strnz__cpy(sio->title_bg, arg, MAXLEN - 1);
        break;
    case LN_FG:
        strnz__cpy(sio->ln_fg, arg, MAXLEN - 1);
        break;
    case LN_BG:
        strnz__cpy(sio->ln_bg, arg, MAXLEN - 1);
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
    setlocale(LC_ALL, "en_US.UTF-8");
    SIO *sio = init->sio;
    if (!init) {
        ssnprintf(tmp_str, sizeof(tmp_str), "%s",
                  "init struct not allocated on entry");
        abend(-1, tmp_str);
        exit(-1);
    }
#ifdef DEBUG_LOG
    open_cmenu_log();
#endif
    e = getenv("CMENU_HOME");
    if (!e || *e == '\0')
        strnz__cpy(init->mapp_home, "~/menuapp", MAXLEN);
    else
        strnz__cpy(init->mapp_home, e, MAXLEN);

    if (init->mapp_home[0] != '\0') {
        expand_tilde(init->mapp_home, MAXLEN - 1);
        if (!verify_dir(init->mapp_home, R_OK))
            abend(-1, "MAPP_HOME directory invalid");
    }
    // CMENU_RC should be an absolute path
    e = getenv("CMENU_RC");
    if (!e || *e == '\0') {
        strnz__cpy(init->minitrc, init->mapp_home, MAXLEN - 1);
        strnz__cat(init->minitrc, "/.minitrc", MAXLEN);
    } else
        strnz__cpy(init->minitrc, e, MAXLEN);
    if (init->mapp_user[0] == '\0') {
        strnz__cpy(init->mapp_user, init->mapp_home, MAXLEN - 1);
        strnz__cat(init->mapp_user, "/user", MAXLEN - 1);
    }
    if (init->mapp_theme[0] == '\0') {
        strnz__cpy(init->mapp_theme, init->mapp_home, MAXLEN - 1);
        strnz__cat(init->mapp_theme, "/themes/default", MAXLEN - 1);
    }
    if (init->mapp_msrc[0] == '\0') {
        strnz__cpy(init->mapp_msrc, init->mapp_home, MAXLEN - 1);
        strnz__cat(init->mapp_msrc, "/msrc", MAXLEN - 1);
    }
    if (init->mapp_data[0] == '\0') {
        strnz__cpy(init->mapp_data, init->mapp_home, MAXLEN - 1);
        strnz__cat(init->mapp_data, "/data", MAXLEN - 1);
    }
    if (init->mapp_help[0] == '\0') {
        strnz__cpy(init->mapp_help, init->mapp_home, MAXLEN - 1);
        strnz__cat(init->mapp_help, "/help", MAXLEN - 1);
    }
    init->mapp_spec[0] = '\0'; /**< menu specification file */
    // Set default colors and settings in SIO struct
    // These can be overridden by the config file or command-line options
    // Included here to ensure SIO has valid defaults even if config parsing fails
    strnz__cpy(sio->bg, "#000000",
               COLOR_LEN - 1); /**< background color */
    strnz__cpy(sio->fg, "#c0c0c0",
               COLOR_LEN - 1);                               /**< foreground color */
    strnz__cpy(sio->box_fg, "#f00000", COLOR_LEN - 1);       /**< bold color */
    strnz__cpy(sio->box_bg, "#000000", COLOR_LEN - 1);       /**< bold color */
    strnz__cpy(sio->title_fg, "#f0f0f0", COLOR_LEN - 1);     /**< title foreground color */
    strnz__cpy(sio->title_bg, "#000000", COLOR_LEN - 1);     /**< title background color */
    strnz__cpy(sio->nt_fg, "#c0c0c0", COLOR_LEN - 1);        /**< normal text foreground color */
    strnz__cpy(sio->nt_bg, "#000000", COLOR_LEN - 1);        /**< normal text background color */
    strnz__cpy(sio->nt_rev_fg, "#000000", COLOR_LEN - 1);    /**< normal text reverse foreground color */
    strnz__cpy(sio->nt_rev_bg, "#c0c0c0", COLOR_LEN - 1);    /**< normal text reverse background color */
    strnz__cpy(sio->nt_hl_fg, "#f00000", COLOR_LEN - 1);     /**< normal text highlight foreground color */
    strnz__cpy(sio->nt_hl_bg, "#000000", COLOR_LEN - 1);     /**< normal text highlight background color */
    strnz__cpy(sio->nt_hl_rev_fg, "#000000", COLOR_LEN - 1); /**< normal text reverse foreground color */
    strnz__cpy(sio->nt_hl_rev_bg, "#c0c0c0", COLOR_LEN - 1); /**< normal text reverse background color */
    strnz__cpy(sio->ln_fg, "#0070ff",
               COLOR_LEN - 1); /**< line number olor */
    strnz__cpy(sio->ln_bg, "#101010",
               COLOR_LEN - 1);                    /**< line number background */
    init->f_erase_remainder = true;               /**< erase remainder on enter */
    init->brackets[0] = '\0';                     /**< field enclosure brackets */
    strnz__cpy(init->fill_char, " ", MAXLEN - 1); /**< field fill character */
    e = getenv("TERM");
    if (e == nullptr || *e == '\0')
        strnz__cpy(term, "xterm-256color", MAXLEN);
    else
        strnz__cpy(term, e, MAXLEN - 1);
    e = getenv("EDITOR");
    if (e && *e != '\0')
        strnz__cpy(init->editor, "vi", MAXLEN - 1);
    else
        strnz__cpy(init->editor, e, MAXLEN - 1);
    process_config_files(init);
    init->mapp_spec[0] = '\0';
    init->argc = argc;
    argp_parse(&argp, argc, argv, 0, 0, init);
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
    Record structure is "parse key=value pairs".
    Skip lines without '='.
    Set init struct values based on key.
    Skip unknown keys. */
int process_config_files(Init *init) {
    char config_file_name[MAXLEN];
    int rc;
    if (!init->minitrc[0]) {
        char *e = getenv("MINITRC");
        if (e)
            strnz__cpy(init->minitrc, e, MAXLEN - 1);
        else
            strnz__cpy(init->minitrc, "~/.minitrc", MAXLEN - 1);
    }

    expand_tilde(init->minitrc, MAXLEN - 1);
    strnz__cpy(config_file_name, init->minitrc, MAXLEN - 1);
    rc = process_config_file(config_file_name, init);
    return rc;
}

int process_config_file(char *config_file_name, Init *init) {
    char include_file_name[MAXLEN];
    char ts[MAXLEN];
    char *sp, *dp;
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
    while (fgets(ts, sizeof(ts), config_fp)) {
        bool inquotes = false;
        if (ts[0] == '#')
            continue;
        sp = ts;
        key[0] = '\0';
        dp = key;
        // copy delimited by "=" into value
        while (*sp != '\0') {
            if (*sp == '\n')
                *dp = *sp = '\0';
            if (*sp == '=') {
                *dp = '\0';
                sp++;
                break;
            }
            if (*sp == '"' && *sp == ' ') {
                sp++;
                continue;
            }
            *dp++ = *sp++;
        }
        value[0] = '\0';
        dp = value;
        // copy delimited by newline or unquoted "#" into value, removing quotes, spaces, semicolons, and newlines, but respecting quotes
        while (*sp != '\0') {
            if ((*sp == '"' || *sp == '\'') && (*(sp + 1) != '\\')) {
                if (!inquotes) {
                    inquotes = true;
                    quote_char = *sp++;
                } else if (*sp == quote_char) {
                    inquotes = false;
                    quote_char = '\0';
                }
            }
            if (!inquotes) {
                if (*sp == '#') {
                    if (unstr_hex_clr(hex_clr_str, sp)) {
                        strnz__cpy(value, hex_clr_str, MAXLEN - 1);
                        dp = value + strlen(value);
                    }
                    break;
                }
            }
            if (*sp == ' ' || *sp == ';') {
                sp++;
                continue;
            }
            if (*sp == '\n') {
                *sp = '\0';
                break;
            }
            *dp++ = *sp++;
        }
        *dp = '\0';
        if (key[0] == '\0')
            continue;
        if (value[0] == '\0')
            continue;
        if (!strcmp(key, "include")) {
            strnz__cpy(include_file_name, value, MAXLEN - 1);
            expand_tilde(include_file_name, MAXLEN - 1);
            process_config_file(include_file_name, init);
            continue;
        }
        if (!strcmp(key, "minitrc")) {
            strnz__cpy(init->minitrc, value, MAXLEN - 1);
            continue;
        }
        if (!strcmp(key, "lines")) {
            init->lines = atoi(value);
            continue;
        }
        if (!strcmp(key, "cols")) {
            init->cols = atoi(value);
            continue;
        }
        if (!strcmp(key, "begy")) {
            init->begy = atoi(value);
            continue;
        }
        if (!strcmp(key, "begx")) {
            init->begx = atoi(value);
            continue;
        }
        if (!strcmp(key, "f_ln")) {
            init->f_ln = str_to_bool(value);
            continue;
        }
        if (!strcmp(key, "f_at_end_remove")) {
            init->f_at_end_remove = str_to_bool(value);
            continue;
        }
        if (!strcmp(key, "f_erase_remainder")) {
            init->f_erase_remainder = str_to_bool(value);
            continue;
        }
        if (!strcmp(key, "brackets")) {
            strnz__cpy(init->brackets, value, 2);
            continue;
        }
        if (!strcmp(key, "fill_char")) {
            if (strlen(value) > 1)
                value[1] = '\0';
            if (wcwidth((int)value[0]) > 1)
                value[0] = '?';
            strnz__cpy(init->fill_char, value, 4);
            continue;
        }
        if (!strcmp(key, "f_ignore_case")) {
            init->f_ignore_case = str_to_bool(value);
            continue;
        }
        if (!strcmp(key, "f_read_theme")) {
            init->f_read_theme = str_to_bool(value);
            continue;
        }
        if (!strcmp(key, "f_squeeze")) {
            init->f_squeeze = str_to_bool(value);
            continue;
        }
        if (!strcmp(key, "f_strip_ansi")) {
            init->f_strip_ansi = str_to_bool(value);
            continue;
        }
        if (!strcmp(key, "f_multiple_cmd_args")) {
            init->f_multiple_cmd_args = str_to_bool(value);
            continue;
        }
        if (!strcmp(key, "select_max")) {
            init->select_max = atoi(value);
            continue;
        }
        if (!strcmp(key, "tab_stop")) {
            init->tab_stop = atoi(value);
            continue;
        }
        if (!strcmp(key, "h_shift")) {
            init->h_shift = atoi(value);
            continue;
        }
        if (!strcmp(key, "wait_timeout")) {
            wait_timeout = atoi(value);
            continue;
        }
        if (!strcmp(key, "title")) {
            strnz__cpy(init->title, value, MAXLEN - 1);
            continue;
        }
        if (!strcmp(key, "cmd")) {
            strnz__cpy(init->cmd, value, MAXLEN - 1);
            continue;
        }
        if (!strcmp(key, "cmd_all")) {
            strnz__cpy(init->cmd_all, value, MAXLEN - 1);
            continue;
        }
        if (!strcmp(key, "parent_cmd")) {
            strnz__cpy(init->parent_cmd, value, MAXLEN - 1);
            continue;
        }
        if (!strcmp(key, "provider_cmd")) {
            strnz__cpy(init->provider_cmd, value, MAXLEN - 1);
            continue;
        }
        if (!strcmp(key, "receiver_cmd")) {
            strnz__cpy(init->receiver_cmd, value, MAXLEN - 1);
            continue;
        }
        if (!strcmp(key, "editor")) {
            strnz__cpy(init->editor, value, MAXLEN - 1);
            continue;
        }
        if (!strcmp(key, "fg")) {
            strnz__cpy(sio->fg, value, COLOR_LEN - 1);
            continue;
        }
        if (!strcmp(key, "bg")) {
            strnz__cpy(sio->bg, value, COLOR_LEN - 1);
            continue;
        }
        if (!strcmp(key, "box_fg")) {
            strnz__cpy(sio->box_fg, value, COLOR_LEN - 1);
            continue;
        }
        if (!strcmp(key, "box_bg")) {
            strnz__cpy(sio->box_bg, value, COLOR_LEN - 1);
            continue;
        }
        if (!strcmp(key, "brackets_fg")) {
            strnz__cpy(sio->brackets_fg, value, COLOR_LEN - 1);
            continue;
        }
        if (!strcmp(key, "brackets_bg")) {
            strnz__cpy(sio->brackets_bg, value, COLOR_LEN - 1);
            continue;
        }
        if (!strcmp(key, "fill_char_fg")) {
            strnz__cpy(sio->fill_char_fg, value, COLOR_LEN - 1);
            continue;
        }
        if (!strcmp(key, "fill_char_bg")) {
            strnz__cpy(sio->fill_char_bg, value, COLOR_LEN - 1);
            continue;
        }
        if (!strcmp(key, "ln_fg")) {
            strnz__cpy(sio->ln_fg, value, COLOR_LEN - 1);
            continue;
        }
        if (!strcmp(key, "ln_bg")) {
            strnz__cpy(sio->ln_bg, value, COLOR_LEN - 1);
            continue;
        }
        if (!strcmp(key, "nt_fg")) {
            strnz__cpy(sio->nt_fg, value, COLOR_LEN - 1);
            continue;
        }
        if (!strcmp(key, "nt_bg")) {
            strnz__cpy(sio->nt_bg, value, COLOR_LEN - 1);
            continue;
        }
        if (!strcmp(key, "nt_rev_fg")) {
            strnz__cpy(sio->nt_rev_fg, value, COLOR_LEN - 1);
            continue;
        }
        if (!strcmp(key, "nt_rev_bg")) {
            strnz__cpy(sio->nt_rev_bg, value, COLOR_LEN - 1);
            continue;
        }
        if (!strcmp(key, "nt_hl_fg")) {
            strnz__cpy(sio->nt_hl_fg, value, COLOR_LEN - 1);
            continue;
        }
        if (!strcmp(key, "nt_hl_bg")) {
            strnz__cpy(sio->nt_hl_bg, value, COLOR_LEN - 1);
            continue;
        }
        if (!strcmp(key, "nt_hl_rev_fg")) {
            strnz__cpy(sio->nt_hl_rev_fg, value, COLOR_LEN - 1);
            continue;
        }
        if (!strcmp(key, "nt_hl_rev_bg")) {
            strnz__cpy(sio->nt_hl_rev_bg, value, COLOR_LEN - 1);
            continue;
        }
        if (!strcmp(key, "title_fg")) {
            strnz__cpy(sio->title_fg, value, COLOR_LEN - 1);
            continue;
        }
        if (!strcmp(key, "title_bg")) {
            strnz__cpy(sio->title_bg, value, COLOR_LEN - 1);
            continue;
        }
        if (!strcmp(key, "red_gamma")) {
            sio->red_gamma = str_to_double(value);
            continue;
        }
        if (!strcmp(key, "green_gamma")) {
            sio->green_gamma = str_to_double(value);
            continue;
        }
        if (!strcmp(key, "blue_gamma")) {
            sio->blue_gamma = str_to_double(value);
            continue;
        }
        if (!strcmp(key, "gray_gamma")) {
            sio->gray_gamma = str_to_double(value);
            continue;
        }
        if (!strcmp(key, "black")) {
            strnz__cpy(sio->black, value, COLOR_LEN - 1);
            continue;
        }
        if (!strcmp(key, "red")) {
            strnz__cpy(sio->red, value, COLOR_LEN - 1);
            continue;
        }
        if (!strcmp(key, "green")) {
            strnz__cpy(sio->green, value, COLOR_LEN - 1);
            continue;
        }
        if (!strcmp(key, "yellow")) {
            strnz__cpy(sio->yellow, value, COLOR_LEN - 1);
            continue;
        }
        if (!strcmp(key, "blue")) {
            strnz__cpy(sio->blue, value, COLOR_LEN - 1);
            continue;
        }
        if (!strcmp(key, "magenta")) {
            strnz__cpy(sio->magenta, value, COLOR_LEN - 1);
            continue;
        }
        if (!strcmp(key, "cyan")) {
            strnz__cpy(sio->cyan, value, COLOR_LEN - 1);
            continue;
        }
        if (!strcmp(key, "white")) {
            strnz__cpy(sio->white, value, COLOR_LEN - 1);
            continue;
        }
        if (!strcmp(key, "orange")) {
            strnz__cpy(sio->orange, value, COLOR_LEN - 1);
            continue;
        }
        if (!strcmp(key, "bblack")) {
            strnz__cpy(sio->bblack, value, COLOR_LEN - 1);
            continue;
        }
        if (!strcmp(key, "bred")) {
            strnz__cpy(sio->bred, value, COLOR_LEN - 1);
            continue;
        }
        if (!strcmp(key, "bgreen")) {
            strnz__cpy(sio->bgreen, value, COLOR_LEN - 1);
            continue;
        }
        if (!strcmp(key, "byellow")) {
            strnz__cpy(sio->byellow, value, COLOR_LEN - 1);
            continue;
        }
        if (!strcmp(key, "bblue")) {
            strnz__cpy(sio->bblue, value, COLOR_LEN - 1);
            continue;
        }
        if (!strcmp(key, "bmagenta")) {
            strnz__cpy(sio->bmagenta, value, COLOR_LEN - 1);
            continue;
        }
        if (!strcmp(key, "bcyan")) {
            strnz__cpy(sio->bcyan, value, COLOR_LEN - 1);
            continue;
        }
        if (!strcmp(key, "bwhite")) {
            strnz__cpy(sio->bwhite, value, COLOR_LEN - 1);
            continue;
        }
        if (!strcmp(key, "borange")) {
            strnz__cpy(sio->borange, value, COLOR_LEN - 1);
            continue;
        }
        if (!strcmp(key, "mapp_spec")) {
            strnz__cpy(init->mapp_spec, value, MAXLEN - 1);
            continue;
        }
        if (!strcmp(key, "mapp_data")) {
            strnz__cpy(init->mapp_data, value, MAXLEN - 1);
            continue;
        }
        if (!strcmp(key, "mapp_help")) {
            strnz__cpy(init->mapp_help, value, MAXLEN - 1);
            continue;
        }
        if (!strcmp(key, "mapp_home")) {
            strnz__cpy(init->mapp_home, value, MAXLEN - 1);
            continue;
        }
        if (!strcmp(key, "mapp_msrc")) {
            strnz__cpy(init->mapp_msrc, value, MAXLEN - 1);
            continue;
        }
        if (!strcmp(key, "mapp_user")) {
            strnz__cpy(init->mapp_user, value, MAXLEN - 1);
            continue;
        }
        if (!strcmp(key, "mapp_theme")) {
            strnz__cpy(init->mapp_theme, value, MAXLEN - 1);
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
    e = getenv("CMENU_HOME");
    minitrc_dmp[0] = '\0';
    char config_s[MAXLEN];
    if (e) {
        strnz__cpy(minitrc_dmp, e, MAXLEN - 1);
        strnz__cat(minitrc_dmp, "/", MAXLEN - 1);
    }
    strnz__cat(minitrc_dmp, "minitrc.dmp", MAXLEN - 1);
    FILE *minitrc_fp = fopen(minitrc_dmp, "w");
    if (minitrc_fp == (FILE *)0) {
        fprintf(stderr, "failed to open file: %s\n", minitrc_dmp);
        return (-1);
    }
    (void)fprintf(minitrc_fp, "# %s\n", minitrc_dmp);
    char *doc[50] = {
        "# C-Menu example configuration file",
        "#",
        "# This file is generated by C-Menu when run with the -W option.",
        "# Copy this file and edit the copy because this file will be",
        "# overwritten each time C-Menu is run with the - W option.",
        "#",
        "# C-Menu processes key value pairs in reading order from its",
        "# main configuraiton file, ~/ menuapp /.minitrc, and any other",
        "# configuration files sourced with include statements such as",
        "# the following:",
        "#",
        "# include = ~/menuapp/theme/default",
        "#",
        "# Assuming your configuration file is in ~/menuapp/theme/Red,",
        "# you could create a symbolic link named default that points to",
        "# Red, and then include default in your main configuration file.",
        "# (Actually ~/menuapp/.minitrc already includes default, so you",
        "# would only need to create the theme file and the symbolic link.",
        "#",
        "# ln -s Red default",
        "#",
        "# Key value pairs included from configuration files are",
        "# processed in reading order as they are included.",
        "#",
        "# This is significant in the event that a key is included more",
        "# than once in the configuration file and / or included files.",
        "# Only the last value read for a key will be used by C-Menu.",
        "#",
        "# If you want to override key values in the C-Menu configuration",
        "# file, you can do so by inserting an include statement for a",
        "# supplemental configuration file below the keys you want to",
        "# override in the C-Menu configuration file. Conversely, if you",
        "# want to use a supplemental configuration as the default,",
        "# include it first.",
        "#",
        "# Parsing: Lines beginning with # are comments and are ignored.",
        "# Lines containing key=value pairs are parsed and the key and",
        "# value are extracted. Lines without an '=' are ignored. Values",
        "# are stripped of leading and trailing whitespace and quotes.",
        "# Values can be enclosed in single or double quotes to preserve",
        "# leading and trailing whitespace. Values can also be specified",
        "# as hex color codes such as #ff0000 for red. If a value is",
        "# specified as a hex color code, it is parsed and stored as a",
        "# hex color code in the configuration. An unquoted '#' that is",
        "# not part of a six digit hex color code and after key values",
        "# have been extracted is the beginning of a comment.", ""};
    for (int i = 0; doc[i][0] != '\0'; i++)
        (void)fprintf(minitrc_fp, "%s\n", doc[i]);
    (void)fprintf(minitrc_fp, "#\n");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "parent_cmd", init->parent_cmd);
    print_argp_doc(minitrc_fp, config_s, "parent_cmd");
    ssnprintf(config_s, MAXLEN - 1, "%s=%d", "cols", init->cols);
    print_argp_doc(minitrc_fp, config_s, "cols");
    ssnprintf(config_s, MAXLEN - 1, "%s=%d", "lines", init->lines);
    print_argp_doc(minitrc_fp, config_s, "lines");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "title", init->title);
    print_argp_doc(minitrc_fp, config_s, "title");
    ssnprintf(config_s, MAXLEN - 1, "%s=%d", "begx", init->begx);
    print_argp_doc(minitrc_fp, config_s, "begx");
    ssnprintf(config_s, MAXLEN - 1, "%s=%d", "begy", init->begy);
    print_argp_doc(minitrc_fp, config_s, "begy");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "cmd_all", init->cmd_all);
    print_argp_doc(minitrc_fp, config_s, "cmd_all");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "cmd", init->cmd);
    print_argp_doc(minitrc_fp, config_s, "cmd");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "mapp_spec", init->mapp_spec);
    print_argp_doc(minitrc_fp, config_s, "mapp_spec");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "help_spec", init->help_spec);
    print_argp_doc(minitrc_fp, config_s, "help_spec");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "in_spec", init->in_spec);
    print_argp_doc(minitrc_fp, config_s, "in_spec");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "out_spec", init->out_spec);
    print_argp_doc(minitrc_fp, config_s, "out_spec");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "receiver_cmd", init->receiver_cmd);
    print_argp_doc(minitrc_fp, config_s, "receiver_cmd");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "provider_cmd", init->provider_cmd);
    print_argp_doc(minitrc_fp, config_s, "provider_cmd");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "f_erase_remainder", init->f_erase_remainder ? "true" : "false");
    print_argp_doc(minitrc_fp, config_s, "f_erase_remainder");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "fill_char", init->fill_char);
    print_argp_doc(minitrc_fp, config_s, "fill_char");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "f_strip_ansi", init->f_strip_ansi ? "true" : "false");
    print_argp_doc(minitrc_fp, config_s, "f_strip_ansi");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "f_multiple_cmd_args", init->f_multiple_cmd_args ? "true" : "false");
    print_argp_doc(minitrc_fp, config_s, "f_multiple_cmd_args");
    ssnprintf(config_s, MAXLEN - 1, "%s=%d", "select_max", init->select_max);
    print_argp_doc(minitrc_fp, config_s, "select_max");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "f_ln", init->f_ln ? "true" : "false");
    print_argp_doc(minitrc_fp, config_s, "f_ln");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "f_squeeze", init->f_squeeze ? "true" : "false");
    print_argp_doc(minitrc_fp, config_s, "f_squeeze");
    ssnprintf(config_s, MAXLEN - 1, "%s=%d", "tab_stop", init->tab_stop);
    print_argp_doc(minitrc_fp, config_s, "tab_stop");
    ssnprintf(config_s, MAXLEN - 1, "%s=%d", "h_shift", init->h_shift);
    print_argp_doc(minitrc_fp, config_s, "h_shift");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "brackets", init->brackets);
    print_argp_doc(minitrc_fp, config_s, "brackets");
    ssnprintf(config_s, MAXLEN - 1, "%s=%d", "wait_timeout", wait_timeout);
    print_argp_doc(minitrc_fp, config_s, "wait_timeout");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "f_ignore_case", init->f_ignore_case ? "true" : "false");
    print_argp_doc(minitrc_fp, config_s, "f_ignore_case");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "f_read_theme", init->f_read_theme ? "true" : "false");
    print_argp_doc(minitrc_fp, config_s, "f_read_theme");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "editor", init->editor);
    print_argp_doc(minitrc_fp, config_s, "editor");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "bg", sio->bg);
    print_argp_doc(minitrc_fp, config_s, "bg");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "fg", sio->fg);
    print_argp_doc(minitrc_fp, config_s, "fg");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "box_fg", sio->box_fg);
    print_argp_doc(minitrc_fp, config_s, "box_fg");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "box_bg", sio->box_bg);
    print_argp_doc(minitrc_fp, config_s, "box_bg");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "brackets_fg", sio->brackets_fg);
    print_argp_doc(minitrc_fp, config_s, "brackets_fg");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "brackets_bg", sio->brackets_bg);
    print_argp_doc(minitrc_fp, config_s, "brackets_bg");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "fill_char_fg", sio->fill_char_fg);
    print_argp_doc(minitrc_fp, config_s, "fill_char_fg");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "fill_char_bg", sio->fill_char_bg);
    print_argp_doc(minitrc_fp, config_s, "fill_char_bg");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "ln_bg", sio->ln_bg);
    print_argp_doc(minitrc_fp, config_s, "ln_bg");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "ln_fg", sio->ln_fg);
    print_argp_doc(minitrc_fp, config_s, "ln_fg");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "nt_fg", sio->nt_fg);
    print_argp_doc(minitrc_fp, config_s, "nt_fg");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "nt_bg", sio->nt_bg);
    print_argp_doc(minitrc_fp, config_s, "nt_bg");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "nt_hl_fg", sio->nt_hl_fg);
    print_argp_doc(minitrc_fp, config_s, "nt_hl_fg");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "nt_hl_bg", sio->nt_hl_bg);
    print_argp_doc(minitrc_fp, config_s, "nt_hl_bg");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "nt_rev_fg", sio->nt_rev_fg);
    print_argp_doc(minitrc_fp, config_s, "nt_rev_fg");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "nt_rev_bg", sio->nt_rev_bg);
    print_argp_doc(minitrc_fp, config_s, "nt_rev_bg");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "nt_hl_rev_fg", sio->nt_hl_rev_fg);
    print_argp_doc(minitrc_fp, config_s, "nt_hl_rev_fg");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "nt_hl_rev_bg", sio->nt_hl_rev_bg);
    print_argp_doc(minitrc_fp, config_s, "nt_hl_rev_bg");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "title_fg", sio->title_fg);
    print_argp_doc(minitrc_fp, config_s, "title_fg");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "title_bg", sio->title_bg);
    print_argp_doc(minitrc_fp, config_s, "title_bg");
    ssnprintf(config_s, MAXLEN - 1, "%s=%0.2f", "blue_gamma", sio->blue_gamma);
    print_argp_doc(minitrc_fp, config_s, "blue_gamma");
    ssnprintf(config_s, MAXLEN - 1, "%s=%0.2f", "gray_gamma", sio->gray_gamma);
    print_argp_doc(minitrc_fp, config_s, "gray_gamma");
    ssnprintf(config_s, MAXLEN - 1, "%s=%0.2f", "green_gamma", sio->green_gamma);
    print_argp_doc(minitrc_fp, config_s, "green_gamma");
    ssnprintf(config_s, MAXLEN - 1, "%s=%0.2f", "red_gamma", sio->red_gamma);
    print_argp_doc(minitrc_fp, config_s, "red_gamma");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "black", sio->black);
    print_argp_doc(minitrc_fp, config_s, "black");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "red", sio->red);
    print_argp_doc(minitrc_fp, config_s, "red");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "green", sio->green);
    print_argp_doc(minitrc_fp, config_s, "green");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "yellow", sio->yellow);
    print_argp_doc(minitrc_fp, config_s, "yellow");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "blue", sio->blue);
    print_argp_doc(minitrc_fp, config_s, "blue");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "magenta", sio->magenta);
    print_argp_doc(minitrc_fp, config_s, "magenta");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "cyan", sio->cyan);
    print_argp_doc(minitrc_fp, config_s, "cyan");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "white", sio->white);
    print_argp_doc(minitrc_fp, config_s, "white");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "bblack", sio->bblack);
    print_argp_doc(minitrc_fp, config_s, "bblack");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "bred", sio->bred);
    print_argp_doc(minitrc_fp, config_s, "bred");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "bgreen", sio->bgreen);
    print_argp_doc(minitrc_fp, config_s, "bgreen");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "byellow", sio->byellow);
    print_argp_doc(minitrc_fp, config_s, "byellow");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "bblue", sio->bblue);
    print_argp_doc(minitrc_fp, config_s, "bblue");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "bmagenta", sio->bmagenta);
    print_argp_doc(minitrc_fp, config_s, "bmagenta");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "bcyan", sio->bcyan);
    print_argp_doc(minitrc_fp, config_s, "bcyan");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "bwhite", sio->bwhite);
    print_argp_doc(minitrc_fp, config_s, "bwhite");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "mapp_data", init->mapp_data);
    print_argp_doc(minitrc_fp, config_s, "mapp_data");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "mapp_help", init->mapp_help);
    print_argp_doc(minitrc_fp, config_s, "mapp_help");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "mapp_home", init->mapp_home);
    print_argp_doc(minitrc_fp, config_s, "mapp_home");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "mapp_msrc", init->mapp_msrc);
    print_argp_doc(minitrc_fp, config_s, "mapp_msrc");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "mapp_user", init->mapp_user);
    print_argp_doc(minitrc_fp, config_s, "mapp_user");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "mapp_theme", init->mapp_theme);
    print_argp_doc(minitrc_fp, config_s, "mapp_theme");
    ssnprintf(config_s, MAXLEN - 1, "%s=%s", "include", init->mapp_theme);
    (void)fprintf(minitrc_fp, "%-34s # default theme file\n", config_s);
    strnz__cpy(tmp_str, "Configuration written to file: ", MAXLEN - 1);
    strnz__cat(tmp_str, minitrc_dmp, MAXLEN - 1);
    Perror(tmp_str);
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
    char ts[MAXLEN];
    char ts2[MAXLEN];
    char *e;

    if (!file_name || !*file_name) {
        *file_spec = '\0';
        return false;
    }
    if (dir) {
        strnz__cpy(ts, dir, MAXLEN - 1);
    } else {
        e = getenv("MAPP_DIR");
        if (e) {
            strnz__cpy(ts, e, MAXLEN - 1);
        } else {
            strnz__cpy(ts, "~/menuapp", MAXLEN - 1);
        }
    }
    trim_path(ts);
    strnz__cpy(ts2, ts, MAXLEN - 1);
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
    @param o - option flag (e.g., "-a:")
    @param name - option name (e.g., "--minitrc")
    @param value - option value to print
    @details This function is used to display the current configuration options
   and their values in a readable format. */
void opt_prt_char(const char *o, const char *name, const char *value) {
    fprintf(stdout, "%3s %-15s: %s\n", o, name, value);
}
/** @brief Print an option and its value in a formatted manner for integer
   values
    @ingroup init
    @param o - option flag (e.g., "-C:")
    @param name - option name (e.g., "--cols")
    @param value - integer option value to print
    @details This function is used to display the current configuration options
   and their integer values in a readable format. */
void opt_prt_str(const char *o, const char *name, const char *value) {
    fprintf(stdout, "%3s %-15s: %s\n", o, name, value);
}
/** @brief Print an option and its value in a formatted manner for integer
   values
    @ingroup init
    @param o - option flag (e.g., "-C:")
    @param name - option name (e.g., "--cols")
    @param value - integer option value to print
    @details This function is used to display the current configuration options
   and their integer values in a readable format. */
void opt_prt_int(const char *o, const char *name, int value) {
    fprintf(stdout, "%3s %-15s: %d\n", o, name, value);
}
/** @brief Print an option and its value in a formatted manner for double
   values
    @ingroup init
    @param o - option flag (e.g., "-r:")
    @param name - option name (e.g., "red_gamma")
    @param value - double option value to print
    @details This function is used to display the current configuration options
   and their double values in a readable format. */
void opt_prt_double(const char *o, const char *name, double value) {
    fprintf(stdout, "%3s %-15s: %0.2f\n", o, name, value);
}
/** @brief Print an option and its value in a formatted manner for boolean
   values
    @ingroup init
    @param o - option flag (e.g., "-z")
    @param name - option name (e.g., "f_squeeze")
    @param value - boolean option value to print
    @details This function is used to display the current configuration options
   and their boolean values in a readable format, printing "true" or "false"
   based on the value. */
void opt_prt_bool(const char *o, const char *name, bool value) {
    fprintf(stdout, "%3s %-15s: %s\n", o, name, value ? "true" : "false");
}
