UI?="-DNOTCURSES_UI"
CC = gcc
CFLAGS = -Wall -I../ \
	"-D_XOPEN_SOURCE_EXTENDED" \
	"-D_XOPEN_SOURCE 700" \
	"-D_GNU_SOURCE 1"
LIBS= -lpanelw -lncursesw
LIBS= -lnotcurses-core -lnotcurses -ltinfo

%: %.c
	$(CC) -g $(CFLAGS) $< -o $@ $(LIBS)

clean:
	rm -f *.out
