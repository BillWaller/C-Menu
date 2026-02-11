/** @file parse_menu_desc.c
 *  @brief Parse menu description file and create Menu
 *  @author Bill Waller
 *  Copyright (c) 2025
 *  MIT License
 *  billxwaller@gmail.com
 *  @date 2026-02-09
 */

#include "common.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

unsigned int parse_menu_description(Init *);
unsigned int get_command_type(char *);
void free_menu_line(Line *);

unsigned int parse_menu_description(Init *init) {
    FILE *fp;
    char tmp_buf[MAXLEN + 1];
    char in_buf[MAXLEN + 1];
    char *in_buf_p;
    int Pos;
    unsigned char ltr;
    unsigned char fltr[127];
    int directive;
    int l;
    char *s, *d, *e;
    int actions = 0;
    int choices = 0;
    int in_fp_line = 0;
    menu = init->menu;
    fp = fopen(menu->mapp_spec, "r");
    if (fp == NULL) {
        strnz__cat(tmp_buf, "file not found", MAXLEN);
        abend(-1, tmp_buf);
        exit(-1);
    }
    while ((fgets(in_buf, MAXLEN, fp)) != NULL) {
        if (in_buf[0] == '\0')
            continue;
        in_fp_line++;
        chrep(in_buf, '\r', '\0');
        chrep(in_buf, '\n', '\0');
        chrep(in_buf, '\t', ' ');
        in_buf_p = in_buf;
        directive = *in_buf_p;
        in_buf_p++;
        strnz__cpy(tmp_buf, in_buf_p, MAXLEN);
        trim(tmp_buf);
        l = strlen(tmp_buf);
        if (!l)
            continue;
        switch (directive) {
        case '#':
            break;
        case '!':
            if (!menu->line_idx)
                break;
            if (menu->line[menu->line_idx - 1]->type != MT_TEXT)
                break;
            menu->line_idx--;
            l = strlen(tmp_buf);
            if (l + 1 > MAXLEN)
                break;
            menu->line[menu->line_idx]->command_str =
                (char *)malloc(MAXLEN + 1);
            if (!menu->line[menu->line_idx]->command_str) {
                sprintf(tmp_str,
                        "0-malloc(%d bytes) failed M-L[%d]->command_str",
                        MAXLEN + 1, menu->line_idx);
                abend(-1, tmp_str);
            }
            menu->line[menu->line_idx]->command_str = strdup(tmp_buf);
            menu->line[menu->line_idx]->command_type =
                get_command_type(tmp_buf);
            s = menu->line[menu->line_idx]->raw_text;
            if (*s == '-' || *s == '_') {
                s++;
                ltr = *s++;
            } else
                ltr = *s;
            l = 0;
            while (*s++ != '\0')
                l++;
            if (l + 1 > MAXLEN)
                l = MAXLEN - 1;
            menu->line[menu->line_idx]->choice_text =
                (char *)malloc(MAXLEN + 1);
            if (!menu->line[menu->line_idx]->choice_text) {
                sprintf(tmp_str,
                        "1-malloc(%d bytes) failed M-L[%d]->choice_text",
                        MAXLEN + 1, menu->line_idx);
                abend(-1, tmp_str);
            }
            d = menu->line[menu->line_idx]->choice_text;
            e = d + l;
            s = menu->line[menu->line_idx]->raw_text;
            if (*s == '-' || *s == '_')
                s += 2;
            while (*s != '\0' && d < e)
                *d++ = *s++;
            *d = (char)'\0';
            if (l > menu->choice_max_len)
                menu->choice_max_len = l;
            if (menu->line[menu->line_idx]->command_type == CT_RETURN)
                menu->line[menu->line_idx]->choice_letter = 'Q';
            else
                menu->line[menu->line_idx]->choice_letter = ltr;
            menu->line[menu->line_idx]->type = MT_CHOICE;
            menu->line_idx++;
            actions++;
            break;
        case ':':
            if (choices > actions) {
                strnz__cpy(em0, "More choices than actions at", MAXLEN - 1);
                ssnprintf(em0, MAXLEN - 1,
                          "More choices than actions at line %d of",
                          in_fp_line);
                strnz__cpy(em1, menu->mapp_spec, MAXLEN - 1);
                strnz__cat(em2, in_buf, MAXLEN - 1);
                display_error(em0, em1, em2, NULL);
                abend(-1, "unrecoverable error");
            }
            chrep(tmp_buf, '\t', ' ');
            l = strlen(tmp_buf);
            if (l > menu->text_max_len)
                menu->text_max_len = l;
            if (!menu->title[0]) { /// in_buf -> Title
                if (l + 5 > MAXLEN)
                    l = MAXLEN - 5;
                strnz__cpy(menu->title, tmp_buf, l);
                l += 4;
                if (l > menu->text_max_len)
                    menu->text_max_len = l;
            } else {
                menu->line[menu->line_idx] = (Line *)malloc(sizeof(Line));
                if (menu->line[menu->line_idx] == (Line *)0) {
                    sprintf(tmp_str,
                            "2-malloc(%ld bytes) failed menu->line[%d]",
                            sizeof(Line), menu->line_idx);
                    abend(-1, tmp_str);
                }
                menu->line[menu->line_idx]->type = MT_TEXT;
                menu->line[menu->line_idx]->raw_text = strdup(tmp_buf);
                menu->line[menu->line_idx]->choice_text = NULL;
                menu->line[menu->line_idx]->choice_letter = '\0';
                menu->line[menu->line_idx]->letter_pos = 0;
                menu->line[menu->line_idx]->command_type = '\0';
                menu->line[menu->line_idx]->command_str = NULL;
                menu->line_idx++;
                choices++;
            }
            break;
        case '?':
            break;
        case ' ':
        case '\0':
        case '\n':
            break;
        default:
            strnz__cpy(tmp_buf, "unrecognized operator in ", MAXLEN);
            abend(-1, tmp_buf);
        }
    }
    fclose(fp);
    menu->item_count = menu->line_idx;
    for (ltr = '0'; ltr < 'z'; ltr++)
        fltr[ltr] = FALSE;
    fltr['Q'] = TRUE;
    for (menu->line_idx = 0; menu->line_idx < menu->item_count;
         menu->line_idx++) {
        ltr = menu->line[menu->line_idx]->choice_letter;
        if (ltr < '0' || ltr > 'z')
            ltr = '0';
        if (!fltr[ltr] ||
            (menu->line[menu->line_idx]->command_type == CT_RETURN &&
             ltr == 'Q')) {
            fltr[ltr] = TRUE;
            s = menu->line[menu->line_idx]->choice_text;
            Pos = 0;
            while (*s != '\0') {
                if (*s == ltr)
                    break;
                s++;
                Pos++;
            }
        } else {
            Pos = 0;
            while (ltr != '\0') {
                Pos++;
                ltr = toupper(menu->line[menu->line_idx]->choice_text[Pos]);
                if (ltr >= '0' && ltr <= 'Z')
                    if (!fltr[ltr])
                        break;
            }
            if (ltr == '\0') {
                for (ltr = '0'; ltr < 'Z'; ltr++)
                    if (!fltr[ltr])
                        break;
                if (ltr > 'z') {
                    Perror("Ran out of letters");
                    return (0);
                }
            }
            fltr[ltr] = TRUE;
            menu->line[menu->line_idx]->choice_letter = ltr;
        }
        menu->line[menu->line_idx]->letter_pos = Pos;
    }
    menu->lines = menu->item_count;
    if (menu->text_max_len > (menu->choice_max_len + 6))
        menu->cols = menu->text_max_len;
    else
        menu->cols = menu->choice_max_len + 6;
    if (menu->cols >= MAXLEN)
        Perror("line too long");
    for (menu->line_idx = 0; menu->line_idx < menu->item_count;
         menu->line_idx++) {
        strnz__cpy(tmp_buf, " x - ", MAXLEN - 1);
        tmp_buf[1] = menu->line[menu->line_idx]->choice_letter;
        strnz__cat(tmp_buf, menu->line[menu->line_idx]->choice_text,
                   MAXLEN - 1);
        strnz__cpy(menu->line[menu->line_idx]->choice_text, tmp_buf,
                   MAXLEN - 1);
    }

    return (0);
}
unsigned int get_command_type(char *t) {
    char *s, *p;

    s = p = t;
    while (*s != ' ' && *s != '\0') {
        if (*s == '/')
            p = s + 1;
        s++;
    }
    *s = '\0';
    if (!strcmp(p, "returnmain"))
        return (CT_RETURNMAIN);
    else if (!strcmp(p, "ckeys"))
        return (CT_CKEYS);
    else if (!strcmp(p, "exec"))
        return (CT_EXEC);
    else if (!strcmp(p, "help"))
        return (CT_HELP);
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
void free_menu_line(Line *line) {

    if (line->raw_text != NULL)
        free(line->raw_text);
    if (line->choice_text != NULL)
        free(line->choice_text);
    if (line->command_str != NULL)
        free(line->command_str);
    line->raw_text = NULL;
    line->choice_text = NULL;
    line->choice_letter = '\0';
    line->letter_pos = 0;
    line->command_type = '\0';
    line->command_str = NULL;
    free(line);
}
