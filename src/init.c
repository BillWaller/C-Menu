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

#include <common.h>
#include <getopt.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

enum {
    MAPP_MSRC = 257,
    MAPP_HELP,
    MAPP_HELP_DIR,
    MAPP_DATA_DIR,
    MAPP_SPEC,
    HELP_SPEC,
    IN_SPEC,
    OUT_SPEC
};

bool f_write_config = false;
bool f_dump_config = false;
bool f_help = false;
bool f_version = false;

int write_config(Init *init);
void display_version();

Init *init = NULL;
void mapp_initialization(Init *, int, char **);
int parse_opt_args(Init *, int, char **);
void zero_opt_args(Init *);
int parse_config(Init *);
void dump_config(Init *, char *);
void usage();

bool derive_file_spec(char *, char *, char *);
int executor = 0;

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

    e = getenv("CMENU_HOME");
    if (!e || *e == '\0')
        strnz__cpy(init->mapp_home, "~/menuapp", MAXLEN);
    else
        strnz__cpy(init->mapp_home, e, MAXLEN);
    e = getenv("CMENU_RC");
    if (!e || *e == '\0')
        strnz__cpy(init->minitrc, "~/menuapp/.minitrc", MAXLEN);
    else
        strnz__cpy(init->minitrc, e, MAXLEN);
    if (init->minitrc[0] == '\0')
        strnz__cpy(init->minitrc, "~/.minitrc", MAXLEN - 1);
    strnz__cpy(sio->bg_clr_x, "#000007",
               COLOR_LEN - 1); /**< background color */
    strnz__cpy(sio->fg_clr_x, "#c0c0c0",
               COLOR_LEN - 1); /**< foreground color */
    strnz__cpy(sio->bo_clr_x, "#f00000", COLOR_LEN - 1); /**< bold color */
    strnz__cpy(sio->ln_clr_x, "#0070ff",
               COLOR_LEN - 1); /**< line number olor */
    strnz__cpy(sio->ln_bg_clr_x, "#101010",
               COLOR_LEN - 1);      /**< line number background */
    init->f_erase_remainder = true; /**< erase remainder on enter */
    init->brackets[0] = '\0';       /**< field enclosure brackets */
    strnz__cpy(init->fill_char, "_", MAXLEN - 1); /**< field fill character */
    init->mapp_spec[0] = '\0'; /**< menu specification file */
    strnz__cpy(init->mapp_home, "~/menuapp", MAXLEN - 1);
    strnz__cpy(init->mapp_user, "~/menuapp/user", MAXLEN - 1);
    strnz__cpy(init->mapp_msrc, "~/menuapp/msrc", MAXLEN - 1);
    strnz__cpy(init->mapp_data, "~/menuapp/data", MAXLEN - 1);
    strnz__cpy(init->mapp_help, "~/menuapp/help", MAXLEN - 1);
    // Priority-4 - cfg_args

    e = getenv("TERM");
    if (*e == '\0')
        strnz__cpy(term, "xterm-256color", MAXLEN);
    else
        strnz__cpy(term, e, MAXLEN - 1);
    e = getenv("EDITOR");
    if (e && *e != '\0')
        strnz__cpy(init->editor, "vi", MAXLEN - 1);
    else
        strnz__cpy(init->editor, e, MAXLEN - 1);
    parse_config(init); /**< generally /home/user/.minitrc */
    parse_opt_args(init, argc, argv);
    if (f_dump_config) {
        dump_config(init, "Configuration after parse_config and "
                          "parse_opt_args");
        if (f_write_config) {
            write_config(init);
        }
        if (f_help) {
            dump_config(init, "Current Configuration");
        }
        if (f_version) {
            display_version();
        }
        if (init->mapp_home[0] != '\0') {
            expand_tilde(init->mapp_home, MAXLEN - 1);
            if (!verify_dir(init->mapp_home, R_OK))
                abend(-1, "MAPP_HOME directory invalid");
        }
    }
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
/** @brief Parse command-line options and set Init struct values accordingly
    @ingroup init
    @param init - pointer to Init struct
    @param argc - argument count
    @param argv - argument vector
    Return index of first non-option argument
    @note Accepts both short and long options
 */
int parse_opt_args(Init *init, int argc, char **argv) {
    int i;
    int opt;
    int longindex = 0;
    int flag = 0;
    char *optstring = "a:b:c:d:ef:g:hi:j:k:m:n:o:p:r:st:u:vw:xzA:B:C:DF:G:H:L:"
                      "MNO:P:Q:R:S:T:VWX:Y:Z";
    struct option long_options[] = {{"help", 0, &flag, MAPP_HELP},
                                    {"mapp_spec", 1, &flag, MAPP_SPEC},
                                    {"mapp_data", 1, &flag, MAPP_DATA_DIR},
                                    {"mapp_help", 1, &flag, MAPP_HELP_DIR},
                                    {"help_spec", 1, &flag, HELP_SPEC},
                                    {"mapp_spec", 1, &flag, MAPP_SPEC},
                                    {"in_spec", 1, &flag, IN_SPEC},
                                    {"out_spec", 1, &flag, OUT_SPEC},
                                    {"mapp_msrc", 1, &flag, MAPP_MSRC},
                                    {0, 0, 0, 0}};
    char mapp[MAXLEN];
    SIO *sio = init->sio;
    base_name(mapp, argv[0]);
    optind = 1;
    while ((opt = getopt_long(argc, argv, optstring, long_options,
                              &longindex)) != -1) {
        switch (opt) {
        case 0:
            switch (flag) {
            case MAPP_DATA_DIR:
                strnz__cpy(init->mapp_data, optarg, MAXLEN - 1);
                break;
            case MAPP_HELP_DIR:
                strnz__cpy(init->mapp_help, optarg, MAXLEN - 1);
                break;
            case MAPP_SPEC:
                strnz__cpy(init->mapp_spec, optarg, MAXLEN - 1);
                break;
            case HELP_SPEC:
                strnz__cpy(init->help_spec, optarg, MAXLEN - 1);
                break;
            case IN_SPEC:
                strnz__cpy(init->in_spec, optarg, MAXLEN - 1);
                break;
            case OUT_SPEC:
                strnz__cpy(init->out_spec, optarg, MAXLEN - 1);
                break;
            case MAPP_MSRC:
                strnz__cpy(init->mapp_msrc, optarg, MAXLEN - 1);
                break;
            default:
                break;
            }
            break;
        case 'a':
            strnz__cpy(init->minitrc, optarg, MAXLEN - 1);
            break;
        case 'b':
            sio->blue_gamma = str_to_double(optarg);
            break;
        case 'c':
            strnz__cpy(init->cmd, optarg, MAXLEN - 1);
            break;
        case 'd':
            strnz__cpy(init->mapp_spec, optarg, MAXLEN - 1);
            break;
        case 'e':
            init->f_erase_remainder = true;
            break;
        case 'f': // fill char
            strnz__cpy(init->fill_char, optarg, 1);
            break;
        case 'g':
            sio->green_gamma = str_to_double(optarg);
            break;
        case 'h':
            f_help = true;
            break;
        case 'i':
            strnz__cpy(init->in_spec, optarg, MAXLEN - 1);
            break;
        case 'j':
            init->f_strip_ansi = true;
            break;
        case 'k':
            strnz__cpy(init->parent_cmd, optarg, MAXLEN - 1);
            break;
        case 'm':
            strnz__cpy(init->mapp_home, optarg, MAXLEN - 1);
            break;
        case 'n':
            init->select_max = atoi(optarg);
            break;
        case 'o':
            strnz__cpy(init->out_spec, optarg, MAXLEN - 1);
            break;
        case 'r':
            sio->red_gamma = str_to_double(optarg);
            break;
        case 's':
            init->f_squeeze = true;
            break;
        case 't':
            init->tab_stop = atoi(optarg);
            if (init->tab_stop < 1)
                init->tab_stop = 1;
            break;
        case 'u': // brackets
            strnz__cpy(init->brackets, optarg, 2);
            break;
        case 'v':
        case 'V':
            f_version = true;
            break;
        case 'x':
            init->f_ignore_case = true;
            break;
        case 'y':
            init->f_at_end_remove = true;
            break;
        case 'A':
            strnz__cpy(init->cmd_all, optarg, MAXLEN - 1);
            break;
        case 'B':
            strnz__cpy(sio->bg_clr_x, optarg, MAXLEN - 1);
            break;
        case 'C':
            init->cols = atoi(optarg);
            break;
        case 'D':
            f_dump_config = true;
            break;
        case 'F':
            strnz__cpy(sio->fg_clr_x, optarg, MAXLEN - 1);
            break;
        case 'G':
            sio->gray_gamma = str_to_double(optarg);
            break;
        case 'H':
            strnz__cpy(init->help_spec, optarg, MAXLEN - 1);
            break;
        case 'L':
            init->lines = atoi(optarg);
            break;
        case 'M':
            init->f_multiple_cmd_args = true;
            break;
        case 'N':
            init->f_ln = true;
            break;
        case 'O':
            strnz__cpy(sio->bo_clr_x, optarg, MAXLEN - 1);
            break;
        case 'R':
            strnz__cpy(init->receiver_cmd, optarg, MAXLEN - 1);
            break;
        case 'S':
            strnz__cpy(init->provider_cmd, optarg, MAXLEN - 1);
            break;
        case 'T':
            strnz__cpy(init->title, optarg, MAXLEN - 1);
            break;
        case 'U':
            strnz__cpy(init->mapp_user, optarg, MAXLEN - 1);
            break;
        case 'W':
            f_write_config = true;
            break;
        case 'X':
            init->begx = atoi(optarg);
            break;
        case 'Y':
            init->begy = atoi(optarg);
            break;
        default:
            break;
        }
    }
    init->argv[0] = strdup(argv[0]);
    i = 0;
    while (i < argc) {
        init->argv[i] = strdup(argv[i]);
        i++;
    }
    init->argv[i] = nullptr;
    init->argc = argc;
    return optind;
}
/** @brief parse the configuration file specified in init->minitrc and set
   Init struct values accordingly
    @ingroup init
    @returns on success, -1 on failure
    @note lines beginning with '#" are comments, discard
    @note copy line to tmp_str removing quotes, spaces, semicolons, and
   newlines
    @note record structure is "parse key=value pairs"
    @note skip lines without '='
    @note set init struct values based on key
    @note skips unknown keys */
int parse_config(Init *init) {
    char ts[MAXLEN];
    char *sp, *dp;
    char tmp_str[MAXLEN];
    SIO *sio = init->sio;
    if (!init->minitrc[0]) {
        char *e = getenv("MINITRC");
        if (e)
            strnz__cpy(init->minitrc, e, MAXLEN - 1);
        else
            strnz__cpy(init->minitrc, "~/.minitrc", MAXLEN - 1);
    }
    expand_tilde(init->minitrc, MAXLEN - 1);
    FILE *config_fp = fopen(init->minitrc, "r");
    if (!config_fp) {
        fprintf(stderr, "failed to read file: %s\n", init->minitrc);
        return (-1);
    }
    while (fgets(ts, sizeof(ts), config_fp)) {
        if (ts[0] != '#') {
            sp = ts;
            dp = tmp_str;
            while (*sp != '\0') {
                if (*sp == '\n') {
                    *dp = *sp = '\0';
                } else {
                    if (*sp != '"' && *sp != ' ' && *sp != ';') {
                        *dp++ = *sp;
                    }
                    sp++;
                }
            }
            *dp = '\0';
            char *key = strtok(tmp_str, "=");
            char *value = strtok(nullptr, "=");
            if (value == nullptr)
                continue;
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
            if (!strcmp(key, "fg_clr_x")) {
                strnz__cpy(sio->fg_clr_x, value, COLOR_LEN - 1);
                continue;
            }
            if (!strcmp(key, "bg_clr_x")) {
                strnz__cpy(sio->bg_clr_x, value, COLOR_LEN - 1);
                continue;
            }
            if (!strcmp(key, "f_ln")) {
                init->f_ln = str_to_bool(value);
                continue;
            }
            if (!strcmp(key, "bo_clr_x")) {
                strnz__cpy(sio->bo_clr_x, value, COLOR_LEN - 1);
                continue;
            }
            if (!strcmp(key, "ln_clr_x")) {
                strnz__cpy(sio->ln_clr_x, value, COLOR_LEN - 1);
                continue;
            }
            if (!strcmp(key, "ln_bg_clr_x")) {
                strnz__cpy(sio->ln_bg_clr_x, value, COLOR_LEN - 1);
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
                strnz__cpy(init->fill_char, value, 2);
                continue;
            }
            if (!strcmp(key, "f_ignore_case")) {
                init->f_ignore_case = str_to_bool(value);
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
            if (!strcmp(key, "select_max")) {
                init->select_max = atoi(value);
                continue;
            }
            if (!strcmp(key, "tab_stop")) {
                init->tab_stop = atoi(value);
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
            if (!strcmp(key, "bg")) {
                strnz__cpy(sio->bg, value, COLOR_LEN - 1);
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
            if (!strcmp(key, "bg")) {
                strnz__cpy(sio->bg, value, COLOR_LEN - 1);
                continue;
            }
            if (!strcmp(key, "editor")) {
                strnz__cpy(init->editor, value, MAXLEN - 1);
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
    @note The configuration is written in key=value format, one per line
    @note Lines beginning with '#' are comments and are ignored when reading
   the config file
    @note The file is created if it does not exist, and overwritten if it
   does exist
 */
int write_config(Init *init) {
    char *e;
    char minitrc_dmp[MAXLEN];
    char tmp_str[MAXLEN];
    SIO *sio = init->sio;
    e = getenv("HOME");
    if (e) {
        strnz__cpy(minitrc_dmp, e, MAXLEN - 1);
        strnz__cat(minitrc_dmp, "/", MAXLEN - 1);
        strnz__cat(minitrc_dmp, "menuapp/minitrc.dmp", MAXLEN - 1);
        ;
    } else {
        strnz__cpy(minitrc_dmp, "./minitrc.dmp", MAXLEN - 1);
    }
    FILE *minitrc_fp = fopen(minitrc_dmp, "w");
    if (minitrc_fp == (FILE *)0) {
        fprintf(stderr, "failed to open file: %s\n", minitrc_dmp);
        return (-1);
    }
    (void)fprintf(minitrc_fp, "# %s\n", "~/.minitrc");
    (void)fprintf(minitrc_fp, "%s=%d\n", "cols", init->cols);
    (void)fprintf(minitrc_fp, "%s=%d\n", "lines", init->lines);
    (void)fprintf(minitrc_fp, "%s=%d\n", "begx", init->begx);
    (void)fprintf(minitrc_fp, "%s=%d\n", "begy", init->begy);
    (void)fprintf(minitrc_fp, "%s=%s\n", "bg_clr_x", sio->bg_clr_x);
    (void)fprintf(minitrc_fp, "%s=%s\n", "fg_clr_x", sio->fg_clr_x);
    (void)fprintf(minitrc_fp, "%s=%s\n", "bo_clr_x", sio->bo_clr_x);
    (void)fprintf(minitrc_fp, "%s=%s\n", "ln_clr_x", sio->ln_clr_x);
    (void)fprintf(minitrc_fp, "%s=%s\n", "ln_bg_clr_x", sio->ln_bg_clr_x);
    (void)fprintf(minitrc_fp, "%s=%0.2f\n", "red_gamma", sio->red_gamma);
    (void)fprintf(minitrc_fp, "%s=%0.2f\n", "green_gamma", sio->green_gamma);
    (void)fprintf(minitrc_fp, "%s=%0.2f\n", "blue_gamma", sio->blue_gamma);
    (void)fprintf(minitrc_fp, "%s=%0.2f\n", "gray_gamma", sio->gray_gamma);
    (void)fprintf(minitrc_fp, "%s=%s\n", "in_spec", init->in_spec);
    (void)fprintf(minitrc_fp, "%s=%s\n", "out_spec", init->out_spec);
    (void)fprintf(minitrc_fp, "%s=%s\n", "provider_cmd", init->provider_cmd);
    (void)fprintf(minitrc_fp, "%s=%s\n", "receiver_cmd", init->receiver_cmd);
    (void)fprintf(minitrc_fp, "%s=%s\n", "cmd_all", init->cmd_all);
    (void)fprintf(minitrc_fp, "%s=%s\n", "parent_cmd", init->parent_cmd);
    (void)fprintf(minitrc_fp, "%s=%s\n", "title", init->title);
    (void)fprintf(minitrc_fp, "%s=%d\n", "select_max", init->select_max);
    (void)fprintf(minitrc_fp, "%s=%s\n", "brackets", init->brackets);
    (void)fprintf(minitrc_fp, "%s=%d\n", "tab_stop", init->tab_stop);
    (void)fprintf(minitrc_fp, "%s=%s\n", "cmd", init->cmd);
    (void)fprintf(minitrc_fp, "%s=%s\n", "f_at_end_remove",
                  init->f_at_end_remove ? "true" : "false");
    (void)fprintf(minitrc_fp, "%s=%s\n", "f_erase_remainder",
                  init->f_erase_remainder ? "true" : "false");
    (void)fprintf(minitrc_fp, "%s=%s\n", "f_ln", init->f_ln ? "true" : "false");
    (void)fprintf(minitrc_fp, "%s=%s\n", "fill_char", init->fill_char);
    (void)fprintf(minitrc_fp, "%s=%s\n", "f_strip_ansi",
                  init->f_strip_ansi ? "true" : "false");
    (void)fprintf(minitrc_fp, "%s=%s\n", "f_ignore_case",
                  init->f_ignore_case ? "true" : "false");
    (void)fprintf(minitrc_fp, "%s=%s\n", "f_squeeze",
                  init->f_squeeze ? "true" : "false");
    (void)fprintf(minitrc_fp, "%s=%s\n", "black", sio->black);
    (void)fprintf(minitrc_fp, "%s=%s\n", "bg", sio->bg);
    (void)fprintf(minitrc_fp, "%s=%s\n", "abg", sio->abg);
    (void)fprintf(minitrc_fp, "%s=%s\n", "black", sio->black);
    (void)fprintf(minitrc_fp, "%s=%s\n", "red", sio->red);
    (void)fprintf(minitrc_fp, "%s=%s\n", "green", sio->green);
    (void)fprintf(minitrc_fp, "%s=%s\n", "yellow", sio->yellow);
    (void)fprintf(minitrc_fp, "%s=%s\n", "blue", sio->blue);
    (void)fprintf(minitrc_fp, "%s=%s\n", "magenta", sio->magenta);
    (void)fprintf(minitrc_fp, "%s=%s\n", "cyan", sio->cyan);
    (void)fprintf(minitrc_fp, "%s=%s\n", "white", sio->white);
    (void)fprintf(minitrc_fp, "%s=%s\n", "bblack", sio->bblack);
    (void)fprintf(minitrc_fp, "%s=%s\n", "bred", sio->bred);
    (void)fprintf(minitrc_fp, "%s=%s\n", "bgreen", sio->bgreen);
    (void)fprintf(minitrc_fp, "%s=%s\n", "byellow", sio->byellow);
    (void)fprintf(minitrc_fp, "%s=%s\n", "bblue", sio->bblue);
    (void)fprintf(minitrc_fp, "%s=%s\n", "bmagenta", sio->bmagenta);
    (void)fprintf(minitrc_fp, "%s=%s\n", "bcyan", sio->bcyan);
    (void)fprintf(minitrc_fp, "%s=%s\n", "bwhite", sio->bwhite);
    (void)fprintf(minitrc_fp, "%s=%s\n", "editor", init->editor);
    (void)fprintf(minitrc_fp, "%s=%s\n", "mapp_spec", init->mapp_spec);
    (void)fprintf(minitrc_fp, "%s=%s\n", "help_spec", init->help_spec);
    (void)fprintf(minitrc_fp, "%s=%s\n", "mapp_data", init->mapp_data);
    (void)fprintf(minitrc_fp, "%s=%s\n", "mapp_help", init->mapp_help);
    (void)fprintf(minitrc_fp, "%s=%s\n", "mapp_home", init->mapp_home);
    (void)fprintf(minitrc_fp, "%s=%s\n", "mapp_msrc", init->mapp_msrc);
    (void)fprintf(minitrc_fp, "%s=%s\n", "mapp_user", init->mapp_user);
    (void)fclose(minitrc_fp);
    strnz__cpy(tmp_str, "Configuration written to file: ", MAXLEN - 1);
    strnz__cat(tmp_str, minitrc_dmp, MAXLEN - 1);
    Perror(tmp_str);
    return 0;
}
/** @brief Derive full file specification from directory and file name
    @ingroup init
    @param file_spec - output full file specification
    @param dir - directory path
    @param file_name - file name
    @returns true if file_spec is derived, false otherwise
    @note If dir is nullptr, use MAPP_DIR environment variable or default
   directory
   ~/menuapp
    @note file_spec should be a pre-allocated char array of size MAXLEN to
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
    @note The version information is defined in the mapp_version variable
   and is printed to stderr when this function is called. */
void display_version() {
    fprintf(stderr, "\nC-Menu %s\n", CM_VERSION);
    exit(EXIT_SUCCESS);
}
/** @brief Display the usage information of the application
    @ingroup init
    @note The usage information is printed to stderr when this function is
   called. After displaying the usage information, the function waits for
   the user to press any key before returning. */
void usage() {
    dump_opts();
    (void)fprintf(stderr, "\n\nPress any key to continue...");
    di_getch();
}
/** @brief Print an option and its value in a formatted manner
    @ingroup init
    @param o - option flag (e.g., "-a:")
    @param name - option name (e.g., "--minitrc")
    @param value - option value to print
    @note This function is used to display the current configuration options
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
    @note This function is used to display the current configuration options
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
    @note This function is used to display the current configuration options
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
    @note This function is used to display the current configuration options
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
    @note This function is used to display the current configuration options
   and their boolean values in a readable format, printing "true" or "false"
   based on the value. */
void opt_prt_bool(const char *o, const char *name, bool value) {
    fprintf(stdout, "%3s %-15s: %s\n", o, name, value ? "true" : "false");
}
/** @brief Dump the current configuration to stderr for debugging purposes
    @ingroup init
    @param init - pointer to Init struct containing the current
   configuration
    @param msg - string to print before dumping the configuration to stderr
   in a readable format, prefixed by the provided title string. */
void dump_config(Init *init, char *msg) {
    SIO *sio = init->sio;
    opt_prt_str("-a:", "--minitrc", init->minitrc);
    opt_prt_int("-L:", "  lines", init->lines);
    opt_prt_int("-C:", "  cols", init->cols);
    opt_prt_int("-X:", "  begx", init->begx);
    opt_prt_int("-Y:", "  begy", init->begy);
    opt_prt_str("   ", "  fg_clr_x", sio->fg_clr_x);
    opt_prt_str("   ", "  bg_clr_x", sio->bg_clr_x);
    opt_prt_str("   ", "  bo_clr_x", sio->bo_clr_x);
    opt_prt_str("   ", "  ln_clr_x", sio->ln_clr_x);
    opt_prt_str("   ", "  ln_bg_clr_x", sio->ln_bg_clr_x);
    opt_prt_str("   ", "  title", init->title);
    opt_prt_double("   ", "  red_gamma", sio->red_gamma);
    opt_prt_double("   ", "  green_gamma", sio->green_gamma);
    opt_prt_double("   ", "  blue_gamma", sio->blue_gamma);
    opt_prt_double("   ", "  gray_gamma", sio->gray_gamma);
    opt_prt_str("-f:", "  fill_char", init->fill_char);
    opt_prt_str("-u", "  brackets", init->brackets);
    opt_prt_int("-t:", "  tab_stop", init->tab_stop);
    opt_prt_int("-n:", "  select_max", init->select_max);
    opt_prt_str("-i:", "  in_spec", init->in_spec);
    opt_prt_str("-o:", "  out_spec", init->out_spec);
    opt_prt_str("-R:", "  receiver_cmd", init->receiver_cmd);
    opt_prt_str("-S:", "  provider_cmd", init->provider_cmd);
    opt_prt_str("-c:", "  cmd", init->cmd);
    opt_prt_str("-A:", "  cmd_all", init->cmd_all);
    opt_prt_str("-k:", "  parent_cmd", init->parent_cmd);
    opt_prt_bool("-e:", "  f_erase_remainder", init->f_erase_remainder);
    opt_prt_bool("-N:", "  f_ln", init->f_ln);
    opt_prt_bool("-a ", "  f_strip_ansi", init->f_strip_ansi);
    opt_prt_bool("-s ", "  f_squeeze", init->f_squeeze);
    opt_prt_bool("-x:", "  f_ignore_case", init->f_ignore_case);
    opt_prt_bool("-y:", "  f_at_end_remove", init->f_at_end_remove);
    opt_prt_str("   ", "  black", sio->black);
    opt_prt_str("   ", "  red", sio->red);
    opt_prt_str("   ", "  green", sio->green);
    opt_prt_str("   ", "  yellow", sio->yellow);
    opt_prt_str("   ", "  blue", sio->blue);
    opt_prt_str("   ", "  magenta", sio->magenta);
    opt_prt_str("   ", "  cyan", sio->cyan);
    opt_prt_str("   ", "  white", sio->white);
    opt_prt_str("   ", "  orange", sio->orange);
    opt_prt_str("   ", "  bblack", sio->bblack);
    opt_prt_str("   ", "  bred", sio->bred);
    opt_prt_str("   ", "  bgreen", sio->bgreen);
    opt_prt_str("   ", "  byellow", sio->byellow);
    opt_prt_str("   ", "  bblue", sio->bblue);
    opt_prt_str("   ", "  bmagenta", sio->bmagenta);
    opt_prt_str("   ", "  bcyan", sio->bcyan);
    opt_prt_str("   ", "  bwhite", sio->bwhite);
    opt_prt_str("   ", "  borange", sio->borange);
    opt_prt_str("   ", "  bg", sio->bg);
    opt_prt_str("   ", "  abg", sio->abg);
    opt_prt_str("   ", "  editor", init->editor);
    opt_prt_str("   ", "  help_spec", init->help_spec);
    opt_prt_str("-d:", "--mapp_spec", init->mapp_spec);
    opt_prt_str("   ", "--mapp_home", init->mapp_home);
    opt_prt_str("   ", "--mapp_data", init->mapp_data);
    opt_prt_str("   ", "--mapp_help", init->mapp_help);
    opt_prt_str("   ", "--mapp_user", init->mapp_user);
    opt_prt_str("   ", "--mapp_msrc", init->mapp_msrc);

    (void)fprintf(stderr, "\n%s\n\n", msg);
}
