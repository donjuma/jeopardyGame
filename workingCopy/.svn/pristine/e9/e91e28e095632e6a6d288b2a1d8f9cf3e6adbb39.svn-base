CC=gcc
CFLAGS = -g -Wall -o

all: o p s

o: prog2_observer.c
	$(CC) -o prog2_observer prog2_observer.c

p: prog2_participant.c
	$(CC) -o prog2_participant prog2_participant.c

s: prog2_server.c
	$(CC) -o prog2_server prog2_server.c

clean:
	rm prog2_observer prog2_participant prog2_server
