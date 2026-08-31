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
#define OBJ_MAXCNT 4096

#ifndef _COMMON_H
typedef struct Init Init;
#endif

/** @struct Pick
   @brief Pick data structure */
typedef struct {
    uint fg_clr_idx; /**< foreground color */
    uint bg_clr_idx; /**< background color */
    uint bo_clr_idx; /**< box color */
    uint lines;      /**< window lines */
    uint width;      /**< window width (columns)*/
    uint begy;       /**< begin y screen position of window top */
    uint begx;       /**< begin x screen position of windor left */
    uint y;          /**< current y (line)*/
    uint x;          /**< current x (column) */
#ifdef ASDF
    WINDOW *win;  /**< pointer to window */
    WINDOW *win2; /**< pointer to 2nd window */
    WINDOW *box;  /**< pointer to box */
#endif
    UiSurface *surface;
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
    char provider_cmd[MAXLEN];   /**< provider command spec */
    char receiver_cmd[MAXLEN];   /**< receiver command spec */
    char cmd[MAXLEN];            /**< command spec */
    char parent_cmd[MAXLEN];     /**< command to be executed by parent */
    bool f_mapp_spec;            /**< flag: mapp spec verified */
    bool f_in_spec;              /**< flag: input spec verified */
    bool f_out_spec;             /**< flag: output spec verified */
    bool f_in_pipe;              /**< flag: input spec is a pipe */
    bool f_out_pipe;             /**< flag: output spec is a pipe */
    bool f_help_spec;            /**< flag: help spec verified */
    bool f_read_theme;           /**< flag: read and process default theme */
    bool f_multiple_cmd_args;    /**< flag: multiple command arguments */
    bool p_view_files;           /**< flag: View pick files */
    bool f_selected[OBJ_MAXCNT]; /**< flag: object selected */
    bool help;                   /**< flag: help requested */
    bool f_provider_cmd;         /**< flag: provider command verified */
    bool f_receiver_cmd;         /**< flag: receiver command verified */
    bool f_cmd;                  /**< flag: command verified */
    char in_buf[BUFSIZ];         /**< input buffer */
    char **m_object;             /**< master object table (as read from input) */
    uint select_idx;             /**< index of current selected object */
    uint select_cnt;             /**< count of selected objects */
    uint select_max;             /**< maximum number of selected objects */
    uint m_cnt;
    uint d_cnt;
    uint m_idx;      /**< count of objects */
    uint d_idx;      /**< index of current object */
    char **d_object; /**< derived object table */
    uint y_offset;
    uint pg_line;        /**< current line on page */
    uint pg_lines;       /**< lines per page */
    uint pg_objs;        /**< objects per page */
    uint tab_idx;        /**< index of current tab */
    uint tbl_pages;      /**< total number of table pages */
    uint tbl_page;       /**< current table page */
    uint tbl_line;       /**< current line on table page */
    uint tbl_lines;      /**< lines per table page */
    uint tbl_cols;       /**< columns per table page */
    uint tbl_col;        /**< current column on table page */
    uint tbl_col_width;  /**< column width on table page */
    uint separator_line; /**< separator between object selector and line editor
                          */
    UiChyron *chyron;    /**< chyron data structure */
} Pick;
// extern Pick *pick; /**< pointer to Pick data structure */

extern void save_object(Pick *, char *);
extern void display_pick_page(Pick *);
extern void reverse_object(Pick *);
extern void toggle_object(Pick *);
extern int output_objects(Pick *);
extern int mpick(int, char **, int, int, int, int, char *, int);
#endif
