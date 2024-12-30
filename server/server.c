#include <sys/socket.h> // sockets
#include <arpa/inet.h> // ipv4
#include <sys/types.h>
#include <unistd.h> // processs
#include <signal.h> // signals
#include "server_utils.h"

// clients array
Client* clients[MAX_CONNECTED_CLIENTS] = {0};

// clients[] synchronization 
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t clients_cond = PTHREAD_COND_INITIALIZER;


void handle_client_thread(Client* client) { // fix changed to client struct
    char buffer[BUFFER_SIZE];
    char client_ip[INET_ADDRSTRLEN];
    int bytes_received;
    
    // get human readable IPv4 address
    socklen_t str_size = INET_ADDRSTRLEN; // INET_ADDRSTRLEN = maximum length of a string representation of an IPv4 address, including the null terminator
    char* client_ip[str_size]; // human readable IPv4 string
    
    inet_ntop(AF_INET, ntohs(client->client_address.sin_addr), client_ip, str_size); // ensure correct Endiannes and convert binary IPv4 to human-readable string
        // AF_INET = IPv4, output to client_ip
    
    // get client name
    bytes_received = recv(client->client_socket, buffer, BUFFER_SIZE - 1, 0); // blocking call untill name is recieved
    buffer[bytes_received] = '\0'; 
    
    int client_name_ln = strlen(buffer); // actual length of name
    char* client_name = (char*)malloc(client_name_ln + 1); // +1 for null terminator
    if (client_name == NULL) {
        perror("failed to allocate memory for client_name");
        remove_client(client->client_socket);
    }

    strncpy(client_name, buffer, client_name_ln);
    client_name[client_name_ln] = '\0'; // ensure null termination
    client.client_name = client_name // save client name
    
    // print client name and ip
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0'; // avoids reading the whole buffer by accident
        printf("client %s connected from %s\n", client.client_name, client_ip);
        
    } else {
        printf("failed to receive client name from %s, removing client. (recv())\n", client_ip);
        remove_client(client->client_socket);
        return NULL;
    }
    
    
    // ---------------- Communicate with the client --------------->
    
// () > 0
    while (1) { // recieve loop from client
    	bytes_received = recv(client->client_socket, buffer, BUFFER_SIZE - 1, 0) // get message
        buffer[bytes_received] = '\0'; // Null-terminate the received data
	
	// check message type
	msg_type = check_message_type(buffer); 
	
	
	switch(msg_type){ // echo the message to intended clients
		case (-1): // exit
			if (send(*client_socket, buffer, bytes_received, 0) < 0) {
            			perror("send");
        		}
			remove_client(client->client_index);
			printf("client %s disconnected\n", client.client_name);
			break;
		case (0): // text
		
		break;
		default: // whisper
			char dest_name[msg_type + 1];
			strncpy(dest_name, buffer + 1, msg_type); // read after @, msg_type characters
			dest_name[msg_type] = 0; // ensure null terminat
			
			int dest_index = find_client_index_by_name(dest_name);
			char out_message[strlen(buffer)+ msg_type + 2]
			
			pthread_mutex_lock(&clients_mutex); 
			
			if (send(clients[reciever_index]->client_socket, buffer, bytes_received, 0) < 0) {
            			perror("send");
        		}
			remove_client(client->client_index);
			printf("client %s disconnected\n", client.client_name);
			
			pthread_mutex_unlock(&clients_mutex);
			break;
		
		break;
	}
        
        
        
        
    }

    if (bytes_received == 0) { // Disconnect/exit
        
        
        // on client disconnecting
        
        
    } else {
        perror("recv");
    }

    // Clean up happens in remove_client()
    
    // terminate child process
    exit(EXIT_SUCCESS);
}


void run_server(int server_socket){
    // reap zombie process infrastructure
    struct sigaction sa;
    sa.sa_handler = reap_zombies; // on signal () call reap_zombies()
    sa.sa_flags = SA_RESTART; // restart interrupted system callse
    sigaction(SIGCHLD, &sa, NULL); // action on SIGCHLD=when child process exits
    
    while(1){
    	if (is_clients_full()) {
            printf("no more room for clients in chat server. (Client array is full)\n");
            
            continue;  // Error, array is full
        }
        
        Client new_client = (Client*)malloc(sizeof(Client));
        new_client.client_address = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in)); // create client sockaddr_in object *pointer*
        new_client.client_socket = (int*)malloc(sizeof(int));// dynamic allocation in order to send pointer to handle_client_thread()
	socklen_t client_len = sizeof(client_address);
    
        new_client->client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_len); // blocking call, creates new client_socket from stream, binds to client_address
        if(new_client->client_socket == -1){ // failed handshake
            printf("server failed to connect to client. (accept())\n");
            free_client_mem(new_client);
            continue; // continue listening
        }
        
        int client_index;
        if ((client_index = insert_client(new_client)) != 0) {
            printf("failed to register new client. (insert_client())\n");
            close(new_client->client_socket);
            free_client_mem(new_client);
        }
        client->client_index = client_index; // save client insertion index in clients
        
        // --------- server connected to client ---------->
        
        pthread_t client_thread;
        if (pthread_create(&client_thread, NULL, handle_client_thread, clients[client_index]) != 0) {
            printf("Failed to create thread for client. (pthread_create())\n");
            remove_client(*new_client.client_socket); // cleans up socket and memory
            continue;
        }
        
        pthread_detach(client_thread); // no need for server_thread to wait for client_thread termination
        
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
