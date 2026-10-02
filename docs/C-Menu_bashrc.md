# C-Menu_bashrc

This bash script contains settings that make using C-Menu more convenient. Add it to or source it from $HOME/.bashrc

Make a back up copy of $HOME/.bashrc before adding this script.

```bash
# ~/.bashrc

export CMENU_SRC=/usr/local/src/C-Menu/src
export CMENU_HOME="$HOME"/menuapp
export CMENU_RC="$HOME"/menuapp/.minitrc

# To enable your system to find the C-Menu binaries, prepend the
# C-Menu bin directory to the PATH environment variable.
# The following prepend_ functions avoid prepending paths more than once.

prepend_path() {
    case ":${PATH}:" in
    *:"$1":*) ;;
    *)
        PATH="$1:$PATH"
        ;;
    esac
}
#
prepend_path "$HOME"/menuapp/bin; do
export PATH

# Do the same for the MANPATH environment variable.

prepend_man_path() {
    case ":${MANPATH}:" in
    *:"$1":*) ;;
    *)
        MANPATH="$1:$MANPATH"
        ;;
    esac
}
prepend_man_path "$CMENU_HOME"/man
export MANPATH

# To start C-Menu by typing "mm"
which menu >/dev/null 2>&1 && mm() {
    menu "$@"
}

# To enter a root shell by typing "xx"
xx() {
    file "$HOME"/menuapp/bin/rsh | grep setuid >/dev/null 2>&1
    if [ "$?" = "0" ]; then
        "$HOME"/menuapp/bin/rsh
    else
        file /usr/local/bin/rsh | grep setuid >/dev/null 2>&1
        if [ "$?" = "0" ]; then
            /usr/local/bin/rsh
        else
            echo "Error: rsh not found or not setuid. Contact System
            Administrator."
        fi
    fi
}

# To exit a root shell by typing "x"
x() { exit >/dev/null 2>&1; }

# Standard shell prompt
export PS1="\[\e[1;32m\]\u@\h(\l)\W▶\[\e[0m\]"

# Red shell prompt to indicate root authority
export XUSER="$(id -un)"
[ "$XUSER" = "root" ] && export PS1="\[\e[1;31m\]\u@\h(\l)\W▶\[\e[0m\]"
```
