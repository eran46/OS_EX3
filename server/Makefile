# compiler
CC = gcc

# flags
CFLAGS = -Wall -g

all: hw3server

hw3server: server.o server_utils.o
	$(CC) $(CFLAGS) -o hw3server server.o server_utils.o

server.o: server.c server_utils.h
	$(CC) $(CFLAGS) -c server.c -o server.o

server_utils.o: server_utils.c server_utils.h
	$(CC) $(CFLAGS) -c server_utils.c -o server_utils.o

clean:
	rm -f hw3server server.o server_utils.o

