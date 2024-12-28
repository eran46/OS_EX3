#include <sys/socket.h> // sockets
#include <arpa/inet.h> // ipv4
#include <sys/types.h>
#include <unistd.h> // process
#include "server_utils.h"


void handle_client(int* client_socket){
    
}

void run_server(int server_socket){
    // create client sockaddr_in object
    struct sockaddr_in client_address;
    socklen_t client_len = sizeof(client_address); // TODO: why ???
    int* client_socket = (int*)malloc(sizeof(int));
    
    while(1){
        *client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_len); // (blocking call) get client 3 handshake
        if(client_socket == -1){ // failed handshake
            printf("server accept failed");
            continue;
        }
        char* client_ip[INET_ADDRSTRLEN]; // INET_ADDRSTRLEN = maximum length of a string representation of an IPv4 address, including the null terminator
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, (socklen_t)INET_ADDRSTRLEN); // convert binary ip to human-readable string
        
        // chose processes instead of threads for client security
        int status;
        pid_t pid;
        if((pid = fork()) == -1){
	    printf("failed to start client chat process");
	    close(*client_socket);
	    break;
	}
        
        if(pid > 0){ // father process
            int reaped = waitpid(-1, &status, WNOHANG); // reap zombie process
            while(reaped > 0){
                reaped = waitpid(-1, &status, WNOHANG);
            }
            if((reaped == -1 && errno != ECHILD){ // error on waitpid that isnt empty server
                printf("failed to reap zombie client process");
                close(client_socket);
                exit();
            }
        }
        else{ // child process
            handle_client(client_socket);
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
    
    if(listen(server_socket, MAX_CLIENT_QUEUE) == -1){ // designate socket as passive listener
        printf("socket listen failed.");
        close(server_socket);
        return 1;
    }
    
    printf("Starting server...");
    run_server(server_socket);
    
    // cleanup
    close(server_socket);

}
