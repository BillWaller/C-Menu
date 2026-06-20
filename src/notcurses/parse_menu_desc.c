#include "nc.h"

#include "../include/common.h"

#include <notcurses/notcurses.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned int parse_menu_description(Init *init) {
    FILE *fp;
    char tmp_str[MAXLEN];
    char tmp_buf[MAXLEN];
    char in_buf[MAXLEN];
    char *in_buf_p;
    unsigned char ltr;
    bool fltr[127];
    int directive;
    int screen_width; // Tracks visible columns instead of raw bytes
    char *s;
    int commands = 0;
    int choices = 0;
    int in_fp_line = 0;
    int vb, vw;

    menu = init->menu;

    for (ltr = 0; ltr < 32; ltr++)
        fltr[ltr] = true;
    for (ltr = 32; ltr < 127; ltr++)
        fltr[ltr] = false;
    fltr['q'] = true;

    fp = fopen(menu->mapp_spec, "r");
    if (fp == NULL) {
        strnz__cpy(tmp_buf, "file not found", MAXLEN);
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

        // Use ncstrwidth to calculate visible character space instead of bytes
        screen_width = ncstrwidth(tmp_buf, &vb, &vw);
        if (screen_width <= 0)
            continue;
        if (directive == '#')
            continue;

        switch (directive) {
        /** '!' Command */
        case '!':
            if (!menu->line_idx)
                break;
            if (menu->line[menu->line_idx - 1]->type != MT_TEXT)
                break;

            menu->line_idx--;
            menu->line[menu->line_idx]->command_str = strdup(tmp_buf);
            menu->line[menu->line_idx]->command_type = get_command_type(tmp_buf);

            s = menu->line[menu->line_idx]->raw_text;
            // Calculate columns for choice layout max constraints
            int s_width = ncstrwidth(s, &vb, &vw);
            if (s_width > menu->choice_max_len) {
                menu->choice_max_len = s_width;
            }

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

        /** ':' Choice */
        case ':':
            if (choices > commands) {
                ssnprintf(em0, MAXLEN - 1, "More choices than commands at line %d of", in_fp_line);
                strnz__cpy(em1, menu->mapp_spec, MAXLEN - 1);
                strnz__cpy(em2, in_buf, MAXLEN - 1);
                display_error(em0, em1, em2, NULL);
                abend(-1, "unrecoverable error");
            }

            if (screen_width > menu->text_max_len) {
                menu->text_max_len = screen_width;
            }

            if (!menu->title[0]) {
                /**< in_buf -> Title */
                int max_title_len = (MAXLEN - 5);
                strnz__cpy(menu->title, tmp_buf, max_title_len);

                int title_width = ncstrwidth(menu->title, &vb, &vw) + 4;
                if (title_width > menu->text_max_len) {
                    menu->text_max_len = title_width;
                }
            } else {
                menu->line[menu->line_idx] = calloc(1, sizeof(Line));
                if (menu->line[menu->line_idx] == NULL) {
                    sprintf(tmp_str, "2-malloc(%ld bytes) failed menu->line[%d]", sizeof(Line), menu->line_idx);
                    abend(-1, tmp_str);
                }
                menu->line[menu->line_idx]->type = MT_TEXT;
                menu->line[menu->line_idx]->raw_text = strdup(tmp_buf);
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
            ssnprintf(em0, MAXLEN - 1, "Invalid directive '%c' at line %d of", directive, in_fp_line);
            strnz__cpy(em1, menu->mapp_spec, MAXLEN - 1);
            strnz__cpy(em2, in_buf, MAXLEN - 1);
            display_error(em0, em1, em2, NULL);
        }
    }
    fclose(fp);

    menu->item_count = menu->line_idx;

    // Assigning selection tracking shortcut hotkeys
    for (menu->line_idx = 0; menu->line_idx < menu->item_count; menu->line_idx++) {
        menu->line[menu->line_idx]->letter_pos = 1;

        if (menu->line[menu->line_idx]->choice_letter != '\0') {
            ltr = menu->line[menu->line_idx]->choice_letter;
            s = menu->line[menu->line_idx]->choice_text + 5;
            while (*s != '\0') {
                if (*s == ltr) {
                    menu->line[menu->line_idx]->letter_pos = s - menu->line[menu->line_idx]->choice_text;
                    break;
                }
                s++;
            }
            fltr[ltr] = true;
        } else {
            s = menu->line[menu->line_idx]->choice_text + 5;
            while (*s != '\0') {
                ltr = *s;
                if (ltr != ' ') {
                    if (!fltr[ltr]) {
                        fltr[ltr] = true;
                        break;
                    }
                }
                s++;
            }
            if (*s != '\0') {
                menu->line[menu->line_idx]->letter_pos = s - menu->line[menu->line_idx]->choice_text;
                ltr = *s;
            } else {
                for (ltr = '0'; ltr < 127; ltr++) {
                    if (fltr[ltr] == false) {
                        fltr[ltr] = true;
                        break;
                    }
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

    // Setting global panel column limits safely based on screen widths
    if (menu->text_max_len > (menu->choice_max_len + 6)) {
        menu->cols = menu->text_max_len;
    } else {
        menu->cols = menu->choice_max_len + 6;
    }

    if (menu->cols >= MAXLEN) {
        Perror("line too long");
    }

    return 0;
}
