# Makefile
# MENU
# Bill Waller
# billxwaller@gmail.com

PREFIX=~/menuapp
SHELL = /bin/sh
OBJS = init.o futil.o fields.o scriou.o dwin.o log.o fckeys.o
CFLAGS = -O0 -g3 -fdiagnostics-color=always -Wall -Wextra \
		-Wno-unused-parameter -Wno-missing-field-initializers \
		-Wno-unused-function -DNCURSES_WIDECHAR 
# CCFLAGS = -fanalyzer -fdiagnostics-color=always \
# 		-Wanalyzer-use-after-free -Wanalyzer-null-dereference \
# 		-Wanalyzer-malloc-leak -Wanalyzer-use-of-uninitialized-value \
# 		-Wanalyzer-double-free -xref -Ohtml -N$*
LDFLAGS =  -lncursesw -ltinfo
CC = gcc
CFLOW_FLAGS = --brief

all:	menu paint paintfile cpick ckeys view dgraph iloan whence rsh
	@echo Make Complete

menu:	menu.o fmenu.o rmenu.o mmenu.o fpaint.o fpick.o fview.o mpick.o \
		mview.o $(OBJS)
	$(CC) $(CFLAGS) menu.o fmenu.o rmenu.o mmenu.o fpaint.o fpick.o \
		fview.o mpick.o mview.o $(OBJS) $(LDFLAGS) -o $@ $(LIBS)

paint:	paint.o fpaint.o fview.o mview.o $(OBJS)
	$(CC) $(CFLAGS) paint.o fpaint.o fview.o mview.o \
		$(OBJS) $(LDFLAGS) -o $@ $(LIBS)

paintfile:	paintfile.o fpaint.o fview.o mview.o $(OBJS)
	$(CC) $(CFLAGS) paintfile.o fpaint.o fview.o mview.o \
		$(OBJS) $(LDFLAGS) -o $@ $(LIBS)

cpick:	cpick.o fpick.o mpick.o fview.o mview.o $(OBJS)
	$(CC) $(CFLAGS) cpick.o fpick.o mpick.o fview.o mview.o $(OBJS) \
		$(LDFLAGS) -o $@ $(LIBS)

view:	view.o fview.o mview.o $(OBJS)
	$(CC) $(CFLAGS) view.o fview.o mview.o $(OBJS) $(LDFLAGS) \
		-o $@ $(LIBS)

ckeys:	ckeys.o $(OBJS)
	$(CC) $(CFLAGS) ckeys.o $(OBJS) $(LDFLAGS) -o $@ $(LIBS)

dgraph:	dgraph.o $(OBJS)
	$(CC) $(CFLAGS) dgraph.o $(OBJS) $(LDFLAGS) -o $@ \
		$(LIBS) -lm

whence:	whence.c
	$(CC) $(CFLAGS) whence.c -o $@ -lc

iloan:	iloan.c
	$(CC) $(CFLAGS) iloan.c -o $@ -lc -lm

rsh:	rsh.c
	$(CC) $(CFLAGS) rsh.c -o $@

cpick.o:			cpick.c		menu.h
ckeys.o:			ckeys.c		menu.h
fckeys.o:			fckeys.c	menu.h
dgraph.o:			dgraph.c	menu.h
dwin.o:				dwin.c	    menu.h
fields.o:			fields.c    menu.h
fmenu.o:			fmenu.c	    menu.h
fpaint.o:			fpaint.c	menu.h
fpick.o:			fpick.c	   	menu.h
futil.o:			futil.c	    menu.h
fview.o:			fview.c	    menu.h
log.o:				log.c	    menu.h
menu.o:				menu.c	    menu.h
mmenu.o:			mmenu.c	    menu.h
mpick.o:			mpick.c    	menu.h
mview.o:			mview.c	    menu.h
paint.o:			paint.c	    menu.h
paintfile.o:		paintfile.c	menu.h
rmenu.o:			rmenu.c	    menu.h
scriou.o:			scriou.c	menu.h
view.o:				view.c	    menu.h
init.o:				init.c		menu.h

clean:
	rm -f core *.o menu paint paintfile cpick view dgraph ckeys \
	   iloan whence rsh make.out

install:
	./instexe dgraph    $(PREFIX)/bin    dgraph        0711    bin    bin
	./instexe menu      $(PREFIX)/bin    menu          0711    bin    bin
	./instexe view      $(PREFIX)/bin    more          0711    bin    bin
	./instexe paint     $(PREFIX)/bin    paint         0711    bin    bin
	./instexe paintfile $(PREFIX)/bin    paintfile     0711    bin    bin
	./instexe cpick     $(PREFIX)/bin    cpick         0711    bin    bin
	./instexe ckeys     $(PREFIX)/bin    ckeys         0711    bin    bin
	./instexe view      $(PREFIX)/bin    view          0711    bin    bin
	./instexe whence    $(PREFIX)/bin    whence        0711    bin    bin
	./instexe rsh       $(PREFIX)/bin    rsh           4711    root   root

uninstall:
	rm -fv	$(PREFIX)/bin/dgraph \
			$(PREFIX)/bin/menu	\
			$(PREFIX)/bin/more	\
			$(PREFIX)/bin/paint	\
			$(PREFIX)/bin/paintfile	\
			$(PREFIX)/bin/cpick	\
			$(PREFIX)/bin/ckeys	\
			$(PREFIX)/bin/view \
			$(PREFIX)/bin/rsh \
			$(PREFIX)/bin/instexe

