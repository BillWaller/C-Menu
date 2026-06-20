CC = gcc
CFLAGS = -Wall
LIBS=-lnotcurses-core -lnotcurses -ltinfo

%: %.c
	$(CC) -g $(CFLAGS) $< -o $@ $(LIBS)


clean:
	rm -f pan1 border *.out
