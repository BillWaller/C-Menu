#include <syslog.h>

#define MAXLEN 1024

void syslog_info(char *s);

// journalctl -t C-Menu
int main(int argc, char **argv) {
    // openlog(ident, option, facility)
    // ident: String prepended to every message (usually program name)
    // option: Flags like LOG_PID (include process ID) or LOG_CONS (log to console on error)
    // facility: Type of program (e.g., LOG_USER, LOG_LOCAL0)

    char user_str[100];
    char ip_str[MAXLEN];
    char tmp_str[1024];
    bool addspace_before = false;
    addspace_before = false;
    int i;

    snprintf(tmp_str, 1023, "%s,%s,%s,", get_local_timestamp(), get_user_str(user_str, 100), get_ip_addresses(ip_str, MAXLEN));
    for (i = 0; i < argc; i++) {
        if (addspace_before == true)
            strnz__cat(tmp_str, " ", 1023);
        strnz__cat(tmp_str, argv[i], 1023);
        addspace_before = true;
    }

    fprintf(stderr, "%s,%s,%s,", get_local_timestamp(), get_user_str(user_str, 100), get_ip_addresses(ip_str, MAXLEN));
    syslog_info("Now is the time");

    return 0;
}
void syslog_info(char *s) {
    openlog("C-Menu", LOG_PID | LOG_CONS, LOG_USER | LOG_INFO);
    syslog(LOG_INFO | LOG_USER, "%s", s);
    closelog();
}
