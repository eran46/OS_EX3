#include <sys/socket.h> // sockets
#include <arpa/inet.h> // ipv4
#include <sys/types.h>
#include <unistd.h> // processs
#include <signal.h> // signals
#include "server_utils.h"


void handle_client(int* client_socket, struct sockaddr_in *client_address) {
	
    
    
    char buffer[BUFFER_SIZE];
    char client_ip[INET_ADDRSTRLEN];
    int bytes_received;
    
    // get human readable IPv4 address
    socklen_t str_size = INET_ADDRSTRLEN; // INET_ADDRSTRLEN = maximum length of a string representation of an IPv4 address, including the null terminator
    char* client_ip[str_size]; // human readable IPv4 string
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, str_size); // convert binary IPv4 to human-readable string
        // AF_INET = IPv4, output to client_ip
        

    // Convert client address to human-readable string
    inet_ntop(AF_INET, &client_address->sin_addr, client_ip, INET_ADDRSTRLEN);
    printf("Client connected from %s:%d\n", client_ip, ntohs(client_address->sin_port));

    // Communicate with the client
    while ((bytes_received = recv(*client_socket, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[bytes_received] = '\0'; // Null-terminate the received data
        printf("Received: %s\n", buffer);

        // Echo the message back to the client
        if (send(*client_socket, buffer, bytes_received, 0) < 0) {
            perror("send");
            break;
        }
    }

    if (bytes_received == 0) {
        printf("Client disconnected from %s:%d\n", client_ip, ntohs(client_address->sin_port));
    } else {
        perror("recv");
    }

    // Clean up
    close(*client_socket);
    free(client_socket);
    exit(EXIT_SUCCESS);
}


void run_server(int server_socket){
    // reap zombie process infrastructure
    struct sigaction sa;
    sa.sa_handler = reap_zombies; // on signal () call reap_zombies()
    sa.sa_flags = SA_RESTART; // restart interrupted system callse
    sigaction(SIGCHLD, &sa, NULL); // action on SIGCHLD=when child process exits
    
    while(1){
    	struct sockaddr_in *client_address; // create client sockaddr_in object *pointer*
	socklen_t client_len = sizeof(client_address);
	int* client_socket = (int*)malloc(sizeof(int)); // dynamic allocation in order to send pointer to handle_client()
    
    
        *client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_len); // blocking call, creates new client_socket from stream, binds to client_address
        if(client_socket == -1){ // failed handshake
            printf("server accept() failed.\n");
            continue; // continue listening
        }
        
        // we chose processes instead of threads for client security, each client doesn't share memory space with other clients.
        // could have chosen non blocking call implementation
        int status;
        pid_t pid;
        if((pid = fork()) < 0){
	    printf("failed to start client chat process. (fork())\n");
	    close(*client_socket);
	    free(client_socket);
	    // TODO: replace with cleanup function that reaps all current child processes before exiting
	    break;
	}
        else if(pid > 0){ // father process
            
            
            close(client_socket); // parent doesnt need handled client socket
        }
        else{ // child process
            close(server_socket); // child doesnt need it
            handle_client(client_socket, client_address);
            exit(); // ensure child process terminates
        }
    }
}

int main(int argc, char* argv[]){
    
    check_arg_num() // check bad argument number
    	
    int port = atoi(argv[1]); // server port
    // TODO: check bad port?
    
    int server_socket = init_server_socket(port); // initialize socket
    
    // struct sockaddr -> generic, struct sockaddr_in -> IPv4
    struct sockaddr_in server_address = init_server_address(port); // initialize the server address
    
    if(bind_socket_to_addr(server_socket, server_address)){ // bind socket to server address
    	close(server_socket);
    	return 1;
    }
    
    if(listen(server_socket, MAX_CLIENT_QUEUE) == -1){ // listen for incoming TCP connections on port
        printf("socket listen failed.\n");
        close(server_socket);
        return 1;
    }
    
    printf("Server started.\n");
    run_server(server_socket);
    
    // cleanup
    close(server_socket);

}
