/** @file init.c
    @brief Initialization for Menu Application Programs
    @author Bill Waller
    Copyright (c) 2025
    MIT License
    billxwaller@gmail.com
    @date 2026-02-09
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
bool f_debug = false;
bool f_stop_on_error = true;

int write_config(Init *init);
void display_version();

Init *init = NULL;

void mapp_initialization(Init *init, int, char **);
int parse_opt_args(Init *, int, char **);
void zero_opt_args(Init *);
int parse_config(Init *);
void dump_config(Init *, char *);
void usage();

void prompt_int_to_str(char *, int);
int prompt_str_to_int(char *);
bool derive_file_spec(char *, char *, char *);
int executor = 0;

/** @brief Main initialization function for MAPP - Menu Application
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
    char *t;
    setlocale(LC_ALL, "en_US.UTF-8");
    SIO *sio = init->sio;
    if (!init) {
        ssnprintf(tmp_str, sizeof(tmp_str), "%s",
                  "init struct not allocated on entry");
        abend(-1, tmp_str);
        exit(-1);
    }
    if (init->minitrc[0] == '\0')
        strnz__cpy(init->minitrc, "~/.minitrc", MAXLEN - 1);
    sio->bg_color = BG_COLOR;       /**< background color */
    sio->fg_color = FG_COLOR;       /**< foreground color */
    sio->bo_color = BO_COLOR;       /**< border color */
    init->f_at_end_clear = true;    /**< clear screen on exit (obsolete) */
    init->f_erase_remainder = true; /**< erase remainder on enter */
    init->brackets[0] = '\0';       /**< field enclosure brackets */
    strnz__cpy(init->fill_char, "_", MAXLEN - 1); /**< field fill character */
    init->prompt_type = PT_LONG;                  /**< view prompt type */
    strnz__cpy(init->mapp_spec, "main.m", MAXLEN - 1);
    strnz__cpy(init->mapp_home, "~/menuapp", MAXLEN - 1);
    strnz__cpy(init->mapp_user, "~/menuapp/user", MAXLEN - 1);
    strnz__cpy(init->mapp_msrc, "~/menuapp/msrc", MAXLEN - 1);
    strnz__cpy(init->mapp_data, "~/menuapp/data", MAXLEN - 1);
    strnz__cpy(init->mapp_help, "~/menuapp/help", MAXLEN - 1);
    // Priority-4 - cfg_args

    t = getenv("TERM");
    if (!t || *t == '\0')
        strnz__cpy(term, "xterm-256color", MAXLEN);
    t = getenv("EDITOR");
    if (t && *t != '\0')
        strnz__cpy(init->editor, "vi", MAXLEN - 1);

    parse_config(init); /**< generally /home/user/.minitrc */
    if (f_debug)
        dump_config(init, "Configuration after parse_config");
    parse_opt_args(init, argc,
                   argv); /**< command-line options override config file */
    if (f_dump_config) {
        dump_config(init, "Configuration after parse_config and "
                          "parse_opt_args");
    } else if (f_debug)
        dump_config(init, "Configuration after parse_opt_args");
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
/** @brief Initialize optional arguments in the Init struct to default values
    @param init - pointer to Init struct to be initialized This function sets
   all optional argument fields in the Init struct to their default values
   before parsing command-line options or configuration file. This ensures that
   any fields not specified by the user will have known default values.
 */
void zero_opt_args(Init *init) {
    init->f_mapp_desc = false;
    init->f_provider_cmd = false;
    init->f_receiver_cmd = false;
    init->f_title = false;
    init->f_help_spec = false;
    init->f_in_spec = false;
    init->f_out_spec = false;
    init->mapp_spec[0] = init->help_spec[0] = '\0';
    init->provider_cmd[0] = init->receiver_cmd[0] = '\0';
    init->title[0] = '\0';
    init->cmd[0] = init->cmd_all[0] = '\0';
    init->in_spec[0] = init->out_spec[0] = '\0';
    init->help_spec[0] = '\0';
    init->in_spec[0] = '\0';
    init->out_spec[0] = '\0';
}
/** @brief Parse command-line options and set Init struct values accordingly
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

    char *optstring =
        "a:b:c:d:ef:g:hi:m:n:o:p:r:st:u:vw:xzA:B:C:DF:G:H:L:MO:P:R:S:T:VWX:Y:Z";
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
        case 'm':
            strnz__cpy(init->mapp_home, optarg, MAXLEN - 1);
            break;
        case 'n':
            init->select_max = atoi(optarg);
            break;
        case 'o':
            strnz__cpy(init->out_spec, optarg, MAXLEN - 1);
            break;
        case 'p':
            strnz__cpy(init->prompt_str, optarg, MAXLEN - 1);
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
        case 'z':
            init->f_at_end_clear = true;
            break;
        case 'A':
            strnz__cpy(init->cmd_all, optarg, MAXLEN - 1);
            break;
        case 'B':
            sio->bg_color = clr_name_to_idx(optarg);
            break;
        case 'C':
            init->cols = atoi(optarg);
            break;
        case 'D':
            f_dump_config = true;
            break;
        case 'F':
            sio->fg_color = clr_name_to_idx(optarg);
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
        case 'O':
            sio->bo_color = clr_name_to_idx(optarg);
            break;
        case 'P':
            strnz__cpy(tmp_str, optarg, MAXLEN - 1);
            init->prompt_type = prompt_str_to_int(tmp_str);
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
        case 'Z':
            init->f_stop_on_error = true;
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
    init->argv[i] = NULL;
    init->argc = argc;
    return optind;
}
/** @brief parse the configuration file specified in init->minitrc and set Init
   struct values accordingly
    @returns on success, -1 on failure
    @note lines beginning with '#" are comments, discard
    @note copy line to tmp_str removing quotes, spaces, semicolons, and newlines
    @note record structure is "parse key=value pairs"
    @note skip lines without '='
    @note set init struct values based on key
    @note skips unknown keys */
int parse_config(Init *init) {
    char ts[MAXLEN];
    char *sp, *dp;

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
            char *value = strtok(NULL, "=");
            if (value == NULL)
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
            if (!strcmp(key, "fg_color")) {
                sio->fg_color = clr_name_to_idx(value);
                continue;
            }
            if (!strcmp(key, "bg_color")) {
                sio->bg_color = clr_name_to_idx(value);
                continue;
            }
            if (!strcmp(key, "bo_color")) {
                sio->bo_color = clr_name_to_idx(value);
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
            if (!strcmp(key, "f_at_end_clear")) {
                init->f_at_end_clear = str_to_bool(value);
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
            if (!strcmp(key, "f_stop_on_error")) {
                init->f_stop_on_error = str_to_bool(value);
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
            if (!strcmp(key, "prompt_type")) {
                strnz__cpy(tmp_str, value, MAXLEN - 1);
                init->prompt_type = prompt_str_to_int(tmp_str);
                continue;
            }
            if (!strcmp(key, "prompt_str")) {
                strnz__cpy(init->prompt_str, value, MAXLEN - 1);
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
/** @brief Convert prompt type string to integer constant
    @param s - prompt type string
    @returns prompt type integer constant
    @note Valid prompt type strings are "short", "long", and "string"
    @note Returns PT_NONE for unrecognized prompt type strings
 */
int prompt_str_to_int(char *s) {
    int prompt_type;
    str_to_lower(s);
    if (strcmp(s, "short") == 0)
        prompt_type = PT_SHORT;
    else if (strcmp(s, "long") == 0)
        prompt_type = PT_LONG;
    else if (strcmp(s, "string") == 0)
        prompt_type = PT_STRING;
    else
        prompt_type = PT_NONE;
    return prompt_type;
}
/** @brief Convert prompt type integer constant to string
    @param s - output prompt type string
    @param prompt_type - prompt type integer constant
    @note Valid prompt type integer constants are PT_SHORT, PT_LONG, and
   PT_STRING
    @note Returns "none" for unrecognized prompt type integer constants
 */
void prompt_int_to_str(char *s, int prompt_type) {
    switch (prompt_type) {
    case PT_SHORT:
        strnz__cpy(s, "short", 7);
        break;
    case PT_LONG:
        strnz__cpy(s, "long", 7);
        break;
    case PT_STRING:
        strnz__cpy(s, "string", 7);
        break;
    default:
        strnz__cpy(s, "none", 7);
        break;
    }
}
/** @brief Write the current configuration to a file specified in init->minitrc
    @param init - pointer to Init struct containing current configuration
    @returns 0 on success, -1 on failure
    @note The configuration is written in key=value format, one per line
    @note Lines beginning with '#' are comments and are ignored when reading the
   config file
    @note The file is created if it does not exist, and overwritten if it does
   exist
 */
int write_config(Init *init) {
    char *e;
    char minitrc_dmp[MAXLEN];

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
    (void)fprintf(minitrc_fp, "%s=%s\n", "bg_color",
                  colors_text[sio->bg_color]);
    (void)fprintf(minitrc_fp, "%s=%s\n", "fg_color",
                  colors_text[sio->fg_color]);
    (void)fprintf(minitrc_fp, "%s=%s\n", "bo_color",
                  colors_text[sio->bo_color]);
    (void)fprintf(minitrc_fp, "%s=%0.2f\n", "red_gamma", sio->red_gamma);
    (void)fprintf(minitrc_fp, "%s=%0.2f\n", "green_gamma", sio->green_gamma);
    (void)fprintf(minitrc_fp, "%s=%0.2f\n", "blue_gamma", sio->blue_gamma);
    (void)fprintf(minitrc_fp, "%s=%0.2f\n", "gray_gamma", sio->gray_gamma);
    (void)fprintf(minitrc_fp, "%s=%s\n", "cmd_all", init->cmd_all);
    (void)fprintf(minitrc_fp, "%s=%s\n", "in_spec", init->in_spec);
    (void)fprintf(minitrc_fp, "%s=%s\n", "out_spec", init->out_spec);
    (void)fprintf(minitrc_fp, "%s=%s\n", "provider_cmd", init->provider_cmd);
    (void)fprintf(minitrc_fp, "%s=%s\n", "receiver_cmd", init->receiver_cmd);
    (void)fprintf(minitrc_fp, "%s=%s\n", "title", init->title);
    (void)fprintf(minitrc_fp, "%s=%d\n", "select_max", init->select_max);
    (void)fprintf(minitrc_fp, "%s=%s\n", "brackets", init->brackets);
    (void)fprintf(minitrc_fp, "%s=%d\n", "tab_stop", init->tab_stop);
    prompt_int_to_str(tmp_str, init->prompt_type);
    (void)fprintf(minitrc_fp, "%s=%s\n", "prompt_type", tmp_str);
    (void)fprintf(minitrc_fp, "%s=%s\n", "prompt_str", init->prompt_str);
    (void)fprintf(minitrc_fp, "%s=%s\n", "cmd", init->cmd);
    (void)fprintf(minitrc_fp, "%s=%s\n", "f_at_end_clear",
                  init->f_at_end_clear ? "true" : "false");
    (void)fprintf(minitrc_fp, "%s=%s\n", "f_at_end_remove",
                  init->f_at_end_remove ? "true" : "false");
    (void)fprintf(minitrc_fp, "%s=%s\n", "f_erase_remainder",
                  init->f_erase_remainder ? "true" : "false");
    (void)fprintf(minitrc_fp, "%s=%s\n", "fill_char", init->fill_char);
    (void)fprintf(minitrc_fp, "%s=%s\n", "f_strip_ansi",
                  init->f_strip_ansi ? "true" : "false");
    (void)fprintf(minitrc_fp, "%s=%s\n", "f_ignore_case",
                  init->f_ignore_case ? "true" : "false");
    (void)fprintf(minitrc_fp, "%s=%s\n", "f_squeeze",
                  init->f_squeeze ? "true" : "false");
    (void)fprintf(minitrc_fp, "%s=%s\n", "f_stop_on_error",
                  init->f_stop_on_error ? "true" : "false");
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
    @param file_spec - output full file specification
    @param dir - directory path
    @param file_name - file name
    @returns true if file_spec is derived, false otherwise
    @note If dir is NULL, use MAPP_DIR environment variable or default directory
   ~/menuapp
    @note file_spec should be a pre-allocated char array of size MAXLEN to hold
   the resulting file specification
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
    @note The version information is defined in the mapp_version variable and is
   printed to stderr when this function is called. */
void display_version() {
    fprintf(stderr, "\nC-Menu version: %s\n", CM_VERSION);
    exit(EXIT_SUCCESS);
}
/** @brief Display the usage information of the application
    @note The usage information is printed to stderr when this function is
   called. After displaying the usage information, the function waits for the
   user to press any key before returning. */
void usage() {
    (void)fprintf(stderr, "\n\nPress any key to continue...");
    di_getch();
}
/** @brief Print an option and its value in a formatted manner
    @param o - option flag (e.g., "-a:")
    @param name - option name (e.g., "--minitrc")
    @param value - option value to print
    @note This function is used to display the current configuration options and
   their values in a readable format. */
void opt_prt_char(const char *o, const char *name, const char *value) {
    fprintf(stdout, "%3s %-15s: %s\n", o, name, value);
}
/** @brief Print an option and its value in a formatted manner for integer
   values
    @param o - option flag (e.g., "-C:")
    @param name - option name (e.g., "--cols")
    @param value - integer option value to print
    @note This function is used to display the current configuration options and
   their integer values in a readable format. */
void opt_prt_str(const char *o, const char *name, const char *value) {
    fprintf(stdout, "%3s %-15s: %s\n", o, name, value);
}
/** @brief Print an option and its value in a formatted manner for integer
   values
    @param o - option flag (e.g., "-C:")
    @param name - option name (e.g., "--cols")
    @param value - integer option value to print
    @note This function is used to display the current configuration options and
   their integer values in a readable format. */
void opt_prt_int(const char *o, const char *name, int value) {
    fprintf(stdout, "%3s %-15s: %d\n", o, name, value);
}
/** @brief Print an option and its value in a formatted manner for double values
    @param o - option flag (e.g., "-r:")
    @param name - option name (e.g., "red_gamma")
    @param value - double option value to print
    @note This function is used to display the current configuration options and
   their double values in a readable format. */
void opt_prt_double(const char *o, const char *name, double value) {
    fprintf(stdout, "%3s %-15s: %0.2f\n", o, name, value);
}
/** @brief Print an option and its value in a formatted manner for boolean
   values
    @param o - option flag (e.g., "-z")
    @param name - option name (e.g., "f_at_end_clear")
    @param value - boolean option value to print
    @note This function is used to display the current configuration options and
   their boolean values in a readable format, printing "true" or "false" based
   on the value. */
void opt_prt_bool(const char *o, const char *name, bool value) {
    fprintf(stdout, "%3s %-15s: %s\n", o, name, value ? "true" : "false");
}
/** @brief Dump the current configuration to stderr for debugging purposes
    @param init - pointer to Init struct containing the current configuration
    @param msg - string to print before dumping the configuration to stderr in a
   readable format, prefixed by the provided title string. */
void dump_config(Init *init, char *msg) {
    SIO *sio = init->sio;
    opt_prt_str("-a:", "--minitrc", init->minitrc);
    opt_prt_int("-L:", "  lines", init->lines);
    opt_prt_int("-C:", "  cols", init->cols);
    opt_prt_int("-X:", "  begx", init->begx);
    opt_prt_int("-Y:", "  begy", init->begy);
    opt_prt_int("-F:", "  fg_color", sio->fg_color);
    opt_prt_int("-B:", "  bg_color", sio->bg_color);
    opt_prt_int("-O:", "  bo_color", sio->bo_color);
    opt_prt_str("-T:", "  title", init->title);
    opt_prt_str("-c:", "  cmd", init->cmd);
    opt_prt_double("-r:", "  red_gamma", sio->red_gamma);
    opt_prt_double("-g:", "  green_gamma", sio->green_gamma);
    opt_prt_double("-b:", "  blue_gamma", sio->blue_gamma);
    opt_prt_double("-G:", "  gray_gamma", sio->gray_gamma);
    opt_prt_str("-f:", "  fill_char", init->fill_char);
    opt_prt_str("-u", "  brackets", init->brackets);
    opt_prt_int("-t:", "  tab_stop", init->tab_stop);
    opt_prt_int("-n:", "  select_max", init->select_max);
    opt_prt_str("-i:", "  in_spec", init->in_spec);
    opt_prt_str("-o:", "  out_spec", init->out_spec);
    opt_prt_str("-R:", "  receiver_cmd", init->receiver_cmd);
    opt_prt_str("-S:", "  provider_cmd", init->provider_cmd);
    opt_prt_bool("-e:", "  f_erase_remainder", init->f_erase_remainder);
    opt_prt_bool("-j:", "  f_strip_ansi", init->f_strip_ansi);
    opt_prt_bool("-s ", "  f_squeeze", init->f_squeeze);
    opt_prt_bool("-x:", "  f_ignore_case", init->f_ignore_case);
    opt_prt_bool("-y:", "  f_at_end_remove", init->f_at_end_remove);
    opt_prt_bool("-z ", "  f_at_end_clear", init->f_at_end_clear);
    opt_prt_bool("-Z ", "  f_stop_on_error", init->f_stop_on_error);
    opt_prt_str("-A:", "  cmd_all", init->cmd_all);
    opt_prt_str("-P:", "  promp_type", tmp_str);
    prompt_int_to_str(tmp_str, init->prompt_type);
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
    opt_prt_str("-H:", "  help_spec", init->help_spec);
    opt_prt_str("-d:", "--mapp_spec", init->mapp_spec);
    opt_prt_str("-m:", "--mapp_home", init->mapp_home);
    opt_prt_str("   ", "--mapp_data", init->mapp_data);
    opt_prt_str("   ", "--mapp_help", init->mapp_help);
    opt_prt_str("-U:", "--mapp_user", init->mapp_user);
    opt_prt_str("   ", "--mapp_msrc", init->mapp_msrc);

    (void)fprintf(stderr, "\n%s\n\n", msg);
}
