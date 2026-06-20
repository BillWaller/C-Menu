#ifndef UI_THEME_H
#define UI_THEME_H 1

#include "ui_backend.h"

typedef enum {
    UI_ROLE_DEFAULT = 0,
    UI_ROLE_NORMAL,
    UI_ROLE_NORMAL_REVERSE,
    UI_ROLE_NORMAL_HIGHLIGHT,
    UI_ROLE_NORMAL_HIGHLIGHT_REVERSE,
    UI_ROLE_BOX,
    UI_ROLE_TITLE,
    UI_ROLE_LINENO,
    UI_ROLE_CMDLINE,
    UI_ROLE_BOLD,
    UI_ROLE_FILL,
    UI_ROLE_BRACKET
} UiRole;

typedef struct {
    UiStyle roles[16];
} UiTheme;

const UiStyle *ui_theme_style(const UiTheme *theme, UiRole role);

#endif
