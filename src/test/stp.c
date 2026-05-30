#define _GNU_SOURCE

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    char *p;
    char *buf1;
    size_t len, size;

    size = strlen("Hello ") + strlen("world") + strlen("!") + 1;
    buf1 = malloc(sizeof(*buf1) * size);
    if (buf1 == NULL)
        err(EXIT_FAILURE, "malloc()");

    p = buf1;
    p = stpcpy(p, "Hello ");
    p = stpcpy(p, "world");
    p = stpcpy(p, "!");
    len = p - buf1;

    char *buf2 = "Now is the time for all good ment to come to the aid of their country.";
    char *d = malloc(sizeof(*d) * (strlen(buf2) + 1));

    p = buf2;
    while (*p != '\0') {
        p = stpcpy(d, p);
        d += strlen(d);
        p += strlen(p);
    }

    printf("[len = %zu]: ", len);
    puts(buf1); // "Hello world!"
    free(buf1);

    exit(EXIT_SUCCESS);
}
