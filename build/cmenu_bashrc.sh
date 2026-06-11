if ! grep "# CMENU BASHRC" "$HOME"/.bashrc >/dev/null 2>&1; then
    sed '1,4d' cmenu_bashrc.sh >>"$HOME"/.bashrc
fi
exit 0
# CMENU BASHRC
which rsh >/dev/null 2>&1 && xx() { rsh "$@"; }
x() { exit >/dev/null 2>&1; }
export PS1="\[\e[1;32m\]\u@\h:\w>\[\e[0m\] "
export XUSER="$(id -un)"
[ "$XUSER" = "root" ] && export PS1="\[\e[1;31m\]\u@\h:\w>\[\e[0m\] "
