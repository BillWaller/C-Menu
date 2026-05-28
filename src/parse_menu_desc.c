/** @file parse_menu_desc.c
    @brief Parse menu description file and create Menu
    @author Bill Waller
    Copyright (c) 2025
    MIT License
    billxwaller@gmail.com
    @date 2026-02-09
 */

/** @defgroup parse_menu Menu Parser
    @brief Functions for parsing menu description files and creating Menu
   structures
 */

#include <common.h>
#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

unsigned int parse_menu_description(Init *);
unsigned int get_command_type(char *);

/** @brief Parse menu description file and create Menu
    @ingroup parse_menu
    @param init Pointer to Init structure containing menu information
    @return 0 on success, non-zero on failure
 */
unsigned int parse_menu_description(Init *init) {
    FILE *fp;
    char tmp_str[MAXLEN];
    char tmp_buf[MAXLEN];
    char in_buf[MAXLEN];
    char *in_buf_p;
    unsigned char ltr;
    bool fltr[127];
    int directive;
    int l;
    char *s;
    int commands = 0;
    int choices = 0;
    int in_fp_line = 0;
    menu = init->menu;
    fp = fopen(menu->mapp_spec, "r");
    if (fp == nullptr) {
        strnz__cpy(tmp_buf, "file not found", MAXLEN);
        abend(-1, tmp_buf);
        exit(-1);
    }
    while ((fgets(in_buf, MAXLEN, fp)) != nullptr) {
        if (in_buf[0] == '\0')
            continue;
        in_fp_line++;
        // Replace carriage returns, newlines, and tabs in in_buf with null
        // terminators and spaces
        chrep(in_buf, '\r', '\0');
        chrep(in_buf, '\n', '\0');
        chrep(in_buf, '\t', ' ');
        in_buf_p = in_buf;
        directive = *in_buf_p;
        in_buf_p++;

        strnz__cpy(tmp_buf, in_buf_p, MAXLEN);
        trim(tmp_buf);
        l = strlen(tmp_buf);
        if (l == 0)
            continue;
        if (directive == '#')
            continue;
        for (ltr = 0; ltr < 32; ltr++)
            fltr[ltr] = true;
        for (ltr = 32; ltr < 127; ltr++)
            fltr[ltr] = false;
        fltr['q'] = true;
        switch (directive) {
        /**  '!' Command */
        case '!':
            if (!menu->line_idx)
                break;
            if (menu->line[menu->line_idx - 1]->type != MT_TEXT)
                break;

            // Convert tmp_buf to command_str and determine command_type
            menu->line_idx--;
            menu->line[menu->line_idx]->command_str = strdup(tmp_buf);
            menu->line[menu->line_idx]->command_type =
                get_command_type(tmp_buf);
            s = menu->line[menu->line_idx]->raw_text;
            l = min(strlen(s), (size_t)(MAXLEN - 1));
            if (l > menu->choice_max_len)
                menu->choice_max_len = l;
            // if the choice text starts with '-' or '_',
            // reserve the next character as the choice_letter
            // and remove the choice_letter designator from the choice text
            if (*s == '-' || *s == '_') {
                s++;
                ltr = *s;
                if (ltr > 31 && ltr < 127) {
                    if (!fltr[ltr]) {
                        fltr[ltr] = true;
                        menu->line[menu->line_idx]->choice_letter = *s;
                    }
                }
                s++;
            }
            strnz__cpy(tmp_buf, " x - ", MAXLEN - 1);
            strnz__cat(tmp_buf, s, MAXLEN - 1);
            menu->line[menu->line_idx]->choice_text = strdup(tmp_buf);
            menu->line[menu->line_idx]->type = MT_CHOICE;
            menu->line_idx++;
            commands++;
            break;
            /**  ':' Choice */
        case ':':
            if (choices > commands) {
                ssnprintf(em0, MAXLEN - 1,
                          "More choices than commands at line %d of",
                          in_fp_line);
                strnz__cpy(em1, menu->mapp_spec, MAXLEN - 1);
                strnz__cpy(em2, in_buf, MAXLEN - 1);
                display_error(em0, em1, em2, nullptr);
                abend(-1, "unrecoverable error");
            }
            l = strlen(tmp_buf);
            menu->text_max_len = max(menu->text_max_len, l);
            if (!menu->title[0]) { /**< in_buf -> Title */
                // if menu title is not set, use this line as the title,
                l = min(l, (MAXLEN - 5));
                strnz__cpy(menu->title, tmp_buf, l);
                l += 4;
                menu->text_max_len = max(menu->text_max_len, l);
            } else {
                // otherwise add it as a text line
                menu->line[menu->line_idx] = calloc(1, sizeof(Line));
                if (menu->line[menu->line_idx] == (Line *)0) {
                    sprintf(tmp_str,
                            "2-malloc(%ld bytes) failed menu->line[%d]",
                            sizeof(Line), menu->line_idx);
                    abend(-1, tmp_str);
                }
                menu->line[menu->line_idx]->type = MT_TEXT;
                menu->line[menu->line_idx]->raw_text = strdup(tmp_buf);
                menu->line_idx++;
                choices++;
            }
            break;
        case '?':
            break; /**  ' ' Empty line, ignore */
        case ' ':
        case '\0':
        case '\n':
            break;
        default:
            ssnprintf(em0, MAXLEN - 1, "Invalid directive '%c' at line %d of",
                      directive, in_fp_line);
            strnz__cpy(em1, menu->mapp_spec, MAXLEN - 1);
            strnz__cpy(em2, in_buf, MAXLEN - 1);
            display_error(em0, em1, em2, nullptr);
        }
    }
    fclose(fp);
    menu->item_count = menu->line_idx;
    for (menu->line_idx = 0; menu->line_idx < menu->item_count;
         menu->line_idx++) {
        menu->line[menu->line_idx]->letter_pos = 1;
        // Try to get a choice_letter
        // skip past " x - "
        if (menu->line[menu->line_idx]->choice_letter != '\0') {
            ltr = menu->line[menu->line_idx]->choice_letter;
            s = menu->line[menu->line_idx]->choice_text + 5;
            while (*s != '\0') {
                if (*s == ltr) {
                    menu->line[menu->line_idx]->letter_pos =
                        s - menu->line[menu->line_idx]->choice_text;
                    break;
                }
                s++;
            }
            fltr[ltr] = true;
        } else {
            s = menu->line[menu->line_idx]->choice_text + 5;
            // Search string for first character that is not a space and not
            // already used as a choice_letter
            while (*s != '\0') {
                if (*s != ' ')
                    if (!fltr[(int)(uintptr_t)*s]) {
                        ltr = *s;
                        fltr[ltr] = true;
                        break;
                    }
                s++;
            }
            if (*s != '\0') {
                menu->line[menu->line_idx]->letter_pos =
                    s - menu->line[menu->line_idx]->choice_text;
                ltr = *s;
            } else {
                // If no letter found in choice text, find the first unused
                // letter in the ASCII range 32-126
                for (ltr = '0'; ltr < 127; ltr++)
                    if (fltr[ltr] == false) {
                        fltr[ltr] = true;
                        break;
                    }
                if (ltr > 126) {
                    Perror("Ran out of letters");
                    return 0;
                }
            }
        }
        menu->line[menu->line_idx]->choice_letter = ltr;
        menu->line[menu->line_idx]->choice_text[1] = ltr;
    }
    menu->lines = menu->item_count;
    if (menu->text_max_len > (menu->choice_max_len + 6))
        menu->cols = menu->text_max_len;
    else
        menu->cols = menu->choice_max_len + 6;
    if (menu->cols >= MAXLEN)
        Perror("line too long");
    return 0;
}
/** @brief Get command type from command string
    @ingroup parse_menu
    @param t Command string
    @return Command type as an unsigned int
 */
unsigned int get_command_type(char *t) {
    char *s, *p;

    s = p = t;
    while (*s != ' ' && *s != '\0') {
        if (*s == '/')
            p = s + 1;
        s++;
    }
    *s = '\0';
    if (!strcmp(p, "ckeys"))
        return (CT_CKEYS);
    else if (!strcmp(p, "exec"))
        return (CT_EXEC);
    else if (!strcmp(p, "help"))
        return (CT_HELP);
    else if (!strcmp(p, "about"))
        return (CT_ABOUT);
    else if (!strcmp(p, "menu"))
        return (CT_MENU);
    else if (!strcmp(p, "form"))
        return (CT_FORM);
    else if (!strcmp(p, "pick"))
        return (CT_PICK);
    else if (!strcmp(p, "return"))
        return (CT_RETURN);
    else if (!strcmp(p, "view"))
        return (CT_VIEW);
    else if (!strcmp(p, "?"))
        return (CT_HELP);
    else if (!strcmp(p, "write_config"))
        return (CT_WRITE_CONFIG);
    return (CT_UNDEFINED);
}
