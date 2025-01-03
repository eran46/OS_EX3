#include "server_utils.h"

#include <sys/socket.h> // sockets
#include <arpa/inet.h> // ipv4
#include <sys/types.h>
#include <unistd.h> // processs
#include <signal.h> // signals
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


// clients array
Client* clients[MAX_CONNECTED_CLIENTS] = {0};

// clients[] synchronization 
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t clients_cond = PTHREAD_COND_INITIALIZER;

void* handle_client_thread(void* arg) { // fix changed to client struct
    // Client* client = (Client*)arg;
    int client_index = *(int*)arg;
    free(arg);
    char buffer[BUFFER_SIZE];
    int bytes_received;
    
    // get human readable IPv4 address
    socklen_t str_size = INET_ADDRSTRLEN; // INET_ADDRSTRLEN = maximum length of a string representation of an IPv4 address, including the null terminator
    char client_ip[str_size]; // human readable IPv4 string
    
    inet_ntop(AF_INET, &(clients[client_index]->client_address->sin_addr), client_ip, str_size); // convert binary IPv4 to human-readable string
        // AF_INET = IPv4, output to client_ip
    
    // get client name
    bytes_received = recv(clients[client_index]->client_socket, buffer, BUFFER_SIZE - 1, 0); // blocking call untill name is recieved
    buffer[bytes_received] = '\0'; 
    
    int client_name_ln = strlen(buffer); // actual length of name
    char* client_name = (char*)malloc(client_name_ln + 1); // +1 for null terminator
    if (client_name == NULL) {
        printf("failed to allocate memory for client_name\n");
        remove_client(client_index);
    }

    strncpy(client_name, buffer, client_name_ln);
    
    printf("[client_name(handle): %s]\n",client_name); // DEBUG
    
    client_name[client_name_ln] = '\0'; // ensure null termination
    clients[client_index]->client_name = client_name; // save client name
    
    // print client name and ip
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0'; // avoids reading the whole buffer by accident
        printf("client %s connected from %s\n", clients[client_index]->client_name, client_ip);
        
    } else {
        printf("failed to receive client name from %s, removing client. (recv())\n", client_ip);
        remove_client(client_index);
        return NULL;
    }
    
    // ---------------- Communicate with the client --------------->
    
    // server message recieve loop
    while (1) { 
    	bytes_received = recv(clients[client_index]->client_socket, buffer, BUFFER_SIZE - 1, 0); // get message
        buffer[bytes_received] = '\0'; // null terminate the received data
        
        
        // calculate size of sent message
	size_t src_len = strlen(clients[client_index]->client_name);
	size_t buffer_len = strlen(buffer);
	size_t out_message_len = src_len + buffer_len + 3; // ": " + null terminator
	
	// pointer to the message to be sent
	char out_message[out_message_len];

	// format the message
	snprintf(out_message, out_message_len, "%s: %s", clients[client_index]->client_name, buffer);
	
	/* 
	msg_type:
	an integert representing the mode of the message:
	(-1) = exit mode
	(0) = any text mode.
	o.w = whisper mode, *and the value is the length of the recipient's name*
	*/
	int msg_type = check_message_type(buffer);
	
	printf("[buffer: %s]\n", buffer); //DEBUG
			
	switch(msg_type){ 
		case (-1): // exit
			echo_msg_to_all_clients(clients[client_index], out_message); // echo the exit message to all the current clients
        		
			remove_client(client_index); // remove the client from clients + close socket + free memory
			printf("client %s disconnected\n", clients[client_index]->client_name);
			break;
		case (0): // text
			// echo the message to all the current clients
			echo_msg_to_all_clients(clients[client_index], out_message);
			break;
		default: // whisper
			char dest_name[msg_type+1]; // get destination client's name
			
			printf("[msg_type: %d]\n", msg_type); //DEBUG
			
			strncpy(dest_name, buffer + 1, msg_type); // parse destination client name
			dest_name[msg_type] = 0; // ensure null termination
			
			printf("[dest_name: %s]\n",dest_name); // DEBUG
			
			pthread_mutex_lock(&clients_mutex);  // accessing clients - critical region
			int dest_index;
			if ((dest_index = find_client_index_by_name(dest_name)) == -1){ // get destination client index in clients
				printf("failed to find destination client. (find_client_index_by_name()).\n");
				pthread_mutex_unlock(&clients_mutex);
				continue;
				
			}
			if (send(clients[dest_index]->client_socket, out_message, strlen(out_message), 0) < 0) {
            			printf("failed to send message from %s to client %s. (send())\n", clients[client_index]->client_name, dest_name);
        		}
			pthread_mutex_unlock(&clients_mutex);
			
			break;
	}
    }
    
    // terminate detached thread
    exit(EXIT_SUCCESS);
}


void run_server(int server_socket){
	while(1){
		pthread_mutex_lock(&clients_mutex);
	    	while(is_clients_full()){ // check if clients array is full - critical code 
	    		pthread_cond_wait(&clients_cond, &clients_mutex);
	    	}
	    	pthread_mutex_unlock(&clients_mutex);
		
		Client* new_client = (Client*)malloc(sizeof(Client));
		new_client->client_address = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in)); // create client sockaddr_in object *pointer*
		socklen_t client_len = sizeof(struct sockaddr_in); // size of client_address
	    
		new_client->client_socket = accept(server_socket, (struct sockaddr*)new_client->client_address, &client_len); // blocking call, creates new client_socket from stream, binds to client_address
		if(new_client->client_socket == -1){ // failed handshake
			printf("server failed to connect to client. (accept())\n");
			free_client_mem(new_client);
    			continue; // continue listening
		}
		
		int* client_index;
		if((client_index = (int*)malloc(sizeof(int))) == NULL){
			printf("failed to allocate memory for client_index.\n");
			close(new_client->client_socket);
			free_client_mem(new_client);
		}
		
		if ((*client_index = insert_client(new_client)) == -1) {
			printf("failed to register new client. (insert_client())\n");
			close(new_client->client_socket);
			free_client_mem(new_client);
		}
		
		clients[*client_index]->client_index = *client_index; // save client insertion index in clients
		
		// --------- server connected to client ---------->
		
		pthread_t client_thread;
		if (pthread_create(&client_thread, NULL, handle_client_thread, (void*)client_index) != 0) {
			printf("Failed to create thread for client. (pthread_create())\n");
			remove_client(new_client->client_index); // cleans up socket and memory
			continue;
		}
		
		pthread_detach(client_thread); // no need for server_thread to wait for client_thread termination
        
    }
}

int main(int argc, char* argv[]){
    
    // localhost ip for testing
    char server_ip[16] = "127.0.0.1"; // SERVER IP
    
    if(incorrect_arg_num(argc)){ // check bad argument number
    	return 1;
    }
    
    int port = atoi(argv[1]); // server port
    
    // initialize socket
    int server_socket; 
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) { // IPv4, Stream Socket, 0=default=TCP
        printf("Socket creation failed");
        return 1;
    }
    
    // struct sockaddr -> generic, struct sockaddr_in -> IPv4
    struct sockaddr_in server_address = init_server_address(port, server_ip); // initialize the server address
    
    if(bind_socket_to_addr(server_socket, &server_address)){ // bind socket to server address
    	close(server_socket);
    	return 1;
    }
    
    if(listen(server_socket, MAX_CLIENT_QUEUE) == -1){ // listen for incoming TCP connections on port
        printf("socket listen failed.\n");
        close(server_socket);
        return 1;
    }
    
    run_server(server_socket);
    
    // cleanup
    close(server_socket);

}
