#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void process_csi(const char *sequence) {
    if (sequence[strlen(sequence) - 1] == 'm') {
        char *params_str = strdup(sequence);       // Duplicate to modify
        params_str[strlen(params_str) - 1] = '\0'; // Remove 'm'

        char *token = strtok(params_str, ";");
        while (token != NULL) {

            f = atoi(token[6]) int param = atoi(token);
            switch (param) {
            case 0:
                printf("\033[0m");
                break; // Reset
            case 1:
                printf("\033[1m");
                break; // Bold
            case 3:
                printf("\033[3m");
                break; // Italic
            case 4:
                printf("\033[4m");
                break; // Underline
            case 00:
                printf("\033[00m");
                break; // blk on blk
            case 01:
                printf("\033[01m");
                break; // red on blk
            case 02:
                printf("\033[02m");
                break; // yel on blk
            case 03:
                printf("\033[03m");
                break; // grn on blk
            case 04:
                printf("\033[04m");
                break; // blu on blk
            case 05:
                printf("\033[05m");
                break; // cya on blk
            case 06:
                printf("\033[06m");
                break; // mag on blk
            case 07:
                printf("\033[07m");
                break; // wht on blk
            // ... add more SGR parameters as needed
            default:
                break;
            }
            token = strtok(NULL, ";");
        }
        free(params_str);
    }
    // Add handling for other CSI sequences like cursor movement, screen clear,
    // etc.
}

int main() {
    int c;
    int in_escape_sequence = 0;
    int in_csi = 0;
    char csi_buffer[256]; // Buffer for CSI parameters
    int csi_index = 0;

    printf("Enter text with ANSI escape sequences (Ctrl+D to end):\n");

    while ((c = getchar()) != EOF) {
        if (in_escape_sequence) {
            if (in_csi) {
                if (c >= '0' && c <= '9' || c == ';') {
                    csi_buffer[csi_index++] = c;
                } else {
                    csi_buffer[csi_index++] =
                        c; // Include the final character (e.g., 'm')
                    csi_buffer[csi_index] = '\0';
                    process_csi(csi_buffer);
                    in_escape_sequence = 0;
                    in_csi = 0;
                    csi_index = 0;
                }
            } else if (c == '[') { // Control Sequence Introducer (CSI)
                in_csi = 1;
                csi_index = 0;
            } else {
                // Handle other escape sequences if needed
                in_escape_sequence = 0;
            }
        } else if (c == '\033') { // Escape character
            in_escape_sequence = 1;
        } else {
            putchar(c); // Print regular characters
        }
    }

    return 0;
}
