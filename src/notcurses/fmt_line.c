#define _XOPEN_SOURCE 600
#include <notcurses/notcurses.h>
#include <stdlib.h>
#include <string.h>

// Note: cmplx_buf (cchar_t*) is replaced by a plain UTF-8 string buffer or direct plane drawing.
// For a direct replacement, we draw straight onto an isolated virtual plane, then copy it.
int fmt_line(View *view, struct ncplane *cp) {
    int i = 0, j = 0;
    char *in_str = view->line_in_s;
    rtrim(view->line_out_s);

    // Track state styles/channels across chunks if manually splitting strings
    uint64_t current_channels = CC_NT_CHANNELS;
    uint32_t current_styles = NCSTYLE_NONE;

    // Clear the plane row line where this text is going
    ncplane_cursor_move_yx(cp, 0, 0);

    while (in_str[i] != '\0') {
        // 1. Let Notcurses natively swallow inline ANSI tokens
        if (in_str[i] == '\033' && in_str[i + 1] == '[') {
            size_t len = strcspn(&in_str[i], "mK ") + 1;

            // Extract the sequence block
            char ansi_tok[MAXLEN];
            memcpy(ansi_tok, &in_str[i], len);
            ansi_tok[len] = '\0';

            if (ansi_tok[len - 1] == 'K') {
                // Handle clear-to-end line instruction natively
                ncplane_erase_region(cp, -1, -1, 1, -1);
            } else {
                // Feed the ANSI token into a Notcurses state block or write it out.
                // Notcurses processes formatting sequences inline instantly!
                int parsed_bytes = 0;
                ncplane_putstr_gcb(cp, &current_styles, &current_channels, ansi_tok, &parsed_bytes);
            }
            i += len;
        }
        // 2. Handle Custom Tab Stops
        else if (in_str[i] == '\t') {
            do {
                // Output a standard space using current active channels/styles
                ncplane_putegc(cp, " ", NULL);
                view->stripped_line_out[j++] = ' ';
            } while ((j < PAD_COLS - 2) && (j % view->tab_stop != 0));
            i++;
        }
        // 3. Handle Standard UTF-8 Text Chunks
        else {
            if (in_str[i] == '\033') {
                i++;
                continue;
            }

            // Find how far the plain string text goes before hitting a tab or escape sequence
            size_t text_span = strcspn(&in_str[i], "\t\033");
            if (text_span > 0) {
                char tmp_chunk[MAXLEN];
                memcpy(tmp_chunk, &in_str[i], text_span);
                tmp_chunk[text_span] = '\0';

                // Put the raw UTF-8 string chunk with implicit inline styling evaluation
                int bytes_written = 0;
                ncplane_putstr_gcb(cp, &current_styles, &current_channels, tmp_chunk, &bytes_written);

                // Replicate your stripped tracing buffer properties
                for (size_t k = 0; k < text_span && (j < PAD_COLS - 1); k++) {
                    view->stripped_line_out[j++] = in_str[i + k];
                }
                i += text_span;
            }
        }
    }

    if (j > view->maxcol) {
        view->maxcol = j;
    }
    view->stripped_line_out[j] = '\0';

    // Returns the calculated visual screen cursor columns
    int current_y, current_x;
    ncplane_cursor_yx(cp, &current_y, &current_x);
    return current_x;
}
