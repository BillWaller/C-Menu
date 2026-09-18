#define SUB_SURFACE_LIST(X) \
    X(BOX)                  \
    X(WIN)                  \
    X(WIN2)                 \
    X(LNNO)                 \
    X(CMDLN)                \
    X(PAD)                  \
    X(WIN3)
#define AS_ENUM(NAME) NAME,
typedef enum {
    SUB_SURFACE_LIST(AS_ENUM)
        SUB_SFC_MAX
} ss_t;

// ss_t defined above, it's an enum

typedef struct {
    UiKey key;
    uint32_t ch; /* Unicode codepoint when key == UIKEY_CHAR */
    bool alt;
    bool ctrl;
    bool shift;
    uint y;
    uint x;
    bool active;
    int chyron;
    ss_t in_win;
    int bstate;
    UiMouseAction mouse_action;
    bool mouse_inside;
    char keybound[16];
} UiEvent;

typedef struct {
    UiChyronKey *key[CHYRON_KEYS]; /**< array of key bindings for the chyron */
    char s[MAXLEN];                /**< the chyron string, for displaying messages in */
    UiCell cmplx_buf[MAXLEN];      /**< the chyron wide character string */
    uint l;                        /**< length of the chyron string, for display */
    struct UiSurface *sfc;         /** pointer to surface for the chyron */
    ss_t w;                        /** index to window of surface */
    uint y;                        /** y coordinante of the chyron in the window */
} UiChyron;

This statement returned false, even though both values are 2,

    if (ev->in_win == chyron->w){}

    (gdb)p ev->in_win
    $1 = 2

         (gdb)p chyron->w
         $2 = WIN2

              why do not `ev->in_win` and `chyron->w` have the same value even though they both seem to refer to the same window(WIN2)
