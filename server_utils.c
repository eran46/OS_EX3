#include "server_utils.h"
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>


int incorrect_arg_num(int argc){
    if(argc < 2){
    	printf("Wrong number of arguments. (missing port)\n");
    	return 1;
    }
    else if(argc > 2){
    	printf("Wrong number of arguments. (too many arguments)\n");
    	return 1;
    }
    return 0;
}

void free_client_mem(Client* client){
	free(client->client_address);
	free(client->client_name);
	free(client);
}

struct sockaddr_in init_server_address(int port, char* server_ip_str){
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address)); // prevent garbage values    
    server_address.sin_family=AF_INET; // IPv4
    server_address.sin_port = htons(port); // port number - ensure Big Endian
    inet_aton(server_ip_str, &server_address.sin_addr); // CHANGE SERVER IP
    
    return server_address;
}


int bind_socket_to_addr(int socket,struct sockaddr_in *server_address){
    if(bind(socket, (struct sockaddr*)server_address, sizeof(*server_address)) == -1){ // bind failed
    	printf("binding socket to server address failed\n");
    	return -1;
    }
    return 0;
}

void init_clients(Client** clients){
	for (int i = 0; i < MAX_CONNECTED_CLIENTS; i++) {
        clients[i] = NULL;
    }
}

// Function to check if the clients array is full
int is_clients_full() {
    for (int i = 0; i < MAX_CONNECTED_CLIENTS; i++) {
        if (clients[i] == NULL) {  // NULL indicates unused slot
            return 0;  // Not full
        }
    }
    return 1;  // Full
}

// insert a new client into the clients array, trust there is room
// clients still have no name when inserted to clients !!
int insert_client(Client* new_client) {
    // Find an empty slot and insert the client
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CONNECTED_CLIENTS; i++) {
        if (clients[i] == NULL) {  // empty slot
            clients[i] = new_client;
            pthread_mutex_unlock(&clients_mutex);
            return i;  // Success
        }
    }

    return -1;  // Shouldn't reach here if array is properly checked
}

// remove a client (disconnect) based on their socket
void remove_client(int index) {
	pthread_mutex_lock(&clients_mutex); 
	close(clients[index]->client_socket); // close client socket
	free_client_mem(clients[index]); // free dynamically allocated memory
	clients[index] = NULL;
	pthread_cond_signal(&clients_cond);
	pthread_mutex_unlock(&clients_mutex);
}

// Helper function to get the index of a client in the clients array
int get_client_index(int client_socket) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CONNECTED_CLIENTS; i++) {
        if (clients[i]->client_socket && clients[i]->client_socket == client_socket) {
            pthread_mutex_unlock(&clients_mutex);
            return i;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    return -1;
}

// 0-text, -1-exit,  n-whisper with name with length n
int check_message_type(const char *input_string) {
    // check exit
    if (strcmp(input_string, "!exit") == 0) {
        return -1;
    }
    
    // check whisper
    // A MESSAGE "@name\0" or "@name \0" DOESNT COUNT AS A MESSAGE
    if (input_string[0] == '@') {
        int i = 1;
        // Ensure that the name is at least one character long and consists of letters/numbers
        while (input_string[i] != ' ' && input_string[i] != '\0') {
            i++;
        }
        
        // A MESSAGE @name or 
        // check for space after the name and text afterwards
        if (input_string[i] == ' ' && input_string[i + 1] != '\0') {
            return i-1;
        }
    }
    
    return 0;
}

int find_client_index_by_name(char* name) {
    for (int i = 0; i < MAX_CONNECTED_CLIENTS; i++) {
        if (clients[i] != NULL && strcmp(clients[i]->client_name, name) == 0) {
            return i; 
        }
    }
    return -1;  // client not found
}

void echo_msg_to_all_clients(Client* client, char* out_message){
	pthread_mutex_lock(&clients_mutex);  // accessing clients - critical region
	for (int i = 0; i < MAX_CONNECTED_CLIENTS; i++){
		if(clients[i] != NULL){
			if (send(clients[i]->client_socket, out_message, strlen(out_message), 0) < 0) {
				printf("failed while sending message from %s to all client. (send())\n", clients[i]->client_name);
			}
		}
	}
	pthread_mutex_unlock(&clients_mutex);
}





