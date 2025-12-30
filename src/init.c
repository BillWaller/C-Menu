// initialization.c
// Bill Waller Copyright (c) 2025
// billxwaller@gmail.com

#include "menu.h"
#include <getopt.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

enum {
    MAPP_MSRC = 257,
    MAPP_HELP,
    MAPP_DATA,
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

const char *mapp_version = "0.5.1";
const char *PgmID = "init.c";

int write_config(Init *init);
void display_version();

// GLOBL INITVARS - DEFAULT VALUES
Init *init = NULL;

void mapp_initialization(Init *init, int, char **);
int parse_opt_args(Init *, int, char **);
void zero_opt_args(Init *);
int parse_config(Init *);
void dump_config(Init *, char *);
void usage();

void prompt_int_to_str(char *, int);
int prompt_str_to_int(char *);
char *tilde_expand(char *);
bool derive_file_spec(char *, char *, char *);
int executor = 0;

// ╭───────────────────────────────────────────────────────────────────╮
// │ MAPP_INITIALIZATION                                               │
// ╰───────────────────────────────────────────────────────────────────╯
void mapp_initialization(Init *init, int argc, char **argv) {
    setlocale(LC_ALL, "en_US.UTF-8");
    if (!init) {
        snprintf(tmp_str, sizeof(tmp_str), "%s",
                 "init struct not allocated on entry");
        abend(-1, tmp_str);
    }
    if (init->minitrc[0] == '\0')
        strnz__cpy(init->minitrc, "~/.minitrc", MAXLEN - 1);
    init->bg_color = BG_COLOR;                    // B: background color
    init->fg_color = FG_COLOR;                    // F: foreground color
    init->bo_color = BO_COLOR;                    // O: border colorZ
    init->help_spec[0] = '\0';                    // H: help spec
    init->f_at_end_clear = true;                  // z  clear screen on exit
    init->f_erase_remainder = true;               // e  erase remainder on enter
    init->f_brackets = true;                      // f  erase remainder on enter
    strnz__cpy(init->fill_char, "_", MAXLEN - 1); // u  underscore
    init->prompt_type = PT_LONG;                  // P: prompt type
    strnz__cpy(init->mapp_spec, "main.m", MAXLEN - 1);
    strnz__cpy(init->mapp_home, "~/menuapp", MAXLEN - 1);
    strnz__cpy(init->mapp_user, "~/menuapp/user", MAXLEN - 1);
    strnz__cpy(init->mapp_msrc, "~/menuapp/msrc", MAXLEN - 1);
    strnz__cpy(init->mapp_data, "~/menuapp/data", MAXLEN - 1);
    strnz__cpy(init->mapp_help, "~/menuapp/help", MAXLEN - 1);

    // Priority-4 - cfg_args
    parse_config(init);
    if (f_debug)
        dump_config(init, "Configuration after parse_config");
    // Priority 1 - opt_args
    parse_opt_args(init, argc, argv);

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
}
// ╭───────────────────────────────────────────────────────────────────╮
// │ ZERO_OPT_ARGS                                                     │
// ╰───────────────────────────────────────────────────────────────────╯
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
    init->view_cmd[0] = init->view_cmd_all[0] = '\0';
    init->in_spec[0] = init->out_spec[0] = '\0';
    init->help_spec[0] = '\0';
    init->in_spec[0] = '\0';
    init->out_spec[0] = '\0';
}
// ╭───────────────────────────────────────────────────────────────────╮
// │ PARSE_OPT_ARGS                                                    │
// ╰───────────────────────────────────────────────────────────────────╯
int parse_opt_args(Init *init, int argc, char **argv) {
    int i;
    int opt;
    int longindex = 0;
    int flag = 0;

    char *optstring =
        "a:b:c:d:f:g:hi:m:n:o:p:rst:uvwxzA:B:C:DE:F:H:L:MO:P:R:S:T:U:VX:Y:Z";
    struct option long_options[] = {
        {"mapp_data", 1, &flag, MAPP_DATA}, {"mapp_spec", 1, &flag, MAPP_SPEC},
        {"mapp_help", 1, &flag, MAPP_HELP}, {"help_spec", 1, &flag, HELP_SPEC},
        {"in_spec", 1, &flag, IN_SPEC},     {"out_spec", 1, &flag, OUT_SPEC},
        {"mapp_msrc", 1, &flag, MAPP_MSRC}, {0, 0, 0, 0}};
    char mapp[MAXLEN];
    base_name(mapp, argv[0]);
    optind = 1;
    while ((opt = getopt_long(argc, argv, optstring, long_options,
                              &longindex)) != -1) {
        switch (opt) {
        case 0:
            //  ╭───────────────────────────────────────────────────╮
            //  │ LONG_OPTIONS                                      │
            //  ╰───────────────────────────────────────────────────╯
            switch (flag) {
            case MAPP_DATA:
                strnz__cpy(init->mapp_data, optarg, MAXLEN - 1);
                break;
            case MAPP_SPEC:
                strnz__cpy(init->mapp_spec, optarg, MAXLEN - 1);
                break;
            case MAPP_HELP:
                strnz__cpy(init->mapp_help, optarg, MAXLEN - 1);
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
            //  ╭───────────────────────────────────────────────────╮
            //  │ SHORT_OPTIONS                                     │
            //  ╰───────────────────────────────────────────────────╯
        case 'a':
            strnz__cpy(init->minitrc, optarg, MAXLEN - 1);
            break;
        case 'b':
            init->blue_gamma = str_to_double(optarg);
            break;
        case 'd':
            strnz__cpy(init->mapp_spec, optarg, MAXLEN - 1);
            break;
        case 'e':
            init->f_erase_remainder = true;
            break;
        case 'u':
            init->f_brackets = true;
            break;
        case 'g':
            init->green_gamma = str_to_double(optarg);
            break;
        case 'h':
            f_help = true;
            break;
        case 'i':
            strnz__cpy(init->in_spec, optarg, MAXLEN - 1);
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
            init->red_gamma = str_to_double(optarg);
            break;
        case 's':
            init->f_squeeze = true;
            break;
        case 't':
            init->tab_stop = atoi(optarg);
            if (init->tab_stop < 1)
                init->tab_stop = 1;
            break;
        case 'f':
            strncpy(init->fill_char, optarg, 2);
            break;
        case 'v':
        case 'V':
            f_version = true;
            break;
        case 'w':
            f_write_config = true;
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
        case 'B':
            init->bg_color = get_color_number(optarg);
            break;
        case 'C':
            init->cols = atoi(optarg);
            break;
        case 'D':
            f_dump_config = true;
            break;
        case 'S':
            strnz__cpy(init->provider_cmd, optarg, MAXLEN - 1);
            break;
        case 'F':
            init->fg_color = get_color_number(optarg);
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
            init->bo_color = get_color_number(optarg);
            break;
        case 'P':
            strnz__cpy(tmp_str, optarg, MAXLEN - 1);
            init->prompt_type = prompt_str_to_int(tmp_str);
            break;
        case 'p':
            strnz__cpy(init->prompt_str, optarg, MAXLEN - 1);
            break;
        case 'R':
            strnz__cpy(init->receiver_cmd, optarg, MAXLEN - 1);
            break;
        case 'c':
            strnz__cpy(init->view_cmd, optarg, MAXLEN - 1);
            break;
        case 'A':
            strnz__cpy(init->view_cmd_all, optarg, MAXLEN - 1);
            break;
        case 'T':
            strnz__cpy(init->title, optarg, MAXLEN - 1);
            break;
        case 'U':
            strnz__cpy(init->mapp_user, optarg, MAXLEN - 1);
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
// ╭────────────────────────────────────────────────────────────────╮
// │ PARSE_CONFIG                                                   │
// ╰────────────────────────────────────────────────────────────────╯
int parse_config(Init *init) {
    char ts[MAXLEN];
    char *sp, *dp;

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
                init->fg_color = get_color_number(value);
                continue;
            }
            if (!strcmp(key, "bg_color")) {
                init->bg_color = get_color_number(value);
                continue;
            }
            if (!strcmp(key, "bo_color")) {
                init->bo_color = get_color_number(value);
                continue;
            }
            if (!strcmp(key, "red_gamma")) {
                init->red_gamma = str_to_double(value);
                continue;
            }
            if (!strcmp(key, "green_gamma")) {
                init->green_gamma = str_to_double(value);
                continue;
            }
            if (!strcmp(key, "blue_gamma")) {
                init->blue_gamma = str_to_double(value);
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
            if (!strcmp(key, "f_brackets")) {
                init->f_brackets = str_to_bool(value);
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
            if (!strcmp(key, "view_cmd")) {
                strnz__cpy(init->view_cmd, value, MAXLEN - 1);
                continue;
            }
            if (!strcmp(key, "view_cmd_all")) {
                strnz__cpy(init->view_cmd_all, value, MAXLEN - 1);
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
                strnz__cpy(init->bg, value, COLOR_LEN - 1);
                continue;
            }
            if (!strcmp(key, "black")) {
                strnz__cpy(init->black, value, COLOR_LEN - 1);
                continue;
            }
            if (!strcmp(key, "red")) {
                strnz__cpy(init->red, value, COLOR_LEN - 1);
                continue;
            }
            if (!strcmp(key, "green")) {
                strnz__cpy(init->green, value, COLOR_LEN - 1);
                continue;
            }
            if (!strcmp(key, "yellow")) {
                strnz__cpy(init->yellow, value, COLOR_LEN - 1);
                continue;
            }
            if (!strcmp(key, "blue")) {
                strnz__cpy(init->blue, value, COLOR_LEN - 1);
                continue;
            }
            if (!strcmp(key, "magenta")) {
                strnz__cpy(init->magenta, value, COLOR_LEN - 1);
                continue;
            }
            if (!strcmp(key, "cyan")) {
                strnz__cpy(init->cyan, value, COLOR_LEN - 1);
                continue;
            }
            if (!strcmp(key, "white")) {
                strnz__cpy(init->white, value, COLOR_LEN - 1);
                continue;
            }
            if (!strcmp(key, "orange")) {
                strnz__cpy(init->orange, value, COLOR_LEN - 1);
                continue;
            }
            if (!strcmp(key, "bblack")) {
                strnz__cpy(init->bblack, value, COLOR_LEN - 1);
                continue;
            }
            if (!strcmp(key, "bred")) {
                strnz__cpy(init->bred, value, COLOR_LEN - 1);
                continue;
            }
            if (!strcmp(key, "bgreen")) {
                strnz__cpy(init->bgreen, value, COLOR_LEN - 1);
                continue;
            }
            if (!strcmp(key, "byellow")) {
                strnz__cpy(init->byellow, value, COLOR_LEN - 1);
                continue;
            }
            if (!strcmp(key, "bblue")) {
                strnz__cpy(init->bblue, value, COLOR_LEN - 1);
                continue;
            }
            if (!strcmp(key, "bmagenta")) {
                strnz__cpy(init->bmagenta, value, COLOR_LEN - 1);
                continue;
            }
            if (!strcmp(key, "bcyan")) {
                strnz__cpy(init->bcyan, value, COLOR_LEN - 1);
                continue;
            }
            if (!strcmp(key, "bwhite")) {
                strnz__cpy(init->bwhite, value, COLOR_LEN - 1);
                continue;
            }
            if (!strcmp(key, "borange")) {
                strnz__cpy(init->borange, value, COLOR_LEN - 1);
                continue;
            }
            if (!strcmp(key, "bg")) {
                strnz__cpy(init->bg, value, COLOR_LEN - 1);
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

void prompt_int_to_str(char *s, int prompt_type) {
    switch (prompt_type) {
    case PT_SHORT:
        strcpy(s, "short");
        break;
    case PT_LONG:
        strcpy(s, "long");
        break;
    case PT_STRING:
        strcpy(s, "string");
        break;
    default:
        strcpy(s, "none");
        break;
    }
}
// ╭────────────────────────────────────────────────────────────────╮
// │ WRITE_CONFIG                                                   │
// ╰────────────────────────────────────────────────────────────────╯
int write_config(Init *init) {
    char *e;
    char minitrc_dmp[MAXLEN];

    e = getenv("HOME");
    if (e) {
        strcpy(minitrc_dmp, e);
        strcat(minitrc_dmp, "/");
        strcat(minitrc_dmp, "menuapp/minitrc.dmp");
    } else {
        strcpy(minitrc_dmp, "./minitrc.dmp");
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
    (void)fprintf(minitrc_fp, "%s=%s\n", "black", init->black);
    (void)fprintf(minitrc_fp, "%s=%s\n", "red", init->red);
    (void)fprintf(minitrc_fp, "%s=%s\n", "green", init->green);
    (void)fprintf(minitrc_fp, "%s=%s\n", "yellow", init->yellow);
    (void)fprintf(minitrc_fp, "%s=%s\n", "blue", init->blue);
    (void)fprintf(minitrc_fp, "%s=%s\n", "magenta", init->magenta);
    (void)fprintf(minitrc_fp, "%s=%s\n", "cyan", init->cyan);
    (void)fprintf(minitrc_fp, "%s=%s\n", "white", init->white);
    (void)fprintf(minitrc_fp, "%s=%s\n", "bblack", init->bblack);
    (void)fprintf(minitrc_fp, "%s=%s\n", "bred", init->bred);
    (void)fprintf(minitrc_fp, "%s=%s\n", "bgreen", init->bgreen);
    (void)fprintf(minitrc_fp, "%s=%s\n", "byellow", init->byellow);
    (void)fprintf(minitrc_fp, "%s=%s\n", "bblue", init->bblue);
    (void)fprintf(minitrc_fp, "%s=%s\n", "bmagenta", init->bmagenta);
    (void)fprintf(minitrc_fp, "%s=%s\n", "bcyan", init->bcyan);
    (void)fprintf(minitrc_fp, "%s=%s\n", "bwhite", init->bwhite);
    (void)fprintf(minitrc_fp, "%s=%s\n", "bg_color",
                  colors_text[init->bg_color]);
    (void)fprintf(minitrc_fp, "%s=%s\n", "fg_color",
                  colors_text[init->fg_color]);
    (void)fprintf(minitrc_fp, "%s=%s\n", "bo_color",
                  colors_text[init->bo_color]);
    (void)fprintf(minitrc_fp, "%s=%0.2f\n", "red_gamma", init->red_gamma);
    (void)fprintf(minitrc_fp, "%s=%0.2f\n", "green_gamma", init->green_gamma);
    (void)fprintf(minitrc_fp, "%s=%0.2f\n", "blue_gamma", init->blue_gamma);
    (void)fprintf(minitrc_fp, "%s=%s\n", "f_at_end_clear",
                  init->f_at_end_clear ? "true" : "false");
    (void)fprintf(minitrc_fp, "%s=%s\n", "f_at_end_remove",
                  init->f_at_end_remove ? "true" : "false");
    (void)fprintf(minitrc_fp, "%s=%s\n", "f_erase_remainder",
                  init->f_erase_remainder ? "true" : "false");
    (void)fprintf(minitrc_fp, "%s=%s\n", "f_brackets",
                  init->f_brackets ? "true" : "false");
    (void)fprintf(minitrc_fp, "%s=%s\n", "fill_char", init->fill_char);
    (void)fprintf(minitrc_fp, "%s=%s\n", "f_ignore_case",
                  init->f_ignore_case ? "true" : "false");
    (void)fprintf(minitrc_fp, "%s=%s\n", "f_squeeze",
                  init->f_squeeze ? "true" : "false");
    (void)fprintf(minitrc_fp, "%s=%s\n", "f_stop_on_error",
                  init->f_stop_on_error ? "true" : "false");
    (void)fprintf(minitrc_fp, "%s=%d\n", "tab_stop", init->tab_stop);
    prompt_int_to_str(tmp_str, init->prompt_type);
    (void)fprintf(minitrc_fp, "%s=%s\n", "prompt_type", tmp_str);
    (void)fprintf(minitrc_fp, "%s=%s\n", "prompt_str", init->prompt_str);
    (void)fprintf(minitrc_fp, "%s=%s\n", "view_cmd", init->view_cmd);
    (void)fprintf(minitrc_fp, "%s=%s\n", "view_cmd_all", init->view_cmd_all);
    (void)fprintf(minitrc_fp, "%s=%s\n", "provider_cmd", init->provider_cmd);
    (void)fprintf(minitrc_fp, "%s=%s\n", "receiver_cmd", init->receiver_cmd);
    (void)fprintf(minitrc_fp, "%s=%s\n", "title", init->title);
    (void)fprintf(minitrc_fp, "%s=%d\n", "select_max", init->select_max);
    (void)fprintf(minitrc_fp, "%s=%s\n", "black", init->black);
    (void)fprintf(minitrc_fp, "%s=%s\n", "bg", init->bg);
    (void)fprintf(minitrc_fp, "%s=%s\n", "abg", init->abg);
    (void)fprintf(minitrc_fp, "%s=%s\n", "mapp_spec", init->mapp_spec);
    (void)fprintf(minitrc_fp, "%s=%s\n", "help_spec", init->help_spec);
    (void)fprintf(minitrc_fp, "%s=%s\n", "in_spec", init->in_spec);
    (void)fprintf(minitrc_fp, "%s=%s\n", "out_spec", init->out_spec);
    (void)fprintf(minitrc_fp, "%s=%s\n", "mapp_data", init->mapp_data);
    (void)fprintf(minitrc_fp, "%s=%s\n", "mapp_help", init->mapp_help);
    (void)fprintf(minitrc_fp, "%s=%s\n", "mapp_home", init->mapp_home);
    (void)fprintf(minitrc_fp, "%s=%s\n", "mapp_msrc", init->mapp_msrc);
    (void)fprintf(minitrc_fp, "%s=%s\n", "mapp_user", init->mapp_user);
    (void)fclose(minitrc_fp);
    strcpy(tmp_str, "Configuration written to file: ");
    strcat(tmp_str, minitrc_dmp);
    Perror(tmp_str);
    return 0;
}
// ╭────────────────────────────────────────────────────────────────╮
// │ DERIVE_FILE_SPEC                                               │
// ╰────────────────────────────────────────────────────────────────╯
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

void display_version() { fprintf(stderr, "\nVersion %s\n", mapp_version); }

void usage() {
    (void)fprintf(stderr, "\n\nPress any key to continue...");
    di_getch();
}

void opt_prt_char(const char *o, const char *name, const char *value) {
    fprintf(stdout, "%3s %-15s: %s\n", o, name, value);
}

void opt_prt_str(const char *o, const char *name, const char *value) {
    fprintf(stdout, "%3s %-15s: %s\n", o, name, value);
}

void opt_prt_int(const char *o, const char *name, int value) {
    fprintf(stdout, "%3s %-15s: %d\n", o, name, value);
}

void opt_prt_double(const char *o, const char *name, double value) {
    fprintf(stdout, "%3s %-15s: %0.2f\n", o, name, value);
}
void opt_prt_bool(const char *o, const char *name, bool value) {
    fprintf(stdout, "%3s %-15s: %s\n", o, name, value ? "true" : "false");
}
// ╭────────────────────────────────────────────────────────────────╮
// │ DUMP_CONFIG                                                    │
// ╰────────────────────────────────────────────────────────────────╯
void dump_config(Init *init, char *msg) {
    opt_prt_str("-a:", "--minitrc", init->minitrc);
    opt_prt_int("-C:", "--cols", init->cols);
    opt_prt_int("-L:", "--lines", init->lines);
    opt_prt_int("-X:", "--begx", init->begx);
    opt_prt_int("-Y:", "--begy", init->begy);
    opt_prt_int("-B:", "--bg_color", init->bg_color);
    opt_prt_int("-F:", "--fg_color", init->fg_color);
    opt_prt_int("-O:", "--bo_color", init->bo_color);
    opt_prt_double("-r:", "  red_gamma", init->red_gamma);
    opt_prt_double("-g:", "  green_gamma", init->blue_gamma);
    opt_prt_double("-b:", "  blue_gamma", init->green_gamma);
    opt_prt_bool("-z ", "  f_at_end_clear", init->f_at_end_clear);
    opt_prt_bool("-y:", "  f_at_end_remove", init->f_at_end_remove);
    opt_prt_bool("-e:", "  f_erase_remainder", init->f_erase_remainder);
    opt_prt_bool("-u", "  f_brackets", init->f_brackets);
    opt_prt_str("-f:", "  fill_char", init->fill_char);
    opt_prt_bool("-x:", "--f_ignore_case", init->f_ignore_case);
    opt_prt_bool("-s ", "--f_squeeze", init->f_squeeze);
    opt_prt_bool("-Z ", "--f_stop_on_error", init->f_stop_on_error);
    opt_prt_int("-t:", "--tab_stop", init->tab_stop);
    prompt_int_to_str(tmp_str, init->prompt_type);
    opt_prt_str("-P:", "--promp_type", tmp_str);
    opt_prt_int("-n:", "--select_max", init->select_max);
    opt_prt_str("-c:", "--view_cmd", init->provider_cmd);
    opt_prt_str("-A:", "--view_cmd_all", init->provider_cmd);
    opt_prt_str("-S:", "--provider_cmd", init->provider_cmd);
    opt_prt_str("-R:", "--receiver_cmd", init->receiver_cmd);
    opt_prt_str("-T:", "--title", init->title);
    opt_prt_str("   ", "--black", init->black);
    opt_prt_str("   ", "--red", init->red);
    opt_prt_str("   ", "--green", init->green);
    opt_prt_str("   ", "--yellow", init->yellow);
    opt_prt_str("   ", "--blue", init->blue);
    opt_prt_str("   ", "--magenta", init->magenta);
    opt_prt_str("   ", "--cyan", init->cyan);
    opt_prt_str("   ", "--white", init->white);
    opt_prt_str("   ", "--orange", init->orange);
    opt_prt_str("   ", "--bblack", init->bblack);
    opt_prt_str("   ", "--bred", init->bred);
    opt_prt_str("   ", "--bgreen", init->bgreen);
    opt_prt_str("   ", "--byellow", init->byellow);
    opt_prt_str("   ", "--bblue", init->bblue);
    opt_prt_str("   ", "--bmagenta", init->bmagenta);
    opt_prt_str("   ", "--bcyan", init->bcyan);
    opt_prt_str("   ", "--bwhite", init->bwhite);
    opt_prt_str("   ", "--borange", init->borange);
    opt_prt_str("   ", "--bg", init->bg);
    opt_prt_str("   ", "--abg", init->abg);
    opt_prt_str("-H:", "--help_spec", init->help_spec);
    opt_prt_str("-i:", "--in_spec", init->in_spec);
    opt_prt_str("-d:", "--mapp_spec", init->mapp_spec);
    opt_prt_str("-o:", "--out_spec", init->out_spec);
    opt_prt_str("   ", "--mapp_data", init->mapp_data);
    opt_prt_str("   ", "--mapp_help", init->mapp_help);
    opt_prt_str("-m:", "--mapp_home", init->mapp_home);
    opt_prt_str("   ", "--mapp_msrc", init->mapp_msrc);
    opt_prt_str("-u:", "--mapp_user", init->mapp_user);
    (void)fprintf(stderr, "\n%s\n\n", msg);
}
