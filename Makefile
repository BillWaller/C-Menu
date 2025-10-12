# Makefile
# MENU
# Bill Waller
# billxwaller@gmail.com

PREFIX=/usr/local
SHELL = /bin/sh
OBJS = futil.o fields.o scriou.o dwin.o
CFLAGS = -g -O0 -Wall
LDFLAGS =  -lncursesw -ltinfo -lconfig
CC = gcc

all:	menu paint paintfile cpick view dgraph iloan whence
	@echo Make Complete

menu:	menu.o fmenu.o rmenu.o mmenu.o fpaint.o fpick.o fview.o mpick.o \
		mview.o init.o $(OBJS)
	$(CC) $(CFLAGS) menu.o fmenu.o rmenu.o mmenu.o fpaint.o fpick.o \
		fview.o mpick.o mview.o init.o $(OBJS) $(LDFLAGS) -o \
		$@ $(LIBS)

paint:	paint.o fpaint.o fview.o mview.o init.o $(OBJS)
	$(CC) $(CFLAGS) paint.o fpaint.o fview.o mview.o init.o \
		$(OBJS) $(LDFLAGS) -o $@ $(LIBS)

paintfile:	paintfile.o fpaint.o fview.o mview.o init.o $(OBJS)
	$(CC) $(CFLAGS) paintfile.o fpaint.o fview.o mview.o init.o \
		$(OBJS) $(LDFLAGS) -o $@ $(LIBS)

cpick:	cpick.o fpick.o mpick.o fview.o mview.o init.o $(OBJS)
	$(CC) $(CFLAGS) cpick.o fpick.o mpick.o fview.o mview.o init.o $(OBJS) \
		$(LDFLAGS) -o $@ $(LIBS)

view:	view.o fview.o mview.o init.o $(OBJS)
	$(CC) $(CFLAGS) view.o fview.o mview.o init.o $(OBJS) $(LDFLAGS) \
		-o $@ $(LIBS)

dgraph:	dgraph.o init.o $(OBJS)
	$(CC) $(CFLAGS) dgraph.o init.o $(OBJS) $(LDFLAGS) -o $@ \
		$(LIBS) -lm

whence:	whence.c
	$(CC) $(CFLAGS) whence.c -o $@ -lc

iloan:	iloan.c
	$(CC) $(CFLAGS) iloan.c -o $@ -lc -lm

cpick.o:			cpick.c		menu.h
dgraph.o:			dgraph.c	menu.h
dwin.o:				dwin.c	    menu.h
fields.o:			fields.c    menu.h
fmenu.o:			fmenu.c	    menu.h
fpaint.o:			fpaint.c	menu.h
fpick.o:			fpick.c	   	menu.h
futil.o:			futil.c	    menu.h
fview.o:			fview.c	    menu.h
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
	rm -f core *.o menu paint paintfile cpick view dgraph\
	   iloan whence	make.out

install:
	./instexe dgraph	$(PREFIX)/bin	dgraph		0711	bin	bin
	./instexe menu		$(PREFIX)/bin	menu		0711	bin	bin
	./instexe view		$(PREFIX)/bin	more		0711	bin	bin
	./instexe paint		$(PREFIX)/bin	paint		0711	bin	bin
	./instexe paintfile	$(PREFIX)/bin	paintfile	0711	bin	bin
	./instexe cpick		$(PREFIX)/bin	cpick		0711	bin	bin
	./instexe view		$(PREFIX)/bin	view		0711	bin	bin
	./instexe whence	$(PREFIX)/bin	whence		0711	bin	bin

uninstall:
	rm -fv	$(PREFIX)/bin/dgraph \
			$(PREFIX)/bin/menu	\
			$(PREFIX)/bin/more	\
			$(PREFIX)/bin/paint	\
			$(PREFIX)/bin/paintfile	\
			$(PREFIX)/bin/cpick	\
			$(PREFIX)/bin/view
