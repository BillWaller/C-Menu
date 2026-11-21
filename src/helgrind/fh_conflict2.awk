#!/usr/bin/awk -f
# helgrind.awk
# Bill Waller 2026
# an AWK script to reformat helgrind output

BEGIN {
    printf("Conflicts\n")
    printf("-------------------------------------------------")
    p_ln_no = 0
    rc = getline
    while (rc) {
        if ($0 == "") {
            break
        }
        ln_no = $1
        c_ln_no = $2
        if (ln_no == p_ln_no) {
            printf("%6s", c_ln_no)
        }
        else {
            printf("\n%6s", c_ln_no)
        }
        p_ln_no = ln_no
        rc = getline
    }
    if (p_ln_no != 0) {
        if (ln_no == p_ln_no) {
            printf("%6s", c_ln_no)
        }
        else {
            printf("\n%6s", c_ln_no)
        }
    }
}
