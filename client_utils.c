#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>     // unix standard functions
#include <arpa/inet.h>  // functions for internet operations

#define BUFFER_SIZE 1024

void *receive_messages(void *socket_desc) {
    int sock = *(int *)socket_desc;
    char buffer[BUFFER_SIZE];

    while (1) {
        memset(buffer, 0, BUFFER_SIZE); // clear buffer to avoid leftover 
        
        int bytes_received = recv(sock, buffer, BUFFER_SIZE - 1, 0); // read data from the server
        if (bytes_received <= 0) {
         	perror("Message received- failed");
            	return NULL;   
        } 

        buffer[bytes_received] = '\0'; // null-terminate the buffer
        printf("%s\n", buffer);
        fflush(stdout); // ensure the output is flushed to the console
    }

    return NULL;
}


void clean_exit(int sock, pthread_t thread) {
    close(sock);
    pthread_join(thread, NULL);
    exit(EXIT_FAILURE);
}
