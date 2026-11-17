CC = gcc
CFLAGS = -Wall -Wextra -O2

all: server cliente

server: server.c server.h
	$(CC) $(CFLAGS) server.c -o server

cliente: cliente.c server.h
	$(CC) $(CFLAGS) cliente.c -o cliente

clean:
	rm -f server cliente
