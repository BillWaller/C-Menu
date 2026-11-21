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
        b_addr = ""
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
                printf("%05s %6s %-6s %4s %8s %7s %-30s %3s %5s\n", c_context, c_errors, a_verb, a_size, a_addr, a_by_addr, a_fn, a_thrd, a_ln)
                printf("%05s %6s %-6s %4s %8s %7s %-30s %3s %5s\n", c_context, "", b_verb, "", "", b_by_addr, b_fn, b_thrd, b_ln)
                t_errors
                t_errors+= c_errors
            }
        }
        rc = getline
    }
    printf("      ------\n");
    printf("      %6s total errors\n", t_errors);
}
