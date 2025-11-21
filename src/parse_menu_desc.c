/* parse_menu_desc.c
 * Bill Waller
 * billxwaller@gmail.com
 * Parse menu description file and create Menu
 */

#include "menu.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

int parse_menu_description(Init *);
char get_command_type(char *);
void free_menu_line(Line *);

int parse_menu_description(Init *init) {
    FILE *fp;
    char tmp_buf[MAXLEN + 1];
    char in_buf[MAXLEN + 1];
    int Pos;
    unsigned char ltr;
    unsigned char fltr[127];
    int col, cnt;
    int l;
    char *s, *d, *e, *p;
    menu = init->menu;
    if ((fp = fopen(menu->mapp_spec, "r")) == NULL) {
        strncat(tmp_buf, "file not found", MAXLEN);
        abend(-1, tmp_buf);
    }
    while ((fgets(in_buf, MAXLEN, fp)) != NULL) {
        if (in_buf[0] == '\0')
            continue;
        switch ((int)in_buf[0]) {
        case '#':
            break;
        case '!':
            if (!menu->line_idx)
                break;
            if (menu->line[menu->line_idx - 1]->type != MT_CENTERED_TEXT)
                break;
            menu->line_idx--;

            s = in_buf; /* in_buf -> command_str */
            s++;
            l = 0;
            while (*s != '\n' && *s != '\0') {
                s++;
                l++;
            }
            if (l + 1 > MAX_COLS)
                l = MAX_COLS - 1;
            d = menu->line[menu->line_idx]->command_str =
                (char *)malloc(MAX_COLS + 1);
            if (!d) {
                sprintf(tmp_str, "malloc(%d bytes) failed M-L[%d]->command_str",
                        MAX_COLS + 1, menu->line_idx);
                abend(-1, tmp_str);
            }
            e = d + l;
            s = in_buf;
            s++;
            while (*s != '\0' && d < e) {
                if (*s == '\t')
                    *s = ' ';
                *d++ = *s++;
            }
            *d = (char)'\0';
            s = menu->line[menu->line_idx]->command_str;
            d = tmp_buf;
            while (*s != '\0')
                *d++ = *s++;
            *d = '\0';
            menu->line[menu->line_idx]->command_type =
                get_command_type(tmp_buf);

            s = menu->line[menu->line_idx]
                    ->raw_text; /* raw_text -> choice_text */
            if (*s == '-' || *s == '_') {
                s++;
                ltr = *s++;
            } else
                ltr = *s;
            l = 0;
            while (*s++ != '\0')
                l++;
            if (l + 1 > MAX_COLS)
                l = MAX_COLS - 1;
            d = menu->line[menu->line_idx]->choice_text =
                (char *)malloc(MAX_COLS + 1);
            if (!d) {
                sprintf(tmp_str, "malloc(%d bytes) failed M-L[%d]->choice_text",
                        MAX_COLS + 1, menu->line_idx);
                abend(-1, tmp_str);
            }
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
            menu->line[menu->line_idx]->option_idx = 0;
            menu->line[menu->line_idx]->option_cnt = 0;
            menu->line_idx++;
            break;

        case ':':
            s = in_buf;
            s++;
            d = tmp_buf;
            e = d + MAX_COLS;
            while (*s == ' ' || *s == '\t')
                s++;
            l = 0;
            while (*s != '\n' && *s != '\0' && d < e) {
                if (*s == '\t')
                    *s = ' ';
                *d++ = *s++;
                l++;
            }
            *d = '\0';
            if (!l)
                break;
            s = tmp_buf;
            if (!l)
                break;
            if (l > menu->text_max_len)
                menu->text_max_len = l;
            if (!menu->title[0]) { /* in_buf -> Title */
                if (l + 5 > MAX_COLS)
                    l = MAX_COLS - 5;
                strncpy(menu->title, tmp_buf, MAXLEN - 1);
                e = d + l;
                while (*s != '\0' && d < e)
                    *d++ = *s++;
                *d = '\0';
                l += 4;
                if (l > menu->text_max_len)
                    menu->text_max_len = l;
            } else { /* in_buf -> raw_text */
                menu->line[menu->line_idx] = (Line *)malloc(sizeof(Line));
                if (menu->line[menu->line_idx] == (Line *)0) {
                    sprintf(tmp_str, "malloc(%ld bytes) failed menu->line[%d]",
                            sizeof(Line), menu->line_idx);
                    abend(-1, tmp_str);
                }
                menu->line[menu->line_idx]->type = MT_CENTERED_TEXT;
                if (*s == '.') {
                    l--;
                    s++;
                }
                if (l + 1 > MAX_COLS)
                    l = MAX_COLS - 1;
                d = menu->line[menu->line_idx]->raw_text =
                    (char *)malloc(MAX_COLS + 1);
                if (!d) {
                    sprintf(tmp_str,
                            "malloc(%d bytes) failed M-L[%d]->raw_text",
                            MAX_COLS + 1, menu->line_idx);
                    abend(-1, tmp_str);
                }
                e = d + l;
                while (*s != '\0' && d < e)
                    *d++ = *s++;
                *d = (char)'\0';
                menu->line[menu->line_idx]->choice_text = NULL;
                menu->line[menu->line_idx]->choice_letter = '\0';
                menu->line[menu->line_idx]->letter_pos = 0;
                menu->line[menu->line_idx]->command_type = '\0';
                menu->line[menu->line_idx]->command_str = NULL;
                menu->line[menu->line_idx]->option_ptr[0] = NULL;
                menu->line[menu->line_idx]->option_col = 0;
                menu->line[menu->line_idx]->option_idx = 0;
                menu->line[menu->line_idx]->option_cnt = 0;
                menu->line_idx++;
            }
            break;

        case '?':
            if (!menu->line_idx)
                break;
            if (menu->line[menu->line_idx - 1]->type != MT_CENTERED_TEXT)
                break;
            menu->line_idx--;
            s = in_buf;
            s++;
            d = tmp_buf;
            while (*s != '?' && *s != '\0' && *s != '\n')
                *d++ = *s++;
            *d = '\0';
            if (*s != '?' || (col = atoi(tmp_buf)) < 0 || col >= MAX_COLS) {
                menu->line[menu->line_idx]->option_cnt = 0;
                break;
            }
            menu->line[menu->line_idx]->option_col = col;
            cnt = 0;
            p = s;
            while (*++p != '\0' && *p != '\n') {
                d = tmp_buf;
                l = 0;
                while (*p != '?' && *p != '\0' && *p != '\n') {
                    *d++ = *p++;
                    l++;
                }
                *d = '\0';
                if (!l)
                    break;
                s = tmp_buf;
                if (l + 1 > MAX_COLS)
                    l = MAX_COLS - 1;
                d = menu->line[menu->line_idx]->option_ptr[cnt] =
                    (char *)malloc(MAX_COLS + 1);
                if (!d) {
                    sprintf(tmp_str,
                            "malloc(%d bytes) failed M-L[%d]->option_ptr[%d]",
                            MAX_COLS + 1, menu->line_idx, cnt);
                    abend(-1, tmp_str);
                }
                e = d + l;
                while (*s != '\0' && d < e)
                    *d++ = *s++;
                *d = (char)'\0';
                if (l > menu->option_max_len)
                    menu->option_max_len = l;
                cnt++;
                if (cnt >= MAXOPTS)
                    break;
            }
            menu->line[menu->line_idx]->option_cnt = cnt - 1;
            menu->line[menu->line_idx]->option_idx = 0;
            menu->line[menu->line_idx]->command_type = CT_TOGGLE;

            s = menu->line[menu->line_idx]
                    ->raw_text; /* raw_text -> choice_text */
            if (*s == '-' || *s == '_') {
                s++;
                ltr = *s++;
            } else
                ltr = *s;
            l = 0;
            while (*s++ != '\0')
                l++;
            if (l + 1 > MAX_COLS)
                l = MAX_COLS - 1;
            d = menu->line[menu->line_idx]->choice_text =
                (char *)malloc(MAX_COLS + 1);
            if (d == (char *)0) {
                sprintf(tmp_str, "malloc(%d bytes) failed M-L[%d]->choice_text",
                        MAX_COLS + 1, menu->line_idx);
                abend(-1, tmp_str);
            }
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
            break;

        case ' ':
        case '\0':
        case '\n':
            break;
        default:
            strncpy(tmp_buf, "unrecognized operator in ", MAXLEN);
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
                    display_error_message("Ran out of letters");
                    return (0);
                }
            }
            fltr[ltr] = TRUE;
            menu->line[menu->line_idx]->choice_letter = ltr;
        }
        menu->line[menu->line_idx]->letter_pos = Pos;
    }

    menu->lines = menu->item_count;
    if (menu->option_max_len > 0)
        menu->option_max_len += 2;
    if (menu->text_max_len > (menu->choice_max_len + menu->option_max_len + 6))
        menu->cols = menu->text_max_len;
    else
        menu->cols = menu->choice_max_len + menu->option_max_len + 6;
    if (menu->cols >= MAX_COLS)
        display_error_message("line too long");
    menu->option_offset = menu->choice_max_len + 7;

    for (menu->line_idx = 0; menu->line_idx < menu->item_count;
         menu->line_idx++) {
        s = menu->line[menu->line_idx]->choice_text;
        d = tmp_buf;
        e = d + menu->choice_max_len + 6;
        *d++ = ' ';
        *d++ = ltr = menu->line[menu->line_idx]->choice_letter;
        *d++ = ' ';
        *d++ = '-';
        *d++ = ' ';
        l = 5;
        while (*s != '\0' && d < e) {
            if (ltr != '\0')
                if (*s == ltr) {
                    *d++ = *s;
                    *d++ = '\b';
                    e += 2;
                    l += 2;
                    ltr = '\0';
                }
            *d++ = *s++;
            l++;
        }
        while (d < e) {
            *d++ = ' ';
            l++;
        }
        *d = '\0';
        if (l + 1 > MAX_COLS)
            l = MAX_COLS - 1;
        s = tmp_buf;
        d = menu->line[menu->line_idx]->choice_text;
        e = d + l;
        while (*s != '\0' && d < e)
            *d++ = *s++;
        *d = '\0';
    }

    return (0);
}

char get_command_type(char *t) {
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
    if (!strcmp(p, "cpick"))
        return (CT_PICK);
    if (!strcmp(p, "ckeys"))
        return (CT_CKEYS);
    if (!strcmp(p, "exec"))
        return (CT_EXEC);
    if (!strcmp(p, "help"))
        return (CT_HELP);
    if (!strcmp(p, "menu"))
        return (CT_MENU);
    if (!strcmp(p, "option"))
        return (CT_MENU);
    if (!strcmp(p, "form"))
        return (CT_FORM);
    if (!strcmp(p, "form_exec"))
        return (CT_FORM);
    if (!strcmp(p, "form_write"))
        return (CT_FORM);
    if (!strcmp(p, "pick"))
        return (CT_PICK);
    if (!strcmp(p, "return"))
        return (CT_RETURN);
    if (!strcmp(p, "view"))
        return (CT_VIEW);
    if (!strcmp(p, "?"))
        return (CT_TOGGLE);
    if (!strcmp(p, "write_config"))
        return (CT_WRITE_CONFIG);
    return (CT_UNDEFINED);
}

void free_menu_line(Line *line) {
    int j;

    line->type = '\0';

    if (line->raw_text != NULL)
        free(line->raw_text);
    if (line->choice_text != NULL)
        free(line->choice_text);
    if (line->command_str != NULL)
        free(line->command_str);
    for (j = 0; j < line->option_cnt && j < MAXOPTS; j++) {
        if (line->option_ptr[j] != NULL) {
            free(line->option_ptr[j]);
            line->option_ptr[j] = NULL;
        }
    }
    line->raw_text = NULL;
    line->choice_text = NULL;
    line->choice_letter = '\0';
    line->letter_pos = 0;
    line->command_type = '\0';
    line->command_str = NULL;
    line->option_ptr[0] = NULL;
    line->option_col = 0;
    line->option_idx = 0;
    line->option_cnt = 0;
    free(line);
}
