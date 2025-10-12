/* initialization.c
 * Bill Waller
 * billxwaller@gmail.com
 */

#include "menu.h"
#include <errno.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* ~/minitrc

  all  c: d: e: g: h i: n: o: s: t: v w x y z D B: F: O: P: V Z
  init c: d: e: g: h i: n: o:       v w       D B: F: O:    V Z
  view c: d: e: g: h i: n: o: s: t: v w x y z D B: F: O: P: V Z
>   c: command executable                   cmd_str
>   d: application directory                mapp_dir
>   e  menu application description file    mapp_desc
>   g: lllcccLLLCCC (lines,cols,begy,begx)  geometry
>   h  display command line help            f_help
>   i: input file_name                      in_file
>   n: number of selections                 selections
>   o: output file_name                     out_file
    s  squeeze multiple blank lines         f_squeeze
    t: number of spaces in tabs             tab_stops
>   v  version                              MENU_VERSION
>   w  write configuration
    x  ignore case in search                f_ignore_case
    y  remove file at end of program        f_at_end_remove
    z  clear screen at startup              f_at_end_clear
>   D  dump configuration
>   B: background color                     bg_color
>   F: foreground color                     fg_color
>   O: border color                         bo_color
    P: {S-Short, L-Long, N-None}[string]    prompt_style
>   V  version                              MENU_VERSION
>   Z  (undocumented) stop on error         f_stop_on_error
    +: execute command on startup           startup_cmd

*/

bool f_write_config = FALSE;
bool f_dump_config = FALSE;
bool f_help = FALSE;
bool f_version = FALSE;
bool f_debug = FALSE;

const char mapp_version[20] = "0.5.1";
const char PgmID[] = "init.c";

int write_config();
void display_version();

/* Globals */

// GLOBL INITVARS - DEFAULT VALUES
char minitrc[MAXLEN];
opt *option;

int initialization(int, char **);
int opt_process_cmdline(int, char **);
int parse_config_minitrc();
int derive_mapp_spec(char *, char *, char *);
void dump_config(char *);
void Usage();

int initialization(int argc, char **argv) {

    setlocale(LC_ALL, "en_US.UTF-8");

    // initialize option struct

    option = (opt *)malloc(sizeof(opt));
    if (!option)
        abend(-1, "malloc failed (option)");
    strcpy(minitrc, "~/.minitrc");
    strcpy(option->mapp_dir, "~/menuapp");
    strcpy(option->mapp_desc, "main.m");
    strcpy(option->mapp_spec, "~/menuapp/main.m");
    option->cmd_str[0] = '\0';
    option->in_file[0] = '\0';
    option->out_file[0] = '\0';
    option->selections = 0;
    option->fg_color = FG_COLOR;
    option->bg_color = BG_COLOR;
    option->bo_color = BO_COLOR;
    parse_config_minitrc();
    if (f_debug)
        dump_config("Configuration after parse_config_minitrc");
    opt_process_cmdline(argc, argv);
    derive_mapp_spec(option->mapp_spec, option->mapp_dir, option->mapp_desc);
    if (f_dump_config) {
        dump_config(
            "Configuration after parse_config_minitrc and opt_process_cmdline");
        exit(0);
    } else if (f_debug)
        dump_config("Configuration after opt_process_cmdline");
    if (f_write_config) {
        write_config();
        exit(0);
    }
    if (f_help) {
        display_version();
        Usage();
        dump_config("Current configuration");
        exit(0);
    }
    if (f_version) {
        display_version();
        exit(0);
    }
    if (f_write_config)
        write_config();
    return (0);
}

int opt_process_cmdline(int argc, char **argv) {
    int opt;
    char *optstring = "c:d:e:g:hi:n:o:s:t:vwxyzDB:F:O:P:VZ";
    while ((opt = getopt(argc, argv, optstring)) != -1) {
        switch (opt) {
        case 'c':
            strncpy(option->cmd_str, optarg, MAXLEN);
            break;
        case 'd':
            strncpy(option->mapp_dir, optarg, MAXLEN);
            break;
        case 'e':
            strncpy(option->mapp_desc, optarg, MAXLEN);
            break;
        case 'g':
            parse_geometry_str(optarg, &lines, &cols, &begx, &begy);
            break;
        case 'h':
            f_help = TRUE;
            break;
        case 'i':
            strncpy(option->in_file, optarg, MAXLEN);
            break;
        case 'n':
            option->selections = atoi(optarg);
            break;
        case 'o':
            strncpy(option->out_file, optarg, MAXLEN);
            break;
        case 's':
            break;
        case 't':
            break;
        case 'v':
        case 'V':
            f_version = TRUE;
            break;
        case 'w':
            f_write_config = TRUE;
            break;
        case 'x':
            break;
        case 'y':
            break;
        case 'z':
            break;
        case 'D':
            f_dump_config = TRUE;
            break;
        case 'F':
            option->fg_color = get_color_number(optarg);
            break;
        case 'B':
            option->bg_color = get_color_number(optarg);
            break;
        case 'O':
            option->bo_color = get_color_number(optarg);
            break;
        case 'P':
            break;
        case 'Z':
            f_stop_on_error = TRUE;
            break;
        case '+':
            strncpy(option->cmd_str, optarg, MAXLEN - 1);
            break;
        default:
            break;
        }
    }
    return (0);
}

int parse_config_minitrc() {
    char *tp, *sp, *dp;
    char ts[MAXLEN];

    char *e = getenv("MINITRC");
    if (e)
        strcpy(ts, e);
    else
        strcpy(ts, minitrc);
    tp = ts;
    if (*tp == '~') {
        tp++;
        while (*tp == '/')
            tp++;
        e = getenv("HOME");
        if (e) {
            strcpy(minitrc, e);
            strcat(minitrc, "/");
            strcat(minitrc, tp);
        }
    }

    FILE *config_fp = fopen(minitrc, "r");
    if (!config_fp) {
        fprintf(stderr, "failed to read file: %s\n", minitrc);
        return (-1);
    }

    while (fgets(ts, sizeof(ts), config_fp)) {
        if (ts[0] != '#') {
            sp = ts;
            dp = tmp_str;
            while (*sp != '\0') {
                if (*sp == '\n')
                    *dp = *sp = '\0';
                else {
                    if (*sp != '"' && *sp != ' ' && *sp != ';')
                        *dp++ = *sp;
                    sp++;
                }
            }
            char *key = strtok(tmp_str, "=");
            char *value = strtok(NULL, "=");
            if (!strcmp(key, "mapp_dir"))
                strcpy(option->mapp_dir, value);
            if (!strcmp(key, "mapp_desc"))
                strcpy(option->mapp_desc, value);
            if (!strcmp(key, "fg_color"))
                option->fg_color = get_color_number(value);
            if (!strcmp(key, "bg_color"))
                option->bg_color = get_color_number(value);
            if (!strcmp(key, "bo_color"))
                option->bo_color = get_color_number(value);
        }
    }
    fclose(config_fp);
    return 0;
}

int write_config() {
    char *e;
    char minitrc_file[MAXLEN];
    e = getenv("HOME");
    if (e) {
        strcpy(minitrc_file, e);
        strcat(minitrc_file, "/");
        strcat(minitrc_file, "menuapp/minitrc.dmp");
    }

    FILE *minitrc_fp = fopen(minitrc_file, "w");
    if (minitrc_fp == (FILE *)0) {
        fprintf(stderr, "failed to open file: %s\n", minitrc_file);
        return (-1);
    }
    fprintf(minitrc_fp, "# %s\n", "~/.minitrc");
    fprintf(minitrc_fp, "%s=%s\n", "mapp_dir", option->mapp_dir);
    fprintf(minitrc_fp, "%s=%s\n", "mapp_desc", option->mapp_desc);
    strcpy(tmp_str, colors_text[option->fg_color]);
    fprintf(minitrc_fp, "%s=%s\n", "fg_color", tmp_str);
    strcpy(tmp_str, colors_text[option->bg_color]);
    fprintf(minitrc_fp, "%s=%s\n", "bg_color", tmp_str);
    strcpy(tmp_str, colors_text[option->bo_color]);
    fprintf(minitrc_fp, "%s=%s\n", "bo_color", tmp_str);
    fclose(minitrc_fp);
    strcpy(tmp_str, "Configuration written to file: ");
    strcat(tmp_str, minitrc_file);
    display_error_message(tmp_str);
    return 0;
}

int derive_mapp_spec(char *file_spec, char *dir, char *file_name) {
    char ts[MAXLEN];
    char ts2[MAXLEN];
    char err_msg[MAXLEN];
    char *tp;
    char *e;

    // choose the directory wisely - 1 command line, 2 env, 3 default
    if (dir[0])
        strncpy(ts, dir, MAXARGS - 1);
    else {
        e = getenv("MAPP_DIR");
        if (e) {
            strncpy(ts, e, MAXARGS - 1);
        } else
            strncpy(ts, "~/menuapp", MAXARGS - 1);
    }
    // replace ~ with $HOME
    tp = ts;
    if (*tp == '~') {
        tp++;
        while (*tp == '/')
            tp++;
        e = getenv("HOME");
        if (e) {
            strncpy(ts2, e, MAXLEN - 1);
            strncat(ts2, "/", MAXLEN - 1);
            strncat(ts2, tp, MAXLEN - 1);
        }
    } else
        strncpy(ts2, ts, MAXLEN - 1);
    // construct the full file specification
    // check that the file exists and is readable
    int sl;
    strncpy(file_spec, ts2, MAXLEN - 1);
    sl = strlen(file_spec);
    if (file_spec[sl] != '/')
        strncat(file_spec, "/", MAXLEN - 1);
    strncat(file_spec, file_name, MAXLEN - 1);
    if (access(file_spec, R_OK)) {
        strncpy(err_msg, "unable to open file: ", MAXLEN - 1);
        strncat(err_msg, file_name, MAXLEN - 1);
        strncat(err_msg, " in directory: ", MAXLEN - 1);
        strncat(err_msg, ts2, MAXLEN - 1);
        strncat(err_msg, " - ", MAXLEN - 1);
        strncat(err_msg, strerror(errno), MAXLEN - 1);
        display_error_message(err_msg);
        // now clobber the file_spec to indicate failure
        *file_spec = '\0';
        return (-1);
    }
    return (0);
}

void display_version() { fprintf(stderr, "\nVersion %s\n", mapp_version); }

void Usage() {
    fprintf(stderr, "\n':' indicates flag requires an option\n\n");
    fprintf(stderr, "menuapp  c: d: e: g: h i: o: s: v w D F: B: O: V\n");
    fprintf(stderr, "menu         d: e:    h          v w D F: B: O: V\n");
    fprintf(stderr, "cpick     c: d: e: g: h i: o: s: v w D F: B: O: V\n");
    fprintf(stderr, "paint     c: d: e: g: h i: o:    v w D F: B: O: V\n");
    fprintf(stderr, "paintfile c: d: e: g: h i: o:    v w D F: B: O: V\n\n");
    fprintf(stderr, "    -c command\n");
    fprintf(stderr, "    -d mapp directory\n");
    fprintf(stderr, "    -e description file (main.m)\n");
    fprintf(stderr, "    -g geometry            (lllcccLLLCCC)\n");
    fprintf(stderr, "    -h                     (help)\n");
    fprintf(stderr, "    -i input-file-name\n");
    fprintf(stderr, "    -n number-of-selections\n");
    fprintf(stderr, "    -o output-file-name\n");
    fprintf(stderr, "    -v version\n");
    fprintf(stderr, "    -w write configuration\n");
    fprintf(stderr, "    -D dump configuration\n");
    fprintf(stderr, "    -F foreground-color\n");
    fprintf(stderr, "    -B background-color\n");
    fprintf(stderr, "    -O border-color\n");
    fprintf(stderr, "    -V version\n");

    fprintf(stderr, "\nColors:\n");
    list_colors();
}

void dump_config(char *msg) {
    fprintf(stderr, "\n%s\n\n", msg);
    fprintf(stderr, "minitrc   = %s\n", minitrc);
    fprintf(stderr, "-d mapp_dir  = %s\n", option->mapp_dir);
    fprintf(stderr, "-e mapp_desc = %s\n", option->mapp_desc);
    fprintf(stderr, "-F fg_color  = %s\n", colors_text[option->fg_color]);
    fprintf(stderr, "-B bg_color  = %s\n", colors_text[option->bg_color]);
    fprintf(stderr, "-O bo_color  = %s\n\n", colors_text[option->bo_color]);
}
