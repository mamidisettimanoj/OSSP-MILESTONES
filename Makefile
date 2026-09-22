CC = gcc
CFLAGS = -Wall -Wextra -g

all: src/session1

src/session1: src/session1.c
	$(CC) $(CFLAGS) -o src/session1 src/session1.c

clean:
	rm -f src/session1
