/** @file pick.h
    @brief Pick data structures, enums, types, end external declarations
    @author Bill Waller
    Copyright (c) 2025
    MIT License
    billxwaller@gmail.com
    @date 2026-02-09
 */

#ifndef _PICK_H
#define _PICK_H 1
#include <cm.h>

/** Pick structures, enums, and data types */

#define OBJ_MAXLEN 80
#define OBJ_MAXCNT 1024

#ifndef _COMMON_H
typedef struct Init Init;
#endif

/** @struct Pick
   @brief Pick data structure */
typedef struct {
    int fg_color;                /**< foreground color */
    int bg_color;                /**< background color */
    int bo_color;                /**< box color */
    int win_lines;               /**< window lines */
    int win_width;               /**< window width (columns)*/
    int begy;                    /**< begin y screen position of window top */
    int begx;                    /**< begin x screen position of windor left */
    int y;                       /**< current y (line)*/
    int x;                       /**< current x (column) */
    WINDOW *win;                 /**< pointer to window */
    WINDOW *box;                 /**< pointer to box */
    char title[MAXLEN];          /**< title string */
    int argc;                    /**< argument count */
    char **argv;                 /**< argument vector */
    FILE *in_fp;                 /**< pointer to input file */
    FILE *out_fp;                /**< pointer to output file */
    int in_fd;                   /**< input file descriptor */
    int out_fd;                  /**< output file descriptor */
    char mapp_spec[MAXLEN];      /**< mapp description file spec */
    char in_spec[MAXLEN];        /**< input file spec or input pipe spec */
    char out_spec[MAXLEN];       /**< output file spec or output pipe spec */
    char help_spec[MAXLEN];      /**< help file spec */
    char chyron_s[MAXLEN];       /**< chyron string */
    char provider_cmd[MAXLEN];   /**< provider command spec */
    char receiver_cmd[MAXLEN];   /**< receiver command spec */
    char cmd[MAXLEN];            /**< command spec */
    bool f_mapp_spec;            /**< flag: mapp spec verified */
    bool f_in_spec;              /**< flag: input spec verified */
    bool f_out_spec;             /**< flag: output spec verified */
    bool f_in_pipe;              /**< flag: input spec is a pipe */
    bool f_out_pipe;             /**< flag: output spec is a pipe */
    bool f_help_spec;            /**< flag: help spec verified */
    bool f_multiple_cmd_args;    /**< flag: multiple command arguments */
    bool f_stop_on_error;        /**< flag: stop on error */
    bool f_selected[OBJ_MAXCNT]; /**< flag: object selected */
    bool help;                   /**< flag: help requested */
    bool f_provider_cmd;         /**< flag: provider command verified */
    bool f_receiver_cmd;         /**< flag: receiver command verified */
    bool f_cmd;                  /**< flag: command verified */
    char in_buf[BUFSIZ];         /**< input buffer */
    char **object;               /**< object array */
    int select_idx;              /**< index of current selected object */
    int select_cnt;              /**< count of selected objects */
    int select_max;              /**< maximum number of selected objects */
    int obj_cnt;                 /**< count of objects */
    int obj_idx;                 /**< index of current object */
    int pg_line;                 /**< current line on page */
    int pg_lines;                /**< lines per page */
    int pg_objs;                 /**< objects per page */
    int tab_idx;                 /**< index of current tab */
    int tbl_pages;               /**< total number of table pages */
    int tbl_page;                /**< current table page */
    int tbl_line;                /**< current line on table page */
    int tbl_lines;               /**< lines per table page */
    int tbl_cols;                /**< columns per table page */
    int tbl_col;                 /**< current column on table page */
    int tbl_col_width;           /**< column width on table page */
} Pick;
extern Pick *pick; /**< pointer to Pick data structure */

extern void save_object(Pick *, char *);
extern void display_page(Pick *);
extern void reverse_object(Pick *);
extern void toggle_object(Pick *);
extern int output_objects(Pick *);
extern int mpick(int, char **, int, int, int, int, char *, int);
#endif
