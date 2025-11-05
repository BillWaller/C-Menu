// opts.h

#ifndef _OPTS_H
#define _OPTS_H 1

enum OptType {
    OT_STRING,
    OT_INT,
    OT_BOOL,
};

enum OptGroup { OG_FILES, OG_DIRS, OG_SPECS, OG_MISC, OG_PARMS, OG_FLAGS };

typedef struct {
    const char *name;
    int type;        // 0=string, 1=int, 2=bool
    int group;       // 0=FILES, 1=SPECS, 2=MISC, 3=PARMS, 4=FLAGS
    const char *use; // which programs use this option
                     // m=menu, p=pick, x=form_exec, w=form_write, v=view
    const char *desc;
} Opts;

extern Opts opts[];

extern void dump_opts_by_name();
extern void dump_opts_by_group();
extern void dump_opts_by_desc();
extern void sort_opts_by_name();
extern void sort_opts_by_group();
extern void sort_opts_by_desc();

#endif
