#!/usr/bin/awk -f
# helgrind.awk
# Bill Waller 2026
# an AWK script to reformat helgrind output

BEGIN {
    c_context = 0
    t_errors = 0
    rc = getline
    while (rc) {
        if ($1 != "read" && $1 != "write") {
            break
        }
        a_verb = $1
        a_size = $2
        a_addr = $3
        a_thrd = $4
        rc = getline
        if (rc == 0) {
            break
        }
        a_by_addr = $1
        a_fn   = $2
        a_ln = $3
        rc = getline
        if (rc != 0 && $1 != "read" && $1 != "write") {
            break
        }
        b_verb = $1
        b_size = $2
        b_addr = a_addr
        b_thrd = $3
        rc = getline
        if (rc == 0) {
            break
        }
        b_by_addr = $1
        b_fn = $2 
        b_ln = $3
        c_context += 1
        c_errors = ""
        rc = getline
        if (rc != 0) {
            if ($2 != "errors") {
                continue
            }
            c_errors = $1
            c_context = $5
            if (c_errors != "") {
                printf("%5s %5s\n", a_ln, b_ln)
                printf("%5s %5s\n", b_ln, a_ln)
            }
        }
        rc = getline
    }
}
