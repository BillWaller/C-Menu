#!/bin/bash
# .bashrc
# shellcheck disable=SC2155
# shellcheck source=/dev/null
# This is a sample .bashrc file for the bash shell.
# It is intended to be a starting point for customizing your
# bash environment. You can copy this file to your home directory
# as .bashrc and modify it to suit your needs.
# WARNING: This file contains commands that are specific to
# certain systems or configurations. Use caution when copying
# and modifying this file. Make sure you understand what each
# command does before using it. Some commands may require
# specific software or settings to work properly.
# This file is provided "as is" without warranty of any kind.
# Use at your own risk.
# The author is not responsible for any damage or loss
# that may result from using this file.
# By using this file, you agree to the terms and conditions
# stated above.
# The purpose of including this file is to provide a source of
# snippets and ideas for customizing your bash environment.
# You may choose to use some or all of the commands in this file,
# or none at all. The choice is yours.
# Enjoy! :-)
# -------------------------------------------------------------------
# GENERAL SETTINGS
# -------------------------------------------------------------------
# set a few environment variables
export BASHRC=1
export LANG=en_US.UTF-8
export LC_ALL=en_US.UTF-8
export LANGUAGE=en_US.UTF-8
export TERM=xterm-256color
export HISTCONTROL=ignoredups:erasedups
export HISTSIZE=10000
export HISTFILESIZE=20000
export HISTTIMEFORMAT="%F %T "
IFS="
"
# Shell Log
unset BASHLOG
unset GH_TOKEN
if [ -f "$HOME"/.shelllog ]; then
	export BASHLOG="1"
fi
this_file="$HOME"/.bashrc
ShellLog() { echo "$(rfc3339)" "$this_file" "$1" >>/var/log/shell.log; }

# PATH
#
# Don't end up with duplicate entries in PATH
# This technique is from bashfaq #24. It's elegant and efficient.

prepend_path() {
	case ":${PATH}:" in
	*:"$1":*) ;;
	*)
		PATH="$1:$PATH"
		;;
	esac
}

# Start with a minimal PATH to avoid inheriting unwanted directories
# from the environment. The prepend_path function is designed to ignore
# duplicate occurrences of the same path.
#
# The order of directories in PATH matters. In this list, precedence
# is in reverse order, ie. last one prepended has highest precedence.
#
# -WARNING- /usr/bin/view, which is generally a link to vim will
# obscure C-Menu View if /usr/bin is prepended after $HOME/menuapp/bin
# Examine your PATH environment variable if you have issues starting
# C-Menu View by typing "view" at the shell prompt.

export PATH=/usr/bin:/bin:/usr/sbin:/sbin
[ -d "/usr/lib/qt6/bin" ] && prepend_path "/usr/lib/qt6/bin"
[ -d "/usr/local/bin" ] && prepend_path "/usr/local/bin"
[ -d "$HOME/.local/bin" ] && prepend_path "$HOME/.local/bin"
[ -d "$HOME/.cargo/bin" ] && prepend_path "$HOME/.cargo/bin"
[ -d "$HOME/menuapp/bin" ] && prepend_path "$HOME/menuapp/bin"
export PATH

# PATH end
#-------------------------------------------------------------------
# BASH SHELL
#-------------------------------------------------------------------

# using xsh, or some other alias reduces visibility and
# vulnerability to some automated attacks and jerkware.
# You could just copy /bin/bash to /bin/xsh. Someone
# suggested a symbolic link, but that would defeat the purpose.
# Ideally, you would compile your own bash from source
# and name it xsh or something else obscure. A static build
# would be even better, but it involves more than just
# compiling bash with --static option. The most reasonable
# solution is to build a set of core libraries including
# libncursesw, tinfo, and readline as static libraries, then
# link bash against those static libraries. The resulting
# binary would be mostly static, except for a few libraries.
# This is left as an exercise for the reader.
#
# Once you have your xsh binary, place it somewhere in your PATH,
# then this line will set your SHELL variable to xsh.
# Leave "export SHELL=BASH" as a fallback. If xsh isn't available,
# you will still have a working shell. Nothing works without a shell.

export SHELL=bash
which xsh >/dev/null 2>&1 && export SHELL=xsh

# BASH SHELL end

#-------------------------------------------------------------------
# RSH
#-------------------------------------------------------------------
# rsh is a small C program that provides a root shell to
# non-privileged users. It is intended for personal systems
# where you are the only user, or where you trust all users.
# It is NOT intended for multi-user systems where security
# is a concern.
# rsh works by being installed as a setuid root binary.
# When a non-privileged user runs rsh, it spawns a root shell.
# To exit the root shell, simply type "exit", press Ctrl-D, or,
# if you have a function like "x" below, just type "x".

# To install rsh, you need to compile it from source.
# You can get the source code from the cmenu project.
# Here are the steps to compile and install rsh:
#
# cd /usr/local/src/cmenu/src
# make
# sudo make install
# ls -l ~/menuapp/bin/rsh
# -rws--x--x. 1 root root 86240 Oct 25 18:48 /home/me/menuapp/bin/rsh
# -------------------------------------------------------------------
# -- or --
# ----------------------------
# su
# cp rsh /usr/local/bin
# chown root:root /usr/local/bin/rsh
# chmod 4755 /usr/local/bin/rsh
# ls /usr/local/bin/rsh
# -rwsr-xr-x. 1 root root 86240 Oct 25 20:00 /usr/local/bin/rsh
# -------------------------------------------------------------------
# Once rsh and the following functions are installed, you can,
# from a standard user shell, type "xx" to start a root shell,
# and "x" to exit that root shell.

xx() {
	if file "$HOME"/menuapp/bin/rsh | grep setuid >/dev/null 2>&1; then
		"$HOME"/menuapp/bin/rsh
	else
		if file /usr/local/bin/rsh | grep setuid >/dev/null 2>&1; then
			/usr/local/bin/rsh
		fi
	fi
}

# Using the following function allows you to exit the root shell
# instantiated by the above function by typing "x".
#
# As an added benefit, it directs the output of exit to /dev/null,
# which suppresses the annoying "exit" message that adds another
# non-relevant line to your terminal.
# This is especially useful when you have nested shells and want
# to exit multiple levels without cluttering the terminal with
# exit messages.

x() { exit >/dev/null 2>&1; }

# for gdb inferior tty
which sleep >/dev/null 2>&1 && s() {
	TTY=$(tty)
	/bin/grep -v "inferior-tty" "$HOME"/.gdbinit >/tmp/.gdbinit.$$ 2>&1
	echo "set inferior-tty $TTY" >>/tmp/.gdbinit.$$
	mv /tmp/.gdbinit.$$ "$HOME"/.gdbinit
	#   clear
	echo "sleeping"
	sleep 50000
}

# old habits die hard - use vi command to start nvim

which nvim >/dev/null 2>&1 && vi() { nvim "$@"; }

# grep

which grep >/dev/null 2>&1 && grep() { /usr/bin/grep -Hn "$@"; }

# typing mm takes less time than menu if you use it often

which menu >/dev/null 2>&1 && mm() {
	menu "$@"
}

# colorize ls output if possible
# lsd is a modern replacement for ls with more features and better defaults
# set PREFER_LSD to 1 to prefer lsd over ls
# set PREFER_LSD to 0 to prefer ls over lsd
PREFER_LSD=1
if [ "$PREFER_LSD" = "1" ]; then
	which lsd >/dev/null 2>&1 && ls() { /usr/bin/lsd "$@"; }
else
	which ls >/dev/null 2>&1 && ls() { /bin/ls --color=auto "$@"; }
fi

# gdb alias to always start in silent mode

which gdb >/dev/null 2>&1 && gdb() { /usr/bin/gdb --silent "$@"; }

# persistent project directory
# kk - change to cmenu source directory
# useful for development and debugging
# This is stupidly simple, but it works for me.
kk() { cd /usr/local/src/cmenu/src || return; }

# -------------------------------------------------------------------
# CURSES
# -------------------------------------------------------------------
# shorter delay for curses escape sequences
# This may cause problems over a slow connection.
# Adjust the value as needed. The default is 1000 milliseconds (1 second).
export ESCDELAY=50
# CURSES end

# -------------------------------------------------------------------
# SHELL PROMPT
# -------------------------------------------------------------------
export PS1="\[\e[1;32m\]\u@\h(\l)\w->\[\e[0m\] "
export XUSER="$(id -un)"
[ "$XUSER" = "root" ] && export PS1="\[\e[1;31m\]\u@\h(\l)\w->\[\e[0m\] "
# SHELL PROMPT end

# -------------------------------------------------------------------
# MANUAL PAGES
# -------------------------------------------------------------------
# If you find your manual pages boring, consider using nvim
# as your man pager. It offers syntax highlighting and a more
# pleasant reading experience. It won't interfere with the Man
# function below, which uses C-Menu View.
export MANPAGER="nvim +Man!"
export MANWIDTH=200
# The Man function below pipes the output of man through a sed script
# that reformats the manual pages for better readability in C-Menu's View.
# It then opens the reformatted output in C-Menu View.
which man >/dev/null 2>&1 && Man() { man -Tutf8 "$@" | sed -f ~/menuapp/msrc/man.sed | view; }
# MANUAL PAGES end

export ESCDELAY=50
export PAGER=$(which view 2>/dev/null)
export MANPAGER="nvim +Man!"
export MANWIDTH=80
export COB_CONFIG_DIR=/usr/share/gnucobol/config
export LPDEST="$HOME"/prtout
export QT_STYLE_OVERRIDE=gtk
export PKG_CONFIG_PATH=/usr/lib64/pkgconfig:/usr/lib/pkgconfig:/usr/share/pkgconfig:/usr/local/lib/pkgconfig:/usr/local/lib64/pkgconfig
export JAVA_HOME=/usr/lib64/jvm/java-21-openjdk-21
export NODE_PATH="$HOME"/node_modules
export python3_host_prog=/usr/bin/python3
export XDG_CONFIG_HOME="$HOME"/.config
export XDG_DATA_HOME="$HOME"/.local/share
export CARGO_HOME="$HOME"/.cargo
export RUSTUP_HOME="$HOME"/.rustup
export LLVM_LIB_DIR=/usr/lib64
export MASON="$HOME/.local/share/lazyvim/mason"
export QML_IMPORT_PATH=/usr/lib64/qt6/qml
export CMENU_SRC=/usr/local/src/cmenu/src
export CMENU_HOME="$HOME"/menuapp
export TTYPATH=/dev/pts
export EDITOR=nvim
shopt -s histappend
PROMPT_COMMAND="history -a; history -r"
# export RA_LOG_FILE="$HOME"/.local/state/"$NVIM_APPNAME"/ra.log
# export RA_LOG=info
clear() {
	tput clear
}
stty ixany
. "$HOME"/.nvim_appname
# .nvim_appname sets NVIM_APPNAME variable as follows
# export NVIM_APPNAME=lazyvim
# This makes nvim use $HOME/.config/lazyvim for its config files
# and $HOME/.local/state/lazyvim for its state files
# -------------------------------------------------------------------
set -o vi
if [ "$BASHLOG" ]; then
	if [ ! -w /var/log/shell.log ]; then
		echo "error: /var/log/shell.log is not writable by $USER"
	else
		ShellLog "--------------------------------"
		ShellLog "TERM=$TERM"
		ShellLog "USER=$USER"
		ShellLog "XUSER=$XUSER"
		ShellLog "LOGNAME=$LOGNAME"
		ShellLog "HOME=$HOME"
		ShellLog "XPWD=$XPWD"
	fi
fi
. "$HOME/.cargo/env"
