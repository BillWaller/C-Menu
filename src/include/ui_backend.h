#ifndef UI_BACKEND_H
#define UI_BACKEND_H 1

/** @file ui_backend.h
    @ingroup ui_backend
    @brief Backend API for terminal UI library
*/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MAXWIN 30

/** @defgroup ui_backend UI Backend
    @brief Backend API for terminal UI library

    This module defines the API for implementing a backend for the terminal UI library.
    It includes types and functions for managing the UI runtime, surfaces, drawing operations,
    input handling, and cursor control.

    The backend is responsible for rendering the UI to the terminal, handling user input events,
    and managing the lifecycle of the UI runtime and surfaces.

    To implement a backend, you need to define the functions declared in this header file
    according to the specifications provided in the function comments.

    @see ui_backend.h
*/

/** @struct UiRuntime
   @brief Opaque structure representing the UI runtime environment.
   @ingroup ui_backend
   @details The UiRuntime structure encapsulates the state and resources associated with the UI runtime. It is initialized using the ui_init function and should be properly shut down using ui_shutdown to release any allocated resources.
   The UiRuntime is responsible for managing the overall UI environment, including rendering, input handling, and surface management. It serves as the central context for all UI operations.
   @see ui_init
   @see ui_shutdown
 */
typedef struct UiRuntime UiRuntime;

/** @struct UiSurface
   @brief Opaque structure representing a drawable surface in the UI.
   @ingroup ui_backend
   @details The UiSurface structure represents a drawable area within the UI. It is created using the ui_surface_new function and should be destroyed using ui_surface_destroy to free any associated resources.
   A UiSurface can be thought of as a canvas on which you can draw text, lines, borders, and other UI elements. Surfaces can be nested, allowing for complex layouts and hierarchies. Each surface has its own position and size, and can be shown or hidden as needed.
   @see ui_surface_new
   @see ui_surface_destroy
*/
typedef struct UiSurface UiSurface;

/** @enum UiKey
   @brief Enumeration of possible key events in the UI.
   @ingroup ui_backend
   @details The UiKey enumeration defines the various key events that can be captured in the UI. This includes character input, special keys (like Enter, Escape, Arrow keys), function keys (F1-F12), and mouse events.
   Each key event is represented by a unique value in the enumeration, allowing for easy identification and handling of user input. When a key event occurs, it is captured in a UiEvent structure, which includes additional information such as modifier keys (Alt, Ctrl, Shift) and mouse actions.
   @see UiEvent
*/
typedef enum {
    UI_KEY_NONE = 0,
    UI_KEY_CHAR,
    UI_KEY_ENTER,
    UI_KEY_ESCAPE,
    UI_KEY_BACKSPACE,
    UI_KEY_TAB,
    UI_KEY_BTAB,
    UI_KEY_UP,
    UI_KEY_DOWN,
    UI_KEY_LEFT,
    UI_KEY_RIGHT,
    UI_KEY_HOME,
    UI_KEY_END,
    UI_KEY_PGUP,
    UI_KEY_PGDN,
    UI_KEY_INSERT,
    UI_KEY_DELETE,
    UI_KEY_RESIZE,
    UI_KEY_MOUSE,
    UI_KEY_F1,
    UI_KEY_F2,
    UI_KEY_F3,
    UI_KEY_F4,
    UI_KEY_F5,
    UI_KEY_F6,
    UI_KEY_F7,
    UI_KEY_F8,
    UI_KEY_F9,
    UI_KEY_F10,
    UI_KEY_F11,
    UI_KEY_F12
} UiKey;

/** @enum UiMouseAction
   @brief Enumeration of possible mouse actions in the UI.
   @ingroup ui_backend
   @details The UiMouseAction enumeration defines the various mouse actions that can be captured in the UI. This includes mouse button presses, releases, drags, and scroll events.
   Each mouse action is represented by a unique value in the enumeration, allowing for easy identification and handling of mouse input. When a mouse event occurs, it is captured in a UiEvent structure, which includes additional information such as the position of the mouse and any modifier keys (Alt, Ctrl, Shift) that were active at the time of the event.
   @see UiEvent
*/
typedef enum {
    UI_MOUSE_NONE = 0,
    UI_MOUSE_PRESS,
    UI_MOUSE_RELEASE,
    UI_MOUSE_DRAG,
    UI_MOUSE_SCROLL_UP,
    UI_MOUSE_SCROLL_DOWN
} UiMouseAction;

/** @enum UiBorderKind
   @brief Enumeration of possible border styles for UI surfaces.
   @ingroup ui_backend
   @details The UiBorderKind enumeration defines the various border styles that can be applied to UI surfaces. This includes no border, ASCII borders, light borders, and rounded borders.
   Each border style is represented by a unique value in the enumeration, allowing for easy selection and application of borders when drawing UI elements. The chosen border style will affect the appearance of the surface's edges when rendered.
   @see ui_draw_border
*/
typedef enum {
    UI_BORDER_NONE = 0,
    UI_BORDER_ASCII,
    UI_BORDER_LIGHT,
    UI_BORDER_ROUNDED
} UiBorderKind;

/** @struct UiColor
   @brief Structure representing a color in the UI.
   @ingroup ui_backend
   @details The UiColor structure represents a color that can be used for foreground and background styling in the UI. It includes RGB values for true color support, as well as an index for palette-based colors.
   The use_rgb flag indicates whether the RGB values should be used (true) or if the index should be used to reference a color from a predefined palette (false). This allows for flexibility in color representation, supporting both modern terminals with true color capabilities and older terminals that rely on a limited color palette.
   @see UiStyle
*/
typedef struct {
    union {
        struct {
            uint8_t a; // alpha    LSB (Little Endian order)
            uint8_t b; // blue
            uint8_t g; // green
            uint8_t r; // red      MSB (Little Endian order)
        };
        struct {
            uint32_t rgba; // 0xRRGGBBAA   red, green, blue, alpha
        };
    };
    bool use_rgb;
    uint32_t index;
} UiColor;

typedef struct {
    UiColor fg;
    UiColor bg;
    uint32_t idx;
} UiColorPair;

/** @struct UiStyle
   @brief Structure representing the style attributes for UI elements.
   @ingroup ui_backend
   @details The UiStyle structure encapsulates the styling attributes that can be applied to UI elements. This includes foreground and background colors, as well as text attributes such as bold, italic, underline, and reverse video.
   The style attributes defined in this structure can be used when drawing text, lines, borders, and other UI components to control their appearance. By configuring the UiStyle appropriately, you can create visually distinct UI elements that enhance the user experience.
   @see UiColor
*/
typedef struct {
    UiColor fg;
    UiColor bg;
    bool bold;
    bool dim;
    bool italic;
    bool underline;
    bool blink;
    bool reverse;
    bool invis;
} UiStyle;

/** @struct UiEvent
   @brief Structure representing an input event in the UI.
   @ingroup ui_backend
   @details The UiEvent structure captures information about an input event that occurs in the UI. This includes key events, mouse events, and resize events.
   The structure contains fields for the key type (UiKey), the Unicode codepoint for character input, modifier keys (Alt, Ctrl, Shift), the position of the event (y, x), and the type of mouse action (UiMouseAction) if applicable. This comprehensive representation allows for detailed handling of user input events within the UI.
   @see UiKey
   @see UiMouseAction
*/
typedef struct {
    UiKey key;
    uint32_t ch; /* Unicode codepoint when key == UI_KEY_CHAR */
    bool alt;
    bool ctrl;
    bool shift;
    int y;
    int x;
    UiMouseAction mouse_action;
} UiEvent;

/** @struct UiRect
   @brief Structure representing a rectangular area in the UI.
   @ingroup ui_backend
   @details The UiRect structure defines a rectangular area within the UI, specified by its top-left corner (y, x) and its dimensions (rows, cols). This structure is commonly used when creating new surfaces or defining areas for drawing operations.
   The y and x fields represent the position of the rectangle's top-left corner relative to its parent surface, while the rows and cols fields specify the height and width of the rectangle, respectively. By using UiRect, you can easily manage and manipulate different areas of the UI for various purposes such as layout management and drawing.
   @see ui_surface_new
*/
typedef struct {
    int y;
    int x;
    int rows;
    int cols;
} UiRect;

/** @struct UiConfig
   @brief Structure representing the configuration options for the UI runtime.
   @ingroup ui_backend
   @details The UiConfig structure encapsulates the configuration options that can be set when initializing the UI runtime. This includes enabling mouse support, using an alternate screen buffer, and controlling cursor visibility.
   The enable_mouse field allows you to specify whether mouse input should be captured and processed by the UI. The enable_alt_screen field determines whether the UI should use an alternate screen buffer, which can help prevent cluttering the main terminal screen. The cursor_visible field controls whether the cursor is visible while the UI is active, which can enhance the user experience in certain applications.
   By configuring these options appropriately, you can tailor the behavior of the UI runtime to suit your application's needs.
   @see ui_init
*/
typedef struct {
    bool enable_mouse;
    bool enable_alt_screen;
    bool cursor_visible;
} UiConfig;

/* @name UI Backend API
   @ingroup ui_backend
   @details The following functions define the API for implementing a backend for the terminal UI library. These functions cover the initialization and shutdown of the UI runtime, management of surfaces, drawing operations, input handling, and cursor control.
   Each function is designed to be implemented by the backend according to the specifications provided in the function comments. By implementing these functions, you can create a functional backend that allows the terminal UI library to render and interact with users effectively.
   Note: ui_init should return NULL on failure and set an appropriate error
   message that can be retrieved by the caller. The error message should provide
   details about the reason for the failure, such as issues with terminal
   capabilities, resource allocation failures, or unsupported features. This
   allows the caller to handle initialization errors gracefully and provide
   feedback to the user.
   @see ui_backend.h
*/
UiRuntime *ui_init(const UiConfig *cfg);
void ui_shutdown(UiRuntime *ui);
void ui_get_screen_size(UiRuntime *ui, int *rows, int *cols);
int ui_render(UiRuntime *ui);
int ui_clear_screen(UiRuntime *ui);
int ui_suspend(UiRuntime *ui);
int ui_resume(UiRuntime *ui);

/* @brief surfaces
   @ingroup ui_backend */
UiSurface *ui_surface_new(UiRuntime *ui, UiSurface *parent, UiRect rect);
void ui_surface_destroy(UiSurface *s);
int ui_surface_move(UiSurface *s, int y, int x);
int ui_surface_resize(UiSurface *s, int rows, int cols);
int ui_surface_clear(UiSurface *s);
int ui_surface_erase(UiSurface *s);
int ui_surface_set_base(UiSurface *s, const UiStyle *style, uint32_t fill_ch);
int ui_surface_set_style(UiSurface *s, const UiStyle *style);

/* @brief drawing
   @ingroup ui_backend */
int ui_draw_text(UiSurface *s, int y, int x, const UiStyle *style, const char *text);
int ui_draw_text_n(UiSurface *s, int y, int x, const UiStyle *style, const char *text, size_t n);
int ui_draw_hline(UiSurface *s, int y, int x, int len, const UiStyle *style);
int ui_draw_vline(UiSurface *s, int y, int x, int len, const UiStyle *style);
int ui_draw_border(UiSurface *s, UiBorderKind kind, const UiStyle *style);
int ui_draw_box_title(UiSurface *s, int x, const UiStyle *style, const char *title);
int ui_bkgrnd(UiSurface *, const UiStyle *, const char *);
int ui_bkgd_set(UiSurface *, const UiStyle *, const char *);
/* @brief clipping / visibility
@ingroup ui_backend */
int ui_surface_show(UiSurface *s);
int ui_surface_hide(UiSurface *s);

/* @brief input
   @ingroup ui_backend */
int ui_get_event(UiRuntime *ui, UiSurface *target, UiEvent *ev, int timeout_ms);

/* @brief cursor
   @ingroup ui_backend */
int ui_cursor_move(UiSurface *s, int y, int x);
int ui_cursor_enable(UiRuntime *ui, bool visible);

extern UiSurface *ui_surface_box[MAXWIN];
extern UiSurface *ui_surface_win[MAXWIN];
extern UiSurface *ui_surface_win2[MAXWIN];

#endif
