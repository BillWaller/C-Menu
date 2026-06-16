CC = gcc
CFLAGS = -Wall
LIBS=-lncursesw -lpanelw -ltinfow

%: %.c %.h
	$(CC) -g $(CFLAGS) $< -o $@ $(LIBS)


clean:
	rm -f pan1 border *.out
