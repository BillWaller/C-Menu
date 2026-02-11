/** @file pick.h
 *  @brief Pick data structures, enums, types, end external declarations
 *  @author Bill Waller
 *  Copyright (c) 2025
 *  MIT License
 *  billxwaller@gmail.com
 *  @date 2026-02-09
 */

#ifndef _PICK_H
#define _PICK_H 1
#include "cm.h"

/// Pick structures, enums, and data types

#define OBJ_MAXLEN 80
#define OBJ_MAXCNT 1024

#ifndef _COMMON_H
typedef struct Init Init;
#endif

typedef struct {
    // colors & geometry
    int fg_color;  // F: foreground_color
    int bg_color;  // B: background_color
    int bo_color;  // O: border_color
    int win_lines; // L: lines
    int win_width; // C: columns
    int begy;      // Y: placement line
    int begx;      // X: placement column
    int y;         // line
    int x;         // column
    // window
    WINDOW *win, *box;
    char title[MAXLEN]; // T: title
    // argument processing
    int argc;
    char **argv;
    FILE *in_fp;
    FILE *out_fp;
    int in_fd;
    int out_fd;
    // files
    char mapp_spec[MAXLEN]; //    application qualified path
    char in_spec[MAXLEN];
    char out_spec[MAXLEN];
    char help_spec[MAXLEN]; //    application help qualified path
    char chyron_s[MAXLEN];  // (ˈkī-ˌrän) a banner at the bottom of the screen
    //
    char provider_cmd[MAXLEN]; // -S: provider command at start
    char receiver_cmd[MAXLEN]; // -R: receiver command at end
    char cmd[MAXLEN];
    // file flags
    bool f_mapp_spec;
    bool f_in_spec;
    bool f_out_spec;
    bool f_in_pipe;
    bool f_out_pipe;
    bool f_help_spec;
    bool f_multiple_cmd_args;
    bool f_stop_on_error;
    bool f_selected[OBJ_MAXCNT];
    bool help;
    bool f_provider_cmd;
    bool f_receiver_cmd;
    bool f_cmd;
    char in_buf[BUFSIZ];
    char **object;
    int select_idx;
    int select_cnt;
    int select_max;
    int obj_cnt;
    int obj_idx;
    int pg_line;
    int pg_lines;
    int pg_objs;
    int tab_idx;
    int tbl_pages;
    int tbl_page;
    int tbl_line;
    int tbl_lines;
    int tbl_cols;
    int tbl_col;
    int tbl_col_width;
} Pick;
extern Pick *pick;
/// pick functions

extern void save_object(Pick *, char *);
extern void display_page(Pick *);
extern void reverse_object(Pick *);
extern void toggle_object(Pick *);
extern int output_objects(Pick *);
extern int mpick(int, char **, int, int, int, int, char *, int);
#endif
