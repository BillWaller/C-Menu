# ╭─────────────────────────────────────────────────────────────────╮
# │ RSH - QUICK ROOT                                                │
# ╰─────────────────────────────────────────────────────────────────╯
# The following functions in your .bashrc will allow you to quickly
# assume a root shell by typing "xx" and exit it by typing "x".
# The red prompt indicates you are in a root shell, and reminds
# you to be cautious while operating with elevated privileges.
# Don't loiter. Get in, do what you need to do, and get out.
# ───────────────────────────────────────────────────────────────────
xx() {
	which rsh >/dev/null 2>&1 && { rsh "$@"; }
}
x() { exit >/dev/null 2>&1; }
# ───────────────────────────────────────────────────────────────────
export PS1="\[\e[1;32m\]\u@\h:\w>\[\e[0m\] "
export XUSER="$(id -un)"
[ "$XUSER" = "root" ] && export PS1="\[\e[1;31m\]\u@\h:\w>\[\e[0m\] "
# ───────────────────────────────────────────────────────────────────
# ╭─────────────────────────────────────────────────────────────────╮
# │ END RSH - QUICK ROOT                                            │
# ╰─────────────────────────────────────────────────────────────────╯
