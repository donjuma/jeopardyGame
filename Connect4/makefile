CC=gcc
CFLAGS = -g -Wall -o
SOURCES = prog0_client.c prog0_server.c

all: c s

c: prog1_client.c
	$(CC) -o prog1_client prog1_client.c

s: prog1_server.c
	$(CC) -o prog1_server prog1_server.c

clean:
	rm prog1_client prog1_server
