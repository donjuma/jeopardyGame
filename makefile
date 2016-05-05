CC=gcc
CFLAGS = -g -Wall -o

all: o p s

o: jeopardyGame_observer.c
	$(CC) -o jeopardyGame_observer jeopardyGame_observer.c

p: jeopardyGame_participant.c
	$(CC) -o jeopardyGame_participant jeopardyGame_participant.c

s: jeopardyGame_server.c
	$(CC) -o jeopardyGame_server jeopardyGame_server.c

clean:
	rm jeopardyGame_observer jeopardyGame_participant jeopardyGame_server
