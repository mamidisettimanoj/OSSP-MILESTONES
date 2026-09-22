CC = gcc
CFLAGS = -Wall -Wextra -g -I./include

TARGETS = src/session1 src/shell

all: $(TARGETS)

src/session1: src/session1.c
	$(CC) $(CFLAGS) -o src/session1 src/session1.c

src/shell: src/shell.c src/history.c src/parser.c
	$(CC) $(CFLAGS) -o src/shell src/shell.c src/history.c src/parser.c

clean:
	rm -f $(TARGETS)

.PHONY: all clean
