# compiler
CC = gcc

# flags
CFLAGS = -Wall -g

all: hw3

hw3: client.o client_utils.o  
	$(CC) $(CFLAGS) -o hw3 client.o client_utils.o

client.o: client.c client.h client_utils.h
	$(CC) $(CFLAGS) -o hw3 client.c -o client.o 

client_utils.o: client_utils.c client_utils.h
	$(CC) $(CFLAGS) -c client_utils.c -o client_utils.o

clean:
	rm -f hw3  client.o client_utils.o 
