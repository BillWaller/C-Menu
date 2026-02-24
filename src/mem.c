/** @file mem.c
    @brief Create and destroy main data structures for C-Menu
    @author Bill Waller
    Copyright (c) 2025
    MIT License
    billxwaller@gmail.com
    @date 2026-02-09
 */

#include <common.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

Init *new_init(int, char **);
Menu *new_menu(Init *init, int, char **, int, int);
Pick *new_pick(Init *init, int, char **, int, int);
Form *new_form(Init *init, int, char **, int, int);
View *new_view(Init *init, int, char **);
View *destroy_view(Init *init);
Form *destroy_form(Init *init);
Pick *destroy_pick(Init *init);
Menu *destroy_menu(Init *init);
Init *destroy_init(Init *init);
bool init_menu_files(Init *, int, char **);
bool init_pick_files(Init *, int, char **);
bool init_form_files(Init *, int, char **);
bool init_view_files(Init *);
bool verify_spec_arg(char *, char *, char *, char *, int);
int init_cnt = 0;

Menu *menu;
Pick *pick;
Form *form;
View *view;
/** @brief Create and initialize Init structure
    @note calloc initializes all fields to zero/NULL
    @param argc, argv - arguments
    idiomatic directory usage:
    @code
        init->mapp_msrc  description files
        init->mapp_help  help files
        init->mapp_data  in, out, data files
        init->mapp_user  executable scripts
        init->mapp_bin   binary executables
    @endcode
    @note Initialize file specifications in priority order:
    1 - Default values
    2 - Configuration file
    3 - Environment variables
    4 - Command line positional arguments
    5 - Command line option arguments
 */
Init *new_init(int argc, char **argv) {
    int i = 0;
    Init *init = calloc(1, sizeof(Init));
    if (init == NULL) {
        abend(-1, "calloc init failed");
        return NULL;
    }
    init->argv = calloc(MAXARGS + 1, sizeof(char *));
    if (init->argv == NULL) {
        free(init);
        ssnprintf(em0, MAXLEN - 1, "%s, line: %d, errno: %d", __FILE__,
                  __LINE__ - 4, errno);
        ssnprintf(em1, MAXLEN - 1, "%s", strerror(errno));
        ssnprintf(em2, MAXLEN - 1, "view->argv = calloc(%d, %d) failed\n",
                  (MAXARGS + 1), sizeof(char *));
        display_error(em0, em1, em2, NULL);
        abend(-1, "calloc init->argv failed");
        return NULL;
    }
    init->argc = argc;
    for (i = 0; i < init->argc; i++) {
        init->argv[i] = strdup(argv[i]);
    }
    init->argv[i] = NULL;
    init->sio = (SIO *)calloc(1, sizeof(SIO));
    if (!init->sio) {
        abend(-1, "calloc sio failed");
        return NULL;
    }
    init_cnt++;
    return init;
}
/** @brief Destroy Init structure
    @param init structure
    @returns NULL
 */
Init *destroy_init(Init *init) {
    int i;
    if (!init)
        return NULL;
    if (init->sio) {
        free(init->sio);
        init->sio = NULL;
    }
    if (init->menu) {
        init->menu = destroy_menu(init);
        init->menu = NULL;
    }
    if (init->view) {
        init->view = destroy_view(init);
        init->view = NULL;
    }
    if (init->form) {
        init->form = destroy_form(init);
        init->form = NULL;
    }
    if (init->pick) {
        init->pick = destroy_pick(init);
        init->pick = NULL;
    }
    for (i = 0; i <= init->argc; i++) {
        if (init->argv[i]) {
            if (init->argv[i] != NULL)
                free(init->argv[i]);
            init->argv[i] = NULL;
        }
    }
    if (init->argv) {
        if (init->argv != NULL)
            free(init->argv);
        init->argv = NULL;
    }
    if (init != NULL) {
        free(init);
        init = NULL;
    }
    init_cnt--;
    return init;
}
/** @brief Create and initialize Menu structure
    @param init structure
    @param argc - number of arguments in argv
    @param argv - Arguments may have been provided by command line,
                    ~/.minitrc,
                    environment variables, or
                    calling program interal to C-Menu
    @param begy, begx - initial position of menu window
 */
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
/** @brief Destroy Menu structure
    @param init structure
    @return NULL
 */
Menu *destroy_menu(Init *init) {
    if (!init->menu)
        return (NULL);
    free(init->menu);
    init->menu = NULL;
    init->menu_cnt--;
    return init->menu;
}
/** @brief Create and initialize Pick structure
    @param init structure
    @param argc - number of arguments in argv
    @param argv - Arguments may have been provided by command line,
                    ~/.minitrc,
                    environment variables, or
                    calling program interal to C-Menu
    @param begy, begx - initial position of pick window
 */
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
        ssnprintf(em0, MAXLEN - 1, "%s, line: %d", __FILE__, __LINE__ - 1);
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
/** @brief Destroy Pick structure
    @param init structure
    @return NULL
 */
Pick *destroy_pick(Init *init) {
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
/** @brief Create and initialize Form structure
    @param init structure
    @param argc - number of arguments in argv
    @param argv - Arguments may have been provided by command line,
                    ~/.minitrc,
                    environment variables, or
                    calling program interal to C-Menu
    @param begy, begx - initial position of form window
 */
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
/** @brief Destroy Form structure
    @param init structure
    @return NULL
 */
Form *destroy_form(Init *init) {
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
/** @brief Create and initialize View structure
    @param init structure
    @param argc - number of arguments in argv
    @param argv - Arguments may have been provided by command line,
                    ~/.minitrc,
                    environment variables, or
                    calling program interal to C-Menu
 */
View *new_view(Init *init, int argc, char **argv) {
    init->view_cnt++;
    init->view = (View *)calloc(1, sizeof(View));
    if (!init->view) {
        free(init->view);
        ssnprintf(em0, MAXLEN - 1, "%s, line: %d, errno: %d", __FILE__,
                  __LINE__ - 1, errno);
        ssnprintf(em1, MAXLEN - 1, "%s", strerror(errno));
        ssnprintf(em2, MAXLEN - 1, "init->view = calloc(%d, %d) failed\n",
                  (argc - optind + 1), sizeof(char *));
        display_error(em0, em1, em2, NULL);
        abend(-1, "calloc init->view failed");
        return false;
    }
    view = init->view;
    view->argv = calloc((argc - optind + 1), sizeof(char *));
    if (view->argv == NULL) {
        free(view->argv);
        ssnprintf(em0, MAXLEN - 1, "%s, line: %d, errno: %d", __FILE__,
                  __LINE__ - 1, errno);
        ssnprintf(em1, MAXLEN - 1, "%s", strerror(errno));
        ssnprintf(em2, MAXLEN - 1, "view->argv = calloc(%d, %d) failed\n",
                  (argc - optind + 1), sizeof(char *));
        display_error(em0, em1, em2, NULL);
        abend(-1, "User terminated program");
        return false;
    }
    int s = optind;
    int d = 0;
    while (s < argc)
        view->argv[d++] = strdup(argv[s++]);
    view->argv[d] = NULL;
    view->argc = d;
    if (!init_view_files(init)) {
        abend(-1, "init_view_files failed");
        return NULL;
    }
    return view;
}
/** @brief Destroy View structure
    @param init structure
    @return NULL
 */
View *destroy_view(Init *init) {
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
/** @brief Verify file specification argument
    @param spec - menu->spec, form->spec, etc.
    @param org_spec - init->._spec | argv[optind]
    @param dir - init->._. directory
    @param alt_dir - literal, "~/menuapp/data", etc.
    @param mode - R_OK, W_OK, X_OK, WC_OK, S_QUIET
    @note mode is a bitwise OR of the following flags:
              S_QUIET - suppress error messages
              WC_OK - write create ok
    @return bool - true if file verified
 */
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
                /** preserve quotes */
                strnz__cpy(spec, org_spec, MAXLEN - 1);
            else
                strnz__cpy(spec, try_spec, MAXLEN - 1);
            return f_spec;
        } else {
            if (!f_dir && dir[0]) {
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
                        if (mode & S_WCOK)
                            f_spec = true;
                        else
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
                        if (mode & S_WCOK)
                            f_spec = true;
                        else
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
            if (f_quote)
                /** preserve quotes */
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
/** @brief Initialize Menu file specifications
    @param init structure
    @param argc - number of arguments in argv
    @param argv - Arguments may have been provided by command line,
                   ~/.minitrc, environment variables, or calling program
                   interal to C-Menu
    @note Positional args: [menu desc], [help file] */
bool init_menu_files(Init *init, int argc, char **argv) {
    char tmp_str[MAXLEN];
    menu->f_mapp_spec =
        verify_spec_arg(menu->mapp_spec, init->mapp_spec, init->mapp_msrc,
                        "~/menuapp/msrc", R_OK);
    menu->f_help_spec =
        verify_spec_arg(menu->help_spec, init->help_spec, init->mapp_help,
                        "~/menuapp/help", R_OK);
    if (optind < argc && !menu->f_mapp_spec) {
        menu->f_mapp_spec =
            verify_spec_arg(menu->mapp_spec, argv[optind], init->mapp_msrc,
                            "~/menuapp/msrc", R_OK);
        if (menu->f_mapp_spec)
            optind++;
    }
    if (optind < argc && !menu->f_help_spec) {
        menu->f_help_spec =
            verify_spec_arg(menu->help_spec, argv[optind], init->mapp_help,
                            "~/menuapp/help", R_OK);
        if (menu->f_help_spec)
            optind++;
    }
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
    if (!menu->f_help_spec) {
        menu->f_help_spec = verify_spec_arg(
            menu->help_spec, "~/menuapp/help/main.help", NULL, NULL, R_OK);
        if (!menu->f_help_spec) {
            strnz__cpy(tmp_str, "menu cannot read help file ", MAXLEN - 1);
            strnz__cat(tmp_str, menu->help_spec, MAXLEN - 1);
            abend(-1, tmp_str);
        }
    }
    menu->f_stop_on_error = init->f_stop_on_error;
    return true;
}
/** @brief Initialize Pick file specifications
    @brief Initialize file specifications
    @param init structure
    @param argc - number of arguments in argv
    @param argv - Arguments may have been provided by command line,
                   ~/.minitrc,
                   environment variables, or
                   calling program interal to C-Menu
    @note Positional args: [pick desc], [in_file], [out_file], [help_file] */
bool init_pick_files(Init *init, int argc, char **argv) {
    pick->f_in_spec = verify_spec_arg(pick->in_spec, init->in_spec,
                                      init->mapp_data, "~/menuapp/data", R_OK);
    pick->f_out_spec =
        verify_spec_arg(pick->out_spec, init->out_spec, init->mapp_data,
                        "~/menuapp/data", W_OK | S_QUIET);
    if (init->provider_cmd[0] != '\0') {
        pick->f_provider_cmd =
            verify_spec_arg(pick->provider_cmd, init->provider_cmd,
                            "~/menuapp/bin", "$PATH", X_OK | S_QUIET);
    }
    if (init->cmd[0] != '\0') {
        pick->f_cmd = verify_spec_arg(pick->cmd, init->cmd, "~/menuapp/bin",
                                      "$PATH", X_OK | S_QUIET);
    }
    if (init->receiver_cmd[0] != '\0') {
        pick->f_receiver_cmd =
            verify_spec_arg(pick->receiver_cmd, init->receiver_cmd,
                            "~/menuapp/bin", "$PATH", X_OK | S_QUIET);
    }
    if (init->title[0])
        strnz__cpy(pick->title, init->title, MAXLEN - 1);
    pick->f_help_spec =
        verify_spec_arg(pick->help_spec, init->help_spec, init->mapp_help,
                        "~/menuapp/help", R_OK);
    if (optind < argc && !pick->f_in_spec) {
        pick->f_in_spec =
            verify_spec_arg(pick->in_spec, argv[optind], init->mapp_data,
                            "~/menuapp/data", R_OK);
        if (pick->f_in_spec)
            optind++;
    }
    if (optind < argc && !pick->f_out_spec) {
        pick->f_out_spec =
            verify_spec_arg(pick->out_spec, argv[optind], init->mapp_data,
                            "~/menuapp/data", W_OK | S_QUIET);
        if (pick->f_out_spec)
            optind++;
    }
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
    if (optind < argc && !pick->f_help_spec) {
        pick->f_help_spec =
            verify_spec_arg(pick->help_spec, argv[optind], init->mapp_help,
                            "~/menuapp/help", R_OK);
        if (pick->f_help_spec)
            optind++;
    }
    pick->select_max = init->select_max;
    pick->f_stop_on_error = init->f_stop_on_error;
    pick->f_multiple_cmd_args = init->f_multiple_cmd_args;
    return true;
}
/** @brief Initialize Form file specifications
    @param init pointer to init structure
    @param argc - number of arguments in argv
    @param argv - Arguments may have been provided by command line ~/.minitrc,
   environment variables, or calling program interal to C-Menu
    @note Positional args: [pick desc], [in_file], [out_file], [help_file] */
bool init_form_files(Init *init, int argc, char **argv) {
    form->f_mapp_spec =
        verify_spec_arg(form->mapp_spec, init->mapp_spec, init->mapp_msrc,
                        "~/menuapp/msrc", R_OK);
    form->f_in_spec = verify_spec_arg(form->in_spec, init->in_spec,
                                      init->mapp_data, "~/menuapp/data", R_OK);
    form->f_out_spec =
        verify_spec_arg(form->out_spec, init->out_spec, init->mapp_data,
                        "~/menuapp/data", W_OK | S_QUIET);
    if (init->provider_cmd[0] != '\0') {
        form->f_provider_cmd =
            verify_spec_arg(form->provider_cmd, init->provider_cmd,
                            "~/menuapp/bin", "$PATH", X_OK | S_QUIET);
    }
    if (init->cmd[0] != '\0') {
        form->f_cmd = verify_spec_arg(form->cmd, init->cmd, "~/menuapp/bin",
                                      "$PATH", X_OK | S_QUIET);
    }
    if (init->receiver_cmd[0] != '\0') {
        form->f_receiver_cmd =
            verify_spec_arg(form->receiver_cmd, init->receiver_cmd,
                            "~/menuapp/bin", "$PATH", X_OK | S_QUIET);
    }
    form->f_help_spec =
        verify_spec_arg(form->help_spec, init->help_spec, init->mapp_help,
                        "~/menuapp/help", R_OK);
    if (optind < argc && !form->f_mapp_spec) {
        form->f_mapp_spec =
            verify_spec_arg(form->mapp_spec, argv[optind], init->mapp_msrc,
                            "~/menuapp/msrc", R_OK);
        if (form->f_mapp_spec)
            optind++;
    }
    if (optind < argc && !form->f_in_spec) {
        form->f_in_spec =
            verify_spec_arg(form->in_spec, argv[optind], init->mapp_data,
                            "~/menuapp/data", R_OK);
        if (form->f_in_spec)
            optind++;
    }
    if (optind < argc && !form->f_out_spec) {
        form->f_out_spec =
            verify_spec_arg(form->out_spec, argv[optind], init->mapp_data,
                            "~/menuapp/data", W_OK | S_QUIET);
        if (form->f_out_spec)
            optind++;
    }
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
    if (optind < argc && !form->f_help_spec) {
        form->f_help_spec =
            verify_spec_arg(form->help_spec, init->help_spec, init->mapp_help,
                            "~/menuapp/help", R_OK);
        if (form->f_help_spec)
            optind++;
    }
    form->f_stop_on_error = init->f_stop_on_error;
    form->f_erase_remainder = init->f_erase_remainder;
    if (form->title[0] == '\0' && init->title[0] != '\0') {
        strip_quotes(init->title);
        strnz__cpy(form->title, init->title, MAXLEN - 1);
    }
    return true;
}
/** @brief Initialize Pick file specifications
    @brief Initialize file specifications
    @param init structure
    @note Positional args: pick desc, in_file, out_file, help_file */
bool init_view_files(Init *init) {
    view = init->view;
    view->lines = init->lines;
    view->cols = init->cols;
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
