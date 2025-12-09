# CMENU BUILD INSTRUCTIONS FOR MAKE

This document provides instructions on how to build the CMENU project using the `make` build system.

## Prerequisites

Before you begin, ensure that you have the following installed on your system:

- A C/C++ compiler (e.g., GCC, Clang)
- Make utility
- Git (to clone the repository)
- NCURSES 6.5 or later with wide character support
- Development libraries for NCURSES
  libncursesw6.5.so and /usr/include/ncursesw

1. From the C-Menu "src" directory, type "make" to build.
2. To install, "sudo make install".
3. To clean, type "make clean".
4. To uninstall, "sudo make uninstall".

If you type "make >make.out 2>&1", all output will be saved in "make.out" for later review.

make >make.out 2>&1 example:

```
gcc -O0 -g3 -std=gnu23 -fdiagnostics-color=always -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wno-unused-function -DNCURSES_WIDECHAR -Wl,--gc-sections -c menu.c
gcc -O0 -g3 -std=gnu23 -fdiagnostics-color=always -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wno-unused-function -DNCURSES_WIDECHAR -Wl,--gc-sections -c menu_engine.c
gcc -O0 -g3 -std=gnu23 -fdiagnostics-color=always -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wno-unused-function -DNCURSES_WIDECHAR -Wl,--gc-sections -c parse_menu_desc.c
gcc -O0 -g3 -std=gnu23 -fdiagnostics-color=always -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wno-unused-function -DNCURSES_WIDECHAR -Wl,--gc-sections -c init_view.c
gcc -O0 -g3 -std=gnu23 -fdiagnostics-color=always -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wno-unused-function -DNCURSES_WIDECHAR -Wl,--gc-sections -c form_engine.c
gcc -O0 -g3 -std=gnu23 -fdiagnostics-color=always -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wno-unused-function -DNCURSES_WIDECHAR -Wl,--gc-sections -c fields.c
gcc -O0 -g3 -std=gnu23 -fdiagnostics-color=always -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wno-unused-function -DNCURSES_WIDECHAR -Wl,--gc-sections -c pick_engine.c
gcc -O0 -g3 -std=gnu23 -fdiagnostics-color=always -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wno-unused-function -DNCURSES_WIDECHAR -Wl,--gc-sections -c curskeys.c
gcc -O0 -g3 -std=gnu23 -fdiagnostics-color=always -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wno-unused-function -DNCURSES_WIDECHAR -Wl,--gc-sections -c mview.c
gcc -O0 -g3 -std=gnu23 -fdiagnostics-color=always -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wno-unused-function -DNCURSES_WIDECHAR -Wl,--gc-sections -c view_engine.c
gcc -O0 -g3 -std=gnu23 -fdiagnostics-color=always -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wno-unused-function -DNCURSES_WIDECHAR -Wl,--gc-sections -c dwin.c
gcc -O0 -g3 -std=gnu23 -fdiagnostics-color=always -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wno-unused-function -DNCURSES_WIDECHAR -Wl,--gc-sections -c futil.c
gcc -O0 -g3 -std=gnu23 -fdiagnostics-color=always -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wno-unused-function -DNCURSES_WIDECHAR -Wl,--gc-sections -c init.c
gcc -O0 -g3 -std=gnu23 -fdiagnostics-color=always -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wno-unused-function -DNCURSES_WIDECHAR -Wl,--gc-sections -c mem.c
gcc -O0 -g3 -std=gnu23 -fdiagnostics-color=always -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wno-unused-function -DNCURSES_WIDECHAR -Wl,--gc-sections -c opts.c
gcc -O0 -g3 -std=gnu23 -fdiagnostics-color=always -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wno-unused-function -DNCURSES_WIDECHAR -Wl,--gc-sections -c scriou.c
gcc -O0 -g3 -std=gnu23 -fdiagnostics-color=always -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wno-unused-function -DNCURSES_WIDECHAR -Wl,--gc-sections -c exec.c
gcc -O0 -g3 -std=gnu23 -fdiagnostics-color=always -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wno-unused-function -DNCURSES_WIDECHAR -Wl,--gc-sections -c sig.c
gcc -O0 -g3 -std=gnu23 -fdiagnostics-color=always -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wno-unused-function -DNCURSES_WIDECHAR -Wl,--gc-sections menu.o menu_engine.o parse_menu_desc.o init_view.o form_engine.o \
 fields.o pick_engine.o curskeys.o mview.o view_engine.o dwin.o futil.o \
 init.o mem.o opts.o scriou.o exec.o sig.o -Wl,--gc-sections -o menu -lncursesw -ltinfo
gcc -O0 -g3 -std=gnu23 -fdiagnostics-color=always -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wno-unused-function -DNCURSES_WIDECHAR -Wl,--gc-sections -c form.c
gcc -O0 -g3 -std=gnu23 -fdiagnostics-color=always -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wno-unused-function -DNCURSES_WIDECHAR -Wl,--gc-sections form.o fields.o form_engine.o mview.o init_view.o view_engine.o \
 dwin.o futil.o init.o mem.o opts.o scriou.o exec.o sig.o -Wl,--gc-sections -o form \
 -lncursesw -ltinfo
gcc -O0 -g3 -std=gnu23 -fdiagnostics-color=always -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wno-unused-function -DNCURSES_WIDECHAR -Wl,--gc-sections -c pick.c
gcc -O0 -g3 -std=gnu23 -fdiagnostics-color=always -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wno-unused-function -DNCURSES_WIDECHAR -Wl,--gc-sections pick.c pick_engine.o mview.o init_view.o view_engine.o \
 dwin.o futil.o init.o mem.o opts.o scriou.o exec.o sig.o \
 -Wl,--gc-sections -o pick -lncursesw -ltinfo
gcc -O0 -g3 -std=gnu23 -fdiagnostics-color=always -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wno-unused-function -DNCURSES_WIDECHAR -Wl,--gc-sections -c ckeys.c
gcc -O0 -g3 -std=gnu23 -fdiagnostics-color=always -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wno-unused-function -DNCURSES_WIDECHAR -Wl,--gc-sections ckeys.o curskeys.o dwin.o futil.o init.o mem.o opts.o scriou.o \
 exec.o sig.o -Wl,--gc-sections -o ckeys -lncursesw -ltinfo
gcc -O0 -g3 -std=gnu23 -fdiagnostics-color=always -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wno-unused-function -DNCURSES_WIDECHAR -Wl,--gc-sections -c view.c
gcc -O0 -g3 -std=gnu23 -fdiagnostics-color=always -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wno-unused-function -DNCURSES_WIDECHAR -Wl,--gc-sections view.o init_view.o view_engine.o mview.o dwin.o futil.o \
 init.o mem.o opts.o scriou.o exec.o sig.o -Wl,--gc-sections -o view -lncursesw -ltinfo
gcc -O0 -g3 -std=gnu23 -fdiagnostics-color=always -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wno-unused-function -DNCURSES_WIDECHAR -Wl,--gc-sections -Wl,--gc-sections whence.c -o whence -lc
gcc -O0 -g3 -std=gnu23 -fdiagnostics-color=always -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wno-unused-function -DNCURSES_WIDECHAR -Wl,--gc-sections -Wl,--gc-sections rsh.c -o rsh
gcc -O0 -g3 -std=gnu23 -fdiagnostics-color=always -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wno-unused-function -DNCURSES_WIDECHAR -Wl,--gc-sections -Wl,--gc-sections iloan.c -o iloan -lc -lm
gcc -O0 -g3 -std=gnu23 -fdiagnostics-color=always -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wno-unused-function -DNCURSES_WIDECHAR -Wl,--gc-sections -c enterchr.c
gcc -O0 -g3 -std=gnu23 -fdiagnostics-color=always -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wno-unused-function -DNCURSES_WIDECHAR -Wl,--gc-sections enterchr.o dwin.o futil.o init.o mem.o opts.o scriou.o sig.o \
 -Wl,--gc-sections -o enterchr -lncursesw -ltinfo
gcc -O0 -g3 -std=gnu23 -fdiagnostics-color=always -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wno-unused-function -DNCURSES_WIDECHAR -Wl,--gc-sections -c enterstr.c
gcc -O0 -g3 -std=gnu23 -fdiagnostics-color=always -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wno-unused-function -DNCURSES_WIDECHAR -Wl,--gc-sections enterstr.o dwin.o futil.o init.o mem.o opts.o scriou.o sig.o \
 -Wl,--gc-sections -o enterstr -lncursesw -ltinfo
Make Complete
```

Type: "sudo make install >make.install.out 2>&1 &" to save the make install
output in "make.install.out" for later review.

Example make install output:

```
./inst.sh	--
./inst.sh	menu		~/menuapp/bin	menu		0711	uname	users
./inst.sh	form		~/menuapp/bin	form		0711	uname	users
./inst.sh	pick		~/menuapp/bin	pick		0711	uname	users
./inst.sh	ckeys		~/menuapp/bin	ckeys		0711	uname	users
./inst.sh	view		~/menuapp/bin	view		0711	uname	users
./inst.sh	whence		~/menuapp/bin	whence		0711	uname	users
./inst.sh	rsh		    ~/menuapp/bin	rsh		    4711	root	root
./inst.sh	rsh		    ~/menuapp/bin	ush		    0711	uname	users
./inst.sh	enterchr	~/menuapp/bin	enterchr	0711	uname	users
./inst.sh	enterstr	~/menuapp/bin	enterstr	0711	uname	users
./inst.sh	iloan		~/menuapp/bin	iloan		0711	uname	users
./inst.sh	-l
```

```
Installed files:
.rwx--x--x. uname users 261 KB Sun Nov 30 00:13:49 2025 /home/uname/menuapp/bin/ckeys
.rwx--x--x. uname users 194 KB Sun Nov 30 00:13:49 2025 /home/uname/menuapp/bin/enterchr
.rwx--x--x. uname users 220 KB Sun Nov 30 00:13:49 2025 /home/uname/menuapp/bin/enterstr
.rwx--x--x. uname users 366 KB Sun Nov 30 00:13:49 2025 /home/uname/menuapp/bin/form
.rwx--x--x. uname users 103 KB Sun Nov 30 00:13:49 2025 /home/uname/menuapp/bin/iloan
.rwx--x--x. uname users 443 KB Sun Nov 30 00:13:49 2025 /home/uname/menuapp/bin/menu
.rwx--x--x. uname users 350 KB Sun Nov 30 00:13:49 2025 /home/uname/menuapp/bin/pick
.rws--x--x. root  root   87 KB Sun Nov 30 00:13:49 2025 /home/uname/menuapp/bin/rsh
.rwx--x--x. uname users  87 KB Sun Nov 30 00:13:49 2025 /home/uname/menuapp/bin/ush
.rwx--x--x. uname users 324 KB Sun Nov 30 00:13:49 2025 /home/uname/menuapp/bin/view
.rwx--x--x. uname users  81 KB Sun Nov 30 00:13:49 2025 /home/uname/menuapp/bin/whence
./inst.sh	-m

To copy the sample menuapp directory to your home directory:
cp -Rdup ../menuapp ~/
```
