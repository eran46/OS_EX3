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

   while (!terminate) { // Check termination flag
        memset(buffer, 0, BUFFER_SIZE); // clear buffer to avoid leftover 
        
        int bytes_received = recv(sock, buffer, BUFFER_SIZE - 1, 0); // read data from the server
        if (bytes_received <= 0) {
        	 if (terminate) {	
                	break; // Exit loop if termination is requested
            	}
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
    terminate = 1;          // signal the receiving thread to exit
    shutdown(sock, SHUT_RDWR); // close the socket for both read and write
    close(sock);            // close the socket descriptor
    pthread_join(thread, NULL); // wait for the receiving thread to finish
    exit(EXIT_SUCCESS);
    }

