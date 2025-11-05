/* initialization.c
 * Bill Waller
 * billxwaller@gmail.com
 */

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
    ANSWER_SPEC,
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
void parse_opt_args(Init *, int, char **);
int parse_config(Init *);
void dump_config(Init *, char *);
void usage();

char *tilde_expand(char *);
bool derive_file_spec(char *, char *, char *);
int executor = 0;

/* ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
   ┃ MAPP_INITIALIZATION                                   ┃
   ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛ */
void mapp_initialization(Init *init, int argc, char **argv) {
    setlocale(LC_ALL, "en_US.UTF-8");
    if (!init) {
        snprintf(tmp_str, sizeof(tmp_str), "%s",
                 "init struct not allocated on entry");
        abend(-1, tmp_str);
    }
    if (init->minitrc[0] == '\0')
        strncpy(init->minitrc, "~/.minitrc", MAXLEN - 1);
    init->bg_color = BG_COLOR;      // B: background color
    init->fg_color = FG_COLOR;      // F: foreground color
    init->bo_color = BO_COLOR;      // O: border colorZ
    init->help_spec[0] = '\0';      // H: help spec
    init->f_at_end_clear = true;    // z  clear screen on exit
    init->f_erase_remainder = true; // e  erase remainder on enter
    strncpy(init->prompt, "S", MAXLEN - 1);
    strncpy(init->mapp_home, "~/menuapp", MAXLEN - 1);
    strncpy(init->mapp_user, "~/menuapp/user", MAXLEN - 1);
    strncpy(init->mapp_msrc, "~/menuapp/msrc", MAXLEN - 1);
    strncpy(init->mapp_data, "~/menuapp/data", MAXLEN - 1);
    strncpy(init->mapp_help, "~/menuapp/help", MAXLEN - 1);

    /*
        Initialization Priorities

        5  def_args   Default values
        4  cfg_args - Configuration file
        3  env_args - Environment variables
        2  pos_args - Command line positional arguments
        1  opt_args - Command line option arguments
     */

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
        usage();
    }
    if (f_version) {
        display_version();
    }
    if (f_write_config)
        write_config(init);
}

/* ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
   ┃ PARSE_OPT_ARGS                                        ┃
   ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛ */
void parse_opt_args(Init *init, int argc, char **argv) {
    int i;
    int opt;
    int longindex = 0;
    int flag = 0;
    char *optstring = "a:c:d:g:hi:m:n:o:rst:vwxzA:B:C:DF:H:L:MO:P:S:T:U:VX:Y:Z";
    struct option long_options[] = {{"answer_spec", 1, &flag, ANSWER_SPEC},
                                    {"mapp_data", 1, &flag, MAPP_DATA},
                                    {"mapp_spec", 1, &flag, MAPP_SPEC},
                                    {"mapp_help", 1, &flag, MAPP_HELP},
                                    {"help_spec", 1, &flag, HELP_SPEC},
                                    {"in_spec", 1, &flag, IN_SPEC},
                                    {"out_spec", 1, &flag, OUT_SPEC},
                                    {"mapp_msrc", 1, &flag, MAPP_MSRC},
                                    {0, 0, 0, 0}};
    char mapp[MAXLEN];
    base_name(mapp, argv[0]);
    optind = 1;
    while ((opt = getopt_long(argc, argv, optstring, long_options,
                              &longindex)) != -1) {
        switch (opt) {
        case 0:
            switch (flag) {
            case ANSWER_SPEC:
                strncpy(init->answer_spec, optarg, MAXLEN - 1);
                break;
            case MAPP_DATA:
                strncpy(init->mapp_data, optarg, MAXLEN - 1);
                break;
            case MAPP_SPEC:
                strncpy(init->mapp_spec, optarg, MAXLEN - 1);
                break;
            case MAPP_HELP:
                strncpy(init->mapp_help, optarg, MAXLEN - 1);
                break;
            case HELP_SPEC:
                strncpy(init->help_spec, optarg, MAXLEN - 1);
                break;
            case IN_SPEC:
                strncpy(init->in_spec, optarg, MAXLEN - 1);
                break;
            case OUT_SPEC:
                strncpy(init->out_spec, optarg, MAXLEN - 1);
                break;
            case MAPP_MSRC:
                strncpy(init->mapp_msrc, optarg, MAXLEN - 1);
                break;
            default:
                break;
            }
            break;
        case 'a':
            strncpy(init->minitrc, optarg, MAXLEN - 1);
            break;
        case 'c':
            strncpy(init->cmd_spec, optarg, MAXLEN - 1);
            break;
        case 'd':
            strncpy(init->mapp_spec, optarg, MAXLEN - 1);
            break;
        case 'e':
            init->f_erase_remainder = true;
            break;
        case 'h':
            f_help = true;
            break;
        case 'i':
            strncpy(init->in_spec, optarg, MAXLEN - 1);
            break;
        case 'm':
            strncpy(init->mapp_home, optarg, MAXLEN - 1);
            break;
        case 'n':
            init->selections = atoi(optarg);
            break;
        case 'o':
            strncpy(init->out_spec, optarg, MAXLEN - 1);
            break;
        case 'r':
            init->f_at_end_remove = true;
            break;
        case 's':
            init->f_squeeze = true;
            break;
        case 't':
            init->tab_stop = atoi(optarg);
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
        case 'A':
            strncpy(init->answer_spec, optarg, MAXLEN - 1);
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
        case 'F':
            init->fg_color = get_color_number(optarg);
            break;
        case 'H':
            strncpy(init->help_spec, optarg, MAXLEN - 1);
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
            strncpy(init->prompt, optarg, MAXLEN - 1);
            break;
        case 'S':
            strncpy(init->start_cmd, optarg, MAXLEN - 1);
            break;
        case 'T':
            strncpy(init->title, optarg, MAXLEN - 1);
            break;
        case 'U':
            strncpy(init->mapp_user, optarg, MAXLEN - 1);
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
    i = 1;
    while (optind < argc)
        init->argv[i++] = strdup(argv[optind++]);
    init->argv[i] = NULL;
    init->argc = i;
    argc = i;
    return;
}

/* ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
   ┃ PARSE_CONFIG                                          ┃
   ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛ */
int parse_config(Init *init) {
    char ts[MAXLEN];
    char *sp, *dp;

    if (!init->minitrc[0]) {
        char *e = getenv("MINITRC");
        if (e)
            strncpy(init->minitrc, e, MAXLEN - 1);
        else
            strncpy(init->minitrc, "~/.minitrc", MAXLEN - 1);
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
            if (!strcmp(key, "minitrc"))
                strncpy(init->minitrc, value, MAXLEN - 1);
            if (!strcmp(key, "fg_color"))
                init->fg_color = get_color_number(value);
            if (!strcmp(key, "bg_color"))
                init->bg_color = get_color_number(value);
            if (!strcmp(key, "bo_color"))
                init->bo_color = get_color_number(value);
            if (!strcmp(key, "lines"))
                init->lines = atoi(value);
            if (!strcmp(key, "cols"))
                init->cols = atoi(value);
            if (!strcmp(key, "begy"))
                init->begy = atoi(value);
            if (!strcmp(key, "begx"))
                init->begx = atoi(value);
            if (!strcmp(key, "f_erase_remainder"))
                init->f_erase_remainder = str_to_bool(value);
            if (!strcmp(key, "f_at_end_remove"))
                init->f_at_end_remove = str_to_bool(value);
            if (!strcmp(key, "f_squeeze"))
                init->f_squeeze = str_to_bool(value);
            if (!strcmp(key, "f_ignore_case"))
                init->f_ignore_case = str_to_bool(value);
            if (!strcmp(key, "f_at_end_clear"))
                init->f_at_end_clear = str_to_bool(value);
            if (!strcmp(key, "f_stop_on_error"))
                init->f_stop_on_error = str_to_bool(value);
            if (!strcmp(key, "selections"))
                init->selections = atoi(value);
            if (!strcmp(key, "tab_stop"))
                init->tab_stop = atoi(value);
            if (!strcmp(key, "prompt"))
                strncpy(init->prompt, value, MAXLEN - 1);
            if (!strcmp(key, "start_cmd"))
                strncpy(init->start_cmd, value, MAXLEN - 1);
            if (!strcmp(key, "mapp_home"))
                strncpy(init->mapp_home, value, MAXLEN - 1);
            if (!strcmp(key, "mapp_data"))
                strncpy(init->mapp_data, value, MAXLEN - 1);
            if (!strcmp(key, "mapp_help"))
                strncpy(init->mapp_help, value, MAXLEN - 1);
            if (!strcmp(key, "mapp_msrc"))
                strncpy(init->mapp_msrc, value, MAXLEN - 1);
            if (!strcmp(key, "mapp_user"))
                strncpy(init->mapp_user, value, MAXLEN - 1);
        }
    }
    (void)fclose(config_fp);
    return 0;
}

/* ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
   ┃ WRITE_CONFIG                                          ┃
   ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛ */
int write_config(Init *init) {
    char *e;
    char minitrc_dmp[MAXLEN];

    e = getenv("HOME");
    if (e) {
        strcpy(minitrc_dmp, e);
        strcat(minitrc_dmp, "/");
        strcat(minitrc_dmp, "menuapp/minitrc.dmp");
    }

    FILE *minitrc_fp = fopen(minitrc_dmp, "w");
    if (minitrc_fp == (FILE *)0) {
        fprintf(stderr, "failed to open file: %s\n", minitrc_dmp);
        return (-1);
    }
    (void)fprintf(minitrc_fp, "# %s\n", "~/.minitrc");
    (void)fprintf(minitrc_fp, "%s=%s\n", "cmd_spec", init->cmd_spec);
    (void)fprintf(minitrc_fp, "%s=%s\n", "mapp_spec", init->mapp_spec);
    (void)fprintf(minitrc_fp, "%s=%s\n", "f_erase_remainder",
                  init->f_erase_remainder ? "true" : "false");
    (void)fprintf(minitrc_fp, "%s=%s\n", "in_spec", init->in_spec);
    (void)fprintf(minitrc_fp, "%s=%s\n", "mapp_home", init->mapp_home);
    (void)fprintf(minitrc_fp, "%s=%d\n", "selections", init->selections);
    (void)fprintf(minitrc_fp, "%s=%s\n", "out_spec", init->out_spec);
    (void)fprintf(minitrc_fp, "%s=%s\n", "f_at_end_remove",
                  init->f_at_end_remove ? "true" : "false");
    (void)fprintf(minitrc_fp, "%s=%s\n", "f_squeeze",
                  init->f_squeeze ? "true" : "false");
    (void)fprintf(minitrc_fp, "%s=%d\n", "tab_stop", init->tab_stop);
    (void)fprintf(minitrc_fp, "%s=%s\n", "mapp_user", init->mapp_user);
    (void)fprintf(minitrc_fp, "%s=%s\n", "f_ignore_case",
                  init->f_ignore_case ? "true" : "false");
    (void)fprintf(minitrc_fp, "%s=%s\n", "f_at_end_clear",
                  init->f_at_end_clear ? "true" : "false");
    (void)fprintf(minitrc_fp, "%s=%s\n", "answer_spec", init->answer_spec);
    (void)fprintf(minitrc_fp, "%s=%s\n", "bg_color",
                  colors_text[init->bg_color]);
    (void)fprintf(minitrc_fp, "%s=%d\n", "cols", init->cols);
    (void)fprintf(minitrc_fp, "%s=%s\n", "fg_color",
                  colors_text[init->fg_color]);
    (void)fprintf(minitrc_fp, "%s=%s\n", "help_spec", init->help_spec);
    (void)fprintf(minitrc_fp, "%s=%d\n", "lines", init->lines);
    (void)fprintf(minitrc_fp, "%s=%s\n", "bo_color",
                  colors_text[init->bo_color]);
    (void)fprintf(minitrc_fp, "%s=%s\n", "prompt", init->prompt);
    (void)fprintf(minitrc_fp, "%s=%s\n", "start_cmd", init->start_cmd);
    (void)fprintf(minitrc_fp, "%s=%s\n", "title", init->title);
    (void)fprintf(minitrc_fp, "%s=%d\n", "begx", init->begx);
    (void)fprintf(minitrc_fp, "%s=%d\n", "begy", init->begy);
    (void)fprintf(minitrc_fp, "%s=%s\n", "f_stop_on_error",
                  init->f_stop_on_error ? "true" : "false");
    (void)fprintf(minitrc_fp, "%s=%s\n", "answer_spec", init->answer_spec);
    (void)fprintf(minitrc_fp, "%s=%s\n", "mapp_data", init->mapp_data);
    (void)fprintf(minitrc_fp, "%s=%s\n", "mapp_spec", init->mapp_spec);
    (void)fprintf(minitrc_fp, "%s=%s\n", "mapp_help", init->mapp_help);
    (void)fprintf(minitrc_fp, "%s=%s\n", "help_spec", init->help_spec);
    (void)fprintf(minitrc_fp, "%s=%s\n", "in_spec", init->in_spec);
    (void)fprintf(minitrc_fp, "%s=%s\n", "out_spec", init->out_spec);
    (void)fprintf(minitrc_fp, "%s=%s\n", "mapp_msrc", init->mapp_msrc);

    (void)fclose(minitrc_fp);
    strcpy(tmp_str, "Configuration written to file: ");
    strcat(tmp_str, minitrc_dmp);
    display_error_message(tmp_str);
    return 0;
}

/* ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
   ┃ DERIVE_FILE_SPEC                                      ┃
   ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛ */
bool derive_file_spec(char *file_spec, char *dir, char *file_name) {
    char ts[MAXLEN];
    char ts2[MAXLEN];
    char *e;

    if (!file_name || !*file_name) {
        *file_spec = '\0';
        return false;
    }

    if (dir) {
        strncpy(ts, dir, MAXLEN - 1);
    } else {
        e = getenv("MAPP_DIR");
        if (e) {
            strncpy(ts, e, MAXLEN - 1);
        } else {
            strncpy(ts, "~/menuapp", MAXLEN - 1);
        }
    }
    trim_path(ts);
    strncpy(ts2, ts, MAXLEN - 1);
    // construct the full file specification
    // check that the file exists and is readable
    strncpy(file_spec, ts2, MAXLEN - 1);
    strncat(file_spec, "/", MAXLEN - 1);
    strncat(file_spec, file_name, MAXLEN - 1);
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

void opt_prt_int(const char *o, const char *name, int value) {
    fprintf(stdout, "%3s %-15s: %d\n", o, name, value);
}

void opt_prt_bool(const char *o, const char *name, bool value) {
    fprintf(stdout, "%3s %-15s: %s\n", o, name, value ? "true" : "false");
}

/* ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
   ┃ DUMP_CONFIG                                           ┃
   ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛ */
void dump_config(Init *init, char *msg) {
    opt_prt_char("-a:", "--minitrc", init->minitrc);
    opt_prt_char("-c:", "--cmd_spec", init->cmd_spec);
    opt_prt_char("-d:", "--mapp_spec", init->mapp_spec);
    opt_prt_bool("-e:", "--f_erase_remainder", init->f_erase_remainder);
    opt_prt_char("-i:", "--in_spec", init->in_spec);
    opt_prt_char("-m:", "--mapp_home", init->mapp_home);
    opt_prt_int("-n:", "--selections", init->selections);
    opt_prt_char("-o:", "--out_spec", init->out_spec);
    opt_prt_bool("-r:", "--f_at_end_remove", init->f_at_end_remove);
    opt_prt_bool("-s ", "--f_squeeze", init->f_squeeze);
    opt_prt_int("-t:", "--tab_stop", init->tab_stop);
    opt_prt_char("-u:", "--mapp_user", init->mapp_user);
    opt_prt_bool("-x:", "--f_ignore_case", init->f_ignore_case);
    opt_prt_bool("-z ", "--f_at_end_clear", init->f_at_end_clear);
    opt_prt_char("-A:", "--answer_spec", init->answer_spec);
    opt_prt_int("-B:", "--bg_color", init->bg_color);
    opt_prt_int("-C:", "--cols", init->cols);
    opt_prt_int("-F:", "--fg_color", init->fg_color);
    opt_prt_char("-H:", "--help_spec", init->help_spec);
    opt_prt_int("-L:", "--lines", init->lines);
    opt_prt_int("-O:", "--bo_color", init->bo_color);
    opt_prt_char("-P:", "--prompt", init->prompt);
    opt_prt_char("-S ", "--start_cmd", init->start_cmd);
    opt_prt_char("-T:", "--title", init->title);
    opt_prt_int("-X:", "--begx", init->begx);
    opt_prt_int("-Y:", "--begy", init->begy);
    opt_prt_bool("-Z ", "--f_stop_on_error", init->f_stop_on_error);
    opt_prt_char("   ", "--answer_spec", init->answer_spec);
    opt_prt_char("   ", "--mapp_data", init->mapp_data);
    opt_prt_char("   ", "--mapp_spec", init->mapp_spec);
    opt_prt_char("   ", "--mapp_help", init->mapp_help);
    opt_prt_char("   ", "--help_spec", init->help_spec);
    opt_prt_char("   ", "--in_spec", init->in_spec);
    opt_prt_char("   ", "--out_spec", init->out_spec);
    opt_prt_char("   ", "--mapp_msrc", init->mapp_msrc);
    (void)fprintf(stderr, "\n%s\n\n", msg);
}
