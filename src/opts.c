/** @file opts.c
    @brief Sort and print options
    @author Bill Waller
    Copyright (c) 2025
    MIT License
    billxwaller@gmail.com
    @date 2026-02-09
 */

#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE(arr) (sizeof((arr)) / sizeof((arr)[0]))

void dump_opts_by_name();
void dump_opts_by_group();
void dump_opts_by_desc();
void dump_opts_by_use(char *, char *);
void dump_opts_by_short_opt();
void sort_opts_by_name();
void sort_opts_by_group();
void sort_opts_by_desc();
void sort_opts_by_short_opt();
void print_opt_header();
void print_opt(int);
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

/** @struct Opts
 *  @brief Option structure
 *  @param name - option name
 *  @param group - option group
 *  @param type - option type
 *  @param use - option use mask
 *  @param short_opt - option short option
 *  @param desc - option description
 */
Opts opts[] = {
    {"minitrc", 0, 2, "mfpv", "a:", "configuration file spec"},
    {"blue_gamma", 1, 4, "...v", "b:", "blue_gamma (View)"},
    {"cmd", 0, 3, "...v", "c:", "view cmd, first file"},
    {"mapp_spec", 0, 0, "mfpv", "d:", "description spec"},
    {"f_erase_remainder", 2, 5, ".f..", "e",
     "erase remainder of line on enter"},
    {"fill_char", 0, 4, "..p.", "f:", "field fill_char"},
    {"green_gamma", 1, 4, "...v", "g:", "green_gamma (View)"},
    {"help", 2, 5, "mfpv", "h", "help"},
    {"in_spec", 0, 0, "..p.", "i:", "input spec"},
    {"mapp_home", 0, 1, "mfpv", "m:", "home directory"},
    {"select_max", 1, 4, ".f..", "n:", "number of selections"},
    {"out_spec", 0, 0, "..p.", "o:", "output spec"},
    {"prompt-str", 0, 3, "...v", "p:", "User supplied string"},
    {"red_gamma", 1, 4, "...v", "r:", "red_gamma (View)"},
    {"f_squeeze", 2, 5, "...v", "s", "squeeze multiple blank lines"},
    {"tab_stop", 1, 4, "...v", "t:", "number of spaces per tab"},
    {"brackets", 0, 4, "..p.", "u:", "brackets around fields"},
    {"f_ignore_case", 2, 5, "...v", "x", "ignore case in search"},
    {"f_at_end_remove", 2, 5, "...v", "y", "remove file at end of program"},
    {"f_at_end_clear", 2, 5, "mfpv", "z", "clear screen at end of program"},
    {"cmd_all", 0, 3, "...v", "A:", "view cmd, all files"},
    {"bg_color", 1, 4, "mfpv", "B:", "background_color"},
    {"lines", 1, 4, "mfpv", "C:", "width in columns"},
    {"fg_color", 1, 4, "mfpv", "F:", "foreground_color"},
    {"help_spec", 0, 0, "mfpv", "H:", "help spec"},
    {"cols", 1, 4, "mfpv", "L:", "height in lines"},
    {"f_mutiple_cmd_args", 2, 5, "..p.", "M", "multiple command arguments"},
    {"bo_color", 1, 4, "mfpv", "O:", "border_color"},
    {"prompt-type", 0, 3, "...v",
     "P:", "prompt (S-Short, L-Long, N-None)[string]"},
    {"receiver_cmd", 0, 3, "...v", "R:", "execute receiver of piped output"},
    {"provider_cmd", 0, 3, "...v", "S:", "execute provider of piped input"},
    {"title", 0, 3, "mfpv", "T:", "title"},
    {"mapp_user", 0, 1, "mfpv", "U:", "user directory"},
    {"begx", 1, 4, "mfpv", "X:", "begin on column"},
    {"begy", 1, 4, "mfpv", "Y:", "begin on line"},
    {"f_stop_on_error", 2, 5, "mfpv", "Z", "stop on error"},

    {"black", 0, 4, "mfpv", "", "black (#000000)"},
    {"red", 0, 4, "mfpv", "", "red (#bf0000)"},
    {"green", 0, 4, "mfpv", "", "green (#00cf00)"},
    {"yellow", 0, 4, "mfpv", "", "yellow (#efbf00)"},
    {"blue", 0, 4, "mfpv", "", "blue (#0000FF)"},
    {"magenta", 0, 4, "mfpv", "", "magenta (#9f009f)"},
    {"cyan", 0, 4, "mfpv", "", "cyan (#00dfdf)"},
    {"white", 0, 4, "mfpv", "", "white (#d0d0d0)"},
    {"orange", 0, 4, "mfpv", "", "orange (#FF5f00)"},
    {"bblack", 0, 4, "mfpv", "", "bright black (#7f7f7f)"},
    {"bred", 0, 4, "mfpv", "", "bright red (#FF3737)"},
    {"bgreen", 0, 4, "mfpv", "", "bright green (#00FF7f)"},
    {"byellow", 0, 4, "mfpv", "", "bright yellow (#FFeF00)"},
    {"bblue", 0, 4, "mfpv", "", "bright blue (#00cfFF)"},
    {"bmagenta", 0, 4, "mfpv", "", "bright magenta (#FF00FF)"},
    {"bcyan", 0, 4, "mfpv", "", "bright cyan (#00FFFF)"},
    {"bwhite", 0, 4, "mfpv", "", "bright white (#FFFFFF)"},
    {"borange", 0, 4, "mfpv", "", "bright orange (#FF7500)"},
    {"bg", 3, 6, "mfpv", "", "background (#000720)"},
    {"abg", 3, 6, "mfpv", "", "alternate background (#000f50)"},
    {"mapp_data", 0, 1, "mfpv", "", "data directory"},
    {"mapp_help", 0, 1, "mfpv", "", "help directory"},
    {"mapp_msrc", 0, 1, "mfpv", "", "source directory"},
    {"", 0, 0, "", "", ""}}; // End marker

/** @brief compare option names
    @param  o1 - pointer to first option
    @param  o2 - pointer to second option
    @return negative if o1 < o2, zero if o1 == o2, positive if o1 > o2
 */
static int comp_opt_name(const void *o1, const void *o2) {
    const Opts *opt1 = o1;
    const Opts *opt2 = o2;
    return strcmp(opt1->name, opt2->name);
}
/** @brief compare option groups
    @param  o1 - pointer to first option
    @param  o2 - pointer to second option
    @return negative if o1 < o2, zero if o1 == o2, positive if o1 > o2
 */
static int comp_opt_group(const void *o1, const void *o2) {
    const Opts *opt1 = o1;
    const Opts *opt2 = o2;
    return (opt1->group - opt2->group);
}
/** @brief compare option descriptions
    @param  o1 - pointer to first option
    @param  o2 - pointer to second option
    @return negative if o1 < o2, zero if o1 == o2, positive if o1 > o2
 */
static int comp_opt_desc(const void *o1, const void *o2) {
    const Opts *opt1 = o1;
    const Opts *opt2 = o2;
    return strcmp(opt1->desc, opt2->desc);
}
/** @brief compare option short options
    @param  o1 - pointer to first option
    @param  o2 - pointer to second option
    @return negative if o1 < o2, zero if o1 == o2, positive if o1 > o2
 */
static int comp_opt_short_opt(const void *o1, const void *o2) {
    const Opts *opt1 = o1;
    const Opts *opt2 = o2;
    return strcmp(opt1->short_opt, opt2->short_opt);
}
/** @brief dump options sorted by description
 */
void dump_opts_by_desc() {
    sort_opts_by_desc();
    dump_opts();
}
/** @brief dump options sorted by name
 */
void dump_opts_by_name() {
    sort_opts_by_name();
    dump_opts();
}
/** @brief dump options sorted by group
 */
void dump_opts_by_group() {
    sort_opts_by_group();
    dump_opts();
}
/** @brief dump options sorted by short option
 */
void dump_opts_by_short_opt() {
    sort_opts_by_short_opt();
    dump_opts();
}
/** @brief dump options name
 */
void sort_opts_by_name() {
    qsort(opts, ARRAY_SIZE(opts), sizeof(opts[0]), comp_opt_name);
}
/**  @brief sort options by group
 */
void sort_opts_by_group() {
    qsort(opts, ARRAY_SIZE(opts), sizeof(opts[0]), comp_opt_group);
}
/** @brief sort options by description
 */
void sort_opts_by_desc() {
    qsort(opts, ARRAY_SIZE(opts), sizeof(opts[0]), comp_opt_desc);
}
/** @brief sort options by short option
 */
void sort_opts_by_short_opt() {
    qsort(opts, ARRAY_SIZE(opts), sizeof(opts[0]), comp_opt_short_opt);
}
/** @brief select option by name
    @param  name - option name
    @return pointer to option, or NULL if not found
 */
Opts *select_opt(char *name) {
    Opts *opt;
    Opts key;
    key.name = name;
    sort_opts_by_name();
    opt = bsearch(&key, opts, ARRAY_SIZE(opts), sizeof(opts[0]), comp_opt_name);
    return opt;
}
/** @brief dump all options
 */
void dump_opts() {
    print_opt_header();
    int i = 0;
    while (opts[i].name != NULL) {
        if (opts[i].name[0] != '\0')
            print_opt(i);
        i++;
    }
}
/** @brief dump options by use mask
    @param  usage - usage string
    @param  mask - use mask string
 */
void dump_opts_by_use(char *usage, char *mask) {
    int i = 0;
    int j = 0;
    printf("\nusage: %s\n\n", usage);
    print_opt_header();
    while (opts[i].name != NULL && opts[i].name[0] != '\0') {
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
        if (opts[i].name[0] != '\0')
            print_opt(i);
        i++;
    }
}
/** @brief print option header
 */
void print_opt_header() {
    printf("   field name         type grp  mask opt description\n");
    printf(
        "   ------------------ ---- ---- ---- --- -------------------------\n");
}

/** @brief print option
    @param  i - option index
 */
void print_opt(int i) {
    char *type;
    char *group;
    char c;
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
