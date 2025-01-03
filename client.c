#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>    // unix standard functions 
#include <arpa/inet.h> //function for internet operation

#include "client_utils.h"
#define BUFFER_SIZE 1024 //size of buffers for sending and receiving messages in bytes
    

int main(int argc, char *argv[]) {   //run client
    
    const char *server_address =argv[1];
    int port = atoi(argv[2]);
    const char *client_name = argv[3];
    
    int sock;
    struct sockaddr_in server_addr;
    char message[BUFFER_SIZE];

    // create socket
    sock = socket(AF_INET, SOCK_STREAM, 0); // SOCK_STREAM- indicates a TCP socket
    if (sock < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // configure server 
    server_addr.sin_family = AF_INET;  //specify the protocol for family
    server_addr.sin_port = htons(port); //assigns port number from host byte order to network byte order
    if (inet_pton(AF_INET, server_address, &server_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // connect to the server
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection to the server failed");
        close(sock);
        exit(EXIT_FAILURE);
    }
    
    // sends client name to the server
    if (send(sock, client_name, strlen(client_name), 0) < 0) {
    	perror("Failed to send client name");
        close(sock);
        exit(EXIT_FAILURE);
    }
    
    printf("Connected to the server at %s:%d\n", server_address, port);


    // create a thread to handle incoming messages
    pthread_t recv_thread;
    if (pthread_create(&recv_thread, NULL, receive_messages, (void *)&sock) != 0) {
        perror("Thread creation failed- can't receive incoming messages");
        clean_exit(sock, recv_thread);
    }
    

    while (1) {
 
        // get user input
        //-----option-----printf("Enter normal message or whisper message or type '!exit' to quit\n");
        
        // handaling NULL input
        if (fgets(message, BUFFER_SIZE, stdin) != NULL) {
            message[strcspn(message, "\n")] = 0; // Remove newline character
        } 
        
        else 	{
            perror("Error reading input or empty input - try again");
            continue;
        }
	
	
        //!exit command
        if (strcmp(message, "!exit") == 0) {
            if (send(sock, message, strlen(message), 0) < 0) {
                perror("Send !exit to the server failed");
                clean_exit(sock, recv_thread);
            }
            printf("Client exiting\n");
            clean_exit(sock, recv_thread);
        }


        // handle messages - server diffrinciate between cases
        if (send(sock, message, strlen(message), 0) < 0) {
            perror("Failed to send message to the server");
            clean_exit(sock, recv_thread);   
        }
    }
    clean_exit(sock, recv_thread);
}

   
