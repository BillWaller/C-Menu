#include <stdio.h>
#include <stdlib.h> // for EXIT_SUCCESS/FAILURE
#include <time.h>

int main(void) {
    char buf[100];
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    size_t len = strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S:%z", t);

    if (len > 0) {
        printf("ISO 8601 time: %s\n", buf);
        return EXIT_SUCCESS;
    } else {
        fprintf(stderr, "strftime failed or buffer too small\n");
        return EXIT_FAILURE;
    }
}
