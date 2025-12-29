// #include "opts.h"
#include "menu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE(arr) (sizeof((arr)) / sizeof((arr)[0]))

void dump_opts_by_name();
void dump_opts_by_group();
void dump_opts_by_desc();
void dump_opts_by_use(char *, char *);
void sort_opts_by_name();
void sort_opts_by_group();
void sort_opts_by_desc();
int strnz__cpy(char *, char *, int);
static int comp_opt_desc(const void *, const void *);
static int comp_opt_group(const void *, const void *);
static int comp_opt_desc(const void *, const void *);
void dump_opts();
#define red "\033[0;31m"
#define green "\033[0;32m"
#define yellow "\033[0;33m"
#define blue "\033[0;34m"
#define magenta "\033[0;35m"
#define cyan "\033[0;36m"
#define white "\033[0;37m"
#define bblue "\033[34;1m"
#define reset "\033[0m"
Opts *select_opt(char *);

Opts opts[] = {
    {"minitrc", 0, 2, "mpfv", "a:", "configuration file spec"},
    {"lines", 1, 4, "mpfv", "C:", "width in columns"},
    {"cols", 1, 4, "mpfv", "L:", "height in lines"},
    {"begx", 1, 4, "mpfv", "X:", "begin on column"},
    {"begy", 1, 4, "mpfv", "Y:", "begin on line"},
    {"title", 0, 3, "mpfv", "T:", "title"},
    {"black", 0, 4, "mpfv", "", "black (#000000)"},
    {"red", 0, 4, "mpfv", "", "red (#bf0000)"},
    {"green", 0, 4, "mpfv", "", "green (#00cf00)"},
    {"yellow", 0, 4, "mpfv", "", "yellow (#efbf00)"},
    {"blue", 0, 4, "mpfv", "", "blue (#0000FF)"},
    {"magenta", 0, 4, "mpfv", "", "magenta (#9f009f)"},
    {"cyan", 0, 4, "mpfv", "", "cyan (#00dfdf)"},
    {"white", 0, 4, "mpfv", "", "white (#d0d0d0)"},
    {"orange", 0, 4, "mpfv", "", "orange (#FF5f00)"},
    {"bblack", 0, 4, "mpfv", "", "bright black (#7f7f7f)"},
    {"bred", 0, 4, "mpfv", "", "bright red (#FF3737)"},
    {"bgreen", 0, 4, "mpfv", "", "bright green (#00FF7f)"},
    {"byellow", 0, 4, "mpfv", "", "bright yellow (#FFeF00)"},
    {"bblue", 0, 4, "mpfv", "", "bright blue (#00cfFF)"},
    {"bmagenta", 0, 4, "mpfv", "", "bright magenta (#FF00FF)"},
    {"bcyan", 0, 4, "mpfv", "", "bright cyan (#00FFFF)"},
    {"bwhite", 0, 4, "mpfv", "", "bright white (#FFFFFF)"},
    {"borange", 0, 4, "mpfv", "", "bright orange (#FF7500)"},
    {"bg", 3, 6, "mpfv", "", "background (#000720)"},
    {"abg", 3, 6, "mpfv", "", "alternate background (#000f50)"},
    {"fg_color", 1, 4, "mpfv", "F:", "foreground_color"},
    {"bg_color", 1, 4, "mpfv", "B:", "background_color"},
    {"bo_color", 1, 4, "mpfv", "O:", "border_color"},
    {"red_gamma", 1, 4, "...v", "r:", "red_gamma (View)"},
    {"green_gamma", 1, 4, "...v", "g:", "green_gamma (View)"},
    {"blue_gamma", 1, 4, "...v", "b:", "blue_gamma (View)"},
    {"f_at_end_clear", 2, 5, "mpfv", "z", "clear screen at end of program"},
    {"f_at_end_remove", 2, 5, "...v", "y", "remove file at end of program"},
    {"f_erase_remainder", 2, 5, "..f.", "e",
     "erase remainder of line on enter"},
    {"f_ignore_case", 2, 5, "...v", "x", "ignore case in search"},
    {"f_squeeze", 2, 5, "...v", "s", "squeeze multiple blank lines"},
    {"f_mutiple_cmd_args", 2, 5, ".p..", "M", "multiple command arguments"},
    {"f_stop_on_error", 2, 5, "mpfv", "Z", "stop on error"},
    {"brackets", 2, 5, "..f.", "u:", "brackets around fields"},
    {"fill_char", 0, 4, "..f.", "f:", "field fill_char"},
    {"select_max", 1, 4, ".p..", "n:", "number of selections"},
    {"tab_stop", 1, 4, "...v", "t:", "number of spaces per tab"},
    {"prompt-type", 0, 3, "...v",
     "P:", "prompt (S-Short, L-Long, N-None)[string]"},
    {"prompt-str", 0, 3, "...v", "p:", "User supplied string"},
    {"view_cmd", 0, 3, "...v", "c:", "view cmd, first file"},
    {"view_cmd_all", 0, 3, "...v", "A:", "view cmd, all files"},
    {"receiver_cmd", 0, 3, "...v", "R:", "execute to receive piped output"},
    {"provider_cmd", 0, 3, "...v", "S:", "execute to provide piped input"},
    {"help_spec", 0, 0, "mpfv", "H:", "help spec"},
    {"in_spec", 0, 0, ".p..", "i:", "input spec"},
    {"out_spec", 0, 0, ".p..", "o:", "output spec"},
    {"mapp_spec", 0, 0, "mpfv", "d:", "description spec"},
    {"mapp_data", 0, 1, "mpfv", "", "data directory"},
    {"mapp_help", 0, 1, "mpfv", "", "help directory"},
    {"mapp_home", 0, 1, "mpfv", "m:", "home directory"},
    {"mapp_msrc", 0, 1, "mpfv", "", "source directory"},
    {"mapp_user", 0, 1, "mpfv", "u:", "user directory"},
    {"", 0, 0, "",
     ", "
     ""}}; // End marker

//  ╭───────────────────────────────────────────────────────────────────╮
static int comp_opt_name(const void *o1, const void *o2) {
    const Opts *opt1 = o1;
    const Opts *opt2 = o2;
    return strcmp(opt1->name, opt2->name);
}

static int comp_opt_group(const void *o1, const void *o2) {
    const Opts *opt1 = o1;
    const Opts *opt2 = o2;
    return (opt1->group - opt2->group);
}
static int comp_opt_desc(const void *o1, const void *o2) {
    const Opts *opt1 = o1;
    const Opts *opt2 = o2;
    return strcmp(opt1->desc, opt2->desc);
}

void dump_opts_by_desc() {
    sort_opts_by_desc();
    dump_opts();
}

void dump_opts_by_name() {
    sort_opts_by_name();
    dump_opts();
}

void dump_opts_by_group() {
    sort_opts_by_group();
    dump_opts();
}

void dump_opts() {
    // struct Opts *opts;
    char *type;
    char *group;
    qsort(opts, ARRAY_SIZE(opts), sizeof(opts[0]), comp_opt_desc);
    int i = 1;
    while (opts[i].name != NULL) {
        switch (opts[i].type) {
        case OT_STRING:
            type = "str";
            break;
        case OT_INT:
            type = "int";
            break;
        case OT_BOOL:
            type = "t/f";
            break;
        case OT_HEX:
            type = "hex";
            break;
        default:
            type = "unk";
        }
        switch (opts[i].group) {
        case OG_FILES:
            group = "file";
            break;
        case OG_DIRS:
            group = "dir";
            break;
        case OG_SPECS:
            group = "spec";
            break;
        case OG_MISC:
            group = "misc";
            break;
        case OG_PARMS:
            group = "prm";
            break;
        case OG_FLAGS:
            group = "flg";
            break;
        case OG_COL:
            group = "col";
            break;
        default:
            group = "unk";
        }
        printf("%02d %-18s %-7s %-4s %-5s %s\n", i, opts[i].name, type, group,
               opts[i].use, opts[i].desc);
        i++;
    }
}

void sort_opts_by_name() {
    qsort(opts, ARRAY_SIZE(opts), sizeof(opts[0]), comp_opt_name);
}

void sort_opts_by_group() {
    qsort(opts, ARRAY_SIZE(opts), sizeof(opts[0]), comp_opt_group);
}

void sort_opts_by_desc() {
    qsort(opts, ARRAY_SIZE(opts), sizeof(opts[0]), comp_opt_desc);
}

Opts *select_opt(char *name) {
    Opts *opt;
    Opts key;
    key.name = name;
    sort_opts_by_name();
    opt = bsearch(&key, opts, ARRAY_SIZE(opts), sizeof(opts[0]), comp_opt_name);
    return opt;
}

void dump_opts_by_use(char *usage, char *mask) {
    char *type;
    char *group;
    int i = 0;
    int j = 0;
    char c;

    printf("\n%s\n\n", usage);

    printf("   field name         type grp  mask opt description\n");
    printf(
        "   ------------------ ---- ---- ---- --- -------------------------\n");
    while (opts[i].name != NULL) {
        for (j = 0; j < 4; j++) {
            if (mask[j] != '.' && mask[j] == opts[i].use[j])
                break;
            else
                continue;
        }
        if (j == 4) {
            i++;
            continue;
        }
        switch (opts[i].type) {
        case OT_STRING:
            type = "str";
            break;
        case OT_INT:
            type = "int";
            break;
        case OT_BOOL:
            type = "bool";
            break;
        case OT_HEX:
            type = "hex";
            break;
        default:
            type = "   ";
        }
        switch (opts[i].group) {
        case OG_FILES:
            group = "file";
            break;
        case OG_DIRS:
            group = "dir";
            break;
        case OG_SPECS:
            group = "spec";
            break;
        case OG_MISC:
            group = "misc";
            break;
        case OG_PARMS:
            group = "prm";
            break;
        case OG_FLAGS:
            group = "flag";
            break;
        case OG_COL:
            group = "col";
            break;
        default:
            group = "   ";
        }
        if (opts[i].short_opt[0] == '\0')
            c = ' ';
        else
            c = '-';
        printf("%02d %s%-18s%s %-4s %-4s %4s %2s%1c%-3s%s%s%s%s\n", i, blue,
               opts[i].name, reset, type, group, opts[i].use, yellow, c,
               opts[i].short_opt, reset, green, opts[i].desc, reset);
        i++;
    }
}
