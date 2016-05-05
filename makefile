CC=gcc
CFLAGS = -g -Wall -o

all: o p s

o: prog3_observer.c
	$(CC) -o prog3_observer prog3_observer.c

p: prog3_participant.c
	$(CC) -o prog3_participant prog3_participant.c

s: prog3_server.c
	$(CC) -o prog3_server prog3_server.c

clean:
	rm prog3_observer prog3_participant prog3_server
