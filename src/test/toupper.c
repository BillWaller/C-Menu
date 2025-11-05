#include <string.h>
#include <unistd.h>

int main(int argc, char **argv) {
    if (argc < 2)
        return 1;
    for (char *p = argv[1]; *p; p++) {
        if (*p >= 'a' && *p <= 'z') {
            *p -= ('a' - 'A');
        }
    }
    write(1, argv[1], strlen(argv[1]));
    return 0;
}
