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
static int comp_opt_desc(const void *, const void *);
static int comp_opt_group(const void *, const void *);
static int comp_opt_desc(const void *, const void *);
void dump_opts();

Opts *select_opt(char *);

Opts opts[] = {
    {"minitrc", 0, 2, "mpfv", "a: configuration file spec"},
    {"lines", 1, 4, "mpfv", "L: width in lines"},
    {"cols", 1, 4, "mpfv", "C: height in columns"},
    {"begx", 1, 4, "mpfv", "X: begin on column"},
    {"begy", 1, 4, "mpfv", "Y: begin on line"},
    {"fg_color", 1, 4, "mpfv", "F: foreground_color"},
    {"bg_color", 1, 4, "mpfv", "B: background_color"},
    {"bo_color", 1, 4, "mpfv", "O: border_color"},
    {"f_at_end_clear", 2, 5, "mpfv", "z  clear screen at end of program"},
    {"f_at_end_remove", 2, 5, "...v", "r: remove file at end of program"},
    {"f_erase_remainder", 2, 5, "..f.", "e: erase remainder of line on enter"},
    {"f_ignore_case", 2, 5, "...v", "x: ignore case in search"},
    {"f_mutiple_cmd_args", 1, 4, "mpfv", "M  multiple command arguments"},
    {"f_squeeze", 2, 5, "...v", "s  squeeze multiple blank lines"},
    {"f_stop_on_error", 2, 5, "mpfv", "Z  stop on error"},
    {"prompt-type", 0, 3, "...v",
     "P: prompt (S-Short, L-Long, N-None)[string]"},
    {"prompt-str", 0, 3, "...v", "User supplied string"},
    {"selections", 1, 4, ".p..", "n: number of selections"},
    {"start_cmd", 0, 3, "...v", "S  command to execute at start of program"},
    {"tab_stop", 1, 4, "...v", "t: number of spaces per tab"},
    {"title", 0, 3, "mpfv", "T: title"},
    {"answer_spec", 0, 0, "..f.", "A: answer spec"},
    {"cmd_spec", 0, 3, ".pfv", "c: command executable"},
    {"help_spec", 0, 0, "mpfv", "H: help spec"},
    {"in_spec", 0, 0, ".p..", "i: input spec"},
    {"out_spec", 0, 0, ".p..", "o: output spec"},
    {"mapp_spec", 0, 0, "mpfv", "d: description spec"},
    {"mapp_data", 0, 1, "mpfv", "   data directory"},
    {"mapp_help", 0, 1, "mpfv", "   help directory"},
    {"mapp_home", 0, 1, "mpfv", "m: home directory"},
    {"mapp_msrc", 0, 1, "mpfv", "   source directory"},
    {"mapp_user", 0, 1, "mpfv", "u: user directory"},
    {"", 0, 0, "", ""}}; // End marker

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
            type = "string";
            break;
        case OT_INT:
            type = "integer";
            break;
        case OT_BOOL:
            type = "yes/no";
            break;
        default:
            type = "unknown";
        }
        switch (opts[i].group) {
        case OG_FILES:
            group = "file name";
            break;
        case OG_DIRS:
            group = "directory";
            break;
        case OG_SPECS:
            group = "file spec";
            break;
        case OG_MISC:
            group = "misc";
            break;
        case OG_PARMS:
            group = "parameters";
            break;
        case OG_FLAGS:
            group = "flag";
            break;
        default:
            group = "unknown";
        }
        printf("%02d %-18s  %-7s   %-10s  %-5s %s\n", i, opts[i].name, type,
               group, opts[i].use, opts[i].desc);
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

    printf(
        "long option          type      group       mask  flg description\n");
    printf("-------------------  -------   ----------  ----- --- "
           "--------------------------------\n");
    while (opts[i].name != NULL) {
        for (j = 0; j < 5; j++) {
            if (mask[j] != '.' && mask[j] == opts[i].use[j])
                break;
        }
        if (j == 5) {
            i++;
            continue;
        }
        switch (opts[i].type) {
        case OT_STRING:
            type = "string";
            break;
        case OT_INT:
            type = "integer";
            break;
        case OT_BOOL:
            type = "yes/no";
            break;
        default:
            type = "unknown";
        }
        switch (opts[i].group) {
        case OG_FILES:
            group = "file name";
            break;
        case OG_DIRS:
            group = "directory";
            break;
        case OG_SPECS:
            group = "file spec";
            break;
        case OG_MISC:
            group = "misc";
            break;
        case OG_PARMS:
            group = "parameters";
            break;
        case OG_FLAGS:
            group = "flag";
            break;
        default:
            group = "unknown";
        }
        if (opts[i].desc[0] == ' ')
            c = ' ';
        else
            c = '-';
        printf("--%-18s  %-7s   %-10s  %-5s %c%s\n", opts[i].name, type, group,
               opts[i].use, c, opts[i].desc);
        i++;
    }
}
