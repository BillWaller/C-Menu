/** mkcc
    @brief Create a cchar_t with the specified color pair index and attributes
    @ingroup color_management
    @param cp Color pair index
    @param attr Attributes to apply to the cchar_t
    @param s Multibyte string to convert to wide character (only the first
   character is used)
    @return cchar_t with the specified color pair index and a space character
    as the wide character */

cchar_t mkcc(short cp, attr_t attr, const char *s) {
    mbstate_t mbstate;
    memset(&mbstate, 0, sizeof(mbstate));
    size_t len;
    size_t n;
    cchar_t cc = {0};
    wchar_t wstr[2] = {L'\0', L'\0'};
    len = strlen(s);
    if (len > 0) {
        n = mbrtowc(wstr, s, MB_CUR_MAX, &mbstate);
        if (n <= 0) {
            wstr[0] = L'?';
            wstr[1] = L'\0';
        } else {
            wstr[1] = L'\0';
        }
    } else {
        wstr[0] = L' ';
        wstr[1] = L'\0';
    }
    ui_setcchar(&cc, wstr, attr, cp, nullptr);
    return cc;
}
// #endif
