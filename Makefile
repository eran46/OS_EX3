# compiler
CC = gcc

# flags
CFLAGS = -Wall -g


all: hw3client hw3server


hw3client: client.o client_utils.o  
	$(CC) $(CFLAGS) -o hw3client client.o client_utils.o

hw3server: server.o server_utils.o
	$(CC) $(CFLAGS) -o hw3server server.o server_utils.o
	
server.o: server.c server_utils.h
	$(CC) $(CFLAGS) -c server.c -o server.o

server_utils.o: server_utils.c server_utils.h
	$(CC) $(CFLAGS) -c server_utils.c -o server_utils.o

client.o: client.c client.h client_utils.h
	$(CC) $(CFLAGS) -c client.c -o client.o 

client_utils.o: client_utils.c client_utils.h
	$(CC) $(CFLAGS) -c client_utils.c -o client_utils.o
	


clean:
	rm -f hw3client hw3server client.o client_utils.o server.o server_utils.o
