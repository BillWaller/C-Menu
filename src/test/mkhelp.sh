name="lf"
lf --version >"$name"_help
lf -? >>"$name"_help
lf --usage >>"$name"_help

bat --theme ansi -l Crystal -f "$name"_help >"$name"-help

lf -pxwrsg -u bill -L -H -S -t f -a 2026-04-01T00:00:00 -b 2026-05-01T00:00:00 -D18 -e '.*\.[ch]$' 2>"$name"_debug
bat --theme ansi -l Crystal -f "$name"_debug >"$name"-debug
