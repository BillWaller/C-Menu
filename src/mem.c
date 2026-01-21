//  mem.c
//  Bill Waller Copyright (c) 2025
//  MIT License
//  Memory establishment and teardown
//  for C-Menu Menu, Pick, Form, View
//  billxwaller@gmail.com

#include "menu.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

Init *new_init(int, char **);
Menu *new_menu(Init *init, int, char **, int, int);
Pick *new_pick(Init *init, int, char **, int, int);
Form *new_form(Init *init, int, char **, int, int);
View *new_view(Init *init, int, char **, int, int);
View *close_view(Init *init);
Form *close_form(Init *init);
Pick *close_pick(Init *init);
Menu *close_menu(Init *init);
Init *close_init(Init *init);
bool mapp_spec(Init *, int, char **);
bool init_menu_files(Init *, int, char **);
bool init_pick_files(Init *, int, char **);
bool init_form_files(Init *, int, char **);
bool init_view_files(Init *, int, char **);
bool verify_spec_arg(char *, char *, char *, char *, int);
int init_cnt = 0;

Menu *menu;
Pick *pick;
Form *form;
View *view;
/// ╭────────────────────────────────────────────────────────────────╮
/// │ NEW_INIT                                                       │
/// ╰────────────────────────────────────────────────────────────────╯
/// Create and initialize an Init structure
Init *new_init(int argc, char **argv) {
    int i = 0;
    // Initialize Init Structure
    Init *init = calloc(1, sizeof(Init));
    if (init == NULL) {
        abend(-1, "calloc init failed");
        return NULL;
    }
    init->argv = calloc(MAXARGS + 1, sizeof(char *));
    if (init->argv == NULL) {
        free(init);
        ssnprintf(em0, MAXLEN - 65, "%s, line: %d", __FILE__, __LINE__ - 1);
        ssnprintf(em1, MAXLEN - 1, "init->argv = calloc(%d, %d) failed\n",
                  MAXARGS + 1, sizeof(char *));
        display_error(em0, em1, NULL, NULL);
        abend(-1, "User terminated program");
        return NULL;
    }
    init->argc = argc;
    for (i = 0; i < init->argc; i++) {
        // Allocate memory for each argument strings
        init->argv[i] = strdup(argv[i]);
    }
    init->argv[i] = NULL;
    init_cnt++;
    return init;
}
/// ╭────────────────────────────────────────────────────────────────╮
/// │ CLOSE_INIT                                                     │
/// ╰────────────────────────────────────────────────────────────────╯
/// Teardown and free an Init structure
Init *close_init(Init *init) {
    int i;
    if (!init)
        return NULL;
    if (init->menu) {
        init->menu = close_menu(init);
        init->menu = NULL;
    }
    if (init->view) {
        init->view = close_view(init);
        init->view = NULL;
    }
    if (init->form) {
        init->form = close_form(init);
        init->form = NULL;
    }
    if (init->pick) {
        init->pick = close_pick(init);
        init->pick = NULL;
    }
    for (i = 0; i <= init->argc; i++) {
        if (init->argv[i]) {
            free(init->argv[i]);
            init->argv[i] = NULL;
        }
    }
    if (init->argv) {
        free(init->argv);
        init->argv = NULL;
    }
    free(init);
    init = NULL;
    init_cnt--;
    return init;
}
/// ╭────────────────────────────────────────────────────────────────╮
/// │ NEW_MENU                                                       │
/// ╰────────────────────────────────────────────────────────────────╯
/// Create and initialize a Menu structure
Menu *new_menu(Init *init, int argc, char **argv, int begy, int begx) {
    init->menu = (Menu *)calloc(1, sizeof(Menu));
    if (!init->menu) {
        abend(-1, "calloc menu failed");
        return NULL;
    }
    init->menu_cnt++;
    menu = init->menu;
    if (!init_menu_files(init, argc, argv)) {
        abend(-1, "init_menu_files failed");
        return NULL;
    }
    menu->begy = begy;
    menu->begx = begx;
    return init->menu;
}
/// ╭────────────────────────────────────────────────────────────────╮
/// │ CLOSE_MENU                                                     │
/// ╰────────────────────────────────────────────────────────────────╯
/// Destroy Menu structure
Menu *close_menu(Init *init) {
    if (!init->menu)
        return (NULL);
    free(init->menu);
    init->menu = NULL;
    init->menu_cnt--;
    return init->menu;
}
/// ╭────────────────────────────────────────────────────────────────╮
/// │ NEW_PICK                                                       │
/// ╰────────────────────────────────────────────────────────────────╯
/// Create and initialize a Pick structure
Pick *new_pick(Init *init, int argc, char **argv, int begy, int begx) {
    init->pick = (Pick *)calloc(1, sizeof(Pick));
    if (!init->pick) {
        Perror("calloc pick failed");
        return NULL;
    }
    init->pick_cnt++;
    pick = init->pick;
    if (!init_pick_files(init, argc, argv)) {
        abend(-1, "init_pick_files failed");
        return NULL;
    }
    pick->object = calloc(OBJ_MAXCNT + 1, sizeof(char *));
    if (pick->object == NULL) {
        ssnprintf(em0, MAXLEN - 65, "%s, line: %d", __FILE__, __LINE__ - 1);
        ssnprintf(em1, MAXLEN - 1,
                  "calloc pick->object = calloc(%d, %d) failed\n",
                  OBJ_MAXCNT + 1, sizeof(char *));
        display_error(em0, em1, NULL, NULL);
        abend(-1, "User terminated program");
    }
    pick->begy = begy;
    pick->begx = begx;
    return init->pick;
}
/// ╭────────────────────────────────────────────────────────────────╮
/// │ CLOSE_PICK                                                     │
/// ╰────────────────────────────────────────────────────────────────╯
/// Destroy Pick structure
Pick *close_pick(Init *init) {
    if (!init->pick)
        return NULL;

    for (pick->obj_idx = 0; pick->obj_idx < pick->obj_cnt; pick->obj_idx++)
        if (pick->object[pick->obj_idx] != NULL)
            free(pick->object[pick->obj_idx]);
    free(pick->object);
    free(pick);
    init->pick = NULL;
    init->pick_cnt--;
    return init->pick;
}
/// ╭────────────────────────────────────────────────────────────────╮
/// │ NEW_FORM                                                       │
/// ╰────────────────────────────────────────────────────────────────╯
/// Create and initialize a Form structure
Form *new_form(Init *init, int argc, char **argv, int begy, int begx) {
    init->form = (Form *)calloc(1, sizeof(Form));
    if (!init->form) {
        abend(-1, "calloc form failed");
        return NULL;
    }
    init->form_cnt++;
    form = init->form;
    if (!init_form_files(init, argc, argv)) {
        abend(-1, "init_form_files failed");
        return NULL;
    }
    strnz__cpy(form->brackets, init->brackets, 3);
    strnz__cpy(form->fill_char, init->fill_char, MAXLEN - 1);
    form->begy = begy;
    form->begx = begx;
    return init->form;
}
/// ╭────────────────────────────────────────────────────────────────╮
/// │ CLOSE_FORM                                                     │
/// ╰────────────────────────────────────────────────────────────────╯
/// Destroy Form structure
Form *close_form(Init *init) {
    int i;

    if (!init->form)
        return NULL;
    for (i = 0; i < init->form->fidx; i++) {
        if (init->form->field[i])
            free(init->form->field[i]);
        init->form->field[i] = NULL;
    }
    for (i = 0; i < init->form->didx; i++) {
        if (init->form->text[i])
            free(init->form->text[i]);
        init->form->text[i] = NULL;
    }
    free(init->form);
    init->form = NULL;
    init->form_cnt--;
    return init->form;
}
/// ╭────────────────────────────────────────────────────────────────╮
/// │ NEW_VIEW                                                       │
/// ╰────────────────────────────────────────────────────────────────╯
/// Create and initialize a View structure
View *new_view(Init *init, int argc, char **argv, int begy, int begx) {

    init->view_cnt++;
    init->view = (View *)calloc(1, sizeof(View));
    if (!init->view) {
        Perror("calloc init->view failed");
        return NULL;
    }
    view = init->view;
    view->argc = argc;
    view->argv = calloc((view->argc + 1), sizeof(char *));
    if (view->argv == NULL) {
        ssnprintf(em0, MAXLEN - 65, "%s, line: %d", __FILE__, __LINE__ - 1);
        ssnprintf(em1, MAXLEN - 1, "view->argv = calloc(%d, %d) failed\n",
                  view->argc + 1, sizeof(char *));
        display_error(em0, em1, NULL, NULL);
        abend(-1, "User terminated program");
    }
    if (!init_view_files(init, view->argc, argv)) {
        abend(-1, "init_view_files failed");
        return NULL;
    }
    return view;
}
/// ╭────────────────────────────────────────────────────────────────╮
/// │ CLOSE_VIEW                                                     │
/// ╰────────────────────────────────────────────────────────────────╯
/// Destroy View structure
View *close_view(Init *init) {
    int i;
    view = init->view;
    for (i = 0; i <= view->argc; i++)
        free(view->argv[i]);
    free(view->argv);
    free(view);
    init->view = NULL;
    view = NULL;
    init->view_cnt--;
    return init->view;
}
/// ╭────────────────────────────────────────────────────────────────╮
/// │ VERIFY_SPEC_ARG                                                │
/// ╰────────────────────────────────────────────────────────────────╯
///  Verify file specification argument
///  @param  char *spec,      -> menu->spec, form->spec, etc.
///  @param  char *src_spec,  -> init->._spec | argv[optind]
///  @param  char *dir,       -> init->._. directory
///  @param  char *alt_dir,   -> literal, "~/menuapp/data", etc.
///  @param  int mode)        -> R_OK, W_OK, X_OK, WC_OK, S_QUIET
///  @flags S_QUIET - suppress error messages
///  @flags WC_OK - write create ok
///  @return bool - true if file verified
bool verify_spec_arg(char *spec, char *org_spec, char *dir, char *alt_dir,
                     int mode) {
    bool f_dir = false;
    bool f_spec = false;
    bool f_quote = true;
    char *s1;
    char *s2;
    char s1_s[MAXLEN];
    char s2_s[MAXLEN];
    char file_name[MAXLEN];
    char try_spec[MAXLEN];
    char idio_spec[MAXLEN];

    if (!org_spec[0])
        return false;
    /// ╭────────────────────────────────────────────────────────────╮
    /// │ USE FILE_NAME FROM SRC_SPEC                                │
    /// ╰────────────────────────────────────────────────────────────╯
    idio_spec[0] = '\0';
    strnz__cpy(try_spec, org_spec, MAXLEN - 1);
    f_quote = stripz_quotes(try_spec);
    s1 = strtok(try_spec, " \t\n");
    strnz__cpy(s1_s, s1, MAXLEN - 1);
    s2 = strtok(NULL, "\n");
    strnz__cpy(s2_s, s2, MAXLEN - 1);
    strnz__cpy(file_name, s1, MAXLEN - 1);
    strnz__cpy(try_spec, file_name, MAXLEN - 1);
    canonicalize_file_spec(try_spec);
    if (try_spec[0]) {
        expand_tilde(try_spec, MAXLEN - 1);
        if (try_spec[0] == '/') {
            f_spec = verify_file(try_spec, mode);
            if (f_quote)
                /// preserve quotes
                strnz__cpy(spec, org_spec, MAXLEN - 1);
            else
                strnz__cpy(spec, try_spec, MAXLEN - 1);
            return f_spec;
        } else {
            if (!f_dir && dir[0]) {
                ///  ╭───────────────────────────────────────────────────╮
                ///  │ IDIOMATIC (PREFERRED) SPEC                        │
                ///  ╰───────────────────────────────────────────────────╯
                if (strcmp(dir, "$PATH") == 0) {
                    strnz__cpy(try_spec, file_name, MAXLEN - 1);
                    f_spec = locate_file_in_path(try_spec, file_name);
                } else {
                    strnz__cpy(try_spec, dir, MAXLEN - 1);
                    expand_tilde(try_spec, MAXLEN - 1);
                    f_dir = verify_dir(try_spec, mode);
                    if (f_dir) {
                        strnz__cat(try_spec, "/", MAXLEN - 1);
                        strnz__cat(try_spec, file_name, MAXLEN - 1);
                        strnz__cpy(idio_spec, try_spec, MAXLEN - 1);
                        f_spec = verify_file(idio_spec, mode | S_QUIET);
                    }
                }
            }
            if (!f_spec && alt_dir && alt_dir[0] != '\0') {
                if (strcmp(alt_dir, "$PATH") == 0) {
                    strnz__cpy(try_spec, file_name, MAXLEN - 1);
                    f_spec = locate_file_in_path(try_spec, file_name);
                } else {
                    strnz__cpy(try_spec, alt_dir, MAXLEN - 1);
                    expand_tilde(try_spec, MAXLEN - 1);
                    f_dir = verify_dir(try_spec, mode);
                    if (f_dir) {
                        strnz__cat(try_spec, "/", MAXLEN - 1);
                        strnz__cat(try_spec, file_name, MAXLEN - 1);
                        f_spec = verify_file(try_spec, mode | S_QUIET);
                    }
                }
            }
            if (!f_spec) {
                strnz__cpy(try_spec, ".", MAXLEN - 1);
                strnz__cat(try_spec, "/", MAXLEN - 1);
                strnz__cat(try_spec, file_name, MAXLEN - 1);
                f_spec = verify_file(try_spec, mode | S_QUIET);
            }
            if (!f_spec && mode == W_OK) {
                strnz__cpy(try_spec, idio_spec, MAXLEN - 1);
                FILE *fp = fopen(try_spec, "a");
                if (fp) {
                    fclose(fp);
                    f_spec = true;
                }
            }
            ///  ╭──────────────────────────────────────────────────╮
            ///  │ PRESERVE QUOTES                                  │
            ///  ╰──────────────────────────────────────────────────╯
            if (f_quote)
                /// preserve quotes
                strnz__cpy(spec, org_spec, MAXLEN - 1);
            else if (f_spec)
                strnz__cpy(spec, try_spec, MAXLEN - 1);
            if (try_spec[0] == '\0' && idio_spec[0] != '\0')
                strnz__cpy(spec, idio_spec, MAXLEN - 1);
            else
                strnz__cpy(spec, try_spec, MAXLEN - 1);
            if (s2_s[0] != '\0') {
                strnz__cat(spec, " ", MAXLEN - 1);
                strnz__cat(spec, s2_s, MAXLEN - 1);
            }
            return f_spec;
        }
    }
    return false;
}

/// init_files
/// @param
///  idiomatic directory usage:
///     init->mapp_msrc  description files
///     init->mapp_help  help files
///     init->mapp_data  in, out, data files
///     init->mapp_user  executable scripts
///     init->mapp_bin   binary executables
///
/// Initialize file specifications
/// Priority order:
/// 1 - Default values
/// 2 - Configuration file
/// 3 - Environment variables
/// 4 - Command line positional arguments
/// 5 - Command line option arguments
///
/// ╭────────────────────────────────────────────────────────────────╮
/// │ INIT_MENU_FILES                                                │
/// ╰────────────────────────────────────────────────────────────────╯
/// Initialize Menu file specifications
bool init_menu_files(Init *init, int argc, char **argv) {
    char tmp_str[MAXLEN];
    ///  @param init_menu_files(Init *init, int argc, char **argv)
    ///  @brief Initialize file specifications
    ///  @param init - Init structure
    ///  @param argc - May have been provided by command line
    ///                ~/.minitrc
    ///                environment variables
    ///                or calling program interal to C-Menu
    ///
    ///
    /// ╭────────────────────────────────────────────────────────────╮
    /// │ MENU MAPP_SPEC - OPT ARG -d: - Priority 5                  │
    /// ╰────────────────────────────────────────────────────────────╯
    menu->f_mapp_spec =
        verify_spec_arg(menu->mapp_spec, init->mapp_spec, init->mapp_msrc,
                        "~/menuapp/msrc", R_OK);
    /// ╭────────────────────────────────────────────────────────────╮
    /// │ MENU HELP_SPEC - OPT ARG -H: - Priority 5                  │
    /// ╰────────────────────────────────────────────────────────────╯
    menu->f_help_spec =
        verify_spec_arg(menu->help_spec, init->help_spec, init->mapp_help,
                        "~/menuapp/help", R_OK);
    ///  ╭───────────────────────────────────────────────────────────╮
    ///  │ MENU MAPP_SPEC - POSITIONAL ARG 1 - Priority 4            │
    ///  ╰───────────────────────────────────────────────────────────╯
    if (optind < argc && !menu->f_mapp_spec) {
        menu->f_mapp_spec =
            verify_spec_arg(menu->mapp_spec, argv[optind], init->mapp_msrc,
                            "~/menuapp/msrc", R_OK);
        if (menu->f_mapp_spec)
            optind++;
    }
    ///  ╭───────────────────────────────────────────────────────────╮
    ///  │ MENU HELP_SPEC - POSITIONAL ARG 2 - Priority 4            │
    ///  ╰───────────────────────────────────────────────────────────╯
    if (optind < argc && !menu->f_help_spec) {
        menu->f_help_spec =
            verify_spec_arg(menu->help_spec, argv[optind], init->mapp_help,
                            "~/menuapp/help", R_OK);
        if (menu->f_help_spec)
            optind++;
    }
    /// ╭───────────────────────────────────────────────────────────╮
    /// │ MENU MAPP_SPEC - FALLBACK DEFAULTS - Priority 1           │
    /// ╰───────────────────────────────────────────────────────────╯
    /// should check menu->mapp_spec[0]
    if (!menu->f_mapp_spec) {
        menu->f_mapp_spec = verify_spec_arg(
            menu->mapp_spec, "~/menuapp/msrc/main.m", NULL, NULL, R_OK);
        if (!menu->f_mapp_spec) {
            strnz__cpy(tmp_str, "menu cannot read description file ",
                       MAXLEN - 1);
            strnz__cat(tmp_str, menu->mapp_spec, MAXLEN - 1);
            abend(-1, tmp_str);
        }
    }
    /// ╭───────────────────────────────────────────────────────────╮
    /// │ MENU HELP_SPEC - FALLBACK DEFAULTS - Priority 1           │
    /// ╰───────────────────────────────────────────────────────────╯
    if (!menu->f_help_spec) {
        menu->f_help_spec = verify_spec_arg(
            menu->help_spec, "~/menuapp/help/main.help", NULL, NULL, R_OK);
        if (!menu->f_help_spec) {
            strnz__cpy(tmp_str, "menu cannot read help file ", MAXLEN - 1);
            strnz__cat(tmp_str, menu->help_spec, MAXLEN - 1);
            abend(-1, tmp_str);
        }
    }
    menu->fg_color = init->fg_color;
    menu->bg_color = init->bg_color;
    menu->bo_color = init->bo_color;
    menu->f_stop_on_error = init->f_stop_on_error;
    return true;
}
/// ╭────────────────────────────────────────────────────────────────╮
/// │ PICK INIT_PICK_FILES                                           │
/// ╰────────────────────────────────────────────────────────────────╯
/// Initialize Pick file specifications
bool init_pick_files(Init *init, int argc, char **argv) {
    /// @param init_pick_files(Init *init, int argc, char **argv)
    /// pick desc, in_file, out_file, help_file
    /// ╭───────────────────────────────────────────────────────────╮
    /// │ PICK IN_SPEC - OPT ARG -i: - Priority 5                   │
    /// ╰───────────────────────────────────────────────────────────╯
    pick->f_in_spec = verify_spec_arg(pick->in_spec, init->in_spec,
                                      init->mapp_data, "~/menuapp/data", R_OK);
    /// ╭───────────────────────────────────────────────────────────╮
    /// │ PICK OUT_SPEC - OPT ARG -o: - Priority 5                  │
    /// ╰───────────────────────────────────────────────────────────╯
    pick->f_out_spec =
        verify_spec_arg(pick->out_spec, init->out_spec, init->mapp_data,
                        "~/menuapp/data", W_OK | S_QUIET);
    /// ╭───────────────────────────────────────────────────────────╮
    /// │ PICK PROVIDER_CMD - OPT ARG -S: - Priority 5              │
    /// ╰───────────────────────────────────────────────────────────╯
    if (init->provider_cmd[0] != '\0') {
        pick->f_provider_cmd =
            verify_spec_arg(pick->provider_cmd, init->provider_cmd,
                            "~/menuapp/bin", "$PATH", X_OK | S_QUIET);
    }
    /// ╭───────────────────────────────────────────────────────────╮
    /// │ PICK CMD - OPT ARG -c: - Priority 5                       │
    /// ╰───────────────────────────────────────────────────────────╯
    if (init->cmd[0] != '\0') {
        pick->f_cmd = verify_spec_arg(pick->cmd, init->cmd, "~/menuapp/bin",
                                      "$PATH", X_OK | S_QUIET);
    }
    /// ╭───────────────────────────────────────────────────────────╮
    /// │ PICK RECEIVER_CMD - OPT ARG -R: - Priority 5              │
    /// ╰───────────────────────────────────────────────────────────╯
    if (init->receiver_cmd[0] != '\0') {
        pick->f_receiver_cmd =
            verify_spec_arg(pick->receiver_cmd, init->receiver_cmd,
                            "~/menuapp/bin", "$PATH", X_OK | S_QUIET);
    }
    /// ╭───────────────────────────────────────────────────────────╮
    /// │ PICK TITLE    - OPT ARG -T: - Priority 5                  │
    /// ╰───────────────────────────────────────────────────────────╯
    if (init->title[0])
        strnz__cpy(pick->title, init->title, MAXLEN - 1);
    /// ╭───────────────────────────────────────────────────────────╮
    /// │ PICK HELP_SPEC - OPT ARG -H: - Priority 5                 │
    /// ╰───────────────────────────────────────────────────────────╯
    pick->f_help_spec =
        verify_spec_arg(pick->help_spec, init->help_spec, init->mapp_help,
                        "~/menuapp/help", R_OK);
    /// ╭───────────────────────────────────────────────────────────╮
    /// │ PICK IN_SPEC - POSITIONAL ARG 1 - Priority 4              │
    /// ╰───────────────────────────────────────────────────────────╯
    if (optind < argc && !pick->f_in_spec) {
        pick->f_in_spec =
            verify_spec_arg(pick->in_spec, argv[optind], init->mapp_data,
                            "~/menuapp/data", R_OK);
        if (pick->f_in_spec)
            optind++;
    }
    /// ╭───────────────────────────────────────────────────────────╮
    /// │ PICK OUT_SPEC - POSITIONAL ARG 2 - Priority 4             │
    /// ╰───────────────────────────────────────────────────────────╯
    if (optind < argc && !pick->f_out_spec) {
        pick->f_out_spec =
            verify_spec_arg(pick->out_spec, argv[optind], init->mapp_data,
                            "~/menuapp/data", W_OK | S_QUIET);
        if (pick->f_out_spec)
            optind++;
    }
    /// ╭───────────────────────────────────────────────────────────╮
    /// │ PICK PROVIDER_CMD - POSITIONAL ARG 3 - Priority 4         │
    /// ╰───────────────────────────────────────────────────────────╯
    if (optind < argc && !pick->f_provider_cmd) {
        if (argv[optind][0] != '\0') {
            pick->f_provider_cmd =
                verify_spec_arg(pick->provider_cmd, argv[optind],
                                "~/menuapp/bin", NULL, X_OK | S_QUIET);
            if (!pick->f_provider_cmd) {
                base_name(tmp_str, argv[optind]);
                pick->f_provider_cmd =
                    locate_file_in_path(pick->provider_cmd, tmp_str);
            }
        }
        optind++;
    }
    /// ╭───────────────────────────────────────────────────────────╮
    /// │ PICK CMD - POSITIONAL ARG 4 - Priority 4                  │
    /// ╰───────────────────────────────────────────────────────────╯
    if (optind < argc && !pick->f_cmd) {
        if (argv[optind][0] != '\0') {
            pick->f_cmd = verify_spec_arg(
                pick->cmd, argv[optind], "~/menuapp/bin", NULL, X_OK | S_QUIET);
            if (!pick->f_cmd) {
                base_name(tmp_str, argv[optind]);
                pick->f_cmd = locate_file_in_path(pick->cmd, tmp_str);
            }
        }
        optind++;
    }
    /// ╭───────────────────────────────────────────────────────────╮
    /// │ PICK RECEIVER_SPEC - POSITIONAL ARG 5 - Priority 4        │
    /// ╰───────────────────────────────────────────────────────────╯
    if (optind < argc && !pick->f_receiver_cmd) {
        if (argv[optind][0] != '\0') {
            pick->f_receiver_cmd =
                verify_spec_arg(pick->receiver_cmd, argv[optind],
                                "~/menuapp/bin", NULL, X_OK | S_QUIET);
            if (!pick->f_receiver_cmd) {
                base_name(tmp_str, argv[optind]);
                pick->f_receiver_cmd =
                    locate_file_in_path(pick->receiver_cmd, tmp_str);
            }
        }
        optind++;
    }
    /// ╭───────────────────────────────────────────────────────────╮
    /// │ PICK HELP_SPEC - POSITIONAL ARG 6 - Priority 4            │
    /// ╰───────────────────────────────────────────────────────────╯
    if (optind < argc && !pick->f_help_spec) {
        pick->f_help_spec =
            verify_spec_arg(pick->help_spec, argv[optind], init->mapp_help,
                            "~/menuapp/help", R_OK);
        if (pick->f_help_spec)
            optind++;
    }
    pick->fg_color = init->fg_color;
    pick->bg_color = init->bg_color;
    pick->bo_color = init->bo_color;
    pick->select_max = init->select_max;
    pick->f_stop_on_error = init->f_stop_on_error;
    pick->f_multiple_cmd_args = init->f_multiple_cmd_args;
    return true;
}
// ╭───────────────────────────────────────────────────────────────╮
// │ INIT_FORM_FILES                                               │
// ╰───────────────────────────────────────────────────────────────╯
/// Initialize Form file specifications
bool init_form_files(Init *init, int argc, char **argv) {

    /// ╭───────────────────────────────────────────────────────────╮
    /// │ FORM MAPP_SPEC - OPT ARG -d: - Priority 5                 │
    /// ╰───────────────────────────────────────────────────────────╯
    form->f_mapp_spec =
        verify_spec_arg(form->mapp_spec, init->mapp_spec, init->mapp_msrc,
                        "~/menuapp/msrc", R_OK);
    /// ╭───────────────────────────────────────────────────────────╮
    /// │ FORM IN_SPEC - OPT ARG -i: - Priority 5                   │
    /// ╰───────────────────────────────────────────────────────────╯
    form->f_in_spec = verify_spec_arg(form->in_spec, init->in_spec,
                                      init->mapp_data, "~/menuapp/data", R_OK);
    /// ╭───────────────────────────────────────────────────────────╮
    /// │ FORM OUT_SPEC - OPT ARG -o: - Priority 5                  │
    /// ╰───────────────────────────────────────────────────────────╯
    form->f_out_spec =
        verify_spec_arg(form->out_spec, init->out_spec, init->mapp_data,
                        "~/menuapp/data", W_OK | S_QUIET);
    /// ╭───────────────────────────────────────────────────────────╮
    /// │ FORM PROVIDER_CMD - OPT ARG -S: - Priority 5              │
    /// ╰───────────────────────────────────────────────────────────╯
    if (init->provider_cmd[0] != '\0') {
        form->f_provider_cmd =
            verify_spec_arg(form->provider_cmd, init->provider_cmd,
                            "~/menuapp/bin", "$PATH", X_OK | S_QUIET);
    }
    /// ╭───────────────────────────────────────────────────────────╮
    /// │ FORM CMD - OPT ARG -c: - Priority 5                       │
    /// ╰───────────────────────────────────────────────────────────╯
    if (init->cmd[0] != '\0') {
        form->f_cmd = verify_spec_arg(form->cmd, init->cmd, "~/menuapp/bin",
                                      "$PATH", X_OK | S_QUIET);
    }
    /// ╭───────────────────────────────────────────────────────────╮
    /// │ FORM RECEIVER_CMD - OPT ARG -R: - Priority 5              │
    /// ╰───────────────────────────────────────────────────────────╯
    if (init->receiver_cmd[0] != '\0') {
        form->f_receiver_cmd =
            verify_spec_arg(form->receiver_cmd, init->receiver_cmd,
                            "~/menuapp/bin", "$PATH", X_OK | S_QUIET);
    }
    /// ╭───────────────────────────────────────────────────────────╮
    /// │ FORM HELP_SPEC - OPT ARG -H: - Priority 5                 │
    /// ╰───────────────────────────────────────────────────────────╯
    form->f_help_spec =
        verify_spec_arg(form->help_spec, init->help_spec, init->mapp_help,
                        "~/menuapp/help", R_OK);
    /// ╭───────────────────────────────────────────────────────────╮
    /// │ FORM MAPP_SPEC - POSITIONAL ARG 1 - Priority 4            │
    /// ╰───────────────────────────────────────────────────────────╯
    if (optind < argc && !form->f_mapp_spec) {
        form->f_mapp_spec =
            verify_spec_arg(form->mapp_spec, argv[optind], init->mapp_msrc,
                            "~/menuapp/msrc", R_OK);
        if (form->f_mapp_spec)
            optind++;
    }
    /// ╭───────────────────────────────────────────────────────────╮
    /// │ FORM IN_SPEC - POSITIONAL ARG 2 - Priority 4              │
    /// ╰───────────────────────────────────────────────────────────╯
    if (optind < argc && !form->f_in_spec) {
        form->f_in_spec =
            verify_spec_arg(form->in_spec, argv[optind], init->mapp_data,
                            "~/menuapp/data", R_OK);
        if (form->f_in_spec)
            optind++;
    }
    /// ╭───────────────────────────────────────────────────────────╮
    /// │ FORM OUT_SPEC - POSITIONAL ARG 3 - Priority 4             │
    /// ╰───────────────────────────────────────────────────────────╯
    if (optind < argc && !form->f_out_spec) {
        form->f_out_spec =
            verify_spec_arg(form->out_spec, argv[optind], init->mapp_data,
                            "~/menuapp/data", W_OK | S_QUIET);
        if (form->f_out_spec)
            optind++;
    }
    /// ╭───────────────────────────────────────────────────────────╮
    /// │ FORM PROVIDER_CMD - POSITIONAL ARG 4 - Priority 4         │
    /// ╰───────────────────────────────────────────────────────────╯
    if (optind < argc && !form->f_provider_cmd) {
        if (argv[optind][0] != '\0') {
            form->f_provider_cmd =
                verify_spec_arg(form->provider_cmd, argv[optind],
                                "~/menuapp/bin", NULL, X_OK | S_QUIET);
            if (!form->f_provider_cmd) {
                base_name(tmp_str, argv[optind]);
                form->f_provider_cmd =
                    locate_file_in_path(form->provider_cmd, tmp_str);
            }
        }
        optind++;
    }
    /// ╭───────────────────────────────────────────────────────────╮
    /// │ FORM CMD - POSITIONAL ARG 5 - Priority 4                  │
    /// ╰───────────────────────────────────────────────────────────╯
    if (optind < argc && !form->f_cmd) {
        if (argv[optind][0] != '\0') {
            form->f_cmd = verify_spec_arg(
                form->cmd, argv[optind], "~/menuapp/bin", NULL, X_OK | S_QUIET);
            if (!form->f_cmd) {
                base_name(tmp_str, argv[optind]);
                form->f_cmd = locate_file_in_path(form->cmd, tmp_str);
            }
        }
        optind++;
    }
    /// ╭───────────────────────────────────────────────────────────╮
    /// │ FORM RECEIVER_SPEC - POSITIONAL ARG 5 - Priority 4        │
    /// ╰───────────────────────────────────────────────────────────╯
    if (optind < argc && !form->f_receiver_cmd) {
        if (argv[optind][0] != '\0') {
            form->f_receiver_cmd =
                verify_spec_arg(form->receiver_cmd, argv[optind],
                                "~/menuapp/bin", NULL, X_OK | S_QUIET);
            if (!form->f_receiver_cmd) {
                base_name(tmp_str, argv[optind]);
                form->f_receiver_cmd =
                    locate_file_in_path(form->receiver_cmd, tmp_str);
            }
        }
        optind++;
    }
    /// ╭───────────────────────────────────────────────────────────╮
    /// │ FORM HELP_SPEC - POSITIONAL ARG 6 - Priority 4            │
    /// ╰───────────────────────────────────────────────────────────╯
    if (optind < argc && !form->f_help_spec) {
        form->f_help_spec =
            verify_spec_arg(form->help_spec, init->help_spec, init->mapp_help,
                            "~/menuapp/help", R_OK);
        if (form->f_help_spec)
            optind++;
    }
    form->fg_color = init->fg_color;
    form->bg_color = init->bg_color;
    form->bo_color = init->bo_color;
    form->f_stop_on_error = init->f_stop_on_error;
    form->f_erase_remainder = init->f_erase_remainder;
    if (form->title[0] == '\0' && init->title[0] != '\0') {
        strip_quotes(init->title);
        strnz__cpy(form->title, init->title, MAXLEN - 1);
    }
    return true;
}
/// ╭───────────────────────────────────────────────────────────────╮
/// │ INIT_VIEW_FILES                                               │
/// ╰───────────────────────────────────────────────────────────────╯
/// Initialize View file specifications
bool init_view_files(Init *init, int argc, char **argv) {
    view = init->view;

    view->argv = calloc((argc - optind + 1), sizeof(char *));
    if (view->argv == NULL) {
        free(view);
        ssnprintf(em0, MAXLEN - 65, "%s, line: %d", __FILE__, __LINE__ - 1);
        ssnprintf(em1, MAXLEN - 1, "view->argv = calloc(%d, %d) failed\n",
                  (argc - optind + 1), sizeof(char *));
        display_error(em0, em1, NULL, NULL);
        abend(-1, "User terminated program");
        return false;
    }
    /// we presume that no unprocessed options remain in argv
    /// so we just copy all args from argv[1..argc-1] to view
    // s = optind;
    int s = optind;
    int d = 0;
    while (s < argc)
        view->argv[d++] = strdup(argv[s++]);
    view->argv[d] = NULL;
    view->argc = d;

    view->lines = init->lines;
    view->cols = init->cols;
    view->fg_color = init->fg_color;
    view->bg_color = init->bg_color;
    view->bo_color = init->bo_color;
    view->f_stop_on_error = init->f_stop_on_error;
    view->f_ignore_case = init->f_ignore_case;
    view->f_at_end_clear = init->f_at_end_clear;
    view->f_at_end_remove = init->f_at_end_remove;
    view->f_squeeze = init->f_squeeze;
    strnz__cpy(view->provider_cmd, init->provider_cmd, MAXLEN - 1);
    strnz__cpy(view->receiver_cmd, init->receiver_cmd, MAXLEN - 1);
    strnz__cpy(view->cmd_all, init->cmd_all, MAXLEN - 1);
    if (view->title[0] == '\0') {
        if (init->title[0] != '\0') {
            strnz__cpy(view->title, init->title, MAXLEN - 1);
        } else {
            if (view->provider_cmd[0] != '\0')
                strnz__cpy(view->title, view->provider_cmd, MAXLEN - 1);
            else {
                if (view->argv[0] != NULL && view->argv[0][0] != '\0')
                    strnz__cpy(view->title, view->argv[0], MAXLEN - 1);
                else
                    strnz__cpy(view->title, "C-Menu View", MAXLEN - 1);
            }
        }
    }
    strip_quotes(view->title);
    if (view->tab_stop == 0)
        view->tab_stop = 4;
    return true;
}
