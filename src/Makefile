#  Makefile
#  Bill Waller Copyright (c) 2025
#  Makefile for C-Menu application suite
#  MIT License
#  billxwaller@gmail.com
#
# Makefile for the MENU application suite
#

REL_CFLAGS = -O1
DBG_CFLAGS = -g3
#
PREFIX=~/menuapp
# LIBDIR could be set to ~/menuapp/lib64
# it would need to be added to /etc/ld.so.conf
LIBDIR=/usr/local/lib64
#
CC = gcc
CFLAGS = -Wall -Wextra -g3
LIBS =  -lcm -lncursesw -ltinfo -lm
BINEXES = menu form pick ckeys view whence rsh iloan enterchr \
		  enterstr lf optsp stripansi
LIB_NAME= libcm.so
LIB_SRCS = dwin.c futil.c scriou.c exec.c sig.c
LIB_OBJS = $(LIB_SRCS:.c=.o)
MENU_SRCS = menu.c menu_engine.c parse_menu_desc.c init_view.c fields.c form_engine.c pick_engine.c curskeys.c mview.c view_engine.c init.c mem.c opts.c
MENU_OBJS= $(MENU_SRCS:.c=.o)
FORM_SRCS = form.c fields.c form_engine.c mview.c init_view.c view_engine.c init.c mem.c opts.c 
FORM_OBJS= $(FORM_SRCS:.c=.o)
PICK_SRCS = pick.c pick_engine.c mview.c init_view.c view_engine.c fields.c form_engine.c init.c mem.c opts.c
PICK_OBJS = $(PICK_SRCS:.c=.o)
VIEW_SRCS = view.c init_view.c view_engine.c fields.c form_engine.c mview.c init.c mem.c opts.c
VIEW_OBJS = $(VIEW_SRCS:.c=.o)
CKEYS_SRCS = ckeys.c curskeys.c init.c mem.c opts.c
CKEYS_OBJS = $(CKEYS_SRCS:.c=.o)

# *** IMPORTANT: USER = YOUR USERID ***
# Unless your user ID is bill, you will need to change this
USER=bill
GROUP=bill
# or, this might work if you save your user and group IDs
# in ./user_id and ./group_id respectively
# USER := $(shell id -u -n)
# GROUP := $(shell id -g -n)

all:	$(LIB_NAME) $(BINEXES)
	@echo Make Complete

menu: $(MENU_OBJS) $(LIB_NAME)
	$(CC) $(CFLAGS) $(MENU_OBJS) -L. $(LIBS) -o $@ -Wl,-rpath,$(LIBDIR)

form: $(FORM_OBJS) $(LIB_NAME)
	$(CC) $(CFLAGS) $(FORM_OBJS) -L. $(LIBS) -o $@ -Wl,-rpath,$(LIBDIR)

pick: $(PICK_OBJS) $(LIB_NAME)
	$(CC) $(CFLAGS) $(PICK_OBJS) -L. $(LIBS) -o $@ -Wl,-rpath,$(LIBDIR)

view: $(VIEW_OBJS) $(LIB_NAME)
	$(CC) $(CFLAGS) $(VIEW_OBJS) -L. $(LIBS) -o $@ -Wl,-rpath,$(LIBDIR)

ckeys: $(CKEYS_OBJS) $(LIB_NAME)
	$(CC) $(CFLAGS) $(CKEYS_OBJS) -L. $(LIBS) -o $@ -Wl,-rpath,$(LIBDIR)

cpick: $(CPICK_OBJS) $(LIB_NAME)
	$(CC) $(CFLAGS) $(CPICK_OBJS) -L. $(LIBS) -o $@ -Wl,-rpath,$(LIBDIR)

optsp:	optsp.o opts.o $(LIB_NAME)
	$(CC) optsp.o opts.o $(CFLAGS) -L. $(LIBS) -o $@ -Wl,-rpath,$(LIBDIR)

enterchr:	enterchr.o $(LIB_NAME)
	$(CC) $(CFLAGS) enterchr.o -L. $(LIBS) -o $@ -Wl,-rpath,$(LIBDIR)

enterstr:	enterstr.o $(LIB_NAME)
	$(CC) $(CFLAGS) enterstr.o -L. $(LIBS) -o $@ -Wl,-rpath,$(LIBDIR)

whence:	whence.o $(LIB_NAME)
	$(CC) $(CFLAGS) whence.o -L. $(LIBS) -o $@ -Wl,-rpath,$(LIBDIR)

iloan:	iloan.o $(LIB_NAME)
	$(CC) $(CFLAGS) iloan.o -L. $(LIBS) -o $@ -Wl,-rpath,$(LIBDIR)

rsh:	rsh.o $(LIB_NAME)
	$(CC) $(CFLAGS) rsh.o -L. $(LIBS) -o $@ -Wl,-rpath,$(LIBDIR)

lf:	lf.o $(LIB_NAME)
	$(CC) $(CFLAGS) lf.o -L. $(LIBS) -o $@ -Wl,-rpath,$(LIBDIR)

stripansi:	stripansi.o  $(LIB_NAME)
	$(CC) $(CFLAGS) stripansi.o -L. $(LIBS) -o $@ -Wl,-rpath,$(LIBDIR)

$(LIB_NAME): $(LIB_OBJS)
	$(CC) $(CFLAGS) -shared -o $@ $^

%.o:	%.c
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

clean:
	rm -rf core *.o $(BINEXES) make.out $(CFLOW_OUTPUT) *.o libcm.so \
		*.cflow scan-build scan-build.out installed gdbcmds tm \
	   	tf tp tv x xx xxx *.out .user_id .group_id

install:
	./inst.sh	--
	./inst.sh	libcm.so	$(LIBDIR)	0711	$(USER)	$(GROUP)
	./inst.sh	menu		$(PREFIX)/bin		0711	$(USER)	$(GROUP)
	./inst.sh	form		$(PREFIX)/bin		0711	$(USER)	$(GROUP)
	./inst.sh	pick		$(PREFIX)/bin		0711	$(USER)	$(GROUP)
	./inst.sh	ckeys		$(PREFIX)/bin		0711	$(USER)	$(GROUP)
	./inst.sh	view		$(PREFIX)/bin		0711	$(USER)	$(GROUP)
	./inst.sh	optsp		$(PREFIX)/bin		0711	$(USER)	$(GROUP)
	./inst.sh	whence		$(PREFIX)/bin		0711	$(USER)	$(GROUP)
	./inst.sh	rsh		$(PREFIX)/bin		4711	root	root
	./inst.sh	enterchr	$(PREFIX)/bin		0711	$(USER)	$(GROUP)
	./inst.sh	enterstr	$(PREFIX)/bin		0711	$(USER)	$(GROUP)
	./inst.sh	iloan		$(PREFIX)/bin		0711	$(USER)	$(GROUP)
	./inst.sh	stripansi	$(PREFIX)/bin		0711	$(USER)	$(GROUP)
	./inst.sh	lf		$(PREFIX)/bin		0711	$(USER)	$(GROUP)
	ldconfig
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
			$(PREFIX)/bin/enterstr \
			$(PREFIX)/bin/stripansi \
			$(PREFIX)/bin/lf \
			$(LIBDIR)/libcm.so
