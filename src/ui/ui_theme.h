#ifndef UI_THEME_H
#define UI_THEME_H 1

/** @file ui_theme.h
   @ingroup ui_backend
   @brief UI theme definitions.
 */
#include "ui_backend.h"

/** @brief UI role definitions.
   @ingroup ui_backend
   These are used to identify the purpose of a UI element, and can be used to
   apply different styles to different elements. The default theme will use these
   roles to apply different colors and styles to different elements.
 */
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

/** @brief UI theme definition.

   A UI theme is a collection of styles for different UI roles. The default theme
   will use the default styles for each role, but custom themes can be created
   by defining different styles for each role.
 */
typedef struct {
    UiStyle roles[16];
} UiTheme;

/** @brief Get the style for a given UI role.
   @ingroup ui_backend
   @param theme The UI theme to get the style from.
   @param role The UI role to get the style for.
   @return The style for the given UI role.
 */
const UiStyle *ui_theme_style(const UiTheme *theme, UiRole role);

#endif
