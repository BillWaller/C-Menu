#  Makefile
#  Bill Waller Copyright (c) 2025
#  Makefile for C-Menu application suite
#  MIT License
#  billxwaller@gmail.com
#
# Makefile for the MENU application suite

USER=bill
GROUP=bill

REL_CFLAGS = -O1
DBG_CFLAGS = -g3

CC = gcc -Wall -Wextra -g3
PREFIX = ~/menuapp
LIB_DIR =  /usr/local/lib64
LIB_NAME= libcm.so

BINEXES = menu form pick ckeys view whence rsh iloan enterchr \
		  enterstr lf optsp stripansi
LIB_SRCS = dwin.c futil.c scriou.c exec.c sig.c
LIB_OBJS = $(LIB_SRCS:.c=.o)
COMMON_SRCS = curskeys.c fields.c form_engine.c init.c init_view.c \
			   mem.c mview.c opts.c pick_engine.c view_engine.c
COMMON_OBJS = $(COMMON_SRCS:.c=.o)
MENU_SRCS = menu.c menu_engine.c parse_menu_desc.c
MENU_OBJS= $(MENU_SRCS:.c=.o)
FORM_SRCS = form.c
FORM_OBJS= $(FORM_SRCS:.c=.o)
PICK_SRCS = pick.c
PICK_OBJS = $(PICK_SRCS:.c=.o)
VIEW_SRCS = view.c
VIEW_OBJS = $(VIEW_SRCS:.c=.o)
CKEYS_SRCS = ckeys.c
CKEYS_OBJS = $(CKEYS_SRCS:.c=.o)
LIBS = -L. -lcm -lncursesw -ltinfo -lm -Wl,-rpath,$(LIB_DIR)

all:	$(LIB_NAME) $(BINEXES)
	@echo Make Complete

menu: 		$(MENU_OBJS)  $(COMMON_OBJS) $(LIB_NAME)
	$(CC) 	$(MENU_OBJS)  $(COMMON_OBJS) -o $@ $(LIBS)

form: 		$(FORM_OBJS)  $(COMMON_OBJS) $(LIB_NAME)
	$(CC) 	$(FORM_OBJS)  $(COMMON_OBJS) -o $@ $(LIBS)

pick: 		$(PICK_OBJS)  $(COMMON_OBJS) $(LIB_NAME)
	$(CC) 	$(PICK_OBJS)  $(COMMON_OBJS) -o $@ $(LIBS)

view: 		$(VIEW_OBJS)  $(COMMON_OBJS) $(LIB_NAME)
	$(CC) 	$(VIEW_OBJS)  $(COMMON_OBJS) -o $@ $(LIBS)

ckeys: 		$(CKEYS_OBJS) $(COMMON_OBJS) $(LIB_NAME)
	$(CC) 	$(CKEYS_OBJS) $(COMMON_OBJS) -o $@ $(LIBS)

enterchr:	enterchr.o 					 $(LIB_NAME)
	$(CC) 	enterchr.o 					 -o $@ $(LIBS)

optsp:		optsp.o opts.o 				 $(LIB_NAME)
	$(CC) 	optsp.o opts.o 				 -o $@ $(LIBS)

enterstr:	enterstr.o 					 $(LIB_NAME)
	$(CC) 	enterstr.o 					 -o $@ $(LIBS)

whence:		whence.o 					 $(LIB_NAME)
	$(CC) 	whence.o 					 -o $@ $(LIBS)

iloan:		iloan.o 					 $(LIB_NAME)
	$(CC) 	iloan.o 					 -o $@ $(LIBS)

rsh:	rsh.o 							 $(LIB_NAME)
	$(CC) 	rsh.o 			-o $@ 		 $(LIBS)

lf:			lf.o 						 $(LIB_NAME)
	$(CC) 	lf.o 			-o $@ 		 $(LIBS)

stripansi:	stripansi.o  				 $(LIB_NAME)
	$(CC) 	stripansi.o 	-o $@		 $(LIBS)

$(LIB_NAME): $(LIB_OBJS)
	$(CC) 	-shared -o $@ $^

%.o:	%.c
	$(CC) 	-c $< -fPIC	-o $@

clean:
	rm -rf core *.o $(BINEXES) make.out $(CFLOW_OUTPUT) *.o libcm.so \
		*.cflow scan-build scan-build.out installed gdbcmds tm \
	   	tf tp tv x xx xxx *.out .user_id .group_id

install:
	./inst.sh	--
	./inst.sh	libcm.so	$(LIB_DIR)	0711	$(USER)	$(GROUP)
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
