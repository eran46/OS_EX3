#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>    // unix standard functions 
#include <arpa/inet.h> //function for internet operation

#define SERVER_PORT 8080  //define number of the server port
#define SERVER_IP "127.0.0.1"  // IP address of the server
#define BUFFER_SIZE 1024 //size of buffers for sending and receiving messages in bytes
    
    
void *receive_messages(void *socket_desc) {
    int sock = *(int *)socket_desc;
    char buffer[BUFFER_SIZE];

    while (1) {
        memset(buffer, 0, BUFFER_SIZE); //clears the contents of the buffer and ensures no leftover data from previous messages interferes with the current message
        
        int bytes_received = recv(sock, buffer, BUFFER_SIZE - 1, 0); //reads data from the server into the buffer
        if (bytes_received <= 0) {
            perror("Message received- failed");
            continue;   
        } 

        buffer[bytes_received] = '\0'; // null-terminate the buffer
        printf("\nServer: %s\n", buffer);
        fflush(stdout); // ensure the prompt is displayed properly
    }
    return NULL;
}



void run_client() {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    char message[BUFFER_SIZE];
    char client_name[BUFFER_SIZE];

    // create socket
    sock = socket(AF_INET, SOCK_STREAM, 0); // SOCK_STREAM- indicates a TCP socket
    if (sock < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // configure server address
    server_addr.sin_family = AF_INET;  //specify the protocol for family
    server_addr.sin_port = htons(SERVER_PORT); //assigns port number from host byte order to network byte order
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // connect to the server
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sock);
        exit(EXIT_FAILURE);
    }

    printf("Connected to the server at %s:%d\n", SERVER_IP, SERVER_PORT);

    // notify the server of the client's name
    printf("Enter your name: ");
    fgets(client_name, BUFFER_SIZE, stdin);
    client_name[strcspn(client_name, "\n")] = 0; // remove newline character

    if (send(sock, client_name, strlen(client_name), 0) < 0) {
        perror("Failed to send client name");
        close(sock);
        exit(EXIT_FAILURE);
    }
    printf("Client name '%s' sent to the server\n", client_name);

    // create a thread to handle incoming messages
    pthread_t recv_thread;
    if (pthread_create(&recv_thread, NULL, receive_messages, (void *)&sock) != 0) {
        perror("Thread creation failed- can't receive incoming messages");
        close(sock);
        exit(EXIT_FAILURE);
    }

    while (1) {
        // get user input
        printf("Enter normal message or whisper message or type '!exit' to quit\n");
        fgets(message, BUFFER_SIZE, stdin);
        message[strcspn(message, "\n")] = 0; // remove newline character

        //exit command
        if (strcmp(message, "!exit") == 0) {
            if (send(sock, message, strlen(message), 0) < 0) {
                perror("Send to server failed");
                break;
            }
            printf("Client exiting\n");
            break;
        }

        // handle whisper messages
        if (strncmp(message, "@", 1) == 0) {
            // whisper message format: @friend message
            if (send(sock, message, strlen(message), 0) < 0) {
                perror("Send to server failed"");
                break;
            }
        } else {
            // normal message
            if (send(sock, message, strlen(message), 0) < 0) {
                perror("Send to server failed"");
                break;
            }
        }
    }

    // close the socket
    close(sock);
    pthread_cancel(recv_thread); // ensure the thread is terminated
    pthread_join(recv_thread, NULL); // wait for the thread to finish
}

   
