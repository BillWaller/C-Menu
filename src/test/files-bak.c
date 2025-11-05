#include "/usr/local/src/cmenu/src/menu.h"
#include <libgen.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define MAXLEN 256
#define TRUE 1
#define FALSE 0

bool test_trim_path(char *input, char *expect, char *got) {
    bool f;
    bool t = FALSE;

    strcpy(got, input);
    f = trim_path(got);
    if (f)
        t = strcmp(expect, got) ? FALSE : TRUE;
    else
        t = (expect == NULL) ? TRUE : FALSE;

    fprintf(stderr, "trim_path(%s) return(%s), %s\n", input,
            f ? "SUCCESS" : "FAILURE", t ? "PASS" : "FAIL");
    fprintf(stderr, "expected: %s\n", expect);
    fprintf(stderr, "     got: %s\n\n", got);
    return (t);
}

bool test_expand_tilde(char *input, char *expect, char *got) {
    bool f;
    bool t = FALSE;

    strcpy(got, input);
    f = expand_tilde(got, MAXLEN);
    if (f)
        t = strcmp(expect, got) ? FALSE : TRUE;
    else
        t = (expect == NULL) ? TRUE : FALSE;

    fprintf(stderr, "expand_tilde(%s) return(%s), %s\n", input,
            f ? "SUCCESS" : "FAILURE", t ? "PASS" : "FAIL");
    fprintf(stderr, "expected: %s\n", expect);
    fprintf(stderr, "     got: %s\n\n", got);
    return (t);
}

bool test_trim_ext(char *input, char *expect, char *got) {
    bool f;
    bool t = FALSE;

    f = trim_ext(got, input);
    if (f)
        t = strcmp(got, expect) ? FALSE : TRUE;
    else
        t = (expect == NULL) ? TRUE : FALSE;

    fprintf(stderr, "trim_ext(%s) return(%s), %s\n", input,
            f ? "SUCCESS" : "FAILURE", t ? "PASS" : "FAIL");
    fprintf(stderr, "expected: %s\n", expect);
    fprintf(stderr, "     got: %s\n\n", got);
    return (t);
}

bool test_dir_name(char *input, char *expect, char *got) {
    bool f;
    bool t = FALSE;

    f = dir_name(got, input);
    if (f)
        t = strcmp(expect, got) ? FALSE : TRUE;
    else
        t = (expect == NULL) ? TRUE : FALSE;

    fprintf(stderr, "dir_name(%s) return(%s), %s\n", input,
            f ? "SUCCESS" : "FAILURE", t ? "PASS" : "FAIL");
    fprintf(stderr, "expected: %s\n", expect);
    fprintf(stderr, "     got: %s\n\n", got);
    return (t);
}

bool test_base_name(char *input, char *expect, char *got) {
    bool f;
    bool t = FALSE;

    f = base_name(got, input);
    if (f)
        t = strcmp(expect, got) ? FALSE : TRUE;
    else
        t = (expect == NULL) ? TRUE : FALSE;

    fprintf(stderr, "base_name(%s) return(%s), %s\n", input,
            f ? "SUCCESS" : "FAILURE", t ? "PASS" : "FAIL");
    fprintf(stderr, "expected: %s\n", expect);
    fprintf(stderr, "     got: %s\n\n", got);
    return (t);
}

int main() {
    char dir[MAXLEN];
    char base[MAXLEN];
    char qfile_spec[MAXLEN];
    bool f_base_name, f_dir_name;
    char path[MAXLEN];
    char buf[MAXLEN];

    test_trim_path("menu", "menu", buf);
    test_trim_path("/home/bill/", "/home/bill", buf);
    test_trim_path("~//", "~/", buf);
    test_trim_path("//", "/", buf);
    test_trim_path("~/", "~/", buf);
    test_trim_path("/", "/", buf);
    test_trim_path("~/menuapp/bin/view", "~/menuapp/bin/view", buf);

    test_expand_tilde("~/.minitrc", "/home/bill/.minitrc", buf);
    test_expand_tilde("~/menuapp", "/home/bill/menuapp", buf);
    test_expand_tilde("~/menuapp/msrc", "/home/bill/menuapp/msrc", buf);
    test_expand_tilde("~/menuapp/bin/view", "/home/bill/menuapp/bin/view", buf);

    test_trim_ext("file.c", "file", buf);
    test_trim_ext("archive.tar.gz", "archive.tar", buf);
    test_trim_ext("no_extension", NULL, buf);
    test_trim_ext(".hiddenfile", NULL, buf);
    test_trim_ext("multiple.dots.in.name.txt", "multiple.dots.in.name", buf);

    test_dir_name("/home/bill/menuapp/msrc", "/home/bill/menuapp", buf);
    test_trim_path("/", "/", buf);
    test_trim_path("/home/bill/", "/home/bill", buf);
    test_trim_path("~/menuapp/bin/view", "~/menuapp/bin/view", buf);

    test_expand_tilde("~/.minitrc", "/home/bill/.minitrc", buf);
    test_expand_tilde("~/menuapp", "/home/bill/menuapp", buf);
    test_expand_tilde("~/menuapp/msrc", "/home/bill/menuapp/msrc", buf);
    test_expand_tilde("~/menuapp/bin/view", "/home/bill/menuapp/bin/view", buf);

    test_dir_name("/home/bill/menuapp/msrc", "/home/bill/menuapp", buf);
    test_dir_name("/", "/", buf);
    test_dir_name("/home/bill/menuapp/msrc/file.c", "/home/bill/menuapp/msrc",
                  buf);
    test_dir_name("/home/bill/menuapp/msrc///", "/home/bill/menuapp/msrc", buf);

    test_base_name("/home/bill/menuapp/msrc/file.c", "file.c", buf);
    test_base_name("/home/bill/menuapp/msrc/", NULL, buf);
    test_base_name("/home/bill/menuapp/msrc///", NULL, buf);
    test_base_name("/home/bill/menuapp/msrc", "msrc", buf);
    return 0;
}
