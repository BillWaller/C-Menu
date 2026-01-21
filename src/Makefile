#  Makefile
#  Bill Waller Copyright (c) 2025
#  Makefile for C-Menu application suite
#  MIT License
#  billxwaller@gmail.com
#
# Makefile for the MENU application suite
#
# If you create a simpler Makefile, without increasing the size of
# executables, please share. I have created much simpler Makefiles that
# work, but only by linking all object files, which increases the size of
# the executables significantly.
#
# This Makefile produces reasonably sized executables considering the
# inclusion of debugging information and symbols.
#
# You can optimize the executables by changing -O0 to -O2, removing -g3,
# and add -Wl,-s to the linker. As you can see, the executables get much
# smaller.
#
# Here are the sizes of the executables in KB using three different
# Makefiles:
#
#           Generic Custom Optimized
# ckeys       358     260     63
# enterchr    297     193     14
# enterstr    328     220     35
# form        403     360     96
# iloan       310      98     20
# menu        437     433    128
# pick        394     348     95
# rsh         293      87     14
# view        386     326     83
# whence      293      81     20
# total      3499    2406    568
#
# If the Child Exit and Signal messages from rsh are annoying,
# remove the -DVERBOSE flag from the rsh compile instruction.
# The default is to compile without -DVERBOSE.
#
# for release, use
REL_CFLAGS = -O2
# for debugging with GDB, use
DBG_CFLAGS = -O0 -g3
#
# Required Libraries:
#
# libc.so.6
# libm.so.6
# libncursesw.so.6
# libtinfo.so.6
#
CFLAGS = $(DBG_CFLAGS) -std=gnu23 -fdiagnostics-color=always -Wall -Wextra \
		-Wno-unused-parameter -Wno-missing-field-initializers \
		-Wno-unused-function -DNCURSES_WIDECHAR -Wl,--gc-sections
LIBS =  -lncursesw -ltinfo -lm
LDFLAGS = -Wl,--gc-sections
CC = gcc
TARGETS = menu form pick ckeys view whence rsh iloan enterchr enterstr lf \
		  optsp stripansi
TARGET_LIB = libmenu.a

all:	$(TARGETS)
	@echo Make Complete

menu:	menu.o menu_engine.o parse_menu_desc.o init_view.o \
		form_engine.o fields.o pick_engine.o curskeys.o mview.o view_engine.o \
		dwin.o futil.o init.o mem.o opts.o scriou.o exec.o sig.o
	$(CC) $(CFLAGS) menu.o menu_engine.o parse_menu_desc.o init_view.o form_engine.o \
		pick_engine.o curskeys.o mview.o view_engine.o \
	   	dwin.o exec.o fields.o futil.o init.o mem.o opts.o scriou.o sig.o $(LDFLAGS) -o $@ $(LIBS)
form:	form.o form_engine.o mview.o init_view.o view_engine.o \
		dwin.o exec.o fields.o futil.o init.o mem.o opts.o scriou.o sig.o
	$(CC) $(CFLAGS) form.o form_engine.o mview.o init_view.o view_engine.o \
		dwin.o exec.o fields.o futil.o init.o mem.o opts.o scriou.o sig.o $(LDFLAGS) -o $@ $(LIBS)
ckeys:	ckeys.o curskeys.o \
		dwin.o futil.o init.o mem.o opts.o scriou.o exec.o sig.o
	$(CC) $(CFLAGS) ckeys.o curskeys.o \
		dwin.o exec.o futil.o init.o mem.o opts.o scriou.o sig.o $(LDFLAGS) -o $@ $(LIBS)
pick:	pick.o  pick_engine.o mview.o init_view.o \
		view_engine.o form_engine.o fields.o \
		dwin.o exec.o futil.o init.o mem.o opts.o scriou.o sig.o
	$(CC) $(CFLAGS) pick.c  pick_engine.o mview.o init_view.o \
		view_engine.o form_engine.o fields.o \
		dwin.o exec.o futil.o init.o mem.o opts.o scriou.o sig.o \
		$(LDFLAGS) -o $@ $(LIBS)
view: view.o init_view.o view_engine.o form_engine.o fields.o mview.o \
   	dwin.o exec.o futil.o init.o mem.o opts.o scriou.o sig.o
	$(CC) $(CFLAGS) view.o init_view.o view_engine.o form_engine.o fields.o mview.o \
		dwin.o exec.o futil.o init.o mem.o opts.o scriou.o sig.o $(LDFLAGS) -o $@ $(LIBS)
optsp:	optsp.c opts.o
	$(CC) $(CFLAGS) $(LDFLAGS) optsp.c opts.o -o $@ $(LIBS)
enterchr:	enterchr.o \
		dwin.o futil.o init.o mem.o opts.o scriou.o sig.o
	$(CC) $(CFLAGS) enterchr.o \
		dwin.o futil.o init.o mem.o opts.o scriou.o sig.o $(LDFLAGS) -o $@ $(LIBS)
enterstr:	enterstr.o \
		dwin.o futil.o init.o mem.o opts.o scriou.o sig.o
	$(CC) $(CFLAGS) enterstr.o \
	   	dwin.o futil.o init.o mem.o opts.o scriou.o sig.o $(LDFLAGS) -o $@ $(LIBS)
whence:	whence.c
	$(CC) $(CFLAGS) $(LDFLAGS) whence.c -o $@ -lc
iloan:	iloan.c
	$(CC) $(CFLAGS) $(LDFLAGS) iloan.c -o $@ -lc -lm
rsh:	rsh.c
	$(CC) $(CFLAGS) $(LDFLAGS) rsh.c -o $@
lf:	lf.c
	$(CC) $(CFLAGS) $(LDFLAGS) lf.c -o $@
stripansi:	stripansi.c
	$(CC) $(CFLAGS) $(LDFLAGS) stripansi.c -o $@

%:	%.c menu.h
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@ $(LIBS)

%.o:	%.c menu.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf core *.o $(TARGETS) make.out $(CFLOW_OUTPUT) *.o *.cflow \
		scan-build scan-build.out installed gdbcmds tm tf tp tv x xx xxx

PREFIX=~/menuapp
OWNER=bill
GROUP=users

install:
	./inst.sh	--
	./inst.sh	menu		$(PREFIX)/bin	menu		0711	$(OWNER)	$(GROUP)
	./inst.sh	form		$(PREFIX)/bin	form		0711	$(OWNER)	$(GROUP)
	./inst.sh	pick		$(PREFIX)/bin	pick		0711	$(OWNER)	$(GROUP)
	./inst.sh	ckeys		$(PREFIX)/bin	ckeys		0711	$(OWNER)	$(GROUP)
	./inst.sh	view		$(PREFIX)/bin	view		0711	$(OWNER)	$(GROUP)
	./inst.sh	optsp		$(PREFIX)/bin	optsp		0711	$(OWNER)	$(GROUP)
	./inst.sh	whence		$(PREFIX)/bin	whence		0711	$(OWNER)	$(GROUP)
	./inst.sh	rsh		$(PREFIX)/bin	rsh		4711	root	root
	./inst.sh	rsh		$(PREFIX)/bin	ush		0711	$(OWNER)	$(GROUP)
	./inst.sh	enterchr	$(PREFIX)/bin	enterchr	0711	$(OWNER)	$(GROUP)
	./inst.sh	enterstr	$(PREFIX)/bin	enterstr	0711	$(OWNER)	$(GROUP)
	./inst.sh	iloan		$(PREFIX)/bin	iloan		0711	$(OWNER)	$(GROUP)
	./inst.sh	stripansi	$(PREFIX)/bin	stripansi	0711	$(OWNER)	$(GROUP)
	./inst.sh	lf		$(PREFIX)/bin	lf		0711	$(OWNER)	$(GROUP)
	./inst.sh -p
	./inst.sh -l
	./inst.sh -m

uninstall:
	rm -fv	$(PREFIX)/bin/menu	\
			$(PREFIX)/bin/form	\
			$(PREFIX)/bin/pick	\
			$(PREFIX)/bin/ckeys	\
			$(PREFIX)/bin/view \
			$(PREFIX)/bin/rsh \
			$(PREFIX)/bin/ush \
			$(PREFIX)/bin/enterchr \
			$(PREFIX)/bin/enterstr
			$(PREFIX)/bin/stripansi
			$(PREFIX)/bin/lf

