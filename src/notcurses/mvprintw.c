static int
compat_mvwprintw(struct ncplane *nc, int y, int x, const char *fmt, ...) {
    va_list va;
    va_start(va, fmt);
    if (ncplane_vprintf_yx(nc, y, x, fmt, va) < 0) {
        va_end(va);
        return ERR;
    }
    va_end(va);
    return OK;
}
