        if (ts[0] != '#') {
            sp = ts;
            dp = tmp_str;
            while (*sp != '\0') {
                if (*sp == '\n')
                    *dp = *sp = '\0';
                else {
                    if (*sp != '"' && *sp != ' ' && *sp != ';')
                        *dp++ = *sp;
                    sp++;
                }
            }
