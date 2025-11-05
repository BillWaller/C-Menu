# Customize 

## PATH

Don't end up with duplicate entries in PATH

- This technique is from bashfaq #24. It's elegant and efficient.

```bash
prepend_path() {
    case ":${PATH}:" in
    *:"$1":*) ;;
    *)
        PATH="$1:$PATH"
        ;;
    esac
}
export PATH=/usr/bin:/bin:/usr/sbin:/sbin
[ -d "/usr/lib/qt6/bin" ] && prepend_path "/usr/lib/qt6/bin"
[ -d "/usr/local/bin" ] && prepend_path "/usr/local/bin"
[ -d "$HOME/.local/bin" ] && prepend_path "$HOME/.local/bin"
[ -d "$HOME/.cargo/bin" ] && prepend_path "$HOME/.cargo/bin"
[ -d "$HOME/menuapp/bin" ] && prepend_path "$HOME/menuapp/bin"
export PATH
```

## BASH SHELL 🎯

Using xsh, or some other alias reduces visibility and
vulnerability to some automated attacks and jerkware.
You could just copy /bin/bash to /bin/xsh. Someone
suggested a symbolic link, but that would defeat the purpose.
Ideally, you would compile your own bash from source
and name it xsh or something else obscure. A static build
would be even better, but it involves more than just
compiling bash with --static option. The most reasonable
solution is to build a set of core libraries including
libncursesw, tinfo, and readline as static libraries, then
link bash against those static libraries. The resulting
binary would be mostly static, except for a few libraries.
This is left as an exercise for the reader.

Once you have your xsh binary, place it somewhere in your PATH,
then this line will set your SHELL variable to xsh.

- IMPORTANT: Leave "export SHELL=BASH" as a fallback.

If xsh isn't available, you will still have a working shell,
and *NOTHING* works without a shell.

```bash
export SHELL=bash
which xsh >/dev/null 2>&1 && export SHELL=xsh
````

## RSH

rsh is a small C program that provides a root shell to
non-privileged users. It is intended for personal systems
where you are the only user, or where you trust all users.
It is NOT intended for multi-user systems where security
is a concern.

rsh works by being installed as a setuid root binary.
When a non-privileged user runs rsh, it spawns a root shell.
To exit the root shell, simply type "exit", press Ctrl-D, or,
if you have a function like "x" below, just type "x".

To install rsh, you need to compile it from source.
You can get the source code from the cmenu project.
Here are the steps to compile and install rsh:

```bash
cd /usr/local/src/cmenu/src
$ make
$ sudo make install
$ ls -l ~/menuapp/bin/rsh
$ -rws--x--x. 1 root root 86240 Oct 25 18:48 /home/me/menuapp/bin/rsh
```

### -- or --

```bash
$ su -
# cp rsh /usr/local/bin
# chown root:root /usr/local/bin/rsh
# chmod 4755 /usr/local/bin/rsh
# ls /usr/local/bin/rsh
# -rwsr-xr-x. 1 root root 86240 Oct 25 20:00 /usr/local/bin/rsh
```

Once rsh and the following functions are installed, you can,
from a standard user shell, type "xx" to start a root shell,
and "x" to exit that root shell.

```bash
which rsh >/dev/null 2>&1 && xx() { rsh; }
```

Using the following function allows you to exit the root shell
instantiated by the above function by typing "x".

As an added benefit, it directs the output of exit to /dev/null,
which suppresses the annoying "exit" message that adds another
non-relevant line to your terminal.

This is especially useful when you have nested shells and want
to exit multiple levels without cluttering the terminal with
exit messages.

```bash
x() { exit >/dev/null 2>&1; }
```


- Old habits die hard - use vi command to start nvim

```bash
which nvim >/dev/null 2>&1 && vi() { nvim "$@"; }
```

- typing mm takes less time than menu if you use it often

```bash
which menu >/dev/null 2>&1 && mm() { menu "$@"; }
```

- colorize ls output if possible

```bash
which ls >/dev/null 2>&1 && ls() { /bin/ls --color=auto "$@"; }
```

- gdb alias to always start in silent mode

```bash
which gdb >/dev/null 2>&1 && gdb() { /usr/bin/gdb --silent "$@"; }
```

- persistent project directory

```bash
pp() { cd /usr/local/src/cmenu/src || return; }
```

- lazyvim config directory

```bash
ll() { cd ~/.config/lazyvim/lua || return; }
```

## CURSES

- shorter delay for curses escape sequences


```bash
export ESCDELAY=50
```

## SHELL PROMPT

Before your co-workers notice you repetitively typing 'whoami', Install
this shell prompt. It's green when you are yourself, but turns red
when you go superuser. It's very low-key, as it should be, but relevant.
Change the colors and information to suit your taste.

```bash
export PS1="\[\e[1;32m\]\u@\h:\w>\[\e[0m\] "
export XUSER="$(id -un)"
[ "$XUSER" = "root" ] && export PS1="\[\e[1;31m\]\u@\h:\w>\[\e[0m\] "
```

## MANUAL PAGES

If you find your manual pages looking dreary and uninviting,
consider using nvim as your man pager. It offers syntax highlighting
and a more pleasant reading experience.

```bash
export MANPAGER="nvim +Man!"
export MANWIDTH=80
```


Live Long and Prosper! 🖖
