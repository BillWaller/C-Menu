#pragma once

#include <stddef.h>
#include <stdint.h>

// Unique identifiers for every help node context
typedef uint32_t cmenu_help_id_t;

#define CMENU_HELP_NONE 0

typedef struct {
    cmenu_help_id_t id;
    const char *context_key; // For human-readable logging / future localization
    const char *title;
    const char *body;
} cmenu_help_node_t;

// Help Engine Core Lifecycle APIs
void cmenu_help_init(const cmenu_help_node_t *catalog, size_t count);
const cmenu_help_node_t *cmenu_help_lookup(cmenu_help_id_t id);
void cmenu_help_free(void);

// Framework UI Abstract Interfaces
typedef struct {
    void (*show_popup)(const char *title, const char *body);
    void (*clear_hover_help)(void);
} cmenu_ui_backend_t;

// Context dispatchers inside the main loop
void cmenu_trigger_f1_help(cmenu_ui_backend_t *ui, cmenu_help_id_t id);
void cmenu_trigger_hover_help(cmenu_ui_backend_t *ui, cmenu_help_id_t id);
