        char user_str[100];
        char ip_str[MAXLEN];
        char tmp_str[1024];
        bool addspace_before = false;
        addspace_before = false;
        ssnprintf(tmp_str, 1023, "%s,%s,%s,", get_local_timestamp(), get_user_str(user_str, 100), get_ip_addresses(ip_str, MAXLEN));
        for (i = 0; i < argc; i++) {
            if (addspace_before == true)
                strnz__cat(tmp_str, " ", 1023);
            strnz__cat(tmp_str, argv[i], 1023);
            addspace_before = true;
        }

        fprintf(stderr, "%s,%s,%s,", get_local_timestamp(), get_user_str(user_str, 100), get_ip_addresses(ip_str, MAXLEN));
